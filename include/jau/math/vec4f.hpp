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

#ifndef JAU_MATH_VEC4F_HPP_
#define JAU_MATH_VEC4F_HPP_

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdarg>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <string>

#include <jau/float_math.hpp>
#include <jau/math/vecbase.hpp>
#include <jau/math/vec3f.hpp>
#include <jau/type_concepts.hpp>

namespace jau::math {

    /** \addtogroup Math
     *
     *  @{
     */

    /**
     * 4D vector using four floating point value_type components.
     *
     * Class complies with jau::req::contiguous_container, i.e. `C++ Named Requirement ContiguousContainer` requirements.
     *
     * Component and overall alignment is natural as sizeof(value_type),
     * i.e. sizeof(value_type) == alignof(value_type)
     */
    template <jau::req::packed_floating_point Value_type>
    class alignas(sizeof(Value_type)) Vector4F : public VectorNT<Value_type, Vector4F<Value_type>, 4>
    {
      public:
        using VectorBase = VectorNT<Value_type, Vector4F<Value_type>, 4>;
        using typename VectorBase::value_type;
        using typename VectorBase::iterator;
        using typename VectorBase::const_iterator;
        using VectorBase::components;
        using VectorBase::zero;
        using VectorBase::one;
        using VectorBase::x;
        using VectorBase::set;

        typedef Vector3F<value_type> Vec3;

        value_type y;
        value_type z;
        value_type w;

        constexpr Vector4F() noexcept
        : VectorBase(zero), y(zero), z(zero), w(zero) {}

        constexpr Vector4F(const value_type v) noexcept
        : VectorBase(v), y(v), z(v), w(v) {}

        constexpr Vector4F(const value_type x_, const value_type y_, const value_type z_, const value_type w_) noexcept
        : VectorBase(x_), y(y_), z(z_), w(w_) {}

        constexpr Vector4F(const Vec3& o3, const value_type w_) noexcept
        : VectorBase(o3.x), y(o3.y), z(o3.z), w(w_) {}

        constexpr Vector4F(const_iterator v) noexcept
        : VectorBase(v) {}

        template<typename container_type>
        requires jau::req::contiguous_container<container_type> &&
                 std::convertible_to<typename container_type::value_type, value_type>
        constexpr Vector4F(const container_type &c) noexcept { VectorBase::set(c.begin(), c.end()); }

        constexpr Vector4F(std::initializer_list<value_type> v) noexcept { VectorBase::set(v.begin(), v.end()); }

        constexpr Vector4F(const Vector4F& o) noexcept = default;
        constexpr Vector4F(Vector4F&& o) noexcept = default;
        constexpr Vector4F& operator=(const Vector4F&) noexcept = default;
        constexpr Vector4F& operator=(Vector4F&&) noexcept = default;

        /** xyzw = this, returns xyzw. */
        constexpr iterator get(iterator xyzw) const noexcept
        { xyzw[0] = x; xyzw[1] = y; xyzw[2] = z; xyzw[3] = w; return xyzw; }

        /** out = { this.x, this.y, this.z } dropping w, returns out. */
        constexpr Vec3& getVec3(Vec3& out) const noexcept
        { out.x = x; out.y = y; out.z = z; return out; }

        constexpr bool operator==(const Vector4F& rhs) const noexcept {
            if (this == &rhs) {
                return true;
            }
            return jau::is_zero(x - rhs.x) && jau::is_zero(y - rhs.y) &&
                   jau::is_zero(z - rhs.z) && jau::is_zero(w - rhs.w);
        }
        /** TODO
        constexpr std::strong_ordering operator<=>(const vec4f_t& rhs) const noexcept {
            return ...
        } */

        /** this = { o, w }, returns this. */
        constexpr Vector4F& set(const Vec3f& o, const value_type w_) noexcept
        { x = o.x; y = o.y; z = o.z; w = w_; return *this; }

