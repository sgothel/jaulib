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

#ifndef JAU_MATH_QUATERNION_HPP_
#define JAU_MATH_QUATERNION_HPP_

#include <cmath>
#include <concepts>
#include <cstdarg>
#include <iostream>
#include <limits>
#include <string>

#include <jau/float_math.hpp>
#include <jau/math/mat4f.hpp>
#include <jau/math/vec3f.hpp>
#include <jau/type_concepts.hpp>

namespace jau::math {

    /** \addtogroup Math
     *
     *  @{
     */

/**
 * Quaternion implementation supporting
 * <a href="http://web.archive.org/web/20041029003853/http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q34">Gimbal-Lock</a> free rotations.
 * <p>
 * All matrix operation provided are in column-major order,
 * as specified in the OpenGL fixed function pipeline, i.e. compatibility profile.
 * See {@link FloatUtil}.
 * </p>
 * <p>
 * See <a href="http://web.archive.org/web/20041029003853/http://www.j3d.org/matrix_faq/matrfaq_latest.html">Matrix-FAQ</a>
 * </p>
 * <p>
 * See <a href="http://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/index.htm">euclideanspace.com-Quaternion</a>
 * </p>
 */
template<std::floating_point Value_type>
class alignas(Value_type) Quaternion {
  public:
    typedef Value_type               value_type;
    typedef value_type*              pointer;
    typedef const value_type*        const_pointer;
    typedef value_type&              reference;
    typedef const value_type&        const_reference;
    typedef value_type*              iterator;
    typedef const value_type*        const_iterator;

    typedef Vector3F<value_type> Vec3;
    typedef Matrix4<value_type> Mat4;

    constexpr static const value_type zero = value_type(0);
    constexpr static const value_type one  = value_type(1);
    constexpr static const value_type two  = value_type(2);
    constexpr static const value_type half = one/two;

    /**
     * Quaternion Epsilon, used with equals method to determine if two Quaternions are close enough to be considered equal.
     * <p>
     * Using {@value}, which is ~8.4 times `std::numeric_limits<value_type>::epsilon()`.
     * </p>
     */
    constexpr static const value_type allowed_deviation = value_type(8.4) * std::numeric_limits<value_type>::epsilon();  // 8.4 * EPSILON(1.1920929E-7f) = 1.0E-6f; double deviation: 1.0E-8f

  private:
    value_type m_x, m_y, m_z, m_w;

  public:

    constexpr Quaternion() noexcept
    : m_x(0), m_y(0), m_z(0), m_w(1) {}

    constexpr Quaternion(const value_type x, const value_type y, const value_type z, const value_type w) noexcept
    : m_x(x), m_y(y), m_z(z), m_w(w) {}

    constexpr Quaternion(const Quaternion& o) noexcept = default;
    constexpr Quaternion(Quaternion&& o) noexcept = default;
    constexpr Quaternion& operator=(const Quaternion&) noexcept = default;
    constexpr Quaternion& operator=(Quaternion&&) noexcept = default;

    /**
     * See {@link #magnitude()} for special handling of {@link FloatUtil#EPSILON epsilon},
     * which is not applied here.
     * @return the squared magnitude of this quaternion.
     */
    constexpr value_type magnitudeSquared() const noexcept {
        return m_w*m_w + m_x*m_x + m_y*m_y + m_z*m_z;
    }

    /**
     * Return the magnitude of this quaternion, i.e. sqrt({@link #magnitudeSquared()})
     * <p>
     * A magnitude of zero shall equal {@link #isIdentity() identity},
     * as performed by {@link #normalize()}.
     * </p>
     * <p>
     * Implementation Details:
     * <ul>
     *   <li> returns 0f if {@link #magnitudeSquared()} is {@link jau::is_zero(value_type, value_type) is zero} using {@link FloatUtil#EPSILON epsilon}</li>
     *   <li> returns 1f if {@link #magnitudeSquared()} is {@link FloatUtil#isEqual(value_type, value_type, value_type) equals 1f} using {@link FloatUtil#EPSILON epsilon}</li>
     * </ul>
     * </p>
     */
    constexpr_cxx26 value_type magnitude() const noexcept {
        const value_type magnitudeSQ = magnitudeSquared();
        if ( jau::is_zero(magnitudeSQ) ) {
            return zero;
        }
        if ( jau::equals(one, magnitudeSQ) ) {
            return one;
        }
        return std::sqrt(magnitudeSQ);
    }

    constexpr value_type w() const noexcept { return m_w; }

    constexpr void setW(value_type w) noexcept { m_w = w; }

    constexpr value_type x() const noexcept { return m_x; }

    constexpr void setX(value_type x) noexcept { m_x = x; }

    constexpr value_type y() const noexcept { return m_y; }

    constexpr void setY(value_type y) noexcept { m_y = y; }

    constexpr value_type z() const noexcept { return m_z; }

    constexpr void setZ(value_type z) noexcept { m_z = z; }

    /**
     * Returns the dot product of this quaternion with the given x,y,z and m_w components.
     */
    constexpr value_type dot(value_type x, value_type y, value_type z, value_type w) const noexcept {
        return m_x * x + m_y * y + m_z * z + m_w * w;
    }

    /**
     * Returns the dot product of this quaternion with the given quaternion
     */
    constexpr value_type dot(const Quaternion& quat) const noexcept {
        return dot(quat.x(), quat.y(), quat.z(), quat.w());
    }

    /**
     * Returns <code>true</code> if this quaternion has identity.
     * <p>
     * Implementation uses epsilon to compare
     * {@link #w() W} {@link jau::equals() against 1f} and
     * {@link #x() X}, {@link #y() Y} and {@link #z() Z}
     * {@link jau::is_zero3f() against zero}.
     * </p>
     */
    constexpr bool isIdentity() const noexcept {
        return jau::equals(one, m_w) && jau::is_zero3f(m_x, m_y, m_z);
        // return m_w == 1f && m_x == 0f && m_y == 0f && m_z == 0f;
    }

    /***
     * Set this quaternion to identity (x=0,y=0,z=0,w=1)
     * @return this quaternion for chaining.
     */
    constexpr Quaternion& setIdentity() noexcept {
        m_x = m_y = m_z = zero; m_w = one;
        return *this;
    }

