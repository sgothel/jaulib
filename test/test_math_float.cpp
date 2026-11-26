/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020-2026 Gothel Software e.K.
 *
 * SPDX-License-Identifier: MIT
 *
 * This Source Code Form is subject to the terms of the MIT License
 * If a copy of the MIT was not distributed with this file,
 * you can obtain one at https://opensource.org/license/mit/.
 */
#include <cassert>
#include <cstring>
#include <cmath>
#include <limits>

#include <jau/test/catch2_ext.hpp>

#include <jau/float_types.hpp>
#include <jau/float_math.hpp>
#include <jau/basic_types.hpp>
#include <jau/type_concepts.hpp>

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
    using namespace jau::iec559;
    REQUIRE( bit_value_raw( POSITIVE_INFINITY ) == binary32_positive_inf_bitval );
    REQUIRE( is_inf( POSITIVE_INFINITY ) == true );
    REQUIRE( is_positive_inf( POSITIVE_INFINITY ) == true );

    REQUIRE( bit_value_raw( NEGATIVE_INFINITY ) == binary32_negative_inf_bitval );
    REQUIRE( is_inf( NEGATIVE_INFINITY ) == true );
    REQUIRE( is_negative_inf( NEGATIVE_INFINITY ) == true );

    REQUIRE( bit_value_raw( NaN )               == binary32_nan_bitval );
    REQUIRE( is_nan( NaN ) == true );

    REQUIRE( POSITIVE_INFINITY == float32_value<float>( binary32_positive_inf_bitval ) );
    REQUIRE( NEGATIVE_INFINITY == float32_value<float>( binary32_negative_inf_bitval ) );
    // by definition not equal: REQUIRE(  NaN == jau::float_value( binary32_nan_bitval ) );

    REQUIRE( true == std::isinf( float32_value<float>( binary32_positive_inf_bitval ) ) );
    REQUIRE( true == std::isinf( float32_value<float>( binary32_negative_inf_bitval ) ) );
    REQUIRE( true == is_inf( float32_value<float>( binary32_positive_inf_bitval ) ) );
    REQUIRE( true == is_positive_inf( float32_value<float>( binary32_positive_inf_bitval ) ) );
    REQUIRE( true == is_inf( float32_value<float>( binary32_negative_inf_bitval ) ) );
    REQUIRE( true == is_negative_inf( float32_value<float>( binary32_negative_inf_bitval ) ) );

    REQUIRE( true == std::isnan( float32_value<float>( binary32_nan_bitval ) ) );
    REQUIRE( true == is_nan( float32_value<float>( binary32_nan_bitval ) ) );
}
static void testIEC559DoubleType() {
    using namespace jau::iec559;
    REQUIRE( bit_value_raw( POSITIVE_INFINITY_DBL ) == binary64_positive_inf_bitval );
    REQUIRE( is_inf( POSITIVE_INFINITY_DBL ) == true );
    REQUIRE( is_positive_inf( POSITIVE_INFINITY_DBL ) == true );

    REQUIRE( bit_value_raw( NEGATIVE_INFINITY_DBL ) == binary64_negative_inf_bitval );
    REQUIRE( is_inf( NEGATIVE_INFINITY_DBL ) == true );
    REQUIRE( is_negative_inf( NEGATIVE_INFINITY_DBL ) == true );

    REQUIRE( bit_value_raw( NaN_DBL )               == binary64_nan_bitval );
    REQUIRE( is_nan( NaN_DBL ) == true );

    REQUIRE( POSITIVE_INFINITY_DBL == float64_value<double>( binary64_positive_inf_bitval ) );
    REQUIRE( NEGATIVE_INFINITY_DBL == float64_value<double>( binary64_negative_inf_bitval ) );
    // by definition not equal: REQUIRE(  NaN_DBL == jau::double_value( binary64_nan_bitval ) );

    REQUIRE( true == std::isinf( float64_value<double>( binary64_positive_inf_bitval ) ) );
    REQUIRE( true == std::isinf( float64_value<double>( binary64_negative_inf_bitval ) ) );
    REQUIRE( true == is_inf( float64_value<double>( binary64_positive_inf_bitval ) ) );
    REQUIRE( true == is_positive_inf( float64_value<double>( binary64_positive_inf_bitval ) ) );
    REQUIRE( true == is_inf( float64_value<double>( binary64_negative_inf_bitval ) ) );
    REQUIRE( true == is_negative_inf( float64_value<double>( binary64_negative_inf_bitval ) ) );

    REQUIRE( true == std::isnan( float64_value<double>( binary64_nan_bitval ) ) );
    REQUIRE( true == is_nan( float64_value<double>( binary64_nan_bitval ) ) );
}
template<class T,
         std::enable_if_t<!std::numeric_limits<T>::is_integer, bool> = true>
static void testFloatBits(const T a) {
    typename jau::uint_bytes<sizeof(T)>::type a_bits = jau::iec559::bit_value(a);
    const float a2 = jau::iec559::float32_value<float>(a_bits);
    REQUIRE( a == a2 );
}
template<class T,
         std::enable_if_t<!std::numeric_limits<T>::is_integer, bool> = true>