        constexpr Vector4F& set(const value_type vx, const value_type vy, const value_type vz, const value_type vw) noexcept
        { x=vx; y=vy; z=vz; w=vw; return *this; }

        /** this = this + {d.x, d.y, d.z, d.w}, component wise. Returns this. */
        constexpr Vector4F& add(const Vector4F& d) noexcept
        { x+=d.x; y+=d.y; z+=d.z; w+=d.w; return *this; }

        /** this = this + {dx, dy, dz, dw}, component wise. Returns this. */
        constexpr Vector4F& add(const value_type dx, const value_type dy, const value_type dz, const value_type dw) noexcept
        { x+=dx; y+=dy; z+=dz; w+=dw; return *this; }

        /** this = this * {s.x, s.y, s.z}, component wise. Returns this. */
        constexpr Vector4F& mul(const Vector4F& s) noexcept
        { x*=s.x; y*=s.y; z*=s.z; w*=s.w; return *this; }

        /** this = this * {sx, sy, sz, sw}, component wise. Returns this. */
        constexpr Vector4F& mul(const value_type sx, const value_type sy, const value_type sz, const value_type sw) noexcept
        { x*=sx; y*=sy; z*=sz; w*=sw; return *this; }

        /** this = this * s, component wise. Returns this. */
        constexpr Vector4F& scale(const value_type s) noexcept
        { x*=s; y*=s; z*=s; w*=s; return *this; }

        /** this = this + rhs, component wise. Returns this. */
        constexpr Vector4F& operator+=(const Vector4F& rhs ) noexcept {
            x+=rhs.x; y+=rhs.y; z+=rhs.z; w+=rhs.w;
            return *this;
        }

        /** this = this - rhs, component wise. Returns this. */
        constexpr Vector4F& operator-=(const Vector4F& rhs ) noexcept
        { x-=rhs.x; y-=rhs.y; z-=rhs.z; w-=rhs.w; return *this;
        }

        /**
         * this = this * {s.x, s.y, s.z, s.w}, component wise.
         * @param s scale factor
         * @return this instance
         */
        constexpr Vector4F& operator*=(const Vector4F& s) noexcept {
            x*=s.x; y*=s.y; z*=s.z; w*=s.w;
            return *this;
        }
        /**
         * this = this / {s.x, s.y, s.z, s.w}, component wise.
         * @param s scale factor
         * @return this instance
         */
        constexpr Vector4F& operator/=(const Vector4F& s) noexcept {
            x/=s.x; y/=s.y; z/=s.z; w/=s.w;
            return *this;
        }

        /**
         * this = this * s, component wise.
         * @param s scale factor
         * @return this instance
         */
        constexpr Vector4F& operator*=(const value_type s ) noexcept
        { x*=s; y*=s; z*=s; w*=s; return *this; }

        /**
         * this = this / s, component wise.
         * @param s scale factor
         * @return this instance
         */
        constexpr Vector4F& operator/=(const value_type s ) noexcept
        { x/=s; y/=s; z/=s; w/=s; return *this; }

        std::string toString() const noexcept { return std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ", " + std::to_string(w); }

        /// Returns true if all component's absolute values are less than epsilon
        constexpr bool is_zero() const noexcept {
            return jau::is_zero(x) && jau::is_zero(y) && jau::is_zero(z) && jau::is_zero(w);
        }

        /// Returns true if all components are positive or zero
        constexpr bool is_positive() const noexcept {
            return jau::is_positive(x) && jau::is_positive(y) && jau::is_positive(z) && jau::is_positive(w);
        }

