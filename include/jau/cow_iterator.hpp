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

#ifndef JAU_COW_ITERATOR_HPP_
#define JAU_COW_ITERATOR_HPP_

#include <cstddef>
#include <limits>
#include <mutex>
#include <utility>

#include <type_traits>
#include <iostream>

#include <jau/cpp_lang_util.hpp>
#include <jau/debug.hpp>
#include <jau/basic_types.hpp>

namespace jau {

    // forward declaration for friendship with cow_rw_iterator
    template <typename Storage_type, typename Storage_ref_type, typename CoW_container>
    class cow_ro_iterator;

    template <typename Storage_type, typename Storage_ref_type, typename CoW_container>
    class cow_rw_iterator;

    /****************************************************************************************
     ****************************************************************************************/

    /**
     * Implementation of a Copy-On-Write (CoW) read-write iterator over mutable value_type storage.<br>
     * Instance holds a copy of the parents' CoW storage and locks its write mutex until
     * write_back() or destruction.
     * <p>
     * Implementation complies with Type Traits iterator_category 'random_access_iterator_tag'
     * </p>
     * <p>
     * This iterator wraps the native iterator of type 'iterator_type'
     * and manages the CoW related resource lifecycle.
     * </p>
     * <p>
     * After completing all mutable operations but before this iterator's destruction,
     * the user might want to write back this iterators' storage to its parents' CoW
     * using write_back()
     * </p>
     * <p>
     * Due to the costly nature of mutable CoW resource management,
     * consider using jau::cow_ro_iterator if elements won't get mutated
     * or any changes can be discarded.
     * </p>
     * <p>
     * To allow data-race free operations on this iterator's data copy from a potentially mutated CoW,
     * only one begin iterator should be retrieved from CoW and all further operations shall use
     * jau::cow_rw_iterator::size(), jau::cow_rw_iterator::begin() and jau::cow_rw_iterator::end().
     * </p>
     * @see jau::cow_rw_iterator::write_back()
     * @see jau::for_each_fidelity
     * @see jau::cow_darray
     */
    template <typename Storage_type, typename Storage_ref_type, typename CoW_container>
    class cow_rw_iterator {
        friend cow_ro_iterator<Storage_type, Storage_ref_type, CoW_container>;
        template<typename, typename, typename, bool, bool, bool> friend class cow_darray;
        template<typename, typename> friend class cow_vector;

        public:
            typedef Storage_type                                storage_t;
            typedef Storage_ref_type                            storage_ref_t;
            typedef CoW_container                               cow_container_t;

            /** Actual iterator type of the contained native iterator, probably a simple pointer. */
            typedef typename storage_t::iterator                iterator_type;

        private:
            typedef std::iterator_traits<iterator_type>         sub_traits_t;

            cow_container_t&                        cow_parent_;
            std::unique_lock<std::recursive_mutex>  lock_;           // can move and swap
            storage_ref_t                           store_ref_;
            iterator_type                           iterator_;

            constexpr explicit cow_rw_iterator(cow_container_t& cow_parent, const storage_ref_t& store, iterator_type iter) noexcept
            : cow_parent_(cow_parent), lock_(cow_parent_.get_write_mutex()), store_ref_(store),
              iterator_(iter) { }

            constexpr explicit cow_rw_iterator(cow_container_t& cow_parent)
            : cow_parent_(cow_parent), lock_(cow_parent_.get_write_mutex()),
              store_ref_(cow_parent.copy_store()), iterator_(store_ref_->begin()) { }

        public:
            typedef typename sub_traits_t::iterator_category    iterator_category;  // random_access_iterator_tag

            typedef typename storage_t::size_type               size_type;          // using our template overload Size_type
            typedef typename storage_t::difference_type         difference_type;    // derived from our Size_type
            // typedef typename storage_t::value_type           value_type;         // OK
            // typedef typename storage_t::reference            reference;          //
            // typedef typename storage_t::pointer              pointer;            //
            typedef typename sub_traits_t::value_type           value_type;         // OK
            typedef typename sub_traits_t::reference            reference;          // 'value_type &'
            typedef typename sub_traits_t::pointer              pointer;            // 'value_type *'

#if __cplusplus > 201703L && __cpp_lib_concepts
            using iterator_concept = std::__detail::__iter_concept<_Iterator>;
#endif

        public:

            /**
             * Replace the parent's current store with this iterators' instance,
             * unlock the CoW parents' write lock and discard all storage references.
             * <p>
             * After calling write_back(), this iterator is invalidated and no more operational.
             * </p>
             * <p>
             * It is the user's responsibility to issue call this method
             * to update the CoW parents' storage.
             * </p>
             * <p>
             * It is not feasible nor effective to automatically earmark a dirty state
             * on mutable operations.<br>
             * This is due to the ambiguous semantics of like <code>operator*()</code>.<br>
             * Also usage of multiple iterators to one CoW instance during a mutable operation
             * complicates such an automated task, especially as we wish to only realize one
             * storage replacement at the end.<br>
             * Lastly, the user probably wants to issue the CoW storage sync
             * in a programmatic deterministic fashion at the end.
             * </p>
             * @see jau::cow_darray::set_store()
             */
            void write_back() noexcept {
                if( nullptr != store_ref_ ) {
                    cow_parent_.set_store(std::move(store_ref_));

                    lock_ = std::unique_lock<std::recursive_mutex>(); // force-dtor-unlock-null
                    store_ref_ = nullptr;
                    iterator_ = iterator_type();
                }
            }

