/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2022-2024 Gothel Software e.K.
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
#ifndef JAU_RECTI2F_HPP_
#define JAU_RECTI2F_HPP_

#include <iostream>

#include <jau/int_math.hpp>

namespace jau::math {

    /** \addtogroup Math
     *
     *  @{
     */

    /**
     * Rectangle with x, y, width and height integer components.
     */
    template<typename Value_type,
             std::enable_if_t<std::is_integral_v<Value_type>, bool> = true>
    class RectI {
      public:
        typedef Value_type                  value_type;
        typedef value_type*                 pointer;
        typedef const value_type*           const_pointer;
        typedef value_type&                 reference;
        typedef const value_type&           const_reference;
        typedef value_type*                 iterator;
        typedef const value_type*           const_iterator;

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

        constexpr void setX(const value_type x) noexcept { m_x = x; }
        constexpr void setY(const value_type y) noexcept { m_y = y; }
        constexpr void setWidth(const value_type width) noexcept { m_width = width; }
        constexpr void setHeight(const value_type height) noexcept { m_height = height; }

        /** Return true if area is zero. */
        constexpr bool is_zero() const noexcept {
            return 0 == m_width || 0 == m_height;
        }

        std::string toString() const noexcept
        { return std::to_string(m_x)+" / "+std::to_string(m_y)+" "+std::to_string(m_width)+" x "+std::to_string(m_height); }
    };

    template<typename T,
             std::enable_if_t<std::numeric_limits<T>::is_integer, bool> = true>
    std::ostream& operator<<(std::ostream& out, const RectI<T>& v) noexcept {
        return out << v.toString();
    }

    typedef RectI<int> Recti;
    static_assert(alignof(int) == alignof(Recti));
    static_assert(sizeof(int)*4 == sizeof(Recti));

/**@}*/

} // namespace jau::math

#endif /*  JAU_RECTI2F_HPP_ */