    /**
     * Normalize a quaternion required if to be used as a rotational quaternion.
     * <p>
     * Implementation Details:
     * <ul>
     *   <li> {@link #setIdentity()} if {@link #magnitude()} is {@link jau::isZero() is zero} using epsilon</li>
     * </ul>
     * </p>
     * @return this quaternion for chaining.
     */
    constexpr Quaternion& normalize() noexcept {
        const value_type norm = magnitude();
        if ( jau::is_zero(norm) ) {
            setIdentity();
        } else {
            const value_type invNorm = one/norm;
            m_w *= invNorm;
            m_x *= invNorm;
            m_y *= invNorm;
            m_z *= invNorm;
        }
        return *this;
    }

    /**
     * Conjugates this quaternion <code>[-x, -y, -z, w]</code>.
     * @return this quaternion for chaining.
     * @see <a href="http://web.archive.org/web/20041029003853/http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q49">Matrix-FAQ Q49</a>
     */
    constexpr Quaternion& conjugate() noexcept {
        m_x = -m_x;
        m_y = -m_y;
        m_z = -m_z;
        return *this;
    }

    /**
     * Invert the quaternion If rotational, will produce a the inverse rotation
     * <p>
     * Implementation Details:
     * <ul>
     *   <li> {@link #conjugate() conjugates} if {@link #magnitudeSquared()} is is {@link FloatUtil#isEqual(value_type, value_type, value_type) equals 1f} using {@link FloatUtil#EPSILON epsilon}</li>
     * </ul>
     * </p>
     * @return this quaternion for chaining.
     * @see <a href="http://web.archive.org/web/20041029003853/http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q50">Matrix-FAQ Q50</a>
     */
    constexpr Quaternion& invert() noexcept {
        const value_type magnitudeSQ = magnitudeSquared();
        if ( jau::equals(one, magnitudeSQ) ) {
            conjugate();
        } else {
            const value_type invmsq = one/magnitudeSQ;
            m_w *= invmsq;
            m_x = -m_x * invmsq;
            m_y = -m_y * invmsq;
            m_z = -m_z * invmsq;
        }
        return *this;
    }

    /**
     * Set all values of this quaternion using the given components.
     * @return this quaternion for chaining.
     */
    constexpr Quaternion& set(const value_type x, const value_type y, const value_type z, const value_type w) noexcept {
        m_x = x;
        m_y = y;
        m_z = z;
        m_w = w;
        return *this;
    }

    /**
     * Add a quaternion: this = this + rhs, returns this
     *
     * @param q quaternion
     * @return this quaternion for chaining.
     * @see <a href="http://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/code/index.htm#add">euclideanspace.com-QuaternionAdd</a>
     */
    constexpr Quaternion& operator+=(const Quaternion& rhs ) noexcept {
        m_x += rhs.m_x;
        m_y += rhs.m_y;
        m_z += rhs.m_z;
        m_w += rhs.m_w;
        return *this;
    }

    /**
     * Subtract a quaternion: this = this - rhs, returns this
     *
     * @param q quaternion
     * @return this quaternion for chaining.
     * @see <a href="http://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/code/index.htm#add">euclideanspace.com-QuaternionAdd</a>
     */
    constexpr Quaternion& operator-=(const Quaternion& rhs) noexcept {
        m_x -= rhs.m_x;
        m_y -= rhs.m_y;
        m_z -= rhs.m_z;
        m_w -= rhs.m_w;
        return *this;
    }

    /**
     * Multiply this quaternion: this = this * rhs, returns this
     *
     * @param q a quaternion to multiply with
     * @return this quaternion for chaining.
     * @see <a href="http://web.archive.org/web/20041029003853/http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q53">Matrix-FAQ Q53</a>
     * @see <a href="http://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/code/index.htm#mul">euclideanspace.com-QuaternionMul</a>
     */
    constexpr Quaternion& operator*=(const Quaternion& rhs) noexcept {
        return set( m_w * rhs.m_x + m_x * rhs.m_w + m_y * rhs.m_z - m_z * rhs.m_y,
                    m_w * rhs.m_y - m_x * rhs.m_z + m_y * rhs.m_w + m_z * rhs.m_x,
                    m_w * rhs.m_z + m_x * rhs.m_y - m_y * rhs.m_x + m_z * rhs.m_w,
                    m_w * rhs.m_w - m_x * rhs.m_x - m_y * rhs.m_y - m_z * rhs.m_z );
    }

    /**
     * Scale this quaternion by a scalar: this = this * rhs, returns this
     *
     * @param n a value_type constant
     * @return this quaternion for chaining.
     * @see <a href="http://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/code/index.htm#scale">euclideanspace.com-QuaternionScale</a>
     */
    constexpr Quaternion& operator*=(const value_type rhs) noexcept {
        m_x *= rhs;
        m_y *= rhs;
        m_z *= rhs;
        m_w *= rhs;
        return *this;
    }

    /**
     * Rotate this quaternion by the given angle and axis.
     * <p>
     * The axis must be a normalized vector.
     * </p>
     * <p>
     * A rotational quaternion is made from the given angle and axis.
     * </p>
     *
     * @param angle in radians
     * @param axisX x-coord of rotation axis
     * @param axisY y-coord of rotation axis
     * @param axisZ m_z-coord of rotation axis
     * @return this quaternion for chaining.
     */
    constexpr_cxx26 Quaternion& rotateByAngleNormalAxis(const value_type angle, const value_type axisX, const value_type axisY, const value_type axisZ) noexcept {
        if( jau::is_zero3f(axisX, axisY, axisZ) ) {
            // no change
            return *this;
        }
        const value_type halfAngle = half * angle;
        const value_type sin = std::sin(halfAngle);
        const value_type qw = std::cos(halfAngle);
        const value_type qx = sin * axisX;
        const value_type qy = sin * axisY;
        const value_type qz = sin * axisZ;
        return set( m_x * qw + m_y * qz - m_z * qy + m_w * qx,
                   -m_x * qz + m_y * qw + m_z * qx + m_w * qy,
                    m_x * qy - m_y * qx + m_z * qw + m_w * qz,
                   -m_x * qx - m_y * qy - m_z * qz + m_w * qw);
    }