            /**
             * C++ named requirements: LegacyIterator: CopyConstructible
             */
            constexpr cow_rw_iterator(const cow_rw_iterator& o) noexcept
            : cow_parent_(o.cow_parent_), lock_(cow_parent_.get_write_mutex()),
              store_ref_(o.store_ref_), iterator_(o.iterator_) { }

            /**
             * Assigns content of other mutable iterator to this one,
             * if they are not identical.
             * <p>
             * C++ named requirements: LegacyIterator: CopyAssignable
             * </p>
             * @param o the new identity value to be copied into this iterator
             * @return reference to this
             */
            constexpr cow_rw_iterator& operator=(const cow_rw_iterator& o) noexcept {
                if( this != &o ) {
                    cow_parent_ = o.cow_parent_;
                    lock_ = std::unique_lock<std::recursive_mutex>( cow_parent_.get_write_mutex() );
                    store_ref_ = o.store_ref_;
                    iterator_ = o.iterator_;
                }
                return *this;
            }


            /**
             * C++ named requirements: LegacyIterator: MoveConstructable
             */
            constexpr cow_rw_iterator(cow_rw_iterator && o) noexcept
            : cow_parent_( o.cow_parent_ ), lock_( std::move( o.lock_ ) ),
              store_ref_( std::move( o.store_ref_ ) ),
              iterator_( std::move(o.iterator_ ) ) {
                // Moved source has been disowned semantically and source's dtor will release resources!
            }

            /**
             * Assigns identity of given mutable iterator,
             * if they are not identical.
             * <p>
             * C++ named requirements: LegacyIterator: MoveAssignable
             * </p>
             * @param o the new identity to be taken
             * @return reference to this
             */
            constexpr cow_rw_iterator& operator=(cow_rw_iterator&& o) noexcept {
                if( this != &o ) {
                    cow_parent_ = o.cow_parent_;
                    lock_ = std::move(o.lock_);
                    store_ref_ = std::move(o.store_ref_);
                    iterator_ = std::move(o.iterator_);
                    // Moved source has been disowned semantically and source's dtor will release resources!
                }
                return *this;
            }

            /**
             * C++ named requirements: LegacyIterator: Swappable
             */
            void swap(cow_rw_iterator& o) noexcept {
                std::swap( cow_parent_, o.cow_parent_);
                std::swap( lock_, o.lock_);
                std::swap( store_ref_, o.store_ref_);
                std::swap( iterator_, o.iterator_);
            }

            /**
             * Returns a new const_iterator pointing to the current position.<br>
             * This is the only explicit conversion operation of mutable -> immutable iterator, see below.
             * <p>
             * Be aware that the resulting cow_ro_iterator points to transient storage
             * of this immutable iterator. In case write_back() won't be called
             * and this iterator destructs, the returned immutable iterator is invalidated.
             * </p>
             * @see size()
             * @see end()
             */
            constexpr cow_ro_iterator<Storage_type, Storage_ref_type, CoW_container> immutable() const noexcept
            { return cow_ro_iterator<Storage_type, Storage_ref_type, CoW_container>( store_ref_, iterator_ ); }

            /**
             * Returns a new iterator pointing to the first element, aka begin.
             * <p>
             * This is an addition API entry, allowing data-race free operations on
             * this iterator's data snapshot from a potentially mutated CoW.
             * </p>
             * @see size()
             * @see end()
             */
            constexpr cow_rw_iterator begin() const noexcept
            { return cow_rw_iterator( cow_parent_, store_ref_, store_ref_->begin()); }

            /**
             * Returns a new iterator pointing to the <i>element following the last element</i>, aka end.<br>
             * <p>
             * This is an addition API entry, allowing data-race free operations on
             * this iterator's data snapshot from a potentially mutated CoW.
             * </p>
             * @see size()
             * @see begin()
             */
            constexpr cow_rw_iterator end() const noexcept
            { return cow_rw_iterator( cow_parent_, store_ref_, store_ref_->end() ); }

            /**
             * Returns true if storage is empty().
             */
            constexpr bool empty() const noexcept { return store_ref_->empty(); }

            /**
             * Returns true if storage capacity has been reached and the next push_back()
             * will grow the storage and invalidates all iterators and references.
             */
            constexpr bool capacity_reached() const noexcept { return store_ref_->capacity_reached(); }

            /**
             * Return the size of the underlying value_type store.
             * <p>
             * This is an addition API entry, allowing data-race free arithmetic on
             * this iterator's data snapshot from a potentially mutated CoW.
             * </p>
             * @see begin()
             * @see end()
             */
            constexpr size_type size() const noexcept { return store_ref_->size(); }

