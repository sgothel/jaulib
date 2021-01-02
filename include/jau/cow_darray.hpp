/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020 Gothel Software e.K.
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

#ifndef JAU_COW_DARRAY_HPP_
#define JAU_COW_DARRAY_HPP_

#include <cstring>
#include <string>
#include <cstdint>
#include <atomic>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <algorithm>

#include <jau/debug.hpp>
#include <jau/darray.hpp>
#include <jau/basic_types.hpp>
#include <jau/ordered_atomic.hpp>
#include <jau/cow_iterator.hpp>

namespace jau {

    /**
     * Implementation of a Copy-On-Write (CoW) using jau::darray as the underlying storage,
     * exposing <i>lock-free</i> read operations using SC-DRF atomic synchronization.
     * <p>
     * The store is owned using a shared reference to the data structure,
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
     * jau::cow_ro_iterator hold a snapshot retrieved via jau::cow_darray::get_snapshot()
     * until its destruction.
     * </p>
     * <p>
     * Mutable storage iterators are supported via jau::cow_rw_iterator,
     * which are constructed holding the write-lock.<br>
     * jau::cow_rw_iterator hold a new store copy via jau::cow_darray::copy_store(),
     * which replaces the current store via jau::cow_darray::set_store() at destruction.
     * </p>
     * <p>
     * Both, jau::cow_ro_iterator and jau::cor_rw_iterator are harmonized
     * to work with jau::darray::const_iterator and jau::darray::iterator
     * for all iterator based operations.
     * </p>
     * <p>
     * Index operation via ::operator[](size_t) or ::at(size_t) are not supported for now,
     * since they would be only valid if Value_type itself is a std::shared_ptr
     * and hence prohibit the destruction of the object if mutating the storage,
     * e.g. via jau::cow_darray::push_back().
     * </p>
     * <p>
     * Custom mutable write operations are also supported via
     * jau::cow_darray::get_write_mutex(), jau::cow_darray::copy_store() and jau::cow_darray::set_store().<br>
     * See example in jau::cow_darray::set_store()
     * </p>
     *
     * </p>
     * See also:
     * <pre>
     * - Sequentially Consistent (SC) ordering or SC-DRF (data race free) <https://en.cppreference.com/w/cpp/atomic/memory_order#Sequentially-consistent_ordering>
     * - std::memory_order <https://en.cppreference.com/w/cpp/atomic/memory_order>
     * </pre>
     */
    template <typename Value_type, typename Alloc_type = std::allocator<Value_type>, typename Size_type = jau::nsize_t>
    class cow_darray
    {
        public:
            typedef darray<Value_type, Alloc_type, Size_type> storage_t;
            typedef std::shared_ptr<storage_t> storage_ref_t;

            /**
             * Immutable, read-only const_iterator, lock-free,
             * holding the current shared store reference until destruction.
             * <p>
             * Using jau::cow_darray::get_snapshot() at construction.
             * </p>
             */
            typedef cow_ro_iterator<Value_type, storage_t, storage_ref_t> const_iterator;

            /**
             * Mutable, read-write iterator, holding the write-lock and a store copy until destruction.
             * <p>
             * Using jau::cow_darray::get_write_mutex(), jau::cow_darray::copy_store() at construction<br>
             * and jau::cow_darray::set_store() at destruction.
             * </p>
             */
            typedef cow_rw_iterator<Value_type, storage_t, storage_ref_t, cow_darray> iterator;

            /** Default growth factor using the golden ratio 1.618 */
            inline static const float DEFAULT_GROWTH_FACTOR = 1.618f;

        private:
            storage_ref_t store_ref;
            sc_atomic_bool sync_atomic;
            std::recursive_mutex mtx_write;

        public:
            // ctor w/o elements

            /**
             * Default constructor, giving almost zero capacity and zero memory footprint, but the shared empty jau::darray
             */
            constexpr cow_darray() noexcept
            : store_ref(std::make_shared<storage_t>()), sync_atomic(false) {}

