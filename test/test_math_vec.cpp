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
#include <jau/math/quaternion.hpp>
#include <jau/math/aabbox2f.hpp>
#include <jau/math/aabbox3f.hpp>
#include <jau/math/mat4f.hpp>
#include <jau/math/recti.hpp>
#include <jau/math/math_error.hpp>

using namespace jau;
using namespace jau::math;

// static const Quaternion QUAT_IDENT = Quaternion(0.0f, 0.0f, 0.0f, 1.0f);

static const Vec2f v2ZERO       = Vec2f();
// static const vec2f v2ONE        = vec2f (  1.0f,  1.0f );
// static const vec2f v2NEG_ONE    = vec2f ( -1.0f, -1.0f );
// static const vec2f v2UNIT_X     = vec2f (  1.0f,  0.0f );
// static const vec2f v2UNIT_X_NEG = vec2f ( -1.0f,  0.0f );
// static const vec2f v2UNIT_Y     = vec2f (  1.0f,  0.0f );
// static const vec2f v2UNIT_Z     = vec2f (  1.0f,  0.0f );

static const Vec3f v3ZERO       = Vec3f();
// static const vec3f v3ONE        = vec3f (  1.0f,  1.0f,  1.0f );
// static const vec3f v3NEG_ONE    = vec3f ( -1.0f, -1.0f, -1.0f );
static const Vec3f v3UNIT_X     = Vec3f (  1.0f,  0.0f,  0.0f );
// static const vec3f v3UNIT_X_NEG = vec3f ( -1.0f,  0.0f,  0.0f );
// static const vec3f v3UNIT_Y     = vec3f (  1.0f,  0.0f,  0.0f );
// static const vec3f v3UNIT_Z     = vec3f (  1.0f,  0.0f,  0.0f );

TEST_CASE( "Math Vec Test 00", "[vec][linear_algebra][math]" ) {
    REQUIRE( Vec2f() == v2ZERO );
    REQUIRE( Vec3f() == v3ZERO );
}

TEST_CASE( "Math Vec Normalize Test 01", "[vec][linear_algebra][math]" ) {
    const Vec3f v0 = v3UNIT_X;
    Vec3f v1 = Vec3f(1, 2, 3);
    REQUIRE( true == jau::equals( 1.0f, v0.length() ) );
    REQUIRE( 1 < v1.length() );
    REQUIRE( true == jau::equals( 1.0f, v1.normalize().length() ) );
}
