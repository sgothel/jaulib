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
#include <thread>
#include <cinttypes>
#include <cstring>

#include <jau/test/catch2_ext.hpp>

#include <jau/int_math.hpp>
#include <jau/float_math.hpp>
#include <jau/math/vec2f.hpp>
#include <jau/math/vec2i.hpp>
#include <jau/math/vec3f.hpp>
#include <jau/math/vec4f.hpp>
#include <jau/math/mat4f.hpp>
#include <jau/math/quaternion.hpp>
#include <jau/math/aabbox2f.hpp>
#include <jau/math/aabbox3f.hpp>
#include <jau/math/mat4f.hpp>
#include <jau/math/recti.hpp>
#include <jau/math/math_error.hpp>

using namespace jau;
using namespace jau::math;

static const float EPSILON = std::numeric_limits<float>::epsilon();

static float* makeIdentity(float m[]) {
  m[0+4*0] = 1;
  m[1+4*0] = 0;
  m[2+4*0] = 0;
  m[3+4*0] = 0;

  m[0+4*1] = 0;
  m[1+4*1] = 1;
  m[2+4*1] = 0;
  m[3+4*1] = 0;

  m[0+4*2] = 0;
  m[1+4*2] = 0;
  m[2+4*2] = 1;
  m[3+4*2] = 0;

  m[0+4*3] = 0;
  m[1+4*3] = 0;
  m[2+4*3] = 0;
  m[3+4*3] = 1;
  return m;
}

static float* invertMatrix(float msrc[], float mres[], float temp[/*4*4*/]) {
    int i, j, k, swap;
    float t;
    for (i = 0; i < 4; i++) {
        const int i4 = i*4;
        for (j = 0; j < 4; j++) {
            temp[i4+j] = msrc[i4+j];
        }
    }
    makeIdentity(mres);

    for (i = 0; i < 4; i++) {
        const int i4 = i*4;

        //
        // Look for largest element in column
        //
        swap = i;
        for (j = i + 1; j < 4; j++) {
            if (std::abs(temp[j*4+i]) > std::abs(temp[i4+i])) {
                swap = j;
            }
        }

        if (swap != i) {
            const int swap4 = swap*4;
            //
            // Swap rows.
            //
            for (k = 0; k < 4; k++) {
                t = temp[i4+k];
                temp[i4+k] = temp[swap4+k];
                temp[swap4+k] = t;

                t = mres[i4+k];
                mres[i4+k] = mres[swap4+k];
                mres[swap4+k] = t;
            }
        }

        if (temp[i4+i] == 0) {
            //
            // No non-zero pivot. The matrix is singular, which shouldn't
            // happen. This means the user gave us a bad matrix.
            //
            return nullptr;
        }

        t = temp[i4+i];
        for (k = 0; k < 4; k++) {
            temp[i4+k] /= t;
            mres[i4+k] /= t;
        }
        for (j = 0; j < 4; j++) {
            if (j != i) {
                const int j4 = j*4;
                t = temp[j4+i];
                for (k = 0; k < 4; k++) {
                    temp[j4+k] -= temp[i4+k] * t;
                    mres[j4+k] -= mres[i4+k]*t;
                }
            }
        }
    }
    return mres;
}

static void testImpl(float matrix[]) {
    float inv1_0[16];
    float inv2_0[16];
    float temp[16];

    // System.err.println(FloatUtil.matrixToString(null, "orig  : ", "%10.7f", matrix, 0, 4, 4, false /* rowMajorOrder */));
    invertMatrix(matrix, inv1_0, temp);
    invertMatrix(inv1_0, inv2_0, temp);
    // System.err.println(FloatUtil.matrixToString(null, "inv1_0: ", "%10.7f", inv1_0, 0, 4, 4, false /* rowMajorOrder */));
    // System.err.println(FloatUtil.matrixToString(null, "inv2_0: ", "%10.7f", inv2_0, 0, 4, 4, false /* rowMajorOrder */));

    COMPARE_NARRAYS_EPS(matrix, inv2_0, 16, EPSILON);

    //
    // Mat4f
    //

    Mat4f matrix_m(matrix);
    Mat4f inv1_4a(matrix_m);
    REQUIRE( true == inv1_4a.invert() );
    Mat4f inv2_4a(inv1_4a);
    REQUIRE( true == inv2_4a.invert() );

    COMPARE_NARRAYS_EPS(inv1_0, inv1_4a.get(temp), 16, Mat4f::inv_deviation);
    COMPARE_NARRAYS_EPS(inv2_0, inv2_4a.get(temp), 16, Mat4f::inv_deviation);
    REQUIRE_MSG( "I4 failure: "+matrix_m.toString()+" != "+inv2_4a.toString(), matrix_m.equals(inv2_4a, Mat4f::inv_deviation));

    Mat4f inv1_4b;
    REQUIRE( true == inv1_4b.invert(matrix_m) );
    Mat4f inv2_4b;
    REQUIRE( true == inv2_4b.invert(inv1_4b) );

    // Assert.assertEquals(new Mat4f(inv1_2), inv1_4b);
    // Assert.assertEquals(new Mat4f(inv2_2), inv2_4b);
    COMPARE_NARRAYS_EPS(inv1_0, inv1_4b.get(temp), 16, Mat4f::inv_deviation);
    COMPARE_NARRAYS_EPS(inv2_0, inv2_4b.get(temp), 16, Mat4f::inv_deviation);
    REQUIRE_MSG( "I4 failure: "+matrix_m.toString()+" != "+inv2_4b.toString(), matrix_m.equals(inv2_4b, Mat4f::inv_deviation));
}

