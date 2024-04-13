/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2014-2024 Gothel Software e.K.
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
#ifndef MAT4f_HPP_
#define MAT4f_HPP_

#include <cmath>
#include <cstdarg>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>
#include <iostream>

#include <jau/float_math.hpp>
#include <jau/math/math_error.hpp>
#include <jau/math/vec3f.hpp>
#include <jau/math/vec4f.hpp>
#include <jau/math/recti.hpp>
#include <jau/math/fov_hv_halves.hpp>

namespace jau::math::geom {
    class Frustum; // forward
}

namespace jau::math {

    /** \addtogroup Math
     *
     *  @{
     */

    class Quaternion; // forward

/**
 * Basic 4x4 float matrix implementation using fields for intensive use-cases (host operations).
 * <p>
 * Implementation covers {@link FloatUtil} matrix functionality, exposed in an object oriented manner.
 * </p>
 * <p>
 * Unlike {@link com.jogamp.math.util.PMVmat4f PMVmat4f}, this class only represents one single matrix.
 * </p>
 * <p>
 * For array operations the layout is expected in column-major order
 * matching OpenGL's implementation, illustration:
 * <pre>
    Row-Major                       Column-Major (OpenGL):

        |  0   1   2  tx |
        |                |
        |  4   5   6  ty |
    M = |                |
        |  8   9  10  tz |
        |                |
        | 12  13  14  15 |

           R   C                      R   C
         m[0*4+3] = tx;             m[0+4*3] = tx;
         m[1*4+3] = ty;             m[1+4*3] = ty;
         m[2*4+3] = tz;             m[2+4*3] = tz;

          RC (std subscript order)   RC (std subscript order)
         m03 = tx;                  m03 = tx;
         m13 = ty;                  m13 = ty;
         m23 = tz;                  m23 = tz;

 * </pre>
 * </p>
 * <p>
 * <ul>
 *   <li><a href="http://web.archive.org/web/20041029003853/http://www.j3d.org/matrix_faq/matrfaq_latest.html">Matrix-FAQ</a></li>
 *   <li><a href="https://en.wikipedia.org/wiki/Matrix_%28mathematics%29">Wikipedia-Matrix</a></li>
 *   <li><a href="http://www.euclideanspace.com/maths/algebra/matrix/index.htm">euclideanspace.com-Matrix</a></li>
 * </ul>
 * </p>
 * <p>
 * Implementation utilizes unrolling of small vertices and matrices wherever possible
 * while trying to access memory in a linear fashion for performance reasons, see:
 * <ul>
 *   <li><a href="https://lessthanoptimal.github.io/Java-Matrix-Benchmark/">java-matrix-benchmark</a></li>
 *   <li><a href="https://github.com/lessthanoptimal/ejml">EJML Efficient Java Matrix Library</a></li>
 * </ul>
 * </p>
 */
class Mat4f {
    private:
        float m00, m10, m20, m30;
        float m01, m11, m21, m31;
        float m02, m12, m22, m32;
        float m03, m13, m23, m33;

        friend geom::Frustum;
        friend Quaternion;

        class Stack {
            private:
                int growSize;
                std::vector<float> buffer;

            public:
                /**
                 * @param initialSize initial size
                 * @param growSize grow size if {@link #position()} is reached, maybe <code>0</code>
                 */
                Stack(int initialSize, int growSize_)
                : growSize(growSize_), buffer(initialSize) {}

                size_t growIfNecessary(int length) {
                    const size_t p = buffer.size();
                    const size_t nsz = buffer.size() + length;
                    if( nsz > buffer.capacity() ) {
                        buffer.reserve(buffer.size() + std::max(length, growSize));
                    }
                    buffer.resize(nsz);
                    return p;
                }

                Mat4f& push(Mat4f& src) {
                    size_t p = growIfNecessary(16);
                    src.get(buffer, p);
                    return src;
                }

                Mat4f& pop(Mat4f& dest) {
                    size_t sz = buffer.size();
                    if( sz < 16 ) {
                        throw jau::IndexOutOfBoundsException(0, sz, E_FILE_LINE);
                    }
                    size_t p = sz - 16;
                    dest.load(buffer, p);
                    buffer.resize(p);
                    return dest;
                }
        };

        Stack stack; // start w/ zero size, growSize is half GL-min size (32)

  public:

    /**
     * Creates a new identity matrix.
     */
    Mat4f() noexcept
    : m00(1.0f), m10(0.0f), m20(0.0f), m30(0.0f),
      m01(0.0f), m11(1.0f), m21(0.0f), m31(0.0f),
      m02(0.0f), m12(0.0f), m22(1.0f), m32(0.0f),
      m03(0.0f), m13(0.0f), m23(0.0f), m33(1.0f),
      stack(0, 16*16)
    { }

    /**
     * Creates a new matrix based on given float[4*4] column major order.
     * @param m 4x4 matrix in column-major order
     */
    Mat4f(const float m[]) noexcept
    : stack(0, 16*16)
    {
        load(m);
    }

    /**
     * Creates a new matrix based on given std::vector 4x4 column major order.
     * @param m 4x4 matrix std::vector in column-major order starting at {@code src_off}
     * @param m_off offset for matrix {@code src}
     */
    Mat4f(const std::vector<float>& m, const size_t m_off) noexcept
    : stack(0, 16*16)
    {
        load(m, m_off);
    }

    /**
     * Creates a new matrix copying the values of the given {@code src} matrix.
     *
     * The stack is not copied.
     */
    Mat4f(const Mat4f& o) noexcept
    : m00(o.m00), m10(o.m10), m20(o.m20), m30(o.m30),
      m01(o.m01), m11(o.m11), m21(o.m21), m31(o.m31),
      m02(o.m02), m12(o.m12), m22(o.m22), m32(o.m32),
      m03(o.m03), m13(o.m13), m23(o.m23), m33(o.m33),
      stack(0, 16*16) { }

    /**
     * Copy assignment using the the values of the given {@code src} matrix.
     *
     * The stack is not copied.
     */
    Mat4f& operator=(const Mat4f& o) noexcept {
        m00 = o.m00; m10 = o.m10; m20 = o.m20; m30 = o.m30;
        m01 = o.m01; m11 = o.m11; m21 = o.m21; m31 = o.m31;
        m02 = o.m02; m12 = o.m12; m22 = o.m22; m32 = o.m32;
        m03 = o.m03; m13 = o.m13; m23 = o.m23; m33 = o.m33;
        return *this;
    }

    constexpr bool equals(const Mat4f& o, const float epsilon=std::numeric_limits<float>::epsilon()) const noexcept {
        if( this == &o ) {
            return true;
        } else {
            return jau::equals(m00, o.m00, epsilon) &&
                   jau::equals(m01, o.m01, epsilon) &&
                   jau::equals(m02, o.m02, epsilon) &&
                   jau::equals(m03, o.m03, epsilon) &&
                   jau::equals(m10, o.m10, epsilon) &&
                   jau::equals(m11, o.m11, epsilon) &&
                   jau::equals(m12, o.m12, epsilon) &&
                   jau::equals(m13, o.m13, epsilon) &&
                   jau::equals(m20, o.m20, epsilon) &&
                   jau::equals(m21, o.m21, epsilon) &&
                   jau::equals(m22, o.m22, epsilon) &&
                   jau::equals(m23, o.m23, epsilon) &&
                   jau::equals(m30, o.m30, epsilon) &&
                   jau::equals(m31, o.m31, epsilon) &&
                   jau::equals(m32, o.m32, epsilon) &&
                   jau::equals(m33, o.m33, epsilon);
        }
    }
    constexpr bool operator==(const Mat4f& rhs) const noexcept {
        return equals(rhs);
    }

    //
    // Write to Matrix via set(..) or load(..)
    //

    /**
     * Returns writable reference to the {@code i}th component of the given column-major order matrix, 0 <= i < 16 w/o boundary check
     */
    float& operator[](size_t i) noexcept {
        return reinterpret_cast<float*>(this)[i];
    }

    /** Sets the {@code i}th component with float {@code v} 0 <= i < 16 w/ boundary check*/
    void set(const jau::nsize_t i, const float v) {
        switch (i) {
            case 0+4*0: m00 = v; break;
            case 1+4*0: m10 = v; break;
            case 2+4*0: m20 = v; break;
            case 3+4*0: m30 = v; break;

            case 0+4*1: m01 = v; break;
            case 1+4*1: m11 = v; break;
            case 2+4*1: m21 = v; break;
            case 3+4*1: m31 = v; break;

            case 0+4*2: m02 = v; break;
            case 1+4*2: m12 = v; break;
            case 2+4*2: m22 = v; break;
            case 3+4*2: m32 = v; break;

            case 0+4*3: m03 = v; break;
            case 1+4*3: m13 = v; break;
            case 2+4*3: m23 = v; break;
            case 3+4*3: m33 = v; break;
            default: throw jau::IndexOutOfBoundsException(i, 16, E_FILE_LINE);
        }
    }

    /**
     * Set this matrix to identity.
     * <pre>
      Translation matrix (Column Order):
      1 0 0 0
      0o.m00 0 0
      0 0 1 0
      0 0 0 1
     * </pre>
     * @return this matrix for chaining
     */
    Mat4f& load_identity() noexcept {
       m00 = m11 = m22 = m33 = 1.0f;
       m01 = m02 = m03 =
       m10 = m12 = m13 =
       m20 = m21 = m23 =
       m30 = m31 = m32 = 0.0f;
       return *this;
    }

    /**
     * Load the values of the given matrix {@code src} to this matrix w/o boundary check
     * @param src the source values
     * @return this matrix for chaining
     */
    Mat4f& load(const Mat4f& src) noexcept {
        m00 = src.m00; m10 = src.m10; m20 = src.m20; m30 = src.m30;
        m01 = src.m01; m11 = src.m11; m21 = src.m21; m31 = src.m31;
        m02 = src.m02; m12 = src.m12; m22 = src.m22; m32 = src.m32;
        m03 = src.m03; m13 = src.m13; m23 = src.m23; m33 = src.m33;
        return *this;
    }

