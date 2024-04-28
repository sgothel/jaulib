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

    // m.glMatrixMode(GLMatrixFunc.GL_PROJECTION);
    // m.glOrthof(0, 10, 0, 10, 1, -1);
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

#if 0

/**
 * PMVMatrix vs Mat4f::mapObjToWin(), both w/ separate P + Mv
 *
 * Both using same Mat4f::mapObjToWin().
 */
@Test
public void test01PMVMatrixToMatrix4f2() {
    final Vec3f winA00 = new Vec3f();
    final Vec3f winA01 = new Vec3f();
    final Vec3f winA10 = new Vec3f();
    final Vec3f winA11 = new Vec3f();
    final Vec3f winB00 = new Vec3f();
    final Vec3f winB01 = new Vec3f();
    final Vec3f winB10 = new Vec3f();
    final Vec3f winB11 = new Vec3f();

    final PMVMatrix m = new PMVMatrix();
    final Matrix4f mat4Mv = new Matrix4f();
    final Matrix4f mat4P = new Matrix4f();
    final float[] mat4Mv_f16 = new float[16];
    final float[] mat4P_f16 = new float[16];

    m.glGetFloatv(GLMatrixFunc.GL_MODELVIEW_MATRIX, mat4Mv_f16, 0);
    m.glGetFloatv(GLMatrixFunc.GL_PROJECTION_MATRIX, mat4P_f16, 0);
    std::cout << FloatUtil.matrixToString(null, "mat4Mv", "%10.5f", mat4Mv_f16, 0, 4, 4, false /* rowMajorOrder */));
    std::cout << FloatUtil.matrixToString(null, "mat4P ", "%10.5f", mat4P_f16,  0, 4, 4, false /* rowMajorOrder */));
    mat4Mv.load( m.getMat(GLMatrixFunc.GL_MODELVIEW_MATRIX) );
    mat4P.load( m.getMat(GLMatrixFunc.GL_PROJECTION_MATRIX) );
    Assert.assertEquals(new Matrix4f(mat4Mv_f16), mat4Mv);
    Assert.assertEquals(new Matrix4f(mat4P_f16), mat4P);
    Assert.assertEquals(mat4Mv, m.getMv());
    Assert.assertEquals(mat4P, m.getP());

    m.mapObjToWin(new Vec3f(1f, 0f, 0f), viewport, winA00);
    std::cout << "A.0.0 - Project 1,0 -->" + winA00);
    Mat4f::mapObjToWin(new Vec3f(1f, 0f, 0f), mat4Mv, mat4P, viewport, winB00);
    std::cout << "B.0.0 - Project 1,0 -->" + winB00);

    m.mapObjToWin(new Vec3f(0f, 0f, 0f), viewport, winA01);
    std::cout << "A.0.1 - Project 0,0 -->" + winA01);
    Mat4f::mapObjToWin(new Vec3f(0f, 0f, 0f), mat4Mv, mat4P, viewport, winB01);
    std::cout << "B.0.1 - Project 0,0 -->" + winB01);

    m.glMatrixMode(GLMatrixFunc.GL_PROJECTION);
    m.glOrthof(0, 10, 0, 10, 1, -1);
    std::cout << "MATRIX - Ortho 0,0,10,10 - Locate the origin in the bottom left and scale");
    std::cout << m);
    m.glGetFloatv(GLMatrixFunc.GL_MODELVIEW_MATRIX, mat4Mv_f16, 0);
    m.glGetFloatv(GLMatrixFunc.GL_PROJECTION_MATRIX, mat4P_f16, 0);
    std::cout << FloatUtil.matrixToString(null, "mat4Mv", "%10.5f", mat4Mv_f16, 0, 4, 4, false /* rowMajorOrder */));
    std::cout << FloatUtil.matrixToString(null, "mat4P ", "%10.5f", mat4P_f16,  0, 4, 4, false /* rowMajorOrder */));
    mat4Mv.load( m.getMat(GLMatrixFunc.GL_MODELVIEW_MATRIX) );
    mat4P.load( m.getMat(GLMatrixFunc.GL_PROJECTION_MATRIX) );
    Assert.assertEquals(new Matrix4f(mat4Mv_f16), mat4Mv);
    Assert.assertEquals(new Matrix4f(mat4P_f16), mat4P);
    Assert.assertEquals(mat4Mv, m.getMv());
    Assert.assertEquals(mat4P, m.getP());

    m.mapObjToWin(new Vec3f(1f, 0f, 0f), viewport, winA10);
    std::cout << "A.1.0 - Project 1,0 -->" +winA10);
    Mat4f::mapObjToWin(new Vec3f(1f, 0f, 0f), mat4Mv, mat4P, viewport, winB10);
    std::cout << "B.1.0 - Project 1,0 -->" +winB10);

    m.mapObjToWin(new Vec3f(0f, 0f, 0f), viewport, winA11);
    std::cout << "A.1.1 - Project 0,0 -->" +winA11);
    Mat4f::mapObjToWin(new Vec3f(0f, 0f, 0f), mat4Mv, mat4P, viewport, winB11);
    std::cout << "B.1.1 - Project 0,0 -->" +winB11);

    Assert.assertEquals("A/B 0.0 Project 1,0 failure", winB00, winA00);
    Assert.assertEquals("A/B 0.1 Project 0,0 failure", winB01, winA01);
    Assert.assertEquals("A/B 1.0 Project 1,0 failure", winB10, winA10);
    Assert.assertEquals("A/B 1.1 Project 0,0 failure", winB11, winA11);
}

