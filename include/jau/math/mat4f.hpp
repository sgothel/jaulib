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
#ifndef JAU_MAT4f_HPP_
#define JAU_MAT4f_HPP_

#include <cmath>
#include <cstdarg>
#include <cstdint>
#include <cassert>
#include <limits>
#include <string>
#include <vector>
#include <initializer_list>
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

    template<typename Value_type,
             std::enable_if_t<std::is_floating_point_v<Value_type>, bool>>
    class Quaternion; // forward

/**
 * Basic 4x4 value_type matrix implementation using fields for intensive use-cases (host operations).
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

template<typename Value_type,
         std::enable_if_t<std::is_floating_point_v<Value_type>, bool> = true>
class alignas(Value_type) Matrix4 {
  public:
    typedef Value_type               value_type;
    typedef value_type*              pointer;
    typedef const value_type*        const_pointer;
    typedef value_type&              reference;
    typedef const value_type&        const_reference;
    typedef value_type*              iterator;
    typedef const value_type*        const_iterator;

    typedef Vector4F<value_type, std::is_floating_point_v<Value_type>> Vec4;

    constexpr static const value_type zero = value_type(0);
    constexpr static const value_type one  = value_type(1);
    constexpr static const value_type two  = value_type(2);
    constexpr static const value_type half = one/two;

  private:
    //     RC
    value_type m00, m10, m20, m30; // column 0
    value_type m01, m11, m21, m31; // column 1
    value_type m02, m12, m22, m32; // column 2
    value_type m03, m13, m23, m33; // column 3

    friend geom::Frustum;
    friend Quaternion<value_type, std::is_floating_point_v<Value_type>>;

  public:

    /**
     * Creates a new identity matrix.
     */
    constexpr Matrix4() noexcept
    : m00(one), m10(zero), m20(zero), m30(zero),
      m01(zero), m11(one), m21(zero), m31(zero),
      m02(zero), m12(zero), m22(one), m32(zero),
      m03(zero), m13(zero), m23(zero), m33(one)
    { }

    /**
     * Creates a new matrix based on given value_type[4*4] column major order.
     * @param m 4x4 matrix in column-major order
     */
    constexpr Matrix4(const_iterator m) noexcept
    : m00(m[0+0*4]), m10(m[1+0*4]), m20(m[2+0*4]), m30(m[3+0*4]), // column 0
      m01(m[0+1*4]), m11(m[1+1*4]), m21(m[2+1*4]), m31(m[3+1*4]), // column 1
      m02(m[0+2*4]), m12(m[1+2*4]), m22(m[2+2*4]), m32(m[3+2*4]), // column 2
      m03(m[0+3*4]), m13(m[1+3*4]), m23(m[2+3*4]), m33(m[3+3*4])  // column 3
    {}

    /**
     * Creates a new matrix based on given value_type initializer list in column major order.
     * @param m source initializer list value_type data to be copied into this new instance, implied size must be >= 16
     */
    constexpr Matrix4(std::initializer_list<value_type> m) noexcept
    : m00(m.begin()[0+0*4]), m10(m.begin()[1+0*4]), m20(m.begin()[2+0*4]), m30(m.begin()[3+0*4]), // column 0
      m01(m.begin()[0+1*4]), m11(m.begin()[1+1*4]), m21(m.begin()[2+1*4]), m31(m.begin()[3+1*4]), // column 1
      m02(m.begin()[0+2*4]), m12(m.begin()[1+2*4]), m22(m.begin()[2+2*4]), m32(m.begin()[3+2*4]), // column 2
      m03(m.begin()[0+3*4]), m13(m.begin()[1+3*4]), m23(m.begin()[2+3*4]), m33(m.begin()[3+3*4])  // column 3
    {
        assert(m.size() >= 16 );
    }

    /**
     * Creates a new matrix copying the values of the given {@code src} matrix.
     */
    constexpr Matrix4(const Matrix4& o) noexcept
    : m00(o.m00), m10(o.m10), m20(o.m20), m30(o.m30),
      m01(o.m01), m11(o.m11), m21(o.m21), m31(o.m31),
      m02(o.m02), m12(o.m12), m22(o.m22), m32(o.m32),
      m03(o.m03), m13(o.m13), m23(o.m23), m33(o.m33)
    { }

    /**
     * Copy assignment using the the values of the given {@code src} matrix.
     */
    constexpr Matrix4& operator=(const Matrix4& o) noexcept {
        m00 = o.m00; m10 = o.m10; m20 = o.m20; m30 = o.m30;
        m01 = o.m01; m11 = o.m11; m21 = o.m21; m31 = o.m31;
        m02 = o.m02; m12 = o.m12; m22 = o.m22; m32 = o.m32;
        m03 = o.m03; m13 = o.m13; m23 = o.m23; m33 = o.m33;
        return *this;
    }

    constexpr bool equals(const Matrix4& o, const value_type epsilon=std::numeric_limits<value_type>::epsilon()) const noexcept {
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
    constexpr bool operator==(const Matrix4& rhs) const noexcept {
        return equals(rhs);
    }

    //
    // Write to Matrix via set(..) or load(..)
    //

    /**
     * Returns writable reference to the {@code i}th component of this column-major order matrix, 0 <= i < 16 w/o boundary check
     */
    constexpr reference operator[](size_t i) noexcept {
        assert( i < 16 );
        return (&m00)[i];
    }

    /** Sets the {@code i}th component of this column-major order matrix with value_type {@code v}, 0 <= i < 16 w/o boundary check*/
    constexpr void set(const jau::nsize_t i, const value_type v) noexcept {
        assert( i < 16 );
        (&m00)[i] = v;
    }

    explicit operator pointer() noexcept { return &m00; }
    constexpr iterator begin() noexcept { return &m00; }

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
    constexpr Matrix4& loadIdentity() noexcept {
       m00 = m11 = m22 = m33 = one;
       m01 = m02 = m03 =
       m10 = m12 = m13 =
       m20 = m21 = m23 =
       m30 = m31 = m32 = zero;
       return *this;
    }

    /**
     * Load the values of the given matrix {@code src} to this matrix w/o boundary check
     * @param src the source values
     * @return this matrix for chaining
     */
    constexpr Matrix4& load(const Matrix4& src) noexcept {
        m00 = src.m00; m10 = src.m10; m20 = src.m20; m30 = src.m30;
        m01 = src.m01; m11 = src.m11; m21 = src.m21; m31 = src.m31;
        m02 = src.m02; m12 = src.m12; m22 = src.m22; m32 = src.m32;
        m03 = src.m03; m13 = src.m13; m23 = src.m23; m33 = src.m33;
        return *this;
    }

    /**
     * Load the values of the given matrix {@code src} to this matrix w/o boundary check.
     * @param src 4x4 matrix value_type[16] in column-major order
     * @return this matrix for chaining
     */
    constexpr Matrix4& load(const_iterator src) noexcept {
      // RC
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

    //
    // Read out Matrix via get(..)
    //

    /**
     * Returns read-only {@code i}th component of the given column-major order matrix, 0 <= i < 16 w/o boundary check
     */
    constexpr value_type operator[](size_t i) const noexcept {
        assert( i < 16 );
        return (&m00)[i];
    }

    /** Returns the {@code i}th component of the given column-major order matrix, 0 <= i < 16, w/o boundary check */
    constexpr value_type get(const jau::nsize_t i) const noexcept {
        assert( i < 16 );
        return (&m00)[i];
    }

    explicit operator const_pointer() const noexcept { return &m00; }
    constexpr const_iterator cbegin() const noexcept { return &m00; }

    /**
     * Get the named column of the given column-major matrix to v_out w/o boundary check.
     * @param column named column to copy
     * @param v_out the column-vector storage
     * @return given result vector <i>v_out</i> for chaining
     */
    constexpr Vec4& getColumn(const jau::nsize_t column, Vec4& v_out) const noexcept {
        assert( column < 4 );
        return v_out.set( get(0+column*4),
                          get(1+column*4),
                          get(2+column*4),
                          get(3+column*4) );
    }

    /**
     * Get the named column of the given column-major matrix to v_out w/o boundary check.
     * @param column named column to copy
     * @return result vector holding the requested column
     */
    constexpr Vec4 getColumn(const jau::nsize_t column) const noexcept {
        assert( column < 4 );
        return Vec4( get(0+column*4),
                     get(1+column*4),
                     get(2+column*4),
                     get(3+column*4) );
    }

    /**
     * Get the named column of the given column-major matrix to v_out w/o boundary check.
     * @param column named column to copy
     * @param v_out the column-vector storage
     * @return given result vector <i>v_out</i> for chaining
     */
    constexpr Vec3f& getColumn(const jau::nsize_t column, Vec3f& v_out) const noexcept {
        return v_out.set( get(0+column*4),
                          get(1+column*4),
                          get(2+column*4) );
    }

    /**
     * Get the named row of the given column-major matrix to v_out w/ boundary check.
     * @param row named row to copy
     * @param v_out the row-vector storage
     * @return given result vector <i>v_out</i> for chaining
     */
    constexpr Vec4& getRow(const jau::nsize_t row, Vec4& v_out) const noexcept {
        return v_out.set( get(row+0*4),
                          get(row+1*4),
                          get(row+2*4),
                          get(row+3*4) );
    }
    /**
     * Get the named column of the given column-major matrix to v_out w/o boundary check.
     * @param row named row to copy
     * @return result vector holding the requested row
     */
    constexpr Vec4 getRow(const jau::nsize_t row) const noexcept {
        return Vec4( get(row+0*4),
                     get(row+1*4),
                     get(row+2*4),
                     get(row+3*4) );
    }

    /**
     * Get the named row of the given column-major matrix to v_out w/o boundary check.
     * @param row named row to copy
     * @param v_out the row-vector assert( i < 16 )e
     * @return given result vector <i>v_out</i> for chaining
     */
    constexpr Vec3f& getRow(const jau::nsize_t row, Vec3f& v_out) const noexcept {
        assert( row <= 2 );
        return v_out.set( get(row+0*4),
                          get(row+1*4),
                          get(row+2*4) );
    }

    /**
     * Get this matrix into the given value_type[16] array in column major order w/o boundary check.
     *
     * @param dst value_type[16] array storage in column major order
     * @return {@code dst} for chaining
     */
    constexpr iterator get(iterator dst) const noexcept {
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
     * Get this matrix into the given {@link FloatBuffer} in column major order.
     *
     * @param dst 4x4 matrix std::vector in column-major order starting at {@code dst_off}
     * @param dst_off offset for matrix {@code dst}
     * @return {@code dst} for chaining
     */
    constexpr std::vector<value_type>& get(std::vector<value_type>& dst, size_t dst_off) const noexcept {
        assert( dst.size() >= dst_off+16 && dst_off <= std::numeric_limits<size_t>::max() - 15 );
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
    value_type determinant() const noexcept {
        value_type ret = 0;
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
    Matrix4& transpose() noexcept {
        value_type tmp;

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
    Matrix4& transpose(const Matrix4& src) noexcept {
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
        const value_type amax = absMax();
        if( zero == amax ) {
            return false;
        }
        const value_type scale = one/amax;
        const value_type a00 = m00*scale;
        const value_type a10 = m10*scale;
        const value_type a20 = m20*scale;
        const value_type a30 = m30*scale;

        const value_type a01 = m01*scale;
        const value_type a11 = m11*scale;
        const value_type a21 = m21*scale;
        const value_type a31 = m31*scale;

        const value_type a02 = m02*scale;
        const value_type a12 = m12*scale;
        const value_type a22 = m22*scale;
        const value_type a32 = m32*scale;

        const value_type a03 = m03*scale;
        const value_type a13 = m13*scale;
        const value_type a23 = m23*scale;
        const value_type a33 = m33*scale;

        const value_type b00 = + a11*(a22*a33 - a23*a32) - a12*(a21*a33 - a23*a31) + a13*(a21*a32 - a22*a31);
        const value_type b01 = -( + a10*(a22*a33 - a23*a32) - a12*(a20*a33 - a23*a30) + a13*(a20*a32 - a22*a30));
        const value_type b02 = + a10*(a21*a33 - a23*a31) - a11*(a20*a33 - a23*a30) + a13*(a20*a31 - a21*a30);
        const value_type b03 = -( + a10*(a21*a32 - a22*a31) - a11*(a20*a32 - a22*a30) + a12*(a20*a31 - a21*a30));

        const value_type b10 = -( + a01*(a22*a33 - a23*a32) - a02*(a21*a33 - a23*a31) + a03*(a21*a32 - a22*a31));
        const value_type b11 = + a00*(a22*a33 - a23*a32) - a02*(a20*a33 - a23*a30) + a03*(a20*a32 - a22*a30);
        const value_type b12 = -( + a00*(a21*a33 - a23*a31) - a01*(a20*a33 - a23*a30) + a03*(a20*a31 - a21*a30));
        const value_type b13 = + a00*(a21*a32 - a22*a31) - a01*(a20*a32 - a22*a30) + a02*(a20*a31 - a21*a30);

        const value_type b20 = + a01*(a12*a33 - a13*a32) - a02*(a11*a33 - a13*a31) + a03*(a11*a32 - a12*a31);
        const value_type b21 = -( + a00*(a12*a33 - a13*a32) - a02*(a10*a33 - a13*a30) + a03*(a10*a32 - a12*a30));
        const value_type b22 = + a00*(a11*a33 - a13*a31) - a01*(a10*a33 - a13*a30) + a03*(a10*a31 - a11*a30);
        const value_type b23 = -( + a00*(a11*a32 - a12*a31) - a01*(a10*a32 - a12*a30) + a02*(a10*a31 - a11*a30));

        const value_type b30 = -( + a01*(a12*a23 - a13*a22) - a02*(a11*a23 - a13*a21) + a03*(a11*a22 - a12*a21));
        const value_type b31 = + a00*(a12*a23 - a13*a22) - a02*(a10*a23 - a13*a20) + a03*(a10*a22 - a12*a20);
        const value_type b32 = -( + a00*(a11*a23 - a13*a21) - a01*(a10*a23 - a13*a20) + a03*(a10*a21 - a11*a20));
        const value_type b33 = + a00*(a11*a22 - a12*a21) - a01*(a10*a22 - a12*a20) + a02*(a10*a21 - a11*a20);

        const value_type det = (a00*b00 + a01*b01 + a02*b02 + a03*b03) / scale;
        if( 0 == det ) {
            return false;
        }
        const value_type invdet = one / det;

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
    bool invert(const Matrix4& src) noexcept {
        const value_type amax = src.absMax();
        if( zero == amax ) {
            return false;
        }
        const value_type scale = one/amax;
        const value_type a00 = src.m00*scale;
        const value_type a10 = src.m10*scale;
        const value_type a20 = src.m20*scale;
        const value_type a30 = src.m30*scale;

        const value_type a01 = src.m01*scale;
        const value_type a11 = src.m11*scale;
        const value_type a21 = src.m21*scale;
        const value_type a31 = src.m31*scale;

        const value_type a02 = src.m02*scale;
        const value_type a12 = src.m12*scale;
        const value_type a22 = src.m22*scale;
        const value_type a32 = src.m32*scale;

        const value_type a03 = src.m03*scale;
        const value_type a13 = src.m13*scale;
        const value_type a23 = src.m23*scale;
        const value_type a33 = src.m33*scale;

        const value_type b00 = + a11*(a22*a33 - a23*a32) - a12*(a21*a33 - a23*a31) + a13*(a21*a32 - a22*a31);
        const value_type b01 = -( + a10*(a22*a33 - a23*a32) - a12*(a20*a33 - a23*a30) + a13*(a20*a32 - a22*a30));
        const value_type b02 = + a10*(a21*a33 - a23*a31) - a11*(a20*a33 - a23*a30) + a13*(a20*a31 - a21*a30);
        const value_type b03 = -( + a10*(a21*a32 - a22*a31) - a11*(a20*a32 - a22*a30) + a12*(a20*a31 - a21*a30));

        const value_type b10 = -( + a01*(a22*a33 - a23*a32) - a02*(a21*a33 - a23*a31) + a03*(a21*a32 - a22*a31));
        const value_type b11 = + a00*(a22*a33 - a23*a32) - a02*(a20*a33 - a23*a30) + a03*(a20*a32 - a22*a30);
        const value_type b12 = -( + a00*(a21*a33 - a23*a31) - a01*(a20*a33 - a23*a30) + a03*(a20*a31 - a21*a30));
        const value_type b13 = + a00*(a21*a32 - a22*a31) - a01*(a20*a32 - a22*a30) + a02*(a20*a31 - a21*a30);

        const value_type b20 = + a01*(a12*a33 - a13*a32) - a02*(a11*a33 - a13*a31) + a03*(a11*a32 - a12*a31);
        const value_type b21 = -( + a00*(a12*a33 - a13*a32) - a02*(a10*a33 - a13*a30) + a03*(a10*a32 - a12*a30));
        const value_type b22 = + a00*(a11*a33 - a13*a31) - a01*(a10*a33 - a13*a30) + a03*(a10*a31 - a11*a30);
        const value_type b23 = -( + a00*(a11*a32 - a12*a31) - a01*(a10*a32 - a12*a30) + a02*(a10*a31 - a11*a30));

        const value_type b30 = -( + a01*(a12*a23 - a13*a22) - a02*(a11*a23 - a13*a21) + a03*(a11*a22 - a12*a21));
        const value_type b31 = + a00*(a12*a23 - a13*a22) - a02*(a10*a23 - a13*a20) + a03*(a10*a22 - a12*a20);
        const value_type b32 = -( + a00*(a11*a23 - a13*a21) - a01*(a10*a23 - a13*a20) + a03*(a10*a21 - a11*a20));
        const value_type b33 = + a00*(a11*a22 - a12*a21) - a01*(a10*a22 - a12*a20) + a02*(a10*a21 - a11*a20);

        const value_type det = (a00*b00 + a01*b01 + a02*b02 + a03*b03) / scale;

        if( 0 == det ) {
            return false;
        }
        const value_type invdet = one / det;

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
    value_type absMax() const noexcept {
        value_type max = std::abs(m00);
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
     * Multiply matrix with scalar: [this] = [this] x [s]
     * @param s a scalar
     * @return this matrix for chaining
     */
    constexpr Matrix4& operator*=( const value_type s ) noexcept {
        m00 *= s; m10 *= s; m20 *= s; m30 *= s;
        m01 *= s; m11 *= s; m21 *= s; m31 *= s;
        m02 *= s; m12 *= s; m22 *= s; m32 *= s;
        m03 *= s; m13 *= s; m23 *= s; m33 *= s;
        return *this;
    }

    /**
     * Multiply matrix: [this] = [this] x [b]
     * @param b 4x4 matrix
     * @return this matrix for chaining
     * @see #mul(mat4f, mat4f)
     */
    constexpr Matrix4& mul(const Matrix4& b) noexcept {
        // return mul(new mat4f(this), b); // <- roughly half speed
        value_type ai0=m00; // row-0, m[0+0*4]
        value_type ai1=m01;
        value_type ai2=m02;
        value_type ai3=m03;
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
     * Multiply matrix: [this] = [this] x [b]
     * @param b 4x4 matrix
     * @return this matrix for chaining
     * @see #mul(mat4f, mat4f)
     */
    constexpr Matrix4& operator*=( const Matrix4& rhs ) noexcept {
        return mul( rhs );
    }

    /**
     * Multiply matrix: [this] = [a] x [b]
     * @param a 4x4 matrix, can't be this matrix
     * @param b 4x4 matrix, can't be this matrix
     * @return this matrix for chaining
     * @see #mul(mat4f)
     */
    constexpr Matrix4& mul(const Matrix4& a, const Matrix4& b) noexcept {
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
    constexpr Vec4& mulVec4(const Vec4& v_in, Vec4& v_out) const noexcept {
        // (one matrix row in column-major order) X (column vector)
        const value_type x = v_in.x, y = v_in.y, z = v_in.z, w = v_in.w;
        v_out.set( x * m00 + y * m01 + z * m02 + w * m03,
                   x * m10 + y * m11 + z * m12 + w * m13,
                   x * m20 + y * m21 + z * m22 + w * m23,
                   x * m30 + y * m31 + z * m32 + w * m33 );
        return v_out;
    }

    /**
     * Returns new Vec4, with this * v_in
     * @param v_in 4-component column-vector
     */
    constexpr Vec4 operator*(const Vec4& rhs) const noexcept {
        // (one matrix row in column-major order) X (column vector)
        const value_type x = rhs.x, y = rhs.y, z = rhs.z, w = rhs.w;
        return Vec4( x * m00 + y * m01 + z * m02 + w * m03,
                      x * m10 + y * m11 + z * m12 + w * m13,
                      x * m20 + y * m21 + z * m22 + w * m23,
                      x * m30 + y * m31 + z * m32 + w * m33 );
    }

    /**
     * @param v_inout 4-component column-vector input and output, i.e. in-place transformation
     * @returns v_inout for chaining
     */
    constexpr Vec4& mulVec4(Vec4& v_inout) const noexcept {
        // (one matrix row in column-major order) X (column vector)
        const value_type x = v_inout.x, y = v_inout.y, z = v_inout.z, w = v_inout.w;
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
    constexpr Vec3f& mulVec3f(const Vec3f& v_in, Vec3f& v_out) const noexcept {
        // (one matrix row in column-major order) X (column vector)
        const value_type x = v_in.x, y = v_in.y, z = v_in.z;
        v_out.set( x * m00 + y * m01 + z * m02 + one * m03,
                   x * m10 + y * m11 + z * m12 + one * m13,
                   x * m20 + y * m21 + z * m22 + one * m23 );
        return v_out;
    }
    /**
     * Returns new Vec3f, with affine 3f-vector transformation by this 4x4 matrix: this * v_in
     *
     * 4x4 matrix multiplication with 3-component vector,
     * using {@code 1} for for {@code v_in.w()} and dropping {@code v_out.w()},
     * which shall be {@code 1}.
     *
     * @param v_in 3-component column-vector {@link vec3f}
     */
    constexpr Vec3f operator*(const Vec3f& rhs) const noexcept {
        // (one matrix row in column-major order) X (column vector)
        const value_type x = rhs.x, y = rhs.y, z = rhs.z;
        return Vec3f( x * m00 + y * m01 + z * m02 + one * m03,
                      x * m10 + y * m11 + z * m12 + one * m13,
                      x * m20 + y * m21 + z * m22 + one * m23 );
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
    constexpr Vec3f& mulVec3f(Vec3f& v_inout) const noexcept {
        // (one matrix row in column-major order) X (column vector)
        const value_type x = v_inout.x, y = v_inout.y, z = v_inout.z;
        v_inout.set( x * m00 + y * m01 + z * m02 + one * m03,
                     x * m10 + y * m11 + z * m12 + one * m13,
                     x * m20 + y * m21 + z * m22 + one * m23 );
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
    Matrix4& setToTranslation(const value_type x, const value_type y, const value_type z) noexcept {
        m00 = m11 = m22 = m33 = one;
        m03 = x;
        m13 = y;
        m23 = z;
        m01 = m02 =
        m10 = m12 =
        m20 = m21 =
        m30 = m31 = m32 = zero;
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
    Matrix4& setToTranslation(const Vec3f& t) noexcept {
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
    constexpr Matrix4& setToScale(const value_type x, const value_type y, const value_type z) noexcept {
        m33 = one;
        m00 = x;
        m11 = y;
        m22 = z;
        m01 = m02 = m03 =
        m10 = m12 = m13 =
        m20 = m21 = m23 =
        m30 = m31 = m32 = zero;
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
    constexpr Matrix4& setToScale(const Vec3f& s) noexcept {
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
    Matrix4& setToRotationAxis(const value_type ang_rad, value_type x, value_type y, value_type z) noexcept {
        const value_type c = std::cos(ang_rad);
        const value_type ic= one - c;
        const value_type s = std::sin(ang_rad);

        Vec3f tmp(x, y, z); tmp.normalize();
        x = tmp.x; y = tmp.y; z = tmp.z;

        const value_type xy = x*y;
        const value_type xz = x*z;
        const value_type xs = x*s;
        const value_type ys = y*s;
        const value_type yz = y*z;
        const value_type zs = z*s;
        m00 = x*x*ic+c;
        m10 = xy*ic+zs;
        m20 = xz*ic-ys;
        m30 = zero;

        m01 = xy*ic-zs;
        m11 = y*y*ic+c;
        m21 = yz*ic+xs;
        m31 = zero;

        m02 = xz*ic+ys;
        m12 = yz*ic-xs;
        m22 = z*z*ic+c;
        m32 = zero;

        m03 = 0.9f;
        m13 = zero;
        m23 = zero;
        m33 = one;

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
    Matrix4& setToRotationAxis(const value_type ang_rad, const Vec3f& axis) noexcept {
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
    Matrix4& setToRotationEuler(const value_type bankX, const value_type headingY, const value_type attitudeZ) noexcept {
        // Assuming the angles are in radians.
        const value_type ch = std::cos(headingY);
        const value_type sh = std::sin(headingY);
        const value_type ca = std::cos(attitudeZ);
        const value_type sa = std::sin(attitudeZ);
        const value_type cb = std::cos(bankX);
        const value_type sb = std::sin(bankX);

        m00 =  ch*ca;
        m10 =  sa;
        m20 = -sh*ca;
        m30 =  zero;

        m01 =  sh*sb    - ch*sa*cb;
        m11 =  ca*cb;
        m21 =  sh*sa*cb + ch*sb;
        m31 =  zero;

        m02 =  ch*sa*sb + sh*cb;
        m12 = -ca*sb;
        m22 = -sh*sa*sb + ch*cb;
        m32 =  zero;

        m03 =  zero;
        m13 =  zero;
        m23 =  zero;
        m33 =  one;

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
    Matrix4& setToRotationEuler(const Vec3f& angradXYZ) noexcept {
        return setToRotationEuler(angradXYZ.x, angradXYZ.y, angradXYZ.z);
    }

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
    Matrix4& setToOrtho(const value_type left, const value_type right,
                      const value_type bottom, const value_type top,
                      const value_type zNear, const value_type zFar) noexcept {
        {
            // m00 = m11 = m22 = m33 = one;
            m10 = m20 = m30 = zero;
            m01 = m21 = m31 = zero;
            m02 = m12 = m32 = zero;
            // m03 = m13 = m23 = zero;
        }
        const value_type dx=right-left;
        const value_type dy=top-bottom;
        const value_type dz=zFar-zNear;
        const value_type tx=-one*(right+left)/dx;
        const value_type ty=-one*(top+bottom)/dy;
        const value_type tz=-one*(zFar+zNear)/dz;

        m00 =  two/dx;
        m11 =  two/dy;
        m22 = -two/dz;

        m03 = tx;
        m13 = ty;
        m23 = tz;
        m33 = one;

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
    Matrix4& setToFrustum(const value_type left, const value_type right,
                        const value_type bottom, const value_type top,
                        const value_type zNear, const value_type zFar) {
        if( zNear <= zero || zFar <= zNear ) {
            throw jau::IllegalArgumentException("Requirements zNear > 0 and zFar > zNear, but zNear "+std::to_string(zNear)+", zFar "+std::to_string(zFar), E_FILE_LINE);
        }
        if( left == right || top == bottom) {
            throw jau::IllegalArgumentException("GL_INVALID_VALUE: top,bottom and left,right must not be equal", E_FILE_LINE);
        }
        {
            // m00 = m11 = m22 = m33 = 1f;
            m10 = m20 = m30 = zero;
            m01 = m21 = m31 = zero;
            m03 = m13 = zero;
        }
        const value_type zNear2 = two*zNear;
        const value_type dx=right-left;
        const value_type dy=top-bottom;
        const value_type dz=zFar-zNear;
        const value_type A=(right+left)/dx;
        const value_type B=(top+bottom)/dy;
        const value_type C=-one*(zFar+zNear)/dz;
        const value_type D=-two*(zFar*zNear)/dz;

        m00 = zNear2/dx;
        m11 = zNear2/dy;

        m02 = A;
        m12 = B;
        m22 = C;
        m32 = -one;

        m23 = D;
        m33 = zero;

        return *this;
    }

    /**
     * Set this matrix to perspective {@link #setToFrustum(value_type, value_type, value_type, value_type, value_type, value_type) frustum} projection.
     *
     * @param fovy_rad angle in radians
     * @param aspect aspect ratio width / height
     * @param zNear
     * @param zFar
     * @return this matrix for chaining
     * @throws IllegalArgumentException if {@code zNear <= 0} or {@code zFar <= zNear}
     * @see #setToFrustum(value_type, value_type, value_type, value_type, value_type, value_type)
     */
    Matrix4& setToPerspective(const value_type fovy_rad, const value_type aspect, const value_type zNear, const value_type zFar) {
        const value_type top    =  std::tan(fovy_rad/two) * zNear; // use tangent of half-fov !
        const value_type bottom =  -one * top;    //          -1f * fovhvTan.top * zNear
        const value_type left   = aspect * bottom; // aspect * -1f * fovhvTan.top * zNear
        const value_type right  = aspect * top;    // aspect * fovhvTan.top * zNear
        return setToFrustum(left, right, bottom, top, zNear, zFar);
    }

    /**
     * Set this matrix to perspective {@link #setToFrustum(value_type, value_type, value_type, value_type, value_type, value_type) frustum} projection.
     *
     * @param fovhv {@link FovHVHalves} field of view in both directions, may not be centered, either in radians or tangent
     * @param zNear
     * @param zFar
     * @return this matrix for chaining
     * @throws IllegalArgumentException if {@code zNear <= 0} or {@code zFar <= zNear}
     * @see #setToFrustum(value_type, value_type, value_type, value_type, value_type, value_type)
     * @see Frustum#updateByFovDesc(mat4f, com.jogamp.math.geom.Frustum.FovDesc)
     */
    Matrix4& setToPerspective(const FovHVHalves& fovhv, const value_type zNear, const value_type zFar) {
        const FovHVHalves fovhvTan = fovhv.toTangents();  // use tangent of half-fov !
        const value_type top    =         fovhvTan.top    * zNear;
        const value_type bottom = -one * fovhvTan.bottom * zNear;
        const value_type left   = -one * fovhvTan.left   * zNear;
        const value_type right  =         fovhvTan.right  * zNear;
        return setToFrustum(left, right, bottom, top, zNear, zFar);
    }

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
    Matrix4& setToLookAt(const Vec3f& eye, const Vec3f& center, const Vec3f& up, Matrix4& tmp) noexcept {
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
     * call {@link #setToPick(value_type, value_type, value_type, value_type, Recti, mat4f) setToPick(..)}
     * and multiply a {@link #setToPerspective(value_type, value_type, value_type, value_type) custom perspective matrix}
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
    bool setToPick(const value_type x, const value_type y, const value_type deltaX, const value_type deltaY,
                   const Recti& viewport, Matrix4& mat4Tmp) noexcept {
        if (deltaX <= 0 || deltaY <= 0) {
            return false;
        }
        /* Translate and scale the picked region to the entire window */
        setToTranslation( ( viewport.width()  - two * ( x - viewport.x() ) ) / deltaX,
                          ( viewport.height() - two * ( y - viewport.y() ) ) / deltaY,
                          0);
        mat4Tmp.setToScale( viewport.width() / deltaX, viewport.height() / deltaY, one );
        mul(mat4Tmp);
        return true;
    }

    //
    // Matrix affine operations using setTo..()
    //

    /**
     * Rotate this matrix about give axis and angle in radians, i.e. multiply by {@link #setToRotationAxis(value_type, value_type, value_type, value_type) axis-rotation matrix}.
     * @see <a href="http://web.archive.org/web/20041029003853/http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q38">Matrix-FAQ Q38</a>
     * @param angrad angle in radians
     * @param x x of rotation axis
     * @param y y of rotation axis
     * @param z z of rotation axis
     * @param tmp temporary mat4f used for multiplication
     * @return this matrix for chaining
     */
    Matrix4& rotate(const value_type ang_rad, const value_type x, const value_type y, const value_type z, Matrix4& tmp) noexcept {
        return mul( tmp.setToRotationAxis(ang_rad, x, y, z) );
    }

    /**
     * Rotate this matrix about give axis and angle in radians, i.e. multiply by {@link #setToRotationAxis(value_type, vec3f) axis-rotation matrix}.
     * @see <a href="http://web.archive.org/web/20041029003853/http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q38">Matrix-FAQ Q38</a>
     * @param angrad angle in radians
     * @param axis rotation axis
     * @param tmp temporary mat4f used for multiplication
     * @return this matrix for chaining
     */
    Matrix4& rotate(const value_type ang_rad, const Vec3f& axis, Matrix4& tmp) noexcept {
        return mul( tmp.setToRotationAxis(ang_rad, axis) );
    }

    /**
     * Translate this matrix, i.e. multiply by {@link #setToTranslation(value_type, value_type, value_type) translation matrix}.
     * @param x x translation
     * @param y y translation
     * @param z z translation
     * @param tmp temporary mat4f used for multiplication
     * @return this matrix for chaining
     */
    Matrix4& translate(const value_type x, const value_type y, const value_type z, Matrix4& tmp) noexcept {
        return mul( tmp.setToTranslation(x, y, z) );
    }

#if 0
    /**
     * Translate this matrix, i.e. multiply by {@link #setToTranslation(vec3f) translation matrix}.
     * @param t translation vec3f
     * @param tmp temporary mat4f used for multiplication
     * @return this matrix for chaining
     */
    public final Matrix4 translate(final Vec3f t, final Matrix4 tmp) {
        return mul( tmp.setToTranslation(t) );
    }

    /**
     * Scale this matrix, i.e. multiply by {@link #setToScale(value_type, value_type, value_type) scale matrix}.
     * @param x x scale
     * @param y y scale
     * @param z z scale
     * @param tmp temporary mat4f used for multiplication
     * @return this matrix for chaining
     */
    public final Matrix4 scale(const value_type x, const value_type y, const value_type z, final Matrix4 tmp) {
        return mul( tmp.setToScale(x, y, z) );
    }

    /**
     * Scale this matrix, i.e. multiply by {@link #setToScale(value_type, value_type, value_type) scale matrix}.
     * @param s scale for x-, y- and z-axis
     * @param tmp temporary mat4f used for multiplication
     * @return this matrix for chaining
     */
    public final Matrix4 scale(const value_type s, final Matrix4 tmp) {
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
     * Equals check using a given {@link FloatUtil#EPSILON} value and {@link FloatUtil#isEqual(value_type, value_type, value_type)}.
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
    public boolean isEqual(final Matrix4 o, const value_type epsilon) {
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
     * Equals check using {@link FloatUtil#EPSILON} value and {@link FloatUtil#isEqual(value_type, value_type, value_type)}.
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
    public boolean isEqual(final Matrix4 o) {
        return isEqual(o, FloatUtil.EPSILON);
    }

    @Override
    public boolean equals(final Object o) {
        if( o instanceof Matrix4 ) {
            return isEqual((Matrix4)o, FloatUtil.EPSILON);
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
    public static boolean mapObjToWin(final Vec3f obj, final Matrix4 mMv, final Matrix4 mP,
                                      final Recti viewport, final Vec3f winPos)
    {
        final Vec4 vec4Tmp1 = new Vec4(obj, 1f);

        // vec4Tmp2 = Mv * o
        // rawWinPos = P  * vec4Tmp2
        // rawWinPos = P * ( Mv * o )
        // rawWinPos = P * Mv * o
        final Vec4 vec4Tmp2 = mMv.mulVec4(vec4Tmp1, new Vec4());
        final Vec4 rawWinPos = mP.mulVec4(vec4Tmp2, vec4Tmp1);

        if (rawWinPos.w() == zero) {
            return false;
        }

        const value_type s = ( one / rawWinPos.w() ) * half;

        // Map x, y and z to range 0-1 (w is ignored)
        rawWinPos.scale(s).add(half, half, half, 0f);

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
    public static boolean mapObjToWin(final Vec3f obj, final Matrix4 mPMv,
                                      final Recti viewport, final Vec3f winPos)
    {
        final Vec4 vec4Tmp2 = new Vec4(obj, 1f);

        // rawWinPos = P * Mv * o
        final Vec4 rawWinPos = mPMv.mulVec4(vec4Tmp2, new Vec4());

        if (rawWinPos.w() == zero) {
            return false;
        }

        const value_type s = ( one / rawWinPos.w() ) * half;

        // Map x, y and z to range 0-1 (w is ignored)
        rawWinPos.scale(s).add(half, half, half, 0f);

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
    public static boolean mapWinToObj(const value_type winx, const value_type winy, const value_type winz,
                                      final Matrix4 mMv, final Matrix4 mP,
                                      final Recti viewport,
                                      final Vec3f objPos,
                                      final Matrix4 mat4Tmp)
    {
        // invPMv = Inv(P x Mv)
        final Matrix4 invPMv = mat4Tmp.mul(mP, mMv);
        if( !invPMv.invert() ) {
            return false;
        }

        final Vec4 winPos = new Vec4(winx, winy, winz, 1f);

        // Map x and y from window coordinates
        winPos.add(-viewport.x(), -viewport.y(), 0f, 0f).mul(1f/viewport.width(), 1f/viewport.height(), 1f, 1f);

        // Map to range -1 to 1
        winPos.mul(2f, 2f, 2f, 1f).add(-1f, -1f, -1f, 0f);

        // rawObjPos = Inv(P x Mv) *  winPos
        final Vec4 rawObjPos = invPMv.mulVec4(winPos, new Vec4());

        if ( rawObjPos.w() == zero ) {
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
    public static boolean mapWinToObj(const value_type winx, const value_type winy, const value_type winz,
                                      final Matrix4 invPMv,
                                      final Recti viewport,
                                      final Vec3f objPos)
    {
        if( null == invPMv ) {
            return false;
        }
        final Vec4 winPos = new Vec4(winx, winy, winz, 1f);

        // Map x and y from window coordinates
        winPos.add(-viewport.x(), -viewport.y(), 0f, 0f).mul(1f/viewport.width(), 1f/viewport.height(), 1f, 1f);

        // Map to range -1 to 1
        winPos.mul(2f, 2f, 2f, 1f).add(-1f, -1f, -1f, 0f);

        // rawObjPos = Inv(P x Mv) *  winPos
        final Vec4 rawObjPos = invPMv.mulVec4(winPos, new Vec4());

        if ( rawObjPos.w() == zero ) {
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
    public static boolean mapWinToObj(const value_type winx, const value_type winy, const value_type winz1, const value_type winz2,
                                      final Matrix4 invPMv,
                                      final Recti viewport,
                                      final Vec3f objPos1, final Vec3f objPos2)
    {
        if( null == invPMv ) {
            return false;
        }
        final Vec4 winPos = new Vec4(winx, winy, winz1, 1f);

        // Map x and y from window coordinates
        winPos.add(-viewport.x(), -viewport.y(), 0f, 0f).mul(1f/viewport.width(), 1f/viewport.height(), 1f, 1f);

        // Map to range -1 to 1
        winPos.mul(2f, 2f, 2f, 1f).add(-1f, -1f, -1f, 0f);

        // rawObjPos = Inv(P x Mv) *  winPos1
        final Vec4 rawObjPos = invPMv.mulVec4(winPos, new Vec4());

        if ( rawObjPos.w() == zero ) {
            return false;
        }
        objPos1.set( rawObjPos.scale( 1f / rawObjPos.w() ) );

        //
        // winz2
        //
        // Map Z to range -1 to 1
        winPos.setZ( winz2 * 2f - 1f );

        // rawObjPos = Inv(P x Mv) *  winPos2
        invPMv.mulVec4(winPos, rawObjPos);

        if ( rawObjPos.w() == zero ) {
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
    public static boolean mapWinToObj4(const value_type winx, const value_type winy, const value_type winz, const value_type clipw,
                                       final Matrix4 mMv, final Matrix4 mP,
                                       final Recti viewport,
                                       const value_type near, const value_type far,
                                       final Vec4 objPos,
                                       final Matrix4 mat4Tmp)
    {
        // invPMv = Inv(P x Mv)
        final Matrix4 invPMv = mat4Tmp.mul(mP, mMv);
        if( !invPMv.invert() ) {
            return false;
        }

        final Vec4 winPos = new Vec4(winx, winy, winz, clipw);

        // Map x and y from window coordinates
        winPos.add(-viewport.x(), -viewport.y(), -near, 0f).mul(1f/viewport.width(), 1f/viewport.height(), 1f/(far-near), 1f);

        // Map to range -1 to 1
        winPos.mul(2f, 2f, 2f, 1f).add(-1f, -1f, -1f, 0f);

        // objPos = Inv(P x Mv) *  winPos
        invPMv.mulVec4(winPos, objPos);

        if ( objPos.w() == zero ) {
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
    public static boolean mapWinToObj4(const value_type winx, const value_type winy, const value_type winz, const value_type clipw,
                                       final Matrix4 invPMv,
                                       final Recti viewport,
                                       const value_type near, const value_type far,
                                       final Vec4 objPos)
    {
        if( null == invPMv ) {
            return false;
        }
        final Vec4 winPos = new Vec4(winx, winy, winz, clipw);

        // Map x and y from window coordinates
        winPos.add(-viewport.x(), -viewport.y(), -near, 0f).mul(1f/viewport.width(), 1f/viewport.height(), 1f/(far-near), 1f);

        // Map to range -1 to 1
        winPos.mul(2f, 2f, 2f, 1f).add(-1f, -1f, -1f, 0f);

        // objPos = Inv(P x Mv) *  winPos
        invPMv.mulVec4(winPos, objPos);

        if ( objPos.w() == zero ) {
            return false;
        }
        return true;
    }

    /**
     * Map two window coordinates w/ shared X/Y and distinctive Z
     * to a {@link Ray}. The resulting {@link Ray} maybe used for <i>picking</i>
     * using a {@link AABBox#getRayIntersection(vec3f, Ray, value_type, boolean)}.
     * <p>
     * Notes for picking <i>winz0</i> and <i>winz1</i>:
     * <ul>
     *   <li>see {@link FloatUtil#getZBufferEpsilon(int, value_type, value_type)}</li>
     *   <li>see {@link FloatUtil#getZBufferValue(int, value_type, value_type, value_type)}</li>
     *   <li>see {@link FloatUtil#getOrthoWinZ(value_type, value_type, value_type)}</li>
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
    public static boolean mapWinToRay(const value_type winx, const value_type winy, const value_type winz0, const value_type winz1,
                                      final Matrix4 mMv, final Matrix4 mP,
                                      final Recti viewport,
                                      final Ray ray,
                                      final Matrix4 mat4Tmp1, final Matrix4 mat4Tmp2) {
        // invPMv = Inv(P x Mv)
        final Matrix4 invPMv = mat4Tmp1.mul(mP, mMv);
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
     * using a {@link AABBox#getRayIntersection(vec3f, Ray, value_type, boolean)}.
     * <p>
     * Notes for picking <i>winz0</i> and <i>winz1</i>:
     * <ul>
     *   <li>see {@link FloatUtil#getZBufferEpsilon(int, value_type, value_type)}</li>
     *   <li>see {@link FloatUtil#getZBufferValue(int, value_type, value_type, value_type)}</li>
     *   <li>see {@link FloatUtil#getOrthoWinZ(value_type, value_type, value_type)}</li>
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
    public static boolean mapWinToRay(const value_type winx, const value_type winy, const value_type winz0, const value_type winz1,
                                      final Matrix4 invPMv,
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
     * @param f the format string of one value_typeing point, i.e. "%10.5f", see {@link java.util.Formatter}
     * @return matrix string representation
     */
    public StringBuilder toString(final StringBuilder sb, final String rowPrefix, final String f) {
        const value_type[] tmp = new value_type[16];
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
     * @param f format string for each value_type element, e.g. "%10.5f"
     * @return matrix
     */
    std::string toString(const std::string& rowPrefix, const std::string& f) const noexcept {
        std::string sb;
        value_type tmp[16];
        get(tmp);
        return jau::mat_to_string(sb, rowPrefix, f, tmp, 4, 4, false /* rowMajorOrder */); // creates a copy-out!
    }

    std::string toString() const noexcept { return toString("", "%10.5f"); }
};

template<typename T,
         std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
constexpr Matrix4<T> operator*(const Matrix4<T>& lhs, const Matrix4<T>& rhs ) noexcept {
    Matrix4<T> r(lhs); r *= rhs; return r;
}

template<typename T,
         std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
constexpr Matrix4<T> operator*(const Matrix4<T>& lhs, const T s ) noexcept {
    Matrix4<T> r(lhs); r *= s; return r;
}

template<typename T,
         std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
constexpr Matrix4<T> operator*(const T s, const Matrix4<T>& rhs) noexcept {
    Matrix4<T> r(rhs); r *= s; return r;
}

template<typename T,
         std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
std::ostream& operator<<(std::ostream& out, const Matrix4<T>& v) noexcept {
    return out << v.toString();
}

typedef Matrix4<float> Mat4f;

static_assert(alignof(float) == alignof(Mat4f));

/**@}*/

} // namespace jau::math

#endif // JAU_MAT4f_HPP_
