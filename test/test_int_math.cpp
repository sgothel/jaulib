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

TEST_CASE( "Int Math Test 00", "[int][type]" ) {
    {
        REQUIRE(  1 == jau::sign( 1) );
        REQUIRE(  0 == jau::sign( 0) );
        REQUIRE( -1 == jau::sign(-1) );
        REQUIRE(  1 == jau::sign( 1_i64) );
        REQUIRE(  0 == jau::sign( 0_i64) );
        REQUIRE( -1 == jau::sign(-1_i64) );
        REQUIRE(  1 == jau::sign( 1_u64) );
        REQUIRE(  0 == jau::sign( 0_u64) );

        REQUIRE(  1 == jau::sign( std::numeric_limits<uint64_t>::max() ) );
        REQUIRE(  1 == jau::sign( std::numeric_limits<int64_t>::max() ) );
        REQUIRE( -1 == jau::sign( std::numeric_limits<int64_t>::min() ) );
    }
    {
        // abs unsigned integral
        REQUIRE( 1_u64 == jau::abs( 1_u64) );
        REQUIRE(  std::numeric_limits<uint64_t>::max() == jau::abs( std::numeric_limits<uint64_t>::max() ) );

        // abs float
        REQUIRE( 1.0f == jau::abs( 1.0f ) );
        REQUIRE( 1.0f == jau::abs(-1.0f ) );
        REQUIRE(  std::numeric_limits<float>::max()  == jau::abs(  std::numeric_limits<float>::max() ) );
        REQUIRE(  std::numeric_limits<float>::min()  == jau::abs(  std::numeric_limits<float>::min() ) );
        REQUIRE(  std::numeric_limits<float>::max()  == jau::abs( -std::numeric_limits<float>::max() ) );
    }
    {
        // abs signed integral
        REQUIRE( 1 == jau::abs( 1) );
        REQUIRE( 1 == jau::abs(-1) );
        REQUIRE( 1 == jau::abs2( 1) );
        REQUIRE( 1 == jau::abs2(-1) );
        REQUIRE( 1_i64 == jau::abs( 1_i64) );
        REQUIRE( 1_i64 == jau::abs(-1_i64) );
        REQUIRE( 1_i64 == jau::abs2( 1_i64) );
        REQUIRE( 1_i64 == jau::abs2(-1_i64) );
        REQUIRE( std::numeric_limits<int64_t>::max()  == jau::abs( std::numeric_limits<int64_t>::max() ) );
        REQUIRE( std::numeric_limits<int64_t>::max()  == jau::abs( std::numeric_limits<int64_t>::min() ) );
        REQUIRE( std::numeric_limits<int64_t>::max()  == jau::abs2( std::numeric_limits<int64_t>::max() ) );
        REQUIRE( std::numeric_limits<int64_t>::min()  == jau::abs2( std::numeric_limits<int64_t>::min() ) );
        REQUIRE( std::numeric_limits<int64_t>::max()  == std::abs( std::numeric_limits<int64_t>::max() ) );
        REQUIRE( std::numeric_limits<int64_t>::min()  == std::abs( std::numeric_limits<int64_t>::min() ) );
        REQUIRE( INT32_MAX  == jau::abs( INT32_MIN ) );
        REQUIRE( INT32_MIN  == jau::abs2( INT32_MIN ) );
        REQUIRE( INT32_MIN  == std::abs( INT32_MIN ) );
    }
    {
        REQUIRE( true == is_power_of_2(  2_u32 ) );
        REQUIRE( true == is_power_of_2(  4_u32 ) );
        REQUIRE( true == is_power_of_2( 64_u32 ) );
    }
    {
        REQUIRE( 0 == high_bit( 0b00000000U ) );
        REQUIRE( 1 == high_bit( 0b00000001U ) );
        REQUIRE( 2 == high_bit( 0b00000010U ) );
        REQUIRE( 2 == high_bit( 0b00000011U ) );
        REQUIRE( 8 == high_bit( 0b11000011U ) );

        REQUIRE( 64 == high_bit( 0b1100001111000011110000111100001111000011110000111100001111000011UL ) );
    }
    {
        {
            {
                uint64_t a = 1, b = 2, r;
                REQUIRE( false == jau::add_overflow(a, b, r) );
                REQUIRE( a + b == r );
            }
            {
                uint64_t a = std::numeric_limits<uint64_t>::max()-2, b = 2, r;
                REQUIRE( false == jau::add_overflow(a, b, r) );
                REQUIRE( a + b == r );
            }
            {
                uint64_t a = std::numeric_limits<uint64_t>::max(), b = 2, r;
                REQUIRE( true == jau::add_overflow(a, b, r) );
            }
        }
        {
            {
                uint64_t a = 2, b = 1, r;
                REQUIRE( false == jau::sub_overflow(a, b, r) );
                REQUIRE( a - b == r );
            }
            {
                uint64_t a = std::numeric_limits<uint64_t>::min()+2, b = 2, r;
                REQUIRE( false == jau::sub_overflow(a, b, r) );
                REQUIRE( a - b == r );
            }
            {
                uint64_t a = 1, b = 2, r;
                REQUIRE( true == jau::sub_overflow(a, b, r) );
            }
            {
                uint64_t a = std::numeric_limits<uint64_t>::min(), b = 2, r;
                REQUIRE( true == jau::sub_overflow(a, b, r) );
            }
        }
        {
            {
                uint64_t a = 1, b = 2, r;
                REQUIRE( false == jau::mul_overflow(a, b, r) );
                REQUIRE( a * b == r );
            }
            {
                uint64_t a = std::numeric_limits<uint64_t>::max()/2, b = 2, r;
                REQUIRE( false == jau::mul_overflow(a, b, r) );
                REQUIRE( a * b == r );
            }
            {
                uint64_t a = std::numeric_limits<uint64_t>::max(), b = 2, r;
                REQUIRE( true == jau::mul_overflow(a, b, r) );
            }
        }
    }
    {
        {
            {
                int64_t a = 1, b = 2, r;
                REQUIRE( false == jau::add_overflow(a, b, r) );
                REQUIRE( a + b == r );
            }
            {
                int64_t a = std::numeric_limits<int64_t>::max()-2, b = 2, r;
                REQUIRE( false == jau::add_overflow(a, b, r) );
                REQUIRE( a + b == r );
            }
            {
                int64_t a = std::numeric_limits<int64_t>::max(), b = 2, r;
                REQUIRE( true == jau::add_overflow(a, b, r) );
            }
        }
        {
            {
                int64_t a = 2, b = 1, r;
                REQUIRE( false == jau::sub_overflow(a, b, r) );
                REQUIRE( a - b == r );
            }
            {
                int64_t a = std::numeric_limits<int64_t>::min()+2, b = 2, r;
                REQUIRE( false == jau::sub_overflow(a, b, r) );
                REQUIRE( a - b == r );
            }
            {
                int64_t a = 1, b = 2, r;
                REQUIRE( false == jau::sub_overflow(a, b, r) );
                REQUIRE( a - b == r );
            }
            {
                uint64_t a = std::numeric_limits<uint64_t>::min(), b = 2, r;
                REQUIRE( true == jau::sub_overflow(a, b, r) );
            }
        }
        {
            {
                int64_t a = 1, b = 2, r;
                REQUIRE( false == jau::mul_overflow(a, b, r) );
                REQUIRE( a * b == r );
            }
            {
                int64_t a = std::numeric_limits<int64_t>::max()/2, b = 2, r;
                REQUIRE( false == jau::mul_overflow(a, b, r) );
                REQUIRE( a * b == r );
            }
            {
                int64_t a = std::numeric_limits<int64_t>::max(), b = 2, r;
                REQUIRE( true == jau::mul_overflow(a, b, r) );
            }
        }
    }
}
