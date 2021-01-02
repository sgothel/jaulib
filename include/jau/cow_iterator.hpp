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

namespace jau {

    /**
     * Implementation of a Copy-On-Write (CoW) read-only iterator for immutable Value_type,
     * holding the shared Value_type& of the current CoW storage until destruction.
     */
    template <typename Value_type, typename Storage_type, typename Storage_ref_type, typename Size_type>
    class cow_ro_iterator {

        private:
            Storage_ref_type store_holder_;
            typename Storage_type::const_iterator iterator_;

        public:
            typedef Size_type                                   size_type;
            typedef typename std::make_signed<Size_type>::type  difference_type;

            constexpr cow_ro_iterator(Storage_ref_type store, typename Storage_type::const_iterator iter)
            : store_holder_(store), iterator_(iter) {}

            constexpr explicit cow_ro_iterator(const cow_ro_iterator& o) noexcept
            : store_holder_(o.store_holder_), iterator_(o.iterator_) {}

            /**
             * Returns a copy of the underlying storage const_iterator.
             */
            typename Storage_type::const_iterator underling() const noexcept { return iterator_; };

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

            // Forward iterator requirements

            constexpr const Value_type& operator*() const noexcept
            { return *iterator_; }

            constexpr const Value_type* operator->() const noexcept
            { return iterator_; }

            /** Pre-increment; Well performing, return *this.  */
            constexpr cow_ro_iterator& operator++() noexcept {
                ++iterator_;
                return *this;
            }

            /** Post-increment; Try to avoid: Low performance due to returning copy-ctor. */
            constexpr cow_ro_iterator& operator++(int) noexcept
            { return cow_ro_iterator(store_holder_, iterator_++); }

            // Bidirectional iterator requirements

            /** Pre-decrement; Well performing, return *this.  */
            constexpr cow_ro_iterator& operator--() noexcept {
                --iterator_;
                return *this;
            }

            /** Post-decrement; Try to avoid: Low performance due to returning copy-ctor. */
            constexpr cow_ro_iterator& operator--(int) noexcept
            { return cow_iterator(store_holder_, iterator_--); }

            // Random access iterator requirements

            /** Subscript of 'element_index', returning immutable Value_type reference. */
            constexpr const Value_type& operator[](difference_type i) const noexcept
            { return iterator_[i]; }

            /** Addition-assignment of 'element_count'; Well performing, return *this.  */
            constexpr cow_ro_iterator& operator+=(difference_type i) noexcept
            { iterator_ += i; return *this; }

            /** Binary 'iterator + element_count'; Try to avoid: Low performance due to returning copy-ctor. */
            constexpr cow_ro_iterator operator+(difference_type rhs) const noexcept
            { return cow_iterator(store_holder_, iterator_ + rhs); }

            /** Subtraction-assignment of 'element_count'; Well performing, return *this.  */
            constexpr cow_ro_iterator& operator-=(difference_type i) noexcept
            { iterator_ -= i; return *this; }

            /** Binary 'iterator - element_count'; Try to avoid: Low performance due to returning copy-ctor. */
            constexpr cow_ro_iterator operator-(difference_type rhs) const noexcept
            { return cow_iterator(store_holder_, iterator_ - rhs); }

            // Distance or element count, binary subtraction of two iterator.

            /** Binary 'iterator - iterator -> element_count'; Well performing, return element_count of type difference_type. */
            constexpr difference_type operator-(const cow_ro_iterator& rhs) const noexcept
            { return iterator_ - rhs.iterator_; }
    };

    /**
     * Implementation of a Copy-On-Write (CoW) read-write iterator for mutable Value_type,
     * holding the write lock, a new copy of the CoW storage and a Value_type& of the CoW container itself.
     * <p>
     * At destruction, the mutated local storage will replace the
     * storage in the CoW container and the lock will be released.
     * </p>
     */
    template <typename Value_type, typename Storage_type, typename Storage_ref_type, typename CoW_container, typename Size_type>
    class cow_rw_iterator {

        private:
            CoW_container& cow_parent_;
            const std::lock_guard<std::recursive_mutex> lock_;
            Storage_ref_type new_store_;
            typename Storage_type::iterator iterator_;

            constexpr explicit cow_rw_iterator(CoW_container& cow_parent, Storage_ref_type& store, typename Storage_type::iterator iter) noexcept
            : cow_parent_(cow_parent), lock_(cow_parent.get_write_mutex()), new_store_(store), iterator_(iter) {}

        public:
            typedef Size_type                                   size_type;
            typedef typename std::make_signed<Size_type>::type  difference_type;

            constexpr cow_rw_iterator(CoW_container& cow_parent, typename Storage_type::iterator (*get_iterator)(Storage_ref_type&))
            : cow_parent_(cow_parent), lock_(cow_parent.get_write_mutex()), new_store_(cow_parent.copy_store()), iterator_(get_iterator(new_store_)) {}

#if __cplusplus > 201703L
            constexpr ~cow_rw_iterator() noexcept
#else
            ~cow_rw_iterator() noexcept
#endif
            {
                cow_parent_.set_store(std::move(new_store_));
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

            // Forward iterator requirements

            constexpr const Value_type& operator*() const noexcept
            { return *iterator_; }

            constexpr const Value_type* operator->() const noexcept
            { return iterator_; }

            constexpr Value_type& operator*() noexcept
            { return *iterator_; }

            constexpr Value_type* operator->() noexcept
            { return iterator_; }

            /** Pre-increment; Well performing, return *this.  */
            constexpr cow_rw_iterator& operator++() noexcept {
                ++iterator_;
                return *this;
            }

            /** Post-increment; Try to avoid: Low performance due to returning copy-ctor. */
            constexpr cow_rw_iterator& operator++(int) noexcept
            { return cow_rw_iterator(cow_parent_, new_store_, iterator_++); }

            // Bidirectional iterator requirements

            /** Pre-decrement; Well performing, return *this.  */
            constexpr cow_rw_iterator& operator--() noexcept {
                --iterator_;
                return *this;
            }

            /** Post-decrement; Try to avoid: Low performance due to returning copy-ctor. */
            constexpr cow_rw_iterator& operator--(int) noexcept
            { return cow_rw_iterator(cow_parent_, new_store_, iterator_--); }

            // Random access iterator requirements

            /** Subscript of 'element_index', returning immutable Value_type reference. */
            constexpr const Value_type& operator[](difference_type i) const noexcept
            { return iterator_[i]; }

            /** Subscript of 'element_index', returning mutable Value_type reference. */
            constexpr Value_type& operator[](difference_type i) noexcept
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