/**
 * GLU ProjectFloat vs Mat4f::mapObjToWin(), both w/ separate P + Mv
 */
@Test
public void test03GLUToMatrix4f2() {
    final float[] winA00 = new float[4];
    final float[] winA01 = new float[4];
    final float[] winA10 = new float[4];
    final float[] winA11 = new float[4];
    final Vec3f winB00 = new Vec3f();
    final Vec3f winB01 = new Vec3f();
    final Vec3f winB10 = new Vec3f();
    final Vec3f winB11 = new Vec3f();

    final PMVMatrix m = new PMVMatrix();
    final Matrix4f mat4Mv = new Matrix4f();
    final Matrix4f mat4P = new Matrix4f();
    final float[] mat4Mv_f16 = new float[16];
    final float[] mat4P_f16 = new float[16];
    final GLU glu = new GLU();

    m.glGetFloatv(GLMatrixFunc.GL_MODELVIEW_MATRIX, mat4Mv_f16, 0);
    m.glGetFloatv(GLMatrixFunc.GL_PROJECTION_MATRIX, mat4P_f16, 0);
    std::cout << FloatUtil.matrixToString(null, "mat4Mv", "%10.5f", mat4Mv_f16, 0, 4, 4, false /* rowMajorOrder */));
    std::cout << FloatUtil.matrixToString(null, "mat4P ", "%10.5f", mat4P_f16,  0, 4, 4, false /* rowMajorOrder */));
    mat4Mv.load( m.getMv() );
    mat4P.load( m.getP() );

    glu.gluProject(1f, 0f, 0f, mat4Mv_f16, 0, mat4P_f16, 0, viewport_i4, 0, winA00, 0);
    std::cout << "A.0.0 - Project 1,0 -->" + winA00);
    Mat4f::mapObjToWin(new Vec3f(1f, 0f, 0f), mat4Mv, mat4P, viewport, winB00);
    std::cout << "B.0.0 - Project 1,0 -->" + winB00);

    glu.gluProject(0f, 0f, 0f, mat4Mv_f16, 0, mat4P_f16, 0, viewport_i4, 0, winA01, 0);
    std::cout << "A.0.1 - Project 0,0 -->" + winA01);
    Mat4f::mapObjToWin(new Vec3f(0f, 0f, 0f), mat4Mv, mat4P, viewport, winB01);
    std::cout << "B.0.1 - Project 0,0 -->" + winB01);

    m.glMatrixMode(GLMatrixFunc.GL_PROJECTION);
    m.glOrthof(0, 10, 0, 10, 1, -1);
    std::cout << "MATRIX - Ortho 0,0,10,10 - Locate the origin in the bottom left and scale");
    std::cout << m);
    m.glGetFloatv(GLMatrixFunc.GL_MODELVIEW_MATRIX, mat4Mv_f16, 0);
    m.glGetFloatv(GLMatrixFunc.GL_PROJECTION_MATRIX, mat4P_f16, 0);
    std::cout << FloatUtil.matrixToString(null, "mat4Mv", "%10.5f", mat4Mv_f16, 0, 4, 4, false /* rowMajorOrder */));
    std::cout << FloatUtil.matrixToString(null, "mat4P ", "%10.5f", mat4P_f16,  0, 4, 4, false /* rowMajorOrder */));
    mat4Mv.load( m.getMv() );
    mat4P.load( m.getP() );

    glu.gluProject(1f, 0f, 0f, mat4Mv_f16, 0, mat4P_f16, 0, viewport_i4, 0, winA10, 0);
    std::cout << "A.1.0 - Project 1,0 -->" +winA10);
    Mat4f::mapObjToWin(new Vec3f(1f, 0f, 0f), mat4Mv, mat4P, viewport, winB10);
    std::cout << "B.1.0 - Project 1,0 -->" +winB10);

    glu.gluProject(0f, 0f, 0f, mat4Mv_f16, 0, mat4P_f16, 0, viewport_i4, 0, winA11, 0);
    std::cout << "A.1.1 - Project 0,0 -->" +winA11);
    Mat4f::mapObjToWin(new Vec3f(0f, 0f, 0f), mat4Mv, mat4P, viewport, winB11);
    std::cout << "B.1.1 - Project 0,0 -->" +winB11);

    Assert.assertEquals("A/B 0.0 Project 1,0 failure", winB00, new Vec3f(winA00));
    Assert.assertEquals("A/B 0.1 Project 0,0 failure", winB01, new Vec3f(winA01));
    Assert.assertEquals("A/B 1.0 Project 1,0 failure", winB10, new Vec3f(winA10));
    Assert.assertEquals("A/B 1.1 Project 0,0 failure", winB11, new Vec3f(winA11));
}

