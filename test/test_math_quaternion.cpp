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

#include <jau/math/mat4f.hpp>
#include <jau/math/quaternion.hpp>

using namespace jau;
using namespace jau::math;

static const float PI = (float)M_PI;
static const float HALF_PI = (float)M_PI_2;
static const float QUARTER_PI = (float)M_PI_4;
static const float EPSILON = std::numeric_limits<float>::epsilon();

static const Quat4f QUAT_IDENT = Quat4f(0, 0, 0, 1);

static const Vec3f ZERO       = Vec3f (  0,  0,  0 );
static const Vec3f ONE        = Vec3f (  1,  1,  1 );
static const Vec3f NEG_ONE    = Vec3f ( -1, -1, -1 );
static const Vec3f UNIT_X     = Vec3f (  1,  0,  0 );
static const Vec3f UNIT_Y     = Vec3f (  0,  1,  0 );
static const Vec3f UNIT_Z     = Vec3f (  0,  0,  1 );
static const Vec3f NEG_UNIT_X = Vec3f ( -1,  0,  0 );
static const Vec3f NEG_UNIT_Y = Vec3f (  0, -1,  0 );
static const Vec3f NEG_UNIT_Z = Vec3f (  0,  0, -1 );

static const Vec4f NEG_ONE_v4 = Vec4f ( -1, -1, -1, 0 );
static const Vec4f ONE_v4     = Vec4f (  1,  1,  1, 0 );

constexpr static const bool DEBUG_MODE = false;

//
// Basic
//

TEST_CASE( "Test 01 Normalize", "[quaternion][linear_algebra][math]" ) {
    const Quat4f quat(0, 1, 2, 3);
    const Quat4f quat2 = Quat4f(quat).normalize();
    // Assert.assertTrue(Math.abs(1 - quat2.magnitude()) <= MACH_EPSILON);
    REQUIRE( true == jau::equals( 0.0f, std::abs( 1 - quat2.magnitude() ) ) ) ;
}

TEST_CASE( "Test 02 Rotate Zero Vector", "[quaternion][linear_algebra][math]" ) {
    Quat4f quat;
    const Vec3f rotVec0 = quat.rotateVector(ZERO);
    REQUIRE( ZERO == rotVec0 );
}

TEST_CASE( "Test 03 Invert and Conugate", "[quaternion][linear_algebra][math]" ) {
    // inversion check
    {
        const Quat4f quat0(0, 1, 2, 3);
        Quat4f quat0Inv = Quat4f(quat0).invert();
        REQUIRE( quat0 == quat0Inv.invert() );
    }
    // conjugate check
    {
        const Quat4f quat0(-1, -2, -3, 4);
        const Quat4f quat0Conj = Quat4f( 1,  2,  3, 4).conjugate();
        REQUIRE( quat0 == quat0Conj );
    }
}

TEST_CASE( "Test 04 Dot", "[quaternion][linear_algebra][math]" ) {
    const Quat4f quat(7, 2, 5, -1);
    REQUIRE( 35.0f == quat.dot(3, 1, 2, -2));
    REQUIRE( -11.0f == quat.dot(Quat4f(-1, 1, -1, 1)));
}

//
// Conversion
//

TEST_CASE( "Test 10 Angle Axis", "[quaternion][linear_algebra][math]" ) {
    Quat4f quat1 = Quat4f().setFromAngleAxis(HALF_PI, Vec3f ( 2, 0, 0 ));
    Quat4f quat2 = Quat4f().setFromAngleNormalAxis(HALF_PI, Vec3f ( 1, 0, 0 ) );

    REQUIRE( quat2 == quat1 );
    REQUIRE( true == jau::equals(0.0f, 1 - quat2.magnitude()));
    REQUIRE( 1 - quat1.magnitude() <= std::numeric_limits<float>::epsilon() );

    Vec3f vecOut1 = quat1.rotateVector(ONE);
    Vec3f vecOut2 = quat2.rotateVector(ONE);
    REQUIRE( vecOut1 == vecOut2 );
    REQUIRE( true == jau::equals( 0.0f, std::abs( vecOut1.dist(vecOut2) ) ) );

    vecOut1 = quat1.rotateVector(UNIT_Z);
    REQUIRE( true == jau::equals( 0.0f, std::abs( NEG_UNIT_Y.dist(vecOut1) ) ) );

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
    Quat4f quat;
    quat.setFromVectors(UNIT_Z, UNIT_X);

    Quat4f quat2;
    quat2.setFromNormalVectors(UNIT_Z, UNIT_X);
    REQUIRE( quat == quat2 );

    quat2.setFromAngleAxis(HALF_PI, UNIT_Y);
    REQUIRE( quat2 == quat );

    quat.setFromVectors(UNIT_Z, NEG_UNIT_Z);
    vecOut = quat.rotateVector(UNIT_Z);
    REQUIRE_THAT( std::abs( NEG_UNIT_Z.dist(vecOut) ), Catch::Matchers::WithinAbs(0.0f, Quat4f::allowed_deviation) );

    quat.setFromVectors(UNIT_X, NEG_UNIT_X);
    vecOut = quat.rotateVector(UNIT_X);
    REQUIRE_THAT( std::abs( NEG_UNIT_X.dist(vecOut) ), Catch::Matchers::WithinAbs(0.0f, Quat4f::allowed_deviation) );

    quat.setFromVectors(UNIT_Y, NEG_UNIT_Y);
    vecOut = quat.rotateVector(UNIT_Y);
    REQUIRE_THAT( std::abs( NEG_UNIT_Y.dist(vecOut) ), Catch::Matchers::WithinAbs(0.0f, Quat4f::allowed_deviation) );

    quat.setFromVectors(ONE, NEG_ONE);
    vecOut = quat.rotateVector(ONE);
    REQUIRE_THAT( std::abs( NEG_ONE.dist(vecOut) ), Catch::Matchers::WithinAbs(0.0f, Quat4f::allowed_deviation) );

    quat.setFromVectors(ZERO, ZERO);
    REQUIRE( QUAT_IDENT == quat );
}


