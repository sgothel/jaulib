/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2010-2024 Gothel Software e.K.
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
#ifndef QUATERNION_HPP_
#define QUATERNION_HPP_

#include <cmath>
#include <cstdarg>
#include <cstdint>
#include <limits>
#include <string>

#include <jau/float_math.hpp>
#include <jau/math/vec3f.hpp>
#include <jau/math/mat4f.hpp>

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
class Quaternion {
  private:
    float m_x, m_y, m_z, m_w;

  public:

    /**
     * Quaternion Epsilon, used with equals method to determine if two Quaternions are close enough to be considered equal.
     * <p>
     * Using {@value}, which is ~10 times {@link FloatUtil#EPSILON}.
     * </p>
     */
    constexpr static const float ALLOWED_DEVIANCE = 1.0E-6f; // FloatUtil.EPSILON == 1.1920929E-7f; double ALLOWED_DEVIANCE: 1.0E-8f

    constexpr Quaternion() noexcept
    : m_x(0), m_y(0), m_z(0), m_w(1) {}

    constexpr Quaternion(const float x, const float y, const float z, const float w) noexcept
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
    float magnitudeSquared() const noexcept {
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
     *   <li> returns 0f if {@link #magnitudeSquared()} is {@link jau::is_zero(float, float) is zero} using {@link FloatUtil#EPSILON epsilon}</li>
     *   <li> returns 1f if {@link #magnitudeSquared()} is {@link FloatUtil#isEqual(float, float, float) equals 1f} using {@link FloatUtil#EPSILON epsilon}</li>
     * </ul>
     * </p>
     */
    float magnitude() const noexcept {
        const float magnitudeSQ = magnitudeSquared();
        if ( jau::is_zero(magnitudeSQ) ) {
            return 0.0f;
        }
        if ( jau::equals(1.0f, magnitudeSQ) ) {
            return 1.0f;
        }
        return std::sqrt(magnitudeSQ);
    }

    float w() const noexcept { return m_w; }

    void set_w(float w) noexcept { m_w = w; }

    float x() const noexcept { return m_x; }

    void set_x(float x) noexcept { m_x = x; }

    float y() const noexcept { return m_y; }

    void set_y(float y) noexcept { m_y = y; }

    float z() const noexcept { return m_z; }

    void set_z(float z) noexcept { m_z = z; }

    /**
     * Returns the dot product of this quaternion with the given x,y,z and m_w components.
     */
    float dot(float x, float y, float z, float w) const noexcept {
        return m_x * x + m_y * y + m_z * z + w * w;
    }

    /**
     * Returns the dot product of this quaternion with the given quaternion
     */
    float dot(const Quaternion& quat) const noexcept {
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
    bool is_identity() const noexcept {
        return jau::equals(1.0f, m_w) && jau::is_zero3f(m_x, m_y, m_z);
        // return m_w == 1f && m_x == 0f && m_y == 0f && m_z == 0f;
    }

    /***
     * Set this quaternion to identity (x=0,y=0,z=0,w=1)
     * @return this quaternion for chaining.
     */
    Quaternion& set_identity() noexcept {
        m_x = m_y = m_z = 0.0f; m_w = 1.0f;
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
    Quaternion& normalize() noexcept {
        const float norm = magnitude();
        if ( jau::is_zero(norm) ) {
            set_identity();
        } else {
            const float invNorm = 1.0f/norm;
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
    Quaternion& conjugate() noexcept {
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
     *   <li> {@link #conjugate() conjugates} if {@link #magnitudeSquared()} is is {@link FloatUtil#isEqual(float, float, float) equals 1f} using {@link FloatUtil#EPSILON epsilon}</li>
     * </ul>
     * </p>
     * @return this quaternion for chaining.
     * @see <a href="http://web.archive.org/web/20041029003853/http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q50">Matrix-FAQ Q50</a>
     */
    Quaternion& invert() noexcept {
        const float magnitudeSQ = magnitudeSquared();
        if ( jau::equals(1.0f, magnitudeSQ) ) {
            conjugate();
        } else {
            const float invmsq = 1.0f/magnitudeSQ;
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
    Quaternion& set(float x, float y, float z, float w) noexcept {
        m_x = x;
        m_y = y;
        m_z = z;
        m_w = w;
        return *this;
    }

    /**
     * Add a quaternion
     *
     * @param q quaternion
     * @return this quaternion for chaining.
     * @see <a href="http://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/code/index.htm#add">euclideanspace.com-QuaternionAdd</a>
     */
    Quaternion& add(const Quaternion& q) noexcept {
        m_x += q.m_x;
        m_y += q.m_y;
        m_z += q.m_z;
        m_w += q.m_w;
        return *this;
    }

    /**
     * Subtract a quaternion
     *
     * @param q quaternion
     * @return this quaternion for chaining.
     * @see <a href="http://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/code/index.htm#add">euclideanspace.com-QuaternionAdd</a>
     */
    Quaternion& subtract(const Quaternion& q) noexcept {
        m_x -= q.m_x;
        m_y -= q.m_y;
        m_z -= q.m_z;
        m_w -= q.m_w;
        return *this;
    }

    /**
     * Multiply this quaternion by the param quaternion
     *
     * @param q a quaternion to multiply with
     * @return this quaternion for chaining.
     * @see <a href="http://web.archive.org/web/20041029003853/http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q53">Matrix-FAQ Q53</a>
     * @see <a href="http://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/code/index.htm#mul">euclideanspace.com-QuaternionMul</a>
     */
    Quaternion& mult(const Quaternion& q) noexcept {
        return set( m_w * q.m_x + m_x * q.m_w + m_y * q.m_z - m_z * q.m_y,
                    m_w * q.m_y - m_x * q.m_z + m_y * q.m_w + m_z * q.m_x,
                    m_w * q.m_z + m_x * q.m_y - m_y * q.m_x + m_z * q.m_w,
                    m_w * q.m_w - m_x * q.m_x - m_y * q.m_y - m_z * q.m_z );
    }

    /**
     * Scale this quaternion by a constant
     *
     * @param n a float constant
     * @return this quaternion for chaining.
     * @see <a href="http://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/code/index.htm#scale">euclideanspace.com-QuaternionScale</a>
     */
    Quaternion& scale(float n) noexcept {
        m_x *= n;
        m_y *= n;
        m_z *= n;
        m_w *= n;
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
    Quaternion& rotateByAngleNormalAxis(float angle, float axisX, float axisY, float axisZ) noexcept {
        if( jau::is_zero3f(axisX, axisY, axisZ) ) {
            // no change
            return *this;
        }
        const float halfAngle = 0.5f * angle;
        const float sin = std::sin(halfAngle);
        const float qw = std::cos(halfAngle);
        const float qx = sin * axisX;
        const float qy = sin * axisY;
        const float qz = sin * axisZ;
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
     * @param axis  Vec3f coord of rotation axis
     * @return this quaternion for chaining.
     */
    Quaternion& rotateByAngleNormalAxis(float angle, const Vec3f& axis) noexcept {
        return rotateByAngleNormalAxis(angle, axis.x, axis.y, axis.z);
    }

    /**
     * Rotate this quaternion around X axis with the given angle in radians
     *
     * @param angle in radians
     * @return this quaternion for chaining.
     */
    Quaternion& rotateByAngleX(float angle) noexcept {
        const float halfAngle = 0.5f * angle;
        const float sin = std::sin(halfAngle);
        const float cos = std::cos(halfAngle);
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
    Quaternion& rotateByAngleY(float angle) noexcept {
        const float halfAngle = 0.5f * angle;
        const float sin = std::sin(halfAngle);
        const float cos = std::cos(halfAngle);
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
    Quaternion& rotateByAngleZ(float angle) noexcept {
        const float halfAngle = 0.5f * angle;
        const float sin = std::sin(halfAngle);
        const float cos = std::cos(halfAngle);
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
     * For details see {@link #rotateByEuler(float, float, float)}.
     * @param angradXYZ euler angle array in radians
     * @return this quaternion for chaining.
     * @see #rotateByEuler(float, float, float)
     */
    Quaternion& rotateByEuler(const Vec3f& angradXYZ) noexcept {
        return rotateByEuler(angradXYZ.x, angradXYZ.y, angradXYZ.z);
    }

    /**
     * Rotates this quaternion from the given Euler rotation angles in radians.
     * <p>
     * The rotations are applied in the given order and using chained rotation per axis:
     * <ul>
     *  <li>y - heading  - {@link #rotateByAngleY(float)}</li>
     *  <li>z - attitude - {@link #rotateByAngleZ(float)}</li>
     *  <li>x - bank     - {@link #rotateByAngleX(float)}</li>
     * </ul>
     * </p>
     * <p>
     * Implementation Details:
     * <ul>
     *   <li> NOP if all angles are {@link jau::is_zero(float, float) is zero} using {@link FloatUtil#EPSILON epsilon}</li>
     *   <li> result is {@link #normalize()}ed</li>
     * </ul>
     * </p>
     * @param bankX the Euler pitch angle in radians. (rotation about the X axis)
     * @param headingY the Euler yaw angle in radians. (rotation about the Y axis)
     * @param attitudeZ the Euler roll angle in radians. (rotation about the Z axis)
     * @return this quaternion for chaining.
     * @see #rotateByAngleY(float)
     * @see #rotateByAngleZ(float)
     * @see #rotateByAngleX(float)
     * @see #setFromEuler(float, float, float)
     */
    Quaternion& rotateByEuler(float bankX, float headingY, float attitudeZ) noexcept {
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
     * @param out result storage for rotated vector, maybe equal to in for in-place rotation
     *
     * @return the given out store for chaining
     * @see <a href="http://web.archive.org/web/20041029003853/http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q63">Matrix-FAQ Q63</a>
     */
    Vec3f& rotateVector(const Vec3f& in, Vec3f& out) noexcept {
        if( in.is_zero() ) {
            out.set(0, 0, 0);
        } else {
            const float vecX = in.x;
            const float vecY = in.y;
            const float vecZ = in.z;
            const float x_x = m_x*m_x;
            const float y_y = m_y*m_y;
            const float z_z = m_z*m_z;
            const float w_w = m_w*m_w;

            out.x =     w_w * vecX
                         + x_x * vecX
                         - z_z * vecX
                         - y_y * vecX
                         + 2.0f * ( m_y*m_w*vecZ - m_z*m_w*vecY + m_y*m_x*vecY + m_z*m_x*vecZ );
                                     ;

            out.y =     y_y * vecY
                         - z_z * vecY
                         + w_w * vecY
                         - x_x * vecY
                         + 2.0f * ( m_x*m_y*vecX + m_z*m_y*vecZ + m_w*m_z*vecX - m_x*m_w*vecZ );

            out.z =     z_z * vecZ
                         - y_y * vecZ
                         - x_x * vecZ
                         + w_w * vecZ
                         + 2.0f * ( m_x*m_z*vecX + m_y*m_z*vecY - m_w*m_y*vecX + m_w*m_x*vecY );
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
     * @param changeAmnt float between 0 and 1 representing interpolation.
     * @return this quaternion for chaining.
     * @see <a href="http://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/slerp/">euclideanspace.com-QuaternionSlerp</a>
     */
    Quaternion& set_slerp(const Quaternion& a, const Quaternion& b, float changeAmnt) noexcept {
        // std::cerr << "Slerp.0: A " << a << ", B " << b << ", t " << changeAmnt << std::endl;
        if (changeAmnt == 0.0f) {
            *this = a;
        } else if (changeAmnt == 1.0f) {
            *this = b;
        } else {
            float bx = b.m_x;
            float by = b.m_y;
            float bz = b.m_z;
            float bw = b.m_w;

            // Calculate angle between them (quat dot product)
            float cosHalfTheta = a.m_x * bx + a.m_y * by + a.m_z * bz + a.m_w * bw;

            float scale0, scale1;

            if( cosHalfTheta >= 0.95f ) {
                // quaternions are close, just use linear interpolation
                scale0 = 1.0f - changeAmnt;
                scale1 = changeAmnt;
            } else if ( cosHalfTheta <= -0.99f ) {
                // the quaternions are nearly opposite,
                // we can pick any axis normal to a,b to do the rotation
                scale0 = 0.5f;
                scale1 = 0.5f;
            } else {
                if( cosHalfTheta <= -std::numeric_limits<float>::epsilon() ) { // FIXME: .. or shall we use the upper bound 'cosHalfTheta < EPSILON' ?
                    // Negate the second quaternion and the result of the dot product (Inversion)
                    bx *= -1.0f;
                    by *= -1.0f;
                    bz *= -1.0f;
                    bw *= -1.0f;
                    cosHalfTheta *= -1.0f;
                }
                const float halfTheta = std::acos(cosHalfTheta);
                const float sinHalfTheta = std::sqrt(1.0f - cosHalfTheta*cosHalfTheta);
                // if theta = 180 degrees then result is not fully defined
                // we could rotate around any axis normal to qa or qb
                if ( std::abs(sinHalfTheta) < 0.001f ){ // fabs is floating point absolute
                    scale0 = 0.5f;
                    scale1 = 0.5f;
                    // throw new InternalError("XXX"); // FIXME should not be reached due to above inversion ?
                } else {
                    // Calculate the scale for q1 and q2, according to the angle and
                    // it's sine value
                    scale0 = std::sin((1.0f - changeAmnt) * halfTheta) / sinHalfTheta;
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
    Quaternion& setLookAt(const Vec3f& directionIn, const Vec3f& upIn,
                          Vec3f& xAxisOut, Vec3f& yAxisOut, Vec3f& zAxisOut) noexcept {
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
            const float m00 = xAxisOut[0];
            const float m01 = yAxisOut[0];
            const float m02 = zAxisOut[0];
            const float m10 = xAxisOut[1];
            const float m11 = yAxisOut[1];
            const float m12 = zAxisOut[1];
            const float m20 = xAxisOut[2];
            const float m21 = yAxisOut[2];
            const float m22 = zAxisOut[2];
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
     *   <li> set_identity() if square vector-length is jau::is_zero2f(float, float) using epsilon</li>
     * </ul>
     * </p>
     * @param v1 not normalized
     * @param v2 not normalized
     * @param tmpPivotVec temp storage for cross product
     * @param tmpNormalVec temp storage to normalize vector
     * @return this quaternion for chaining.
     */
    Quaternion& setFromVectors(const Vec3f& v1, const Vec3f& v2, Vec3f& tmpPivotVec, Vec3f& tmpNormalVec) noexcept {
        const float factor = v1.length() * v2.length();
        if ( jau::is_zero(factor) ) {
            return set_identity();
        } else {
            const float dot = v1.dot(v2) / factor; // normalize
            const float theta = std::acos(std::max(-1.0f, std::min(dot, 1.0f))); // clipping [-1..1]

            tmpPivotVec.cross(v1, v2);

            if ( dot < 0.0f && jau::is_zero( tmpPivotVec.length() ) ) {
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
                tmpPivotVec[(dominantIndex + 2) % 3] =  0.0f;
            }
            return setFromAngleAxis(theta, tmpPivotVec, tmpNormalVec);
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
     *   <li> {@link #setIdentity()} if square vector-length is {@link jau::is_zero(float, float) is zero} using {@link FloatUtil#EPSILON epsilon}</li>
     * </ul>
     * </p>
     * @param v1 normalized
     * @param v2 normalized
     * @param tmpPivotVec temp storage for cross product
     * @return this quaternion for chaining.
     */
    Quaternion& setFromNormalVectors(const Vec3f& v1, const Vec3f& v2, Vec3f& tmpPivotVec) noexcept {
        const float factor = v1.length() * v2.length();
        if ( jau::is_zero(factor) ) {
            return set_identity();
        } else {
            const float dot = v1.dot(v2) / factor; // normalize
            const float theta = std::acos(std::max(-1.0f, std::min(dot, 1.0f))); // clipping [-1..1]

            tmpPivotVec.cross(v1, v2);

            if ( dot < 0.0f && jau::is_zero( tmpPivotVec.length() ) ) {
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
                tmpPivotVec[(dominantIndex + 2) % 3] =  0.0f;
            }
            return setFromAngleNormalAxis(theta, tmpPivotVec);
        }
    }

    /***
     * Initialize this quaternion with given non-normalized axis vector and rotation angle
     * <p>
     * Implementation Details:
     * <ul>
     *   <li> {@link #setIdentity()} if axis is {@link jau::is_zero(float, float) is zero} using {@link FloatUtil#EPSILON epsilon}</li>
     * </ul>
     * </p>
     * @param angle rotation angle (rads)
     * @param vector axis vector not normalized
     * @param tmpV3f temp storage to normalize vector
     * @return this quaternion for chaining.
     *
     * @see <a href="http://web.archive.org/web/20041029003853/http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q56">Matrix-FAQ Q56</a>
     * @see #toAngleAxis(Vec3f)
     */
    Quaternion& setFromAngleAxis(const float angle, const Vec3f& vector, Vec3f& tmpV3f) noexcept {
        ( tmpV3f = vector ).normalize();
        return setFromAngleNormalAxis(angle, tmpV3f);
    }

    /***
     * Initialize this quaternion with given normalized axis vector and rotation angle
     * <p>
     * Implementation Details:
     * <ul>
     *   <li> {@link #setIdentity()} if axis is {@link jau::is_zero(float, float) is zero} using {@link FloatUtil#EPSILON epsilon}</li>
     * </ul>
     * </p>
     * @param angle rotation angle (rads)
     * @param vector axis vector normalized
     * @return this quaternion for chaining.
     *
     * @see <a href="http://web.archive.org/web/20041029003853/http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q56">Matrix-FAQ Q56</a>
     * @see #toAngleAxis(Vec3f)
     */
    Quaternion& setFromAngleNormalAxis(const float angle, const Vec3f& vector) noexcept {
        if( vector.is_zero() ) {
            set_identity();
        } else {
            const float halfangle = angle * 0.5f;
            const float sin = std::sin(halfangle);
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
     * @see #setFromAngleAxis(float, Vec3f, Vec3f)
     */
    float toAngleAxis(Vec3f& axis) const noexcept {
        const float sqrLength = m_x*m_x + m_y*m_y + m_z*m_z;
        float angle;
        if ( jau::is_zero(sqrLength) ) { // length is ~0
            angle = 0.0f;
            axis.set( 1.0f, 0.0f, 0.0f );
        } else {
            angle = std::acos(m_w) * 2.0f;
            const float invLength = 1.0f / std::sqrt(sqrLength);
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
     * For details see {@link #setFromEuler(float, float, float)}.
     * @param angradXYZ euler angle vector in radians holding x-bank, m_y-heading and m_z-attitude
     * @return this quaternion for chaining.
     * @see #setFromEuler(float, float, float)
     */
    Quaternion& setFromEuler(const Vec3f& angradXYZ) noexcept {
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
     *   <li> {@link #setIdentity()} if all angles are {@link jau::is_zero(float, float) is zero} using {@link FloatUtil#EPSILON epsilon}</li>
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
     * @see #toEuler(Vec3f)
     */
    Quaternion& setFromEuler(const float bankX, const float headingY, const float attitudeZ) noexcept {
        if ( jau::is_zero3f(bankX, headingY, attitudeZ) ) {
            return set_identity();
        } else {
            float angle = headingY * 0.5f;
            const float sinHeadingY = std::sin(angle);
            const float cosHeadingY = std::cos(angle);
            angle = attitudeZ * 0.5f;
            const float sinAttitudeZ = std::sin(angle);
            const float cosAttitudeZ = std::cos(angle);
            angle = bankX * 0.5f;
            const float sinBankX = std::sin(angle);
            const float cosBankX = std::cos(angle);

            // variables used to reduce multiplication calls.
            const float cosHeadingXcosAttitude = cosHeadingY * cosAttitudeZ;
            const float sinHeadingXsinAttitude = sinHeadingY * sinAttitudeZ;
            const float cosHeadingXsinAttitude = cosHeadingY * sinAttitudeZ;
            const float sinHeadingXcosAttitude = sinHeadingY * cosAttitudeZ;

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
     * @param result euler angle result vector for radians x-bank, y-heading and z-attitude
     * @return the Vec3f `result` filled with x-bank, y-heading and m_z-attitude
     * @see <a href="http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/index.htm">euclideanspace.com-quaternionToEuler</a>
     * @see #setFromEuler(float, float, float)
     */
    Vec3f& toEuler(Vec3f& result) noexcept {
        const float sqw = m_w*m_w;
        const float sqx = m_x*m_x;
        const float sqy = m_y*m_y;
        const float sqz = m_z*m_z;
        const float unit = sqx + sqy + sqz + sqw; // if normalized is one, otherwise, is correction factor
        const float test = m_x*m_y + m_z*m_w;

        if (test > 0.499f * unit) { // singularity at north pole
            result.set( 0.0f,                        // m_x-bank
                        2.0f * std::atan2(m_x, m_w), // y-heading
                        M_PI_2 );                    // z-attitude
        } else if (test < -0.499f * unit) { // singularity at south pole
            result.set( 0.0f,                        // m_x-bank
                       -2.0 * std::atan2(m_x, m_w),  // m_y-heading
                       -M_PI_2 );                    // m_z-attitude
        } else {
            result.set( std::atan2(2.0f * m_x * m_w - 2.0f * m_y * m_z, -sqx + sqy - sqz + sqw), // m_x-bank
                        std::atan2(2.0f * m_y * m_w - 2.0f * m_x * m_z,  sqx - sqy - sqz + sqw), // m_y-heading
                        std::asin( 2.0f * test / unit) );                                        // z-attitude
        }
        return result;
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
    Quaternion& setFromMat3(const float m00, const float m01, const float m02,
                            const float m10, const float m11, const float m12,
                            const float m20, const float m21, const float m22) noexcept {
        // Note: Other implementations uses 'T' w/o '+1f' and compares 'T >= 0' while adding missing 1f in sqrt expr.
        //       However .. this causes setLookAt(..) to fail and actually violates the 'trace definition'.

        // The trace T is the sum of the diagonal elements; see
        // http://mathworld.wolfram.com/MatrixTrace.html
        const float T = m00 + m11 + m22 + 1.0f;
        if ( T > 0.0f ) {
            const float S = 0.5f / std::sqrt(T);  // S = 1 / ( 2 t )
            m_w = 0.25f / S;                      // m_w = 1 / ( 4 S ) = t / 2
            m_x = ( m21 - m12 ) * S;
            m_y = ( m02 - m20 ) * S;
            m_z = ( m10 - m01 ) * S;
        } else if ( m00 > m11 && m00 > m22) {
            const float S = 0.5f / std::sqrt(1.0f + m00 - m11 - m22); // S=4*qx
            m_w = ( m21 - m12 ) * S;
            m_x = 0.25f / S;
            m_y = ( m10 + m01 ) * S;
            m_z = ( m02 + m20 ) * S;
        } else if ( m11 > m22 ) {
            const float S = 0.5f / std::sqrt(1.0f + m11 - m00 - m22); // S=4*qy
            m_w = ( m02 - m20 ) * S;
            m_x = ( m20 + m01 ) * S;
            m_y = 0.25f / S;
            m_z = ( m21 + m12 ) * S;
        } else {
            const float S = 0.5f / std::sqrt(1.0f + m22 - m00 - m11); // S=4*qz
            m_w = ( m10 - m01 ) * S;
            m_x = ( m02 + m20 ) * S;
            m_y = ( m21 + m12 ) * S;
            m_z = 0.25f / S;
        }
        return *this;
    }

    /**
     * Compute the quaternion from a 3x3 column rotation matrix from mat4f instance
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
     * @see #setFromMatrix(float, float, float, float, float, float, float, float, float)
     */
    Quaternion& setFromMat3(const Mat4f& m) noexcept {
        return setFromMat3(m.m00, m.m01, m.m02, m.m10, m.m11, m.m12, m.m20, m.m21, m.m22);
    }

    /**
     * Transform this quaternion to a normalized 4x4 column matrix representing the rotation.
     * <p>
     * Implementation Details:
     * <ul>
     *   <li> makes identity matrix if {@link #magnitudeSquared()} is {@link jau::is_zero(float, float) is zero} using {@link FloatUtil#EPSILON epsilon}</li>
     * </ul>
     * </p>
     *
     * @param matrix store for the resulting normalized column matrix 4x4
     * @return the given matrix store
     * @see <a href="http://web.archive.org/web/20041029003853/http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q54">Matrix-FAQ Q54</a>
     * @see #setFromMatrix(float, float, float, float, float, float, float, float, float)
     * @see Matrix4f#setToRotation(Quaternion)
     */
    Mat4f& toMatrix(Mat4f& matrix) noexcept {
        return matrix.setToRotation(*this);
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
    Quaternion& setFromAxes(const Vec3f& xAxis, const Vec3f& yAxis, const Vec3f& zAxis) noexcept {
        return setFromMat3(xAxis.x, yAxis.x, zAxis.x,
                           xAxis.y, yAxis.y, zAxis.y,
                           xAxis.z, yAxis.z, zAxis.z);
    }

    /**
     * Extracts this quaternion's <i>orthogonal</i> rotation axes.
     *
     * @param xAxis vector representing the <i>orthogonal</i> x-axis of the coordinate system.
     * @param yAxis vector representing the <i>orthogonal</i> y-axis of the coordinate system.
     * @param zAxis vector representing the <i>orthogonal</i> m_z-axis of the coordinate system.
     * @param tmpMat4 temporary float[4] matrix, used to transform this quaternion to a matrix.
     */
    void toAxes(Vec3f& xAxis, Vec3f& yAxis, Vec3f& zAxis, Mat4f& tmpMat4) const noexcept {
        tmpMat4.setToRotation(*this);
        tmpMat4.getColumn(2, zAxis);
        tmpMat4.getColumn(1, yAxis);
        tmpMat4.getColumn(0, xAxis);
    }

    //
    // std overrides
    //

    /**
     * @param o the object to compare for equality
     * @return true if this quaternion and the provided quaternion have roughly the same x, m_y, m_z and m_w values.
     */
    constexpr bool equals(const Quaternion& o) const noexcept {
        if (this == &o) {
            return true;
        }
        return std::abs(m_x - o.m_x) <= ALLOWED_DEVIANCE &&
               std::abs(m_y - o.m_y) <= ALLOWED_DEVIANCE &&
               std::abs(m_z - o.m_z) <= ALLOWED_DEVIANCE &&
               std::abs(m_w - o.m_w) <= ALLOWED_DEVIANCE;
    }

    std::string toString() const noexcept {
        return "Quat[m_x "+std::to_string(m_x)+", y "+std::to_string(m_y)+", z "+std::to_string(m_z)+", w "+std::to_string(m_w)+"]";
    }
};

constexpr bool operator==(const Quaternion& lhs, const Quaternion& rhs ) noexcept {
    return lhs.equals(rhs);
}

/**@}*/

} // namespace jau::math

#endif // QUATERNION_HPP_
