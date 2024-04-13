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
#ifndef VEC4F_HPP_
#define VEC4F_HPP_

#include <cmath>
#include <cstdarg>
#include <cstdint>
#include <limits>
#include <string>

#include <jau/float_math.hpp>
#include <jau/math/vec3f.hpp>

namespace jau::math {

    /** \addtogroup Math
     *
     *  @{
     */

    /**
     * 4D vector using four float components.
     */
    class Vec4f {
        public:
            float x;
            float y;
            float z;
            float w;

            constexpr Vec4f() noexcept
            : x(0), y(0), z(0), w(0) {}

            constexpr Vec4f(const float x_, const float y_, const float z_, const float w_) noexcept
            : x(x_), y(y_), z(z_), w(w_) {}

            constexpr Vec4f(const Vec4f& o) noexcept = default;
            constexpr Vec4f(Vec4f&& o) noexcept = default;
            constexpr Vec4f& operator=(const Vec4f&) noexcept = default;
            constexpr Vec4f& operator=(Vec4f&&) noexcept = default;

            /** Returns read-only component w/o boundary check */
            float operator[](size_t i) const noexcept {
                return reinterpret_cast<const float*>(this)[i];
            }

            /** Returns writeable reference to component w/o boundary check */
            float& operator[](size_t i) noexcept {
                return reinterpret_cast<float*>(this)[i];
            }

            constexpr bool operator==(const Vec4f& rhs ) const noexcept {
                if( this == &rhs ) {
                    return true;
                }
                return jau::is_zero(x - rhs.x) && jau::is_zero(y - rhs.y) &&
                       jau::is_zero(z - rhs.z) && jau::is_zero(w - rhs.w);
            }
            /** TODO
            constexpr bool operator<=>(const vec4f_t& rhs ) const noexcept {
                return ...
            } */

            /** this = { o, w }, returns this. */
            constexpr Vec4f& set(const Vec3f& o, const float w_) noexcept {
                x = o.x;
                y = o.y;
                z = o.z;
                w = w_;
                return *this;
            }

            constexpr Vec4f& set(const float vx, const float vy, const float vz, const float vw) noexcept
            { x=vx; y=vy; z=vz; w=vw; return *this; }

            constexpr void add(const float dx, const float dy, const float dz, const float dw) noexcept {
                x+=dx; y+=dy; z+=dz; w+=dw;
            }

            constexpr Vec4f& operator+=(const Vec4f& rhs ) noexcept {
                x+=rhs.x; y+=rhs.y; z+=rhs.z; w+=rhs.w;
                return *this;
            }

            constexpr Vec4f& operator-=(const Vec4f& rhs ) noexcept {
                x-=rhs.x; y-=rhs.y; z-=rhs.z; w-=rhs.w;
                return *this;
            }

            /**
             * Scale this vector with given scale factor
             * @param s scale factor
             * @return this instance
             */
            constexpr Vec4f& operator*=(const float s ) noexcept {
                x*=s; y*=s; z*=s; w*=s;
                return *this;
            }

            /**
             * Divide this vector with given scale factor
             * @param s scale factor
             * @return this instance
             */
            constexpr Vec4f& operator/=(const float s ) noexcept {
                x/=s; y/=s; z/=s; w/=s;
                return *this;
            }

            std::string toString() const noexcept { return std::to_string(x)+"/"+std::to_string(y)+"/"+std::to_string(z)+"/"+std::to_string(w); }

            constexpr bool is_zero() const noexcept {
                return jau::is_zero(x) && jau::is_zero(y) && jau::is_zero(z) && jau::is_zero(w);
            }

            /**
             * Return the squared length of a vector, a.k.a the squared <i>norm</i> or squared <i>magnitude</i>
             */
            constexpr float length_sq() const noexcept {
                return x*x + y*y + z*z + w*w;
            }

            /**
             * Return the length of a vector, a.k.a the <i>norm</i> or <i>magnitude</i>
             */
            constexpr float length() const noexcept {
                return std::sqrt(length_sq());
            }

            /**
             * Normalize this vector in place
             */
            constexpr Vec4f& normalize() noexcept {
                const float lengthSq = length_sq();
                if ( jau::is_zero( lengthSq ) ) {
                    x = 0.0f;
                    y = 0.0f;
                    z = 0.0f;
                    w = 0.0f;
                } else {
                    const float invSqr = 1.0f / std::sqrt(lengthSq);
                    x *= invSqr;
                    y *= invSqr;
                    z *= invSqr;
                    w *= invSqr;
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
            constexpr float dist_sq(const Vec4f& o) const noexcept {
                const float dx = x - o.x;
                const float dy = y - o.y;
                const float dz = z - o.z;
                const float dw = w - o.w;
                return dx*dx + dy*dy + dz*dz + dw*dw;
            }

            /**
             * Return the distance between this vector and the given one.
             */
            constexpr float dist(const Vec4f& o) const noexcept {
                return std::sqrt(dist_sq(o));
            }

            bool intersects(const Vec4f& o) const noexcept {
                const float eps = std::numeric_limits<float>::epsilon();
                if( std::abs(x-o.x) >= eps || std::abs(y-o.y) >= eps ||
                    std::abs(z-o.z) >= eps || std::abs(w-o.w) >= eps ) {
                    return false;
                }
                return true;
            }
    };
    typedef Vec4f Point4f;

    constexpr Vec4f operator+(const Vec4f& lhs, const Vec4f& rhs ) noexcept {
        return Vec4f(lhs) += rhs;
    }

    constexpr Vec4f operator-(const Vec4f& lhs, const Vec4f& rhs ) noexcept {
        return Vec4f(lhs) -= rhs;
    }

    constexpr Vec4f operator*(const Vec4f& lhs, const float s ) noexcept {
        return Vec4f(lhs) *= s;
    }

    constexpr Vec4f operator*(const float s, const Vec4f& rhs) noexcept {
        return Vec4f(rhs) *= s;
    }

    constexpr Vec4f operator/(const Vec4f& lhs, const float s ) noexcept {
        return Vec4f(lhs) /= s;
    }

    /**@}*/

} // namespace jau::math

#endif /*  VEC4F_HPP_ */
