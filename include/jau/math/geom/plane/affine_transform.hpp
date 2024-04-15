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
#ifndef JAU_AFFINETRANSFORM_HPP_
#define JAU_AFFINETRANSFORM_HPP_

#include <jau/float_math.hpp>
#include <jau/math/math_error.hpp>
#include <jau/math/vec2f.hpp>
#include <jau/math/vec3f.hpp>
#include <jau/math/vec4f.hpp>
#include <jau/math/aabbox3f.hpp>

namespace jau::math::geom::plane {

    /** \addtogroup Math
     *
     *  @{
     */

    enum class AffineTransformType : int {
        /** The <code>AffineTransformType::TYPE_UNKNOWN</code> is an initial type_t value */
        UNKNOWN = -1,
        IDENTITY = 0,
        TRANSLATION = 1,
        UNIFORM_SCALE = 2,
        GENERAL_SCALE = 4,
        QUADRANT_ROTATION = 8,
        GENERAL_ROTATION = 16,
        GENERAL_TRANSFORM = 32,
        FLIP = 64,
        MASK_SCALE = UNIFORM_SCALE | GENERAL_SCALE,
        MASK_ROTATION = QUADRANT_ROTATION | GENERAL_ROTATION
    };
    constexpr static int number(const AffineTransformType rhs) noexcept {
        return static_cast<int>(rhs);
    }
    constexpr static AffineTransformType operator ^(const AffineTransformType lhs, const AffineTransformType rhs) noexcept {
        return static_cast<AffineTransformType> ( number(lhs) ^ number(rhs) );
    }
    constexpr static AffineTransformType operator |(const AffineTransformType lhs, const AffineTransformType rhs) noexcept {
        return static_cast<AffineTransformType> ( number(lhs) | number(rhs) );
    }
    constexpr static AffineTransformType& operator |=(AffineTransformType& lhs, const AffineTransformType rhs) noexcept {
        lhs = static_cast<AffineTransformType> ( number(lhs) | number(rhs) );
        return lhs;
    }
    constexpr static AffineTransformType operator &(const AffineTransformType lhs, const AffineTransformType rhs) noexcept {
        return static_cast<AffineTransformType> ( number(lhs) & number(rhs) );
    }
    constexpr static bool operator ==(const AffineTransformType lhs, const AffineTransformType rhs) noexcept {
        return number(lhs) == number(rhs);
    }
    constexpr static bool operator !=(const AffineTransformType lhs, const AffineTransformType rhs) noexcept {
        return !( lhs == rhs );
    }
    constexpr static bool is_set(const AffineTransformType mask, const AffineTransformType bit) noexcept {
        return bit == ( mask & bit );
    }

    /**
     * Represents a affine 2x3 transformation matrix in column major order (memory layout).
     *
     * Field notation row-column: m10 (row 1, column 0).
     */
    class AffineTransform {
      private:
            // 2x3 matrix in column major order, notation row-column: m10 (row 1, column 0)

            /** scale-x */
            float m00;
            /** shear-y */
            float m10;
            /** shear-x */
            float m01;
            /** scale-x */
            float m11;
            /** translate-x */
            float m02;
            /** translate-y */
            float m12;

            AffineTransformType m_type;

            constexpr static const char* determinantIsZero = "Zero Determinante";

      public:

        /** The min absolute value equivalent to zero, aka EPSILON. */
        constexpr static const float ZERO = (float)1E-10;

        /** Initialized to identity. */
        constexpr AffineTransform() noexcept
        : m00(1.0f), m10(0.0f),
          m01(0.0f), m11(1.0f),
          m02(0.0f), m12(0.0f),
          m_type(AffineTransformType::IDENTITY)
        { }

        constexpr AffineTransform(const float m00_, const float m10_, const float m01_, const float m11_, const float m02_, const float m12_) noexcept
        : m00(m00_), m10(m10_),
          m01(m01_), m11(m11_),
          m02(m02_), m12(m12_),
          m_type(AffineTransformType::UNKNOWN)
        { }

        /**
         * @param mat2xN either a 2x2 or 2x3 matrix depending on mat_len
         * @param mat_len either 6 for 2x3 matrix or 4 for 2x2 matrix
         */
        AffineTransform(const float mat2xN[], const jau::nsize_t mat_len) noexcept
        : m00(mat2xN[0]), m10(mat2xN[1]),
          m01(mat2xN[2]), m11(mat2xN[3]),
          m02(0.0f),      m12(0.0f),
          m_type(AffineTransformType::UNKNOWN)
        {
            if (mat_len > 4) {
                m02 = mat2xN[4];
                m12 = mat2xN[5];
            }
        }