TEST_CASE( "Test 12 From and to Euler Angles", "[quaternion][linear_algebra][math]" ) {
    // Y.Z.X -> X.Y.Z
    Quat4f quat;
    Vec3f angles0Exp( 0, HALF_PI, 0 );
    quat.setFromEuler(angles0Exp);
    REQUIRE_THAT( quat.magnitude(), Catch::Matchers::WithinAbs(1.0f, EPSILON) );

    Vec3f angles0Has = quat.toEuler();
    REQUIRE( angles0Exp == angles0Has );

    Quat4f quat2;
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
    Quat4f quat;
    quat.setFromEuler(0, HALF_PI, 0); // 90 degrees y-axis
    REQUIRE_THAT( quat.magnitude(), Catch::Matchers::WithinAbs(1.0f, EPSILON) );

    Vec3f v2 = quat.rotateVector(UNIT_X);
    REQUIRE_THAT( std::abs( NEG_UNIT_Z.dist(v2)), Catch::Matchers::WithinAbs(0.0f, EPSILON) );

    quat.setFromEuler(0, 0, -HALF_PI);
    REQUIRE_THAT( quat.magnitude(), Catch::Matchers::WithinAbs(1.0f, EPSILON) );
    v2 = quat.rotateVector(UNIT_X);
    REQUIRE_THAT( std::abs( NEG_UNIT_Y.dist(v2)), Catch::Matchers::WithinAbs(0.0f, EPSILON) );

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
    Quat4f quat1;
    Quat4f quat2;

    //
    // IDENTITY CHECK
    //
    mat1.loadIdentity();
    quat1.set(0, 0, 0, 0);
    quat1.toMatrix(mat2);
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
        // Validate Matrix via Euler rotation on Quat4f!
        quat1.setFromEuler(a, 0, 0);
        {
            quat1.toMatrix(mat2);
            // mat2 = quat.toMatrix();
            REQUIRE( mat1 == mat2 );
            quat2.setFromMat(mat1);
            REQUIRE( quat1 == quat2 );

            float mat2_0[16];
            mat2.get(mat2_0);
            Mat4f mat2c;
            mat2c.load(mat2_0);
            REQUIRE( mat2 == mat2c );
            REQUIRE( mat1 == mat2c );
        }
        vecHas = quat1.rotateVector(UNIT_Y);
        REQUIRE_THAT( std::abs( UNIT_Z.dist(vecHas) ), Catch::Matchers::WithinAbs(0.0f, Quat4f::allowed_deviation) );
    }
    {
        quat1.toMatrix(mat1);
        quat2.setFromMat(mat1);
        REQUIRE( quat1 == quat2 );
    }
    vecHas = quat1.rotateVector(UNIT_Y);
    REQUIRE_THAT( std::abs( UNIT_Z.dist(vecHas) ), Catch::Matchers::WithinAbs(0.0f, Quat4f::allowed_deviation) );

    quat1.toMatrix(mat2);
    REQUIRE( mat1 == mat2 );

    vecHas = quat1.rotateVector(NEG_ONE);
    {
        // use Vec3f math
        mat2.mulVec3(NEG_ONE, vecOut3);
        REQUIRE_THAT( std::abs( vecHas.dist(vecOut3) ), Catch::Matchers::WithinAbs(0.0f, Quat4f::allowed_deviation) );
        REQUIRE( vecHas == vecOut3);
    }
    {
        // use Vec4f math
        (mat2 * NEG_ONE_v4).getVec3(vecOut3);

        REQUIRE_THAT( std::abs( vecHas.dist(vecOut3) ), Catch::Matchers::WithinAbs(0.0f, Quat4f::allowed_deviation) );
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
        // Validate Matrix via Euler rotation on Quat4f!
        quat1.setFromEuler(a, 0, 0);
        quat1.toMatrix(mat2);
        REQUIRE(mat1 == mat2);
        vecHas = quat1.rotateVector(UNIT_Y);
        REQUIRE_THAT( std::abs( NEG_UNIT_Y.dist(vecHas) ), Catch::Matchers::WithinAbs(0.0f, Quat4f::allowed_deviation) );
    }
    quat1.setFromMat(mat1);
    vecHas = quat1.rotateVector(UNIT_Y);
    REQUIRE_THAT( std::abs( NEG_UNIT_Y.dist(vecHas) ), Catch::Matchers::WithinAbs(0.0f, Quat4f::allowed_deviation) );

    quat1.toMatrix(mat2);
    REQUIRE(mat1 == mat2);

    vecHas = quat1.rotateVector(ONE);
    (mat2 * ONE_v4).getVec3(vecOut3);
    REQUIRE_THAT( std::abs( vecHas.dist(vecOut3) ), Catch::Matchers::WithinAbs(0.0f, Quat4f::allowed_deviation) );

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
        // Validate Matrix via Euler rotation on Quat4f!
        quat1.setFromEuler(0, a, 0);
        quat1.toMatrix(mat2);
        REQUIRE(mat1 == mat2);

        vecHas = quat1.rotateVector(UNIT_X);
        REQUIRE_THAT( std::abs( NEG_UNIT_X.dist(vecHas) ), Catch::Matchers::WithinAbs(0.0f, Quat4f::allowed_deviation) );
    }
    quat1.setFromMat(mat1);
    vecHas = quat1.rotateVector(UNIT_X);
    REQUIRE_THAT( std::abs( NEG_UNIT_X.dist(vecHas) ), Catch::Matchers::WithinAbs(0.0f, Quat4f::allowed_deviation) );

    quat1.toMatrix(mat2);
    REQUIRE(mat1 == mat2);

    vecHas = quat1.rotateVector(NEG_ONE);
    (mat2 * NEG_ONE_v4).getVec3(vecOut3);
    REQUIRE_THAT( std::abs( vecHas.dist(vecOut3) ), Catch::Matchers::WithinAbs(0.0f, Quat4f::allowed_deviation) );

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
        // Validate Matrix via Euler rotation on Quat4f!
        quat1.setFromEuler(0, 0, a);
        quat1.toMatrix(mat2);
        REQUIRE(mat1 == mat2);
        vecHas = quat1.rotateVector(UNIT_X);
        REQUIRE_THAT( std::abs( NEG_UNIT_X.dist(vecHas) ), Catch::Matchers::WithinAbs(0.0f, Quat4f::allowed_deviation) );
    }
    quat1.setFromMat(mat1);
    vecHas = quat1.rotateVector(UNIT_X);
    REQUIRE_THAT( std::abs( NEG_UNIT_X.dist(vecHas) ), Catch::Matchers::WithinAbs(0.0f, Quat4f::allowed_deviation) );

    quat1.toMatrix(mat2);
    REQUIRE(mat1 == mat2);

    vecHas = quat1.rotateVector(ONE);
    vecOut3 = to_vec3( mat2 * ONE_v4 );
    // (mat2 * ONE_v4).getVec3(vecOut3);
    REQUIRE_THAT( std::abs( vecHas.dist(vecOut3) ), Catch::Matchers::WithinAbs(0.0f, Quat4f::allowed_deviation) );

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

