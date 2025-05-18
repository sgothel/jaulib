/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020-2024 Gothel Software e.K.
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
#include <cassert>
#include <cinttypes>
#include <cstring>
#include <cmath>

#include <jau/test/catch2_ext.hpp>

#include <jau/float_math.hpp>
#include <jau/basic_types.hpp>

using namespace jau;

static const float MACH_EPSILON = jau::machineEpsilon<float>();
static const double MACH_EPSILON_DOUBLE = jau::machineEpsilon<double>();

static const float MIN_VALUE = std::numeric_limits<float>::min();
static const float MAX_VALUE = std::numeric_limits<float>::max();
static const float POSITIVE_INFINITY =  std::numeric_limits<float>::infinity();
static const float NEGATIVE_INFINITY = -std::numeric_limits<float>::infinity();
static const float NaN =  std::numeric_limits<float>::quiet_NaN();

// static const double MIN_VALUE_DBL = std::numeric_limits<double>::min();
// static const double MAX_VALUE_DBL = std::numeric_limits<double>::max();
static const double POSITIVE_INFINITY_DBL =  std::numeric_limits<double>::infinity();
static const double NEGATIVE_INFINITY_DBL = -std::numeric_limits<double>::infinity();
static const double NaN_DBL =  std::numeric_limits<double>::quiet_NaN();

static void testIEC559FloatType() {
    REQUIRE( jau::bit_value_raw( POSITIVE_INFINITY ) == jau::float_iec559_positive_inf_bitval );
    REQUIRE( jau::bit_value_raw( NEGATIVE_INFINITY ) == jau::float_iec559_negative_inf_bitval );
    REQUIRE( jau::bit_value_raw( NaN )               == jau::float_iec559_nan_bitval );

    REQUIRE( POSITIVE_INFINITY == jau::float_value( jau::float_iec559_positive_inf_bitval ) );
    REQUIRE( NEGATIVE_INFINITY == jau::float_value( jau::float_iec559_negative_inf_bitval ) );
    // by definition not equal: REQUIRE(  NaN == jau::float_value( jau::float_iec559_nan_bitval ) );

    REQUIRE( true == std::isinf( jau::float_value( jau::float_iec559_positive_inf_bitval ) ) );
    REQUIRE( true == std::isinf( jau::float_value( jau::float_iec559_negative_inf_bitval ) ) );
    REQUIRE( true == std::isnan( jau::float_value( jau::float_iec559_nan_bitval ) ) );
}
static void testIEC559DoubleType() {
    REQUIRE( jau::bit_value_raw( POSITIVE_INFINITY_DBL ) == jau::double_iec559_positive_inf_bitval );
    REQUIRE( jau::bit_value_raw( NEGATIVE_INFINITY_DBL ) == jau::double_iec559_negative_inf_bitval );
    REQUIRE( jau::bit_value_raw( NaN_DBL )               == jau::double_iec559_nan_bitval );

    REQUIRE( POSITIVE_INFINITY_DBL == jau::double_value( jau::double_iec559_positive_inf_bitval ) );
    REQUIRE( NEGATIVE_INFINITY_DBL == jau::double_value( jau::double_iec559_negative_inf_bitval ) );
    // by definition not equal: REQUIRE(  NaN_DBL == jau::double_value( jau::double_iec559_nan_bitval ) );

    REQUIRE( true == std::isinf( jau::double_value( jau::double_iec559_positive_inf_bitval ) ) );
    REQUIRE( true == std::isinf( jau::double_value( jau::double_iec559_negative_inf_bitval ) ) );
    REQUIRE( true == std::isnan( jau::double_value( jau::double_iec559_nan_bitval ) ) );
}
template<class T,
         std::enable_if_t<!std::numeric_limits<T>::is_integer, bool> = true>
static void testFloatBits(const T a) {
    typename jau::uint_bytes<sizeof(T)>::type a_bits = jau::bit_value(a);
    const float a2 = jau::float_value(a_bits);
    REQUIRE( a == a2 );
}
template<class T,
         std::enable_if_t<!std::numeric_limits<T>::is_integer, bool> = true>
