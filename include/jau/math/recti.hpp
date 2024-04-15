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
    class Recti {
      private:
        int m_x;
        int m_y;
        int m_width;
        int m_height;

      public:
        constexpr Recti() noexcept
        : m_x(0), m_y(0), m_width(0), m_height(0) {}

        constexpr Recti(const int xywh[/*4*/]) noexcept{
            set(xywh);
        }

        constexpr Recti(const int x, const int y, const int width, const int height) noexcept {
            set(x, y, width, height);
        }

        constexpr Recti(const Recti& o) noexcept = default;
        constexpr Recti(Recti&& o) noexcept = default;
        constexpr Recti& operator=(const Recti&) noexcept = default;
        constexpr Recti& operator=(Recti&&) noexcept = default;

        constexpr bool operator==(const Recti& rhs ) const noexcept {
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
        constexpr Recti& set(const int x, const int y, const int width, const int height) noexcept {
            m_x = x;
            m_y = y;
            m_width = width;
            m_height= height;
            return *this;
        }

        /** this = xywh, returns this. */
        constexpr Recti& set(const int xywh[/*4*/]) noexcept {
            m_x = xywh[0];
            m_y = xywh[1];
            m_width = xywh[2];
            m_height= xywh[3];
            return *this;
        }

        /** xywh = this, returns xywh. */
        int* get(int xywh[/*4*/]) const noexcept {
            xywh[0] = m_x;
            xywh[1] = m_y;
            xywh[2] = m_width;
            xywh[3] = m_height;
            return xywh;
        }

        int x() const noexcept { return m_x; }
        int y() const noexcept { return m_y; }
        int width() const noexcept { return m_width; }
        int height() const noexcept { return m_height; }

        void setX(const int x) noexcept { m_x = x; }
        void setY(const int y) noexcept { m_y = y; }
        void setWidth(const int width) noexcept { m_width = width; }
        void setHeight(const int height) noexcept { m_height = height; }

        /** Return true if area is zero. */
        bool is_zero() const noexcept {
            return 0 == m_width || 0 == m_height;
        }

        std::string toString() const noexcept
        { return std::to_string(m_x)+"/"+std::to_string(m_y)+" "+std::to_string(m_width)+"x"+std::to_string(m_height); }
    };

    std::ostream& operator<<(std::ostream& out, const Recti& v) noexcept {
        return out << v.toString();
    }

/**@}*/

} // namespace jau::math

#endif /*  JAU_RECTI2F_HPP_ */

