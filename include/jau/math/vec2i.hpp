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
#ifndef JAU_VEC2I_HPP_
#define JAU_VEC2I_HPP_

#include <cmath>
#include <cstdarg>
#include <cstdint>
#include <limits>
#include <string>
#include <iostream>

#include <jau/float_math.hpp>

namespace jau::math {

    /** \addtogroup Math
     *
     *  @{
     */

    /**
     * 2D vector using two integer components.
     */
    struct Vec2i {
        int x;
        int y;

        constexpr Vec2i() noexcept
        : x(0), y(0) {}

        constexpr Vec2i(const int x_, const int y_) noexcept
        : x(x_), y(y_) {}

        constexpr Vec2i(const Vec2i& o) noexcept = default;
        constexpr Vec2i(Vec2i&& o) noexcept = default;
        constexpr Vec2i& operator=(const Vec2i&) noexcept = default;
        constexpr Vec2i& operator=(Vec2i&&) noexcept = default;

        /** Returns read-only component w/o boundary check */
        int operator[](size_t i) const noexcept {
            return reinterpret_cast<const int*>(this)[i];
        }

        /** Returns writeable reference to component w/o boundary check */
        int& operator[](size_t i) noexcept {
            return reinterpret_cast<int*>(this)[i];
        }

        constexpr bool operator==(const Vec2i& rhs ) const noexcept {
            if( this == &rhs ) {
                return true;
            }
            return x == rhs.x && y == rhs.y;
        }
        /** TODO
        constexpr bool operator<=>(const vec2i_t& rhs ) const noexcept {
            return ...
        } */

        constexpr Vec2i& set(const int vx, const int vy) noexcept
        { x=vx; y=vy; return *this; }

        constexpr Vec2i& add(const int dx, const int dy) noexcept
        { x+=dx; y+=dy; return *this; }

        constexpr Vec2i& operator+=(const Vec2i& rhs ) noexcept {
            x+=rhs.x; y+=rhs.y;
            return *this;
        }

        constexpr Vec2i& operator-=(const Vec2i& rhs ) noexcept {
            x-=rhs.x; y-=rhs.y;
            return *this;
        }

        /**
         * Scale this vector with given scale factor
         * @param s scale factor
         * @return this instance
         */
        constexpr Vec2i& operator*=(const int s ) noexcept {
            x*=s; y*=s;
            return *this;
        }

        /**
         * Divide this vector with given scale factor
         * @param s scale factor
         * @return this instance
         */
        constexpr Vec2i& operator/=(const int s ) noexcept {
            x/=s; y/=s;
            return *this;
        }

        void rotate(const float radians, const Vec2i& ctr) {
            const float cos = std::cos(radians);
            const float sin = std::sin(radians);
            rotate(sin, cos, ctr);
        }
        void rotate(const float sin, const float cos, const Vec2i& ctr) {
            const float x0 = (float)(x - ctr.x);
            const float y0 = (float)(y - ctr.y);
            const int tmp = jau::round_to_int( x0 * cos - y0 * sin ) + ctr.x;
                        y = jau::round_to_int( x0 * sin + y0 * cos ) + ctr.y;
            x = tmp;
        }

        std::string toString() const noexcept { return std::to_string(x)+"/"+std::to_string(y); }

        bool intersects(const Vec2i& o) {
            return x == o.x && y == o.y;
        }
    };
    typedef Vec2i Point2i;

    constexpr Vec2i operator+(const Vec2i& lhs, const Vec2i& rhs ) noexcept {
        return Vec2i(lhs) += rhs;
    }

    constexpr Vec2i operator-(const Vec2i& lhs, const Vec2i& rhs ) noexcept {
        return Vec2i(lhs) -= rhs;
    }

    constexpr Vec2i operator*(const Vec2i& lhs, const float s ) noexcept {
        return Vec2i(lhs) *= s;
    }

    constexpr Vec2i operator*(const float s, const Vec2i& rhs) noexcept {
        return Vec2i(rhs) *= s;
    }

    constexpr Vec2i operator/(const Vec2i& lhs, const float s ) noexcept {
        return Vec2i(lhs) /= s;
    }

    std::ostream& operator<<(std::ostream& out, const Vec2i& v) noexcept {
        return out << v.toString();
    }

    /**@}*/

} // namespace jau::math

#endif /*  JAU_VEC2I_HPP_ */