    /**
     * Load the values of the given matrix {@code src} to this matrix w/o boundary check.
     * @param src 4x4 matrix float[16] in column-major order
     * @return this matrix for chaining
     */
    Mat4f& load(const float src[]) noexcept {
        m00 = src[0+0*4]; // column 0
        m10 = src[1+0*4];
        m20 = src[2+0*4];
        m30 = src[3+0*4];
        m01 = src[0+1*4]; // column 1
        m11 = src[1+1*4];
        m21 = src[2+1*4];
        m31 = src[3+1*4];
        m02 = src[0+2*4]; // column 2
        m12 = src[1+2*4];
        m22 = src[2+2*4];
        m32 = src[3+2*4];
        m03 = src[0+3*4]; // column 3
        m13 = src[1+3*4];
        m23 = src[2+3*4];
        m33 = src[3+3*4];
        return *this;
    }

    /**
     * Load the values of the given matrix {@code src} to this matrix w/ boundary check.
     * @param src 4x4 matrix std::vector in column-major order starting at {@code src_off}
     * @param src_off offset for matrix {@code src}
     * @return this matrix for chaining
     */
    Mat4f& load(const std::vector<float>& src, const size_t src_off) {
        if( src.size() < src_off+16 || src_off > std::numeric_limits<size_t>::max() - 15) {
            throw jau::IndexOutOfBoundsException(src_off, 16, E_FILE_LINE);
        }
        m00 = src[src_off+0+0*4];
        m10 = src[src_off+1+0*4];
        m20 = src[src_off+2+0*4];
        m30 = src[src_off+3+0*4];
        m01 = src[src_off+0+1*4];
        m11 = src[src_off+1+1*4];
        m21 = src[src_off+2+1*4];
        m31 = src[src_off+3+1*4];
        m02 = src[src_off+0+2*4];
        m12 = src[src_off+1+2*4];
        m22 = src[src_off+2+2*4];
        m32 = src[src_off+3+2*4];
        m03 = src[src_off+0+3*4];
        m13 = src[src_off+1+3*4];
        m23 = src[src_off+2+3*4];
        m33 = src[src_off+3+3*4];
        return *this;
    }

    //
    // Read out Matrix via get(..)
    //

    /**
     * Returns read-only {@code i}th component of the given column-major order matrix, 0 <= i < 16 w/o boundary check
     */
    float operator[](size_t i) const noexcept {
        return reinterpret_cast<const float*>(this)[i];
    }

    /** Returns the {@code i}th component of the given column-major order matrix, 0 <= i < 16, w/ boundary check */
    float get(const jau::nsize_t i) const {
        switch (i) {
            case 0+4*0: return m00;
            case 1+4*0: return m10;
            case 2+4*0: return m20;
            case 3+4*0: return m30;

            case 0+4*1: return m01;
            case 1+4*1: return m11;
            case 2+4*1: return m21;
            case 3+4*1: return m31;

            case 0+4*2: return m02;
            case 1+4*2: return m12;
            case 2+4*2: return m22;
            case 3+4*2: return m32;

            case 0+4*3: return m03;
            case 1+4*3: return m13;
            case 2+4*3: return m23;
            case 3+4*3: return m33;

            default: throw jau::IndexOutOfBoundsException(i, 16, E_FILE_LINE);
        }
    }

    /**
     * Get the named column of the given column-major matrix to v_out w/ boundary check.
     * @param column named column to copy
     * @param v_out the column-vector storage
     * @return given result vector <i>v_out</i> for chaining
     */
    Vec4f& getColumn(const jau::nsize_t column, Vec4f& v_out) const {
        if( column > 3 ) {
            throw jau::IndexOutOfBoundsException(3+column*4, 16, E_FILE_LINE);
        }
        v_out.set( get(0+column*4),
                   get(1+column*4),
                   get(2+column*4),
                   get(3+column*4) );
        return v_out;
    }

    /**
     * Get the named column of the given column-major matrix to v_out w/ boundary check.
     * @param column named column to copy
     * @param v_out the column-vector storage
     * @return given result vector <i>v_out</i> for chaining
     */
    Vec3f& getColumn(const jau::nsize_t column, Vec3f& v_out) const {
        if( column > 3 ) {
            throw jau::IndexOutOfBoundsException(2+column*4, 16, E_FILE_LINE);
        }
        v_out.set( get(0+column*4),
                   get(1+column*4),
                   get(2+column*4) );
        return v_out;
    }

    /**
     * Get the named row of the given column-major matrix to v_out w/ boundary check.
     * @param row named row to copy
     * @param v_out the row-vector storage
     * @return given result vector <i>v_out</i> for chaining
     */
    Vec4f& getRow(const jau::nsize_t row, Vec4f& v_out) const {
        if( row > 3 ) {
            throw jau::IndexOutOfBoundsException(row+3*4, 16, E_FILE_LINE);
        }
        v_out.set( get(row+0*4),
                   get(row+1*4),
                   get(row+2*4),
                   get(row+3*4) );
        return v_out;
    }

    /**
     * Get the named row of the given column-major matrix to v_out w/ boundary check.
     * @param row named row to copy
     * @param v_out the row-vector storage
     * @return given result vector <i>v_out</i> for chaining
     */
    Vec3f& getRow(const jau::nsize_t row, Vec3f& v_out) const {
        if( row > 3 ) {
            throw jau::IndexOutOfBoundsException(row+2*4, 16, E_FILE_LINE);
        }
        v_out.set( get(row+0*4),
                   get(row+1*4),
                   get(row+2*4) );
        return v_out;
    }

    /**
     * Get this matrix into the given float[16] array at {@code dst_off} in column major order w/ boundary check.
     *
     * @param dst float[16] array storage in column major order
     * @param dst_off offset
     * @return {@code dst} for chaining
     */
    float* get(float* dst, size_t dst_off) const {
        if( dst_off > std::numeric_limits<size_t>::max() - 15) {
            throw jau::IndexOutOfBoundsException(dst_off, 16, E_FILE_LINE);
        }
        dst[dst_off++] = m00;
        dst[dst_off++] = m10;
        dst[dst_off++] = m20;
        dst[dst_off++] = m30;
        dst[dst_off++] = m01;
        dst[dst_off++] = m11;
        dst[dst_off++] = m21;
        dst[dst_off++] = m31;
        dst[dst_off++] = m02;
        dst[dst_off++] = m12;
        dst[dst_off++] = m22;
        dst[dst_off++] = m32;
        dst[dst_off++] = m03;
        dst[dst_off++] = m13;
        dst[dst_off++] = m23;
        dst[dst_off++] = m33;
        return dst;
    }

    /**
     * Get this matrix into the given float[16] array in column major order w/o boundary check.
     *
     * @param dst float[16] array storage in column major order
     * @return {@code dst} for chaining
     */
    float* get(float* dst) const noexcept {
        dst[0+0*4] = m00;
        dst[1+0*4] = m10;
        dst[2+0*4] = m20;
        dst[3+0*4] = m30;
        dst[0+1*4] = m01;
        dst[1+1*4] = m11;
        dst[2+1*4] = m21;
        dst[3+1*4] = m31;
        dst[0+2*4] = m02;
        dst[1+2*4] = m12;
        dst[2+2*4] = m22;
        dst[3+2*4] = m32;
        dst[0+3*4] = m03;
        dst[1+3*4] = m13;
        dst[2+3*4] = m23;
        dst[3+3*4] = m33;
        return dst;
    }

    /**
     * Get this matrix into the given {@link FloatBuffer} in column major order w/ boundary check.
     *
     * @param dst 4x4 matrix std::vector in column-major order starting at {@code dst_off}
     * @param dst_off offset for matrix {@code dst}
     * @return {@code dst} for chaining
     */
    std::vector<float>& get(std::vector<float>& dst, size_t dst_off) const {
        if( dst.size() < dst_off+16 || dst_off > std::numeric_limits<size_t>::max() - 15) {
            throw jau::IndexOutOfBoundsException(dst_off, 16, E_FILE_LINE);
        }
        dst[dst_off++] = m00;
        dst[dst_off++] = m10;
        dst[dst_off++] = m20;
        dst[dst_off++] = m30;
        dst[dst_off++] = m01;
        dst[dst_off++] = m11;
        dst[dst_off++] = m21;
        dst[dst_off++] = m31;
        dst[dst_off++] = m02;
        dst[dst_off++] = m12;
        dst[dst_off++] = m22;
        dst[dst_off++] = m32;
        dst[dst_off++] = m03;
        dst[dst_off++] = m13;
        dst[dst_off++] = m23;
        dst[dst_off++] = m33;
        return dst;
    }

    //
    // Basic matrix operations
    //

    /**
     * Returns the determinant of this matrix
     * @return the matrix determinant
     */
    float determinant() const noexcept {
        float ret = 0;
        ret += m00 * ( + m11*(m22*m33 - m23*m32) - m12*(m21*m33 - m23*m31) + m13*(m21*m32 - m22*m31));
        ret -= m01 * ( + m10*(m22*m33 - m23*m32) - m12*(m20*m33 - m23*m30) + m13*(m20*m32 - m22*m30));
        ret += m02 * ( + m10*(m21*m33 - m23*m31) - m11*(m20*m33 - m23*m30) + m13*(m20*m31 - m21*m30));
        ret -= m03 * ( + m10*(m21*m32 - m22*m31) - m11*(m20*m32 - m22*m30) + m12*(m20*m31 - m21*m30));
        return ret;
    }

    /**
     * Transpose this matrix.
     *
     * @return this matrix for chaining
     */
    Mat4f& transpose() noexcept {
        float tmp;

        tmp = m10;
        m10 = m01;
        m01 = tmp;

        tmp = m20;
        m20 = m02;
        m02 = tmp;

        tmp = m30;
        m30 = m03;
        m03 = tmp;

        tmp = m21;
        m21 = m12;
        m12 = tmp;

        tmp = m31;
        m31 = m13;
        m13 = tmp;

        tmp = m32;
        m32 = m23;
        m23 = tmp;

        return *this;
    }

