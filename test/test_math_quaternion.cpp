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

static const float PI = (float)M_PI;
static const float HALF_PI = (float)M_PI_2;
static const float QUARTER_PI = (float)M_PI_4;
static const float EPSILON = std::numeric_limits<float>::epsilon();

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

static const Vec4f ONE_NEG_v4 = Vec4f ( -1, -1, -1, 0 );
static const Vec4f ONE_v4     = Vec4f (  1,  1,  1, 0 );

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
    REQUIRE_THAT( std::abs( UNIT_Z_NEG.dist(vecOut) ), Catch::Matchers::WithinAbs(0.0f, Quaternion::ALLOWED_DEVIANCE) );

    quat.setFromVectors(UNIT_X, UNIT_X_NEG);
    vecOut = quat.rotateVector(UNIT_X);
    REQUIRE_THAT( std::abs( UNIT_X_NEG.dist(vecOut) ), Catch::Matchers::WithinAbs(0.0f, Quaternion::ALLOWED_DEVIANCE) );

    quat.setFromVectors(UNIT_Y, UNIT_Y_NEG);
    vecOut = quat.rotateVector(UNIT_Y);
    REQUIRE_THAT( std::abs( UNIT_Y_NEG.dist(vecOut) ), Catch::Matchers::WithinAbs(0.0f, Quaternion::ALLOWED_DEVIANCE) );

    quat.setFromVectors(ONE, ONE_NEG);
    vecOut = quat.rotateVector(ONE);
    REQUIRE_THAT( std::abs( ONE_NEG.dist(vecOut) ), Catch::Matchers::WithinAbs(0.0f, Quaternion::ALLOWED_DEVIANCE) );

    quat.setFromVectors(ZERO, ZERO);
    REQUIRE( QUAT_IDENT == quat );
}


TEST_CASE( "Test 12 From and to Euler Angles", "[quaternion][linear_algebra][math]" ) {
    // Y.Z.X -> X.Y.Z
    Quaternion quat;
    Vec3f angles0Exp( 0, HALF_PI, 0 );
    quat.setFromEuler(angles0Exp);
    REQUIRE_THAT( quat.magnitude(), Catch::Matchers::WithinAbs(1.0f, EPSILON) );

    Vec3f angles0Has = quat.toEuler();
    REQUIRE( angles0Exp == angles0Has );

    Quaternion quat2;
    quat2.setFromEuler(angles0Has);
    REQUIRE( quat == quat2 );

    ///

    Vec3f angles1Exp(0, 0, -HALF_PI);
    quat.setFromEuler(angles1Exp);
    REQUIRE_THAT( quat.magnitude(), Catch::Matchers::WithinAbs(1.0f, EPSILON) );

    Vec3f angles1Has = quat.toEuler();
    REQUIRE( angles1Exp == angles1Has );

    quat2.setFromEuler(angles1Has);
    REQUIRE( quat == quat2 );

    ///

    Vec3f angles2Exp(HALF_PI, 0, 0);
    quat.setFromEuler(angles2Exp);
    REQUIRE_THAT( quat.magnitude(), Catch::Matchers::WithinAbs(1.0f, EPSILON) );

    Vec3f angles2Has = quat.toEuler();
    REQUIRE( angles2Exp == angles2Has );

    quat2.setFromEuler(angles2Has);
    REQUIRE( quat == quat2 );
}

TEST_CASE( "Test 13 From Euler Angles and Rotate Vec", "[quaternion][linear_algebra][math]" ) {
    Quaternion quat;
    quat.setFromEuler(0, HALF_PI, 0); // 90 degrees y-axis
    REQUIRE_THAT( quat.magnitude(), Catch::Matchers::WithinAbs(1.0f, EPSILON) );

    Vec3f v2 = quat.rotateVector(UNIT_X);
    REQUIRE_THAT( std::abs( UNIT_Z_NEG.dist(v2)), Catch::Matchers::WithinAbs(0.0f, EPSILON) );

    quat.setFromEuler(0, 0, -HALF_PI);
    REQUIRE_THAT( quat.magnitude(), Catch::Matchers::WithinAbs(1.0f, EPSILON) );
    v2 = quat.rotateVector(UNIT_X);
    REQUIRE_THAT( std::abs( UNIT_Y_NEG.dist(v2)), Catch::Matchers::WithinAbs(0.0f, EPSILON) );

    quat.setFromEuler(HALF_PI, 0, 0);
    REQUIRE_THAT( quat.magnitude(), Catch::Matchers::WithinAbs(1.0f, EPSILON) );
    v2 = quat.rotateVector(UNIT_Y);
    REQUIRE_THAT( std::abs( UNIT_Z.dist(v2)), Catch::Matchers::WithinAbs(0.0f, EPSILON) );
}

