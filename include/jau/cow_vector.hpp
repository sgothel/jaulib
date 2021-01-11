/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020 Gothel Software e.K.
 * Copyright (c) 2020 ZAFENA AB
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef JAU_COW_VECTOR_HPP_
#define JAU_COW_VECTOR_HPP_

#include <cstring>
#include <string>
#include <cstdint>
#include <limits>
#include <atomic>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <algorithm>

#include <jau/cpp_lang_macros.hpp>
#include <jau/debug.hpp>
#include <jau/basic_types.hpp>
#include <jau/ordered_atomic.hpp>
#include <jau/cow_iterator.hpp>

namespace jau {

    /**
     * Implementation of a Copy-On-Write (CoW) using std::vector as the underlying storage,
     * exposing <i>lock-free</i> read operations using SC-DRF atomic synchronization.
     * <p>
     * This class shall be compliant with <i>C++ named requirements for Container</i>.
     * </p>
     * <p>
     * The vector's store is owned using a shared reference to the data structure,
     * allowing its replacement on Copy-On-Write (CoW).
     * </p>
     * <p>
     * Writing to the store utilizes a mutex lock to avoid data races
     * on the instances' write operations only, leaving read operations <i>lock-free</i>.<br>
     * Write operations replace the store reference with a new instance using
     * jau::sc_atomic_critical to synchronize with read operations.
     * </p>
     * <p>
     * Reading from the store is <i>lock-free</i> and accesses the store reference using
     * jau::sc_atomic_critical to synchronizing with write operations.
     * </p>
     * <p>
     * Immutable storage const_iterators are supported via jau::cow_ro_iterator,
     * which are constructed <i>lock-free</i>.<br>
     * jau::cow_ro_iterator hold a snapshot retrieved via jau::cow_vector::snapshot()
     * until its destruction.
     * </p>
     * <p>
     * Mutable storage iterators are supported via jau::cow_rw_iterator,
     * which holds a copy of this CoW storage and locks its write mutex until
     * jau::cow_rw_iterator::write_back() or its destruction.<br>
     * After completing all mutable operations but before this iterator's destruction,
     * the user might want to write back this iterators' storage to this CoW
     * using jau::cow_rw_iterator::write_back().
     * </p>
     * <p>
     * Index operation via ::operator[](size_type) or ::at(size_type) are not supported,
     * since they would be only valid if value_type itself is a std::shared_ptr
     * and hence prohibit the destruction of the object if mutating the storage,
     * e.g. via jau::cow_vector::push_back().
     * </p>
     * <p>
     * Custom mutable write operations are also supported via
     * jau::cow_vector::get_write_mutex(), jau::cow_vector::copy_store() and jau::cow_vector::set_store().<br>
     * See example in jau::cow_vector::set_store()
     * </p>
     * See also:
     * <pre>
     * - Sequentially Consistent (SC) ordering or SC-DRF (data race free) <https://en.cppreference.com/w/cpp/atomic/memory_order#Sequentially-consistent_ordering>
     * - std::memory_order <https://en.cppreference.com/w/cpp/atomic/memory_order>
     * </pre>
     * \deprecated jau::cow_vector will be retired, use jau::cow_darray and potentially jau::darray.
     * @see jau::cow_darray
     * @see jau::cow_ro_iterator
     * @see jau::for_each_fidelity
     * @see jau::cow_rw_iterator
     * @see jau::cow_rw_iterator::write_back()
     */
    template <typename Value_type, typename Alloc_type = std::allocator<Value_type>>
    class cow_vector
    {
        public:
            // typedefs' for C++ named requirements: Container

            typedef Value_type                                  value_type;
            typedef value_type*                                 pointer;
            typedef const value_type*                           const_pointer;
            typedef value_type&                                 reference;
            typedef const value_type&                           const_reference;
            typedef std::size_t                                 size_type;
            typedef typename std::make_signed<size_type>::type  difference_type;
            typedef Alloc_type                                  allocator_type;

            typedef std::vector<value_type, allocator_type>     storage_t;
            typedef std::shared_ptr<storage_t>                  storage_ref_t;

            typedef cow_vector<value_type, allocator_type>      cow_container_t;

            /**
             * @see jau::cow_darray::const_iterator
             * @see jau::cow_ro_iterator
             */
            typedef cow_ro_iterator<storage_t, storage_ref_t, cow_container_t> const_iterator;