        /**
         * Return the squared length of a vector, a.k.a the squared <i>norm</i> or squared <i>magnitude</i>
         */
        constexpr value_type length_sq() const noexcept {
            return x*x + y*y + z*z + w*w;
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
        constexpr Vector4F& normalize() noexcept {
            const value_type lengthSq = length_sq();
            if (jau::is_zero(lengthSq)) {
                x = zero;
                y = zero;
                z = zero;
                w = zero;
            } else {
                const value_type invSqr = one / std::sqrt(lengthSq);
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
        constexpr value_type dist_sq(const Vector4F& o) const noexcept {
            const value_type dx = x - o.x;
            const value_type dy = y - o.y;
            const value_type dz = z - o.z;
            const value_type dw = w - o.w;
            return dx*dx + dy*dy + dz*dz + dw*dw;
        }

        /**
         * Return the distance between this vector and the given one.
         */
        constexpr value_type dist(const Vector4F& o) const noexcept {
            return std::sqrt(dist_sq(o));
        }

        constexpr_cxx23 bool intersects(const Vector4F& o) const noexcept {
            const value_type eps = std::numeric_limits<value_type>::epsilon();
            if (std::abs(x - o.x) >= eps || std::abs(y - o.y) >= eps ||
                std::abs(z - o.z) >= eps || std::abs(w - o.w) >= eps) {
                return false;
            }
            return true;
        }
    };

    template <jau::req::packed_floating_point T>
    constexpr Vector4F<T> operator+(const Vector4F<T>& lhs, const Vector4F<T>& rhs) noexcept {
        // Returning a Vector4 object from the returned reference of operator+=()
        // may hinder copy-elision or "named return value optimization" (NRVO).
        // return Vector4<T>(lhs) += rhs;

        // Returning named object allows copy-elision (NRVO),
        // only one object is created 'on target'.
        Vector4F<T> r(lhs); r += rhs; return r;
    }

    template <jau::req::packed_floating_point T>
    constexpr Vector4F<T> operator-(const Vector4F<T>& lhs, const Vector4F<T>& rhs ) noexcept {
        Vector4F<T> r(lhs); r -= rhs; return r;
    }

    template <jau::req::packed_floating_point T>
    constexpr Vector4F<T> operator-(const Vector4F<T>& lhs) noexcept {
        Vector4F<T> r(lhs);
        r *= -1;
        return r;
    }

    template <jau::req::packed_floating_point T>
    constexpr Vector4F<T> operator*(const Vector4F<T>& lhs, const T s ) noexcept {
        Vector4F<T> r(lhs); r *= s; return r;
    }

    template <jau::req::packed_floating_point T>
    constexpr Vector4F<T> operator*(const T s, const Vector4F<T>& rhs) noexcept {
        Vector4F<T> r(rhs); r *= s; return r;
    }

    template <jau::req::packed_floating_point T>
    constexpr Vector4F<T> operator/(const Vector4F<T>& lhs, const T s ) noexcept {
        Vector4F<T> r(lhs); r /= s; return r;
    }

    template <jau::req::packed_floating_point T>
    constexpr Vector4F<T> operator/(const T s, const Vector4F<T>& rhs) noexcept {
        Vector4F<T> r(rhs);
        r.x=s/r.x; r.y=s/r.y; r.z=s/r.z; r.w=s/r.w;
        return r;
    }

    /// Returns the component-wise minimum of given vectors
    template <jau::req::packed_floating_point T>
    constexpr Vector4F<T> min(const Vector4F<T>& lhs, const Vector4F<T>& rhs) noexcept {
        return Vector4F<T>(std::min(lhs.x, rhs.x), std::min(lhs.y, rhs.y), std::min(lhs.z, rhs.z), std::min(lhs.w, rhs.w));
    }
    /// Returns the component-wise minimum of given vector and scalar
    template<jau::req::packed_floating_point T>
    constexpr Vector4F<T> min(const Vector4F<T>& lhs, const T rhs) noexcept {
        return Vector4F<T>(std::min(lhs.x, rhs), std::min(lhs.y, rhs), std::min(lhs.z, rhs), std::min(lhs.w, rhs));
    }

    /// Returns the component-wise maximum of given vectors
    template <jau::req::packed_floating_point T>
    constexpr Vector4F<T> max(const Vector4F<T>& lhs, const Vector4F<T>& rhs) noexcept {
        return Vector4F<T>(std::max(lhs.x, rhs.x), std::max(lhs.y, rhs.y), std::max(lhs.z, rhs.z), std::max(lhs.w, rhs.w));
    }
    /// Returns the component-wise maximum of given vector and scalar
    template<jau::req::packed_floating_point T>
    constexpr Vector4F<T> max(const Vector4F<T>& lhs, const T rhs) noexcept {
        return Vector4F<T>(std::max(lhs.x, rhs), std::max(lhs.y, rhs), std::max(lhs.z, rhs), std::max(lhs.w, rhs));
    }

    /// Returns the component-wise clamped-value of given value-vector to the min_val and max_val vector
    template<jau::req::packed_floating_point T>
    constexpr Vector4F<T> clamp(const Vector4F<T>& v, const Vector4F<T>& min_val, const Vector4F<T>& max_val) noexcept {
        return Vector4F<T>(std::min(std::max(v.x, min_val.x), max_val.x),
                           std::min(std::max(v.y, min_val.y), max_val.y),
                           std::min(std::max(v.z, min_val.z), max_val.z),
                           std::min(std::max(v.w, min_val.w), max_val.w));
    }
    /// Returns the component-wise clamped-value of given value-vector to the min_val and max_val scalar
    template<jau::req::packed_floating_point T>
    constexpr Vector4F<T> clamp(const Vector4F<T>& v, const T min_val, const T max_val) noexcept {
        return Vector4F<T>(std::min(std::max(v.x, min_val), max_val),
                           std::min(std::max(v.y, min_val), max_val),
                           std::min(std::max(v.z, min_val), max_val),
                           std::min(std::max(v.w, min_val), max_val));
    }

    /// Returns the component-wise absolute values of given vector
    template <jau::req::packed_floating_point T>
    constexpr Vector4F<T> abs(const Vector4F<T>& lhs) noexcept {
        return Vector4F<T>(std::abs(lhs.x), std::abs(lhs.y), std::abs(lhs.z), std::abs(lhs.w));
    }

    /** out = { this.x, this.y, this.z } dropping w, returns out. */
    template <jau::req::packed_floating_point T>
    constexpr Vector3F<T> to_vec3(const Vector4F<T>& v) noexcept {
        Vector3F<T> r; v.getVec3(r); return r;
    }

    template <jau::req::packed_floating_point T>
    std::ostream& operator<<(std::ostream& out, const Vector4F<T>& v) noexcept {
        return out << v.toString();
    }

    static_assert(4 == Vector4F<double>::components);
    static_assert(sizeof(double) == Vector4F<double>::value_alignment);
    static_assert(sizeof(double) == alignof(Vector4F<double>));
    static_assert(sizeof(double) * 4 == Vector4F<double>::byte_size);
    static_assert(sizeof(double) * 4 == sizeof(Vector4F<double>));

    typedef Vector4F<float> Vec4f;
    static_assert(true == jau::req::contiguous_container<Vec4f>);
    static_assert(4 == Vec4f::components);
    static_assert(sizeof(float) == Vec4f::value_alignment);
    static_assert(sizeof(float) == alignof(Vec4f));
    static_assert(sizeof(float) * 4 == Vec4f::byte_size);
    static_assert(sizeof(float) * 4 == sizeof(Vec4f));

    /**
     * Point4F alias of Vector4F
     */
    template <jau::req::packed_floating_point T>
    using Point4F = Vector4F<T>;

    typedef Point4F<float> Point4f;
    static_assert(true == jau::req::contiguous_container<Point4f>);
    static_assert(4 == Point4f::components);
    static_assert(sizeof(float) == Point4f::value_alignment);
    static_assert(sizeof(float) == alignof(Point4f));
    static_assert(sizeof(float) * 4 == Point4f::byte_size);
    static_assert(sizeof(float) * 4 == sizeof(Point4f));

    /**@}*/

}  // namespace jau::math

#endif /*  JAU_MATH_VEC4F_HPP_ */