    /**
     * Rotate this quaternion by the given angle and axis.
     * <p>
     * The axis must be a normalized vector.
     * </p>
     * <p>
     * A rotational quaternion is made from the given angle and axis.
     * </p>
     *
     * @param angle in radians
     * @param axis  Vec3 coord of rotation axis
     * @return this quaternion for chaining.
     */
    constexpr_cxx26 Quaternion& rotateByAngleNormalAxis(const value_type angle, const Vec3& axis) noexcept {
        return rotateByAngleNormalAxis(angle, axis.x, axis.y, axis.z);
    }

    /**
     * Rotate this quaternion around X axis with the given angle in radians
     *
     * @param angle in radians
     * @return this quaternion for chaining.
     */
    constexpr_cxx26 Quaternion& rotateByAngleX(const value_type angle) noexcept {
        const value_type halfAngle = half * angle;
        return rotateByAngleX(std::sin(halfAngle), std::cos(halfAngle));
    }
    /** Rotate this quaternion around X axis with the given angle's sin + cos values */
    constexpr Quaternion& rotateByAngleX(const value_type sin, const value_type cos) noexcept {
        return set( m_x * cos + m_w * sin,
                    m_y * cos + m_z * sin,
                   -m_y * sin + m_z * cos,
                   -m_x * sin + m_w * cos);
    }

    /**
     * Rotate this quaternion around Y axis with the given angle in radians
     *
     * @param angle in radians
     * @return this quaternion for chaining.
     */
    constexpr_cxx26 Quaternion& rotateByAngleY(value_type angle) noexcept {
        const value_type halfAngle = half * angle;
        return rotateByAngleY(std::sin(halfAngle), std::cos(halfAngle));
    }
    /** Rotate this quaternion around Y axis with the given angle's sin + cos values */
    constexpr Quaternion& rotateByAngleY(const value_type sin, const value_type cos) noexcept {
        return set( m_x * cos - m_z * sin,
                    m_y * cos + m_w * sin,
                    m_x * sin + m_z * cos,
                   -m_y * sin + m_w * cos);
    }

    /**
     * Rotate this quaternion around Z axis with the given angle in radians
     *
     * @param angle in radians
     * @return this quaternion for chaining.
     */
    constexpr_cxx26 Quaternion& rotateByAngleZ(value_type angle) noexcept {
        const value_type halfAngle = half * angle;
        return rotateByAngleZ(std::sin(halfAngle), std::cos(halfAngle));
    }
    /** Rotate this quaternion around Y axis with the given angle's sin + cos values */
    constexpr Quaternion& rotateByAngleZ(const value_type sin, const value_type cos) noexcept {
        return set( m_x * cos + m_y * sin,
                   -m_x * sin + m_y * cos,
                    m_z * cos + m_w * sin,
                   -m_z * sin + m_w * cos);
    }

    /**
     * Rotates this quaternion from the given Euler rotation array <code>angradXYZ</code> in radians.
     * <p>
     * The <code>angradXYZ</code> array is laid out in natural order:
     * <ul>
     *  <li>x - bank</li>
     *  <li>y - heading</li>
     *  <li>z - attitude</li>
     * </ul>
     * </p>
     * For details see {@link #rotateByEuler(value_type, value_type, value_type)}.
     * @param angradXYZ euler angle array in radians
     * @return this quaternion for chaining.
     * @see #rotateByEuler(value_type, value_type, value_type)
     */
    Quaternion& rotateByEuler(const Vec3& angradXYZ) noexcept {
        return rotateByEuler(angradXYZ.x, angradXYZ.y, angradXYZ.z);
    }

    /**
     * Rotates this quaternion from the given Euler rotation angles in radians.
     * <p>
     * The rotations are applied in the given order and using chained rotation per axis:
     * <ul>
     *  <li>y - heading  - {@link #rotateByAngleY(value_type)}</li>
     *  <li>z - attitude - {@link #rotateByAngleZ(value_type)}</li>
     *  <li>x - bank     - {@link #rotateByAngleX(value_type)}</li>
     * </ul>
     * </p>
     * <p>
     * Implementation Details:
     * <ul>
     *   <li> NOP if all angles are {@link jau::is_zero(value_type, value_type) is zero} using {@link FloatUtil#EPSILON epsilon}</li>
     *   <li> result is {@link #normalize()}ed</li>
     * </ul>
     * </p>
     * @param bankX the Euler pitch angle in radians. (rotation about the X axis)
     * @param headingY the Euler yaw angle in radians. (rotation about the Y axis)
     * @param attitudeZ the Euler roll angle in radians. (rotation about the Z axis)
     * @return this quaternion for chaining.
     * @see #rotateByAngleY(value_type)
     * @see #rotateByAngleZ(value_type)
     * @see #rotateByAngleX(value_type)
     * @see #setFromEuler(value_type, value_type, value_type)
     */
    constexpr_cxx26 Quaternion& rotateByEuler(const value_type bankX, const value_type headingY, const value_type attitudeZ) noexcept {
        if ( jau::is_zero3f(bankX, headingY, attitudeZ) ) {
            return *this;
        } else {
            // setFromEuler muls: ( 8 + 4 ) , + quat muls 24 = 36
            // this:  8  + 8 + 8 + 4 = 28 muls
            return rotateByAngleY(headingY).rotateByAngleZ(attitudeZ).rotateByAngleX(bankX).normalize();
        }
    }

    /***
     * Rotate the given vector by this quaternion
     * @param in vector to be rotated
     *
     * @return new Vec3 result
     * @see <a href="http://web.archive.org/web/20041029003853/http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q63">Matrix-FAQ Q63</a>
     */
    Vec3 rotateVector(const Vec3& in) noexcept {
        Vec3 out;
        rotateVector(in, out);
        return out;
    }

