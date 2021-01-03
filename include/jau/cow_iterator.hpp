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

#include <type_traits>

namespace jau {

    /**
     * Implementation of a Copy-On-Write (CoW) read-only iterator for immutable value_type.<br>
     * Instance holds the 'shared value_type reference', Storage_ref_type,
     * of the current CoW storage until destruction.
     * <p>
     * This iterator simply wraps the native iterator of type 'iterator_type'
     * and manages the CoW related resource lifecycle.
     * </p>
     * <p>
     * Implementation complies with iterator_category 'random_access_iterator_tag'
     * </p>
     */
    template <typename Storage_type, typename Storage_ref_type>
    class cow_ro_iterator {
        public:
            /** Actual const iterator type of the contained native iterator, probably a simple pointer. */
            typedef typename Storage_type::const_iterator       iterator_type;

        private:
            typedef std::iterator_traits<iterator_type>         sub_traits_t;

            Storage_ref_type store_holder_;
            iterator_type    iterator_;

        public:
            typedef typename sub_traits_t::iterator_category    iterator_category;  // random_access_iterator_tag

            typedef typename Storage_type::size_type            size_type;          // using our template overload Size_type
            typedef typename Storage_type::difference_type      difference_type;    // derived from our Size_type
            // typedef typename Storage_type::value_type        value_type;         // OK
            // typedef typename Storage_type::reference         reference;          // for some reason, the 'const' is lost
            // typedef typename Storage_type::pointer           pointer;            // for some reason, the 'const' is lost
            typedef typename sub_traits_t::value_type           value_type;         // OK
            typedef typename sub_traits_t::reference            reference;          // 'const value_type &'
            typedef typename sub_traits_t::pointer              pointer;            // 'const value_type *'

#if 0
// TODO ???
#if __cplusplus > 201703L && __cpp_lib_concepts
            using iterator_concept = std::__detail::__iter_concept<_Iterator>;
#endif
#endif

        public:
            constexpr cow_ro_iterator(Storage_ref_type store, iterator_type iter)
            : store_holder_(store), iterator_(iter) { }

            // C++ named requirements: LegacyIterator: CopyConstructible
            constexpr cow_ro_iterator(const cow_ro_iterator& o) noexcept
            : store_holder_(o.store_holder_), iterator_(o.iterator_) {}

            // C++ named requirements: LegacyIterator: CopyAssignable
            constexpr cow_ro_iterator& operator=(const cow_ro_iterator& o) noexcept {
                store_holder_ = o.store_holder_;
                iterator_ = o.iterator_;
            }

            // C++ named requirements: LegacyIterator: MoveConstructable
            constexpr cow_ro_iterator(cow_ro_iterator && o) noexcept
            : store_holder_(std::move(o.store_holder_)), iterator_(std::move(o.iterator_)) {
                o.store_holder_ = nullptr;
                // o.iterator_ = nullptr;
            }

            // C++ named requirements: LegacyIterator: MoveAssignable
            constexpr cow_ro_iterator& operator=(cow_ro_iterator&& o) noexcept {
                store_holder_ = std::move(o.store_holder_);
                iterator_ = std::move(o.iterator_);
                o.store_holder_ = nullptr;
                // o.iterator_ = nullptr;
            }

            // C++ named requirements: LegacyIterator: Swappable
            void swap(cow_ro_iterator& o) noexcept {
                std::swap( store_holder_, o.store_holder_);
                std::swap( iterator_, o.iterator_);
            }

            /**
             * Returns a copy of the underlying storage const_iterator.
             */
            iterator_type underling() const noexcept { return iterator_; };

            // Multipass guarantee equality

            constexpr bool operator==(const cow_ro_iterator& rhs) const noexcept {
                if( this == &rhs ) {
                    return true;
                }
                // only testing identity of pointer, OK for Multipass guarantee
                return store_holder_ == rhs.store_holder_ &&
                       iterator_ == rhs.iterator_;
            }
            constexpr bool operator!=(const cow_ro_iterator& rhs) const noexcept
            { return !(*this == rhs); }

            // Relation