            /**
             * Creating an empty instance with initial capacity and other (default) properties.
             * @param capacity initial capacity of the new instance.
             * @param growth_factor given growth factor
             * @param alloc given Alloc_type
             */
            constexpr explicit cow_darray(Size_type capacity, const float growth_factor=DEFAULT_GROWTH_FACTOR, const Alloc_type& alloc = Alloc_type())
            : store_ref(std::make_shared<storage_t>(capacity, growth_factor, alloc)), sync_atomic(false) {}

            // conversion ctor on storage_t elements

            constexpr cow_darray(const storage_t& x)
            : store_ref(std::make_shared<storage_t>(x)), sync_atomic(false) {}

            constexpr explicit cow_darray(const storage_t& x, const float growth_factor, const Alloc_type& alloc)
            : store_ref(std::make_shared<storage_t>(x, growth_factor, alloc)), sync_atomic(false) {}

            constexpr cow_darray(storage_t && x) noexcept
            : store_ref(std::make_shared<storage_t>(std::move(x))), sync_atomic(false) {}

            constexpr explicit cow_darray(storage_t && x, const float growth_factor, const Alloc_type& alloc) noexcept
            : store_ref(std::make_shared<storage_t>(std::move(x), growth_factor, alloc)), sync_atomic(false) {}

            // copy_ctor on cow_darray elements

            /**
             * Creates a new instance, copying all elements from the given array.<br>
             * Capacity and size will equal the given array, i.e. the result is a trimmed array.
             * @param x the given cow_darray, all elements will be copied into the new instance.
             */
            constexpr cow_darray(const cow_darray& x)
            : sync_atomic(false) {
                storage_ref_t x_store_ref;
                {
                    sc_atomic_critical sync_x( const_cast<cow_darray *>(&x)->sync_atomic );
                    x_store_ref = x.store_ref;
                }
                store_ref = std::make_shared<storage_t>( *x_store_ref );
            }

            /**
             * Creates a new instance, copying all elements from the given array.<br>
             * Capacity and size will equal the given array, i.e. the result is a trimmed array.
             * @param x the given cow_darray, all elements will be copied into the new instance.
             * @param growth_factor custom growth factor
             * @param alloc custom Alloc_type instance
             */
            constexpr explicit cow_darray(const cow_darray& x, const float growth_factor, const Alloc_type& alloc)
            : sync_atomic(false) {
                storage_ref_t x_store_ref;
                {
                    sc_atomic_critical sync_x( const_cast<cow_darray *>(&x)->sync_atomic );
                    x_store_ref = x.store_ref;
                }
                store_ref = std::make_shared<storage_t>( *x_store_ref, growth_factor, alloc );
            }

            /**
             * Creates a new instance with custom initial storage capacity, copying all elements from the given array.<br>
             * Size will equal the given array.
             * <p>
             * Calculated safe capacity is: <code>std::max<Size_type>(x.size(), _capacity)</code>
             * </p>
             * @param x the given cow_darray, all elements will be copied into the new instance.
             * @param _capacity custom initial storage capacity
             * @param growth_factor custom growth factor
             * @param alloc custom Alloc_type instance
             */
            constexpr explicit cow_darray(const cow_darray& x, const Size_type _capacity, const float growth_factor, const Alloc_type& alloc)
            : sync_atomic(false) {
                storage_ref_t x_store_ref;
                {
                    sc_atomic_critical sync_x( const_cast<cow_darray *>(&x)->sync_atomic );
                    x_store_ref = x.store_ref;
                }
                store_ref = std::make_shared<storage_t>( *x_store_ref, _capacity, growth_factor, alloc );
            }

            // move_ctor on cow_darray elements

            constexpr cow_darray(cow_darray && x) noexcept {
                // swap store_ref
                store_ref = std::move(x.store_ref);
                x.store_ref = nullptr;
                // not really necessary
                sync_atomic = std::move(x.sync_atomic);
            }

            // ctor on const_iterator and foreign template iterator