    /**
     * Transpose the given {@code src} matrix into this matrix.
     *
     * @param src source 4x4 matrix
     * @return this matrix (result) for chaining
     */
    Mat4f& transpose(const Mat4f& src) noexcept {
        if( &src == this ) {
            return transpose();
        }
        m00 = src.m00;
        m10 = src.m01;
        m20 = src.m02;
        m30 = src.m03;

        m01 = src.m10;
        m11 = src.m11;
        m21 = src.m12;
        m31 = src.m13;

        m02 = src.m20;
        m12 = src.m21;
        m22 = src.m22;
        m32 = src.m23;

        m03 = src.m30;
        m13 = src.m31;
        m23 = src.m32;
        m33 = src.m33;
        return *this;
    }

    /**
     * Invert this matrix.
     * @return false if this matrix is singular and inversion not possible, otherwise true
     */
    bool invert() noexcept {
        const float amax = absMax();
        if( 0.0f == amax ) {
            return false;
        }
        const float scale = 1.0f/amax;
        const float a00 = m00*scale;
        const float a10 = m10*scale;
        const float a20 = m20*scale;
        const float a30 = m30*scale;

        const float a01 = m01*scale;
        const float a11 = m11*scale;
        const float a21 = m21*scale;
        const float a31 = m31*scale;

        const float a02 = m02*scale;
        const float a12 = m12*scale;
        const float a22 = m22*scale;
        const float a32 = m32*scale;

        const float a03 = m03*scale;
        const float a13 = m13*scale;
        const float a23 = m23*scale;
        const float a33 = m33*scale;

        const float b00 = + a11*(a22*a33 - a23*a32) - a12*(a21*a33 - a23*a31) + a13*(a21*a32 - a22*a31);
        const float b01 = -( + a10*(a22*a33 - a23*a32) - a12*(a20*a33 - a23*a30) + a13*(a20*a32 - a22*a30));
        const float b02 = + a10*(a21*a33 - a23*a31) - a11*(a20*a33 - a23*a30) + a13*(a20*a31 - a21*a30);
        const float b03 = -( + a10*(a21*a32 - a22*a31) - a11*(a20*a32 - a22*a30) + a12*(a20*a31 - a21*a30));

        const float b10 = -( + a01*(a22*a33 - a23*a32) - a02*(a21*a33 - a23*a31) + a03*(a21*a32 - a22*a31));
        const float b11 = + a00*(a22*a33 - a23*a32) - a02*(a20*a33 - a23*a30) + a03*(a20*a32 - a22*a30);
        const float b12 = -( + a00*(a21*a33 - a23*a31) - a01*(a20*a33 - a23*a30) + a03*(a20*a31 - a21*a30));
        const float b13 = + a00*(a21*a32 - a22*a31) - a01*(a20*a32 - a22*a30) + a02*(a20*a31 - a21*a30);

        const float b20 = + a01*(a12*a33 - a13*a32) - a02*(a11*a33 - a13*a31) + a03*(a11*a32 - a12*a31);
        const float b21 = -( + a00*(a12*a33 - a13*a32) - a02*(a10*a33 - a13*a30) + a03*(a10*a32 - a12*a30));
        const float b22 = + a00*(a11*a33 - a13*a31) - a01*(a10*a33 - a13*a30) + a03*(a10*a31 - a11*a30);
        const float b23 = -( + a00*(a11*a32 - a12*a31) - a01*(a10*a32 - a12*a30) + a02*(a10*a31 - a11*a30));

        const float b30 = -( + a01*(a12*a23 - a13*a22) - a02*(a11*a23 - a13*a21) + a03*(a11*a22 - a12*a21));
        const float b31 = + a00*(a12*a23 - a13*a22) - a02*(a10*a23 - a13*a20) + a03*(a10*a22 - a12*a20);
        const float b32 = -( + a00*(a11*a23 - a13*a21) - a01*(a10*a23 - a13*a20) + a03*(a10*a21 - a11*a20));
        const float b33 = + a00*(a11*a22 - a12*a21) - a01*(a10*a22 - a12*a20) + a02*(a10*a21 - a11*a20);

        const float det = (a00*b00 + a01*b01 + a02*b02 + a03*b03) / scale;
        if( 0 == det ) {
            return false;
        }
        const float invdet = 1.0f / det;

        m00 = b00 * invdet;
        m10 = b01 * invdet;
        m20 = b02 * invdet;
        m30 = b03 * invdet;

        m01 = b10 * invdet;
        m11 = b11 * invdet;
        m21 = b12 * invdet;
        m31 = b13 * invdet;

        m02 = b20 * invdet;
        m12 = b21 * invdet;
        m22 = b22 * invdet;
        m32 = b23 * invdet;

        m03 = b30 * invdet;
        m13 = b31 * invdet;
        m23 = b32 * invdet;
        m33 = b33 * invdet;
        return true;
    }

    /**
     * Invert the {@code src} matrix values into this matrix
     * @param src the source matrix, which values are to be inverted
     * @return false if {@code src} matrix is singular and inversion not possible, otherwise true
     */
    bool invert(const Mat4f& src) noexcept {
        const float amax = src.absMax();
        if( 0.0f == amax ) {
            return false;
        }
        const float scale = 1.0f/amax;
        const float a00 = src.m00*scale;
        const float a10 = src.m10*scale;
        const float a20 = src.m20*scale;
        const float a30 = src.m30*scale;

        const float a01 = src.m01*scale;
        const float a11 = src.m11*scale;
        const float a21 = src.m21*scale;
        const float a31 = src.m31*scale;

        const float a02 = src.m02*scale;
        const float a12 = src.m12*scale;
        const float a22 = src.m22*scale;
        const float a32 = src.m32*scale;

        const float a03 = src.m03*scale;
        const float a13 = src.m13*scale;
        const float a23 = src.m23*scale;
        const float a33 = src.m33*scale;

        const float b00 = + a11*(a22*a33 - a23*a32) - a12*(a21*a33 - a23*a31) + a13*(a21*a32 - a22*a31);
        const float b01 = -( + a10*(a22*a33 - a23*a32) - a12*(a20*a33 - a23*a30) + a13*(a20*a32 - a22*a30));
        const float b02 = + a10*(a21*a33 - a23*a31) - a11*(a20*a33 - a23*a30) + a13*(a20*a31 - a21*a30);
        const float b03 = -( + a10*(a21*a32 - a22*a31) - a11*(a20*a32 - a22*a30) + a12*(a20*a31 - a21*a30));

        const float b10 = -( + a01*(a22*a33 - a23*a32) - a02*(a21*a33 - a23*a31) + a03*(a21*a32 - a22*a31));
        const float b11 = + a00*(a22*a33 - a23*a32) - a02*(a20*a33 - a23*a30) + a03*(a20*a32 - a22*a30);
        const float b12 = -( + a00*(a21*a33 - a23*a31) - a01*(a20*a33 - a23*a30) + a03*(a20*a31 - a21*a30));
        const float b13 = + a00*(a21*a32 - a22*a31) - a01*(a20*a32 - a22*a30) + a02*(a20*a31 - a21*a30);

        const float b20 = + a01*(a12*a33 - a13*a32) - a02*(a11*a33 - a13*a31) + a03*(a11*a32 - a12*a31);
        const float b21 = -( + a00*(a12*a33 - a13*a32) - a02*(a10*a33 - a13*a30) + a03*(a10*a32 - a12*a30));
        const float b22 = + a00*(a11*a33 - a13*a31) - a01*(a10*a33 - a13*a30) + a03*(a10*a31 - a11*a30);
        const float b23 = -( + a00*(a11*a32 - a12*a31) - a01*(a10*a32 - a12*a30) + a02*(a10*a31 - a11*a30));

        const float b30 = -( + a01*(a12*a23 - a13*a22) - a02*(a11*a23 - a13*a21) + a03*(a11*a22 - a12*a21));
        const float b31 = + a00*(a12*a23 - a13*a22) - a02*(a10*a23 - a13*a20) + a03*(a10*a22 - a12*a20);
        const float b32 = -( + a00*(a11*a23 - a13*a21) - a01*(a10*a23 - a13*a20) + a03*(a10*a21 - a11*a20));
        const float b33 = + a00*(a11*a22 - a12*a21) - a01*(a10*a22 - a12*a20) + a02*(a10*a21 - a11*a20);

        const float det = (a00*b00 + a01*b01 + a02*b02 + a03*b03) / scale;

        if( 0 == det ) {
            return false;
        }
        const float invdet = 1.0f / det;

        m00 = b00 * invdet;
        m10 = b01 * invdet;
        m20 = b02 * invdet;
        m30 = b03 * invdet;

        m01 = b10 * invdet;
        m11 = b11 * invdet;
        m21 = b12 * invdet;
        m31 = b13 * invdet;

        m02 = b20 * invdet;
        m12 = b21 * invdet;
        m22 = b22 * invdet;
        m32 = b23 * invdet;

        m03 = b30 * invdet;
        m13 = b31 * invdet;
        m23 = b32 * invdet;
        m33 = b33 * invdet;
        return true;
    }

  private:
    float absMax() const noexcept {
        float max = std::abs(m00);
        max = std::max(max, std::abs(m01));
        max = std::max(max, std::abs(m02));
        max = std::max(max, std::abs(m03));

        max = std::max(max, std::abs(m10));
        max = std::max(max, std::abs(m11));
        max = std::max(max, std::abs(m12));
        max = std::max(max, std::abs(m13));

        max = std::max(max, std::abs(m20));
        max = std::max(max, std::abs(m21));
        max = std::max(max, std::abs(m22));
        max = std::max(max, std::abs(m23));

        max = std::max(max, std::abs(m30));
        max = std::max(max, std::abs(m31));
        max = std::max(max, std::abs(m32));
        max = std::max(max, std::abs(m33));
        return max;
    }