            constexpr bool operator<=(const cow_ro_iterator& rhs) const noexcept {
                if( this == &rhs ) {
                    return true;
                }
                return store_holder_ == rhs.store_holder_ &&
                       iterator_ <= rhs.iterator_;
            }
            constexpr bool operator<(const cow_ro_iterator& rhs) const noexcept {
                if( this == &rhs ) {
                    return false;
                }
                return store_holder_ == rhs.store_holder_ &&
                       iterator_ < rhs.iterator_;
            }
            constexpr bool operator>=(const cow_ro_iterator& rhs) const noexcept {
                if( this == &rhs ) {
                    return true;
                }
                return store_holder_ == rhs.store_holder_ &&
                       iterator_ >= rhs.iterator_;
            }
            constexpr bool operator>(const cow_ro_iterator& rhs) const noexcept {
                if( this == &rhs ) {
                    return false;
                }
                return store_holder_ == rhs.store_holder_ &&
                       iterator_ > rhs.iterator_;
            }

            // Forward iterator requirements

            constexpr const reference operator*() const noexcept
            { return *iterator_; }

            constexpr const pointer operator->() const noexcept
            { return iterator_; }

            /** Pre-increment; Well performing, return *this.  */
            constexpr cow_ro_iterator& operator++() noexcept {
                ++iterator_;
                return *this;
            }

            /** Post-increment; Try to avoid: Low performance due to returning copy-ctor. */
            constexpr cow_ro_iterator operator++(int) noexcept
            { return cow_ro_iterator(store_holder_, iterator_++); }

            // Bidirectional iterator requirements

            /** Pre-decrement; Well performing, return *this.  */
            constexpr cow_ro_iterator& operator--() noexcept {
                --iterator_;
                return *this;
            }

            /** Post-decrement; Try to avoid: Low performance due to returning copy-ctor. */
            constexpr cow_ro_iterator operator--(int) noexcept
            { return cow_ro_iterator(store_holder_, iterator_--); }

            // Random access iterator requirements

            /** Subscript of 'element_index', returning immutable Value_type reference. */
            constexpr const reference operator[](difference_type i) const noexcept
            { return iterator_[i]; }

            /** Addition-assignment of 'element_count'; Well performing, return *this.  */
            constexpr cow_ro_iterator& operator+=(difference_type i) noexcept
            { iterator_ += i; return *this; }

            /** Binary 'iterator + element_count'; Try to avoid: Low performance due to returning copy-ctor. */
            constexpr cow_ro_iterator operator+(difference_type rhs) const noexcept
            { return cow_ro_iterator(store_holder_, iterator_ + rhs); }

            /** Subtraction-assignment of 'element_count'; Well performing, return *this.  */
            constexpr cow_ro_iterator& operator-=(difference_type i) noexcept
            { iterator_ -= i; return *this; }

            /** Binary 'iterator - element_count'; Try to avoid: Low performance due to returning copy-ctor. */
            constexpr cow_ro_iterator operator-(difference_type rhs) const noexcept
            { return cow_ro_iterator(store_holder_, iterator_ - rhs); }

            // Distance or element count, binary subtraction of two iterator.

            /** Binary 'iterator - iterator -> element_count'; Well performing, return element_count of type difference_type. */
            constexpr difference_type operator-(const cow_ro_iterator& rhs) const noexcept
            { return iterator_ - rhs.iterator_; }
    };

    /**
     * Implementation of a Copy-On-Write (CoW) read-write iterator for mutable value_type.<br>
     * Instance holds the 'shared value_type reference', Storage_ref_type,
     * of the current CoW storage until destruction.
     * <p>
     * This iterator simply wraps the native iterator of type 'iterator_type'
     * and manages the CoW related resource lifecycle.
     * </p>
     * <p>
     * At destruction, the mutated local storage will replace the
     * storage in the CoW container and the lock will be released.
     * </p>
     */
    template <typename Storage_type, typename Storage_ref_type, typename CoW_container>
    class cow_rw_iterator {
        public:
            /** Actual iterator type of the contained native iterator, probably a simple pointer. */
            typedef typename Storage_type::iterator             iterator_type;

        private:
            typedef std::iterator_traits<iterator_type>         sub_traits_t;

            CoW_container& cow_parent_;
            std::lock_guard<std::recursive_mutex> lock_;
            Storage_ref_type new_store_;
            iterator_type    iterator_;

            constexpr explicit cow_rw_iterator(CoW_container& cow_parent, Storage_ref_type& store, iterator_type iter) noexcept
            : cow_parent_(cow_parent), lock_(cow_parent.get_write_mutex()), new_store_(store), iterator_(iter) {}

