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

// static const float PI = (float)M_PI;
static const float HALF_PI = (float)M_PI_2;

static const Quaternion QUAT_IDENT = Quaternion(0, 0, 0, 1);

static const Vec3f ZERO       = Vec3f (  0,  0,  0 );
static const Vec3f ONE        = Vec3f (  1,  1,  1 );
static const Vec3f ONE_NEG    = Vec3f ( -1, -1, -1 );
static const Vec3f UNIT_X     = Vec3f (  1,  0,  0 );
static const Vec3f UNIT_Y     = Vec3f (  0,  1,  0 );
static const Vec3f UNIT_Z     = Vec3f (  0,  0,  1 );
static const Vec3f UNIT_X_NEG = Vec3f ( -1,  0,  0 );
static const Vec3f UNIT_Y_NEG = Vec3f (  0, -1,  0 );
static const Vec3f UNIT_Z_NEG = Vec3f (  0,  0, -1 );

// static const Vec4f NEG_ONE_v4 = Vec4f ( -1, -1, -1, 0 );
// static const Vec4f ONE_v4     = Vec4f (  1,  1,  1, 0 );

//
// Basic
//

TEST_CASE( "Test 01 Normalize", "[quaternion][linear_algebra][math]" ) {
    const Quaternion quat(0, 1, 2, 3);
    const Quaternion quat2 = Quaternion(quat).normalize();
    // Assert.assertTrue(Math.abs(1 - quat2.magnitude()) <= MACH_EPSILON);
    REQUIRE( true == jau::equals( 0.0f, std::abs( 1 - quat2.magnitude() ) ) ) ;
}

TEST_CASE( "Test 02 Rotate Zero Vector", "[quaternion][linear_algebra][math]" ) {
    Quaternion quat;
    const Vec3f rotVec0 = quat.rotateVector(ZERO);
    REQUIRE( ZERO == rotVec0 );
}

TEST_CASE( "Test 03 Invert and Conugate", "[quaternion][linear_algebra][math]" ) {
    // inversion check
    {
        const Quaternion quat0(0, 1, 2, 3);
        Quaternion quat0Inv = Quaternion(quat0).invert();
        REQUIRE( quat0 == quat0Inv.invert() );
    }
    // conjugate check
    {
        const Quaternion quat0(-1, -2, -3, 4);
        const Quaternion quat0Conj = Quaternion( 1,  2,  3, 4).conjugate();
        REQUIRE( quat0 == quat0Conj );
    }
}

TEST_CASE( "Test 04 Dot", "[quaternion][linear_algebra][math]" ) {
    const Quaternion quat(7, 2, 5, -1);
    REQUIRE( 35.0f == quat.dot(3, 1, 2, -2));
    REQUIRE( -11.0f == quat.dot(Quaternion(-1, 1, -1, 1)));
}

//
// Conversion
//

TEST_CASE( "Test 10 Angle Axis", "[quaternion][linear_algebra][math]" ) {
    Quaternion quat1 = Quaternion().setFromAngleAxis(HALF_PI, Vec3f ( 2, 0, 0 ));
    Quaternion quat2 = Quaternion().setFromAngleNormalAxis(HALF_PI, Vec3f ( 1, 0, 0 ) );

    REQUIRE( quat2 == quat1 );
    REQUIRE( true == jau::equals(0.0f, 1 - quat2.magnitude()));
    REQUIRE( 1 - quat1.magnitude() <= std::numeric_limits<float>::epsilon() );

    Vec3f vecOut1 = quat1.rotateVector(ONE);
    Vec3f vecOut2 = quat2.rotateVector(ONE);
    REQUIRE( vecOut1 == vecOut2 );
    REQUIRE( true == jau::equals( 0.0f, std::abs( vecOut1.dist(vecOut2) ) ) );

    vecOut1 = quat1.rotateVector(UNIT_Z);
    REQUIRE( true == jau::equals( 0.0f, std::abs( UNIT_Y_NEG.dist(vecOut1) ) ) );

    quat2.setFromAngleAxis(HALF_PI, ZERO);
    REQUIRE( QUAT_IDENT == quat2 );

    float angle = quat1.toAngleAxis(vecOut1);
    quat2.setFromAngleAxis(angle, vecOut1);
    REQUIRE( quat1 == quat2 );

    quat1.set(0, 0, 0, 0);
    angle = quat1.toAngleAxis(vecOut1);
    REQUIRE(0.0f == angle);
    REQUIRE(UNIT_X == vecOut1);
}

TEST_CASE( "Test 11 From Vec to Vec", "[quaternion][linear_algebra][math]" ) {
    Vec3f vecOut;
    Quaternion quat;
    quat.setFromVectors(UNIT_Z, UNIT_X);

    Quaternion quat2;
    quat2.setFromNormalVectors(UNIT_Z, UNIT_X);
    REQUIRE( quat == quat2 );

    quat2.setFromAngleAxis(HALF_PI, UNIT_Y);
    REQUIRE( quat2 == quat );

    quat.setFromVectors(UNIT_Z, UNIT_Z_NEG);
    vecOut = quat.rotateVector(UNIT_Z);
    REQUIRE( true == jau::equals(0.0f, std::abs( UNIT_Z_NEG.dist(vecOut) ), Quaternion::ALLOWED_DEVIANCE ) );

    quat.setFromVectors(UNIT_X, UNIT_X_NEG);
    vecOut = quat.rotateVector(UNIT_X);
    REQUIRE( true == jau::equals(0.0f, std::abs( UNIT_X_NEG.dist(vecOut) ), Quaternion::ALLOWED_DEVIANCE ) );

    quat.setFromVectors(UNIT_Y, UNIT_Y_NEG);
    vecOut = quat.rotateVector(UNIT_Y);
    REQUIRE( true == jau::equals(0.0f, std::abs( UNIT_Y_NEG.dist(vecOut) ), Quaternion::ALLOWED_DEVIANCE ) );

    quat.setFromVectors(ONE, ONE_NEG);
    vecOut = quat.rotateVector(ONE);
    REQUIRE( true == jau::equals(0.0f, std::abs( ONE_NEG.dist(vecOut) ), Quaternion::ALLOWED_DEVIANCE ) );

    quat.setFromVectors(ZERO, ZERO);
    REQUIRE( QUAT_IDENT == quat );
}