    /***
     * Rotate the given vector by this quaternion
     * @param in vector to be rotated
     * @param vecOut result storage for rotated vector, maybe equal to vecIn for in-place rotation
     * @return the given out store for chaining
     * @see <a href="http://web.archive.org/web/20041029003853/http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q63">Matrix-FAQ Q63</a>
     */
    Vec3& rotateVector(const Vec3& in, Vec3& out) noexcept {
        if( in.is_zero() ) {
            out.set(0, 0, 0);
        } else {
            const value_type vecX = in.x;
            const value_type vecY = in.y;
            const value_type vecZ = in.z;
            const value_type x_x = m_x*m_x;
            const value_type y_y = m_y*m_y;
            const value_type z_z = m_z*m_z;
            const value_type w_w = m_w*m_w;

            out.x =     w_w * vecX
                      + x_x * vecX
                      - z_z * vecX
                      - y_y * vecX
                      + two * ( m_y*m_w*vecZ - m_z*m_w*vecY + m_y*m_x*vecY + m_z*m_x*vecZ );
                                     ;

            out.y =     y_y * vecY
                      - z_z * vecY
                      + w_w * vecY
                      - x_x * vecY
                      + two * ( m_x*m_y*vecX + m_z*m_y*vecZ + m_w*m_z*vecX - m_x*m_w*vecZ );

            out.z =     z_z * vecZ
                      - y_y * vecZ
                      - x_x * vecZ
                      + w_w * vecZ
                      + two * ( m_x*m_z*vecX + m_y*m_z*vecY - m_w*m_y*vecX + m_w*m_x*vecY );
        }
        return out;
    }

    /**
     * Set this quaternion to a spherical linear interpolation
     * between the given start and end quaternions by the given change amount.
     * <p>
     * Note: Method <i>does not</i> normalize this quaternion!
     * </p>
     *
     * @param a start quaternion
     * @param b end  quaternion
     * @param changeAmnt value_type between 0 and 1 representing interpolation.
     * @return this quaternion for chaining.
     * @see <a href="http://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/slerp/">euclideanspace.com-QuaternionSlerp</a>
     */
    constexpr_cxx26 Quaternion& setSlerp(const Quaternion& a, const Quaternion& b, const value_type changeAmnt) noexcept {
        // std::cerr << "Slerp.0: A " << a << ", B " << b << ", t " << changeAmnt << std::endl;
        if (changeAmnt == zero) {
            *this = a;
        } else if (changeAmnt == one) {
            *this = b;
        } else {
            value_type bx = b.m_x;
            value_type by = b.m_y;
            value_type bz = b.m_z;
            value_type bw = b.m_w;

            // Calculate angle between them (quat dot product)
            value_type cosHalfTheta = a.m_x * bx + a.m_y * by + a.m_z * bz + a.m_w * bw;

            value_type scale0, scale1;

            if( cosHalfTheta >= 0.95f ) {
                // quaternions are close, just use linear interpolation
                scale0 = one - changeAmnt;
                scale1 = changeAmnt;
            } else if ( cosHalfTheta <= -0.99f ) {
                // the quaternions are nearly opposite,
                // we can pick any axis normal to a,b to do the rotation
                scale0 = half;
                scale1 = half;
            } else {
                if( cosHalfTheta <= -std::numeric_limits<value_type>::epsilon() ) { // FIXME: .. or shall we use the upper bound 'cosHalfTheta < EPSILON' ?
                    // Negate the second quaternion and the result of the dot product (Inversion)
                    bx *= -one;
                    by *= -one;
                    bz *= -one;
                    bw *= -one;
                    cosHalfTheta *= -one;
                }
                const value_type halfTheta = std::acos(cosHalfTheta);
                const value_type sinHalfTheta = std::sqrt(one - cosHalfTheta*cosHalfTheta);
                // if theta = 180 degrees then result is not fully defined
                // we could rotate around any axis normal to qa or qb
                if ( std::abs(sinHalfTheta) < 0.001f ){ // fabs is floating point absolute
                    scale0 = half;
                    scale1 = half;
                    // throw new InternalError("XXX"); // FIXME should not be reached due to above inversion ?
                } else {
                    // Calculate the scale for q1 and q2, according to the angle and
                    // it's sine value
                    scale0 = std::sin((one - changeAmnt) * halfTheta) / sinHalfTheta;
                    scale1 = std::sin(changeAmnt * halfTheta) / sinHalfTheta;
                }
            }

            m_x = a.m_x * scale0 + bx * scale1;
            m_y = a.m_y * scale0 + by * scale1;
            m_z = a.m_z * scale0 + bz * scale1;
            m_w = a.m_w * scale0 + bw * scale1;
        }
        return *this;
    }

    /**
     * Set this quaternion to equal the rotation required
     * to point the z-axis at <i>direction</i> and the y-axis to <i>up</i>.
     * <p>
     * Implementation generates a 3x3 matrix
     * and is equal with ProjectFloat's lookAt(..).<br/>
     * </p>
     * Implementation Details:
     * <ul>
     *   <li> result is {@link #normalize()}ed</li>
     * </ul>
     * </p>
     * @param directionIn where to <i>look</i> at
     * @param upIn a vector indicating the local <i>up</i> direction.
     * @param xAxisOut vector storing the <i>orthogonal</i> x-axis of the coordinate system.
     * @param yAxisOut vector storing the <i>orthogonal</i> y-axis of the coordinate system.
     * @param zAxisOut vector storing the <i>orthogonal</i> m_z-axis of the coordinate system.
     * @return this quaternion for chaining.
     * @see <a href="http://www.euclideanspace.com/maths/algebra/vectors/lookat/index.htm">euclideanspace.com-LookUp</a>
     */
    Quaternion& setLookAt(const Vec3& directionIn, const Vec3& upIn,
                          Vec3& xAxisOut, Vec3& yAxisOut, Vec3& zAxisOut) noexcept {
        // Z = norm(dir)
        (zAxisOut = directionIn).normalize();

        // X = upIn m_x Z
        //     (borrow yAxisOut for upNorm)
        (yAxisOut = upIn).normalize();
        xAxisOut.cross(yAxisOut, zAxisOut).normalize();

        // Y = Z m_x X
        //
        yAxisOut.cross(zAxisOut, xAxisOut).normalize();

        /**
            const value_type m00 = xAxisOut[0];
            const value_type m01 = yAxisOut[0];
            const value_type m02 = zAxisOut[0];
            const value_type m10 = xAxisOut[1];
            const value_type m11 = yAxisOut[1];
            const value_type m12 = zAxisOut[1];
            const value_type m20 = xAxisOut[2];
            const value_type m21 = yAxisOut[2];
            const value_type m22 = zAxisOut[2];
         */
        return setFromAxes(xAxisOut, yAxisOut, zAxisOut).normalize();
    }

