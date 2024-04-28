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
#include <cassert>
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
    template<typename Value_type,
             std::enable_if_t<std::is_integral_v<Value_type>, bool> = true>
    struct alignas(Value_type) Vector2I {
        typedef Value_type                  value_type;
        typedef value_type*                 pointer;
        typedef const value_type*           const_pointer;
        typedef value_type&                 reference;
        typedef const value_type&           const_reference;
        typedef value_type*                 iterator;
        typedef const value_type*           const_iterator;

        typedef typename jau::float_bytes<sizeof(value_type)>::type float_type;

        constexpr static const value_type zero = value_type(0);
        constexpr static const value_type one  = value_type(1);

        value_type x;
        value_type y;

        constexpr Vector2I() noexcept
        : x(zero), y(zero) {}

        constexpr Vector2I(const value_type v) noexcept
        : x(v), y(v) {}

        constexpr Vector2I(const value_type x_, const value_type y_) noexcept
        : x(x_), y(y_) {}

        constexpr Vector2I(const Vector2I& o) noexcept = default;
        constexpr Vector2I(Vector2I&& o) noexcept = default;
        constexpr Vector2I& operator=(const Vector2I&) noexcept = default;
        constexpr Vector2I& operator=(Vector2I&&) noexcept = default;

        /** Returns read-only component */
        constexpr value_type operator[](size_t i) const noexcept {
            assert(i < 2);
            return (&x)[i];
        }

        /** Returns writeable reference to component */
        constexpr reference operator[](size_t i) noexcept {
            assert(i < 2);
            return (&x)[i];
        }

        /** xy = this, returns xy. */
        constexpr iterator get(iterator xy) const noexcept {
            xy[0] = x;
            xy[1] = y;
            return xy;
        }

        constexpr bool operator==(const Vector2I& rhs ) const noexcept {
            if( this == &rhs ) {
                return true;
            }
            return x == rhs.x && y == rhs.y;
        }
        /** TODO
        constexpr bool operator<=>(const vec2i_t& rhs ) const noexcept {
            return ...
        } */

        constexpr Vector2I& set(const value_type vx, const value_type vy) noexcept
        { x=vx; y=vy; return *this; }

        /** this = xy, returns this. */
        constexpr Vector2I& set(const_iterator xy) noexcept
        { x=xy[0]; y=xy[1]; return *this; }

        /** this = this + {sx, sy}, returns this. */
        constexpr Vector2I& add(const value_type dx, const value_type dy) noexcept
        { x+=dx; y+=dy; return *this; }

        /** this = this * {sx, sy}, returns this. */
        constexpr Vector2I& mul(const value_type sx, const value_type sy) noexcept
        { x*=sx; y*=sy; return *this; }

        /** this = this * s, returns this. */
        constexpr Vector2I& scale(const value_type s) noexcept
        { x*=s; y*=s; return *this; }

        /** this = this + rhs, returns this. */
        constexpr Vector2I& operator+=(const Vector2I& rhs ) noexcept {
            x+=rhs.x; y+=rhs.y;
            return *this;
        }

        /** this = this - rhs, returns this. */
        constexpr Vector2I& operator-=(const Vector2I& rhs ) noexcept {
            x-=rhs.x; y-=rhs.y;
            return *this;
        }

        /**
         * Scale this vector with given scale factor
         * @param s scale factor
         * @return this instance
         */
        constexpr Vector2I& operator*=(const value_type s ) noexcept {
            x*=s; y*=s;
            return *this;
        }

        /**
         * Divide this vector with given scale factor
         * @param s scale factor
         * @return this instance
         */
        constexpr Vector2I& operator/=(const value_type s ) noexcept {
            x/=s; y/=s;
            return *this;
        }

        constexpr_cxx26 void rotate(const float_type radians, const Vector2I& ctr) {
            const float_type cos = std::cos(radians);
            const float_type sin = std::sin(radians);
            rotate(sin, cos, ctr);
        }
        void rotate(const float_type sin, const float_type cos, const Vector2I& ctr) {
            const float_type x0 = static_cast<float_type>(x - ctr.x);
            const float_type y0 = static_cast<float_type>(y - ctr.y);
            const value_type tmp = jau::round_to_int<float_type>( x0 * cos - y0 * sin ) + ctr.x;
                         y = jau::round_to_int<float_type>( x0 * sin + y0 * cos ) + ctr.y;
            x = tmp;
        }

        std::string toString() const noexcept { return std::to_string(x)+" / "+std::to_string(y); }

        constexpr bool is_zero() const noexcept {
            return jau::is_zero(x) && jau::is_zero(y);
        }

        /**
         * Return the squared length of this vector, a.k.a the squared <i>norm</i> or squared <i>magnitude</i>
         */
        constexpr value_type length_sq() const noexcept {
            return x*x + y*y;
        }

        /**
         * Return the length of this vector, a.k.a the <i>norm</i> or <i>magnitude</i>
         */
        constexpr value_type length() const noexcept {
            // return jau::round_to_int<float_type>( std::sqrt<float_type>( length_sq() ) );
            return jau::round_to_int<float_type>( std::sqrt( static_cast<float_type> ( length_sq() )) );
        }

        /** Normalize this vector in place, returns *this */
        constexpr Vector2I& normalize() noexcept {
            const value_type lengthSq = length_sq();
            if ( jau::is_zero( lengthSq ) ) {
                x = zero;
                y = zero;
            } else {
                const float_type invSqr = one / static_cast<float_type>( std::sqrt(lengthSq) );
                x = jau::round_to_int<float_type>( x * invSqr );
                y = jau::round_to_int<float_type>( x * invSqr );
            }
            return *this;
        }


        bool intersects(const Vector2I& o) {
            return x == o.x && y == o.y;
        }
    };

    template<typename T,
             std::enable_if_t<std::numeric_limits<T>::is_integer, bool> = true>
    constexpr Vector2I<T> operator+(const Vector2I<T>& lhs, const Vector2I<T>& rhs ) noexcept {
        // Returning a Vector2I<T> object from the returned reference of operator+=()
        // may hinder copy-elision or "named return value optimization" (NRVO).
        // return Vector2I<T>(lhs) += rhs;

        // Returning named object allows copy-elision (NRVO),
        // only one object is created 'on target'.
        Vector2I<T> r(lhs); r += rhs; return r;
    }

    template<typename T,
             std::enable_if_t<std::numeric_limits<T>::is_integer, bool> = true>
    constexpr Vector2I<T> operator-(const Vector2I<T>& lhs, const Vector2I<T>& rhs ) noexcept {
        Vector2I<T> r(lhs); r -= rhs; return r;
    }

    template<typename T,
             std::enable_if_t<std::numeric_limits<T>::is_integer, bool> = true>
    constexpr Vector2I<T> operator*(const Vector2I<T>& lhs, const float s ) noexcept {
        Vector2I<T> r(lhs); r *= s; return r;
    }

    template<typename T,
             std::enable_if_t<std::numeric_limits<T>::is_integer, bool> = true>
    constexpr Vector2I<T> operator*(const float s, const Vector2I<T>& rhs) noexcept {
        Vector2I<T> r(rhs); r *= s; return r;
    }

    template<typename T,
             std::enable_if_t<std::numeric_limits<T>::is_integer, bool> = true>
    constexpr Vector2I<T> operator/(const Vector2I<T>& lhs, const float s ) noexcept {
        Vector2I<T> r(lhs); r /= s; return r;
    }

    template<typename T,
             std::enable_if_t<std::numeric_limits<T>::is_integer, bool> = true>
    std::ostream& operator<<(std::ostream& out, const Vector2I<T>& v) noexcept {
        return out << v.toString();
    }

    typedef Vector2I<int> Vec2i;
    static_assert(alignof(int) == alignof(Vec2i));
    static_assert(sizeof(int)*2 == sizeof(Vec2i));

    /**@}*/

} // namespace jau::math

#endif /*  JAU_VEC2I_HPP_ */