        constexpr AffineTransform(const AffineTransform& o) noexcept = default;
        constexpr AffineTransform(AffineTransform&& o) noexcept = default;
        constexpr AffineTransform& operator=(const AffineTransform&) noexcept = default;
        constexpr AffineTransform& operator=(AffineTransform&&) noexcept = default;

        /*
         * Method returns type of affine transformation.
         *
         * Transform matrix is
         *   m00 m01 m02
         *   m10 m11 m12
         *
         * According analytic geometry new basis vectors are (m00, m01) and (m10, m11),
         * translation vector is (m02, m12). Original basis vectors are (1, 0) and (0, 1).
         * Type transformations classification:
         *   AffineTransformType::TYPE_IDENTITY - new basis equals original one and zero translation
         *   AffineTransformType::TYPE_TRANSLATION - translation vector isn't zero
         *   AffineTransformType::TYPE_UNIFORM_SCALE - vectors length of new basis equals
         *   AffineTransformType::TYPE_GENERAL_SCALE - vectors length of new basis doesn't equal
         *   AffineTransformType::TYPE_FLIP - new basis vector orientation differ from original one
         *   AffineTransformType::TYPE_QUADRANT_ROTATION - new basis is rotated by 90, 180, 270, or 360 degrees
         *   AffineTransformType::TYPE_GENERAL_ROTATION - new basis is rotated by arbitrary angle
         *   AffineTransformType::TYPE_GENERAL_TRANSFORM - transformation can't be inversed
         */
        AffineTransformType getType() const noexcept {
            if (m_type != AffineTransformType::UNKNOWN) {
                return m_type;
            }

            AffineTransformType type = AffineTransformType::IDENTITY;

            if (m00 * m01 + m10 * m11 != 0.0f) {
                type |= AffineTransformType::GENERAL_TRANSFORM;
                return type;
            }

            if ( !jau::is_zero(m02) || !jau::is_zero(m12) ) {
                type |= AffineTransformType::TRANSLATION;
            } else if ( jau::equals(m00, 1.0f) && jau::equals(m11, 1.0f) &&
                        jau::is_zero(m01) && jau::is_zero(m10) ) {
                type = AffineTransformType::IDENTITY;
                return type;
            }

            if (m00 * m11 - m01 * m10 < 0.0f) {
                type |= AffineTransformType::FLIP;
            }

            const float dx = m00 * m00 + m10 * m10;
            const float dy = m01 * m01 + m11 * m11;
            if ( !jau::equals(dx, dy) ) {
                type |= AffineTransformType::GENERAL_SCALE;
            } else if ( !jau::equals(dx, 1.0f) ) {
                type |= AffineTransformType::UNIFORM_SCALE;
            }

            if ( ( jau::is_zero( m00 ) && jau::is_zero( m11 ) ) ||
                 ( jau::is_zero( m10 ) && jau::is_zero( m01 ) && (m00 < 0.0f || m11 < 0.0f) ) )
            {
                type |= AffineTransformType::QUADRANT_ROTATION;
            } else if ( !jau::is_zero( m01 ) || !jau::is_zero( m10 ) ) {
                type |= AffineTransformType::GENERAL_ROTATION;
            }
            return type;
        }

        float scaleX() const noexcept { return m00; }

        float scaleY() const noexcept { return m11; }

        float shearX() const noexcept { return m01; }

        float shearY() const noexcept { return m10; }

        float translateX() const noexcept { return m02; }

        float translateY() const noexcept { return m12; }

        bool isIdentity() const noexcept { return getType() == AffineTransformType::IDENTITY; }

        /**
         * @param mat2xN either a 2x2 or 2x3 matrix depending on mat_len
         * @param mat_len either 6 for 2x3 matrix or 4 for 2x2 matrix
         */
        void getMatrix(float mat2xN[], const jau::nsize_t mat_len) const noexcept {
            mat2xN[0] = m00;
            mat2xN[1] = m10;
            mat2xN[2] = m01;
            mat2xN[3] = m11;
            if (mat_len > 4) {
                mat2xN[4] = m02;
                mat2xN[5] = m12;
            }
        }

        float determinant() const noexcept { return m00 * m11 - m01 * m10; }

