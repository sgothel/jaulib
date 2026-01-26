/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2024 Gothel Software e.K.
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
#include <cassert>
#include <cstring>

#include <jau/test/catch2_ext.hpp>

#include <jau/math/mat4f.hpp>
#include <jau/math/util/sstack.hpp>

using namespace jau;
using namespace jau::math;

static const float mI_0[] = {    1,    0,    0,    0,
                                 0,    1,    0,    0,
                                 0,    0,    1,    0,
                                 0,    0,    0,    1 };
static const Mat4f mI(mI_0);

static const float m1_0[] = {    1,    3,    4,    0,
                                 6,    7,    8,    5,
                                98,    7,    6,    9,
                                54,    3,    2,    5 };
static const Mat4f m1(m1_0);

static const float m1T_0[] = {   1,    6,   98,   54,
                                 3,    7,    7,    3,
                                 4,    8,    6,    2,
                                 0,    5,    9,    5 };
static const Mat4f m1T(m1T_0);

static const float m2_0[] = {    1,    6,   98,   54,
                                 3,    7,    7,    3,
                                 4,    8,    6,    2,
                                 0,    5,    9,    5 };
static const Mat4f m2(m2_0);

static const float m2xm1_0[] = {   26,   59,  143,   71,
                                   59,  174,  730,  386,
                                  143,  730, 9770, 5370,
                                   71,  386, 5370, 2954 };
static const Mat4f m2xm1(m2xm1_0);

static const float m1xm2_0[] = {12557,  893,  748, 1182,
                                  893,  116,  116,  113,
                                  748,  116,  120,  104,
                                 1182,  113,  104,  131 };
static const Mat4f m1xm2(m1xm2_0);

TEST_CASE( "Test 00 Load Get", "[mat4f][linear_algebra][math]" ) {
    {
        Mat4f m;
        REQUIRE(mI == m);
    }
    {
        float f16[16];
        m1.get(f16);
        COMPARE_NARRAYS_EPS(m1_0, f16, 16, EPSILON<float>);

        Mat4f m;
        m.load(f16);
        REQUIRE(m1 == m);
    }
}

TEST_CASE( "Test 01 Mul", "[mat4f][linear_algebra][math]" ) {
    {
        REQUIRE(m1xm2 == m1 * m2);
        Mat4f m; m.mul(m1, m2);
        REQUIRE(m1xm2 == m);
    }
    {
        REQUIRE(m2xm1 == m2 * m1);
        Mat4f m; m.mul(m2, m1);
        REQUIRE(m2xm1 == m);
    }
}

TEST_CASE( "Test 02 Transpose", "[mat4f][linear_algebra][math]" ) {
    REQUIRE(m1T == Mat4f(m1).transpose());
    REQUIRE(m1T == Mat4f().transpose(m1));
}

TEST_CASE( "Test 10 LookAtNegZ", "[mat4f][linear_algebra][math]" ) {
    Mat4f m;
    // Look towards -z
    m.setToLookAt(
            Vec3f(0, 0,  0),  // eye
            Vec3f(0, 0, -1),  // center
            Vec3f(0, 1,  0)); // up

    /**
     * The 3 rows of the matrix (= the 3 columns of the array/buffer) should be: side, up, -forward.
     */
    Mat4f exp( { 1, 0, 0, 0,
                 0, 1, 0, 0,
                 0, 0, 1, 0,
                 0, 0, 0, 1  } );

    REQUIRE(exp == m);
}

TEST_CASE( "Test 11 LookAtPosY", "[mat4f][linear_algebra][math]" ) {
    Mat4f m;
    // Look towards -z
    m.setToLookAt(
            Vec3f(0, 0, 0),  // eye
            Vec3f(0, 1, 0),  // center
            Vec3f(0, 0, 1)); // up

    /**
     * The 3 rows of the matrix (= the 3 columns of the array/buffer) should be: side, up, -forward.
     */
    Mat4f exp( { 1, 0,  0, 0,
                 0, 0, -1, 0,
                 0, 1,  0, 0,
                 0, 0,  0, 1
            } );

    REQUIRE(exp == m);
}

TEST_CASE( "Test 20 Float16Stack", "[stack][mat4f][math]" ) {
    jau::math::util::Stack16f s1;
    Mat4f m10( {  1.0f,  2.0f,  3.0f,  4.0f,  // column 0
                  5.0f,  6.0f,  7.0f,  8.0f,  // column 1
                  9.0f, 10.0f, 11.0f, 12.0f,  // column 2
                 13.0f, 14.0f, 15.0f, 16.0f  // column 3
              } );
    Mat4f m20 = m10 * 2.0f;
    std::cout << "mat4 m10 " << m10 << std::endl;
    std::cout << "mat4 m20 " << m20 << std::endl;
    s1.push(m10.cbegin());
    s1.push(m20.cbegin());
    Mat4f m22, m12;
    s1.pop(m22.begin());
    s1.pop(m12.begin());
    REQUIRE( m22 == m20 );
    REQUIRE( m12 == m10 );
}

TEST_CASE( "Test 21 Mat4fStack", "[stack][mat4f][math]" ) {
    jau::math::util::Mat4fStack s1;
    Mat4f m10( {  1.0f,  2.0f,  3.0f,  4.0f,  // column 0
                  5.0f,  6.0f,  7.0f,  8.0f,  // column 1
                  9.0f, 10.0f, 11.0f, 12.0f,  // column 2
                 13.0f, 14.0f, 15.0f, 16.0f  // column 3
              } );
    Mat4f m20 = m10 * 2.0f;
    std::cout << "mat4 m10 " << m10 << std::endl;
    std::cout << "mat4 m20 " << m20 << std::endl;
    s1.push(m10);
    s1.push(m20);
    Mat4f m22, m12;
    s1.pop(m22);
    s1.pop(m12);
    REQUIRE( m22 == m20 );
    REQUIRE( m12 == m10 );
}