            /**
             * @see jau::cow_darray::iterator
             * @see jau::cow_rw_iterator
             */
            typedef cow_rw_iterator<storage_t, storage_ref_t, cow_container_t> iterator;

        private:
            static constexpr size_type DIFF_MAX = std::numeric_limits<difference_type>::max();

            storage_ref_t store_ref;
            mutable sc_atomic_bool sync_atomic;
            mutable std::recursive_mutex mtx_write;

        public:
            // ctor

            constexpr cow_vector() noexcept
            : store_ref( std::make_shared<storage_t>() ), sync_atomic(false) {}

            constexpr explicit cow_vector(const allocator_type & a) noexcept
            : store_ref( std::make_shared<storage_t>(a) ), sync_atomic(false) { }

            constexpr explicit cow_vector(size_type n, const allocator_type& a = allocator_type())
            : store_ref( std::make_shared<storage_t>(n, a) ), sync_atomic(false) { }

            constexpr cow_vector(size_type n, const value_type& value, const allocator_type& a = allocator_type())
            : store_ref( std::make_shared<storage_t>(n, value, a) ), sync_atomic(false) { }

            constexpr explicit cow_vector(const storage_t& x)
            : store_ref( std::make_shared<storage_t>(x, x->get_allocator()) ), sync_atomic(false) { }

            __constexpr_non_literal_atomic__
            cow_vector(const cow_vector& x)
            : sync_atomic(false) {
                storage_ref_t x_store_ref;
                {
                    sc_atomic_critical sync_x( x.sync_atomic );
                    x_store_ref = x.store_ref;
                }
                store_ref = std::make_shared<storage_t>( *x_store_ref, x_store_ref->get_allocator() );
            }

            /**
             * Like std::vector::operator=(&), assignment
             * <p>
             * This write operation uses a mutex lock and is blocking this instances' write operations only.
             * </p>
             */
            cow_vector& operator=(const cow_vector& x) {
                std::lock_guard<std::recursive_mutex> lock(mtx_write);
                storage_ref_t x_store_ref;
                {
                    sc_atomic_critical sync_x( x.sync_atomic );
                    x_store_ref = x.store_ref;
                }
                storage_ref_t new_store_ref = std::make_shared<storage_t>( *x_store_ref, x_store_ref->get_allocator() );
                {
                    sc_atomic_critical sync(sync_atomic);
                    store_ref = std::move(new_store_ref);
                }
                return *this;
            }

            constexpr cow_vector(cow_vector && x) noexcept {
                // Stragtegy-1: Acquire lock, blocking
                // - If somebody else holds the lock, we wait.
                // - Then we own the lock
                // - Post move-op, the source object does not exist anymore
                std::unique_lock<std::recursive_mutex>  lock(x.mtx_write); // *this doesn't exist yet, not locking ourselves
                {
                    store_ref = std::move(x.store_ref);
                    // sync_atomic = std::move(x.sync_atomic);
                    // mtx_write will be a fresh one, but we hold the source's lock

                    // Moved source array has been taken over, null its store_ref
                    x.store_ref = nullptr;
                }
            }

            /**
             * Like std::vector::operator=(&&), move.
             * <p>
             * This write operation uses a mutex lock and is blocking both cow_vector instance's write operations.
             * </p>
             */
            cow_vector& operator=(cow_vector&& x) {
                // Stragtegy-2: Acquire locks of both, blocking
                // - If somebody else holds the lock, we wait.
                // - Then we own the lock for both instances
                // - Post move-op, the source object does not exist anymore
                std::unique_lock<std::recursive_mutex> lock1(x.mtx_write, std::defer_lock); // utilize std::lock(r, w), allowing mixed order waiting on read/write ops
                std::unique_lock<std::recursive_mutex> lock2(  mtx_write, std::defer_lock); // otherwise RAII-style relinquish via destructor
                std::lock(lock1, lock2);
                {
                    sc_atomic_critical sync_x( x.sync_atomic );
                    sc_atomic_critical sync  (   sync_atomic );
                    store_ref = std::move(x.store_ref);
                    // mtx_write and the atomic will be kept as is, but we hold the source's lock

                    // Moved source array has been taken over, null its store_ref
                    x.store_ref = nullptr;
                }
                return *this;
            }

