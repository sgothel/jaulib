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
#ifndef VEC2F_HPP_
#define VEC2F_HPP_

#include <cmath>
#include <cstdarg>
#include <cstdint>
#include <limits>
#include <string>

#include <jau/float_math.hpp>

namespace jau::math {

    /** \addtogroup Math
     *
     *  @{
     */

    /**
     * 2D vector using two float components.
     */
    class Vec2f {
    public:
        float x;
        float y;

        static constexpr Vec2f from_length_angle(const float magnitude, const float radians) noexcept {
            return Vec2f(magnitude * std::cos(radians), magnitude * std::sin(radians));
        }

        constexpr Vec2f() noexcept
        : x(0), y(0) {}

        constexpr Vec2f(const float x_, const float y_) noexcept
        : x(x_), y(y_) {}

        constexpr Vec2f(const Vec2f& o) noexcept = default;
        constexpr Vec2f(Vec2f&& o) noexcept = default;
        constexpr Vec2f& operator=(const Vec2f&) noexcept = default;
        constexpr Vec2f& operator=(Vec2f&&) noexcept = default;

        /** Returns read-only component w/o boundary check */
        float operator[](size_t i) const noexcept {
            return reinterpret_cast<const float*>(this)[i];
        }

        /** Returns writeable reference to component w/o boundary check */
        float& operator[](size_t i) noexcept {
            return reinterpret_cast<float*>(this)[i];
        }

        constexpr bool operator==(const Vec2f& rhs ) const noexcept {
            if( this == &rhs ) {
                return true;
            }
            return jau::is_zero(x - rhs.x) && jau::is_zero(y - rhs.y);
        }
        /** TODO
        constexpr bool operator<=>(const vec2f_t& rhs ) const noexcept {
            return ...
        } */

        constexpr Vec2f& set(const float vx, const float vy) noexcept
        { x=vx; y=vy; return *this; }

        constexpr Vec2f& add(const float dx, const float dy) noexcept
        { x+=dx; y+=dy; return *this; }

        constexpr Vec2f& operator+=(const Vec2f& rhs ) noexcept {
            x+=rhs.x; y+=rhs.y;
            return *this;
        }

        constexpr Vec2f& operator-=(const Vec2f& rhs ) noexcept {
            x-=rhs.x; y-=rhs.y;
            return *this;
        }

        /**
         * Scale this vector with given scale factor
         * @param s scale factor
         * @return this instance
         */
        constexpr Vec2f& operator*=(const float s ) noexcept {
            x*=s; y*=s;
            return *this;
        }

        /**
         * Divide this vector with given scale factor
         * @param s scale factor
         * @return this instance
         */
        constexpr Vec2f& operator/=(const float s ) noexcept {
            x/=s; y/=s;
            return *this;
        }

        /** Rotates this vector in place, returns *this */
        Vec2f& rotate(const float radians, const Vec2f& ctr) noexcept {
            return rotate(std::sin(radians), std::cos(radians), ctr);
        }

        /** Rotates this vector in place, returns *this */
        constexpr Vec2f& rotate(const float sin, const float cos, const Vec2f& ctr) noexcept {
            const float x0 = x - ctr.x;
            const float y0 = y - ctr.y;
            x = x0 * cos - y0 * sin + ctr.x;
            y = x0 * sin + y0 * cos + ctr.y;
            return *this;
        }

        /** Rotates this vector in place, returns *this */
        Vec2f& rotate(const float radians) noexcept {
            return rotate(std::sin(radians), std::cos(radians));
        }