TEST_CASE( "Test 15a Axes And Matrix", "[matrix][quaternion][linear_algebra][math]" ) {
    Vec3f eulerExp( 0, HALF_PI, 0 );
    Mat4f matExp1;
    matExp1.setToRotationEuler(eulerExp);

    Mat4f matHas;
    Quat4f quat1;
    quat1.setFromEuler(eulerExp);
    quat1.toMatrix(matHas);
    REQUIRE( matExp1 == matHas);

    Quat4f quat2;
    quat2.setFromMat(matExp1);
    Vec3f eulerHas = quat2.toEuler();
    std::cout << "exp-euler " << eulerExp << std::endl;
    std::cout << "has-euler " << eulerHas << std::endl;
    REQUIRE(eulerExp == eulerHas);

    REQUIRE(quat2 == quat1);

    Vec3f angles = quat2.toEuler();
    quat1.setFromEuler(angles);
    REQUIRE(quat2 == quat1);
}

TEST_CASE( "Test 15b Axes And Matrix", "[matrix][quaternion][linear_algebra][math]" ) {
    Vec3f eulerExp(HALF_PI, 0, 0);
    Mat4f matExp;
    matExp.setToRotationEuler(eulerExp);

    Mat4f matHas;
    Quat4f quat1;
    quat1.setFromEuler(eulerExp);
    quat1.toMatrix(matHas);
    REQUIRE( matExp == matHas);

    Quat4f quat2;
    quat2.setFromMat(matExp);
    Vec3f eulerHas = quat2.toEuler();
    std::cout << "exp-euler " << eulerExp << std::endl;
    std::cout << "has-euler " << eulerHas << std::endl;
    REQUIRE(eulerExp == eulerHas);

    REQUIRE(quat2 == quat1);

    Vec3f angles = quat2.toEuler();
    quat1.setFromEuler(angles);
    REQUIRE(quat2 == quat1);
}