static void testFloatBits(const T a, const typename jau::uint_bytes<sizeof(T)>::type exp_a_bits) {
    const typename jau::uint_bytes<sizeof(T)>::type a_bits = jau::bit_value(a);
    REQUIRE( exp_a_bits == a_bits );
    const float a2 = jau::float_value(a_bits);
    REQUIRE( a == a2 );
}

TEST_CASE( "Float IEEE 754 (IEC 559) Test 00", "[math][datatype][float][iec559]" ) {
    fprintf(stderr, "float:  ieee 754 / iec559: has %d\n",
            std::numeric_limits<float>::is_iec559);

    fprintf(stderr, "float:  +infinity: has %d, value 0x%X =?= 0x%X: %d\n",
            std::numeric_limits<float>::has_infinity,
            jau::bit_value_raw(POSITIVE_INFINITY),
            jau::float_iec559_positive_inf_bitval,
            jau::bit_value_raw(POSITIVE_INFINITY) == jau::float_iec559_positive_inf_bitval);

    fprintf(stderr, "float:  -infinity: has %d, value 0x%X =?= 0x%X: %d\n",
            std::numeric_limits<float>::has_infinity,
            jau::bit_value_raw(NEGATIVE_INFINITY),
            jau::float_iec559_negative_inf_bitval,
            jau::bit_value_raw(NEGATIVE_INFINITY) == jau::float_iec559_negative_inf_bitval);

    fprintf(stderr, "float:  quiet NAN: has %d, value 0x%X =?= 0x%X: %d\n",
            std::numeric_limits<float>::has_quiet_NaN,
            jau::bit_value_raw(NaN),
            jau::float_iec559_nan_bitval,
            jau::bit_value_raw(NaN) == jau::float_iec559_nan_bitval);

    if( std::numeric_limits<float>::is_iec559 ) {
        testIEC559FloatType();
        testIEC559DoubleType();
    }
    testFloatBits(0.0f);
    testFloatBits(0.0f, 0);
    testFloatBits(MIN_VALUE);
    testFloatBits(MAX_VALUE);
    testFloatBits(std::numeric_limits<float>::lowest());
}

TEST_CASE( "Float Epsilon Test 01", "[math][datatype][float][epsilon]" ) {
    static float epsilon_f0 = std::numeric_limits<float>::epsilon();
    static double epsilon_d0 = std::numeric_limits<double>::epsilon();

    static float epsilon_f1 = MACH_EPSILON;
    static double epsilon_d1 = MACH_EPSILON_DOUBLE;

    float epsilon_fd = epsilon_f1 - epsilon_f0;
    double epsilon_dd = epsilon_d1 - epsilon_d0;

    fprintf(stderr, "std::numeric_limits<float>::epsilon()  : %e\n", epsilon_f0);
    fprintf(stderr, "std::numeric_limits<double>::epsilon() : %le\n", epsilon_d0);
    fprintf(stderr, "jau::machineEpsilon<float>()           : %e\n", epsilon_f1);
    fprintf(stderr, "jau::machineEpsilon<double>()          : %le\n", epsilon_d1);

    fprintf(stderr, "float:  approximation - numeric_limits : %e\n", epsilon_fd);
    fprintf(stderr, "double: approximation - numeric_limits : %le\n", epsilon_dd);

    REQUIRE(jau::equals(epsilon_f1, epsilon_f0, 5, epsilon_f0));
    REQUIRE(jau::equals(epsilon_d1, epsilon_d0, 5, epsilon_d0));
}

//
// Zero
//