            /**
             * Creates a new instance,
             * copying all elements from the given template input-iterator value_type range [first, last).<br>
             * Size will equal the range [first, last), i.e. <code>size_type(last-first)</code>.
             * @tparam InputIt template input-iterator custom type
             * @param first template input-iterator to first element of value_type range [first, last)
             * @param last template input-iterator to last element of value_type range [first, last)
             * @param alloc custom allocator_type instance
             */
            template< class InputIt >
            constexpr cow_vector(InputIt first, InputIt last, const allocator_type& alloc = allocator_type())
            : store_ref(std::make_shared<storage_t>(first, last, alloc)), sync_atomic(false)
            { }

            /**
             * Create a new instance from an initializer list.
             *
             * @param initlist initializer_list.
             * @param alloc allocator
             */
            constexpr cow_vector(std::initializer_list<value_type> initlist, const allocator_type& alloc = allocator_type())
            : store_ref(std::make_shared<storage_t>(initlist, alloc)), sync_atomic(false)
            { }

            ~cow_vector() noexcept { }

            /**
             * Returns <code>std::numeric_limits<difference_type>::max()</code> as the maximum array size.
             * <p>
             * We rely on the signed <code>difference_type</code> for pointer arithmetic,
             * deducing ranges from iterator.
             * </p>
             */
            constexpr size_type max_size() const noexcept { return DIFF_MAX; }

            // cow_vector features

            /**
             * Returns this instances' recursive write mutex, allowing user to
             * implement more complex mutable write operations.
             * <p>
             * See example in jau::cow_vector::set_store()
             * </p>
             *
             * @see jau::cow_vector::get_write_mutex()
             * @see jau::cow_vector::copy_store()
             * @see jau::cow_vector::set_store()
             */
            constexpr std::recursive_mutex & get_write_mutex() noexcept { return mtx_write; }

            /**
             * Returns a new shared_ptr copy of the underlying store,
             * i.e. using a new copy-constructed vectore.
             * <p>
             * See example in jau::cow_vector::set_store()
             * </p>
             * <p>
             * This special operation uses a mutex lock and is blocking this instances' write operations only.
             * </p>
             * @see jau::cow_vector::get_write_mutex()
             * @see jau::cow_vector::copy_store()
             * @see jau::cow_vector::set_store()
             */
            __constexpr_non_literal_atomic__
            storage_ref_t copy_store() {
                std::lock_guard<std::recursive_mutex> lock(mtx_write);
                return std::make_shared<storage_t>( *store_ref, store_ref->get_allocator() );
            }

            /**
             * Special case facility allowing the user to replace the current store
             * with the given value, potentially acquired via jau::cow_vector::copy_store()
             * and mutated while holding the jau::cow_vector::get_write_mutex() lock.
             * <p>
             * This is a move operation, i.e. the given new_store_ref is invalid on the caller side
             * after this operation. <br>
             * User shall pass the store via std::move()
             * <pre>
             *     cow_vector<std::shared_ptr<Thing>> list;
             *     ...
             *     {
             *         std::lock_guard<std::recursive_mutex> lock(list.get_write_mutex());
             *         std::shared_ptr<std::vector<std::shared_ptr<Thing>>> snapshot = list.copy_store();
             *         ...
             *         some fancy mutation
             *         ...
             *         list.set_store(std::move(snapshot));
             *     }
             * </pre>
             * </p>
             * @param new_store_ref the user store to be moved here, replacing the current store.
             *
             * @see jau::cow_vector::get_write_mutex()
             * @see jau::cow_vector::copy_store()
             * @see jau::cow_vector::set_store()
             */
            __constexpr_non_literal_atomic__
            void set_store(storage_ref_t && new_store_ref) noexcept {
                std::lock_guard<std::recursive_mutex> lock(mtx_write);
                sc_atomic_critical sync(sync_atomic);
                store_ref = std::move( new_store_ref );
            }

            /**
             * Returns the current snapshot of the underlying shared std::vector<T> reference.
             * <p>
             * Note that this snapshot will be outdated by the next (concurrent) write operation.<br>
             * The returned referenced vector is still valid and not mutated,
             * but does not represent the current content of this cow_vector instance.
             * </p>
             * <p>
             * This read operation is <i>lock-free</i>.
             * </p>
             * @see jau::for_each_cow
             */
            __constexpr_non_literal_atomic__
            storage_ref_t snapshot() const noexcept {
                sc_atomic_critical sync( sync_atomic );
                return store_ref;
            }

