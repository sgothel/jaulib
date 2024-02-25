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

#define JAU_INT_MATH_EXPERIMENTAL 1

#include <jau/int_math.hpp>

using namespace jau;
using namespace jau::int_literals;

TEST_CASE( "Int Math Bench 01a", "[abs][benchmark][arithmetic][math]" ) {
    BENCHMARK("abs Benchmark") {
        REQUIRE( 1 == jau::abs( 1) );
        REQUIRE( 1 == jau::abs(-1) );
        REQUIRE( 1_i64 == jau::abs( 1_i64) );
        REQUIRE( 1_i64 == jau::abs(-1_i64) );
        REQUIRE( std::numeric_limits<int64_t>::max()  == jau::abs( std::numeric_limits<int64_t>::max() ) );
        REQUIRE( std::numeric_limits<int64_t>::max()  == jau::abs( std::numeric_limits<int64_t>::min() ) );
        REQUIRE( INT32_MAX  == jau::abs( INT32_MIN ) );
    };
}
TEST_CASE( "Int Math Bench 01b", "[abs2][benchmark][arithmetic][math]" ) {
    BENCHMARK("abs2 Benchmark") {
        REQUIRE( 1 == jau::abs2( 1) );
        REQUIRE( 1 == jau::abs2(-1) );
        REQUIRE( 1_i64 == jau::abs2( 1_i64) );
        REQUIRE( 1_i64 == jau::abs2(-1_i64) );
        REQUIRE( std::numeric_limits<int64_t>::max()  == jau::abs2( std::numeric_limits<int64_t>::max() ) );
        REQUIRE( std::numeric_limits<int64_t>::min()  == jau::abs2( std::numeric_limits<int64_t>::min() ) );
        REQUIRE( INT32_MIN  == jau::abs2( INT32_MIN ) );
    };
}

TEST_CASE( "Int Math Bench 02a", "[min][max][benchmark][arithmetic][math]" ) {
    BENCHMARK("MinMax Benchmark") {
        REQUIRE(         0  == jau::min( 0, INT32_MAX ) );
        REQUIRE( INT32_MAX  == jau::max( 0, INT32_MAX ) );
        REQUIRE( INT32_MAX-1== jau::min( INT32_MAX-1, INT32_MAX ) );
        REQUIRE( INT32_MAX  == jau::max( INT32_MAX-1, INT32_MAX ) );
        REQUIRE( INT32_MIN  == jau::min( 0, INT32_MIN ) );
        REQUIRE(         0  == jau::max( 0, INT32_MIN ) );
        REQUIRE( INT32_MIN  == jau::min( INT32_MIN+1, INT32_MIN ) );
        REQUIRE( INT32_MIN+1== jau::max( INT32_MIN+1, INT32_MIN ) );
    };
}
TEST_CASE( "Int Math Bench 03a", "[min2][max2][benchmark][arithmetic][math]" ) {
    BENCHMARK("Min2Max2 Benchmark") {
        REQUIRE(         0  == jau::min2( 0, INT32_MAX ) );
        REQUIRE( INT32_MAX  == jau::max2( 0, INT32_MAX ) );
        REQUIRE( INT32_MAX-1== jau::min2( INT32_MAX-1, INT32_MAX ) );
        REQUIRE( INT32_MAX  == jau::max2( INT32_MAX-1, INT32_MAX ) );
        REQUIRE( INT32_MIN+1  == jau::min2( 0, INT32_MIN+1 ) ); // limitation: `MIN <= x - y <= MAX`
        REQUIRE(         0  == jau::max2( 0, INT32_MIN+1 ) );   // limitation: `MIN <= x - y <= MAX`
        REQUIRE( INT32_MIN  == jau::min2( INT32_MIN+1, INT32_MIN ) );
        REQUIRE( INT32_MIN+1== jau::max2( INT32_MIN+1, INT32_MIN ) );
    };
}