static void testZeroWithEpsilon1(const int tstNum, const bool exp, const float a, const float epsilon=std::numeric_limits<float>::epsilon()) {
    const bool zero = jau::is_zero(a, epsilon);
    const float delta = a-0.0f;
    fprintf(stderr, "Zero.WE.%d: a: %f, -> d %f, exp %d, zero %d, epsilon %f\n",
            tstNum, a, delta, exp, zero, epsilon);
    REQUIRE(exp == zero);
}
static void testZeroWithEpsilon1(int i, const float epsilon=std::numeric_limits<float>::epsilon()) {
    testZeroWithEpsilon1(i++, true, 0.0f, epsilon);
    testZeroWithEpsilon1(i++, true, 0.0f-epsilon/2.0f, epsilon);
    testZeroWithEpsilon1(i++, true, 0.0f+epsilon/2.0f, epsilon);
    testZeroWithEpsilon1(i++, true, 0.0f-MIN_VALUE, epsilon);
    testZeroWithEpsilon1(i++, true, 0.0f+MIN_VALUE, epsilon);
    testZeroWithEpsilon1(i++, true, -0.0f, epsilon);
    testZeroWithEpsilon1(i++, true, +0.0f, epsilon);

    testZeroWithEpsilon1(i++, false, 0.0f+epsilon+MIN_VALUE, epsilon);
    testZeroWithEpsilon1(i++, false, 0.0f-epsilon-MIN_VALUE, epsilon);
}
TEST_CASE( "Float Zero Fixed Epsilon Test 10", "[math][datatype][float][epsilon]" ) {
    testZeroWithEpsilon1(100);
}
TEST_CASE( "Float Zero Mach Epsilon Test 11", "[math][datatype][float][epsilon]" ) {
    testZeroWithEpsilon1(200, MACH_EPSILON);
}

static void testZeroNoEpsilon(const int tstNum, const bool exp, const float a) {
    const bool zero = jau::is_zero_raw(a);
    const float delta = a-0.0f;
    fprintf(stderr, "Zero.NE.%d: a: %f, -> d %f, exp %d, zero %d\n",
            tstNum, a, delta, exp, zero);
    REQUIRE(exp == zero);
}
TEST_CASE( "Float Zero No Epsilon Test 12", "[math][datatype][float][epsilon]" ) {
    int i = 400;
    testZeroNoEpsilon(i++, true, 0.0f);
    testZeroNoEpsilon(i++, false, 0.0f-MIN_VALUE);
    testZeroNoEpsilon(i++, false, 0.0f+MIN_VALUE);
    testZeroNoEpsilon(i++, true, -0.0f);
    testZeroNoEpsilon(i++, true, +0.0f);

    testZeroNoEpsilon(i++, false, 0.0f+MIN_VALUE);
    testZeroNoEpsilon(i++, false, 0.0f-MIN_VALUE);
}

//
// Equals
//

static void testEqualsWithEpsilon(const int tstNum, const bool exp, const float a, const float b, const float epsilon=std::numeric_limits<float>::epsilon()) {
    const bool equal =  jau::equals(a, b, epsilon);
    const int comp = jau::compare(a, b, epsilon);
    const bool comp_eq = 0 == comp;
    const float delta = a-b;
    fprintf(stderr, "Equal.WE.%d: a: %f, b: %f -> d %f, exp %d, equal %d, comp %d, epsilon %f\n",
            tstNum, a, b, delta, exp, equal, comp, epsilon);
    REQUIRE(exp == comp_eq);
    REQUIRE(exp == equal);
}

static void testEqualsWithEpsilon(int i, const float epsilon=std::numeric_limits<float>::epsilon()) {
    testEqualsWithEpsilon(i++, true, 0.0f, 0.0f, epsilon);
    testEqualsWithEpsilon(i++, true, 1.0f, 1.0f-epsilon/2.0f, epsilon);
    testEqualsWithEpsilon(i++, true, 1.0f, 1.0f+epsilon/2.0f, epsilon);
    testEqualsWithEpsilon(i++, true, 1.0f, 1.0f-MIN_VALUE, epsilon);
    testEqualsWithEpsilon(i++, true, 1.0f, 1.0f+MIN_VALUE, epsilon);
    testEqualsWithEpsilon(i++, true, MAX_VALUE, MAX_VALUE, epsilon);
    testEqualsWithEpsilon(i++, true, MIN_VALUE, MIN_VALUE, epsilon);
    testEqualsWithEpsilon(i++, true, NEGATIVE_INFINITY, NEGATIVE_INFINITY, epsilon);
    testEqualsWithEpsilon(i++, true, POSITIVE_INFINITY, POSITIVE_INFINITY, epsilon);
    testEqualsWithEpsilon(i++, true, NaN, NaN, epsilon);
    testEqualsWithEpsilon(i++, true, -0.0f, 0.0f, epsilon);
    testEqualsWithEpsilon(i++, true, 0.0f, -0.0f, epsilon);

    testEqualsWithEpsilon(i++, false, 1.0f, 1.0f+epsilon+MIN_VALUE, epsilon);
    testEqualsWithEpsilon(i++, false, 1.0f, 1.0f-epsilon-MIN_VALUE, epsilon);
}