            // const_iterator, non mutable, read-only

            // Removed for clarity: "constexpr const_iterator begin() const noexcept"

            /**
             * See description in jau::cow_darray::cbegin()
             */
            constexpr const_iterator cbegin() const noexcept {
                return const_iterator(snapshot(), store_ref->cbegin());
            }

            // iterator, mutable, read-write

            /**
             * See description in jau::cow_darray::begin()
             */
            constexpr iterator begin() {
                return iterator(*this);
            }

            // read access

            allocator_type get_allocator() const noexcept {
                sc_atomic_critical sync( sync_atomic );
                return store_ref->get_allocator();
            }

            __constexpr_non_literal_atomic__
            size_type capacity() const noexcept {
                sc_atomic_critical sync( sync_atomic );
                return store_ref->capacity();
            }

            /**
             * Like std::vector::empty().
             * <p>
             * This read operation is <i>lock-free</i>.
             * </p>
             */
            __constexpr_non_literal_atomic__
            bool empty() const noexcept {
                sc_atomic_critical sync( sync_atomic );
                return store_ref->empty();
            }

            /**
             * Like std::vector::size().
             * <p>
             * This read operation is <i>lock-free</i>.
             * </p>
             */
            __constexpr_non_literal_atomic__
            size_type size() const noexcept {
                sc_atomic_critical sync( sync_atomic );
                return store_ref->size();
            }

            // write access

            void reserve(size_type new_capacity) {
                std::lock_guard<std::recursive_mutex> lock(mtx_write);
                storage_ref_t old_store_ref = store_ref;
                if( new_capacity > old_store_ref->capacity() ) {
                    storage_ref_t new_store_ref = std::make_shared<storage_t>( *old_store_ref, old_store_ref->get_allocator() );
                    new_store_ref->reserve(new_capacity);
                    sc_atomic_critical sync( sync_atomic );
                    store_ref = std::move(new_store_ref);
                }
            }

            /**
             * Like std::vector::clear(), but ending with zero capacity.
             * <p>
             * This write operation uses a mutex lock and is blocking this instances' write operations.
             * </p>
             */
            __constexpr_non_literal_atomic__
            void clear() noexcept {
                std::lock_guard<std::recursive_mutex> lock(mtx_write);
                storage_ref_t new_store_ref = std::make_shared<storage_t>();
                {
                    sc_atomic_critical sync(sync_atomic);
                    store_ref = std::move(new_store_ref);
                }
            }

            /**
             * Like std::vector::swap().
             * <p>
             * This write operation uses a mutex lock and is blocking both cow_vector instance's write operations.
             * </p>
             */
            __constexpr_non_literal_atomic__
            void swap(cow_vector& x) noexcept {
                std::unique_lock<std::recursive_mutex> lock(mtx_write, std::defer_lock); // utilize std::lock(a, b), allowing mixed order waiting on either object
                std::unique_lock<std::recursive_mutex> lock_x(x.mtx_write, std::defer_lock); // otherwise RAII-style relinquish via destructor
                std::lock(lock, lock_x);
                {
                    sc_atomic_critical sync_x( x.sync_atomic );
                    sc_atomic_critical sync(sync_atomic);
                    storage_ref_t x_store_ref = x.store_ref;
                    x.store_ref = store_ref;
                    store_ref = x_store_ref;
                }
            }

            /**
             * Like std::vector::pop_back().
             * <p>
             * This write operation uses a mutex lock and is blocking this instances' write operations only.
             * </p>
             */
            __constexpr_non_literal_atomic__
            void pop_back() noexcept {
                std::lock_guard<std::recursive_mutex> lock(mtx_write);
                storage_ref_t old_store_ref = store_ref;
                if( 0 < old_store_ref->size() ) {
                    storage_ref_t new_store_ref = std::make_shared<storage_t>( *old_store_ref, old_store_ref->get_allocator() );
                    new_store_ref->pop_back();
                    {
                        sc_atomic_critical sync(sync_atomic);
                        store_ref = std::move(new_store_ref);
                    }
                }
            }