TEST_CASE( "Test 02", "[mat4f][linear_algebra][math]" ) {
    float p[] = { 2.3464675f, 0,          0,        0,
                  0,          2.4142134f, 0,        0,
                  0,          0,         -1.0002f, -1,
                  0,          0,        -20.002f,   0 };
    testImpl(p);
}

TEST_CASE( "Test 03", "[mat4f][linear_algebra][math]" ) {
    float mv[] = { 1, 0,    0, 0,
                   0, 1,    0, 0,
                   0, 0,    1, 0,
                   0, 0, -200, 1 } ;
    testImpl(mv);
}

TEST_CASE( "Test 04", "[mat4f][linear_algebra][math]" ) {
    float p[] = { 2.3464675f, 0,          0,        0,
                  0,          2.4142134f, 0,        0,
                  0,          0,         -1.0002f, -1,
                  0,          0,        -20.002f,   0 };

    testImpl(p);
}

TEST_CASE( "Test 05 Perf01", "[mat4f][linear_algebra][math]" ) {
    float p1[] = { 2.3464675f, 0,          0,        0,
                   0,          2.4142134f, 0,        0,
                   0,          0,         -1.0002f, -1,
                   0,          0,        -20.002f,   0 };
    Mat4f p1_m(p1);

    float p2[] = { 26,   59,  143,   71,
                   59,  174,  730,  386,
                  143,  730, 9770, 5370,
                   71,  386, 5370, 2954 };
    Mat4f p2_m(p2);

    Mat4f res_m;

    const size_t warmups = 1000_u64;
    const size_t loops = 10_u64*1000000_u64;
    jau::fraction_i64 tI4a = fractions_i64::zero;
    jau::fraction_i64 tI4b = fractions_i64::zero;

    // avoid optimizing out unused computation results by simply adding up determinat
    double dr = 1;

    //
    // Mat4f
    //

    // warm-up
    for(size_t i=0; i<warmups; i++) {
        res_m.invert(p1_m);
        dr += res_m.determinant();
        res_m.invert(p2_m);
        dr += res_m.determinant();
    }
    jau::fraction_timespec t_0 = jau::getMonotonicTime();
    for(size_t i=0; i<loops; i++) {
        res_m.invert(p1_m);
        dr += res_m.determinant();
        res_m.invert(p2_m);
        dr += res_m.determinant();
    }
    tI4a = (getMonotonicTime() - t_0).to_fraction_i64();
    REQUIRE( false == jau::is_zero(dr) );

    // warm-up
    for(size_t i=0; i<warmups; i++) {
        res_m.load(p1_m).invert();
        dr += res_m.determinant();
        res_m.load(p2_m).invert();
        dr += res_m.determinant();
    }

    t_0 = jau::getMonotonicTime();
    for(size_t i=0; i<loops; i++) {
        res_m.load(p1_m).invert();
        dr += res_m.determinant();
        res_m.load(p2_m).invert();
        dr += res_m.determinant();
    }
    tI4b = (getMonotonicTime() - t_0).to_fraction_i64();
    REQUIRE( false == jau::is_zero(dr) );

    printf("Checkmark %f\n", dr);
    printf("Summary loops %6zu: I4a %6s ms total (%s us), %f ns/inv, I4a / I4b %f%%\n", loops,
            jau::to_decstring(tI4a.to_ms()).c_str(), jau::to_decstring(tI4a.to_us()).c_str(),
            (double)tI4a.to_ns()/2.0/(double)loops, tI4a.to_double()/tI4b.to_double()*100.0);
    printf("Summary loops %6zu: I4b %6s ms total (%s us), %f ns/inv, I4b / I4a %f%%\n", loops,
            jau::to_decstring(tI4b.to_ms()).c_str(), jau::to_decstring(tI4b.to_us()).c_str(),
            (double)tI4b.to_ns()/2.0/(double)loops, tI4b.to_double()/tI4a.to_double()*100.0);
}