            /**
             * Returns this instances' underlying shared storage by reference.
             */
            constexpr storage_t& storage() const noexcept {
                return *store_ref_;
            }

            /**
             * Returns the distance to_end() using zero as first index. A.k.a the remaining elements iterable.
             */
            constexpr difference_type dist_end() const noexcept { return store_ref_->end() - iterator_; }

            /**
             * Returns true, if this iterator points to end().
             */
            constexpr bool is_end() const noexcept { return iterator_ == store_ref_->end(); }

            /**
             * This iterator is set to the last element, end(). Returns *this;
             */
            constexpr cow_rw_iterator& to_end() noexcept
            { iterator_ = store_ref_->end(); return *this; }

            /**
             * Returns the distance to_begin() using zero as first index. A.k.a the index from start.
             */
            constexpr difference_type dist_begin() const noexcept { return iterator_ - store_ref_->begin(); }

            /**
             * Returns true, if this iterator points to begin().
             */
            constexpr bool is_begin() const noexcept { return iterator_ == store_ref_->begin(); }

            /**
             * This iterator is set to the first element, begin(). Returns *this;
             */
            constexpr cow_rw_iterator& to_begin() noexcept
            { iterator_ = store_ref_->begin(); return *this; }

            /**
             * Returns a copy of the underlying storage iterator.
             */
            constexpr iterator_type base() const noexcept { return iterator_; }

            // Multipass guarantee equality

            /**
             * Returns signum or three-way comparison value
             * <pre>
             *    0 if equal (both, store and iteratore),
             *   -1 if this->iterator_ < rhs_iter and
             *    1 if this->iterator_ > rhs_iter (otherwise)
             * </pre>
             * @param rhs_store right-hand side store
             * @param rhs_iter right-hand side iterator
             */
            constexpr int compare(const cow_rw_iterator& rhs) const noexcept {
                return store_ref_ == rhs.store_ref_ && iterator_ == rhs.iterator_ ? 0
                       : ( iterator_ < rhs.iterator_ ? -1 : 1);
            }

            constexpr bool operator==(const cow_rw_iterator& rhs) const noexcept
            { return compare(rhs) == 0; }

            constexpr bool operator!=(const cow_rw_iterator& rhs) const noexcept
            { return compare(rhs) != 0; }

            // Relation

            constexpr bool operator<=(const cow_rw_iterator& rhs) const noexcept
            { return compare(rhs) <= 0; }

            constexpr bool operator<(const cow_rw_iterator& rhs) const noexcept
            { return compare(rhs) < 0; }

            constexpr bool operator>=(const cow_rw_iterator& rhs) const noexcept
            { return compare(rhs) >= 0; }

            constexpr bool operator>(const cow_rw_iterator& rhs) const noexcept
            { return compare(rhs) > 0; }

            // Forward iterator requirements

            /**
             * Dereferencing iterator to value_type reference
             * @return immutable reference to value_type
             */
            constexpr const reference operator*() const noexcept {
                return *iterator_;
            }

            /**
             * Pointer to member access.
             * @return immutable pointer to value_type
             */
            constexpr const pointer operator->() const noexcept {
                return &(*iterator_); // just in case iterator_type is a class, trick via dereference
            }

            /**
             * Dereferencing iterator to value_type reference.
             * @return mutable reference to value_type
             */
            constexpr reference operator*() noexcept { return *iterator_; }

            /**
             * Pointer to member access.
             * @return mutable pointer to value_type
             */
            constexpr pointer operator->() noexcept {
                return &(*iterator_); // just in case iterator_type is a class, trick via dereference
            }

            /** Pre-increment; Well performing, return *this.  */
            constexpr cow_rw_iterator& operator++() noexcept {
                ++iterator_;
                return *this;
            }

            /** Post-increment; Try to avoid: Low performance due to returning copy-ctor. */
            constexpr cow_rw_iterator operator++(int) noexcept
            { return cow_rw_iterator(cow_parent_, store_ref_, iterator_++); }

            // Bidirectional iterator requirements

            /** Pre-decrement; Well performing, return *this.  */
            constexpr cow_rw_iterator& operator--() noexcept {
                --iterator_;
                return *this;
            }

            /** Post-decrement; Try to avoid: Low performance due to returning copy-ctor. */
            constexpr cow_rw_iterator operator--(int) noexcept
            { return cow_rw_iterator(cow_parent_, store_ref_, iterator_--); }

            // Random access iterator requirements

            /** Subscript of 'element_index', returning immutable Value_type reference. */
            constexpr const reference operator[](difference_type i) const noexcept
            { return iterator_[i]; }

            /**
             * Subscript of 'element_index', returning mutable Value_type reference.
             */
            constexpr reference operator[](difference_type i) noexcept {
                return iterator_[i];
            }

            /** Addition-assignment of 'element_count'; Well performing, return *this.  */
            constexpr cow_rw_iterator& operator+=(difference_type i) noexcept
            { iterator_ += i; return *this; }