            /**
             * Like std::vector::push_back(), copy
             * <p>
             * This write operation uses a mutex lock and is blocking this instances' write operations only.
             * </p>
             * @param x the value to be added at the tail.
             */
            __constexpr_non_literal_atomic__
            void push_back(const value_type& x) {
                std::lock_guard<std::recursive_mutex> lock(mtx_write);
                storage_ref_t new_store_ref = std::make_shared<storage_t>( *store_ref, store_ref->get_allocator() );
                new_store_ref->push_back(x);
                {
                    sc_atomic_critical sync(sync_atomic);
                    store_ref = std::move(new_store_ref);
                }
            }

            /**
             * Like std::vector::push_back(), move
             * <p>
             * This write operation uses a mutex lock and is blocking this instances' write operations only.
             * </p>
             */
            __constexpr_non_literal_atomic__
            void push_back(value_type&& x) {
                std::lock_guard<std::recursive_mutex> lock(mtx_write);
                storage_ref_t new_store_ref = std::make_shared<storage_t>( *store_ref, store_ref->get_allocator() );
                new_store_ref->push_back( std::move(x) );
                {
                    sc_atomic_critical sync(sync_atomic);
                    store_ref = std::move(new_store_ref);
                }
            }

            /**
             * Like std::vector::emplace_back(), construct a new element in place at the end().
             * <p>
             * Constructs the element at the end() using placement new.
             * </p>
             * <p>
             * size will be increased by one.
             * </p>
             * @param args arguments to forward to the constructor of the element
             */
            template<typename... Args>
            __constexpr_non_literal_atomic__
            reference emplace_back(Args&&... args) {
                std::lock_guard<std::recursive_mutex> lock(mtx_write);
                storage_ref_t new_store_ref = std::make_shared<storage_t>( *store_ref, store_ref->get_allocator() );
                reference res = new_store_ref->emplace_back( std::forward<Args>(args)... );
                {
                    sc_atomic_critical sync(sync_atomic);
                    store_ref = std::move(new_store_ref);
                }
                return res;
            }

            /**
             * Generic value_type equal comparator to be user defined for e.g. jau::cow_vector::push_back_unique().
             * @param a one element of the equality test.
             * @param b the other element of the equality test.
             * @return true if both are equal
             */
            typedef bool(*equal_comparator)(const value_type& a, const value_type& b);

            /**
             * Like std::vector::push_back(), but only if the newly added element does not yet exist.
             * <p>
             * This write operation uses a mutex lock and is blocking this instances' write operations only.
             * </p>
             * <p>
             * Examples
             * <pre>
             *     static jau::cow_vector<Thing>::equal_comparator thingEqComparator =
             *                  [](const Thing &a, const Thing &b) -> bool { return a == b; };
             *     ...
             *     jau::cow_vector<Thing> list;
             *
             *     bool added = list.push_back_unique(new_element, thingEqComparator);
             *     ...
             *     cow_vector<std::shared_ptr<Thing>> listOfRefs;
             *     bool added = listOfRefs.push_back_unique(new_element,
             *                    [](const std::shared_ptr<Thing> &a, const std::shared_ptr<Thing> &b) -> bool { return *a == *b; });
             * </pre>
             * </p>
             * @param x the value to be added at the tail, if not existing yet.
             * @param comparator the equal comparator to return true if both given elements are equal
             * @return true if the element has been uniquely added, otherwise false
             */
            __constexpr_non_literal_atomic__
            bool push_back_unique(const value_type& x, equal_comparator comparator) {
                std::lock_guard<std::recursive_mutex> lock(mtx_write);
                for(auto it = store_ref->begin(); it != store_ref->end(); ) {
                    if( comparator( *it, x ) ) {
                        return false; // already included
                    } else {
                        ++it;
                    }
                }
                push_back(x);
                return true;
            }

