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
#ifndef VEC3F_HPP_
#define VEC3F_HPP_

#include <cmath>
#include <cstdarg>
#include <cstdint>
#include <limits>
#include <string>
#include <iostream>

#include <jau/float_math.hpp>
#include <jau/math/vec2f.hpp>

namespace jau::math {

    /** \addtogroup Math
     *
     *  @{
     */

    /**
     * 3D vector using three float components.
     */
    struct Vec3f {
        public:
            float x;
            float y;
            float z;

            constexpr Vec3f() noexcept
            : x(0), y(0), z(0) {}

            constexpr Vec3f(const float x_, const float y_, const float z_) noexcept
            : x(x_), y(y_), z(z_) {}

            constexpr Vec3f(const Vec3f& o) noexcept = default;
            constexpr Vec3f(Vec3f&& o) noexcept = default;
            constexpr Vec3f& operator=(const Vec3f&) noexcept = default;
            constexpr Vec3f& operator=(Vec3f&&) noexcept = default;

            /** Returns read-only component w/o boundary check */
            float operator[](size_t i) const noexcept {
                return reinterpret_cast<const float*>(this)[i];
            }

            /** Returns writeable reference to component w/o boundary check */
            float& operator[](size_t i) noexcept {
                return reinterpret_cast<float*>(this)[i];
            }

            constexpr bool equals(const Vec3f& o, const float epsilon=std::numeric_limits<float>::epsilon()) const noexcept {
                if( this == &o ) {
                    return true;
                } else {
                    return jau::equals(x, o.x, epsilon) &&
                           jau::equals(y, o.y, epsilon) &&
                           jau::equals(z, o.z, epsilon);
                }
            }
            constexpr bool operator==(const Vec3f& rhs) const noexcept {
                return equals(rhs);
            }
            /** TODO
            constexpr bool operator<=>(const vec3f_t& rhs ) const noexcept {
                return ...
            } */

            /** this = { o, z }, returns this. */
            constexpr Vec3f& set(const Vec2f& o, const float z_) noexcept {
                x = o.x;
                y = o.y;
                z = z_;
                return *this;
            }

            constexpr Vec3f& set(const float vx, const float vy, const float vz) noexcept
            { x=vx; y=vy; z=vz; return *this; }

            constexpr void add(const float dx, const float dy, const float dz) noexcept { x+=dx; y+=dy; z+=dz; }

            constexpr Vec3f& operator+=(const Vec3f& rhs ) noexcept {
                x+=rhs.x; y+=rhs.y; z+=rhs.z;
                return *this;
            }

            constexpr Vec3f& operator-=(const Vec3f& rhs ) noexcept {
                x-=rhs.x; y-=rhs.y; z-=rhs.z;
                return *this;
            }

            /**
             * Scale this vector with given scale factor
             * @param s scale factor
             * @return this instance
             */
            constexpr Vec3f& operator*=(const float s ) noexcept {
                x*=s; y*=s; z*=s;
                return *this;
            }

            /**
             * Divide this vector with given scale factor
             * @param s scale factor
             * @return this instance
             */
            constexpr Vec3f& operator/=(const float s ) noexcept {
                x/=s; y/=s; z/=s;
                return *this;
            }

            void rotate(const float radians, const Vec3f& ctr) noexcept {
                // FIXME z, consider using a matrix or quaternions
                const float cos = std::cos(radians);
                const float sin = std::sin(radians);
                const float x0 = x - ctr.x;
                const float y0 = y - ctr.y;
                const float tmp = x0 * cos - y0 * sin + ctr.x;
                              y = x0 * sin + y0 * cos + ctr.y;
                x = tmp;
            }
            constexpr void rotate(const float sin, const float cos, const Vec3f& ctr) noexcept {
                // FIXME z, consider using a matrix or quaternions
                const float x0 = x - ctr.x;
                const float y0 = y - ctr.y;
                const float tmp = x0 * cos - y0 * sin + ctr.x;
                              y = x0 * sin + y0 * cos + ctr.y;
                x = tmp;
            }

            std::string toString() const noexcept { return std::to_string(x)+"/"+std::to_string(y)+"/"+std::to_string(z); }