TEST_CASE( "Float Equals Fixed Epsilon Test 20", "[math][datatype][float][epsilon]" ) {
    testEqualsWithEpsilon(100);
}

TEST_CASE( "Float Equals Mach Epsilon Test 21", "[math][datatype][float][epsilon]" ) {
    testEqualsWithEpsilon(200, MACH_EPSILON);
}

static void testEqualsNoEpsilon(const int tstNum, const bool exp, const float a, const float b) {
    const bool equal =  jau::equals_raw(a, b);
    const int comp = jau::compare(a, b);
    const bool comp_eq = 0 == comp;
    const float delta = a-b;
    fprintf(stderr, "Equal.NE.%d: a: %f, b: %f -> d %f, exp %d, equal %d, comp %d\n",
            tstNum, a, b, delta, exp, equal, comp);
    REQUIRE(exp == comp_eq);
    REQUIRE(exp == equal);
}

TEST_CASE( "Float Equals No Epsilon Test 22", "[math][datatype][float][epsilon]" ) {
    int i=0;
    testEqualsNoEpsilon(i++, true, 0.0f, 0.0f);

    testEqualsNoEpsilon(i++, true, MAX_VALUE, MAX_VALUE);
    testEqualsNoEpsilon(i++, true, MIN_VALUE, MIN_VALUE);
    testEqualsNoEpsilon(i++, true, NEGATIVE_INFINITY, NEGATIVE_INFINITY);
    testEqualsNoEpsilon(i++, true, POSITIVE_INFINITY, POSITIVE_INFINITY);
    testEqualsNoEpsilon(i++, true, NaN, NaN);
    testEqualsNoEpsilon(i++, false, -0.0f, 0.0f);
    testEqualsNoEpsilon(i++, false, 0.0f, -0.0f);
}


//
// Compare
//

static void testCompareNoEpsilon(const int tstNum, const int exp, const float a, const float b) {
    const bool equal =  jau::equals_raw(a, b);
    const int comp = jau::compare(a, b);
    const float delta = a-b;
    const uint32_t a_bits = jau::bit_value(a);
    const uint32_t b_bits = jau::bit_value(b);
    const int32_t a_sbits = static_cast<int32_t>(a_bits);
    const int32_t b_sbits = static_cast<int32_t>(b_bits);
    fprintf(stderr, "Comp.NE.%d: a: %f 0x%X %d, b: %f 0x%X %d -> d %f, equal %d, comp: exp %d, has %d\n",
            tstNum, a, a_bits, a_sbits, b, b_bits, b_sbits, delta, equal, exp, comp);
    REQUIRE(exp == comp);
}

TEST_CASE( "Float Compare Zero Epsilon Test 10", "[math][datatype][float][epsilon]" ) {
    int i=0;
    testCompareNoEpsilon(i++, 0, 0.0f, 0.0f);
    testCompareNoEpsilon(i++, 0, MAX_VALUE, MAX_VALUE);
    testCompareNoEpsilon(i++, 0, MIN_VALUE, MIN_VALUE);

    testCompareNoEpsilon(i++,  1,  1.0f,  0.0f);
    testCompareNoEpsilon(i++, -1,  0.0f,  1.0f);
    testCompareNoEpsilon(i++,  1,  0.0f, -1.0f);
    testCompareNoEpsilon(i++, -1, -1.0f,  0.0f);

    testCompareNoEpsilon(i++,  1, MAX_VALUE, MIN_VALUE);
    testCompareNoEpsilon(i++, -1, MIN_VALUE, MAX_VALUE);

    testCompareNoEpsilon(i++, -1, -0.0f, 0.0f);
    testCompareNoEpsilon(i++,  1, 0.0f, -0.0f);

    if( std::numeric_limits<float>::has_infinity ) {
        testCompareNoEpsilon(i++,  0, NEGATIVE_INFINITY, NEGATIVE_INFINITY);
        testCompareNoEpsilon(i++,  0,  POSITIVE_INFINITY,  POSITIVE_INFINITY);
        testCompareNoEpsilon(i++,  1,  POSITIVE_INFINITY, NEGATIVE_INFINITY);
        testCompareNoEpsilon(i++, -1, NEGATIVE_INFINITY,  POSITIVE_INFINITY);
    }

    if( std::numeric_limits<float>::has_quiet_NaN ) {
        testCompareNoEpsilon(i++, 0, NaN, NaN); // NAN
        testCompareNoEpsilon(i++, -1, 0.0f, NaN);
        testCompareNoEpsilon(i++,  1, NaN, 0.0f);
    }
}