            /**
             * Erase either the first matching element or all matching elements.
             * <p>
             * This write operation uses a mutex lock and is blocking this instances' write operations only.
             * </p>
             * <p>
             * Examples
             * <pre>
             *     cow_vector<Thing> list;
             *     int count = list.erase_matching(element, true,
             *                    [](const Thing &a, const Thing &b) -> bool { return a == b; });
             *     ...
             *     static jau::cow_vector<Thing>::equal_comparator thingRefEqComparator =
             *                  [](const std::shared_ptr<Thing> &a, const std::shared_ptr<Thing> &b) -> bool { return *a == *b; };
             *     ...
             *     cow_vector<std::shared_ptr<Thing>> listOfRefs;
             *     int count = listOfRefs.erase_matching(element, false, thingRefEqComparator);
             * </pre>
             * </p>
             * @param x the value to be added at the tail, if not existing yet.
             * @param all_matching if true, erase all matching elements, otherwise only the first matching element.
             * @param comparator the equal comparator to return true if both given elements are equal
             * @return number of erased elements
             */
            __constexpr_non_literal_atomic__
            int erase_matching(const value_type& x, const bool all_matching, equal_comparator comparator) {
                int count = 0;
                std::lock_guard<std::recursive_mutex> lock(mtx_write);
                storage_ref_t new_store_ref = std::make_shared<storage_t>( *store_ref, store_ref->get_allocator() );
                for(auto it = new_store_ref->begin(); it != new_store_ref->end(); ) {
                    if( comparator( *it, x ) ) {
                        it = new_store_ref->erase(it);
                        ++count;
                        if( !all_matching ) {
                            break;
                        }
                    } else {
                        ++it;
                    }
                }
                if( 0 < count ) { // mutated new_store_ref?
                    sc_atomic_critical sync(sync_atomic);
                    store_ref = std::move(new_store_ref);
                } // else throw away new_store_ref
                return count;
            }

            __constexpr_cxx20_ std::string toString() const noexcept {
                std::string res("{ " + std::to_string( size() ) + ": ");
                int i=0;
                jau::for_each_const(*this, [&res, &i](const value_type & e) {
                    if( 1 < ++i ) { res.append(", "); }
                    res.append( jau::to_string(e) );
                } );
                res.append(" }");
                return res;
            }
    };

    /****************************************************************************************
     ****************************************************************************************/

    template<typename Value_type, typename Alloc_type>
    std::ostream & operator << (std::ostream &out, const cow_vector<Value_type, Alloc_type> &c) {
        out << c.toString();
        return out;
    }

    /****************************************************************************************
     ****************************************************************************************/

    template<typename Value_type, typename Alloc_type>
    inline bool operator==(const cow_vector<Value_type, Alloc_type>& rhs, const cow_vector<Value_type, Alloc_type>& lhs) {
        if( &rhs == &lhs ) {
            return true;
        }
        typename cow_vector<Value_type, Alloc_type>::const_iterator rhs_cend = rhs.cbegin();
        rhs_cend += rhs.size();
        return (rhs.size() == lhs.size() && std::equal(rhs.cbegin(), rhs_cend, lhs.cbegin()));
    }
    template<typename Value_type, typename Alloc_type>
    inline bool operator!=(const cow_vector<Value_type, Alloc_type>& rhs, const cow_vector<Value_type, Alloc_type>& lhs) {
        return !(rhs==lhs);
    }

    template<typename Value_type, typename Alloc_type>
    inline bool operator<(const cow_vector<Value_type, Alloc_type>& rhs, const cow_vector<Value_type, Alloc_type>& lhs) {
        typename cow_vector<Value_type, Alloc_type>::const_iterator rhs_cend = rhs.cbegin();
        rhs_cend += rhs.size();
        typename cow_vector<Value_type, Alloc_type>::const_iterator lhs_cend = lhs.cbegin();
        lhs_cend += lhs.size();
        return std::lexicographical_compare(rhs.cbegin(), rhs_cend, lhs.begin(), lhs_cend);
    }

    template<typename Value_type, typename Alloc_type>
    inline bool operator>(const cow_vector<Value_type, Alloc_type>& rhs, const cow_vector<Value_type, Alloc_type>& lhs)
    { return lhs < rhs; }

    template<typename Value_type, typename Alloc_type>
    inline bool operator<=(const cow_vector<Value_type, Alloc_type>& rhs, const cow_vector<Value_type, Alloc_type>& lhs)
    { return !(lhs < rhs); }

    template<typename Value_type, typename Alloc_type>
    inline bool operator>=(const cow_vector<Value_type, Alloc_type>& rhs, const cow_vector<Value_type, Alloc_type>& lhs)
    { return !(rhs < lhs); }

    template<typename Value_type, typename Alloc_type>
    inline void swap(cow_vector<Value_type, Alloc_type>& rhs, cow_vector<Value_type, Alloc_type>& lhs) noexcept
    { rhs.swap(lhs); }
} /* namespace jau */

#endif /* JAU_COW_VECTOR_HPP_ */