            /**
             * Creates a new instance with custom initial storage capacity,
             * copying all elements from the given const_iterator Value_type range [first, last).<br>
             * Size will equal the range [first, last), i.e. <code>Size_type(last-first)</code>.
             * <p>
             * Calculated safe capacity is: <code>std::max<Size_type>(Size_type(last-first), _capacity)</code>
             * </p>
             * @param _capacity custom initial storage capacity
             * @param first const_iterator to first element of Value_type range [first, last)
             * @param last const_iterator to last element of Value_type range [first, last)
             * @param growth_factor custom growth factor
             * @param alloc custom Alloc_type instance
             */
            constexpr cow_darray(const Size_type _capacity, const_iterator first, const_iterator last,
                             const float growth_factor=DEFAULT_GROWTH_FACTOR, const Alloc_type& alloc = Alloc_type())
            : store_ref(std::make_shared<storage_t>(_capacity, first.underling(), last.underling(), growth_factor, alloc)), sync_atomic(false)
            { }

            /**
             * Creates a new instance with custom initial storage capacity,
             * copying all elements from the given template input-iterator Value_type range [first, last).<br>
             * Size will equal the range [first, last), i.e. <code>Size_type(last-first)</code>.
             * <p>
             * Calculated safe capacity is: <code>std::max<Size_type>(Size_type(last-first), _capacity)</code>
             * </p>
             * @tparam InputIt template input-iterator custom type
             * @param _capacity custom initial storage capacity
             * @param first template input-iterator to first element of Value_type range [first, last)
             * @param last template input-iterator to last element of Value_type range [first, last)
             * @param growth_factor custom growth factor
             * @param alloc custom Alloc_type instance
             */
            template< class InputIt >
            constexpr explicit cow_darray(const Size_type _capacity, InputIt first, InputIt last,
                                      const float growth_factor=DEFAULT_GROWTH_FACTOR, const Alloc_type& alloc = Alloc_type())
            : store_ref(std::make_shared<storage_t>(_capacity, first, last, growth_factor, alloc)), sync_atomic(false)
            { }


            ~cow_darray() noexcept { }

            // cow_vector features

            /**
             * Returns this instances' recursive write mutex, allowing user to
             * implement more complex mutable write operations.
             * <p>
             * See example in jau::cow_darray::set_store()
             * </p>
             *
             * @see jau::cow_darray::get_write_mutex()
             * @see jau::cow_darray::copy_store()
             * @see jau::cow_darray::set_store()
             */
            constexpr std::recursive_mutex & get_write_mutex() noexcept { return mtx_write; }

            /**
             * Returns a new shared_ptr copy of the underlying store,
             * i.e. using a new copy-constructed vectore.
             * <p>
             * See example in jau::cow_darray::set_store()
             * </p>
             * <p>
             * This special operation uses a mutex lock and is blocking this instances' write operations only.
             * </p>
             * @see jau::cow_darray::get_write_mutex()
             * @see jau::cow_darray::copy_store()
             * @see jau::cow_darray::set_store()
             */
            storage_ref_t copy_store() {
                const std::lock_guard<std::recursive_mutex> lock(mtx_write);
                return std::make_shared<storage_t>( *store_ref );
            }

            /**
             * Special case facility allowing the user to replace the current store
             * with the given value, potentially acquired via jau::cow_darray::copy_store()
             * and mutated while holding the jau::cow_darray::get_write_mutex() lock.
             * <p>
             * This is a move operation, i.e. the given new_store_ref is invalid on the caller side
             * after this operation. <br>
             * User shall pass the store via std::move()
             * <pre>
             *     cow_darray<std::shared_ptr<Thing>> list;
             *     ...
             *     {
             *         const std::lock_guard<std::recursive_mutex> lock(list.get_write_mutex());
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
             * @see jau::cow_darray::get_write_mutex()
             * @see jau::cow_darray::copy_store()
             * @see jau::cow_darray::set_store()
             */
            constexpr void set_store(storage_ref_t && new_store_ref) noexcept {
                const std::lock_guard<std::recursive_mutex> lock(mtx_write);
                sc_atomic_critical sync(sync_atomic);
                store_ref = std::move( new_store_ref );
            }

