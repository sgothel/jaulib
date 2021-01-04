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
#include <iostream>

#include <jau/cpp_lang_macros.hpp>
#include <jau/basic_types.hpp>

namespace jau {

    // forward declaration for friendship with cow_rw_iterator
    template <typename Storage_type, typename Storage_ref_type, typename CoW_container>
    class cow_ro_iterator;

    template <typename Storage_type, typename Storage_ref_type, typename CoW_container>
    class cow_rw_iterator;

    /**
     * iterator_type -> std::string conversion
     * <p>
     * TODO: Test whether 'base()' method actually exists,
     * however - this is good enough for stl iterator_type
     * having implemented the inner native iterator (a pointer).
     * </p>
     * @tparam iterator_type the iterator type
     * @param iter the iterator
     * @param if iterator_type is a class
     * @return the std::string represenation
     */
    template< class iterator_type >
        std::string to_string(const iterator_type & iter,
            typename std::enable_if<
                    std::is_class<iterator_type>::value
                >::type* = 0
    ) {
        return aptrHexString( (void*) ( iter.base() ) );
    }

    /**
     * iterator_type -> std::string conversion
     * @tparam iterator_type the iterator type
     * @param iter the iterator
     * @param if iterator_type is not a class
     * @return the std::string represenation
     */
    template< class iterator_type >
        std::string to_string(const iterator_type & iter,
            typename std::enable_if<
                    !std::is_class<iterator_type>::value
                >::type* = 0
    ) {
        return aptrHexString((void*)iter);
    }

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
        friend cow_ro_iterator<Storage_type, Storage_ref_type, CoW_container>;

        public:
            /** Actual iterator type of the contained native iterator, probably a simple pointer. */
            typedef typename Storage_type::iterator             iterator_type;

        private:
            typedef std::iterator_traits<iterator_type>         sub_traits_t;

            CoW_container& cow_parent_;
            std::lock_guard<std::recursive_mutex> lock_;
            Storage_ref_type store_ref_;
            iterator_type    iterator_;

            constexpr explicit cow_rw_iterator(CoW_container& cow_parent, const Storage_ref_type& store, iterator_type iter) noexcept
            : cow_parent_(cow_parent), lock_(cow_parent.get_write_mutex()), store_ref_(store), iterator_(iter) {}

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

#if __cplusplus > 201703L && __cpp_lib_concepts
            using iterator_concept = std::__detail::__iter_concept<_Iterator>;
#endif

        public:

            constexpr cow_rw_iterator(CoW_container& cow_parent, iterator_type (*get_iterator)(Storage_ref_type&))
            : cow_parent_(cow_parent), lock_(cow_parent.get_write_mutex()),
              store_ref_(cow_parent.copy_store()), iterator_(get_iterator(store_ref_)) {}

#if 0
            constexpr cow_rw_iterator(CoW_container& cow_parent, iterator_type iter)
            : cow_parent_(cow_parent), lock_(cow_parent.get_write_mutex()),
              store_ref_(cow_parent.copy_store()), iterator_(iter) {}
#endif

#if __cplusplus > 201703L
            constexpr ~cow_rw_iterator() noexcept
#else
            ~cow_rw_iterator() noexcept
#endif
            {
                cow_parent_.set_store(std::move(store_ref_));
            }

            // C++ named requirements: LegacyIterator: CopyConstructible
            constexpr cow_rw_iterator(const cow_rw_iterator& o) noexcept
            : cow_parent_(o.cow_parent_), lock_(cow_parent_.get_write_mutex()),
              store_ref_(o.store_ref_), iterator_(o.iterator_) {}

            // C++ named requirements: LegacyIterator: CopyAssignable
            constexpr cow_rw_iterator& operator=(const cow_rw_iterator& o) noexcept {
                cow_parent_ = o.cow_parent_;
                lock_ = cow_parent_.get_write_mutex();
                store_ref_ = o.store_ref_;
                iterator_ = o.iterator_;
                return *this;
            }

            // C++ named requirements: LegacyIterator: MoveConstructable
            constexpr cow_rw_iterator(cow_rw_iterator && o) noexcept
            : cow_parent_(std::move(o.cow_parent_)), lock_(cow_parent_.get_write_mutex()),
              store_ref_(std::move(o.store_ref_)), iterator_(std::move(o.iterator_)) {
                o.lock_ = nullptr; // ???
                o.store_ref_ = nullptr;
                // o.iterator_ = nullptr;
            }

            // C++ named requirements: LegacyIterator: MoveAssignable
            constexpr cow_rw_iterator& operator=(cow_rw_iterator&& o) noexcept {
                cow_parent_ = std::move(o.cow_parent_);
                lock_ = cow_parent_.get_write_mutex();
                store_ref_ = std::move(o.store_ref_);
                iterator_ = std::move(o.iterator_);
                o.store_ref_ = nullptr;
                // o.iterator_ = nullptr;
                return *this;
            }

            // C++ named requirements: LegacyIterator: Swappable
            void swap(cow_rw_iterator& o) noexcept {
                std::swap( cow_parent_, o.cow_parent_);
                // std::swap( lock_, o.lock_); // lock stays in each
                std::swap( store_ref_, o.store_ref_);
                std::swap( iterator_, o.iterator_);
            }