  public:
    /**
     * Multiply matrix: [this] = [this] x [b]
     * @param b 4x4 matrix
     * @return this matrix for chaining
     * @see #mul(mat4f, mat4f)
     */
    Mat4f& mul(const Mat4f& b) noexcept {
        // return mul(new mat4f(this), b); // <- roughly half speed
        float ai0=m00; // row-0, m[0+0*4]
        float ai1=m01;
        float ai2=m02;
        float ai3=m03;
        m00 = ai0 * b.m00  +  ai1 * b.m10  +  ai2 * b.m20  +  ai3 * b.m30 ;
        m01 = ai0 * b.m01  +  ai1 * b.m11  +  ai2 * b.m21  +  ai3 * b.m31 ;
        m02 = ai0 * b.m02  +  ai1 * b.m12  +  ai2 * b.m22  +  ai3 * b.m32 ;
        m03 = ai0 * b.m03  +  ai1 * b.m13  +  ai2 * b.m23  +  ai3 * b.m33 ;

        ai0=m10; //row-1, m[1+0*4]
        ai1=m11;
        ai2=m12;
        ai3=m13;
        m10 = ai0 * b.m00  +  ai1 * b.m10  +  ai2 * b.m20  +  ai3 * b.m30 ;
        m11 = ai0 * b.m01  +  ai1 * b.m11  +  ai2 * b.m21  +  ai3 * b.m31 ;
        m12 = ai0 * b.m02  +  ai1 * b.m12  +  ai2 * b.m22  +  ai3 * b.m32 ;
        m13 = ai0 * b.m03  +  ai1 * b.m13  +  ai2 * b.m23  +  ai3 * b.m33 ;

        ai0=m20; // row-2, m[2+0*4]
        ai1=m21;
        ai2=m22;
        ai3=m23;
        m20 = ai0 * b.m00  +  ai1 * b.m10  +  ai2 * b.m20  +  ai3 * b.m30 ;
        m21 = ai0 * b.m01  +  ai1 * b.m11  +  ai2 * b.m21  +  ai3 * b.m31 ;
        m22 = ai0 * b.m02  +  ai1 * b.m12  +  ai2 * b.m22  +  ai3 * b.m32 ;
        m23 = ai0 * b.m03  +  ai1 * b.m13  +  ai2 * b.m23  +  ai3 * b.m33 ;

        ai0=m30; // row-3, m[3+0*4]
        ai1=m31;
        ai2=m32;
        ai3=m33;
        m30 = ai0 * b.m00  +  ai1 * b.m10  +  ai2 * b.m20  +  ai3 * b.m30 ;
        m31 = ai0 * b.m01  +  ai1 * b.m11  +  ai2 * b.m21  +  ai3 * b.m31 ;
        m32 = ai0 * b.m02  +  ai1 * b.m12  +  ai2 * b.m22  +  ai3 * b.m32 ;
        m33 = ai0 * b.m03  +  ai1 * b.m13  +  ai2 * b.m23  +  ai3 * b.m33 ;
        return *this;
    }

    /**
     * Multiply matrix: [this] = [a] x [b]
     * @param a 4x4 matrix, can't be this matrix
     * @param b 4x4 matrix, can't be this matrix
     * @return this matrix for chaining
     * @see #mul(mat4f)
     */
    Mat4f& mul(const Mat4f& a, const Mat4f& b) noexcept {
        // row-0, m[0+0*4]
        m00 = a.m00 * b.m00  +  a.m01 * b.m10  +  a.m02 * b.m20  +  a.m03 * b.m30 ;
        m01 = a.m00 * b.m01  +  a.m01 * b.m11  +  a.m02 * b.m21  +  a.m03 * b.m31 ;
        m02 = a.m00 * b.m02  +  a.m01 * b.m12  +  a.m02 * b.m22  +  a.m03 * b.m32 ;
        m03 = a.m00 * b.m03  +  a.m01 * b.m13  +  a.m02 * b.m23  +  a.m03 * b.m33 ;

        //row-1, m[1+0*4]
        m10 = a.m10 * b.m00  +  a.m11 * b.m10  +  a.m12 * b.m20  +  a.m13 * b.m30 ;
        m11 = a.m10 * b.m01  +  a.m11 * b.m11  +  a.m12 * b.m21  +  a.m13 * b.m31 ;
        m12 = a.m10 * b.m02  +  a.m11 * b.m12  +  a.m12 * b.m22  +  a.m13 * b.m32 ;
        m13 = a.m10 * b.m03  +  a.m11 * b.m13  +  a.m12 * b.m23  +  a.m13 * b.m33 ;

        // row-2, m[2+0*4]
        m20 = a.m20 * b.m00  +  a.m21 * b.m10  +  a.m22 * b.m20  +  a.m23 * b.m30 ;
        m21 = a.m20 * b.m01  +  a.m21 * b.m11  +  a.m22 * b.m21  +  a.m23 * b.m31 ;
        m22 = a.m20 * b.m02  +  a.m21 * b.m12  +  a.m22 * b.m22  +  a.m23 * b.m32 ;
        m23 = a.m20 * b.m03  +  a.m21 * b.m13  +  a.m22 * b.m23  +  a.m23 * b.m33 ;

        // row-3, m[3+0*4]
        m30 = a.m30 * b.m00  +  a.m31 * b.m10  +  a.m32 * b.m20  +  a.m33 * b.m30 ;
        m31 = a.m30 * b.m01  +  a.m31 * b.m11  +  a.m32 * b.m21  +  a.m33 * b.m31 ;
        m32 = a.m30 * b.m02  +  a.m31 * b.m12  +  a.m32 * b.m22  +  a.m33 * b.m32 ;
        m33 = a.m30 * b.m03  +  a.m31 * b.m13  +  a.m32 * b.m23  +  a.m33 * b.m33 ;

        return *this;
    }

    /**
     * @param v_in 4-component column-vector, can be v_out for in-place transformation
     * @param v_out this * v_in
     * @returns v_out for chaining
     */
    Vec4f& mulVec4f(const Vec4f& v_in, Vec4f& v_out) const noexcept {
        // (one matrix row in column-major order) X (column vector)
        const float x = v_in.x, y = v_in.y, z = v_in.z, w = v_in.w;
        v_out.set( x * m00 + y * m01 + z * m02 + w * m03,
                   x * m10 + y * m11 + z * m12 + w * m13,
                   x * m20 + y * m21 + z * m22 + w * m23,
                   x * m30 + y * m31 + z * m32 + w * m33 );
        return v_out;
    }

    /**
     * @param v_inout 4-component column-vector input and output, i.e. in-place transformation
     * @returns v_inout for chaining
     */
    Vec4f& mulVec4f(Vec4f& v_inout) const noexcept {
        // (one matrix row in column-major order) X (column vector)
        const float x = v_inout.x, y = v_inout.y, z = v_inout.z, w = v_inout.w;
        v_inout.set( x * m00 + y * m01 + z * m02 + w * m03,
                     x * m10 + y * m11 + z * m12 + w * m13,
                     x * m20 + y * m21 + z * m22 + w * m23,
                     x * m30 + y * m31 + z * m32 + w * m33 );
        return v_inout;
    }

    /**
     * Affine 3f-vector transformation by 4x4 matrix
     *
     * 4x4 matrix multiplication with 3-component vector,
     * using {@code 1} for for {@code v_in.w()} and dropping {@code v_out.w()},
     * which shall be {@code 1}.
     *
     * @param v_in 3-component column-vector {@link vec3f}, can be v_out for in-place transformation
     * @param v_out m_in * v_in, 3-component column-vector {@link vec3f}
     * @returns v_out for chaining
     */
    Vec3f& mulVec3f(const Vec3f& v_in, Vec3f& v_out) const noexcept {
        // (one matrix row in column-major order) X (column vector)
        const float x = v_in.x, y = v_in.y, z = v_in.z;
        v_out.set( x * m00 + y * m01 + z * m02 + 1.0f * m03,
                   x * m10 + y * m11 + z * m12 + 1.0f * m13,
                   x * m20 + y * m21 + z * m22 + 1.0f * m23 );
        return v_out;
    }

    /**
     * Affine 3f-vector transformation by 4x4 matrix
     *
     * 4x4 matrix multiplication with 3-component vector,
     * using {@code 1} for for {@code v_inout.w()} and dropping {@code v_inout.w()},
     * which shall be {@code 1}.
     *
     * @param v_inout 3-component column-vector {@link vec3f} input and output, i.e. in-place transformation
     * @returns v_inout for chaining
     */
    Vec3f& mulVec3f(Vec3f& v_inout) const noexcept {
        // (one matrix row in column-major order) X (column vector)
        const float x = v_inout.x, y = v_inout.y, z = v_inout.z;
        v_inout.set( x * m00 + y * m01 + z * m02 + 1.0f * m03,
                     x * m10 + y * m11 + z * m12 + 1.0f * m13,
                     x * m20 + y * m21 + z * m22 + 1.0f * m23 );
        return v_inout;
    }

    //
    // Matrix setTo...(), affine + basic
    //

    /**
     * Set this matrix to translation.
     * <pre>
      Translation matrix (Column Order):
      1 0 0 0
      0 1 0 0
      0 0 1 0
      x y z 1
     * </pre>
     * @param x x-axis translate
     * @param y y-axis translate
     * @param z z-axis translate
     * @return this matrix for chaining
     */
    Mat4f& setToTranslation(const float x, const float y, const float z) noexcept {
        m00 = m11 = m22 = m33 = 1.0f;
        m03 = x;
        m13 = y;
        m23 = z;
        m01 = m02 =
        m10 = m12 =
        m20 = m21 =
        m30 = m31 = m32 = 0.0f;
        return *this;
    }

    /**
     * Set this matrix to translation.
     * <pre>
      Translation matrix (Column Order):
      1 0 0 0
      0 1 0 0
      0 0 1 0
      x y z 1
     * </pre>
     * @param t translate vec3f
     * @return this matrix for chaining
     */
    Mat4f& setToTranslation(const Vec3f& t) noexcept {
        return setToTranslation(t.x, t.y, t.z);
    }

