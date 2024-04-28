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

// static const float EPSILON = std::numeric_limits<float>::epsilon();

// Simple 10 x 10 view port
static Recti viewport(0,0,10,10);
// static int viewport_i4[] = { 0, 0, 10, 10 };

/**
 * PMVMatrix w/ separate P + Mv vs Mat4f::mapObjToWin() w/ single PMv
 *
 * Both using same Mat4f::mapObjToWin(..).
 */
TEST_CASE( "Test 01 PMVMatrixToMatrix4f", "[mat4f][linear_algebra][math]" ) {
    Vec3f winA00, winA01, winA10, winA11;
    Vec3f winB00, winB01, winB10, winB11;

    PMVMat4f m;
    Mat4f mat4PMv;
    m.getMulPMv(mat4PMv);
    // std::cout << mat4PMv.toString(null, "mat4PMv", "%10.5f"));

    m.mapObjToWin(Vec3f(1, 0, 0), viewport, winA00); // separate P + Mv
    std::cout << "A.0.0 - Project 1,0 -->" << winA00 << std::endl;
    Mat4f::mapObjToWin(Vec3f(1, 0, 0), mat4PMv, viewport, winB00); // single PMv
    std::cout << "B.0.0 - Project 1,0 -->" << winB00 << std::endl;

    m.mapObjToWin(Vec3f(0, 0, 0), viewport, winA01);
    std::cout << "A.0.1 - Project 0,0 -->" << winA01 << std::endl;
    Mat4f::mapObjToWin(Vec3f(0, 0, 0), mat4PMv, viewport, winB01);
    std::cout << "B.0.1 - Project 0,0 -->" << winB01 << std::endl;

    m.orthoP(0, 10, 0, 10, 1, -1);
    std::cout << "MATRIX - Ortho 0,0,10,10 - Locate the origin in the bottom left and scale" << std::endl;
    std::cout << m << std::endl;
    m.getMulPMv(mat4PMv);
    std::cout << mat4PMv.toString("mat4PMv", "%10.5f")  << std::endl;

    m.mapObjToWin(Vec3f(1, 0, 0), viewport, winA10);
    std::cout << "A.1.0 - Project 1,0 -->" << winA10 << std::endl;
    Mat4f::mapObjToWin(Vec3f(1, 0, 0), mat4PMv, viewport, winB10);
    std::cout << "B.1.0 - Project 1,0 -->" << winB10 << std::endl;

    m.mapObjToWin(Vec3f(0, 0, 0), viewport, winA11);
    std::cout << "A.1.1 - Project 0,0 -->" << winA11 << std::endl;
    Mat4f::mapObjToWin(Vec3f(0, 0, 0), mat4PMv, viewport, winB11);
    std::cout << "B.1.1 - Project 0,0 -->" << winB11 << std::endl;

    REQUIRE_MSG("A/B 0.0 Project 1,0 failure", winB00 == winA00);
    REQUIRE_MSG("A/B 0.1 Project 0,0 failure", winB01 == winA01);
    REQUIRE_MSG("A/B 1.0 Project 1,0 failure", winB10 == winA10);
    REQUIRE_MSG("A/B 1.1 Project 0,0 failure", winB11 == winA11);
}

/**
 * PMVMatrix vs Mat4f::mapObjToWin(), both w/ separate P + Mv
 *
 * Both using same Mat4f::mapObjToWin().
 */
TEST_CASE( "Test 02 PMVMatrixToMatrix4f 2", "[mat4f][linear_algebra][math]" ) {
    Vec3f winA00, winA01, winA10, winA11;
    Vec3f winB00, winB01, winB10, winB11;

    PMVMat4f m;
    Mat4f mat4Mv, mat4P;

    float mat4Mv_f16[16];
    float mat4P_f16[16];

    m.getMv().get(mat4Mv_f16);
    m.getP().get(mat4P_f16);

    std::cout << m.getMv().toString("mat4Mv") << std::endl;
    std::cout << m.getP().toString("mat4P") << std::endl;
    mat4Mv.load( mat4Mv_f16 );
    mat4P.load( mat4P_f16 );
    REQUIRE( Mat4f(mat4Mv_f16) == mat4Mv);
    REQUIRE( Mat4f(mat4P_f16) == mat4P);
    REQUIRE( m.getMv() == mat4Mv);
    REQUIRE( m.getP() == mat4P);

    m.mapObjToWin(Vec3f(1, 0, 0), viewport, winA00);
    std::cout << "A.0.0 - Project 1,0 -->" << winA00 << std::endl;
    Mat4f::mapObjToWin(Vec3f(1, 0, 0), mat4Mv, mat4P, viewport, winB00);
    std::cout << "B.0.0 - Project 1,0 -->" << winB00 << std::endl;

    m.mapObjToWin(Vec3f(0, 0, 0), viewport, winA01);
    std::cout << "A.0.1 - Project 0,0 -->" << winA01 << std::endl;
    Mat4f::mapObjToWin(Vec3f(0, 0, 0), mat4Mv, mat4P, viewport, winB01);
    std::cout << "B.0.1 - Project 0,0 -->" << winB01 << std::endl;

    m.orthoP(0, 10, 0, 10, 1, -1);
    std::cout << "MATRIX - Ortho 0,0,10,10 - Locate the origin in the bottom left and scale" << std::endl;
    std::cout << m << std::endl;
    m.getMv().get(mat4Mv_f16);
    m.getP().get(mat4P_f16);
    std::cout << m.getMv().toString("mat4Mv") << std::endl;
    std::cout << m.getP().toString("mat4P") << std::endl;
    mat4Mv.load( mat4Mv_f16 );
    mat4P.load( mat4P_f16 );
    REQUIRE( Mat4f(mat4Mv_f16) == mat4Mv);
    REQUIRE( Mat4f(mat4P_f16) == mat4P);
    REQUIRE( m.getMv() == mat4Mv);
    REQUIRE( m.getP() == mat4P);

    m.mapObjToWin(Vec3f(1, 0, 0), viewport, winA10);
    std::cout << "A.1.0 - Project 1,0 -->" << winA10 << std::endl;
    Mat4f::mapObjToWin(Vec3f(1, 0, 0), mat4Mv, mat4P, viewport, winB10);
    std::cout << "B.1.0 - Project 1,0 -->" << winB10 << std::endl;

    m.mapObjToWin(Vec3f(0, 0, 0), viewport, winA11);
    std::cout << "A.1.1 - Project 0,0 -->" << winA11 << std::endl;
    Mat4f::mapObjToWin(Vec3f(0, 0, 0), mat4Mv, mat4P, viewport, winB11);
    std::cout << "B.1.1 - Project 0,0 -->" << winB11 << std::endl;

    REQUIRE_MSG("A/B 0.0 Project 1,0 failure", winB00 == winA00);
    REQUIRE_MSG("A/B 0.1 Project 0,0 failure", winB01 == winA01);
    REQUIRE_MSG("A/B 1.0 Project 1,0 failure", winB10 == winA10);
    REQUIRE_MSG("A/B 1.1 Project 0,0 failure", winB11 == winA11);
}
