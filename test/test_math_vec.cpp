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

using namespace jau;
using namespace jau::math;

template<class T, class U>
void test_vec(std::ostream& out, const char* prefix) {
    out << "Test: " << std::string(prefix) << ", sizeof(U) = " << sizeof(U) << std::endl;
    T a( U(1) ), b( U(2) );
    out << "- a: " << a << ", len = " << std::to_string(a.length()) << ", len(normal(a)) = " << std::to_string(T(a).normalize().length()) << std::endl;
    out << "- b: " << b << ", len = " << std::to_string(b.length()) << ", len(normal(b)) = " << std::to_string(T(b).normalize().length()) << std::endl;

    REQUIRE( T(U(2)) == a * U(2) );
    REQUIRE( T(U(1)) == b / U(2) );

    a.normalize();
    b.normalize();

    REQUIRE( jau::equals(U(1), a.length() ) );
    REQUIRE( jau::equals(U(1), b.length() ) );
}

template<class T>
void dump_align_props(std::ostream& out, const char* prefix) {
    out << std::string(prefix) << "{size " << std::to_string( sizeof(T) ) << ", alignment " << std::to_string( alignof(T) )
               << " }" << std::endl;
}

TEST_CASE( "Math Vec Test 00", "[vec][linear_algebra][math]" ) {
    static_assert(alignof(int) == alignof(Vec2i));
    static_assert(alignof(float) == alignof(Vec2f));
    static_assert(alignof(float) == alignof(Vec3f));
    static_assert(alignof(float) == alignof(Vec4f));
    static_assert(alignof(float) == alignof(Mat4f));

    dump_align_props<int>(std::cout, "int");
    dump_align_props<float>(std::cout, "float");
    dump_align_props<Vec2i>(std::cout, "Vec2i");
    dump_align_props<Vec2f>(std::cout, "Vec2f");
    dump_align_props<Vec3f>(std::cout, "Vec3f");
    dump_align_props<Vec4f>(std::cout, "Vec4f");
    dump_align_props<Mat4f>(std::cout, "Mat4f");

    std::cout << "A v2 " << Vec2f(1, 2) << std::endl;
    std::cout << "A v3 " << Vec3f(1, 2, 3) << std::endl;
    std::cout << "A v4 " << Vec4f(1, 2, 3, 4) << std::endl;
    {
        const float mf[] = {  1.0f,  2.0f,  3.0f,  4.0f, // column 0
                              5.0f,  6.0f,  7.0f,  8.0f, // column 1
                              9.0f, 10.0f, 11.0f, 12.0f, // column 2
                             13.0f, 14.0f, 15.0f, 16.0f  // column 3
                           };
        std::cout << "A mat4 " << Mat4f(mf) << std::endl;
    }

    REQUIRE( Vec2f() == Vec2f(0, 0) );
    REQUIRE( Vec3f() == Vec3f(0, 0, 0) );
    REQUIRE( 0.0f == Vec2f().length() );
    REQUIRE( 0.0f == Vec3f().length() );

    test_vec<Vector2I<int>, int>(std::cout, "Vector2I<int>");
    test_vec<Vector2I<long>, long>(std::cout, "Vector2I<long>");

    test_vec<Vector2F<float>, float>(std::cout, "Vector2F<float>");
    test_vec<Vector2F<double>, double>(std::cout, "Vector2F<double>");

    test_vec<Vector3F<float>, float>(std::cout, "Vector3F<float>");
    test_vec<Vector3F<double>, double>(std::cout, "Vector3F<double>");

    test_vec<Vector4F<float>, float>(std::cout, "Vector4F<float>");
    test_vec<Vector4F<double>, double>(std::cout, "Vector4F<double>");
}

TEST_CASE( "Math Vec Normalize Test 01", "[vec][linear_algebra][math]" ) {
    const Vec3f v0(1, 0, 0);
    Vec3f v1(1, 2, 3);
    REQUIRE( true == jau::equals( 1.0f, v0.length() ) );
    REQUIRE( 1 < v1.length() );
    REQUIRE( true == jau::equals( 1.0f, v1.normalize().length() ) );
}