TEST_CASE( "Test 15c Axes And Matrix", "[matrix][quaternion][linear_algebra][math]" ) {
    Vec3f eulerExp1(QUARTER_PI, HALF_PI, 0); // 45 degr on X, 90 degr on Y
    float eulerExp0[3];
    eulerExp1.get(eulerExp0);

    Mat4f matExp;
    matExp.setToRotationEuler(eulerExp1);

    Mat4f matHas;
    Quat4f quat1;
    quat1.setFromEuler(eulerExp1);
    quat1.toMatrix(matHas);
    printf("float epsilon %.20f\n", EPSILON);
    std::cout << "matExp " << matExp << std::endl;
    std::cout << "matHas " << matHas << std::endl;
    // REQUIRE(matExp == matHas);
    REQUIRE( matExp.equals(matHas, 2*EPSILON) ); // due to clang -O3 math optimizations, gcc -O3 OK
    // eps  0.00000011920928955078
    // exp -0.000000044
    // has  0.000000000 (gcc -O3, ok)
    // has  0.000000119 (clang -O3, err)

    Quat4f quat2;
    quat2.setFromMat(matExp);
    Vec3f eulerHas1 = quat2.toEuler();
    std::cout << "exp-euler " << eulerExp1 << std::endl;
    std::cout << "has-euler " << eulerHas1 << std::endl;
    std::cout << "diff-euler " << ( eulerExp1 - eulerHas1 ) << std::endl;
    {
        float eulerHas0[3];
        eulerHas1.get(eulerHas0);
        Vec3f eulerHas0v(eulerHas0), eulerExp0v(eulerExp0);
        REQUIRE( eulerHas0v == eulerExp0v );
    }
    REQUIRE_THAT( std::abs( eulerExp1.dist(eulerHas1) ), Catch::Matchers::WithinAbs(0.0f, Quat4f::allowed_deviation) );
    REQUIRE( true == eulerExp1.equals(eulerHas1, Quat4f::allowed_deviation) );

    REQUIRE(quat2 == quat1);

    Vec3f angles = quat2.toEuler();
    quat1.setFromEuler(angles);
    REQUIRE(quat2 == quat1);
}

//
// Functions
//

TEST_CASE( "Test 20 Add Subtract", "[quaternion][linear_algebra][math]" ) {
    {
        Quat4f quatExp(1, 2, 3, 4);
        Quat4f quat1(0, 1, 2, 3);
        Quat4f quat2(1, 1, 1, 1);

        // +=
        Quat4f quatHas = quat1;
        quatHas += quat2;
        REQUIRE(quatExp == quatHas);

        // +
        quatHas = quat1 + quat2;
        REQUIRE(quatExp == quatHas);

    }
    {
        Quat4f quatExp(-1, 0, 1, 2);
        Quat4f quat1, quat2, quatHas;
        quat1.set(0, 1, 2, 3);
        quat2.set(1, 1, 1, 1);

        // -=
        quatHas = quat1;
        quatHas -= quat2; // q3 = q1 - q2
        REQUIRE(quatExp == quatHas);

        // -
        quatHas = quat1 - quat2;
        REQUIRE(quatExp == quatHas);
    }
}