    /**
     * Set this matrix to scale.
     * <pre>
      Scale matrix (Any Order):
      x 0 0 0
      0 y 0 0
      0 0 z 0
      0 0 0 1
     * </pre>
     * @param x x-axis scale
     * @param y y-axis scale
     * @param z z-axis scale
     * @return this matrix for chaining
     */
    Mat4f& setToScale(const float x, const float y, const float z) noexcept {
        m33 = 1.0f;
        m00 = x;
        m11 = y;
        m22 = z;
        m01 = m02 = m03 =
        m10 = m12 = m13 =
        m20 = m21 = m23 =
        m30 = m31 = m32 = 0.0f;
        return *this;
    }

    /**
     * Set this matrix to scale.
     * <pre>
      Scale matrix (Any Order):
      x 0 0 0
      0 y 0 0
      0 0 z 0
      0 0 0 1
     * </pre>
     * @param s scale vec3f
     * @return this matrix for chaining
     */
    Mat4f& setToScale(const Vec3f& s) noexcept {
        return setToScale(s.x, s.y, s.z);
    }

    /**
     * Set this matrix to rotation from the given axis and angle in radians.
     * <pre>
        Rotation matrix (Column Order):
        xx(1-c)+c  xy(1-c)+zs xz(1-c)-ys 0
        xy(1-c)-zs yy(1-c)+c  yz(1-c)+xs 0
        xz(1-c)+ys yz(1-c)-xs zz(1-c)+c  0
        0          0          0          1
     * </pre>
     * @see <a href="http://web.archive.org/web/20041029003853/http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q38">Matrix-FAQ Q38</a>
     * @param ang_rad angle in radians
     * @param x x of rotation axis
     * @param y y of rotation axis
     * @param z z of rotation axis
     * @return this matrix for chaining
     */
    Mat4f& setToRotationAxis(const float ang_rad, float x, float y, float z) noexcept {
        const float c = std::cos(ang_rad);
        const float ic= 1.0f - c;
        const float s = std::sin(ang_rad);

        Vec3f tmp(x, y, z); tmp.normalize();
        x = tmp.x; y = tmp.y; z = tmp.z;

        const float xy = x*y;
        const float xz = x*z;
        const float xs = x*s;
        const float ys = y*s;
        const float yz = y*z;
        const float zs = z*s;
        m00 = x*x*ic+c;
        m10 = xy*ic+zs;
        m20 = xz*ic-ys;
        m30 = 0.0f;

        m01 = xy*ic-zs;
        m11 = y*y*ic+c;
        m21 = yz*ic+xs;
        m31 = 0.0f;

        m02 = xz*ic+ys;
        m12 = yz*ic-xs;
        m22 = z*z*ic+c;
        m32 = 0.0f;

        m03 = 0.9f;
        m13 = 0.0f;
        m23 = 0.0f;
        m33 = 1.0f;

        return *this;
    }

    /**
     * Set this matrix to rotation from the given axis and angle in radians.
     * <pre>
        Rotation matrix (Column Order):
        xx(1-c)+c  xy(1-c)+zs xz(1-c)-ys 0
        xy(1-c)-zs yy(1-c)+c  yz(1-c)+xs 0
        xz(1-c)+ys yz(1-c)-xs zz(1-c)+c  0
        0          0          0          1
     * </pre>
     * @see <a href="http://web.archive.org/web/20041029003853/http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q38">Matrix-FAQ Q38</a>
     * @param ang_rad angle in radians
     * @param axis rotation axis
     * @return this matrix for chaining
     */
    Mat4f& setToRotationAxis(const float ang_rad, const Vec3f& axis) noexcept {
        return setToRotationAxis(ang_rad, axis.x, axis.y, axis.z);
    }

    /**
     * Set this matrix to rotation from the given Euler rotation angles in radians.
     * <p>
     * The rotations are applied in the given order:
     * <ul>
     *  <li>y - heading</li>
     *  <li>z - attitude</li>
     *  <li>x - bank</li>
     * </ul>
     * </p>
     * @param bankX the Euler pitch angle in radians. (rotation about the X axis)
     * @param headingY the Euler yaw angle in radians. (rotation about the Y axis)
     * @param attitudeZ the Euler roll angle in radians. (rotation about the Z axis)
     * @return this matrix for chaining
     * <p>
     * Implementation does not use Quaternion and hence is exposed to
     * <a href="http://web.archive.org/web/20041029003853/http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q34">Gimbal-Lock</a>,
     * consider using {@link #setToRotation(Quaternion)}.
     * </p>
     * @see <a href="http://web.archive.org/web/20041029003853/http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q36">Matrix-FAQ Q36</a>
     * @see <a href="http://www.euclideanspace.com/maths/geometry/rotations/conversions/eulerToMatrix/index.htm">euclideanspace.com-eulerToMatrix</a>
     * @see #setToRotation(Quaternion)
     */
    Mat4f& setToRotationEuler(const float bankX, const float headingY, const float attitudeZ) noexcept {
        // Assuming the angles are in radians.
        const float ch = std::cos(headingY);
        const float sh = std::sin(headingY);
        const float ca = std::cos(attitudeZ);
        const float sa = std::sin(attitudeZ);
        const float cb = std::cos(bankX);
        const float sb = std::sin(bankX);

        m00 =  ch*ca;
        m10 =  sa;
        m20 = -sh*ca;
        m30 =  0.0f;

        m01 =  sh*sb    - ch*sa*cb;
        m11 =  ca*cb;
        m21 =  sh*sa*cb + ch*sb;
        m31 =  0.0f;

        m02 =  ch*sa*sb + sh*cb;
        m12 = -ca*sb;
        m22 = -sh*sa*sb + ch*cb;
        m32 =  0.0f;

        m03 =  0.0f;
        m13 =  0.0f;
        m23 =  0.0f;
        m33 =  1.0f;

        return *this;
    }

    /**
     * Set this matrix to rotation from the given Euler rotation angles in radians.
     * <p>
     * The rotations are applied in the given order:
     * <ul>
     *  <li>y - heading</li>
     *  <li>z - attitude</li>
     *  <li>x - bank</li>
     * </ul>
     * </p>
     * @param angradXYZ euler angle vector in radians holding x-bank, y-heading and z-attitude
     * @return this quaternion for chaining.
     * <p>
     * Implementation does not use Quaternion and hence is exposed to
     * <a href="http://web.archive.org/web/20041029003853/http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q34">Gimbal-Lock</a>,
     * consider using {@link #setToRotation(Quaternion)}.
     * </p>
     * @see <a href="http://web.archive.org/web/20041029003853/http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q36">Matrix-FAQ Q36</a>
     * @see <a href="http://www.euclideanspace.com/maths/geometry/rotations/conversions/eulerToMatrix/index.htm">euclideanspace.com-eulerToMatrix</a>
     * @see #setToRotation(Quaternion)
     */
    Mat4f& setToRotationEuler(const Vec3f& angradXYZ) noexcept {
        return setToRotationEuler(angradXYZ.x, angradXYZ.y, angradXYZ.z);
    }

    /**
     * Set this matrix to rotation using the given Quaternion.
     * <p>
     * Implementation Details:
     * <ul>
     *   <li> makes identity matrix if {@link #magnitudeSquared()} is {@link FloatUtil#isZero(float, float) is zero} using {@link FloatUtil#EPSILON epsilon}</li>
     *   <li> The fields [m00 .. m22] define the rotation</li>
     * </ul>
     * </p>
     *
     * @param q the Quaternion representing the rotation
     * @return this matrix for chaining
     * @see <a href="http://web.archive.org/web/20041029003853/http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q54">Matrix-FAQ Q54</a>
     * @see Quaternion#toMatrix(float[])
     * @see #getRotation()
     */
    Mat4f& setToRotation(const Quaternion& q) noexcept;

    /**
     * Returns the rotation [m00 .. m22] fields converted to a Quaternion.
     * @param res resulting Quaternion
     * @return the resulting Quaternion for chaining.
     * @see Quaternion#setFromMatrix(float, float, float, float, float, float, float, float, float)
     * @see #setToRotation(Quaternion)
     */
    Quaternion& getRotation(Quaternion& res) const noexcept;

    /**
     * Set this matrix to orthogonal projection.
     * <pre>
      Ortho matrix (Column Order):
      2/dx  0     0    0
      0     2/dy  0    0
      0     0     2/dz 0
      tx    ty    tz   1
     * </pre>
     * @param left
     * @param right
     * @param bottom
     * @param top
     * @param zNear
     * @param zFar
     * @return this matrix for chaining
     */
    Mat4f& setToOrtho(const float left, const float right,
                      const float bottom, const float top,
                      const float zNear, const float zFar) noexcept {
        {
            // m00 = m11 = m22 = m33 = 1.0f;
            m10 = m20 = m30 = 0.0f;
            m01 = m21 = m31 = 0.0f;
            m02 = m12 = m32 = 0.0f;
            // m03 = m13 = m23 = 0.0f;
        }
        const float dx=right-left;
        const float dy=top-bottom;
        const float dz=zFar-zNear;
        const float tx=-1.0f*(right+left)/dx;
        const float ty=-1.0f*(top+bottom)/dy;
        const float tz=-1.0f*(zFar+zNear)/dz;

        m00 =  2.0f/dx;
        m11 =  2.0f/dy;
        m22 = -2.0f/dz;

        m03 = tx;
        m13 = ty;
        m23 = tz;
        m33 = 1.0f;

        return *this;
    }

