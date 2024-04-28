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
#include <cassert>
#include <cinttypes>
#include <cstring>

#include <jau/test/catch2_ext.hpp>

#include <jau/math/util/pmvmat4f.hpp>

using namespace jau;
using namespace jau::math;
using namespace jau::math::util;

static const float NaN =  std::numeric_limits<float>::quiet_NaN();

TEST_CASE( "Test 01 Unproject NaN", "[unproject][mat4f][linear_algebra][math]" ) {
    const Mat4f mMv({1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1});
    const Mat4f mP({1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1});
    const Recti viewport(0,0,800,600);
    const float pick[] = { 400, 300, 0 };

    Vec3f objPos(NaN, NaN, NaN);
    Mat4f tmp;

    // gluUnProject
    bool res = Mat4f::mapWinToObj(pick[0], pick[1], pick[2],
                                  mMv, mP, viewport,
                                  objPos, tmp);

    REQUIRE( false == std::isnan(objPos.x) );
    REQUIRE( false == std::isnan(objPos.y) );
    REQUIRE( false == std::isnan(objPos.z) );

    REQUIRE( true == res );
}

TEST_CASE( "Test 10 Unproject Pick 1", "[unproject][mat4f][linear_algebra][math]" ) {
    const Mat4f mMv({ 1, 0, 0, 0,
                      0, 1, 0, 0,
                      0, 0, 1, 0,
                      0, 0, 0, 1 });
    const Mat4f mP({ 2.3464675f, 0,          0,        0,
                     0,          2.4142134f, 0,        0,
                     0,          0,         -1.0002f, -1,
                     0,          0,        -20.002f,   0 });
    const Recti viewport(0, 0, 1000, 1000);
    const float pick[] = { 250, 250, 0.5f };

    const Vec3f expected(-4.2612f, -4.1417f, -19.9980f );
    Vec3f result;
    Mat4f tmp;

    // gluUnProject
    bool res = Mat4f::mapWinToObj(pick[0], pick[1], pick[2],
                                  mMv, mP, viewport,
                                  result, tmp);

    REQUIRE( true == res );
    COMPARE_NARRAYS_EPS(expected.cbegin(), result.cbegin(), 3, 0.0001f);
}

TEST_CASE( "Test 11 Unproject Pick 2", "[unproject][mat4f][linear_algebra][math]" ) {
    const Mat4f mMv({ 1, 0,    0, 0,
                      0, 1,    0, 0,
                      0, 0,    1, 0,
                      0, 0, -200, 1 });
    const Mat4f mP({ 2.3464675f, 0,          0,        0,
                     0,          2.4142134f, 0,        0,
                     0,          0,         -1.0002f, -1,
                     0,          0,        -20.002f,   0 });
    const Recti viewport(0, 0, 1000, 1000);
    const float pick[] = { 250, 250, 0.5f };

    const Vec3f expected(-4.2612f, -4.1417f, 180.002f );
    Vec3f result;
    Mat4f tmp;

    // gluUnProject
    bool res = Mat4f::mapWinToObj(pick[0], pick[1], pick[2],
                                  mMv, mP, viewport,
                                  result, tmp);

    REQUIRE( true == res );
    COMPARE_NARRAYS_EPS(expected.cbegin(), result.cbegin(), 3, 0.0001f);
}