            /** Binary 'iterator + element_count'; Try to avoid: Low performance due to returning copy-ctor. */
            constexpr cow_rw_iterator operator+(difference_type rhs) const noexcept
            { return cow_rw_iterator(cow_parent_, store_ref_, iterator_ + rhs); }

            /** Subtraction-assignment of 'element_count'; Well performing, return *this.  */
            constexpr cow_rw_iterator& operator-=(difference_type i) noexcept
            { iterator_ -= i; return *this; }

            /** Binary 'iterator - element_count'; Try to avoid: Low performance due to returning copy-ctor. */
            constexpr cow_rw_iterator operator-(difference_type rhs) const noexcept
            { return cow_rw_iterator(cow_parent_, store_ref_, iterator_ - rhs); }

            // Distance or element count, binary subtraction of two iterator.

            /** Binary 'iterator - iterator -> element_count'; Well performing, return element_count of type difference_type. */
            constexpr difference_type operator-(const cow_rw_iterator& rhs) const noexcept
            { return iterator_ - rhs.iterator_; }

            constexpr_func_cxx20 std::string toString() const noexcept {
                return jau::to_string(iterator_);
            }
#if 0
            constexpr_func_cxx20 operator std::string() const noexcept {
                return toString();
            }
#endif
            constexpr_func_cxx20 std::string get_info() const noexcept {
                return "cow_rw_iterator[this "+jau::aptrHexString(this)+", CoW "+jau::aptrHexString(&cow_parent_)+
                        ", store "+jau::aptrHexString(&store_ref_)+
                       ", "+jau::to_string(iterator_)+"]";
            }

            /**
             * Removes the last element and sets this iterator to end()
             */
            constexpr void pop_back() noexcept {
                store_ref_->pop_back();
                iterator_ = store_ref_->end();
            }

            /**
             * Erases the element at the current position.
             * <p>
             * This iterator is set to the element following the last removed element.
             * </p>
             */
            constexpr void erase () {
                iterator_ = store_ref_->erase(iterator_);
            }

            /**
             * Like std::vector::erase(), removes the elements in the range [current, current+count).
             * <p>
             * This iterator is set to the element following the last removed element.
             * </p>
             */
            constexpr void erase (size_type count) {
                iterator_ = store_ref_->erase(iterator_, iterator_+count);
            }

            /**
             * Inserts the element before the current position
             * and moves all elements from there to the right beforehand.
             * <p>
             * size will be increased by one.
             * </p>
             * <p>
             * This iterator is set to the inserted element.
             * </p>
             */
            constexpr void insert(const value_type& x) {
                iterator_ = store_ref_->insert(iterator_, x);
            }

            /**
             * Inserts the element before the current position (std::move operation)
             * and moves all elements from there to the right beforehand.
             * <p>
             * size will be increased by one.
             * </p>
             * <p>
             * This iterator is set to the inserted element.
             * </p>
             */
            constexpr void insert(value_type&& x) {
                iterator_ = store_ref_->insert(iterator_, std::move(x));
            }

            /**
             * Like std::vector::emplace(), construct a new element in place.
             * <p>
             * Constructs the element before the current position using placement new
             * and moves all elements from there to the right beforehand.
             * </p>
             * <p>
             * size will be increased by one.
             * </p>
             * <p>
             * This iterator is set to the inserted element.
             * </p>
             * @param args arguments to forward to the constructor of the element
             */
            template<typename... Args>
            constexpr void emplace(Args&&... args) {
                iterator_ = store_ref_->emplace(iterator_, std::forward<Args>(args)... );
            }

            /**
             * Like std::vector::insert(), inserting the value_type range [first, last).
             * <p>
             * This iterator is set to the first element inserted, or pos if first==last.
             * </p>
             * @tparam InputIt foreign input-iterator to range of value_type [first, last)
             * @param first first foreign input-iterator to range of value_type [first, last)
             * @param last last foreign input-iterator to range of value_type [first, last)
             */
            template< class InputIt >
            constexpr void insert( InputIt first, InputIt last ) {
                iterator_ = store_ref_->insert(iterator_, first, last);
            }

            /**
             * Like std::vector::push_back(), copy
             * <p>
             * This iterator is set to the end.
             * </p>
             * @param x the value to be added at the tail.
             */
            constexpr void push_back(const value_type& x) {
                store_ref_->push_back(x);
                iterator_ = store_ref_->end();
            }

            /**
             * Like std::vector::push_back(), move
             * <p>
             * This iterator is set to the end.
             * </p>
             * @param x the value to be added at the tail.
             */
            constexpr void push_back(value_type&& x) {
                store_ref_->push_back(std::move(x));
                iterator_ = store_ref_->end();
            }