            constexpr bool is_zero() const noexcept {
                return jau::is_zero(x) && jau::is_zero(y) && jau::is_zero(z);
            }

            /**
             * Return the squared length of a vector, a.k.a the squared <i>norm</i> or squared <i>magnitude</i>
             */
            constexpr float length_sq() const noexcept {
                return x*x + y*y + z*z;
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
            constexpr Vec3f& normalize() noexcept {
                const float lengthSq = length_sq();
                if ( jau::is_zero( lengthSq ) ) {
                    x = 0.0f;
                    y = 0.0f;
                    z = 0.0f;
                } else {
                    const float invSqr = 1.0f / std::sqrt(lengthSq);
                    x *= invSqr;
                    y *= invSqr;
                    z *= invSqr;
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
            constexpr float dist_sq(const Vec3f& o) const noexcept {
                const float dx = x - o.x;
                const float dy = y - o.y;
                const float dz = z - o.z;
                return dx*dx + dy*dy + dz*dz;
            }

            /**
             * Return the distance between this vector and the given one.
             */
            constexpr float dist(const Vec3f& o) const noexcept {
                return std::sqrt(dist_sq(o));
            }

            /**
             * Return the dot product of this vector and the given one
             * @return the dot product as float
             */
            constexpr float dot(const Vec3f& o) const noexcept {
                return x*o.x + y*o.y + z*o.z;
            }

            /**
             * cross product this x b
             * @return new resulting vector
             */
            constexpr Vec3f cross(const Vec3f& b) const noexcept {
                return Vec3f( y * b.z - z * b.y,
                              z * b.x - x * b.z,
                              x * b.y - y * b.y);
            }

            /**
             * cross product this = a x b, with a, b different from this
             * @return this for chaining
             */
            constexpr Vec3f& cross(const Vec3f& a, const Vec3f& b) noexcept {
                x = a.y * b.z - a.z * b.y;
                y = a.z * b.x - a.x * b.z;
                z = a.x * b.y - a.y * b.y;
                return *this;
            }

            /**
             * Return the cosines of the angle between to vectors
             * @param vec1 vector 1
             * @param vec2 vector 2
             */
            constexpr float cos_angle(const Vec3f& o) const noexcept {
                return dot(o) / ( length() * o.length() ) ;
            }

            /**
             * Return the angle between to vectors in radians
             * @param vec1 vector 1
             * @param vec2 vector 2
             */
            float angle(const Vec3f& o) noexcept {
                return std::acos( cos_angle(o) );
            }

            bool intersects(const Vec3f& o) const noexcept {
                const float eps = std::numeric_limits<float>::epsilon();
                if( std::abs(x-o.x) >= eps || std::abs(y-o.y) >= eps || std::abs(z-o.z) >= eps ) {
                    return false;
                }
                return true;
            }
    };
    typedef Vec3f Point3f;

    constexpr Vec3f operator+(const Vec3f& lhs, const Vec3f& rhs ) noexcept {
        return Vec3f(lhs) += rhs;
    }

    constexpr Vec3f operator-(const Vec3f& lhs, const Vec3f& rhs ) noexcept {
        return Vec3f(lhs) -= rhs;
    }

    constexpr Vec3f operator*(const Vec3f& lhs, const float s ) noexcept {
        return Vec3f(lhs) *= s;
    }

    constexpr Vec3f operator*(const float s, const Vec3f& rhs) noexcept {
        return Vec3f(rhs) *= s;
    }

    constexpr Vec3f operator/(const Vec3f& lhs, const float s ) noexcept {
        return Vec3f(lhs) /= s;
    }

    std::ostream& operator<<(std::ostream& out, const Vec3f& v) noexcept {
        return out << v.toString();
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
    struct Ray3f {
        /** Origin of Ray. */
        Point3f orig;

        /** Normalized direction vector of ray. */
        Vec3f dir;

        std::string toString() const noexcept { return "Ray[orig "+orig.toString()+", dir "+dir.toString() +"]"; }
    };

    std::ostream& operator<<(std::ostream& out, const Ray3f& v) noexcept {
        return out << v.toString();
    }

    /**@}*/

} // namespace jau::math

#endif /*  VEC3F_HPP_ */