            /**
             * Returns the current snapshot of the underlying shared std::vector<T> reference.
             * <p>
             * Note that this snapshot will be outdated by the next (concurrent) write operation.<br>
             * The returned referenced vector is still valid and not mutated,
             * but does not represent the current content of this cow_darray instance.
             * </p>
             * <p>
             * This read operation is <i>lock-free</i>.
             * </p>
             * @see jau::for_each_cow
             */
            constexpr storage_ref_t get_snapshot() const noexcept {
                sc_atomic_critical sync( const_cast<cow_darray *>(this)->sync_atomic );
                return store_ref;
            }

            // const_iterator, non mutable, read-only

            constexpr const_iterator begin() const noexcept {
                return const_iterator(get_snapshot(), store_ref->cbegin());
            }

            constexpr const_iterator cbegin() const noexcept {
                return const_iterator(get_snapshot(), store_ref->cbegin());
            }

            constexpr const_iterator end() const noexcept {
                return const_iterator(get_snapshot(), store_ref->cend());
            }

            constexpr const_iterator cend() const noexcept {
                return const_iterator(get_snapshot(), store_ref->cend());
            }

            // iterator, mutable, read-write

            constexpr iterator begin() noexcept {
                return iterator(*this, [](storage_ref_t& new_store) -> iterator { return new_store->begin(); } );
            }

            constexpr iterator end() noexcept {
                return iterator(*this, [](storage_ref_t& new_store) -> iterator { return new_store->end(); } );
            }

            // read access

            const Alloc_type& get_allocator_ref() const noexcept {
                sc_atomic_critical sync( const_cast<cow_darray *>(this)->sync_atomic );
                return store_ref->get_allocator_ref();
            }

            Alloc_type get_allocator() const noexcept {
                sc_atomic_critical sync( const_cast<cow_darray *>(this)->sync_atomic );
                return store_ref->get_allocator();
            }

            constexpr Size_type capacity() const noexcept {
                sc_atomic_critical sync( const_cast<cow_darray *>(this)->sync_atomic );
                return store_ref->capacity();
            }

            /**
             * Like std::vector::empty().
             * <p>
             * This read operation is <i>lock-free</i>.
             * </p>
             */
            constexpr bool empty() const noexcept {
                sc_atomic_critical sync( const_cast<cow_darray *>(this)->sync_atomic );
                return store_ref->empty();
            }

            /**
             * Like std::vector::size().
             * <p>
             * This read operation is <i>lock-free</i>.
             * </p>
             */
            constexpr Size_type size() const noexcept {
                sc_atomic_critical sync( const_cast<cow_darray *>(this)->sync_atomic );
                return store_ref->size();
            }

            // write access

            /**
             * Like std::vector::reserve(), increases this instance's capacity to <code>new_capacity</code>.
             * <p>
             * Only creates a new storage and invalidates iterators if <code>new_capacity</code>
             * is greater than the current jau::darray::capacity().
             * </p>
             * <p>
             * This write operation uses a mutex lock and is blocking this instances' write operations only.
             * </p>
             */
            void reserve(Size_type new_capacity) {
                const std::lock_guard<std::recursive_mutex> lock(mtx_write);
                storage_ref_t old_store_ref = store_ref;
                if( new_capacity > old_store_ref->capacity() ) {
                    storage_ref_t new_store_ref = std::make_shared<storage_t>( *old_store_ref, new_capacity,
                                                                               old_store_ref->growth_factor(),
                                                                               old_store_ref->get_allocator_ref() );
                    sc_atomic_critical sync( sync_atomic );
                    store_ref = std::move(new_store_ref);
                }
            }

            /**
             * Like std::vector::operator=(&), assignment, but copying from the underling jau::darray
             * <p>
             * This write operation uses a mutex lock and is blocking this instances' write operations only.
             * </p>
             */
            cow_darray& operator=(const storage_t& x) {
                const std::lock_guard<std::recursive_mutex> lock(mtx_write);
                storage_ref_t new_store_ref = std::make_shared<storage_t>( x );
                sc_atomic_critical sync(sync_atomic);
                store_ref = std::move(new_store_ref);
                return *this;
            }