TEST_CASE( "Math Vec Angle Test 02", "[vec][linear_algebra][math]" ) {
    // test 0 deg
    {
        std::cout << "Test 0-deg, UNIT_X vecs" << std::endl;
        Vec3f v0(1, 0, 0);
        Vec3f v1(1, 0, 0);
        std::cout << "v0 " << v0 << std::endl;
        std::cout << "v1 " << v1 << std::endl;

        const float a0_v0_v1 = v0.angle(v1);
        std::cout << "a0(v0, v1) = " << a0_v0_v1 << " rad, " << jau::rad_to_adeg(a0_v0_v1) << " deg, via dot, acos" << std::endl;
        REQUIRE(true == jau::equals(0.0f, a0_v0_v1));
    }
    // test 0 deg
    {
        std::cout << "Test 0-deg, free vecs" << std::endl;
        const Vec3f v0(0.14f, 0.07f, 0.0f);
        const Vec3f v1(0.33f, 0.07f, 0.0f);
        const Vec3f v0_1 = v1 - v0;
        std::cout << "v0 " << v0 << std::endl;
        std::cout << "v1 " << v1 << std::endl;
        std::cout << "v0_1 " << v0_1 << std::endl;

        const float a0_x_v0_1 = Vec3f(1, 0, 0).angle(v0_1);
        std::cout << "a0(X, v0_1) = " << a0_x_v0_1 << " rad, " << jau::rad_to_adeg(a0_x_v0_1) << " deg, via dot, acos" << std::endl;
        REQUIRE(true == jau::equals(0.0f, a0_x_v0_1));
    }
    // test 180 deg
    {
        std::cout << "Test 180-deg, free vecs" << std::endl;
        const Vec3f v0(0.33f, 0.07f, 0.0f);
        const Vec3f v1(0.14f, 0.07f, 0.0f);
        const Vec3f v0_1 = v1 - v0;
        std::cout << "v0 " << v0 << std::endl;
        std::cout << "v1 " << v1 << std::endl;
        std::cout << "v0_1 " << v0_1 << std::endl;

        const float a0_x_v0_1 = Vec3f(1, 0, 0).angle(v0_1);
        std::cout << "a0(X, v0_1) = " << a0_x_v0_1 << " rad, " << jau::rad_to_adeg(a0_x_v0_1) << " deg, via dot, acos" << std::endl;
        REQUIRE(true == jau::equals((float)M_PI, a0_x_v0_1));
    }
    // test 90 deg
    {
        std::cout << "Test 90-deg, UNIT_X, UNIT_Y vecs" << std::endl;
        const Vec3f v0(1, 0, 0);
        const Vec3f v1(0, 1, 0);
        std::cout << "v0 " << v0 << std::endl;
        std::cout << "v1 " << v1 << std::endl;
        const float a0_v0_v1 = v0.angle(v1);
        std::cout << "a0(v0, v1) = " << a0_v0_v1 << " rad, " << jau::rad_to_adeg(a0_v0_v1) << " deg, via dot, acos" << std::endl;
        REQUIRE(true == jau::equals((float)M_PI_2, a0_v0_v1));
    }
    // test 180 deg
    {
        std::cout << "Test 180-deg, UNIT_X, UNIT_X_NEG vecs" << std::endl;
        const Vec3f v0(1, 0, 0);
        const Vec3f v1(-1, 0, 0);
        std::cout << "v0 " << v0 << std::endl;
        std::cout << "v1 " << v1 << std::endl;
        const float a0_v0_v1 = v0.angle(v1);
        std::cout << "a0(v0, v1) = " << a0_v0_v1 << " rad, " << jau::rad_to_adeg(a0_v0_v1) << " deg, via dot, acos" << std::endl;
        REQUIRE(true == jau::equals((float)M_PI, a0_v0_v1));
    }
}