    //
    // Conversions
    //

    /**
     * Initialize this quaternion from two vectors
     * <pre>
     *   q = (s,v) = (v1•v2 , v1 × v2),
     *     angle = angle(v1, v2) = v1•v2
     *      axis = normal(v1 x v2)
     * </pre>
     * <p>
     * Implementation Details:
     * <ul>
     *   <li> set_identity() if square vector-length is jau::is_zero2f(value_type, value_type) using epsilon</li>
     * </ul>
     * </p>
     * @param v1 not normalized
     * @param v2 not normalized
     * @return this quaternion for chaining.
     */
    constexpr_cxx26 Quaternion& setFromVectors(const Vec3& v1, const Vec3& v2) noexcept {
        const value_type factor = v1.length() * v2.length();
        if ( jau::is_zero(factor) ) {
            return setIdentity();
        } else {
            const value_type dot = v1.dot(v2) / factor; // normalize
            const value_type theta = std::acos(std::max(-one, std::min(dot, one))); // clipping [-1..1]

            Vec3 tmpPivotVec = v1.cross(v2);

            if ( dot < zero && jau::is_zero( tmpPivotVec.length() ) ) {
                // Vectors parallel and opposite direction, therefore a rotation of 180 degrees about any vector
                // perpendicular to this vector will rotate vector a onto vector b.
                //
                // The following guarantees the dot-product will be 0.0.
                int dominantIndex;
                if (std::abs(v1.x) > std::abs(v1.y)) {
                    if (std::abs(v1.x) > std::abs(v1.z)) {
                        dominantIndex = 0;
                    } else {
                        dominantIndex = 2;
                    }
                } else {
                    if (std::abs(v1.y) > std::abs(v1.z)) {
                        dominantIndex = 1;
                    } else {
                        dominantIndex = 2;
                    }
                }
                tmpPivotVec[dominantIndex]           = -v1[ (dominantIndex + 1) % 3 ];
                tmpPivotVec[(dominantIndex + 1) % 3] =  v1[ dominantIndex ];
                tmpPivotVec[(dominantIndex + 2) % 3] =  zero;
            }
            return setFromAngleAxis(theta, tmpPivotVec);
        }
    }

    /**
     * Initialize this quaternion from two normalized vectors
     * <pre>
     *   q = (s,v) = (v1•v2 , v1 × v2),
     *     angle = angle(v1, v2) = v1•v2
     *      axis = v1 x v2
     * </pre>
     * <p>
     * Implementation Details:
     * <ul>
     *   <li> {@link #setIdentity()} if square vector-length is {@link jau::is_zero(value_type, value_type) is zero} using {@link FloatUtil#EPSILON epsilon}</li>
     * </ul>
     * </p>
     * @param v1 normalized
     * @param v2 normalized
     * @return this quaternion for chaining.
     */
    constexpr_cxx26 Quaternion& setFromNormalVectors(const Vec3& v1, const Vec3& v2) noexcept {
        const value_type factor = v1.length() * v2.length();
        if ( jau::is_zero(factor) ) {
            return setIdentity();
        } else {
            const value_type dot = v1.dot(v2) / factor; // normalize
            const value_type theta = std::acos(std::max(-one, std::min(dot, one))); // clipping [-1..1]

            Vec3 tmpPivotVec = v1.cross(v2);

            if ( dot < zero && jau::is_zero( tmpPivotVec.length() ) ) {
                // Vectors parallel and opposite direction, therefore a rotation of 180 degrees about any vector
                // perpendicular to this vector will rotate vector a onto vector b.
                //
                // The following guarantees the dot-product will be 0.0.
                int dominantIndex;
                if (std::abs(v1.x) > std::abs(v1.y)) {
                    if (std::abs(v1.x) > std::abs(v1.z)) {
                        dominantIndex = 0;
                    } else {
                        dominantIndex = 2;
                    }
                } else {
                    if (std::abs(v1.y) > std::abs(v1.z)) {
                        dominantIndex = 1;
                    } else {
                        dominantIndex = 2;
                    }
                }
                tmpPivotVec[dominantIndex]           = -v1[ (dominantIndex + 1) % 3 ];
                tmpPivotVec[(dominantIndex + 1) % 3] =  v1[ dominantIndex ];
                tmpPivotVec[(dominantIndex + 2) % 3] =  zero;
            }
            return setFromAngleNormalAxis(theta, tmpPivotVec);
        }
    }

    /***
     * Initialize this quaternion with given non-normalized axis vector and rotation angle
     * <p>
     * Implementation Details:
     * <ul>
     *   <li> {@link #setIdentity()} if axis is {@link jau::is_zero(value_type, value_type) is zero} using {@link FloatUtil#EPSILON epsilon}</li>
     * </ul>
     * </p>
     * @param angle rotation angle (rads)
     * @param vector axis vector not normalized
     * @return this quaternion for chaining.
     *
     * @see <a href="http://web.archive.org/web/20041029003853/http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q56">Matrix-FAQ Q56</a>
     * @see #toAngleAxis(Vec3)
     */
    Quaternion& setFromAngleAxis(const value_type angle, const Vec3& vector) noexcept {
        return setFromAngleNormalAxis(angle, Vec3(vector).normalize());
    }

    /***
     * Initialize this quaternion with given normalized axis vector and rotation angle
     * <p>
     * Implementation Details:
     * <ul>
     *   <li> {@link #setIdentity()} if axis is {@link jau::is_zero(value_type, value_type) is zero} using {@link FloatUtil#EPSILON epsilon}</li>
     * </ul>
     * </p>
     * @param angle rotation angle (rads)
     * @param vector axis vector normalized
     * @return this quaternion for chaining.
     *
     * @see <a href="http://web.archive.org/web/20041029003853/http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q56">Matrix-FAQ Q56</a>
     * @see #toAngleAxis(Vec3)
     */
    constexpr_cxx26 Quaternion& setFromAngleNormalAxis(const value_type angle, const Vec3& vector) noexcept {
        if( vector.is_zero() ) {
            setIdentity();
        } else {
            const value_type halfangle = angle * half;
            const value_type sin = std::sin(halfangle);
            m_x = vector.x * sin;
            m_y = vector.y * sin;
            m_z = vector.z * sin;
            m_w = std::cos(halfangle);
        }
        return *this;
    }