            /**
             * Returns a copy of the underlying storage iterator.
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

            constexpr const reference operator*() const noexcept {
                return *iterator_;
            }

            constexpr const pointer operator->() const noexcept {
                return &(*iterator_); // just in case iterator_type is a class, trick via dereference
            }

            constexpr reference operator*() noexcept {
                return *iterator_;
            }

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

            /** Subscript of 'element_index', returning mutable Value_type reference. */
            constexpr reference operator[](difference_type i) noexcept
            { return iterator_[i]; }

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

            __constexpr_cxx20_ std::string toString() const noexcept {
                return "cow_rw_iterator["+jau::to_string(iterator_)+"]";
            }
#if 0
            __constexpr_cxx20_ operator std::string() const noexcept {
                return toString();
            }
#endif
    };

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
    template <typename Storage_type, typename Storage_ref_type, typename CoW_container>
    class cow_ro_iterator {
        public:
            /** Actual const iterator type of the contained native iterator, probably a simple pointer. */
            typedef typename Storage_type::const_iterator       iterator_type;

        private:
            typedef std::iterator_traits<iterator_type>         sub_traits_t;

            Storage_ref_type store_ref_;
            iterator_type    iterator_;

        public:
            typedef typename sub_traits_t::iterator_category    iterator_category;  // random_access_iterator_tag

            typedef typename Storage_type::size_type            size_type;          // using our template overload Size_type
            typedef typename Storage_type::difference_type      difference_type;    // derived from our Size_type
            // typedef typename Storage_type::value_type        value_type;         // OK
            // typedef typename Storage_type::reference         reference;          // Storage_type is not 'const'
            // typedef typename Storage_type::pointer           pointer;            // Storage_type is not 'const'
            typedef typename sub_traits_t::value_type           value_type;         // OK
            typedef typename sub_traits_t::reference            reference;          // 'const value_type &'
            typedef typename sub_traits_t::pointer              pointer;            // 'const value_type *'

#if __cplusplus > 201703L && __cpp_lib_concepts
            using iterator_concept = std::__detail::__iter_concept<_Iterator>;
#endif

        public:
            constexpr cow_ro_iterator() noexcept
            : store_ref_(nullptr), iterator_() { }

            constexpr cow_ro_iterator(Storage_ref_type store, iterator_type iter) noexcept
            : store_ref_(store), iterator_(iter) { }

            /**
             * Conversion constructor: cow_rw_iterator -> cow_ro_iterator
             * <p>
             * Explicit due to high costs of potential automatic and accidental conversion,
             * using a temporary cow_rw_iterator instance involving storage copy etc.
             * </p>
             */
            constexpr explicit cow_ro_iterator(const cow_rw_iterator<Storage_type, Storage_ref_type, CoW_container>& o) noexcept
            : store_ref_(o.store_ref_), iterator_(o.iterator_) {}

            // C++ named requirements: LegacyIterator: CopyConstructible
            constexpr cow_ro_iterator(const cow_ro_iterator& o) noexcept
            : store_ref_(o.store_ref_), iterator_(o.iterator_) {}

            // C++ named requirements: LegacyIterator: CopyAssignable
            constexpr cow_ro_iterator& operator=(const cow_ro_iterator& o) noexcept {
                store_ref_ = o.store_ref_;
                iterator_ = o.iterator_;
                return *this;
            }

            constexpr cow_ro_iterator& operator=(const cow_rw_iterator<Storage_type, Storage_ref_type, CoW_container>& o) noexcept {
                store_ref_ = o.store_ref_;
                iterator_ = o.iterator_;
                return *this;
            }

            // C++ named requirements: LegacyIterator: MoveConstructable
            constexpr cow_ro_iterator(cow_ro_iterator && o) noexcept
            : store_ref_(std::move(o.store_ref_)), iterator_(std::move(o.iterator_)) {
                o.store_ref_ = nullptr;
                // o.iterator_ = nullptr;
            }

            // C++ named requirements: LegacyIterator: MoveAssignable
            constexpr cow_ro_iterator& operator=(cow_ro_iterator&& o) noexcept {
                store_ref_ = std::move(o.store_ref_);
                iterator_ = std::move(o.iterator_);
                o.store_ref_ = nullptr;
                // o.iterator_ = nullptr;
                return *this;
            }

            // C++ named requirements: LegacyIterator: Swappable
            void swap(cow_ro_iterator& o) noexcept {
                std::swap( store_ref_, o.store_ref_);
                std::swap( iterator_, o.iterator_);
            }

            /**
             * Returns a copy of the underlying storage const_iterator.
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

            constexpr int compare(const cow_rw_iterator<Storage_type, Storage_ref_type, CoW_container>& rhs) const noexcept {
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

            constexpr difference_type distance(const cow_rw_iterator<Storage_type, Storage_ref_type, CoW_container>& rhs) const noexcept
            { return iterator_ - rhs.iterator_; }

            __constexpr_cxx20_ std::string toString() const noexcept {
                return "cow_ro_iterator["+jau::to_string(iterator_)+"]";
            }
#if 0
            __constexpr_cxx20_ operator std::string() const noexcept {
                return toString();
            }
#endif
    };

} /* namespace jau */

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


#endif /* JAU_COW_ITERATOR_HPP_ */