    /**
     * Set this matrix to frustum.
     * <pre>
      Frustum matrix (Column Order):
      2*zNear/dx   0          0   0
      0            2*zNear/dy 0   0
      A            B          C  -1
      0            0          D   0
     * </pre>
     * @param left
     * @param right
     * @param bottom
     * @param top
     * @param zNear
     * @param zFar
     * @return this matrix for chaining
     * @throws IllegalArgumentException if {@code zNear <= 0} or {@code zFar <= zNear}
     *                                  or {@code left == right}, or {@code bottom == top}.
     */
    Mat4f& setToFrustum(const float left, const float right,
                        const float bottom, const float top,
                        const float zNear, const float zFar) {
        if( zNear <= 0.0f || zFar <= zNear ) {
            throw jau::IllegalArgumentException("Requirements zNear > 0 and zFar > zNear, but zNear "+std::to_string(zNear)+", zFar "+std::to_string(zFar), E_FILE_LINE);
        }
        if( left == right || top == bottom) {
            throw jau::IllegalArgumentException("GL_INVALID_VALUE: top,bottom and left,right must not be equal", E_FILE_LINE);
        }
        {
            // m00 = m11 = m22 = m33 = 1f;
            m10 = m20 = m30 = 0.0f;
            m01 = m21 = m31 = 0.0f;
            m03 = m13 = 0.0f;
        }
        const float zNear2 = 2.0f*zNear;
        const float dx=right-left;
        const float dy=top-bottom;
        const float dz=zFar-zNear;
        const float A=(right+left)/dx;
        const float B=(top+bottom)/dy;
        const float C=-1.0f*(zFar+zNear)/dz;
        const float D=-2.0f*(zFar*zNear)/dz;

        m00 = zNear2/dx;
        m11 = zNear2/dy;

        m02 = A;
        m12 = B;
        m22 = C;
        m32 = -1.0f;

        m23 = D;
        m33 = 0.0f;

        return *this;
    }

    /**
     * Set this matrix to perspective {@link #setToFrustum(float, float, float, float, float, float) frustum} projection.
     *
     * @param fovy_rad angle in radians
     * @param aspect aspect ratio width / height
     * @param zNear
     * @param zFar
     * @return this matrix for chaining
     * @throws IllegalArgumentException if {@code zNear <= 0} or {@code zFar <= zNear}
     * @see #setToFrustum(float, float, float, float, float, float)
     */
    Mat4f& setToPerspective(const float fovy_rad, const float aspect, const float zNear, const float zFar) {
        const float top    =  std::tan(fovy_rad/2.0f) * zNear; // use tangent of half-fov !
        const float bottom =  -1.0f * top;    //          -1f * fovhvTan.top * zNear
        const float left   = aspect * bottom; // aspect * -1f * fovhvTan.top * zNear
        const float right  = aspect * top;    // aspect * fovhvTan.top * zNear
        return setToFrustum(left, right, bottom, top, zNear, zFar);
    }

    /**
     * Set this matrix to perspective {@link #setToFrustum(float, float, float, float, float, float) frustum} projection.
     *
     * @param fovhv {@link FovHVHalves} field of view in both directions, may not be centered, either in radians or tangent
     * @param zNear
     * @param zFar
     * @return this matrix for chaining
     * @throws IllegalArgumentException if {@code zNear <= 0} or {@code zFar <= zNear}
     * @see #setToFrustum(float, float, float, float, float, float)
     * @see Frustum#updateByFovDesc(mat4f, com.jogamp.math.geom.Frustum.FovDesc)
     */
    Mat4f& setToPerspective(const FovHVHalves& fovhv, const float zNear, const float zFar) {
        const FovHVHalves fovhvTan = fovhv.toTangents();  // use tangent of half-fov !
        const float top    =         fovhvTan.top    * zNear;
        const float bottom = -1.0f * fovhvTan.bottom * zNear;
        const float left   = -1.0f * fovhvTan.left   * zNear;
        const float right  =         fovhvTan.right  * zNear;
        return setToFrustum(left, right, bottom, top, zNear, zFar);
    }

    /**
     * Calculate the frustum planes in world coordinates
     * using this column major order matrix, usually a projection (P) or premultiplied P*MV matrix.
     *
     * Frustum plane's normals will point to the inside of the viewing frustum,
     * as required by the {@link Frustum} class.
     *
     * May use geom::Frustum::setFromMat4() directly.
     *
     * @see geom::Frustum::setFromMat4()
     */
    geom::Frustum& getFrustum(geom::Frustum& frustum) noexcept;

    /**
     * Set this matrix to the <i>look-at</i> matrix based on given parameters.
     * <p>
     * Consist out of two matrix multiplications:
     * <pre>
     *   <b>R</b> = <b>L</b> x <b>T</b>,
     *   with <b>L</b> for <i>look-at</i> matrix and
     *        <b>T</b> for eye translation.
     *
     *   Result <b>R</b> can be utilized for <i>projection or modelview</i> multiplication, i.e.
     *          <b>M</b> = <b>M</b> x <b>R</b>,
     *          with <b>M</b> being the <i>projection or modelview</i> matrix.
     * </pre>
     * </p>
     * @param eye 3 component eye vector
     * @param center 3 component center vector
     * @param up 3 component up vector
     * @param tmp temporary mat4f used for multiplication
     * @return this matrix for chaining
     */
    Mat4f& setToLookAt(const Vec3f& eye, const Vec3f& center, const Vec3f& up, Mat4f& tmp) noexcept {
        // normalized forward!
        const Vec3f fwd = ( center - eye ).normalize();

        /* Side = forward x up, normalized */
        const Vec3f side = fwd.cross(up).normalize();

        /* Recompute up as: up = side x forward */
        const Vec3f up2 = side.cross(fwd);

        m00 = side.x;
        m10 = up2.x;
        m20 = -fwd.x;
        m30 = 0;

        m01 = side.y;
        m11 = up2.y;
        m21 = -fwd.y;
        m31 = 0;

        m02 = side.z;
        m12 = up2.z;
        m22 = -fwd.z;
        m32 = 0;

        m03 = 0;
        m13 = 0;
        m23 = 0;
        m33 = 1;

        return mul( tmp.setToTranslation( -eye.x, -eye.y, -eye.z ) );
    }

    /**
     * Set this matrix to the <i>pick</i> matrix based on given parameters.
     * <p>
     * Traditional <code>gluPickMatrix</code> implementation.
     * </p>
     * <p>
     * Consist out of two matrix multiplications:
     * <pre>
     *   <b>R</b> = <b>T</b> x <b>S</b>,
     *   with <b>T</b> for viewport translation matrix and
     *        <b>S</b> for viewport scale matrix.
     *
     *   Result <b>R</b> can be utilized for <i>projection</i> multiplication, i.e.
     *          <b>P</b> = <b>P</b> x <b>R</b>,
     *          with <b>P</b> being the <i>projection</i> matrix.
     * </pre>
     * </p>
     * <p>
     * To effectively use the generated pick matrix for picking,
     * call {@link #setToPick(float, float, float, float, Recti, mat4f) setToPick(..)}
     * and multiply a {@link #setToPerspective(float, float, float, float) custom perspective matrix}
     * by this pick matrix. Then you may load the result onto the perspective matrix stack.
     * </p>
     * @param x the center x-component of a picking region in window coordinates
     * @param y the center y-component of a picking region in window coordinates
     * @param deltaX the width of the picking region in window coordinates.
     * @param deltaY the height of the picking region in window coordinates.
     * @param viewport Rect4i viewport
     * @param mat4Tmp temp storage
     * @return true if successful or false if either delta value is <= zero.
     */
    bool setToPick(const float x, const float y, const float deltaX, const float deltaY,
                   const Recti& viewport, Mat4f& mat4Tmp) noexcept {
        if (deltaX <= 0 || deltaY <= 0) {
            return false;
        }
        /* Translate and scale the picked region to the entire window */
        setToTranslation( ( viewport.width()  - 2.0f * ( x - viewport.x() ) ) / deltaX,
                          ( viewport.height() - 2.0f * ( y - viewport.y() ) ) / deltaY,
                          0);
        mat4Tmp.setToScale( viewport.width() / deltaX, viewport.height() / deltaY, 1.0f );
        mul(mat4Tmp);
        return true;
    }

    //
    // Matrix affine operations using setTo..()
    //

    /**
     * Rotate this matrix about give axis and angle in radians, i.e. multiply by {@link #setToRotationAxis(float, float, float, float) axis-rotation matrix}.
     * @see <a href="http://web.archive.org/web/20041029003853/http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q38">Matrix-FAQ Q38</a>
     * @param angrad angle in radians
     * @param x x of rotation axis
     * @param y y of rotation axis
     * @param z z of rotation axis
     * @param tmp temporary mat4f used for multiplication
     * @return this matrix for chaining
     */
    Mat4f& rotate(const float ang_rad, const float x, const float y, const float z, Mat4f& tmp) noexcept {
        return mul( tmp.setToRotationAxis(ang_rad, x, y, z) );
    }

    /**
     * Rotate this matrix about give axis and angle in radians, i.e. multiply by {@link #setToRotationAxis(float, vec3f) axis-rotation matrix}.
     * @see <a href="http://web.archive.org/web/20041029003853/http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q38">Matrix-FAQ Q38</a>
     * @param angrad angle in radians
     * @param axis rotation axis
     * @param tmp temporary mat4f used for multiplication
     * @return this matrix for chaining
     */
    Mat4f& rotate(const float ang_rad, const Vec3f& axis, Mat4f& tmp) noexcept {
        return mul( tmp.setToRotationAxis(ang_rad, axis) );
    }

    /**
     * Rotate this matrix with the given {@link Quaternion}, i.e. multiply by {@link #setToRotation(Quaternion) Quaternion's rotation matrix}.
     * @param tmp temporary mat4f used for multiplication
     * @return this matrix for chaining
     */
    Mat4f& rotate(const Quaternion& quat, Mat4f& tmp) noexcept {
        return mul( tmp.setToRotation(quat) );
    }

    /**
     * Translate this matrix, i.e. multiply by {@link #setToTranslation(float, float, float) translation matrix}.
     * @param x x translation
     * @param y y translation
     * @param z z translation
     * @param tmp temporary mat4f used for multiplication
     * @return this matrix for chaining
     */
    Mat4f& translate(const float x, const float y, const float z, Mat4f& tmp) noexcept {
        return mul( tmp.setToTranslation(x, y, z) );
    }

#if 0
    /**
     * Translate this matrix, i.e. multiply by {@link #setToTranslation(vec3f) translation matrix}.
     * @param t translation vec3f
     * @param tmp temporary mat4f used for multiplication
     * @return this matrix for chaining
     */
    public final Mat4f translate(final Vec3f t, final Mat4f tmp) {
        return mul( tmp.setToTranslation(t) );
    }