        AffineTransform& set(const float m00_, const float m10_, const float m01_, const float m11_, const float m02_, const float m12_) noexcept {
            m_type = AffineTransformType::UNKNOWN;
            m00 = m00_;
            m10 = m10_;
            m01 = m01_;
            m11 = m11_;
            m02 = m02_;
            m12 = m12_;
            return *this;
        }

        AffineTransform& setToIdentity() noexcept {
            m_type = AffineTransformType::IDENTITY;
            m00 = m11 = 1.0f;
            m10 = m01 = m02 = m12 = 0.0f;
            return *this;
        }

        AffineTransform& setToTranslation(const float mx, const float my) noexcept {
            m00 = m11 = 1.0f;
            m01 = m10 = 0.0f;
            m02 = mx;
            m12 = my;
            if ( jau::is_zero(mx) && jau::is_zero(my) ) {
                m_type = AffineTransformType::IDENTITY;
            } else {
                m_type = AffineTransformType::TRANSLATION;
            }
            return *this;
        }

        AffineTransform& setToScale(const float scx, const float scy) noexcept {
            m00 = scx;
            m11 = scy;
            m10 = m01 = m02 = m12 = 0.0f;
            if ( !jau::equals(scx, 1.0f) || !jau::equals(scy, 1.0f) ) {
                m_type = AffineTransformType::UNKNOWN;
            } else {
                m_type = AffineTransformType::IDENTITY;
            }
            return *this;
        }

        AffineTransform& setToShear(const float shx, const float shy) noexcept {
            m00 = m11 = 1.0f;
            m02 = m12 = 0.0f;
            m01 = shx;
            m10 = shy;
            if ( !jau::is_zero(shx) || !jau::is_zero(shy) ) {
                m_type = AffineTransformType::UNKNOWN;
            } else {
                m_type = AffineTransformType::IDENTITY;
            }
            return *this;
        }

        AffineTransform& setToRotation(const float angle) noexcept {
            float sin = std::sin(angle);
            float cos = std::cos(angle);
            if (std::abs(cos) < ZERO) {
                cos = 0.0f;
                sin = sin > 0.0f ? 1.0f : -1.0f;
            } else if (std::abs(sin) < ZERO) {
                sin = 0.0f;
                cos = cos > 0.0f ? 1.0f : -1.0f;
            }
            m00 = m11 = cos;
            m01 = -sin;
            m10 = sin;
            m02 = m12 = 0.0f;
            m_type = AffineTransformType::UNKNOWN;
            return *this;
        }

        AffineTransform& setToRotation(const float angle, const float px, const float py) noexcept {
            setToRotation(angle);
            m02 = px * (1.0f - m00) + py * m10;
            m12 = py * (1.0f - m00) - px * m10;
            m_type = AffineTransformType::UNKNOWN;
            return *this;
        }

        AffineTransform& translate(const float mx, const float my, AffineTransform& tmp) noexcept {
            return concat(tmp.setToTranslation(mx, my));
        }

        AffineTransform& scale(const float scx, const float scy, AffineTransform& tmp) noexcept {
            return concat(tmp.setToScale(scx, scy));
        }

        AffineTransform& shear(const float shx, const float shy, AffineTransform& tmp) noexcept {
            return concat(tmp.setToShear(shx, shy));
        }

        AffineTransform& rotate(const float angle, AffineTransform& tmp) noexcept {
            return concat(tmp.setToRotation(angle));
        }

        AffineTransform& rotate(const float angle, const float px, const float py, AffineTransform& tmp) noexcept {
            return concat(tmp.setToRotation(angle, px, py));
        }

        /**
         * Multiply matrix of two AffineTransform objects.
         * @param tL - the AffineTransform object is a multiplicand (left argument)
         * @param tR - the AffineTransform object is a multiplier (right argument)
         *
         * @return A new AffineTransform object containing the result of [tL] X [tR].
         */
        static AffineTransform mul(const AffineTransform& tL, const AffineTransform& tR) noexcept {
            return AffineTransform(
                    tR.m00 * tL.m00 + tR.m10 * tL.m01,          // m00
                    tR.m00 * tL.m10 + tR.m10 * tL.m11,          // m10
                    tR.m01 * tL.m00 + tR.m11 * tL.m01,          // m01
                    tR.m01 * tL.m10 + tR.m11 * tL.m11,          // m11
                    tR.m02 * tL.m00 + tR.m12 * tL.m01 + tL.m02, // m02
                    tR.m02 * tL.m10 + tR.m12 * tL.m11 + tL.m12);// m12
        }

