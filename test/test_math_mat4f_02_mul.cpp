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

#include <jau/math/mat4f.hpp>

using namespace jau;
using namespace jau::math;

static const float m1_0[] = {    1,    3,    4,    0,
                                 6,    7,    8,    5,
                                98,    7,    6,    9,
                                54,    3,    2,    5 };
static const Mat4f m1(m1_0);

static const float m2_0[] = {    1,    6,   98,   54,
                                 3,    7,    7,    3,
                                 4,    8,    6,    2,
                                 0,    5,    9,    5 };
static const Mat4f m2(m2_0);

TEST_CASE( "Test 05 Perf01", "[mat4f][linear_algebra][math]" ) {
    Mat4f res_m;

    const size_t warmups = is_debug_enabled() ? 100_u64 : 1000_u64;
    const size_t loops = is_debug_enabled() ? 1_u64*1000000_u64 : 300_u64*1000000_u64;
    jau::fraction_i64 tI4a = fractions_i64::zero;
    jau::fraction_i64 tI4b = fractions_i64::zero;

    const uint64_t tI5Max = 1000; // 1s
    size_t loops5a = 0;
    jau::fraction_i64 tI5a = fractions_i64::zero;
    size_t loops5b = 0;
    jau::fraction_i64 tI5b = fractions_i64::zero;

    // avoid optimizing out unused computation results by simply adding up determinat
    double dr = 1;

    //
    // Mat4f
    //

    // warm-up
    for(size_t i=0; i<warmups; i++) {
        res_m = m1 * m2;
        dr += res_m.determinant();
        res_m = m2 * m1;
        dr += res_m.determinant();
    }

    jau::fraction_timespec t_0 = jau::getMonotonicTime();
    for(size_t i=0; i<loops; i++) {
        res_m = m1 * m2;
        dr += res_m.determinant();
        res_m = m2 * m1;
        dr += res_m.determinant();
    }
    tI4a = (getMonotonicTime() - t_0).to_fraction_i64();
    REQUIRE( dr > 0 );

    // warm-up
    for(size_t i=0; i<warmups; i++) {
        res_m.load(m1);
        res_m.mul(m2);
        dr += res_m.determinant();
        res_m.load(m2);
        res_m.mul(m1);
        dr += res_m.determinant();
    }

    t_0 = jau::getMonotonicTime();
    for(size_t i=0; i<loops; i++) {
        res_m.load(m1);
        res_m.mul(m2);
        dr += res_m.determinant();
        res_m.load(m2);
        res_m.mul(m1);
        dr += res_m.determinant();
    }
    tI4b = (getMonotonicTime() - t_0).to_fraction_i64();
    REQUIRE( dr > 0 );

    if( catch_perf_analysis ) {
        tI5a = fractions_i64::zero;
        t_0 = jau::getMonotonicTime();
        uint64_t t_5 = jau::getCurrentMilliseconds();
        uint64_t td_5=0;
        while( td_5 < tI5Max ) {
            res_m = m1 * m2;
            dr += res_m.determinant();
            res_m = m2 * m1;
            dr += res_m.determinant();
            ++loops5a;
            // if( 0 == loops5a % 1000000 ) {
                td_5 = jau::getCurrentMilliseconds() - t_5;
            // }
        }
        tI5a = (getMonotonicTime() - t_0).to_fraction_i64();
        REQUIRE( dr > 0 );

        tI5b = fractions_i64::zero;
        t_0 = jau::getMonotonicTime();
        t_5 = jau::getCurrentMilliseconds();
        td_5=0;
        while( td_5 < tI5Max ) {
            res_m.load(m1);
            res_m.mul(m2);
            dr += res_m.determinant();
            res_m.load(m2);
            res_m.mul(m1);
            dr += res_m.determinant();
            ++loops5b;
            // if( 0 == loops5b % 1000000 ) {
                td_5 = jau::getCurrentMilliseconds() - t_5;
            // }
        }
        tI5b = (getMonotonicTime() - t_0).to_fraction_i64();
        REQUIRE( dr > 0 );
    }

    printf("Checkmark %f\n", dr);
    printf("Summary loops %6zu: I4a %6s ms total (%s us), %f ns/mul, I4a / I4b %f%%\n", loops,
            jau::to_decstring(tI4a.to_ms()).c_str(), jau::to_decstring(tI4a.to_us()).c_str(),
            (double)tI4a.to_ns()/2.0/(double)loops, tI4a.to_double()/tI4b.to_double()*100.0);
    printf("Summary loops %6zu: I4b %6s ms total (%s us), %f ns/mul, I4b / I4a %f%%\n", loops,
            jau::to_decstring(tI4b.to_ms()).c_str(), jau::to_decstring(tI4b.to_us()).c_str(),
            (double)tI4b.to_ns()/2.0/(double)loops, tI4b.to_double()/tI4a.to_double()*100.0);

    if( catch_perf_analysis ) {
        printf("Summary loops %6zu: I5a %6s ms total, %f ns/mul, I5a / I5b %f%%\n", loops5a,
                jau::to_decstring(tI5a.to_ms()).c_str(),
                (double)tI5a.to_ns()/2.0/(double)loops5a, tI5a.to_double()/tI5b.to_double()*100.0);
        printf("Summary loops %6zu: I5b %6s ms total, %f ns/mul, I5b / I5a %f%%\n", loops5b,
                jau::to_decstring(tI5b.to_ms()).c_str(),
                (double)tI5b.to_ns()/2.0/(double)loops5b, tI5b.to_double()/tI5a.to_double()*100.0);
    }
}