TEST_CASE( "Test 21 Multiply", "[quaternion][linear_algebra][math]" ) {
    // scalar
    {
        Quat4f quatExp(1, 2, 4, 6);
        Quat4f quat1(0.5f, 1, 2, 3);
        Quat4f quat2;

        // *= scalar
        quat2 = quat1;
        quat2 *= 2; // q2 = q1 * 2
        REQUIRE(quatExp ==  quat2);

        // * scalar
        quat2 = quat1 * 2.0f;
        REQUIRE(quatExp ==  quat2);
        // * scalar
        quat2 = 2.0f * quat1;
        REQUIRE(quatExp ==  quat2);
    }

    {
        Quat4f quat1;
        Quat4f quat2;

        //
        // mul and cmp rotated vector
        //
        {
            // q *= q
            quat1.setFromAngleNormalAxis(QUARTER_PI, UNIT_Y); // 45 degr on Y
            quat2 = quat1;
            quat2 *= quat1; // q2 = q1 * q1 -> 2 * 45 degr -> 90 degr on Y
            Vec3f vecOut = quat2.rotateVector(UNIT_Z);
            REQUIRE_THAT( std::abs( UNIT_X.dist(vecOut) ), Catch::Matchers::WithinAbs(0.0f, Quat4f::allowed_deviation) );

            // q * q
            quat1.setFromAngleNormalAxis(QUARTER_PI, UNIT_Y); // 45 degr on Y
            quat2 = quat1 * quat1; // q2 = q1 * q1 -> 2 * 45 degr -> 90 degr on Y
            vecOut = quat2.rotateVector(UNIT_Z);
            REQUIRE_THAT( std::abs( UNIT_X.dist(vecOut) ), Catch::Matchers::WithinAbs(0.0f, Quat4f::allowed_deviation) );
        }
        {
            quat1.setFromAngleNormalAxis(QUARTER_PI, UNIT_Y); // 45 degr on Y
            quat2.setFromAngleNormalAxis(HALF_PI, UNIT_Y); // 90 degr on Y
            quat1 *= quat1; // q1 = q1 * q1 -> 2 * 45 degr ->  90 degr on Y
            quat1 *= quat2; // q1 = q1 * q2 -> 2 * 90 degr -> 180 degr on Y
            Vec3f vecOut = quat1.rotateVector(UNIT_Z);
            REQUIRE_THAT( std::abs( NEG_UNIT_Z.dist(vecOut) ), Catch::Matchers::WithinAbs(0.0f, Quat4f::allowed_deviation) );

            quat1.setFromAngleNormalAxis(QUARTER_PI, UNIT_Y); // 45 degr on Y
            quat2.setFromAngleNormalAxis(HALF_PI, UNIT_Y); // 90 degr on Y
            quat1 = quat1 * quat1 * quat2; // q1 = q1 * q1 * q2 -> 2 * 90 degr -> 180 degr on Y
            quat1.rotateVector(UNIT_Z, vecOut);
            REQUIRE_THAT( std::abs( NEG_UNIT_Z.dist(vecOut) ), Catch::Matchers::WithinAbs(0.0f, Quat4f::allowed_deviation) );
        }
        {
            quat2.setFromEuler(0, HALF_PI, 0);
            quat1 *= quat2; // q1 = q1 * q2 = q1 * rotMat(0, 90degr, 0)
            Vec3f vecOut = quat1.rotateVector(UNIT_Z);
            REQUIRE_THAT( std::abs( NEG_UNIT_X.dist(vecOut) ), Catch::Matchers::WithinAbs(0.0f, Quat4f::allowed_deviation) );
        }
    }
}

TEST_CASE( "Test 22 Invert-Mult-Normal-Conjugate", "[quaternion][linear_algebra][math]" ) {
    Quat4f quat0(0, 1, 2, 3);
    Quat4f quat1(quat0);
    Quat4f quat2(quat0);
    quat1.invert();    // q1 = invert(q0)
    quat2 *= quat1;    // q2 = q0 * q1 = q0 * invert(q0)
    REQUIRE(QUAT_IDENT == quat2);
    quat1.invert();
    REQUIRE(quat0 == quat1);

    // normalized version
    quat0.setFromAngleNormalAxis(QUARTER_PI, UNIT_Y);
    quat1 = quat0;
    quat1.invert(); // q1 = invert(q0)
    quat2 = quat0 * quat1; // q2 = q0 * q1 = q0 * invert(q0)
    REQUIRE(QUAT_IDENT == quat2);
    quat1.invert();
    REQUIRE(quat0 == quat1);

    // conjugate check
    quat0.set(-1.0f, -2.0f, -3.0f, 4.0f);
    quat1.set( 1.0f,  2.0f,  3.0f, 4.0f);
    quat2 = quat1;
    quat2.conjugate();
    REQUIRE(quat0 == quat2);
}