static void testCompareWithEpsilon(const int tstNum, const int exp, const float a, const float b, const float epsilon) {
    const bool equal =  jau::equals(a, b, epsilon);
    const int comp = jau::compare(a, b, epsilon);
    const float delta = a-b;
    fprintf(stderr, "Comp.WE.%d: a: %f, b: %f -> d %f, equal %d, comp: exp %d, has %d\n",
            tstNum, a, b, delta, equal, exp, comp);
    REQUIRE(exp == comp);
}

static void test05CompareWithEpsilon(int i, const float epsilon) {
    testCompareWithEpsilon(i++, 0, 0.0f, 0.0f, epsilon);
    testCompareWithEpsilon(i++, 0, 1.0f, 1.0f-epsilon/2.0f, epsilon);
    testCompareWithEpsilon(i++, 0, 1.0f, 1.0f+epsilon/2.0f, epsilon);
    testCompareWithEpsilon(i++, 0, 1.0f, 1.0f-MIN_VALUE, epsilon);
    testCompareWithEpsilon(i++, 0, 1.0f, 1.0f+MIN_VALUE, epsilon);
    testCompareWithEpsilon(i++, 0, MAX_VALUE, MAX_VALUE, epsilon);
    testCompareWithEpsilon(i++, 0, MIN_VALUE, MIN_VALUE, epsilon);

    testCompareWithEpsilon(i++,  1,  1.0f,  0.0f, epsilon);
    testCompareWithEpsilon(i++, -1,  0.0f,  1.0f, epsilon);
    testCompareWithEpsilon(i++,  1,  0.0f, -1.0f, epsilon);
    testCompareWithEpsilon(i++, -1, -1.0f,  0.0f, epsilon);

    testCompareWithEpsilon(i++,  1, MAX_VALUE, MIN_VALUE, epsilon);
    testCompareWithEpsilon(i++, -1, MIN_VALUE, MAX_VALUE, epsilon);

    testCompareWithEpsilon(i++,  0, -0.0f, 0.0f, epsilon);
    testCompareWithEpsilon(i++,  0, 0.0f, -0.0f, epsilon);

    if( std::numeric_limits<float>::has_infinity ) {
        testCompareWithEpsilon(i++,  0, NEGATIVE_INFINITY, NEGATIVE_INFINITY, epsilon);
        testCompareWithEpsilon(i++,  0,  POSITIVE_INFINITY,  POSITIVE_INFINITY, epsilon);
        testCompareWithEpsilon(i++,  1,  POSITIVE_INFINITY, NEGATIVE_INFINITY, epsilon);
        testCompareWithEpsilon(i++, -1, NEGATIVE_INFINITY,  POSITIVE_INFINITY, epsilon);
    }

    if( std::numeric_limits<float>::has_quiet_NaN ) {
        testCompareWithEpsilon(i++, 0, NaN, NaN, epsilon); // NAN
        testCompareWithEpsilon(i++, -1, 0.0f, NaN, epsilon);
        testCompareWithEpsilon(i++,  1, NaN, 0.0f, epsilon);
    }
}

TEST_CASE( "Float Compare Fixed Epsilon Test 20", "[math][datatype][float][epsilon]" ) {
    test05CompareWithEpsilon(100, std::numeric_limits<float>::epsilon());
}

TEST_CASE( "Float Compare Mac Epsilon Test 21", "[math][datatype][float][epsilon]" ) {
    test05CompareWithEpsilon(200, MACH_EPSILON);
}