            /**
             * Like std::vector::operator=(&&), move, but taking the underling jau::darray
             * <p>
             * This write operation uses a mutex lock and is blocking this instances' write operations only.
             * </p>
             */
            cow_darray& operator=(storage_t&& x) {
                const std::lock_guard<std::recursive_mutex> lock(mtx_write);
                storage_ref_t new_store_ref = std::make_shared<storage_t>( std::move(x) );
                sc_atomic_critical sync(sync_atomic);
                store_ref = std::move(new_store_ref);
                return *this;
            }

            /**
             * Like std::vector::operator=(&), assignment
             * <p>
             * This write operation uses a mutex lock and is blocking this instances' write operations only.
             * </p>
             */
            cow_darray& operator=(const cow_darray& x) {
                const std::lock_guard<std::recursive_mutex> lock(mtx_write);
                storage_ref_t x_store_ref;
                {
                    sc_atomic_critical sync_x( const_cast<cow_darray *>(&x)->sync_atomic );
                    x_store_ref = x.store_ref;
                }
                storage_ref_t new_store_ref = std::make_shared<storage_t>( *x_store_ref );
                {
                    sc_atomic_critical sync(sync_atomic);
                    store_ref = std::move(new_store_ref);
                }
                return *this;
            }

            /**
             * Like std::vector::operator=(&&), move.
             * <p>
             * This write operation uses a mutex lock and is blocking both cow_vector instance's write operations.
             * </p>
             */
            cow_darray& operator=(cow_darray&& x) {
                std::unique_lock<std::recursive_mutex> lock(mtx_write, std::defer_lock); // utilize std::lock(a, b), allowing mixed order waiting on either object
                std::unique_lock<std::recursive_mutex> lock_x(x.mtx_write, std::defer_lock); // otherwise RAII-style relinquish via destructor
                std::lock(lock, lock_x);
                {
                    sc_atomic_critical sync_x( x.sync_atomic );
                    sc_atomic_critical sync(sync_atomic);
                    // swap store_ref
                    store_ref = std::move(x.store_ref);
                    x.store_ref = nullptr;
                }
                return *this;
            }

            /**
             * Like std::vector::clear(), but ending with zero capacity.
             * <p>
             * This write operation uses a mutex lock and is blocking this instances' write operations.
             * </p>
             */
            void clear() noexcept {
                const std::lock_guard<std::recursive_mutex> lock(mtx_write);
                storage_ref_t new_store_ref = std::make_shared<storage_t>();
                {
                    sc_atomic_critical sync(sync_atomic);
                    store_ref = std::move(new_store_ref);
                }
            }