    /**
     * Transform the rotational quaternion to axis based rotation angles
     *
     * @param axis storage for computed axis
     * @return the rotation angle in radians
     * @see #setFromAngleAxis(value_type, Vec3, Vec3)
     */
    constexpr_cxx26 value_type toAngleAxis(Vec3& axis) const noexcept {
        const value_type sqrLength = m_x*m_x + m_y*m_y + m_z*m_z;
        value_type angle;
        if ( jau::is_zero(sqrLength) ) { // length is ~0
            angle = zero;
            axis.set( one, zero, zero );
        } else {
            angle = std::acos(m_w) * two;
            const value_type invLength = one / std::sqrt(sqrLength);
            axis.set( m_x * invLength,
                      m_y * invLength,
                      m_z * invLength );
        }
        return angle;
    }

    /**
     * Initializes this quaternion from the given Euler rotation array <code>angradXYZ</code> in radians.
     * <p>
     * The <code>angradXYZ</code> vector is laid out in natural order:
     * <ul>
     *  <li>x - bank</li>
     *  <li>y - heading</li>
     *  <li>z - attitude</li>
     * </ul>
     * </p>
     * For details see {@link #setFromEuler(value_type, value_type, value_type)}.
     * @param angradXYZ euler angle vector in radians holding x-bank, m_y-heading and m_z-attitude
     * @return this quaternion for chaining.
     * @see #setFromEuler(value_type, value_type, value_type)
     */
    constexpr_cxx26 Quaternion& setFromEuler(const Vec3& angradXYZ) noexcept {
        return setFromEuler(angradXYZ.x, angradXYZ.y, angradXYZ.z);
    }

    /**
     * Initializes this quaternion from the given Euler rotation angles in radians.
     * <p>
     * The rotations are applied in the given order:
     * <ul>
     *  <li>y - heading</li>
     *  <li>z - attitude</li>
     *  <li>x - bank</li>
     * </ul>
     * </p>
     * <p>
     * Implementation Details:
     * <ul>
     *   <li> {@link #setIdentity()} if all angles are {@link jau::is_zero(value_type, value_type) is zero} using {@link FloatUtil#EPSILON epsilon}</li>
     *   <li> result is {@link #normalize()}ed</li>
     * </ul>
     * </p>
     * @param bankX the Euler pitch angle in radians. (rotation about the X axis)
     * @param headingY the Euler yaw angle in radians. (rotation about the Y axis)
     * @param attitudeZ the Euler roll angle in radians. (rotation about the Z axis)
     * @return this quaternion for chaining.
     *
     * @see <a href="http://web.archive.org/web/20041029003853/http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q60">Matrix-FAQ Q60</a>
     * @see <a href="http://vered.rose.utoronto.ca/people/david_dir/GEMS/GEMS.html">Gems</a>
     * @see <a href="http://www.euclideanspace.com/maths/geometry/rotations/conversions/eulerToQuaternion/index.htm">euclideanspace.com-eulerToQuaternion</a>
     * @see #toEuler(Vec3)
     */
    constexpr_cxx26 Quaternion& setFromEuler(const value_type bankX, const value_type headingY, const value_type attitudeZ) noexcept {
        if ( jau::is_zero3f(bankX, headingY, attitudeZ) ) {
            return setIdentity();
        } else {
            value_type angle = headingY * half;
            const value_type sinHeadingY = std::sin(angle);
            const value_type cosHeadingY = std::cos(angle);
            angle = attitudeZ * half;
            const value_type sinAttitudeZ = std::sin(angle);
            const value_type cosAttitudeZ = std::cos(angle);
            angle = bankX * half;
            const value_type sinBankX = std::sin(angle);
            const value_type cosBankX = std::cos(angle);

            // variables used to reduce multiplication calls.
            const value_type cosHeadingXcosAttitude = cosHeadingY * cosAttitudeZ;
            const value_type sinHeadingXsinAttitude = sinHeadingY * sinAttitudeZ;
            const value_type cosHeadingXsinAttitude = cosHeadingY * sinAttitudeZ;
            const value_type sinHeadingXcosAttitude = sinHeadingY * cosAttitudeZ;

            m_w = cosHeadingXcosAttitude * cosBankX - sinHeadingXsinAttitude * sinBankX;
            m_x = cosHeadingXcosAttitude * sinBankX + sinHeadingXsinAttitude * cosBankX;
            m_y = sinHeadingXcosAttitude * cosBankX + cosHeadingXsinAttitude * sinBankX;
            m_z = cosHeadingXsinAttitude * cosBankX - sinHeadingXcosAttitude * sinBankX;
            return normalize();
        }
    }

    /**
     * Transform this quaternion to Euler rotation angles in radians (pitchX, yawY and rollZ).
     * <p>
     * The <code>result</code> array is laid out in natural order:
     * <ul>
     *  <li>x - bank</li>
     *  <li>y - heading</li>
     *  <li>z - attitude</li>
     * </ul>
     * </p>
     *
     * @return new Vec3 euler angle result vector filled with x-bank, y-heading and m_z-attitude
     * @see <a href="http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/index.htm">euclideanspace.com-quaternionToEuler</a>
     * @see #setFromEuler(value_type, value_type, value_type)
     */
    constexpr_cxx26 Vec3 toEuler() noexcept {
        const value_type sqw = m_w*m_w;
        const value_type sqx = m_x*m_x;
        const value_type sqy = m_y*m_y;
        const value_type sqz = m_z*m_z;
        const value_type unit = sqx + sqy + sqz + sqw; // if normalized is one, otherwise, is correction factor
        const value_type test = m_x*m_y + m_z*m_w;

        if (test > 0.499f * unit) { // singularity at north pole
            return Vec3( zero,                        // m_x-bank
                         two * std::atan2(m_x, m_w),  // y-heading
                         jau::PI_2<value_type> );     // z-attitude
        } else if (test < -0.499f * unit) { // singularity at south pole
            return Vec3( zero,                        // m_x-bank
                        -two * std::atan2(m_x, m_w),  // m_y-heading
                        -jau::PI_2<value_type> );     // m_z-attitude
        } else {
            return Vec3( std::atan2(two * m_x * m_w - two * m_y * m_z, -sqx + sqy - sqz + sqw), // m_x-bank
                         std::atan2(two * m_y * m_w - two * m_x * m_z,  sqx - sqy - sqz + sqw), // m_y-heading
                         std::asin( two * test / unit) );                                        // z-attitude
        }
    }