    /**
     * Scale this matrix, i.e. multiply by {@link #setToScale(float, float, float) scale matrix}.
     * @param x x scale
     * @param y y scale
     * @param z z scale
     * @param tmp temporary mat4f used for multiplication
     * @return this matrix for chaining
     */
    public final Mat4f scale(const float x, const float y, const float z, final Mat4f tmp) {
        return mul( tmp.setToScale(x, y, z) );
    }

    /**
     * Scale this matrix, i.e. multiply by {@link #setToScale(float, float, float) scale matrix}.
     * @param s scale for x-, y- and z-axis
     * @param tmp temporary mat4f used for multiplication
     * @return this matrix for chaining
     */
    public final Mat4f scale(const float s, final Mat4f tmp) {
        return mul( tmp.setToScale(s, s, s) );
    }

    //
    // Matrix Stack
    //

    /**
     * Push the matrix to it's stack, while preserving this matrix values.
     * @see #pop()
     */
    public final void push() {
        stack.push(this);
    }

    /**
     * Pop the current matrix from it's stack, replacing this matrix values.
     * @see #push()
     */
    public final void pop() {
        stack.pop(this);
    }

    //
    // equals
    //

    /**
     * Equals check using a given {@link FloatUtil#EPSILON} value and {@link FloatUtil#isEqual(float, float, float)}.
     * <p>
     * Implementation considers following corner cases:
     * <ul>
     *    <li>NaN == NaN</li>
     *    <li>+Inf == +Inf</li>
     *    <li>-Inf == -Inf</li>
     * </ul>
     * @param o comparison value
     * @param epsilon consider using {@link FloatUtil#EPSILON}
     * @return true if all components differ less than {@code epsilon}, otherwise false.
     */
    public boolean isEqual(final Mat4f o, const float epsilon) {
        if( this == o ) {
            return true;
        } else {
            return FloatUtil.isEqual(m00, o.m00, epsilon) &&
                   FloatUtil.isEqual(m01, o.m01, epsilon) &&
                   FloatUtil.isEqual(m02, o.m02, epsilon) &&
                   FloatUtil.isEqual(m03, o.m03, epsilon) &&
                   FloatUtil.isEqual(m10, o.m10, epsilon) &&
                   FloatUtil.isEqual(m11, o.m11, epsilon) &&
                   FloatUtil.isEqual(m12, o.m12, epsilon) &&
                   FloatUtil.isEqual(m13, o.m13, epsilon) &&
                   FloatUtil.isEqual(m20, o.m20, epsilon) &&
                   FloatUtil.isEqual(m21, o.m21, epsilon) &&
                   FloatUtil.isEqual(m22, o.m22, epsilon) &&
                   FloatUtil.isEqual(m23, o.m23, epsilon) &&
                   FloatUtil.isEqual(m30, o.m30, epsilon) &&
                   FloatUtil.isEqual(m31, o.m31, epsilon) &&
                   FloatUtil.isEqual(m32, o.m32, epsilon) &&
                   FloatUtil.isEqual(m33, o.m33, epsilon);
        }
    }

    /**
     * Equals check using {@link FloatUtil#EPSILON} value and {@link FloatUtil#isEqual(float, float, float)}.
     * <p>
     * Implementation considers following corner cases:
     * <ul>
     *    <li>NaN == NaN</li>
     *    <li>+Inf == +Inf</li>
     *    <li>-Inf == -Inf</li>
     * </ul>
     * @param o comparison value
     * @return true if all components differ less than {@link FloatUtil#EPSILON}, otherwise false.
     */
    public boolean isEqual(final Mat4f o) {
        return isEqual(o, FloatUtil.EPSILON);
    }

    @Override
    public boolean equals(final Object o) {
        if( o instanceof Mat4f ) {
            return isEqual((Mat4f)o, FloatUtil.EPSILON);
        } else {
            return false;
        }
    }

    //
    // Static multi Matrix ops
    //

    /**
     * Map object coordinates to window coordinates.
     * <p>
     * Traditional <code>gluProject</code> implementation.
     * </p>
     *
     * @param obj object position, 3 component vector
     * @param mMv modelview matrix
     * @param mP projection matrix
     * @param viewport Rect4i viewport
     * @param winPos 3 component window coordinate, the result
     * @return true if successful, otherwise false (z is 1)
     */
    public static boolean mapObjToWin(final Vec3f obj, final Mat4f mMv, final Mat4f mP,
                                      final Recti viewport, final Vec3f winPos)
    {
        final Vec4f vec4Tmp1 = new Vec4f(obj, 1f);

        // vec4Tmp2 = Mv * o
        // rawWinPos = P  * vec4Tmp2
        // rawWinPos = P * ( Mv * o )
        // rawWinPos = P * Mv * o
        final Vec4f vec4Tmp2 = mMv.mulVec4f(vec4Tmp1, new Vec4f());
        final Vec4f rawWinPos = mP.mulVec4f(vec4Tmp2, vec4Tmp1);

        if (rawWinPos.w() == 0.0f) {
            return false;
        }

        const float s = ( 1.0f / rawWinPos.w() ) * 0.5f;

        // Map x, y and z to range 0-1 (w is ignored)
        rawWinPos.scale(s).add(0.5f, 0.5f, 0.5f, 0f);

        // Map x,y to viewport
        winPos.set( rawWinPos.x() * viewport.width() +  viewport.x(),
                    rawWinPos.y() * viewport.height() + viewport.y(),
                    rawWinPos.z() );

        return true;
    }

    /**
     * Map object coordinates to window coordinates.
     * <p>
     * Traditional <code>gluProject</code> implementation.
     * </p>
     *
     * @param obj object position, 3 component vector
     * @param mPMv [projection] x [modelview] matrix, i.e. P x Mv
     * @param viewport Rect4i viewport
     * @param winPos 3 component window coordinate, the result
     * @return true if successful, otherwise false (z is 1)
     */
    public static boolean mapObjToWin(final Vec3f obj, final Mat4f mPMv,
                                      final Recti viewport, final Vec3f winPos)
    {
        final Vec4f vec4Tmp2 = new Vec4f(obj, 1f);

        // rawWinPos = P * Mv * o
        final Vec4f rawWinPos = mPMv.mulVec4f(vec4Tmp2, new Vec4f());

        if (rawWinPos.w() == 0.0f) {
            return false;
        }

        const float s = ( 1.0f / rawWinPos.w() ) * 0.5f;

        // Map x, y and z to range 0-1 (w is ignored)
        rawWinPos.scale(s).add(0.5f, 0.5f, 0.5f, 0f);

        // Map x,y to viewport
        winPos.set( rawWinPos.x() * viewport.width() +  viewport.x(),
                    rawWinPos.y() * viewport.height() + viewport.y(),
                    rawWinPos.z() );

        return true;
    }

    /**
     * Map window coordinates to object coordinates.
     * <p>
     * Traditional <code>gluUnProject</code> implementation.
     * </p>
     *
     * @param winx
     * @param winy
     * @param winz
     * @param mMv 4x4 modelview matrix
     * @param mP 4x4 projection matrix
     * @param viewport Rect4i viewport
     * @param objPos 3 component object coordinate, the result
     * @param mat4Tmp 16 component matrix for temp storage
     * @return true if successful, otherwise false (failed to invert matrix, or becomes infinity due to zero z)
     */
    public static boolean mapWinToObj(const float winx, const float winy, const float winz,
                                      final Mat4f mMv, final Mat4f mP,
                                      final Recti viewport,
                                      final Vec3f objPos,
                                      final Mat4f mat4Tmp)
    {
        // invPMv = Inv(P x Mv)
        final Mat4f invPMv = mat4Tmp.mul(mP, mMv);
        if( !invPMv.invert() ) {
            return false;
        }

        final Vec4f winPos = new Vec4f(winx, winy, winz, 1f);

        // Map x and y from window coordinates
        winPos.add(-viewport.x(), -viewport.y(), 0f, 0f).mul(1f/viewport.width(), 1f/viewport.height(), 1f, 1f);

        // Map to range -1 to 1
        winPos.mul(2f, 2f, 2f, 1f).add(-1f, -1f, -1f, 0f);

        // rawObjPos = Inv(P x Mv) *  winPos
        final Vec4f rawObjPos = invPMv.mulVec4f(winPos, new Vec4f());

        if ( rawObjPos.w() == 0.0f ) {
            return false;
        }
        objPos.set( rawObjPos.scale( 1f / rawObjPos.w() ) );

        return true;
    }

    /**
     * Map window coordinates to object coordinates.
     * <p>
     * Traditional <code>gluUnProject</code> implementation.
     * </p>
     *
     * @param winx
     * @param winy
     * @param winz
     * @param invPMv inverse [projection] x [modelview] matrix, i.e. Inv(P x Mv), if null method returns false
     * @param viewport Rect4i viewport
     * @param objPos 3 component object coordinate, the result
     * @return true if successful, otherwise false (null invert matrix, or becomes infinity due to zero z)
     */
    public static boolean mapWinToObj(const float winx, const float winy, const float winz,
                                      final Mat4f invPMv,
                                      final Recti viewport,
                                      final Vec3f objPos)
    {
        if( null == invPMv ) {
            return false;
        }
        final Vec4f winPos = new Vec4f(winx, winy, winz, 1f);

        // Map x and y from window coordinates
        winPos.add(-viewport.x(), -viewport.y(), 0f, 0f).mul(1f/viewport.width(), 1f/viewport.height(), 1f, 1f);

        // Map to range -1 to 1
        winPos.mul(2f, 2f, 2f, 1f).add(-1f, -1f, -1f, 0f);

        // rawObjPos = Inv(P x Mv) *  winPos
        final Vec4f rawObjPos = invPMv.mulVec4f(winPos, new Vec4f());

        if ( rawObjPos.w() == 0.0f ) {
            return false;
        }
        objPos.set( rawObjPos.scale( 1f / rawObjPos.w() ) );

        return true;
    }