TEST_CASE( "Test 23 Rotation Order", "[quaternion][linear_algebra][math]" ) {
    {
        Quat4f quat1; quat1.setFromEuler( -2 * HALF_PI, 0, 0); // -180 degr X
        Quat4f quat2; quat2.rotateByAngleX( -2 * HALF_PI); // angle: -180 degrees, axis X
        REQUIRE(quat1 == quat2);
    }
    {
        Quat4f quat1 = Quat4f().setFromEuler(    HALF_PI, 0, 0); //   90 degr X
        Quat4f quat2 = Quat4f().rotateByAngleX(  HALF_PI); // angle:   90 degrees, axis X
        REQUIRE(quat1 == quat2);
    }
    {
        Quat4f quat1 = Quat4f().setFromEuler(  HALF_PI, QUARTER_PI, 0);
        Quat4f quat2 = Quat4f().rotateByAngleY(QUARTER_PI).rotateByAngleX(HALF_PI);
        REQUIRE(quat1 == quat2);
    }
    {
        Quat4f quat1 = Quat4f().setFromEuler( PI, QUARTER_PI, HALF_PI);
        Quat4f quat2 = Quat4f().rotateByAngleY(QUARTER_PI).rotateByAngleZ(HALF_PI).rotateByAngleX(PI);
        REQUIRE(quat1 == quat2);
    }

    Vec3f vecExp;
    Vec3f vecRot;
    Quat4f quat;

    // Try a new way with new angles...
    quat.setFromEuler(HALF_PI, QUARTER_PI, PI);
    vecRot.set(1, 1, 1);
    quat.rotateVector(vecRot, vecRot); // in-place

    // expected
    Quat4f worker;
    // put together matrix, then apply to vector, so YZX
    worker.rotateByAngleY(QUARTER_PI).rotateByAngleZ(PI).rotateByAngleX(HALF_PI);
    vecExp.set(1, 1, 1);
    vecExp = quat.rotateVector(vecExp); // new vec3, assign back
    REQUIRE_THAT( vecExp.dist(vecRot), Catch::Matchers::WithinAbs(0.0f, EPSILON) );
    REQUIRE(vecExp == vecRot);

    // test axis rotation methods against general purpose
    // X AXIS
    vecExp.set(1, 1, 1);
    vecRot.set(1, 1, 1);
    worker.setIdentity().rotateByAngleX(QUARTER_PI).rotateVector(vecExp, vecExp);
    worker.setIdentity().rotateByAngleNormalAxis(QUARTER_PI, 1, 0, 0).rotateVector(vecRot, vecRot);
    REQUIRE_THAT( vecExp.dist(vecRot), Catch::Matchers::WithinAbs(0.0f, EPSILON) );
    REQUIRE(vecExp == vecRot);

    // Y AXIS
    vecExp.set(1, 1, 1);
    vecRot.set(1, 1, 1);
    worker.setIdentity().rotateByAngleY(QUARTER_PI).rotateVector(vecExp, vecExp);
    worker.setIdentity().rotateByAngleNormalAxis(QUARTER_PI, 0, 1, 0).rotateVector(vecRot, vecRot);
    REQUIRE_THAT( vecExp.dist(vecRot), Catch::Matchers::WithinAbs(0.0f, EPSILON) );
    REQUIRE(vecExp == vecRot);

    // Z AXIS
    vecExp.set(1, 1, 1);
    vecRot.set(1, 1, 1);
    worker.setIdentity().rotateByAngleZ(QUARTER_PI).rotateVector(vecExp, vecExp);
    worker.setIdentity().rotateByAngleNormalAxis(QUARTER_PI, 0, 0, 1).rotateVector(vecRot, vecRot);
    REQUIRE_THAT( vecExp.dist(vecRot), Catch::Matchers::WithinAbs(0.0f, EPSILON) );
    REQUIRE(vecExp == vecRot);

    quat = worker;
    worker.rotateByAngleNormalAxis(0, 0, 0, 0);
    REQUIRE(quat == worker);
}

TEST_CASE( "Test 24 Axes", "[quaternion][linear_algebra][math]" ) {
    Quat4f quat0 = Quat4f().rotateByAngleX(QUARTER_PI).rotateByAngleY(HALF_PI);
    Mat4f rotMat;
    quat0.toMatrix(rotMat);
    Vec3f xAxis, yAxis, zAxis;
    rotMat.getColumn(0, xAxis);
    rotMat.getColumn(1, yAxis);
    rotMat.getColumn(2, zAxis);

    Quat4f quat1 = Quat4f().setFromAxes(xAxis, yAxis, zAxis);
    REQUIRE(quat0 == quat1);
    Quat4f quat2 = Quat4f().setFromMat(rotMat);
    REQUIRE(quat2 == quat1);

    quat1.toAxes(xAxis, yAxis, zAxis, rotMat);
    quat2.setFromAxes(xAxis, yAxis, zAxis);
    REQUIRE(quat0 == quat2);
    REQUIRE(quat1 == quat2);
}