    /**
     * Compute the quaternion from a 3x3 column rotation matrix
     * <p>
     * See <a href="ftp://ftp.cis.upenn.edu/pub/graphics/shoemake/quatut.ps.Z">Graphics Gems Code</a>,<br/>
     * <a href="http://mathworld.wolfram.com/MatrixTrace.html">MatrixTrace</a>.
     * </p>
     * <p>
     * Buggy <a href="http://web.archive.org/web/20041029003853/http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q55">Matrix-FAQ Q55</a>
     * </p>
     *
     * @return this quaternion for chaining.
     * @see #setFromMatrix(Matrix4f)
     */
    constexpr Quaternion& setFromMat(const value_type m00, const value_type m01, const value_type m02,
                                     const value_type m10, const value_type m11, const value_type m12,
                                     const value_type m20, const value_type m21, const value_type m22) noexcept {
        // Note: Other implementations uses 'T' w/o '+1f' and compares 'T >= 0' while adding missing 1f in sqrt expr.
        //       However .. this causes setLookAt(..) to fail and actually violates the 'trace definition'.

        // The trace T is the sum of the diagonal elements; see
        // http://mathworld.wolfram.com/MatrixTrace.html
        const value_type T = m00 + m11 + m22 + one;
        if ( T > zero ) {
            const value_type S = half / std::sqrt(T);  // S = 1 / ( 2 t )
            m_w = 0.25f / S;                      // m_w = 1 / ( 4 S ) = t / 2
            m_x = ( m21 - m12 ) * S;
            m_y = ( m02 - m20 ) * S;
            m_z = ( m10 - m01 ) * S;
        } else if ( m00 > m11 && m00 > m22) {
            const value_type S = half / std::sqrt(one + m00 - m11 - m22); // S=4*qx
            m_w = ( m21 - m12 ) * S;
            m_x = 0.25f / S;
            m_y = ( m10 + m01 ) * S;
            m_z = ( m02 + m20 ) * S;
        } else if ( m11 > m22 ) {
            const value_type S = half / std::sqrt(one + m11 - m00 - m22); // S=4*qy
            m_w = ( m02 - m20 ) * S;
            m_x = ( m20 + m01 ) * S;
            m_y = 0.25f / S;
            m_z = ( m21 + m12 ) * S;
        } else {
            const value_type S = half / std::sqrt(one + m22 - m00 - m11); // S=4*qz
            m_w = ( m10 - m01 ) * S;
            m_x = ( m02 + m20 ) * S;
            m_y = ( m21 + m12 ) * S;
            m_z = 0.25f / S;
        }
        return *this;
    }

    /**
     * Compute the quaternion from a 3x3 column rotation matrix from Mat4 instance
     * <p>
     * See <a href="ftp://ftp.cis.upenn.edu/pub/graphics/shoemake/quatut.ps.Z">Graphics Gems Code</a>,<br/>
     * <a href="http://mathworld.wolfram.com/MatrixTrace.html">MatrixTrace</a>.
     * </p>
     * <p>
     * Buggy <a href="http://web.archive.org/web/20041029003853/http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q55">Matrix-FAQ Q55</a>
     * </p>
     *
     * @return this quaternion for chaining.
     * @see Matrix4f#getRotation(Quaternion)
     * @see #setFromMatrix(value_type, value_type, value_type, value_type, value_type, value_type, value_type, value_type, value_type)
     */
    constexpr Quaternion& setFromMat(const Mat4& m) noexcept {
        return setFromMat(m.m00, m.m01, m.m02, m.m10, m.m11, m.m12, m.m20, m.m21, m.m22);
    }

    /**
     * Initializes this quaternion to represent a rotation formed by the given three <i>orthogonal</i> axes.
     * <p>
     * No validation whether the axes are <i>orthogonal</i> is performed.
     * </p>
     *
     * @param xAxis vector representing the <i>orthogonal</i> x-axis of the coordinate system.
     * @param yAxis vector representing the <i>orthogonal</i> y-axis of the coordinate system.
     * @param zAxis vector representing the <i>orthogonal</i> m_z-axis of the coordinate system.
     * @return this quaternion for chaining.
     */
    constexpr Quaternion& setFromAxes(const Vec3& xAxis, const Vec3& yAxis, const Vec3& zAxis) noexcept {
        return setFromMat(xAxis.x, yAxis.x, zAxis.x,
                          xAxis.y, yAxis.y, zAxis.y,
                          xAxis.z, yAxis.z, zAxis.z);
    }

    /**
     * Transform this quaternion to a normalized 4x4 column matrix representing the rotation.
     *
     * Implementation Details:
     * - makes identity matrix if {@link #magnitudeSquared()} is {@link jau::is_zero(value_type, value_type) is zero} using {@link FloatUtil#EPSILON epsilon}
     * - Mat4 fields [m00 .. m22] define the rotation
     *
     * @return resulting normalized column matrix 4x4
     * @see [Matrix-FAQ Q54](http://web.archive.org/web/20041029003853/http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q54)
     * @see #setFromMatrix(value_type, value_type, value_type, value_type, value_type, value_type, value_type, value_type, value_type)
     * @see Matrix4f#setToRotation(Quaternion)
     */
    constexpr Mat4 toMatrix() const noexcept {
        Mat4 m; toMatrix(m); return m;
    }

