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

using namespace jau;
using namespace jau::int_literals;

TEST_CASE( "Int Math Bench 01a", "[abs][benchmark][arithmetic][math]" ) {
    BENCHMARK("jau::abs Benchmark") {
        REQUIRE( 1 == jau::abs( 1) );
        REQUIRE( 1 == jau::abs(-1) );
        REQUIRE( 1_i64 == jau::abs( 1_i64) );
        REQUIRE( 1_i64 == jau::abs(-1_i64) );
        REQUIRE( std::numeric_limits<int64_t>::max()  == jau::abs( std::numeric_limits<int64_t>::max() ) );
        REQUIRE( INT32_MAX  == jau::abs( INT32_MAX ) );
    };
}
TEST_CASE( "Int Math Bench 01b", "[ct_abs][benchmark][arithmetic][math]" ) {
    BENCHMARK("jau::ct_abs Benchmark") {
        REQUIRE( 1 == jau::ct_abs( 1) );
        REQUIRE( 1 == jau::ct_abs(-1) );
        REQUIRE( 1_i64 == jau::ct_abs( 1_i64) );
        REQUIRE( 1_i64 == jau::ct_abs(-1_i64) );
        REQUIRE( std::numeric_limits<int64_t>::max()  == jau::ct_abs( std::numeric_limits<int64_t>::max() ) );
        // REQUIRE( std::numeric_limits<int64_t>::max()  == jau::ct_abs( std::numeric_limits<int64_t>::min() ) ); // UB
        REQUIRE( INT32_MAX  == jau::ct_abs( INT32_MAX ) );
        // REQUIRE( INT32_MAX  == jau::ct_abs( INT32_MIN ) ); // UB
    };
}
TEST_CASE( "Int Math Bench 01c", "[abs][benchmark][arithmetic][math]" ) {
    BENCHMARK("std::abs Benchmark") {
        REQUIRE( 1 == std::abs( 1) );
        REQUIRE( 1 == std::abs(-1) );
        REQUIRE( 1_i64 == std::abs( 1_i64) );
        REQUIRE( 1_i64 == std::abs(-1_i64) );
        REQUIRE( std::numeric_limits<int64_t>::max()  == std::abs( std::numeric_limits<int64_t>::max() ) );
        REQUIRE( INT32_MAX  == std::abs( INT32_MAX ) );
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
TEST_CASE( "Int Math Bench 03a", "[ct_min][ct_max][benchmark][arithmetic][math]" ) {
    BENCHMARK("Min2Max2 Benchmark") {
        REQUIRE(         0  == jau::ct_min( 0, INT32_MAX ) );
        REQUIRE( INT32_MAX  == jau::ct_max( 0, INT32_MAX ) );
        REQUIRE( INT32_MAX-1== jau::ct_min( INT32_MAX-1, INT32_MAX ) );
        REQUIRE( INT32_MAX  == jau::ct_max( INT32_MAX-1, INT32_MAX ) );
        REQUIRE( INT32_MIN+1  == jau::ct_min( 0, INT32_MIN+1 ) ); // limitation: `MIN <= x - y <= MAX`
        REQUIRE(         0  == jau::ct_max( 0, INT32_MIN+1 ) );   // limitation: `MIN <= x - y <= MAX`
        REQUIRE( INT32_MIN  == jau::ct_min( INT32_MIN+1, INT32_MIN ) );
        REQUIRE( INT32_MIN+1== jau::ct_max( INT32_MIN+1, INT32_MIN ) );
    };
}