TEST_CASE( "Test 14 Matrix", "[matrix][quaternion][linear_algebra][math]" ) {
    Vec3f vecHas;
    Vec3f vecOut3;
    // Vec4f vecOut4;
    Mat4f mat1;
    Mat4f mat2;
    Quaternion quat;

    //
    // IDENTITY CHECK
    //
    mat1.loadIdentity();
    quat.set(0, 0, 0, 0);
    quat.toMatrix(mat2);
    // mat2 = quat.toMatrix();
    REQUIRE( mat1 == mat2 );

    //
    // 90 degrees rotation on X
    //

    float a = HALF_PI;
    float mat1_0[] = { // Column Order
        1,  0,                 0,       0, //
        0,  std::cos(a),  std::sin(a),  0, //
        0, -std::sin(a),  std::cos(a),  0,
        0,  0,                 0,       1  };
    mat1.load( mat1_0 );
    {
        // Matrix4f load() <-> toFloats()
        float mat2_0[16];
        mat1.get(mat2_0);
        for(int i=0; i<16; ++i) { REQUIRE_THAT( mat2_0[i], Catch::Matchers::WithinAbs(mat1_0[i], EPSILON) ); }
    }
    {
        // Validate Matrix via Euler rotation on Quaternion!
        quat.setFromEuler(a, 0, 0);
        {
            quat.toMatrix(mat2);
            // mat2 = quat.toMatrix();
            REQUIRE( mat1 == mat2 );

            float mat2_0[16];
            mat2.get(mat2_0);
            Mat4f mat2c;
            mat2c.load(mat2_0);
            REQUIRE( mat2 == mat2c );
            REQUIRE( mat1 == mat2c );
        }
        vecHas = quat.rotateVector(UNIT_Y);
        REQUIRE_THAT( std::abs( UNIT_Z.dist(vecHas) ), Catch::Matchers::WithinAbs(0.0f, Quaternion::ALLOWED_DEVIANCE) );
    }
    mat1.getRotation(quat);
    quat.setFromMat3(mat1);
    vecHas = quat.rotateVector(UNIT_Y);
    REQUIRE_THAT( std::abs( UNIT_Z.dist(vecHas) ), Catch::Matchers::WithinAbs(0.0f, Quaternion::ALLOWED_DEVIANCE) );

    quat.toMatrix(mat2);
    REQUIRE( mat1 == mat2 );

    vecHas = quat.rotateVector(ONE_NEG);
    {
        // use Vec3f math
        mat2.mulVec3f(ONE_NEG, vecOut3);
        REQUIRE_THAT( std::abs( vecHas.dist(vecOut3) ), Catch::Matchers::WithinAbs(0.0f, Quaternion::ALLOWED_DEVIANCE) );
        REQUIRE( vecHas == vecOut3);
    }
    {
        // use Vec4f math

        // mat2.mulVec4f(ONE_NEG_v4, vecOut4);
        // vecOut3.set(vecOut4);
        vecOut3.set( mat2 * ONE_NEG_v4 ); // simpler

        REQUIRE_THAT( std::abs( vecHas.dist(vecOut3) ), Catch::Matchers::WithinAbs(0.0f, Quaternion::ALLOWED_DEVIANCE) );
        REQUIRE( vecHas == vecOut3);
    }

    //
    // 180 degrees rotation on X
    //
    a = PI;
    {
        float fa[] = { // Column Order
                1,  0,                 0,       0, //
                0,   std::cos(a), std::sin(a),  0, //
                0,  -std::sin(a), std::cos(a),  0,
                0,  0,                 0,       1 };
        mat1.load( fa );
    }
    {
        // Validate Matrix via Euler rotation on Quaternion!
        quat.setFromEuler(a, 0, 0);
        quat.toMatrix(mat2);
        REQUIRE(mat1 == mat2);
        vecHas = quat.rotateVector(UNIT_Y);
        REQUIRE_THAT( std::abs( UNIT_Y_NEG.dist(vecHas) ), Catch::Matchers::WithinAbs(0.0f, Quaternion::ALLOWED_DEVIANCE) );
    }
    quat.setFromMat3(mat1);
    vecHas = quat.rotateVector(UNIT_Y);
    REQUIRE_THAT( std::abs( UNIT_Y_NEG.dist(vecHas) ), Catch::Matchers::WithinAbs(0.0f, Quaternion::ALLOWED_DEVIANCE) );

    quat.toMatrix(mat2);
    REQUIRE(mat1 == mat2);

    vecHas = quat.rotateVector(ONE);
    // mat2.mulVec4f(ONE_v4, vecOut4);
    // vecOut3.set(vecOut4);
    vecOut3.set( mat2 * ONE_v4 ); // simpler
    REQUIRE_THAT( std::abs( vecHas.dist(vecOut3) ), Catch::Matchers::WithinAbs(0.0f, Quaternion::ALLOWED_DEVIANCE) );

    //
    // 180 degrees rotation on Y
    //
    a = PI;
    {
        float fa[] = { // Column Order
                 std::cos(a), 0,  -std::sin(a), 0, //
                 0,           1,   0,           0, //
                 std::sin(a), 0,   std::cos(a), 0,
                 0,           0,   0,           1 };
        mat1.load( fa );
    }
    {
        // Validate Matrix via Euler rotation on Quaternion!
        quat.setFromEuler(0, a, 0);
        quat.toMatrix(mat2);
        REQUIRE(mat1 == mat2);

        vecHas = quat.rotateVector(UNIT_X);
        REQUIRE_THAT( std::abs( UNIT_X_NEG.dist(vecHas) ), Catch::Matchers::WithinAbs(0.0f, Quaternion::ALLOWED_DEVIANCE) );
    }
    quat.setFromMat3(mat1);
    vecHas = quat.rotateVector(UNIT_X);
    REQUIRE_THAT( std::abs( UNIT_X_NEG.dist(vecHas) ), Catch::Matchers::WithinAbs(0.0f, Quaternion::ALLOWED_DEVIANCE) );

    quat.toMatrix(mat2);
    REQUIRE(mat1 == mat2);

    vecHas = quat.rotateVector(ONE_NEG);
    // mat2.mulVec4f(ONE_NEG_v4, vecOut4);
    // vecOut3.set(vecOut4);
    vecOut3.set( mat2 * ONE_NEG_v4 ); // simpler
    REQUIRE_THAT( std::abs( vecHas.dist(vecOut3) ), Catch::Matchers::WithinAbs(0.0f, Quaternion::ALLOWED_DEVIANCE) );

    //
    // 180 degrees rotation on Z
    //
    a = PI;
    {
        float fa[] = { // Column Order
                  std::cos(a), std::sin(a), 0, 0, //
                 -std::sin(a), std::cos(a), 0, 0,
                  0,           0,           1, 0,
                  0,           0,           0, 1 };
        mat1.load( fa );
    }
    {
        // Validate Matrix via Euler rotation on Quaternion!
        quat.setFromEuler(0, 0, a);
        quat.toMatrix(mat2);
        REQUIRE(mat1 == mat2);
        vecHas = quat.rotateVector(UNIT_X);
        REQUIRE_THAT( std::abs( UNIT_X_NEG.dist(vecHas) ), Catch::Matchers::WithinAbs(0.0f, Quaternion::ALLOWED_DEVIANCE) );
    }
    quat.setFromMat3(mat1);
    vecHas = quat.rotateVector(UNIT_X);
    REQUIRE_THAT( std::abs( UNIT_X_NEG.dist(vecHas) ), Catch::Matchers::WithinAbs(0.0f, Quaternion::ALLOWED_DEVIANCE) );

    quat.toMatrix(mat2);
    REQUIRE(mat1 == mat2);

    vecHas = quat.rotateVector(ONE);
    // mat2.mulVec4f(ONE_v4, vecOut4);
    // vecOut3.set(vecOut4);
    vecOut3.set( mat2 * ONE_v4 ); // simpler
    REQUIRE_THAT( std::abs( vecHas.dist(vecOut3) ), Catch::Matchers::WithinAbs(0.0f, Quaternion::ALLOWED_DEVIANCE) );

    //
    // Test Matrix-Columns
    //

    a = QUARTER_PI;
    Vec3f vecExp0( std::cos(a), std::sin(a), 0);
    Vec3f vecExp1(-std::sin(a), std::cos(a), 0);
    Vec3f vecExp2( 0,                0,                1);
    Vec3f vecCol;
    {
        float fa[] = { // Column Order
                std::cos(a), std::sin(a), 0, 0, //
               -std::sin(a), std::cos(a), 0, 0,
                0,           0,           1, 0,
                0,           0,           0, 1 };
        mat1.load( fa );
    }
    mat1.getColumn(0, vecCol);
    REQUIRE(vecExp0 == vecCol);
    REQUIRE_THAT( std::abs( vecExp0.dist(vecCol) ), Catch::Matchers::WithinAbs(0.0f, EPSILON) );

    mat1.getColumn(1, vecCol);
    REQUIRE(vecExp1 == vecCol);
    REQUIRE_THAT( std::abs( vecExp1.dist(vecCol) ), Catch::Matchers::WithinAbs(0.0f, EPSILON) );

    mat1.getColumn(2, vecCol);
    REQUIRE(vecExp2 == vecCol);
    REQUIRE_THAT( std::abs( vecExp2.dist(vecCol) ), Catch::Matchers::WithinAbs(0.0f, EPSILON) );
}