        /** Rotates this vector in place, returns *this */
        constexpr Vec2f& rotate(const float sin, const float cos) noexcept {
            const float x0 = x;
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
        constexpr float length_sq() const noexcept {
            return x*x + y*y;
        }

        /**
         * Return the length of this vector, a.k.a the <i>norm</i> or <i>magnitude</i>
         */
        constexpr float length() const noexcept {
            return std::sqrt(length_sq());
        }

        /**
         * Return the direction angle of this vector in radians
         */
        float angle() const noexcept {
            // Utilize atan2 taking y=sin(a) and x=cos(a), resulting in proper direction angle for all quadrants.
            return std::atan2( y, x );
        }

        /** Normalize this vector in place, returns *this */
        constexpr Vec2f& normalize() noexcept {
            const float lengthSq = length_sq();
            if ( jau::is_zero( lengthSq ) ) {
                x = 0.0f;
                y = 0.0f;
            } else {
                const float invSqr = 1.0f / std::sqrt(lengthSq);
                x *= invSqr;
                y *= invSqr;
            }
            return *this;
        }

        /**
         * Return the squared distance between this vector and the given one.
         * <p>
         * When comparing the relative distance between two points it is usually sufficient to compare the squared
         * distances, thus avoiding an expensive square root operation.
         * </p>
         */
        constexpr float dist_sq(const Vec2f& o) const noexcept {
            const float dx = x - o.x;
            const float dy = y - o.y;
            return dx*dx + dy*dy;
        }

        /**
         * Return the distance between this vector and the given one.
         */
        constexpr float dist(const Vec2f& o) const noexcept {
            return std::sqrt(dist_sq(o));
        }

        /**
         * Return the dot product of this vector and the given one
         * @return the dot product as float
         */
        constexpr float dot(const Vec2f& o) const noexcept {
            return x*o.x + y*o.y;
        }

        /**
         * Returns cross product of this vectors and the given one, i.e. *this x o.
         *
         * The 2D cross product is identical with the 2D perp dot product.
         *
         * @return the resulting scalar
         */
        constexpr float cross(const Vec2f& o) const noexcept {
            return x * o.y - y * o.x;
        }

        /**
         * Return the cosines of the angle between two vectors
         */
        constexpr float cos_angle(const Vec2f& o) const noexcept {
            return dot(o) / ( length() * o.length() ) ;
        }

        /**
         * Return the angle between two vectors in radians
         */
        float angle(const Vec2f& o) const noexcept {
            return std::acos( cos_angle(o) );
        }

        /**
         * Return the counter-clock-wise (CCW) normal of this vector, i.e. perp(endicular) vector
         */
        Vec2f normal_ccw() const noexcept {
            return Vec2f(-y, x);
        }

        bool intersects(const Vec2f& o) const noexcept {
            const float eps = std::numeric_limits<float>::epsilon();
            if( std::abs(x-o.x) >= eps || std::abs(y-o.y) >= eps ) {
                return false;
            }
            return true;
        }
    };
    typedef Vec2f Point2f;

    constexpr Vec2f operator+(const Vec2f& lhs, const Vec2f& rhs ) noexcept {
        return Vec2f(lhs) += rhs;
    }

    constexpr Vec2f operator-(const Vec2f& lhs, const Vec2f& rhs ) noexcept {
        return Vec2f(lhs) -= rhs;
    }

    constexpr Vec2f operator*(const Vec2f& lhs, const float s ) noexcept {
        return Vec2f(lhs) *= s;
    }

    constexpr Vec2f operator*(const float s, const Vec2f& rhs) noexcept {
        return Vec2f(rhs) *= s;
    }

    constexpr Vec2f operator/(const Vec2f& lhs, const float s ) noexcept {
        return Vec2f(lhs) /= s;
    }

    /**
     * Simple compound denoting a ray.
     * <p>
     * A ray, also known as a half line, consists out of it's <i>origin</i>
     * and <i>direction</i>. Hence it is bound to only the <i>origin</i> side,
     * where the other end is +infinitive.
     * <pre>
     * R(t) = R0 + Rd * t with R0 origin, Rd direction and t > 0.0
     * </pre>
     * </p>
     */
    class Ray2f {
    public:
        /** Origin of Ray. */
        Point2f orig;

        /** Normalized direction vector of ray. */
        Vec2f dir;

        std::string toString() const noexcept { return "Ray[orig "+orig.toString()+", dir "+dir.toString() +"]"; }
    };

    /**@}*/

} // namespace jau::math

#endif /*  VEC2F_HPP_ */