        public:
            typedef typename sub_traits_t::iterator_category    iterator_category;  // random_access_iterator_tag

            typedef typename Storage_type::size_type            size_type;          // using our template overload Size_type
            typedef typename Storage_type::difference_type      difference_type;    // derived from our Size_type
            // typedef typename Storage_type::value_type        value_type;         // OK
            // typedef typename Storage_type::reference         reference;          //
            // typedef typename Storage_type::pointer           pointer;            //
            typedef typename sub_traits_t::value_type           value_type;         // OK
            typedef typename sub_traits_t::reference            reference;          // 'value_type &'
            typedef typename sub_traits_t::pointer              pointer;            // 'value_type *'

    #if 0
    // TODO ???
    #if __cplusplus > 201703L && __cpp_lib_concepts
            using iterator_concept = std::__detail::__iter_concept<_Iterator>;
    #endif
    #endif

        public:

            constexpr cow_rw_iterator(CoW_container& cow_parent, iterator_type (*get_iterator)(Storage_ref_type&))
            : cow_parent_(cow_parent), lock_(cow_parent.get_write_mutex()),
              new_store_(cow_parent.copy_store()), iterator_(get_iterator(new_store_)) {}

            constexpr cow_rw_iterator(CoW_container& cow_parent, iterator_type iter)
            : cow_parent_(cow_parent), lock_(cow_parent.get_write_mutex()),
              new_store_(cow_parent.copy_store()), iterator_(iter) {}

#if __cplusplus > 201703L
            constexpr ~cow_rw_iterator() noexcept
#else
            ~cow_rw_iterator() noexcept
#endif
            {
                cow_parent_.set_store(std::move(new_store_));
            }

            // C++ named requirements: LegacyIterator: CopyConstructible
            constexpr cow_rw_iterator(const cow_rw_iterator& o) noexcept
            : cow_parent_(o.cow_parent_), lock_(cow_parent_.get_write_mutex()),
              new_store_(o.new_store_), iterator_(o.iterator_) {}

            // C++ named requirements: LegacyIterator: CopyAssignable
            constexpr cow_rw_iterator& operator=(const cow_rw_iterator& o) noexcept {
                cow_parent_ = o.cow_parent_;
                lock_ = cow_parent_.get_write_mutex();
                new_store_ = o.new_store_;
                iterator_ = o.iterator_;
            }

            // C++ named requirements: LegacyIterator: MoveConstructable
            constexpr cow_rw_iterator(cow_rw_iterator && o) noexcept
            : cow_parent_(std::move(o.cow_parent_)), lock_(cow_parent_.get_write_mutex()),
              new_store_(std::move(o.new_store_)), iterator_(std::move(o.iterator_)) {
                o.lock_ = nullptr; // ???
                o.new_store_ = nullptr;
                // o.iterator_ = nullptr;
            }

            // C++ named requirements: LegacyIterator: MoveAssignable
            constexpr cow_rw_iterator& operator=(cow_rw_iterator&& o) noexcept {
                cow_parent_ = std::move(o.cow_parent_);
                lock_ = cow_parent_.get_write_mutex();
                new_store_ = std::move(o.new_store_);
                iterator_ = std::move(o.iterator_);
                o.new_store_ = nullptr;
                // o.iterator_ = nullptr;
            }

            // C++ named requirements: LegacyIterator: Swappable
            void swap(cow_rw_iterator& o) noexcept {
                std::swap( cow_parent_, o.cow_parent_);
                // std::swap( lock_, o.lock_); // lock stays in each
                std::swap( new_store_, o.new_store_);
                std::swap( iterator_, o.iterator_);
            }

            /**
             * Returns a copy of the underlying storage iterator.
             */
            typename Storage_type::iterator underling() const noexcept { return iterator_; };

            // Multipass guarantee equality

            constexpr bool operator==(const cow_rw_iterator& rhs) const noexcept {
                if( this == &rhs ) {
                    return true;
                }
                // only testing identity of pointer, OK for Multipass guarantee
                return &cow_parent_ == &rhs.cow_parent_ &&
                       new_store_ == rhs.new_store_ &&
                       iterator_ == rhs.iterator_;
            }
            constexpr bool operator!=(const cow_rw_iterator& rhs) const noexcept
            { return !(*this == rhs); }