TEST_CASE( "Test 25 Slerp", "[quaternion][linear_algebra][math]" ) {
    Quat4f quat1;     // angle: 0 degrees
    Quat4f quat2 = Quat4f().rotateByAngleY(HALF_PI); // angle: 90 degrees, axis Y

    Vec3f vecExp( std::sin(QUARTER_PI), 0, std::sin(QUARTER_PI) );
    Vec3f vecHas;
    Quat4f quatS;
    // System.err.println("Slerp #01: 1/2 * 90 degrees Y");
    quatS.setSlerp(quat1, quat2, 0.5f);
    quatS.rotateVector(UNIT_Z, vecHas);
    // System.err.println("exp0 "+Arrays.toString(vecExp));
    // System.err.println("has0 "+Arrays.toString(vecHas));
    REQUIRE_THAT( std::abs( vecExp.dist(vecHas)), Catch::Matchers::WithinAbs(0.0f, Quat4f::allowed_deviation) );

    if( !vecExp.equals(vecHas) ) {
        std::cout << "Deviation: " << vecExp << ", " << vecHas << ": " << ( vecExp - vecHas ) << ", dist " << vecExp.dist(vecHas) << std::endl;
    }
    // REQUIRE(vecExp == vecHas);

    // delta == 100%
    quat2.setIdentity().rotateByAngleZ(PI); // angle: 180 degrees, axis Z
    // System.err.println("Slerp #02: 1 * 180 degrees Z");
    quatS.setSlerp(quat1, quat2, 1.0f);
    quatS.rotateVector(UNIT_X, vecHas);
    // System.err.println("exp0 "+Arrays.toString(NEG_UNIT_X));
    // System.err.println("has0 "+Arrays.toString(vecHas));
    REQUIRE_THAT( std::abs( NEG_UNIT_X.dist(vecHas)), Catch::Matchers::WithinAbs(0.0f, Quat4f::allowed_deviation) );
    REQUIRE(NEG_UNIT_X == vecHas);

    quat2.setIdentity().rotateByAngleZ(PI); // angle: 180 degrees, axis Z
    // System.err.println("Slerp #03: 1/2 * 180 degrees Z");
    quatS.setSlerp(quat1, quat2, 0.5f);
    quatS.rotateVector(UNIT_X, vecHas);
    // System.err.println("exp0 "+Arrays.toString(UNIT_Y));
    // System.err.println("has0 "+Arrays.toString(vecHas));
    REQUIRE_THAT( std::abs( UNIT_Y.dist(vecHas)), Catch::Matchers::WithinAbs(0.0f, Quat4f::allowed_deviation) );
    if( !UNIT_Y.equals(vecHas) ) {
        std::cout << "Deviation: " << UNIT_Y << ", " << vecHas << ": " << ( UNIT_Y - vecHas ) << ", dist " << UNIT_Y.dist(vecHas) << std::endl;
    }
    // REQUIRE(UNIT_Y == vecHas);

    // delta == 0%
    quat2.setIdentity().rotateByAngleZ(PI); // angle: 180 degrees, axis Z
    // System.err.println("Slerp #04: 0 * 180 degrees Z");
    quatS.setSlerp(quat1, quat2, 0.0f);
    quatS.rotateVector(UNIT_X, vecHas);
    // System.err.println("exp0 "+Arrays.toString(UNIT_X));
    // System.err.println("has0 "+Arrays.toString(vecHas));
    REQUIRE_THAT( std::abs( UNIT_X.dist(vecHas)), Catch::Matchers::WithinAbs(0.0f, Quat4f::allowed_deviation) );
    REQUIRE(UNIT_X == vecHas);

    // a==b
    quat2.setIdentity();
    // System.err.println("Slerp #05: 1/4 * 0 degrees");
    quatS.setSlerp(quat1, quat2, 0.25f); // 1/4 of identity .. NOP
    quatS.rotateVector(UNIT_X, vecHas);
    // System.err.println("exp0 "+Arrays.toString(UNIT_X));
    // System.err.println("has0 "+Arrays.toString(vecHas));
    REQUIRE_THAT( std::abs( UNIT_X.dist(vecHas)), Catch::Matchers::WithinAbs(0.0f, Quat4f::allowed_deviation) );
    REQUIRE(UNIT_X == vecHas);

    // negative dot product
    vecExp.set(0, -std::sin(QUARTER_PI), std::sin(QUARTER_PI));
    quat1.setIdentity().rotateByAngleX( -2 * HALF_PI); // angle: -180 degrees, axis X
    quat2.setIdentity().rotateByAngleX(      HALF_PI); // angle:   90 degrees, axis X
    // System.err.println("Slerp #06: 1/2 * 270 degrees");
    quatS.setSlerp(quat1, quat2, 0.5f);
    quatS.rotateVector(UNIT_Y, vecHas);
    // System.err.println("exp0 "+Arrays.toString(vecExp));
    // System.err.println("has0 "+Arrays.toString(vecHas));
    REQUIRE_THAT( std::abs( vecExp.dist(vecHas)), Catch::Matchers::WithinAbs(0.0f, Quat4f::allowed_deviation) );
    if( !vecExp.equals(vecHas) ) {
        std::cout << "Deviation: " << vecExp << ", " << vecHas << ": " << ( vecExp - vecHas ) << ", dist " << vecExp.dist(vecHas) << std::endl;
    }
    // REQUIRE(vecExp == vecHas);
}