            /**
             * Like std::vector::emplace_back(), construct a new element in place at the end().
             * <p>
             * Constructs the element at the end() using placement new.
             * </p>
             * <p>
             * size will be increased by one.
             * </p>
             * <p>
             * This iterator is set to the end.
             * </p>
             * @param args arguments to forward to the constructor of the element
             */
            template<typename... Args>
            constexpr reference emplace_back(Args&&... args) {
                reference res = store_ref_->emplace_back(std::forward<Args>(args)...);
                iterator_ = store_ref_->end();
                return res;
            }

            /**
             * Like std::vector::push_back(), but appends the value_type range [first, last).
             * <p>
             * This iterator is set to the end.
             * </p>
             * @tparam InputIt foreign input-iterator to range of value_type [first, last)
             * @param first first foreign input-iterator to range of value_type [first, last)
             * @param last last foreign input-iterator to range of value_type [first, last)
             */
            template< class InputIt >
            constexpr void push_back( InputIt first, InputIt last ) {
                store_ref_->push_back(first, last);
                iterator_ = store_ref_->end();
            }
    };

    /**
     * Implementation of a Copy-On-Write (CoW) read-onlu iterator over immutable value_type storage.<br>
     * Instance holds a shared storage snapshot of the parents' CoW storage until destruction.
     * <p>
     * Implementation complies with Type Traits iterator_category 'random_access_iterator_tag'
     * </p>
     * <p>
     * Implementation simply wraps the native iterator of type 'iterator_type'
     * and manages the CoW related resource lifecycle.
     * </p>
     * <p>
     * This iterator is the preferred choice if no mutations are made to the elements state
     * itself, or all changes can be discarded after the iterator's destruction.<br>
     * This avoids the costly mutex lock and storage copy of jau::cow_rw_iterator.<br>
     * Also see jau::for_each_fidelity to iterate through in this good faith fashion.
     * </p>
     * <p>
     * To allow data-race free operations on this iterator's data snapshot from a potentially mutated CoW,
     * only one begin iterator should be retrieved from CoW and all further operations shall use
     * jau::cow_ro_iterator::size(), jau::cow_ro_iterator::begin() and jau::cow_ro_iterator::end().
     * </p>
     * @see jau::cow_ro_iterator::size()
     * @see jau::cow_ro_iterator::begin()
     * @see jau::cow_ro_iterator::end()
     * @see jau::for_each_fidelity
     * @see jau::cow_darray
     */
    template <typename Storage_type, typename Storage_ref_type, typename CoW_container>
    class cow_ro_iterator {
        friend cow_rw_iterator<Storage_type, Storage_ref_type, CoW_container>;
        template<typename, typename, typename, bool, bool, bool> friend class cow_darray;
        template<typename, typename> friend class cow_vector;

        public:
            typedef Storage_type                                storage_t;
            typedef Storage_ref_type                            storage_ref_t;
            typedef CoW_container                               cow_container_t;

            /** Actual const iterator type of the contained native iterator, probably a simple pointer. */
            typedef typename storage_t::const_iterator          iterator_type;

        private:
            typedef std::iterator_traits<iterator_type>         sub_traits_t;

            storage_ref_t  store_ref_;
            iterator_type  iterator_;

            constexpr cow_ro_iterator(storage_ref_t store, iterator_type it) noexcept
            : store_ref_(store), iterator_(it) { }

        public:
            typedef typename sub_traits_t::iterator_category    iterator_category;  // random_access_iterator_tag

            typedef typename storage_t::size_type               size_type;          // using our template overload Size_type
            typedef typename storage_t::difference_type         difference_type;    // derived from our Size_type
            // typedef typename storage_t::value_type           value_type;         // OK
            // typedef typename storage_t::reference            reference;          // storage_t is not 'const'
            // typedef typename storage_t::pointer              pointer;            // storage_t is not 'const'
            typedef typename sub_traits_t::value_type           value_type;         // OK
            typedef typename sub_traits_t::reference            reference;          // 'const value_type &'
            typedef typename sub_traits_t::pointer              pointer;            // 'const value_type *'

#if __cplusplus > 201703L && __cpp_lib_concepts
            using iterator_concept = std::__detail::__iter_concept<_Iterator>;
#endif

        public:
            constexpr cow_ro_iterator() noexcept
            : store_ref_(nullptr), iterator_() { }

            // C++ named requirements: LegacyIterator: CopyConstructible
            constexpr cow_ro_iterator(const cow_ro_iterator& o) noexcept
            : store_ref_(o.store_ref_), iterator_(o.iterator_) {}

            // C++ named requirements: LegacyIterator: CopyAssignable
            constexpr cow_ro_iterator& operator=(const cow_ro_iterator& o) noexcept {
                if( this != &o ) {
                    store_ref_ = o.store_ref_;
                    iterator_ = o.iterator_;
                }
                return *this;
            }

            // C++ named requirements: LegacyIterator: MoveConstructable
            constexpr cow_ro_iterator(cow_ro_iterator && o) noexcept
            : store_ref_(std::move(o.store_ref_)), iterator_(std::move(o.iterator_)) {
                // Moved source has been disowned semantically and source's dtor will release resources!
            }