        /**
         * Concatenates the given matrix to this.
         * <p>
         * Implementations performs the matrix multiplication:
         * <pre>
         *   [this] = [this] X [tR]
         * </pre>
         * </p>
         * @param tR the right-argument of the matrix multiplication
         * @return this transform for chaining
         */
        AffineTransform& concat(const AffineTransform& tR) noexcept {
            // set(mul(this, tR));
            m_type = AffineTransformType::UNKNOWN;
            set( tR.m00 * m00 + tR.m10 * m01,       // m00
                 tR.m00 * m10 + tR.m10 * m11,       // m10
                 tR.m01 * m00 + tR.m11 * m01,       // m01
                 tR.m01 * m10 + tR.m11 * m11,       // m11
                 tR.m02 * m00 + tR.m12 * m01 + m02, // m02
                 tR.m02 * m10 + tR.m12 * m11 + m12);// m12
            return *this;
        }

        /**
         * Pre-concatenates the given matrix to this.
         * <p>
         * Implementations performs the matrix multiplication:
         * <pre>
         *   [this] = [tL] X [this]
         * </pre>
         * </p>
         * @param tL the left-argument of the matrix multiplication
         * @return this transform for chaining
         */
        AffineTransform& preConcat(const AffineTransform& tL) noexcept {
            // setTransform(multiply(tL, this));
            m_type = AffineTransformType::UNKNOWN;
            set( m00 * tL.m00 + m10 * tL.m01,          // m00
                 m00 * tL.m10 + m10 * tL.m11,          // m10
                 m01 * tL.m00 + m11 * tL.m01,          // m01
                 m01 * tL.m10 + m11 * tL.m11,          // m11
                 m02 * tL.m00 + m12 * tL.m01 + tL.m02, // m02
                 m02 * tL.m10 + m12 * tL.m11 + tL.m12);// m12
            return *this;
        }

        AffineTransform createInverse() const {
            const float det = determinant();
            if (std::abs(det) < ZERO) {
                throw MathDomainError(determinantIsZero, E_FILE_LINE);
            }
            return AffineTransform(
                     m11 / det, // m00
                    -m10 / det, // m10
                    -m01 / det, // m01
                     m00 / det, // m11
                    (m01 * m12 - m11 * m02) / det, // m02
                    (m10 * m02 - m00 * m12) / det  // m12
            );
        }

        /**
         *
         * @param src
         * @param dst
         * @return dst for chaining
         */
        AABBox3f& transform(const AABBox3f& src, AABBox3f& dst) const noexcept {
            const Vec3f& lo = src.low();
            const Vec3f& hi = src.high();
            dst.setSize(lo.x * m00 + lo.y * m01 + m02, lo.x * m10 + lo.y * m11 + m12, lo.z,
                        hi.x * m00 + hi.y * m01 + m02, hi.x * m10 + hi.y * m11 + m12, hi.z);
            return dst;
        }

        /**
         * @param src
         * @param dst
         * @return dst for chaining
        Vertex& transform(const Vertex& src, const Vertex& dst) const noexcept {
            const float x = src.x();
            const float y = src.y();
            dst.setCoord(x * m00 + y * m01 + m02, x * m10 + y * m11 + m12, src.z());
            return dst;
        }
        void transform(const Vertex src[], const Vertex dst[], size_t length) const noexcept {
            size_t srcOff=0;
            size_t dstOff=0;
            while (--length >= 0) {
                const Vertex& srcPoint = src[srcOff++];
                Vertex& dstPoint = dst[dstOff++];
                const float x = srcPoint.x();
                const float y = srcPoint.y();
                dstPoint.setCoord(x * m00 + y * m01 + m02, x * m10 + y * m11 + m12, srcPoint.z());
            }
        } */

        /**
         * @param src float[2] source of transformation
         * @param dst float[2] destination of transformation, maybe be equal to <code>src</code>
         * @return dst for chaining
         */
        float* transform(const float src[], float dst[]) const noexcept {
            const float x = src[0];
            const float y = src[1];
            dst[0] = x * m00 + y * m01 + m02;
            dst[1] = x * m10 + y * m11 + m12;
            return dst;
        }