    /**
     * Transform this quaternion to a normalized 4x4 column matrix representing the rotation.
     *
     * Implementation Details:
     * - makes identity matrix if {@link #magnitudeSquared()} is {@link jau::is_zero(value_type, value_type) is zero} using {@link FloatUtil#EPSILON epsilon}
     * - Mat4 fields [m00 .. m22] define the rotation
     *
     * @param out store for the resulting normalized column matrix 4x4
     * @return the given matrix store
     * @see [Matrix-FAQ Q54](http://web.archive.org/web/20041029003853/http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q54)
     * @see #setFromMatrix(value_type, value_type, value_type, value_type, value_type, value_type, value_type, value_type, value_type)
     * @see Matrix4f#setToRotation(Quaternion)
     */
    constexpr Mat4& toMatrix(Mat4& m) const noexcept {
        // pre-multiply scaled-reciprocal-magnitude to reduce multiplications
        const value_type norm = magnitudeSquared();
        if ( jau::is_zero(norm) ) {
            // identity matrix -> srecip = 0f
            m.loadIdentity();
            return m;
        }
        value_type srecip;
        if ( jau::equals(one, norm) ) {
            srecip = two;
        } else {
            srecip = two / norm;
        }
        const value_type xs = srecip * m_x;
        const value_type ys = srecip * m_y;
        const value_type zs = srecip * m_z;

        const value_type xx = m_x * xs;
        const value_type xy = m_x * ys;
        const value_type xz = m_x * zs;
        const value_type xw = xs  * m_w;
        const value_type yy = m_y * ys;
        const value_type yz = m_y * zs;
        const value_type yw = ys  * m_w;
        const value_type zz = m_z * zs;
        const value_type zw = zs  * m_w;

        m.m00 = one - ( yy + zz );
        m.m01 =       ( xy - zw );
        m.m02 =       ( xz + yw );
        m.m03 = zero;

        m.m10 =       ( xy + zw );
        m.m11 = one - ( xx + zz );
        m.m12 =       ( yz - xw );
        m.m13 = zero;

        m.m20 =       ( xz - yw );
        m.m21 =       ( yz + xw );
        m.m22 = one - ( xx + yy );
        m.m23 = zero;

        m.m30 = m.m31 = m.m32 = zero;
        m.m33 = one;
        return m;
    }

    /**
     * Extracts this quaternion's <i>orthogonal</i> rotation axes.
     *
     * @param xAxis vector representing the <i>orthogonal</i> x-axis of the coordinate system.
     * @param yAxis vector representing the <i>orthogonal</i> y-axis of the coordinate system.
     * @param zAxis vector representing the <i>orthogonal</i> m_z-axis of the coordinate system.
     * @param tmp temporary Matrix4 used for toMatrix()
     */
    constexpr void toAxes(Vec3& xAxis, Vec3& yAxis, Vec3& zAxis, Matrix4<value_type>& tmp) const noexcept {
        toMatrix(tmp);
        tmp.getColumn(2, zAxis);
        tmp.getColumn(1, yAxis);
        tmp.getColumn(0, xAxis);
    }

    /**
     * Extracts this quaternion's <i>orthogonal</i> rotation axes.
     *
     * @param xAxis vector representing the <i>orthogonal</i> x-axis of the coordinate system.
     * @param yAxis vector representing the <i>orthogonal</i> y-axis of the coordinate system.
     * @param zAxis vector representing the <i>orthogonal</i> m_z-axis of the coordinate system.
     */
    constexpr void toAxes(Vec3& xAxis, Vec3& yAxis, Vec3& zAxis) const noexcept {
        Matrix4<value_type> tmp;
        toAxes(xAxis, yAxis, zAxis, tmp);
    }

    //
    // std overrides
    //

    /**
     * @param o the object to compare for equality
     * @return true if this quaternion and the provided quaternion have roughly the same x, m_y, m_z and m_w values.
     */
    constexpr bool operator==(const Quaternion& o) const noexcept {
        if (this == &o) {
            return true;
        }
        return std::abs(m_x - o.m_x) <= allowed_deviation &&
               std::abs(m_y - o.m_y) <= allowed_deviation &&
               std::abs(m_z - o.m_z) <= allowed_deviation &&
               std::abs(m_w - o.m_w) <= allowed_deviation;
    }

    std::string toString() const noexcept {
        return "Quat[x "+std::to_string(m_x)+", y "+std::to_string(m_y)+", z "+std::to_string(m_z)+", w "+std::to_string(m_w)+"]";
    }
};

template<std::floating_point T>
constexpr Quaternion<T> operator+(const Quaternion<T>& lhs, const Quaternion<T>& rhs ) noexcept {
    Quaternion<T> r(lhs); r += rhs; return r;
}

template<std::floating_point T>
constexpr Quaternion<T> operator-(const Quaternion<T>& lhs, const Quaternion<T>& rhs ) noexcept {
    Quaternion<T> r(lhs); r -= rhs; return r;
}

template<std::floating_point T>
constexpr Quaternion<T> operator*(const Quaternion<T>& lhs, const Quaternion<T>& rhs ) noexcept {
    Quaternion<T> r(lhs); r *= rhs; return r;
}

template<std::floating_point T>
constexpr Quaternion<T> operator*(const Quaternion<T>& lhs, const T s ) noexcept {
    Quaternion<T> r(lhs); r *= s; return r;
}

template<std::floating_point T>
constexpr Quaternion<T> operator*(const T s, const Quaternion<T>& rhs) noexcept {
    Quaternion<T> r(rhs); r *= s; return r;
}

template<std::floating_point T>
std::ostream& operator<<(std::ostream& out, const Quaternion<T>& v) noexcept {
    return out << v.toString();
}

typedef Quaternion<float> Quat4f;
static_assert(alignof(float) == alignof(Quat4f));

template <jau::req::packed_floating_point Value_type>
Matrix4<Value_type>&
Matrix4<Value_type>::setToRotation(const Matrix4<Value_type>::Quat& q) {
    Matrix4<Value_type> m0;
    Quaternion<Value_type> r;
    m0.setToRotation(r);
    r.toMatrix(m0);

    q.toMatrix(*this);
    return *this;
}

template <jau::req::packed_floating_point Value_type>
Matrix4<Value_type>::Quat&
Matrix4<Value_type>::getRotation(Matrix4<Value_type>::Quat& res) const noexcept {
    return res.setFromMat(m00, m01, m02, m10, m11, m12, m20, m21, m22);
}

template <jau::req::packed_floating_point Value_type>
Matrix4<Value_type>&
Matrix4<Value_type>::rotate(const Matrix4<Value_type>::Quat& quat) noexcept {
    Matrix4<Value_type> tmp;
    return mul( quat.toMatrix(tmp) );
}

/**@}*/

} // namespace jau::math

#endif // JAU_MATH_QUATERNION_HPP_
