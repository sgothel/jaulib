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

#ifndef JAU_MATH_RECTI2F_HPP_
#define JAU_MATH_RECTI2F_HPP_

#include <iostream>

#include <jau/int_math.hpp>
#include <jau/math/vec2i.hpp>

namespace jau::math {

    /** \addtogroup Math
     *
     *  @{
     */

    /**
     * Rectangle with x, y, width and height value_type components.
     *
     * Component and overall alignment is natural as sizeof(value_type),
     * i.e. sizeof(value_type) == alignof(value_type)
     */
    template<typename Value_type,
             std::enable_if_t<std::is_integral_v<Value_type> &&
                              sizeof(Value_type) == alignof(Value_type), bool> = true>
    class alignas(sizeof(Value_type)) RectI {
      public:
        typedef Value_type                  value_type;
        typedef value_type*                 pointer;
        typedef const value_type*           const_pointer;
        typedef value_type&                 reference;
        typedef const value_type&           const_reference;
        typedef value_type*                 iterator;
        typedef const value_type*           const_iterator;

        /** value alignment is sizeof(value_type) */
        constexpr static int value_alignment = sizeof(value_type);

        /** Number of value_type components  */
        constexpr static const size_t components = 4;

        /** Size in bytes with value_alignment */
        constexpr static const size_t byte_size = components * sizeof(value_type);

      private:
        value_type m_x;
        value_type m_y;
        value_type m_width;
        value_type m_height;

      public:
        constexpr RectI() noexcept
        : m_x(0), m_y(0), m_width(0), m_height(0) {}

        constexpr RectI(const value_type xywh[/*4*/]) noexcept{
            set(xywh);
        }

        constexpr RectI(const Vector2I<value_type>& pos, const Vector2I<value_type>& size) noexcept {
            set(pos, size);
        }

        constexpr RectI(const value_type x, const value_type y, const value_type width, const value_type height) noexcept {
            set(x, y, width, height);
        }

        constexpr RectI(const RectI& o) noexcept = default;
        constexpr RectI(RectI&& o) noexcept = default;
        constexpr RectI& operator=(const RectI&) noexcept = default;
        constexpr RectI& operator=(RectI&&) noexcept = default;

        constexpr bool operator==(const RectI& rhs ) const noexcept {
            if( this == &rhs ) {
                return true;
            }
            return m_x == rhs.m_x && m_y == rhs.m_y &&
                   m_width == rhs.m_width &&
                   m_height == rhs.m_height;
        }
        /** TODO
        constexpr bool operator<=>(const Recti_t& rhs ) const noexcept {
            return ...
        } */

        constexpr RectI& set(const Vector2I<value_type>& pos, const Vector2I<value_type>& size) noexcept
        { m_x = pos.x; m_y = pos.y; m_width = size.x; m_height= size.y; return *this; }

        /** this = { x, y, width, height }, returns this. */
        constexpr RectI& set(const value_type x, const value_type y, const value_type width, const value_type height) noexcept
        { m_x = x; m_y = y; m_width = width; m_height= height; return *this; }

        /** this = xywh, returns this. */
        constexpr RectI& set(const_iterator xywh) noexcept
        { m_x = xywh[0]; m_y = xywh[1]; m_width = xywh[2]; m_height= xywh[3]; return *this; }

        /** xywh = this, returns xywh. */
        iterator get(iterator xywh) const noexcept
        { xywh[0] = m_x; xywh[1] = m_y; xywh[2] = m_width; xywh[3] = m_height; return xywh; }

        constexpr value_type x() const noexcept { return m_x; }
        constexpr value_type y() const noexcept { return m_y; }
        constexpr value_type width() const noexcept { return m_width; }
        constexpr value_type height() const noexcept { return m_height; }
        constexpr Vector2I<value_type> getPosition() const noexcept { return Vector2I<value_type>(m_x, m_y); }
        constexpr Vector2I<value_type> getSize() const noexcept { return Vector2I<value_type>(m_width, m_height); }

        constexpr void setX(const value_type x) noexcept { m_x = x; }
        constexpr void setY(const value_type y) noexcept { m_y = y; }
        constexpr void setWidth(const value_type width) noexcept { m_width = width; }
        constexpr void setHeight(const value_type height) noexcept { m_height = height; }
        constexpr void setPosition(const Vector2I<value_type>& pos) noexcept { m_x = pos.x; m_y = pos.y; }
        constexpr void setSize(const Vector2I<value_type>& size) noexcept { m_width = size.x; m_height = size.y; }

        /** Return true if area is zero. */
        constexpr bool is_zero() const noexcept {
            return 0 == m_width || 0 == m_height;
        }

        std::string toString() const noexcept
        { return std::to_string(m_x)+"/"+std::to_string(m_y)+" "+std::to_string(m_width)+"x"+std::to_string(m_height); }
    };

    template<typename T,
             std::enable_if_t<std::numeric_limits<T>::is_integer, bool> = true>
    std::ostream& operator<<(std::ostream& out, const RectI<T>& v) noexcept {
        return out << v.toString();
    }

    typedef RectI<int> Recti;
    static_assert(4 == Recti::components);
    static_assert(sizeof(int) == Recti::value_alignment);
    static_assert(sizeof(int) == alignof(Recti));
    static_assert(sizeof(int)*4 == sizeof(Recti));

/**@}*/

} // namespace jau::math

#endif /*  JAU_MATH_RECTI2F_HPP_ */