    /**
     * Map two window coordinates to two object coordinates,
     * distinguished by their z component.
     * <p>
     * Traditional <code>gluUnProject</code> implementation.
     * </p>
     *
     * @param winx
     * @param winy
     * @param winz1
     * @param winz2
     * @param invPMv inverse [projection] x [modelview] matrix, i.e. Inv(P x Mv), if null method returns false
     * @param viewport Rect4i viewport vector
     * @param objPos1 3 component object coordinate, the result
     * @return true if successful, otherwise false (null invert matrix, or becomes infinity due to zero z)
     */
    public static boolean mapWinToObj(const float winx, const float winy, const float winz1, const float winz2,
                                      final Mat4f invPMv,
                                      final Recti viewport,
                                      final Vec3f objPos1, final Vec3f objPos2)
    {
        if( null == invPMv ) {
            return false;
        }
        final Vec4f winPos = new Vec4f(winx, winy, winz1, 1f);

        // Map x and y from window coordinates
        winPos.add(-viewport.x(), -viewport.y(), 0f, 0f).mul(1f/viewport.width(), 1f/viewport.height(), 1f, 1f);

        // Map to range -1 to 1
        winPos.mul(2f, 2f, 2f, 1f).add(-1f, -1f, -1f, 0f);

        // rawObjPos = Inv(P x Mv) *  winPos1
        final Vec4f rawObjPos = invPMv.mulVec4f(winPos, new Vec4f());

        if ( rawObjPos.w() == 0.0f ) {
            return false;
        }
        objPos1.set( rawObjPos.scale( 1f / rawObjPos.w() ) );

        //
        // winz2
        //
        // Map Z to range -1 to 1
        winPos.setZ( winz2 * 2f - 1f );

        // rawObjPos = Inv(P x Mv) *  winPos2
        invPMv.mulVec4f(winPos, rawObjPos);

        if ( rawObjPos.w() == 0.0f ) {
            return false;
        }
        objPos2.set( rawObjPos.scale( 1f / rawObjPos.w() ) );

        return true;
    }

    /**
     * Map window coordinates to object coordinates.
     * <p>
     * Traditional <code>gluUnProject4</code> implementation.
     * </p>
     *
     * @param winx
     * @param winy
     * @param winz
     * @param clipw
     * @param mMv 4x4 modelview matrix
     * @param mP 4x4 projection matrix
     * @param viewport Rect4i viewport vector
     * @param near
     * @param far
     * @param obj_pos 4 component object coordinate, the result
     * @param mat4Tmp 16 component matrix for temp storage
     * @return true if successful, otherwise false (failed to invert matrix, or becomes infinity due to zero z)
     */
    public static boolean mapWinToObj4(const float winx, const float winy, const float winz, const float clipw,
                                       final Mat4f mMv, final Mat4f mP,
                                       final Recti viewport,
                                       const float near, const float far,
                                       final Vec4f objPos,
                                       final Mat4f mat4Tmp)
    {
        // invPMv = Inv(P x Mv)
        final Mat4f invPMv = mat4Tmp.mul(mP, mMv);
        if( !invPMv.invert() ) {
            return false;
        }

        final Vec4f winPos = new Vec4f(winx, winy, winz, clipw);

        // Map x and y from window coordinates
        winPos.add(-viewport.x(), -viewport.y(), -near, 0f).mul(1f/viewport.width(), 1f/viewport.height(), 1f/(far-near), 1f);

        // Map to range -1 to 1
        winPos.mul(2f, 2f, 2f, 1f).add(-1f, -1f, -1f, 0f);

        // objPos = Inv(P x Mv) *  winPos
        invPMv.mulVec4f(winPos, objPos);

        if ( objPos.w() == 0.0f ) {
            return false;
        }
        return true;
    }

    /**
     * Map window coordinates to object coordinates.
     * <p>
     * Traditional <code>gluUnProject4</code> implementation.
     * </p>
     *
     * @param winx
     * @param winy
     * @param winz
     * @param clipw
     * @param invPMv inverse [projection] x [modelview] matrix, i.e. Inv(P x Mv), if null method returns false
     * @param viewport Rect4i viewport vector
     * @param near
     * @param far
     * @param obj_pos 4 component object coordinate, the result
     * @return true if successful, otherwise false (null invert matrix, or becomes infinity due to zero z)
     */
    public static boolean mapWinToObj4(const float winx, const float winy, const float winz, const float clipw,
                                       final Mat4f invPMv,
                                       final Recti viewport,
                                       const float near, const float far,
                                       final Vec4f objPos)
    {
        if( null == invPMv ) {
            return false;
        }
        final Vec4f winPos = new Vec4f(winx, winy, winz, clipw);

        // Map x and y from window coordinates
        winPos.add(-viewport.x(), -viewport.y(), -near, 0f).mul(1f/viewport.width(), 1f/viewport.height(), 1f/(far-near), 1f);

        // Map to range -1 to 1
        winPos.mul(2f, 2f, 2f, 1f).add(-1f, -1f, -1f, 0f);

        // objPos = Inv(P x Mv) *  winPos
        invPMv.mulVec4f(winPos, objPos);

        if ( objPos.w() == 0.0f ) {
            return false;
        }
        return true;
    }

    /**
     * Map two window coordinates w/ shared X/Y and distinctive Z
     * to a {@link Ray}. The resulting {@link Ray} maybe used for <i>picking</i>
     * using a {@link AABBox#getRayIntersection(vec3f, Ray, float, boolean)}.
     * <p>
     * Notes for picking <i>winz0</i> and <i>winz1</i>:
     * <ul>
     *   <li>see {@link FloatUtil#getZBufferEpsilon(int, float, float)}</li>
     *   <li>see {@link FloatUtil#getZBufferValue(int, float, float, float)}</li>
     *   <li>see {@link FloatUtil#getOrthoWinZ(float, float, float)}</li>
     * </ul>
     * </p>
     * @param winx
     * @param winy
     * @param winz0
     * @param winz1
     * @param mMv 4x4 modelview matrix
     * @param mP 4x4 projection matrix
     * @param viewport Rect4i viewport
     * @param ray storage for the resulting {@link Ray}
     * @param mat4Tmp1 16 component matrix for temp storage
     * @param mat4Tmp2 16 component matrix for temp storage
     * @return true if successful, otherwise false (failed to invert matrix, or becomes z is infinity)
     */
    public static boolean mapWinToRay(const float winx, const float winy, const float winz0, const float winz1,
                                      final Mat4f mMv, final Mat4f mP,
                                      final Recti viewport,
                                      final Ray ray,
                                      final Mat4f mat4Tmp1, final Mat4f mat4Tmp2) {
        // invPMv = Inv(P x Mv)
        final Mat4f invPMv = mat4Tmp1.mul(mP, mMv);
        if( !invPMv.invert() ) {
            return false;
        }

        if( mapWinToObj(winx, winy, winz0, winz1, invPMv, viewport, ray.orig, ray.dir) ) {
            ray.dir.sub(ray.orig).normalize();
            return true;
        } else {
            return false;
        }
    }

    /**
     * Map two window coordinates w/ shared X/Y and distinctive Z
     * to a {@link Ray}. The resulting {@link Ray} maybe used for <i>picking</i>
     * using a {@link AABBox#getRayIntersection(vec3f, Ray, float, boolean)}.
     * <p>
     * Notes for picking <i>winz0</i> and <i>winz1</i>:
     * <ul>
     *   <li>see {@link FloatUtil#getZBufferEpsilon(int, float, float)}</li>
     *   <li>see {@link FloatUtil#getZBufferValue(int, float, float, float)}</li>
     *   <li>see {@link FloatUtil#getOrthoWinZ(float, float, float)}</li>
     * </ul>
     * </p>
     * @param winx
     * @param winy
     * @param winz0
     * @param winz1
     * @param invPMv inverse [projection] x [modelview] matrix, i.e. Inv(P x Mv), if null method returns false
     * @param viewport Rect4i viewport
     * @param ray storage for the resulting {@link Ray}
     * @return true if successful, otherwise false (null invert matrix, or becomes z is infinity)
     */
    public static boolean mapWinToRay(const float winx, const float winy, const float winz0, const float winz1,
                                      final Mat4f invPMv,
                                      final Recti viewport,
                                      final Ray ray) {
        if( mapWinToObj(winx, winy, winz0, winz1, invPMv, viewport, ray.orig, ray.dir) ) {
            ray.dir.sub(ray.orig).normalize();
            return true;
        } else {
            return false;
        }
    }

    //
    // String and internals
    //

    /**
     * @param sb optional passed StringBuilder instance to be used
     * @param rowPrefix optional prefix for each row
     * @param f the format string of one floating point, i.e. "%10.5f", see {@link java.util.Formatter}
     * @return matrix string representation
     */
    public StringBuilder toString(final StringBuilder sb, final String rowPrefix, final String f) {
        const float[] tmp = new float[16];
        this.get(tmp);
        return FloatUtil.matrixToString(sb, rowPrefix, f,tmp, 0, 4, 4, false /* rowMajorOrder */);
    }

    @Override
    public String toString() {
        return toString(null, null, "%10.5f").toString();
    }
#endif

    /**
     * Returns a formatted string representation of this matrix
     * @param rowPrefix prefix for each row
     * @param f format string for each float element, e.g. "%10.5f"
     * @return matrix
     */
    std::string toString(const std::string& rowPrefix, const std::string& f) const noexcept;

    std::string toString() const noexcept { return toString("", "%10.5f"); }
};

std::ostream& operator<<(std::ostream& out, const Mat4f& v) noexcept {
    return out << v.toString();
}

/**@}*/

} // namespace jau::math

#endif // MAT4f_HPP_
