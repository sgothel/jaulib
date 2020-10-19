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

#ifndef JAU_COW_ARRAY_HPP_
#define JAU_COW_ARRAY_HPP_

#include <cstring>
#include <string>
#include <cstdint>
#include <atomic>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <array>
#include <algorithm>

#include <jau/debug.hpp>
#include <jau/basic_types.hpp>
#include <jau/ordered_atomic.hpp>


namespace jau {

    /**
     * Implementation of a Copy-On-Write (CoW) array,
     * exposing <i>lock-free</i> read operations using SC-DRF atomic synchronization.
     * <p>
     * The array's store is owned using a reference to the data structure,
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
     * Naturally iterators are not supported directly,
     * otherwise we would need to copy the store to iterate through.<br>
     * However, one can utilize iteration using the shared snapshot
     * of the underlying array store via jau::cow_array::get_snapshot() for read operations.
     * </p>
     * <p>
     * Custom mutable write operations are also supported via
     * jau::cow_array::get_write_mutex(), jau::cow_array::copy_store() and jau::cow_array::set_store().<br>
     * See example in jau::cow_array::set_store()
     * </p>
     * See also:
     * <pre>
     * - Sequentially Consistent (SC) ordering or SC-DRF (data race free) <https://en.cppreference.com/w/cpp/atomic/memory_order#Sequentially-consistent_ordering>
     * - std::memory_order <https://en.cppreference.com/w/cpp/atomic/memory_order>
     * </pre>
     */
    template <typename Value_type, std::size_t Array_size> class cow_array {
        private:
            typedef std::array<Value_type, Array_size> array_t;
            typedef std::shared_ptr<array_t> array_ref;

            static void fill_impl(array_t & a, const Value_type& x) {
                for(size_t i=0; i < Array_size; i++) {
                    a[i] = x;
                }
            }

            array_ref store_ref;
            sc_atomic_bool sync_atomic;
            std::recursive_mutex mtx_write;

        public:
            // ctor

            cow_array() noexcept
            : store_ref(new array_t()), sync_atomic(false) {}

            explicit cow_array(const Value_type& x)
            : store_ref(new array_t()), sync_atomic(false) {
                fill_impl(*store_ref, x);
            }

            cow_array(const cow_array& x)
            : sync_atomic(false) {
                array_ref x_store_ref;
                {
                    sc_atomic_critical sync_x( const_cast<cow_array *>(&x)->sync_atomic );
                    x_store_ref = x.store_ref;
                }
                store_ref = array_ref( new array_t(*x_store_ref) );
            }

            explicit cow_array(const array_t& x)
            : store_ref(new array_t(x)), sync_atomic(false) { }

            cow_array(cow_array && x) noexcept {
                // swap store_ref
                store_ref = x.store_ref;
                x.store_ref = nullptr;
                // not really necessary
                sync_atomic = x.sync_atomic;
            }

            ~cow_array() noexcept { }

            /**
             * Returns this instances' recursive write mutex, allowing user to
             * implement more complex mutable write operations.
             * <p>
             * See example in jau::cow_array::set_store()
             * </p>
             *
             * @see jau::cow_array::get_write_mutex()
             * @see jau::cow_array::copy_store()
             * @see jau::cow_array::set_store()
             */
            std::recursive_mutex & get_write_mutex() { return mtx_write; }

            /**
             * Returns a new shared_ptr copy of the underlying store,
             * i.e. using a new copy-constructed arraye.
             * <p>
             * See example in jau::cow_array::set_store()
             * </p>
             * <p>
             * This special operation uses a mutex lock and is blocking this instances' write operations only.
             * </p>
             * @see jau::cow_array::get_write_mutex()
             * @see jau::cow_array::copy_store()
             * @see jau::cow_array::set_store()
             */
            std::shared_ptr<std::array<Value_type, Array_size>> copy_store() noexcept {
                const std::lock_guard<std::recursive_mutex> lock(mtx_write);
                return array_ref( new array_t(*store_ref) );
            }

            /**
             * Special case facility allowing the user to replace the current store
             * with the given value, potentially acquired via jau::cow_array::copy_store()
             * and mutated while holding the jau::cow_array::get_write_mutex() lock.
             * <p>
             * This is a move operation, i.e. the given new_store_ref is invalid on the caller side
             * after this operation. <br>
             * User shall pass the store via std::move()
             * <pre>
             *     cow_array<std::shared_ptr<Thing>> list;
             *     ...
             *     {
             *         const std::lock_guard<std::recursive_mutex> lock(list.get_write_mutex());
             *         std::shared_ptr<std::array<std::shared_ptr<Thing>>> snapshot = list.copy_store();
             *         ...
             *         some fancy mutation
             *         ...
             *         list.set_store(std::move(snapshot));
             *     }
             * </pre>
             * </p>
             * @param new_store_ref the user store to be moved here, replacing the current store.
             *
             * @see jau::cow_array::get_write_mutex()
             * @see jau::cow_array::copy_store()
             * @see jau::cow_array::set_store()
             */
            void set_store(std::shared_ptr<std::array<Value_type, Array_size>> && new_store_ref) noexcept {
                const std::lock_guard<std::recursive_mutex> lock(mtx_write);
                sc_atomic_critical sync(sync_atomic);
                store_ref = new_store_ref;
            }

            // read access

            /**
             * Returns the current snapshot of the underlying shared std::array<T> reference.
             * <p>
             * Note that this snapshot will be outdated by the next (concurrent) write operation.<br>
             * The returned referenced array is still valid and not mutated,
             * but does not represent the current content of this cow_array instance.
             * </p>
             * <p>
             * This read operation is <i>lock-free</i>.
             * </p>
             * @see jau::for_each_cow
             */
            std::shared_ptr<std::array<Value_type, Array_size>> get_snapshot() const noexcept {
                sc_atomic_critical sync( const_cast<cow_array *>(this)->sync_atomic );
                return store_ref;
            }

            /**
             * Like std::array<T>::data() const noexcept.
             * <p>
             * This read operation is <i>lock-free</i>.
             * </p>
             */
            const Value_type* data() const noexcept {
                sc_atomic_critical sync( const_cast<cow_array *>(this)->sync_atomic );
                return store_ref->data();
            }

            /**
             * Like std::array<T>::empty().
             * <p>
             * This read operation is <i>lock-free</i>.
             * </p>
             */
            bool empty() const noexcept {
                sc_atomic_critical sync( const_cast<cow_array *>(this)->sync_atomic );
                return store_ref->empty();
            }

            /**
             * Like std::array<T>::size().
             * <p>
             * This read operation is <i>lock-free</i>.
             * </p>
             */
            size_t size() const noexcept {
                sc_atomic_critical sync( const_cast<cow_array *>(this)->sync_atomic );
                return store_ref->size();
            }

            /**
             * Like std::array<T>::operator[](size_t).
             * <p>
             * This read operation is <i>lock-free</i>.
             * </p>
             */
            const Value_type & operator[](size_t i) const noexcept {
                sc_atomic_critical sync( const_cast<cow_array *>(this)->sync_atomic );
                return (*store_ref)[i];
            }

            /**
             * Like std::array<T>::operator[](size_t).
             * <p>
             * This read operation is <i>lock-free</i>.
             * </p>
             * <p>
             * Mutation of the resulting Value_type
             * is not synchronized via this cow_vector instance.
             * </p>
             * @see put()
             */
            Value_type & operator[](size_t i) noexcept {
                sc_atomic_critical sync(sync_atomic);
                return (*store_ref)[i];
            }

            /**
             * Like std::array<T>::at(size_t).
             * <p>
             * This read operation is <i>lock-free</i>.
             * </p>
             */
            const Value_type & at(size_t i) const {
                sc_atomic_critical sync( const_cast<cow_array *>(this)->sync_atomic );
                return store_ref->at(i);
            }

            /**
             * Like std::array<T>::at(size_t).
             * <p>
             * This read operation is <i>lock-free</i>.
             * </p>
             * <p>
             * Mutation of the resulting Value_type
             * is not synchronized via this cow_vector instance.
             * </p>
             * @see put()
             */
            Value_type & at(size_t i) {
                sc_atomic_critical sync(sync_atomic);
                return store_ref->at(i);
            }

            // write access

            /**
             * Like implicit std::array<T>::operator=(&), assignment
             * <p>
             * This write operation uses a mutex lock and is blocking this instances' write operations only.
             * </p>
             */
            cow_array& operator=(const cow_array& x) {
                const std::lock_guard<std::recursive_mutex> lock(mtx_write);
                array_ref x_store_ref;
                {
                    sc_atomic_critical sync_x( const_cast<cow_array *>(&x)->sync_atomic );
                    x_store_ref = x.store_ref;
                }
                array_ref new_store_ref = array_ref( new array_t(*x_store_ref) );
                {
                    sc_atomic_critical sync(sync_atomic);
                    store_ref = new_store_ref;
                }
                return *this;
            }

            /**
             * Like implicit std::array<T>::operator=(&&), move.
             * <p>
             * This write operation uses a mutex lock and is blocking this instances' write operations only.
             * </p>
             */
            cow_array& operator=(const cow_array&& x) {
                const std::lock_guard<std::recursive_mutex> lock(mtx_write);
                sc_atomic_critical sync(sync_atomic);
                // swap store_ref
                store_ref = x.store_ref;
                x.store_ref = nullptr;
                return *this;
            }

            /**
             * Like std::array<T>::fill()
             * <p>
             * This write operation uses a mutex lock and is blocking this instances' write operations.
             * </p>
             */
            void fill(const Value_type& x) {
                const std::lock_guard<std::recursive_mutex> lock(mtx_write);
                array_ref new_store_ref = array_ref( new array_t() );
                fill_impl(*new_store_ref, x);
                {
                    sc_atomic_critical sync(sync_atomic);
                    store_ref = new_store_ref;
                }
            }

            /**
             * Like std::array<T>::swap().
             * <p>
             * This write operation uses a mutex lock and is blocking both cow_array instance's write operations.
             * </p>
             */
            void swap(cow_array& x) noexcept {
                std::unique_lock<std::recursive_mutex> lock(mtx_write, std::defer_lock); // utilize std::lock(a, b), allowing mixed order waiting on either object
                std::unique_lock<std::recursive_mutex> lock_x(x.mtx_write, std::defer_lock); // otherwise RAII-style relinquish via destructor
                std::lock(lock, lock_x);
                {
                    sc_atomic_critical sync_x( const_cast<cow_array *>(&x)->sync_atomic );
                    sc_atomic_critical sync(sync_atomic);
                    array_ref x_store_ref = x.store_ref;
                    x.store_ref = store_ref;
                    store_ref = x_store_ref;
                }
            }

            /**
             * Thread safe Value_type copy assignment to Value_type at given position with bounds checking.
             * <p>
             * This write operation uses a mutex lock and is blocking this instances' write operations only.
             * </p>
             * @param x the value to be added at the tail.
             */
            void put(size_t i, const Value_type& x) {
                const std::lock_guard<std::recursive_mutex> lock(mtx_write);
                array_ref new_store_ref = array_ref( new array_t(*store_ref) );
                new_store_ref->at(i) = x;
                {
                    sc_atomic_critical sync(sync_atomic);
                    store_ref = new_store_ref;
                }
            }

            /**
             * Thread safe Value_type move assignment to Value_type at given position with bounds checking.
             * <p>
             * This write operation uses a mutex lock and is blocking this instances' write operations only.
             * </p>
             */
            void put(size_t i, Value_type&& x) {
                const std::lock_guard<std::recursive_mutex> lock(mtx_write);
                array_ref new_store_ref = array_ref( new array_t(*store_ref) );
                new_store_ref->at(i) = std::move(x);
                {
                    sc_atomic_critical sync(sync_atomic);
                    store_ref = new_store_ref;
                }
            }
    };

    /**
     * Custom for_each template on a jau::cow_array snapshot
     * using jau::cow_array::get_snapshot().
     * <p>
     * jau::for_each_idx() would also be usable, however,
     * iterating through the snapshot preserves consistency over the
     * fetched collection at the time of invocation.
     * </p>
     * <p>
     * Method performs UnaryFunction on all snapshot elements [0..n-1].
     * </p>
     */
    template<typename Value_type, std::size_t Array_size, class UnaryFunction>
    constexpr UnaryFunction for_each_cow(cow_array<Value_type, Array_size> &cow, UnaryFunction f)
    {
        std::shared_ptr<std::array<Value_type, Array_size>> snapshot = cow.get_snapshot();
        const size_t size = snapshot->size();
        for (size_t i = 0; i < size; i++) {
            f( (*snapshot)[i] );
        }
        return f; // implicit move since C++11
    }

} /* namespace jau */

#endif /* JAU_COW_VECTOR_HPP_ */
