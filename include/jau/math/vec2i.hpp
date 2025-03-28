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

#ifndef JAU_MATH_VEC2I_HPP_
#define JAU_MATH_VEC2I_HPP_

#include <cmath>
#include <cstdarg>
#include <cassert>
#include <limits>
#include <string>
#include <algorithm>
#include <iostream>

#include <jau/float_math.hpp>

namespace jau::math {

    /** \addtogroup Math
     *
     *  @{
     */

    /**
     * 2D vector using two value_type components.
     *
     * Component and overall alignment is natural as sizeof(value_type),
     * i.e. sizeof(value_type) == alignof(value_type)
     */
    template<typename Value_type,
             std::enable_if_t<std::is_integral_v<Value_type> &&
                              sizeof(Value_type) == alignof(Value_type), bool> = true>
    class alignas(sizeof(Value_type)) Vector2I {
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
        constexpr static const size_t components = 2;

        /** Size in bytes with value_alignment */
        constexpr static const size_t byte_size = components * sizeof(value_type);

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

        constexpr Vector2I copy() noexcept { return Vector2I(*this); }

        /** Returns read-only component */
        constexpr value_type operator[](size_t i) const noexcept {
            assert(i < 2);
            return (&x)[i];
        }

        explicit operator const_pointer() const noexcept { return &x; }
        constexpr const_iterator cbegin() const noexcept { return &x; }

        /** Returns writeable reference to component */
        constexpr reference operator[](size_t i) noexcept {
            assert(i < 2);
            return (&x)[i];
        }

        explicit operator pointer() noexcept { return &x; }
        constexpr iterator begin() noexcept { return &x; }

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

        /** this = this + {d.x, d.y}, component wise. Returns this. */
        constexpr Vector2I& add(const Vector2I& d) noexcept
        { x+=d.x; y+=d.y; return *this; }

        /** this = this + {sx, sy}, component wise. Returns this. */
        constexpr Vector2I& add(const value_type dx, const value_type dy) noexcept
        { x+=dx; y+=dy; return *this; }

        /** this = this * {s.x, s.y}, component wise. Returns this. */
        constexpr Vector2I& mul(const Vector2I& s) noexcept
        { x*=s.x; y*=s.y; return *this; }

        /** this = this * {sx, sy}, component wise. Returns this. */
        constexpr Vector2I& mul(const value_type sx, const value_type sy) noexcept
        { x*=sx; y*=sy; return *this; }

        /** this = this * s, component wise. Returns this. */
        constexpr Vector2I& scale(const value_type s) noexcept
        { x*=s; y*=s; return *this; }

        /** this = this + rhs, component wise. Returns this. */
        constexpr Vector2I& operator+=(const Vector2I& rhs ) noexcept {
            x+=rhs.x; y+=rhs.y;
            return *this;
        }

        /** this = this - rhs, component wise. Returns this. */
        constexpr Vector2I& operator-=(const Vector2I& rhs ) noexcept {
            x-=rhs.x; y-=rhs.y;
            return *this;
        }

        /**
         * this = this * {s.x, s.y}, component wise.
         * @param s scale factor
         * @return this instance
         */
        constexpr Vector2I& operator*=(const Vector2I& s) noexcept {
            x*=s.x; y*=s.y;
            return *this;
        }
        /**
         * this = this / {s.x, s.y}, component wise.
         * @param s scale factor
         * @return this instance
         */
        constexpr Vector2I& operator/=(const Vector2I& s) noexcept {
            x/=s.x; y/=s.y;
            return *this;
        }

        /**
         * this = this * s, component wise.
         * @param s scale factor
         * @return this instance
         */
        constexpr Vector2I& operator*=(const value_type s ) noexcept {
            x*=s; y*=s;
            return *this;
        }

        /**
         * this = this * s, component wise.
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

        std::string toString() const noexcept { return std::to_string(x)+", "+std::to_string(y); }

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
    constexpr Vector2I<T> operator-(const Vector2I<T>& lhs) noexcept {
        Vector2I<T> r(lhs);
        r *= -1;
        return r;
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
             std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
    constexpr Vector2I<T> operator/(const T s, const Vector2I<T>& rhs) noexcept {
        Vector2I<T> r(rhs);
        r.x=s/r.x; r.y=s/r.y;
        return r;
    }

    template<typename T,
             std::enable_if_t<std::numeric_limits<T>::is_integer, bool> = true>
    constexpr Vector2I<T> min(const Vector2I<T>& lhs, const Vector2I<T>& rhs) noexcept {
        return Vector2I<T>(std::min(lhs.x, rhs.x), std::min(lhs.y, rhs.y));
    }

    template<typename T,
             std::enable_if_t<std::numeric_limits<T>::is_integer, bool> = true>
    constexpr Vector2I<T> max(const Vector2I<T>& lhs, const Vector2I<T>& rhs) noexcept {
        return Vector2I<T>(std::max(lhs.x, rhs.x), std::max(lhs.y, rhs.y));
    }

    template<typename T,
             std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
    constexpr Vector2I<T> abs(const Vector2I<T>& lhs) noexcept {
        return Vector2I<T>(std::abs(lhs.x), std::abs(lhs.y));
    }

    template<typename T,
             std::enable_if_t<std::numeric_limits<T>::is_integer, bool> = true>
    std::ostream& operator<<(std::ostream& out, const Vector2I<T>& v) noexcept {
        return out << v.toString();
    }

    typedef Vector2I<int> Vec2i;
    static_assert(2 == Vec2i::components);
    static_assert(sizeof(int) == Vec2i::value_alignment);
    static_assert(sizeof(int) == alignof(Vec2i));
    static_assert(sizeof(int)*2 == Vec2i::byte_size);
    static_assert(sizeof(int)*2 == sizeof(Vec2i));

    /**
     * Point2I alias of Vector2I
     */
    template<typename Value_type,
             std::enable_if_t<std::numeric_limits<Value_type>::is_integer &&
                              sizeof(Value_type) == alignof(Value_type), bool> = true>
    using Point2I = Vector2I<Value_type>;

    typedef Point2I<int> Point2i;
    static_assert(2 == Point2i::components);
    static_assert(sizeof(int) == Point2i::value_alignment);
    static_assert(sizeof(int) == alignof(Point2i));
    static_assert(sizeof(int)*2 == Point2i::byte_size);
    static_assert(sizeof(int)*2 == sizeof(Point2i));

    /**@}*/

} // namespace jau::math

#endif /*  JAU_MATH_VEC2I_HPP_ */

