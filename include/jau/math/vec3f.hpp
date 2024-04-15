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
#ifndef JAU_VEC3F_HPP_
#define JAU_VEC3F_HPP_

#include <cmath>
#include <cstdarg>
#include <cstdint>
#include <cassert>
#include <limits>
#include <string>
#include <initializer_list>
#include <iostream>

#include <jau/float_math.hpp>
#include <jau/math/vec2f.hpp>

namespace jau::math {

    /** \addtogroup Math
     *
     *  @{
     */

    /**
     * 3D vector using three value_type components.
     */
    template<typename Value_type,
             std::enable_if_t<std::is_floating_point_v<Value_type>, bool> = true>
    class alignas(Value_type) Vector3F {
        public:
            typedef Value_type                  value_type;
            typedef value_type*                 pointer;
            typedef const value_type*           const_pointer;
            typedef value_type&                 reference;
            typedef const value_type&           const_reference;
            typedef value_type*                 iterator;
            typedef const value_type*           const_iterator;

            constexpr static const value_type zero = value_type(0);
            constexpr static const value_type one  = value_type(1);

            value_type x;
            value_type y;
            value_type z;

            constexpr Vector3F() noexcept
            : x(zero), y(zero), z(zero) {}

            constexpr Vector3F(const value_type v) noexcept
            : x(v), y(v), z(v) {}

            constexpr Vector3F(const value_type x_, const value_type y_, const value_type z_) noexcept
            : x(x_), y(y_), z(z_) {}

            constexpr Vector3F(const_iterator v) noexcept
            : x(v[0]), y(v[1]), z(v[2]) {}

            constexpr Vector3F(std::initializer_list<value_type> v) noexcept
            : x(v[0]), y(v[1]), z(v[2]) {}

            constexpr Vector3F(const Vector3F& o) noexcept = default;
            constexpr Vector3F(Vector3F&& o) noexcept = default;
            constexpr Vector3F& operator=(const Vector3F&) noexcept = default;
            constexpr Vector3F& operator=(Vector3F&&) noexcept = default;

            /** Returns read-only component */
            constexpr value_type operator[](size_t i) const noexcept {
                assert(i < 3);
                return (&x)[i];
            }

            /** Returns writeable reference to component  */
            constexpr reference operator[](size_t i) noexcept {
                assert(i < 3);
                return (&x)[i];
            }

            /** xyz = this, returns xyz. */
            constexpr iterator get(iterator xyz) const noexcept {
                xyz[0] = x;
                xyz[1] = y;
                xyz[2] = z;
                return xyz;
            }

            constexpr bool equals(const Vector3F& o, const value_type epsilon=std::numeric_limits<value_type>::epsilon()) const noexcept {
                if( this == &o ) {
                    return true;
                } else {
                    return jau::equals(x, o.x, epsilon) &&
                           jau::equals(y, o.y, epsilon) &&
                           jau::equals(z, o.z, epsilon);
                }
            }
            constexpr bool operator==(const Vector3F& rhs) const noexcept {
                return equals(rhs);
            }
            /** TODO
            constexpr bool operator<=>(const vec3f_t& rhs ) const noexcept {
                return ...
            } */

            /** this = { o, z }, returns this. */
            constexpr Vector3F& set(const Vec2f& o, const value_type z_) noexcept {
                x = o.x;
                y = o.y;
                z = z_;
                return *this;
            }

            constexpr Vector3F& set(const value_type vx, const value_type vy, const value_type vz) noexcept
            { x=vx; y=vy; z=vz; return *this; }

            /** this = xyz, returns this. */
            constexpr Vector3F& set(const_iterator xyz) noexcept
            { x=xyz[0]; y=xyz[1]; z=xyz[2]; return *this; }

            constexpr void add(const value_type dx, const value_type dy, const value_type dz) noexcept { x+=dx; y+=dy; z+=dz; }

            constexpr Vector3F& operator+=(const Vector3F& rhs ) noexcept {
                x+=rhs.x; y+=rhs.y; z+=rhs.z;
                return *this;
            }

            constexpr Vector3F& operator-=(const Vector3F& rhs ) noexcept {
                x-=rhs.x; y-=rhs.y; z-=rhs.z;
                return *this;
            }

            /**
             * Scale this vector with given scale factor
             * @param s scale factor
             * @return this instance
             */
            constexpr Vector3F& operator*=(const value_type s ) noexcept {
                x*=s; y*=s; z*=s;
                return *this;
            }

            /**
             * Divide this vector with given scale factor
             * @param s scale factor
             * @return this instance
             */
            constexpr Vector3F& operator/=(const value_type s ) noexcept {
                x/=s; y/=s; z/=s;
                return *this;
            }

            /** Rotates this vector around the Z-axis in place, returns *this */
            constexpr_cxx26 Vector3F& rotateZ(const value_type radians) noexcept {
                return rotateZ(std::sin(radians), std::cos(radians));
            }

            /** Rotates this vector in place, returns *this */
            constexpr Vector3F& rotateZ(const value_type sin, const value_type cos) noexcept {
                const value_type x0 = x;
                x = x0 * cos - y * sin;
                y = x0 * sin + y * cos;
                return *this;
            }

            std::string toString() const noexcept { return std::to_string(x)+"/"+std::to_string(y)+"/"+std::to_string(z); }