            /**
             * Like std::vector::swap().
             * <p>
             * This write operation uses a mutex lock and is blocking both cow_darray instance's write operations.
             * </p>
             */
            void swap(cow_darray& x) noexcept {
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
            void pop_back() noexcept {
                const std::lock_guard<std::recursive_mutex> lock(mtx_write);
                storage_ref_t old_store_ref = store_ref;
                if( !old_store_ref->empty() ) {
                    storage_ref_t new_store_ref = std::make_shared<storage_t>( old_store_ref->capacity(),
                                                                               old_store_ref->cbegin(),
                                                                               old_store_ref->cend()-1,
                                                                               old_store_ref->growth_factor(),
                                                                               old_store_ref->get_allocator_ref() );
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
            void push_back(const Value_type& x) {
                const std::lock_guard<std::recursive_mutex> lock(mtx_write);
                storage_ref_t old_store_ref = store_ref;
                if( old_store_ref->capacity_reached() ) {
                    // grow and swap all refs
                    storage_ref_t new_store_ref = std::make_shared<storage_t>( *old_store_ref, old_store_ref->grow_capacity(),
                                                                               old_store_ref->growth_factor(),
                                                                               old_store_ref->get_allocator_ref() );
                    new_store_ref->push_back(x);
                    {
                        sc_atomic_critical sync(sync_atomic);
                        store_ref = std::move(new_store_ref);
                    }
                } else {
                    // just append ..
                    store_ref->push_back(x);
                }
            }

            /**
             * Like std::vector::push_back(), move
             * <p>
             * This write operation uses a mutex lock and is blocking this instances' write operations only.
             * </p>
             */
            void push_back(Value_type&& x) {
                const std::lock_guard<std::recursive_mutex> lock(mtx_write);
                storage_ref_t old_store_ref = store_ref;
                if( old_store_ref->capacity_reached() ) {
                    // grow and swap all refs
                    storage_ref_t new_store_ref = std::make_shared<storage_t>( *old_store_ref, old_store_ref->grow_capacity(),
                                                                               old_store_ref->growth_factor(),
                                                                               old_store_ref->get_allocator_ref() );
                    new_store_ref->push_back( std::move(x) );
                    {
                        sc_atomic_critical sync(sync_atomic);
                        store_ref = std::move(new_store_ref);
                    }
                } else {
                    // just append ..
                    store_ref->push_back( std::move(x) );
                }
            }

            /**
             * Like std::vector::push_back(), but appends the whole Value_type range [first, last).
             * <p>
             * This write operation uses a mutex lock and is blocking this instances' write operations only.
             * </p>
             * @tparam InputIt foreign input-iterator to range of Value_type [first, last)
             * @param first first foreign input-iterator to range of Value_type [first, last)
             * @param last last foreign input-iterator to range of Value_type [first, last)
             */
            template< class InputIt >
            constexpr void push_back( InputIt first, InputIt last ) {
                const std::lock_guard<std::recursive_mutex> lock(mtx_write);
                storage_ref_t old_store_ref = store_ref;
                const Size_type new_size_ = old_store_ref->size() + Size_type(last - first);

                if( new_size_ > old_store_ref->capacity() ) {
                    // grow and swap all refs
                    storage_ref_t new_store_ref = std::make_shared<storage_t>( *old_store_ref, new_size_,
                                                                               old_store_ref->growth_factor(),
                                                                               old_store_ref->get_allocator_ref() );
                    store_ref->push_back( first, last );
                    {
                        sc_atomic_critical sync(sync_atomic);
                        store_ref = std::move(new_store_ref);
                    }
                } else {
                    // just append ..
                    store_ref->push_back( first, last );
                }
            }

            /**
             * Like std::vector::push_back(), but appends the whole Value_type range [first, last).
             * <p>
             * This write operation uses a mutex lock and is blocking this instances' write operations only.
             * </p>
             * @param first first const_iterator to range of Value_type [first, last)
             * @param last last const_iterator to range of Value_type [first, last)
             */
            constexpr void push_back( const_iterator first, const_iterator last ) {
                const std::lock_guard<std::recursive_mutex> lock(mtx_write);
                storage_ref_t old_store_ref = store_ref;
                const Size_type new_size_ = old_store_ref->size() + Size_type(last - first);

                if( new_size_ > old_store_ref->capacity() ) {
                    // grow and swap all refs
                    storage_ref_t new_store_ref = std::make_shared<storage_t>( *old_store_ref, new_size_,
                                                                               old_store_ref->growth_factor(),
                                                                               old_store_ref->get_allocator_ref() );
                    store_ref->push_back( first.underling(), last.underling() );
                    {
                        sc_atomic_critical sync(sync_atomic);
                        store_ref = std::move(new_store_ref);
                    }
                } else {
                    // just append ..
                    store_ref->push_back( first.underling(), last.underling() );
                }
            }

            /**
             * Generic Value_type equal comparator to be user defined for e.g. jau::cow_darray::push_back_unique().
             * @param a one element of the equality test.
             * @param b the other element of the equality test.
             * @return true if both are equal
             */
            typedef bool(*equal_comparator)(const Value_type& a, const Value_type& b);

            /**
             * Like std::vector::push_back(), but only if the newly added element does not yet exist.
             * <p>
             * This write operation uses a mutex lock and is blocking this instances' write operations only.
             * </p>
             * <p>
             * Examples
             * <pre>
             *     static jau::cow_darray<Thing>::equal_comparator thingEqComparator =
             *                  [](const Thing &a, const Thing &b) -> bool { return a == b; };
             *     ...
             *     jau::cow_darray<Thing> list;
             *
             *     bool added = list.push_back_unique(new_element, thingEqComparator);
             *     ...
             *     cow_darray<std::shared_ptr<Thing>> listOfRefs;
             *     bool added = listOfRefs.push_back_unique(new_element,
             *                    [](const std::shared_ptr<Thing> &a, const std::shared_ptr<Thing> &b) -> bool { return *a == *b; });
             * </pre>
             * </p>
             * @param x the value to be added at the tail, if not existing yet.
             * @param comparator the equal comparator to return true if both given elements are equal
             * @return true if the element has been uniquely added, otherwise false
             */
            bool push_back_unique(const Value_type& x, equal_comparator comparator) {
                const std::lock_guard<std::recursive_mutex> lock(mtx_write);
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
             *     cow_darray<Thing> list;
             *     int count = list.erase_matching(element, true,
             *                    [](const Thing &a, const Thing &b) -> bool { return a == b; });
             *     ...
             *     static jau::cow_darray<Thing>::equal_comparator thingRefEqComparator =
             *                  [](const std::shared_ptr<Thing> &a, const std::shared_ptr<Thing> &b) -> bool { return *a == *b; };
             *     ...
             *     cow_darray<std::shared_ptr<Thing>> listOfRefs;
             *     int count = listOfRefs.erase_matching(element, false, thingRefEqComparator);
             * </pre>
             * </p>
             * @param x the value to be added at the tail, if not existing yet.
             * @param all_matching if true, erase all matching elements, otherwise only the first matching element.
             * @param comparator the equal comparator to return true if both given elements are equal
             * @return number of erased elements
             */
            int erase_matching(const Value_type& x, const bool all_matching, equal_comparator comparator) {
                int count = 0;
                const std::lock_guard<std::recursive_mutex> lock(mtx_write);
                storage_ref_t new_store_ref = std::make_shared<storage_t>( *store_ref );
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

            /**
             * Thread safe Value_type copy assignment to Value_type at given position with bounds checking.
             * <p>
             * This write operation uses a mutex lock and is blocking this instances' write operations only.
             * </p>
             * <p>
             * To mutate multiple elements, use the more efficient jau::cow_rw_iterator via begin() and end().
             * </p>
             * @param i the position within this store
             * @param x the value to be assigned to the object at the given position
             */
            void put(size_t i, const Value_type& x) {
                const std::lock_guard<std::recursive_mutex> lock(mtx_write);
                storage_ref_t new_store_ref = std::make_shared<storage_t>( *store_ref );
                new_store_ref->at(i) = x;
                {
                    sc_atomic_critical sync(sync_atomic);
                    store_ref = std::move(new_store_ref);
                }
            }

            /**
             * Thread safe Value_type move assignment to Value_type at given position with bounds checking.
             * <p>
             * This write operation uses a mutex lock and is blocking this instances' write operations only.
             * </p>
             * <p>
             * To mutate multiple elements, use the more efficient jau::cow_rw_iterator via begin() and end().
             * </p>
             * @param i the position within this store
             * @param x the value to be assigned to the object at the given position
             */
            void put(size_t i, Value_type&& x) {
                const std::lock_guard<std::recursive_mutex> lock(mtx_write);
                storage_ref_t new_store_ref = std::make_shared<storage_t>( *store_ref );
                new_store_ref->at(i) = std::move(x);
                {
                    sc_atomic_critical sync(sync_atomic);
                    store_ref = std::move(new_store_ref);
                }
            }
    };

} /* namespace jau */

#endif /* JAU_COW_DARRAY_HPP_ */
