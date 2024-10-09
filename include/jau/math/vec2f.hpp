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
#ifndef JAU_VEC2F_HPP_
#define JAU_VEC2F_HPP_

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
     * 2D vector using two value_type components.
     *
     * Component and overall alignment is natural as sizeof(value_type),
     * i.e. sizeof(value_type) == alignof(value_type)
     */
    template<typename Value_type,
             std::enable_if_t<std::is_floating_point_v<Value_type> &&
                              sizeof(Value_type) == alignof(Value_type), bool> = true>
    class alignas(sizeof(Value_type)) Vector2F {
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

        constexpr static const value_type zero = value_type(0);
        constexpr static const value_type one = value_type(1);

        value_type x;
        value_type y;

        static constexpr_cxx26 Vector2F from_length_angle(const value_type magnitude, const value_type radians) noexcept {
            return Vector2F(magnitude * std::cos(radians), magnitude * std::sin(radians));
        }

        constexpr Vector2F() noexcept
        : x(zero), y(zero) {}

        constexpr Vector2F(const value_type v) noexcept
        : x(v), y(v) {}

        constexpr Vector2F(const value_type x_, const value_type y_) noexcept
        : x(x_), y(y_) {}

        constexpr Vector2F(const Vector2F& o) noexcept = default;
        constexpr Vector2F(Vector2F&& o) noexcept = default;
        constexpr Vector2F& operator=(const Vector2F&) noexcept = default;
        constexpr Vector2F& operator=(Vector2F&&) noexcept = default;

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

        constexpr bool operator==(const Vector2F& rhs ) const noexcept {
            if( this == &rhs ) {
                return true;
            }
            return jau::is_zero(x - rhs.x) && jau::is_zero(y - rhs.y);
        }
        /** TODO
        constexpr bool operator<=>(const vec2f_t& rhs ) const noexcept {
            return ...
        } */

        constexpr Vector2F& set(const value_type vx, const value_type vy) noexcept
        { x=vx; y=vy; return *this; }

        /** this = xy, returns this. */
        constexpr Vector2F& set(const_iterator xy) noexcept
        { x=xy[0]; y=xy[1]; return *this; }

        /** this = this + {sx, sy}, returns this. */
        constexpr Vector2F& add(const value_type dx, const value_type dy) noexcept
        { x+=dx; y+=dy; return *this; }

        /** this = this * {sx, sy}, returns this. */
        constexpr Vector2F& mul(const value_type sx, const value_type sy) noexcept
        { x*=sx; y*=sy; return *this; }

        /** this = this * s, returns this. */
        constexpr Vector2F& scale(const value_type s) noexcept
        { x*=s; y*=s; return *this; }

        /** this = this + rhs, returns this. */
        constexpr Vector2F& operator+=(const Vector2F& rhs ) noexcept {
            x+=rhs.x; y+=rhs.y;
            return *this;
        }

        /** this = this - rhs, returns this. */
        constexpr Vector2F& operator-=(const Vector2F& rhs ) noexcept {
            x-=rhs.x; y-=rhs.y;
            return *this;
        }

        /**
         * Scale this vector with given scale factor
         * @param s scale factor
         * @return this instance
         */
        constexpr Vector2F& operator*=(const value_type s ) noexcept {
            x*=s; y*=s;
            return *this;
        }

        /**
         * Divide this vector with given scale factor
         * @param s scale factor
         * @return this instance
         */
        constexpr Vector2F& operator/=(const value_type s ) noexcept {
            x/=s; y/=s;
            return *this;
        }

        /** Rotates this vector in place, returns *this */
        constexpr_cxx26 Vector2F& rotate(const value_type radians, const Vector2F& ctr) noexcept {
            return rotate(std::sin(radians), std::cos(radians), ctr);
        }

        /** Rotates this vector in place, returns *this */
        constexpr Vector2F& rotate(const value_type sin, const value_type cos, const Vector2F& ctr) noexcept {
            const value_type x0 = x - ctr.x;
            const value_type y0 = y - ctr.y;
            x = x0 * cos - y0 * sin + ctr.x;
            y = x0 * sin + y0 * cos + ctr.y;
            return *this;
        }

        /** Rotates this vector in place, returns *this */
        constexpr_cxx26 Vector2F& rotate(const value_type radians) noexcept {
            return rotate(std::sin(radians), std::cos(radians));
        }

        /** Rotates this vector in place, returns *this */
        constexpr Vector2F& rotate(const value_type sin, const value_type cos) noexcept {
            const value_type x0 = x;
            x = x0 * cos - y * sin;
            y = x0 * sin + y * cos;
            return *this;
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
            return std::sqrt(length_sq());
        }

        /** Normalize this vector in place, returns *this */
        constexpr Vector2F& normalize() noexcept {
            const value_type lengthSq = length_sq();
            if ( jau::is_zero( lengthSq ) ) {
                x = zero;
                y = zero;
            } else {
                const value_type invSqr = one / std::sqrt(lengthSq);
                x *= invSqr;
                y *= invSqr;
            }
            return *this;
        }

        /**
         * Return the direction angle of this vector in radians
         */
        constexpr_cxx26 value_type angle() const noexcept {
            // Utilize atan2 taking y=sin(a) and x=cos(a), resulting in proper direction angle for all quadrants.
            return std::atan2( y, x );
        }

        /**
         * Return the squared distance between this vector and the given one.
         * <p>
         * When comparing the relative distance between two points it is usually sufficient to compare the squared
         * distances, thus avoiding an expensive square root operation.
         * </p>
         */
        constexpr value_type dist_sq(const Vector2F& o) const noexcept {
            const value_type dx = x - o.x;
            const value_type dy = y - o.y;
            return dx*dx + dy*dy;
        }

        /**
         * Return the distance between this vector and the given one.
         */
        constexpr value_type dist(const Vector2F& o) const noexcept {
            return std::sqrt(dist_sq(o));
        }

        /**
         * Return the dot product of this vector and the given one
         * @return the dot product as value_type
         */
        constexpr value_type dot(const Vector2F& o) const noexcept {
            return x*o.x + y*o.y;
        }

        /**
         * Returns cross product of this vectors and the given one, i.e. *this x o.
         *
         * The 2D cross product is identical with the 2D perp dot product.
         *
         * @return the resulting scalar
         */
        constexpr value_type cross(const Vector2F& o) const noexcept {
            return x * o.y - y * o.x;
        }

        /**
         * Return the cosines of the angle between two vectors
         */
        constexpr value_type cos_angle(const Vector2F& o) const noexcept {
            return dot(o) / ( length() * o.length() ) ;
        }

        /**
         * Return the angle between two vectors in radians
         */
        constexpr_cxx26 value_type angle(const Vector2F& o) const noexcept {
            return std::acos( cos_angle(o) );
        }

        /**
         * Return the counter-clock-wise (CCW) normal of this vector, i.e. perp(endicular) vector
         */
        constexpr Vector2F normal_ccw() const noexcept {
            return Vector2F(-y, x);
        }

        constexpr_cxx23 bool intersects(const Vector2F& o) const noexcept {
            const value_type eps = std::numeric_limits<value_type>::epsilon();
            if( std::abs(x-o.x) >= eps || std::abs(y-o.y) >= eps ) {
                return false;
            }
            return true;
        }
    };

    template<typename T,
             std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
    constexpr Vector2F<T> operator+(const Vector2F<T>& lhs, const Vector2F<T>& rhs ) noexcept {
        // Returning a Vector2F<T> object from the returned reference of operator+=()
        // may hinder copy-elision or "named return value optimization" (NRVO).
        // return Vector2F<T>(lhs) += rhs;

        // Returning named object allows copy-elision (NRVO),
        // only one object is created 'on target'.
        Vector2F<T> r(lhs); r += rhs; return r;
    }

    template<typename T,
             std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
    constexpr Vector2F<T> operator-(const Vector2F<T>& lhs, const Vector2F<T>& rhs ) noexcept {
        Vector2F<T> r(lhs); r -= rhs; return r;
    }

    template<typename T,
             std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
    constexpr Vector2F<T> operator*(const Vector2F<T>& lhs, const T s ) noexcept {
        Vector2F<T> r(lhs); r *= s; return r;
    }

    template<typename T,
             std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
    constexpr Vector2F<T> operator*(const T s, const Vector2F<T>& rhs) noexcept {
        Vector2F<T> r(rhs); r *= s; return r;
    }

    template<typename T,
             std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
    constexpr Vector2F<T> operator/(const Vector2F<T>& lhs, const T s ) noexcept {
        Vector2F<T> r(lhs); r /= s; return r;
    }

    template<typename T,
             std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
    std::ostream& operator<<(std::ostream& out, const Vector2F<T>& v) noexcept {
        return out << v.toString();
    }

    static_assert(sizeof(float) == alignof(float)); // natural alignment (reconsider otherwise)

    typedef Vector2F<float> Vec2f;
    static_assert(2 == Vec2f::components);
    static_assert(sizeof(float) == Vec2f::value_alignment);
    static_assert(sizeof(float) == alignof(Vec2f));
    static_assert(sizeof(float)*2 == Vec2f::byte_size);
    static_assert(sizeof(float)*2 == sizeof(Vec2f));

    /**
     * Point2F alias of Vector2F
     */
    template<typename Value_type,
             std::enable_if_t<std::is_floating_point_v<Value_type> &&
                              sizeof(Value_type) == alignof(Value_type), bool> = true>
    using Point2F = Vector2F<Value_type>;

    typedef Point2F<float> Point2f;
    static_assert(2 == Point2f::components);
    static_assert(sizeof(float) == Point2f::value_alignment);
    static_assert(sizeof(float) == alignof(Point2f));
    static_assert(sizeof(float)*2 == Point2f::byte_size);
    static_assert(sizeof(float)*2 == sizeof(Point2f));

    /**
     * Simple compound denoting a ray.
     *
     * Component and overall alignment is as sizeof(value_type), i.e. packed.
     *
     * A ray, also known as a half line, consists out of it's <i>origin</i>
     * and <i>direction</i>. Hence it is bound to only the <i>origin</i> side,
     * where the other end is +infinitive.
     * <pre>
     * R(t) = R0 + Rd * t with R0 origin, Rd direction and t > 0.0
     * </pre>
     */
    template<typename Value_type,
             std::enable_if_t<std::is_floating_point_v<Value_type> &&
                              sizeof(Value_type) == alignof(Value_type), bool> = true>
    class alignas(sizeof(Value_type)) Ray2F {
      public:
        typedef Value_type                  value_type;
        typedef value_type*                 pointer;
        typedef const value_type*           const_pointer;

        /** value alignment is sizeof(value_type) */
        constexpr static int value_alignment = sizeof(value_type);

        /** Number of value_type components  */
        constexpr static const size_t components = 4;

        /** Size in bytes with value_alignment */
        constexpr static const size_t byte_size = components * sizeof(value_type);

        /** Origin of Ray. */
        alignas(value_alignment) Point2F<value_type> orig;

        /** Normalized direction vector of ray. */
        alignas(value_alignment) Vector2F<value_type> dir;

        std::string toString() const noexcept { return "Ray[orig "+orig.toString()+", dir "+dir.toString() +"]"; }
    };

    template<typename T,
             std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
    std::ostream& operator<<(std::ostream& out, const Ray2F<T>& v) noexcept {
        return out << v.toString();
    }

    typedef Ray2F<float> Ray2f;
    static_assert(4 == Ray2f::components);
    static_assert(sizeof(float) == Ray2f::value_alignment);
    static_assert(sizeof(float) == alignof(Ray2f));
    static_assert(sizeof(float)*4 == Ray2f::byte_size);
    static_assert(sizeof(float)*4 == sizeof(Ray2f));

    /**@}*/

} // namespace jau::math

#endif /*  JAU_VEC2F_HPP_ */