            constexpr bool is_zero() const noexcept {
                return jau::is_zero(x) && jau::is_zero(y) && jau::is_zero(z);
            }

            /**
             * Return the squared length of a vector, a.k.a the squared <i>norm</i> or squared <i>magnitude</i>
             */
            constexpr value_type length_sq() const noexcept {
                return x*x + y*y + z*z;
            }

            /**
             * Return the length of a vector, a.k.a the <i>norm</i> or <i>magnitude</i>
             */
            constexpr value_type length() const noexcept {
                return std::sqrt(length_sq());
            }

            /**
             * Normalize this vector in place
             */
            constexpr Vector3F& normalize() noexcept {
                const value_type lengthSq = length_sq();
                if ( jau::is_zero( lengthSq ) ) {
                    x = zero;
                    y = zero;
                    z = zero;
                } else {
                    const value_type invSqr = one / std::sqrt(lengthSq);
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
            constexpr value_type dist_sq(const Vector3F& o) const noexcept {
                const value_type dx = x - o.x;
                const value_type dy = y - o.y;
                const value_type dz = z - o.z;
                return dx*dx + dy*dy + dz*dz;
            }

            /**
             * Return the distance between this vector and the given one.
             */
            constexpr value_type dist(const Vector3F& o) const noexcept {
                return std::sqrt(dist_sq(o));
            }

            /**
             * Return the dot product of this vector and the given one
             * @return the dot product as value_type
             */
            constexpr value_type dot(const Vector3F& o) const noexcept {
                return x*o.x + y*o.y + z*o.z;
            }

            /**
             * cross product this x b
             * @return new resulting vector
             */
            constexpr Vector3F cross(const Vector3F& b) const noexcept {
                return Vector3F( y * b.z - z * b.y,
                              z * b.x - x * b.z,
                              x * b.y - y * b.y);
            }

            /**
             * cross product this = a x b, with a, b different from this
             * @return this for chaining
             */
            constexpr Vector3F& cross(const Vector3F& a, const Vector3F& b) noexcept {
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
            constexpr value_type cos_angle(const Vector3F& o) const noexcept {
                return dot(o) / ( length() * o.length() ) ;
            }

            /**
             * Return the angle between to vectors in radians
             * @param vec1 vector 1
             * @param vec2 vector 2
             */
            constexpr_cxx26 value_type angle(const Vector3F& o) const noexcept {
                return std::acos( cos_angle(o) );
            }

            constexpr_cxx23 bool intersects(const Vector3F& o) const noexcept {
                const value_type eps = std::numeric_limits<value_type>::epsilon();
                if( std::abs(x-o.x) >= eps || std::abs(y-o.y) >= eps || std::abs(z-o.z) >= eps ) {
                    return false;
                }
                return true;
            }
    };

    template<typename T,
             std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
    constexpr Vector3F<T> operator+(const Vector3F<T>& lhs, const Vector3F<T>& rhs ) noexcept {
        // Returning a Vector3 object from the returned reference of operator+=()
        // may hinder copy-elision or "named return value optimization" (NRVO).
        // return Vector3<T>(lhs) += rhs;

        // Returning named object allows copy-elision (NRVO),
        // only one object is created 'on target'.
        Vector3F<T> r(lhs); r += rhs; return r;
    }

    template<typename T,
             std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
    constexpr Vector3F<T> operator-(const Vector3F<T>& lhs, const Vector3F<T>& rhs ) noexcept {
        Vector3F<T> r(lhs); r -= rhs; return r;
    }

    template<typename T,
             std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
    constexpr Vector3F<T> operator*(const Vector3F<T>& lhs, const T s ) noexcept {
        Vector3F<T> r(lhs); r *= s; return r;
    }

    template<typename T,
             std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
    constexpr Vector3F<T> operator*(const T s, const Vector3F<T>& rhs) noexcept {
        Vector3F<T> r(rhs); r *= s; return r;
    }

    template<typename T,
             std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
    constexpr Vector3F<T> operator/(const Vector3F<T>& lhs, const T s ) noexcept {
        Vector3F<T> r(lhs); r /= s; return r;
    }

    template<typename T,
             std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
    std::ostream& operator<<(std::ostream& out, const Vector3F<T>& v) noexcept {
        return out << v.toString();
    }

    typedef Vector3F<float> Vec3f;
    static_assert(alignof(float) == alignof(Vec3f));

    /**
     * Point3F alias of Vector3F
     */
    template<typename T,
             std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
    using Point3F = Vector3F<T>;

    typedef Point3F<float> Point3f;
    static_assert(alignof(float) == alignof(Point3f));

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
    template<typename T,
             std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
    struct alignas(T) Ray3F {
        /** Origin of Ray. */
        Point3F<T> orig;

        /** Normalized direction vector of ray. */
        Vector3F<T> dir;

        std::string toString() const noexcept { return "Ray[orig "+orig.toString()+", dir "+dir.toString() +"]"; }
    };

    template<typename T,
             std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
    std::ostream& operator<<(std::ostream& out, const Ray3F<T>& v) noexcept {
        return out << v.toString();
    }

    typedef Ray3F<float> Ray3f;
    static_assert(alignof(float) == alignof(Ray3f));

    /**@}*/

} // namespace jau::math

#endif /*  JAU_VEC3F_HPP_ */