static void testFloatBits(const T a, const typename jau::uint_bytes<sizeof(T)>::type exp_a_bits) {
    const typename jau::uint_bytes<sizeof(T)>::type a_bits = jau::iec559::bit_value(a);
    REQUIRE( exp_a_bits == a_bits );
    const float a2 = jau::iec559::float32_value<float>(a_bits);
    REQUIRE( a == a2 );
}

TEST_CASE( "Float IEEE 754 (IEC 559) Test 00", "[math][datatype][float][iec559]" ) {
    fprintf(stderr, "float:  ieee 754 / iec559: has %d\n",
            std::numeric_limits<float>::is_iec559);

    fprintf(stderr, "float:  +infinity: has %d, value 0x%X =?= 0x%X: %d\n",
            std::numeric_limits<float>::has_infinity,
            jau::iec559::bit_value_raw(POSITIVE_INFINITY),
            jau::iec559::binary32_positive_inf_bitval,
            jau::iec559::bit_value_raw(POSITIVE_INFINITY) == jau::iec559::binary32_positive_inf_bitval);

    fprintf(stderr, "float:  -infinity: has %d, value 0x%X =?= 0x%X: %d\n",
            std::numeric_limits<float>::has_infinity,
            jau::iec559::bit_value_raw(NEGATIVE_INFINITY),
            jau::iec559::binary32_negative_inf_bitval,
            jau::iec559::bit_value_raw(NEGATIVE_INFINITY) == jau::iec559::binary32_negative_inf_bitval);

    fprintf(stderr, "float:  quiet NAN: has %d, value 0x%X =?= 0x%X: %d\n",
            std::numeric_limits<float>::has_quiet_NaN,
            jau::iec559::bit_value_raw(NaN),
            jau::iec559::binary32_nan_bitval,
            jau::iec559::bit_value_raw(NaN) == jau::iec559::binary32_nan_bitval);

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

template<jau::req::floating_point T>
static void checkIEC559Float(T value0) {
    const int sign0 = value0 >= 0.0 ? 1 : -1;
    const int sign1 = jau::iec559::sign(value0);
    const int expval0 = std::ilogb(value0);
    const int expval1 = jau::iec559::exponent(value0);

    // scalbn(arg, exp) == arg × pow(FLT_RADIX, exp) -> ldexp(arg, exp) == arg x pow(2, exp)
    const T significand0 = std::abs(value0) / std::scalbn(1.0, expval0);

    const T significand1 = jau::iec559::significand(value0);
    const T value1 = sign1 * significand1 * std::pow(2.0, expval1);

    fprintf(stderr, "checkIEC559Float:%zu: v %f = %f, sign[0: %d, 1: %d], exp[0: %d, 1: %d], significant[0: %f, 1: %f]\n",
        sizeof(T)<<3, value0, value1, sign0, sign1, expval0, expval1, significand0, significand1);

    REQUIRE(sign0 == sign1);
    REQUIRE(expval0 == expval1);
    REQUIRE_THAT( significand1, Catch::Matchers::WithinRel(significand0, std::numeric_limits<T>::epsilon()));
    REQUIRE_THAT( value1, Catch::Matchers::WithinRel(value0, std::numeric_limits<T>::epsilon()));
}

TEST_CASE( "Float IEEE 754 (IEC 559) Test 01", "[math][datatype][float][iec559]" ) {
    checkIEC559Float<double>(1.2);
    checkIEC559Float<float>(1.2f);

    checkIEC559Float<double>(12.3);
    checkIEC559Float<float>(12.3f);

    checkIEC559Float<double>(-12.3);
    checkIEC559Float<float>(-12.3f);

    checkIEC559Float<double>(123.45);
    checkIEC559Float<float>(123.45f);
    checkIEC559Float<double>(-123.45);
    checkIEC559Float<float>(-123.45f);
}

//
// Zero
//

TEST_CASE( "Float Epsilon Test 10", "[math][datatype][float][epsilon]" ) {
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
TEST_CASE( "Float Zero Fixed Epsilon Test 11", "[math][datatype][float][epsilon]" ) {
    testZeroWithEpsilon1(100);
}
TEST_CASE( "Float Zero Mach Epsilon Test 12", "[math][datatype][float][epsilon]" ) {
    testZeroWithEpsilon1(200, MACH_EPSILON);
}

static void testZeroNoEpsilon(const int tstNum, const bool exp, const float a) {
    const bool zero = jau::is_zero_raw(a);
    const float delta = a-0.0f;
    fprintf(stderr, "Zero.NE.%d: a: %f, -> d %f, exp %d, zero %d\n",
            tstNum, a, delta, exp, zero);
    REQUIRE(exp == zero);
}
TEST_CASE( "Float Zero No Epsilon Test 13", "[math][datatype][float][epsilon]" ) {
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
    const uint32_t a_bits = jau::iec559::bit_value(a);
    const uint32_t b_bits = jau::iec559::bit_value(b);
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