/**
 * GLU ProjectDouble vs Mat4f::mapObjToWin(), both w/ separate P + Mv
 */
@Test
public void test04GLUDoubleToMatrix4f2() {
    final double[] winA00 = new double[3];
    final double[] winA01 = new double[3];
    final double[] winA10 = new double[3];
    final double[] winA11 = new double[3];
    final Vec3f winB00 = new Vec3f();
    final Vec3f winB01 = new Vec3f();
    final Vec3f winB10 = new Vec3f();
    final Vec3f winB11 = new Vec3f();

    final PMVMatrix m = new PMVMatrix();
    final Matrix4f mat4Mv = new Matrix4f();
    final Matrix4f mat4P = new Matrix4f();
    final float[] mat4Mv_f16 = new float[16];
    final float[] mat4P_f16 = new float[16];
    final ProjectDouble glu = new ProjectDouble();

    m.glGetFloatv(GLMatrixFunc.GL_MODELVIEW_MATRIX, mat4Mv_f16, 0);
    m.glGetFloatv(GLMatrixFunc.GL_PROJECTION_MATRIX, mat4P_f16, 0);
    std::cout << FloatUtil.matrixToString(null, "mat4Mv", "%10.5f", mat4Mv_f16, 0, 4, 4, false /* rowMajorOrder */));
    std::cout << FloatUtil.matrixToString(null, "mat4P ", "%10.5f", mat4P_f16,  0, 4, 4, false /* rowMajorOrder */));
    mat4Mv.load( m.getMv() );
    mat4P.load( m.getP() );
    double[] mat4Mv_d16 = Buffers.getDoubleArray(mat4Mv_f16, 0, null, 0, -1);
    double[] mat4P_d16 = Buffers.getDoubleArray(mat4P_f16, 0, null, 0, -1);

    glu.gluProject(1f, 0f, 0f, mat4Mv_d16, 0, mat4P_d16, 0, viewport_i4, 0, winA00, 0);
    std::cout << "A.0.0 - Project 1,0 -->" + Arrays.toString(winA00));
    Mat4f::mapObjToWin(new Vec3f(1f, 0f, 0f), mat4Mv, mat4P, viewport, winB00);
    std::cout << "B.0.0 - Project 1,0 -->" + winB00);

    glu.gluProject(0f, 0f, 0f, mat4Mv_d16, 0, mat4P_d16, 0, viewport_i4, 0, winA01, 0);
    std::cout << "A.0.1 - Project 0,0 -->" + Arrays.toString(winA01));
    Mat4f::mapObjToWin(new Vec3f(0f, 0f, 0f), mat4Mv, mat4P, viewport, winB01);
    std::cout << "B.0.1 - Project 0,0 -->" + winB01);

    m.glMatrixMode(GLMatrixFunc.GL_PROJECTION);
    m.glOrthof(0, 10, 0, 10, 1, -1);
    std::cout << "MATRIX - Ortho 0,0,10,10 - Locate the origin in the bottom left and scale");
    std::cout << m);
    m.glGetFloatv(GLMatrixFunc.GL_MODELVIEW_MATRIX, mat4Mv_f16, 0);
    m.glGetFloatv(GLMatrixFunc.GL_PROJECTION_MATRIX, mat4P_f16, 0);
    std::cout << FloatUtil.matrixToString(null, "mat4Mv", "%10.5f", mat4Mv_f16, 0, 4, 4, false /* rowMajorOrder */));
    std::cout << FloatUtil.matrixToString(null, "mat4P ", "%10.5f", mat4P_f16,  0, 4, 4, false /* rowMajorOrder */));
    mat4Mv.load( m.getMv() );
    mat4P.load( m.getP() );
    mat4Mv_d16 = Buffers.getDoubleArray(mat4Mv_f16, 0, null, 0, -1);
    mat4P_d16 = Buffers.getDoubleArray(mat4P_f16, 0, null, 0, -1);

    glu.gluProject(1f, 0f, 0f, mat4Mv_d16, 0, mat4P_d16, 0, viewport_i4, 0, winA10, 0);
    std::cout << "A.1.0 - Project 1,0 -->" +Arrays.toString(winA10));
    Mat4f::mapObjToWin(new Vec3f(1f, 0f, 0f), mat4Mv, mat4P, viewport, winB10);
    std::cout << "B.1.0 - Project 1,0 -->" +winB10);

    glu.gluProject(0f, 0f, 0f, mat4Mv_d16, 0, mat4P_d16, 0, viewport_i4, 0, winA11, 0);
    std::cout << "A.1.1 - Project 0,0 -->" +Arrays.toString(winA11));
    Mat4f::mapObjToWin(new Vec3f(0f, 0f, 0f), mat4Mv, mat4P, viewport, winB11);
    std::cout << "B.1.1 - Project 0,0 -->" +winB11);

    final float[] tmp = new float[3];
    double[] d_winBxx = Buffers.getDoubleArray(winB00.get(tmp), 0, null, 0, -1);
    Assert.assertArrayEquals("A/B 0.0 Project 1,0 failure", d_winBxx, winA00, epsilon);
    d_winBxx = Buffers.getDoubleArray(winB01.get(tmp), 0, null, 0, -1);
    Assert.assertArrayEquals("A/B 0.1 Project 0,0 failure", d_winBxx, winA01, epsilon);
    d_winBxx = Buffers.getDoubleArray(winB10.get(tmp), 0, null, 0, -1);
    Assert.assertArrayEquals("A/B 1.0 Project 1,0 failure", d_winBxx, winA10, epsilon);
    d_winBxx = Buffers.getDoubleArray(winB11.get(tmp), 0, null, 0, -1);
    Assert.assertArrayEquals("A/B 1.1 Project 0,0 failure", d_winBxx, winA11, epsilon);
}

#endif
