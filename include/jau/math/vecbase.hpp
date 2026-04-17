/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright Gothel Software e.K.
 *
 * SPDX-License-Identifier: MIT
 *
 * This Source Code Form is subject to the terms of the MIT License
 * If a copy of the MIT was not distributed with this file,
 * you can obtain one at https://opensource.org/license/mit/.
 */

#ifndef JAU_MATH_VECBASE_HPP_
#define JAU_MATH_VECBASE_HPP_

#include <cassert>
#include <cmath>
#include <cstdarg>
#include <initializer_list>

#include <jau/float_math.hpp>
#include <jau/type_concepts.hpp>

namespace jau::math {

    /** \addtogroup Math
     *
     *  @{
     */

    /**
     * Generic abstract base vector class as a CRTP via derived type `Self`.
     *
     * Class complies with jau::req::contiguous_container, i.e. `C++ Named Requirement ContiguousContainer` requirements.
     */
    template <jau::req::arithmetic Value_type, class Self, const size_t Components>
    class alignas(sizeof(Value_type)) VectorNT {
      public:
        typedef Value_type               value_type;
        typedef value_type*              pointer;
        typedef const value_type*        const_pointer;
        typedef value_type&              reference;
        typedef const value_type&        const_reference;
        typedef value_type*              iterator;
        typedef const value_type*        const_iterator;

        typedef std::size_t              size_type;
        typedef std::ptrdiff_t           difference_type;

        /** value alignment is sizeof(value_type) */
        constexpr static int value_alignment = sizeof(value_type);

        /** Number of value_type components  */
        constexpr static const size_type components = Components;

        /** Size in bytes with value_alignment */
        constexpr static const size_type byte_size = components * sizeof(value_type);

        constexpr static const value_type zero = value_type(0);
        constexpr static const value_type one = value_type(1);

        value_type x;

      private:
        friend Self;

        constexpr VectorNT() noexcept
        : x(zero) {}

        constexpr VectorNT(const value_type v) noexcept
        : x(v) {}

        constexpr VectorNT(const_iterator v) noexcept
        { set(v, v+components); }

        constexpr VectorNT(const VectorNT& o) noexcept = default;
        constexpr VectorNT(VectorNT&& o) noexcept = default;
        constexpr VectorNT& operator=(const VectorNT&) noexcept = default;
        constexpr VectorNT& operator=(VectorNT&&) noexcept = default;

        constexpr Self& self() noexcept { return *static_cast<Self*>(this); }
        constexpr const Self& self() const noexcept { return *static_cast<const Self*>(this); }

      public:
        constexpr Self copy() const noexcept { return Self(self()); }

        explicit operator const_pointer() const noexcept { return &x; }
        explicit operator pointer() noexcept { return &x; }

        constexpr const_iterator cbegin() const noexcept { return &x; }
        constexpr iterator begin() noexcept { return &x; }

        constexpr const_iterator cend() const noexcept { return cbegin() + components; }
        constexpr iterator end() noexcept { return begin() + components; }

        constexpr pointer data() noexcept { return &x; }
        constexpr const_pointer data() const noexcept { return &x; }

        /// Returns the number of components
        constexpr size_type size() const noexcept { return components; }

        /** Returns read-only component */
        constexpr value_type operator[](size_type i) const noexcept {
            assert(i < components);
            return (&x)[i];
        }

        /** Returns writeable reference to component */
        constexpr reference operator[](size_type i) noexcept {
            assert(i < components);
            return (&x)[i];
        }

        /** this = xyzw, returns this. */
        constexpr Self& set(const_iterator xyzw) noexcept { set(xyzw, xyzw+components); return self(); }

        /** this = { x, y, z }, returns this. */
        constexpr Self& set(std::initializer_list<value_type> v) noexcept {
            return set(v.begin(), v.end());
        }
        /** this = { c.begin() ... c.end() }, returns this. */
        template<typename container_type>
        requires jau::req::contiguous_container<container_type> &&
                 std::convertible_to<typename container_type::value_type, value_type>
        constexpr Self& set(const container_type &c) noexcept
        { return set(c.cbegin(), c.cend()); }
        /** this = { *begin ... *end }, returns this. */
        constexpr Self& set(const_iterator begin, const_iterator end) noexcept {
            pointer d=&x; const_pointer d_end=d+components;
            while (d!=d_end && begin!=end) {
                *d++ = *begin++;
            }
            while (d!=d_end) {
                *d++ = 0; // zero remainder
            }
            return self();
        }
    };

    /**@}*/

}  // namespace jau::math

#endif /*  JAU_MATH_VECBASE_HPP_ */