            // C++ named requirements: LegacyIterator: MoveAssignable
            constexpr cow_ro_iterator& operator=(cow_ro_iterator&& o) noexcept {
                if( this != &o ) {
                    store_ref_ = std::move(o.store_ref_);
                    iterator_ = std::move(o.iterator_);
                    // Moved source has been disowned semantically and source's dtor will release resources!
                }
                return *this;
            }

            // C++ named requirements: LegacyIterator: Swappable
            void swap(cow_ro_iterator& o) noexcept {
                std::swap( store_ref_, o.store_ref_);
                std::swap( iterator_, o.iterator_);
            }

            /**
             * Returns a new const_iterator pointing to the first element, aka begin.
             * <p>
             * This is an addition API entry, allowing data-race free operations on
             * this iterator's data snapshot from a potentially mutated CoW.
             * </p>
             * @see size()
             * @see end()
             */
            constexpr cow_ro_iterator cbegin() const noexcept
            { return cow_ro_iterator( store_ref_, store_ref_->cbegin() ); }

            /**
             * Returns a new const_iterator pointing to the <i>element following the last element</i>, aka end.<br>
             * <p>
             * This is an addition API entry, allowing data-race free operations on
             * this iterator's data snapshot from a potentially mutated CoW.
             * </p>
             * @see size()
             * @see begin()
             */
            constexpr cow_ro_iterator cend() const noexcept
            { return cow_ro_iterator( store_ref_, store_ref_->cend() ); }

            /**
             * Returns true if storage is empty().
             */
            constexpr bool empty() const noexcept { return store_ref_->empty(); }

            /**
             * Returns true if storage capacity has been reached and the next push_back()
             * will grow the storage and invalidates all iterators and references.
             */
            constexpr bool capacity_reached() const noexcept { return store_ref_->capacity_reached(); }

            /**
             * Return the size of the underlying value_type store.
             * <p>
             * This is an addition API entry, allowing data-race free arithmetic on
             * this iterator's data snapshot from a potentially mutated CoW.
             * </p>
             * @see begin()
             * @see end()
             */
            constexpr size_type size() const noexcept { return store_ref_->size(); }

            /**
             * Returns this instances' underlying shared storage by reference.
             */
            constexpr storage_t& storage() const noexcept {
                return *store_ref_;
            }

            /**
             * Returns the distance to_end() using zero as first index. A.k.a the remaining elements iterable.
             */
            constexpr difference_type dist_end() const noexcept { return store_ref_->cend() - iterator_; }

            /**
             * Returns true, if this iterator points to cend().
             */
            constexpr bool is_end() const noexcept { return iterator_ == store_ref_->cend(); }

            /**
             * This iterator is set to the last element, cend(). Returns *this;
             */
            constexpr cow_ro_iterator& to_end() noexcept
            { iterator_ = store_ref_->cend(); return *this; }

            /**
             * Returns the distance to_begin() using zero as first index. A.k.a the index from start.
             */
            constexpr difference_type dist_begin() const noexcept { return iterator_ - store_ref_->cbegin(); }

            /**
             * Returns true, if this iterator points to cbegin().
             */
            constexpr bool is_begin() const noexcept { return iterator_ == store_ref_->cbegin(); }

            /**
             * This iterator is set to the first element, cbegin(). Returns *this;
             */
            constexpr cow_ro_iterator& to_begin() noexcept
            { iterator_ = store_ref_->cbegin(); return *this; }

            /**
             * Returns a copy of the underlying storage const_iterator.
             * <p>
             * This is an addition API entry, inspired by the STL std::normal_iterator.
             * </p>
             */
            constexpr iterator_type base() const noexcept { return iterator_; };

            // Multipass guarantee equality

            /**
             * Returns signum or three-way comparison value
             * <pre>
             *    0 if equal (both, store and iteratore),
             *   -1 if this->iterator_ < rhs_iter and
             *    1 if this->iterator_ > rhs_iter (otherwise)
             * </pre>
             * @param rhs_store right-hand side store
             * @param rhs_iter right-hand side iterator
             */
            constexpr int compare(const cow_ro_iterator& rhs) const noexcept {
                return store_ref_ == rhs.store_ref_ && iterator_ == rhs.iterator_ ? 0
                       : ( iterator_ < rhs.iterator_ ? -1 : 1);
            }

            constexpr int compare(const cow_rw_iterator<storage_t, storage_ref_t, cow_container_t>& rhs) const noexcept {
                return store_ref_ == rhs.store_ref_ && iterator_ == rhs.iterator_ ? 0
                       : ( iterator_ < rhs.iterator_ ? -1 : 1);
            }

            constexpr bool operator==(const cow_ro_iterator& rhs) const noexcept
            { return compare(rhs) == 0; }

            constexpr bool operator!=(const cow_ro_iterator& rhs) const noexcept
            { return compare(rhs) != 0; }

            // Relation

            constexpr bool operator<=(const cow_ro_iterator& rhs) const noexcept
            { return compare(rhs) <= 0; }

            constexpr bool operator<(const cow_ro_iterator& rhs) const noexcept
            { return compare(rhs) < 0; }