            // Relation

            constexpr bool operator<=(const cow_rw_iterator& rhs) const noexcept {
                if( this == &rhs ) {
                    return true;
                }
                return &cow_parent_ == &rhs.cow_parent_ &&
                       new_store_ == rhs.new_store_ &&
                       iterator_ <= rhs.iterator_;
            }
            constexpr bool operator<(const cow_rw_iterator& rhs) const noexcept {
                if( this == &rhs ) {
                    return false;
                }
                return &cow_parent_ == &rhs.cow_parent_ &&
                       new_store_ == rhs.new_store_ &&
                       iterator_ < rhs.iterator_;
            }
            constexpr bool operator>=(const cow_rw_iterator& rhs) const noexcept {
                if( this == &rhs ) {
                    return true;
                }
                return &cow_parent_ == &rhs.cow_parent_ &&
                       new_store_ == rhs.new_store_ &&
                       iterator_ >= rhs.iterator_;
            }
            constexpr bool operator>(const cow_rw_iterator& rhs) const noexcept {
                if( this == &rhs ) {
                    return false;
                }
                return &cow_parent_ == &rhs.cow_parent_ &&
                       new_store_ == rhs.new_store_ &&
                       iterator_ > rhs.iterator_;
            }

            // Forward iterator requirements

            constexpr const reference operator*() const noexcept
            { return *iterator_; }

            constexpr const pointer operator->() const noexcept
            { return iterator_; }

            constexpr reference operator*() noexcept
            { return *iterator_; }

            constexpr pointer operator->() noexcept
            { return iterator_; }

            /** Pre-increment; Well performing, return *this.  */
            constexpr cow_rw_iterator& operator++() noexcept {
                ++iterator_;
                return *this;
            }

            /** Post-increment; Try to avoid: Low performance due to returning copy-ctor. */
            constexpr cow_rw_iterator operator++(int) noexcept
            { return cow_rw_iterator(cow_parent_, new_store_, iterator_++); }

            // Bidirectional iterator requirements

            /** Pre-decrement; Well performing, return *this.  */
            constexpr cow_rw_iterator& operator--() noexcept {
                --iterator_;
                return *this;
            }

            /** Post-decrement; Try to avoid: Low performance due to returning copy-ctor. */
            constexpr cow_rw_iterator operator--(int) noexcept
            { return cow_rw_iterator(cow_parent_, new_store_, iterator_--); }

            // Random access iterator requirements

            /** Subscript of 'element_index', returning immutable Value_type reference. */
            constexpr const reference operator[](difference_type i) const noexcept
            { return iterator_[i]; }

            /** Subscript of 'element_index', returning mutable Value_type reference. */
            constexpr reference operator[](difference_type i) noexcept
            { return iterator_[i]; }

            /** Addition-assignment of 'element_count'; Well performing, return *this.  */
            constexpr cow_rw_iterator& operator+=(difference_type i) noexcept
            { iterator_ += i; return *this; }

            /** Binary 'iterator + element_count'; Try to avoid: Low performance due to returning copy-ctor. */
            constexpr cow_rw_iterator operator+(difference_type rhs) const noexcept
            { return cow_rw_iterator(cow_parent_, new_store_, iterator_ + rhs); }

            /** Subtraction-assignment of 'element_count'; Well performing, return *this.  */
            constexpr cow_rw_iterator& operator-=(difference_type i) noexcept
            { iterator_ -= i; return *this; }

            /** Binary 'iterator - element_count'; Try to avoid: Low performance due to returning copy-ctor. */
            constexpr cow_rw_iterator operator-(difference_type rhs) const noexcept
            { return cow_rw_iterator(cow_parent_, new_store_, iterator_ - rhs); }

            // constexpr const cow_rw_iterator& base() const noexcept
            // { return iterator_; }

            // Distance or element count, binary subtraction of two iterator.

            /** Binary 'iterator - iterator -> element_count'; Well performing, return element_count of type difference_type. */
            constexpr difference_type operator-(const cow_rw_iterator& rhs) const noexcept
            { return iterator_ - rhs.iterator_; }
    };

} /* namespace jau */

#endif /* JAU_COW_ITERATOR_HPP_ */