TEST_CASE( "Test 26 LookAt", "[quaternion][linear_algebra][math]" ) {
    Vec3f xAxis, yAxis, zAxis, vecHas;

    if( DEBUG_MODE ) std::cout << "LookAt #01" << std::endl;
    Vec3f direction = NEG_UNIT_X;
    Quat4f quat;
    quat.setLookAt(direction, UNIT_Y, xAxis, yAxis, zAxis);
    REQUIRE_THAT( direction.dist( quat.rotateVector(UNIT_Z, vecHas) ), Catch::Matchers::WithinAbs(0.0f, Quat4f::allowed_deviation) );
    REQUIRE(direction == vecHas);

    if( DEBUG_MODE ) {
        std::cout << "quat0 " << quat << std::endl;
        std::cout << "exp0 "  << direction << ", len " << direction.length() << std::endl;
        std::cout << "has0 "  << vecHas  << ", len " << vecHas.length() << std::endl;

        std::cout << std::endl << "LookAt #02" << std::endl;
    }
    (direction = ONE).normalize();
    quat.setLookAt(direction, UNIT_Y, xAxis, yAxis, zAxis);
    if( DEBUG_MODE ) {
        std::cout << "direction " << direction << std::endl;
        std::cout << "quat0.0 " << quat << std::endl;
    }
    quat.rotateVector(UNIT_Z, vecHas);
    if( DEBUG_MODE ) {
        std::cout << "quat0.1 " << quat << std::endl;
        std::cout << "xAxis " << xAxis << ", len " << xAxis.length() << std::endl;
        std::cout << "yAxis " << yAxis << ", len " << yAxis.length() << std::endl;
        std::cout << "zAxis " << zAxis << ", len " << zAxis.length() << std::endl;
        std::cout << "exp0 "  << direction << ", len " << direction.length() << std::endl;
        std::cout << "has0 "  << vecHas  << ", len " << vecHas.length() << std::endl;
    }
    // REQUIRE(0f, VectorUtil.distance(direction, quat.rotateVector(vecHas, 0, UNIT_Z, 0)), Quat4f.ALLOWED_DEVIANCE);
    REQUIRE_THAT( direction.dist(vecHas), Catch::Matchers::WithinAbs(0.0f, Quat4f::allowed_deviation) );
    REQUIRE(direction == vecHas);

    if( DEBUG_MODE ) std::cout << "LookAt #03" << std::endl;
    direction.set(-1, 2, -1).normalize();
    quat.setLookAt(direction, UNIT_Y, xAxis, yAxis, zAxis);
    if( DEBUG_MODE ) std::cout << "quat0 " << quat << std::endl;
    quat.rotateVector(UNIT_Z, vecHas);
    if( DEBUG_MODE ) {
        std::cout << "xAxis " << xAxis << ", len " << xAxis.length() << std::endl;
        std::cout << "yAxis " << yAxis << ", len " << yAxis.length() << std::endl;
        std::cout << "zAxis " << zAxis << ", len " << zAxis.length() << std::endl;
        std::cout << "exp0 " << direction << ", len " << direction.length() << std::endl;
        std::cout << "has0 " << vecHas << ", len " << vecHas.length() << std::endl;
    }
    // REQUIRE(0f, VectorUtil.distance(direction, quat.rotateVector(vecHas, 0, UNIT_Z, 0)), Quat4f.ALLOWED_DEVIANCE);
    REQUIRE_THAT( direction.dist(vecHas), Catch::Matchers::WithinAbs(0.0f, Quat4f::allowed_deviation) );
    if( !direction.equals(vecHas) ) {
        std::cout << "Deviation: " << direction << ", " << vecHas << ": " << ( direction - vecHas ) << ", dist " << direction.dist(vecHas) << std::endl;
    }
    // REQUIRE(direction == vecHas);
}