            constexpr bool operator>=(const cow_ro_iterator& rhs) const noexcept
            { return compare(rhs) >= 0; }

            constexpr bool operator>(const cow_ro_iterator& rhs) const noexcept
            { return compare(rhs) > 0; }

            // Forward iterator requirements

            constexpr const reference operator*() const noexcept {
                return *iterator_;
            }

            constexpr const pointer operator->() const noexcept {
                return &(*iterator_); // just in case iterator_type is a class, trick via dereference
            }

            /** Pre-increment; Well performing, return *this.  */
            constexpr cow_ro_iterator& operator++() noexcept {
                ++iterator_;
                return *this;
            }

            /** Post-increment; Try to avoid: Low performance due to returning copy-ctor. */
            constexpr cow_ro_iterator operator++(int) noexcept
            { return cow_ro_iterator(store_ref_, iterator_++); }

            // Bidirectional iterator requirements

            /** Pre-decrement; Well performing, return *this.  */
            constexpr cow_ro_iterator& operator--() noexcept {
                --iterator_;
                return *this;
            }

            /** Post-decrement; Try to avoid: Low performance due to returning copy-ctor. */
            constexpr cow_ro_iterator operator--(int) noexcept
            { return cow_ro_iterator(store_ref_, iterator_--); }

            // Random access iterator requirements

            /** Subscript of 'element_index', returning immutable Value_type reference. */
            constexpr const reference operator[](difference_type i) const noexcept
            { return iterator_[i]; }

            /** Addition-assignment of 'element_count'; Well performing, return *this.  */
            constexpr cow_ro_iterator& operator+=(difference_type i) noexcept
            { iterator_ += i; return *this; }

            /** Binary 'iterator + element_count'; Try to avoid: Low performance due to returning copy-ctor. */
            constexpr cow_ro_iterator operator+(difference_type rhs) const noexcept
            { return cow_ro_iterator(store_ref_, iterator_ + rhs); }

            /** Subtraction-assignment of 'element_count'; Well performing, return *this.  */
            constexpr cow_ro_iterator& operator-=(difference_type i) noexcept
            { iterator_ -= i; return *this; }

            /** Binary 'iterator - element_count'; Try to avoid: Low performance due to returning copy-ctor. */
            constexpr cow_ro_iterator operator-(difference_type rhs) const noexcept
            { return cow_ro_iterator(store_ref_, iterator_ - rhs); }

            // Distance or element count, binary subtraction of two iterator.

            /** Binary 'iterator - iterator -> element_count'; Well performing, return element_count of type difference_type. */
            constexpr difference_type operator-(const cow_ro_iterator& rhs) const noexcept
            { return iterator_ - rhs.iterator_; }

            constexpr difference_type distance(const cow_rw_iterator<storage_t, storage_ref_t, cow_container_t>& rhs) const noexcept
            { return iterator_ - rhs.iterator_; }

            constexpr_func_cxx20 std::string toString() const noexcept {
                return jau::to_string(iterator_);
            }
#if 0
            constexpr_func_cxx20 operator std::string() const noexcept {
                return toString();
            }
#endif
            constexpr_func_cxx20 std::string get_info() const noexcept {
                return "cow_ro_iterator[this "+jau::aptrHexString(this)+
                        ", store "+jau::aptrHexString(&store_ref_)+
                       ", "+jau::to_string(iterator_)+"]";
            }
    };

    /****************************************************************************************
     ****************************************************************************************/

    template <typename Storage_type, typename Storage_ref_type, typename CoW_container>
    std::ostream & operator << (std::ostream &out, const cow_rw_iterator<Storage_type, Storage_ref_type, CoW_container> &c) {
        out << c.toString();
        return out;
    }

    template <typename Storage_type, typename Storage_ref_type, typename CoW_container>
    std::ostream & operator << (std::ostream &out, const cow_ro_iterator<Storage_type, Storage_ref_type, CoW_container> &c) {
        out << c.toString();
        return out;
    }

    /****************************************************************************************
     ****************************************************************************************/

    template <typename Storage_type, typename Storage_ref_type, typename CoW_container>
    constexpr bool operator==(const cow_ro_iterator<Storage_type, Storage_ref_type, CoW_container>& lhs,
                              const cow_rw_iterator<Storage_type, Storage_ref_type, CoW_container>& rhs) noexcept
    { return lhs.compare(rhs) == 0; }

    template <typename Storage_type, typename Storage_ref_type, typename CoW_container>
    constexpr bool operator!=(const cow_ro_iterator<Storage_type, Storage_ref_type, CoW_container>& lhs,
                              const cow_rw_iterator<Storage_type, Storage_ref_type, CoW_container>& rhs) noexcept
    { return lhs.compare(rhs) != 0; }

    template <typename Storage_type, typename Storage_ref_type, typename CoW_container>
    constexpr bool operator==(const cow_rw_iterator<Storage_type, Storage_ref_type, CoW_container>& lhs,
                              const cow_ro_iterator<Storage_type, Storage_ref_type, CoW_container>& rhs) noexcept
    { return rhs.compare(lhs) == 0; }