        void transform(const float src[], float dst[], size_t length) const noexcept {
            const float* src_end = src + length * 2;
            if (src <= dst && dst < src_end ) {
                // overlap -> reverse
                size_t srcOff = length * 2 - 2;
                size_t dstOff = length * 2 - 2;
                while (length-- > 0) {
                    const float x = src[srcOff + 0];
                    const float y = src[srcOff + 1];
                    dst[dstOff + 0] = x * m00 + y * m01 + m02;
                    dst[dstOff + 1] = x * m10 + y * m11 + m12;
                    srcOff -= 2;
                    dstOff -= 2;
                }
            } else {
                size_t srcOff = 0;
                size_t dstOff = 0;
                while (length-- > 0) {
                    const float x = src[srcOff++];
                    const float y = src[srcOff++];
                    dst[dstOff++] = x * m00 + y * m01 + m02;
                    dst[dstOff++] = x * m10 + y * m11 + m12;
                }
            }
        }

        /**
         * @param src source of transformation
         * @param dst destination of transformation, maybe be equal to <code>src</code>
         * @return dst for chaining
         */
        Vec2f& transform(const Vec2f& src, Vec2f& dst) const noexcept {
            const float x = src.x;
            const float y = src.y;
            dst.x = x * m00 + y * m01 + m02;
            dst.y = x * m10 + y * m11 + m12;
            return dst;
        }

        /**
         * @param src source of transformation
         * @param dst destination of transformation, maybe be equal to <code>src</code>
         * @return dst for chaining
         */
        Vec3f& transform(const Vec3f& src, Vec3f& dst) const noexcept {
            const float x = src.x;
            const float y = src.y;
            dst.x = x * m00 + y * m01 + m02;
            dst.y = x * m10 + y * m11 + m12;
            dst.z = src.z; // just copy z
            return dst;
        }

        /**
         *
         * @param src
         * @param dst
         * @return return dst for chaining
        Vertex& deltaTransform(const Vertex& src, Vertex& dst) const noexcept {
            const float x = src.x();
            const float y = src.y();
            dst.setCoord(x * m00 + y * m01, x * m10 + y * m11, src.z());
            return dst;
        }
         */

        void deltaTransform(const float src[], float dst[], size_t length) const noexcept {
            size_t srcOff = 0;
            size_t dstOff = 0;
            while (length-- > 0) {
                const float x = src[srcOff++];
                const float y = src[srcOff++];
                dst[dstOff++] = x * m00 + y * m01;
                dst[dstOff++] = x * m10 + y * m11;
            }
        }

        /**
         *
         * @param src
         * @param dst
         * @return return dst for chaining
         * @throws NoninvertibleTransformException
        Vertex& inverseTransform(const Vertex& src, Vertex& dst) const {
            const float det = getDeterminant();
            if (std::abs(det) < ZERO) {
                throw new NoninvertibleTransformException(determinantIsZero);
            }
            const float x = src.x() - m02;
            const float y = src.y() - m12;
            dst.setCoord((x * m11 - y * m01) / det, (y * m00 - x * m10) / det, src.z());
            return dst;
        } */

        void inverseTransform(const float src[], float dst[], size_t length) const {
            size_t srcOff = 0;
            size_t dstOff = 0;
            const float det = determinant();
            if (std::abs(det) < ZERO) {
                throw MathDomainError(determinantIsZero, E_FILE_LINE);
            }
            while (length-- > 0) {
                const float x = src[srcOff++] - m02;
                const float y = src[srcOff++] - m12;
                dst[dstOff++] = (x * m11 - y * m01) / det;
                dst[dstOff++] = (y * m00 - x * m10) / det;
            }
        }

        std::string toString() const noexcept {
            return "AffineTransform[[" + std::to_string(m00) + ", " + std::to_string(m01) + ", " + std::to_string(m02) + "], ["
                                       + std::to_string(m10) + ", " + std::to_string(m11) + ", " + std::to_string(m12) + "]]";
        }

        constexpr bool equals(const AffineTransform& o, const float epsilon=std::numeric_limits<float>::epsilon()) const noexcept {
            if( this == &o ) {
                return true;
            } else {
                return jau::equals(m00, o.m00, epsilon) &&
                       jau::equals(m01, o.m01, epsilon) &&
                       jau::equals(m02, o.m02, epsilon) &&
                       jau::equals(m10, o.m10, epsilon) &&
                       jau::equals(m11, o.m11, epsilon) &&
                       jau::equals(m12, o.m12, epsilon);
            }
        }
        constexpr bool operator==(const AffineTransform& rhs) const noexcept {
            return equals(rhs);
        }
    };

/**@}*/

} // namespace jau::math::geom::plane

#endif /*  JAU_AFFINETRANSFORM_HPP_ */