    template <typename Storage_type, typename Storage_ref_type, typename CoW_container>
    constexpr bool operator!=(const cow_rw_iterator<Storage_type, Storage_ref_type, CoW_container>& lhs,
                              const cow_ro_iterator<Storage_type, Storage_ref_type, CoW_container>& rhs) noexcept
    { return rhs.compare(lhs) != 0; }

    template <typename Storage_type, typename Storage_ref_type, typename CoW_container>
    constexpr bool operator<=(const cow_ro_iterator<Storage_type, Storage_ref_type, CoW_container>& lhs,
                              const cow_rw_iterator<Storage_type, Storage_ref_type, CoW_container>& rhs) noexcept
    { return lhs.compare(rhs) <= 0; }

    template <typename Storage_type, typename Storage_ref_type, typename CoW_container>
    constexpr bool operator<=(const cow_rw_iterator<Storage_type, Storage_ref_type, CoW_container>& lhs,
                              const cow_ro_iterator<Storage_type, Storage_ref_type, CoW_container>& rhs) noexcept
    { return rhs.compare(lhs) > 0; }

    template <typename Storage_type, typename Storage_ref_type, typename CoW_container>
    constexpr bool operator<(const cow_ro_iterator<Storage_type, Storage_ref_type, CoW_container>& lhs,
                             const cow_rw_iterator<Storage_type, Storage_ref_type, CoW_container>& rhs) noexcept
    { return lhs.compare(rhs) < 0; }

    template <typename Storage_type, typename Storage_ref_type, typename CoW_container>
    constexpr bool operator<(const cow_rw_iterator<Storage_type, Storage_ref_type, CoW_container>& lhs,
                             const cow_ro_iterator<Storage_type, Storage_ref_type, CoW_container>& rhs) noexcept
    { return rhs.compare(lhs) >= 0; }

    template <typename Storage_type, typename Storage_ref_type, typename CoW_container>
    constexpr bool operator>=(const cow_ro_iterator<Storage_type, Storage_ref_type, CoW_container>& lhs,
                              const cow_rw_iterator<Storage_type, Storage_ref_type, CoW_container>& rhs) noexcept
    { return lhs.compare(rhs) >= 0; }

    template <typename Storage_type, typename Storage_ref_type, typename CoW_container>
    constexpr bool operator>=(const cow_rw_iterator<Storage_type, Storage_ref_type, CoW_container>& lhs,
                              const cow_ro_iterator<Storage_type, Storage_ref_type, CoW_container>& rhs) noexcept
    { return rhs.compare(lhs) < 0; }

    template <typename Storage_type, typename Storage_ref_type, typename CoW_container>
    constexpr bool operator>(const cow_ro_iterator<Storage_type, Storage_ref_type, CoW_container>& lhs,
                             const cow_rw_iterator<Storage_type, Storage_ref_type, CoW_container>& rhs) noexcept
    { return lhs.compare(rhs) > 0; }

    template <typename Storage_type, typename Storage_ref_type, typename CoW_container>
    constexpr bool operator>(const cow_rw_iterator<Storage_type, Storage_ref_type, CoW_container>& lhs,
                             const cow_ro_iterator<Storage_type, Storage_ref_type, CoW_container>& rhs) noexcept
    { return rhs.compare(lhs) <= 0; }

    template <typename Storage_type, typename Storage_ref_type, typename CoW_container>
    constexpr typename Storage_type::difference_type operator-
                ( const cow_ro_iterator<Storage_type, Storage_ref_type, CoW_container>& lhs,
                  const cow_rw_iterator<Storage_type, Storage_ref_type, CoW_container>& rhs) noexcept
    { return lhs.distance(rhs); }

    template <typename Storage_type, typename Storage_ref_type, typename CoW_container>
    constexpr typename Storage_type::difference_type operator-
                ( const cow_rw_iterator<Storage_type, Storage_ref_type, CoW_container>& lhs,
                  const cow_ro_iterator<Storage_type, Storage_ref_type, CoW_container>& rhs) noexcept
    { return rhs.distance(lhs) * -1; }

    /****************************************************************************************
     ****************************************************************************************/

    /**
     * <code>template< class T > is_cow_type<T>::value</code> compile-time Type Trait,
     * determining whether the given template class is a CoW type, e.g. jau::cow_darray,
     * jau::cow_vector or any of their iterator.
     */
    template< class, class = void >
    struct is_cow_type : std::false_type { };

    /**
     * <code>template< class T > is_cow_type<T>::value</code> compile-time Type Trait,
     * determining whether the given template class is a CoW type, e.g. jau::cow_darray,
     * jau::cow_vector or any of their iterator.
     */
    template< class T >
    struct is_cow_type<T, std::void_t<typename T::cow_container_t>> : std::true_type { };

    /****************************************************************************************
     ****************************************************************************************/

} /* namespace jau */


#endif /* JAU_COW_ITERATOR_HPP_ */
