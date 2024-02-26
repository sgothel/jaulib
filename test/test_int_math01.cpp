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

TEST_CASE( "Int Math Test 00", "[sign][arithmetic][math]" ) {
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
TEST_CASE( "Int Math Test 01", "[round][align][arithmetic][math]" ) {
    {
        REQUIRE(  0_u32 == jau::round_up( 0_u32, 1_u32) );
        REQUIRE(  1_u32 == jau::round_up( 1_u32, 1_u32) );
        REQUIRE(  2_u32 == jau::round_up( 2_u32, 1_u32) );

        REQUIRE(  0_u32 == jau::round_up( 0_u32, 8_u32) );
        REQUIRE(  8_u32 == jau::round_up( 1_u32, 8_u32) );
        REQUIRE(  8_u32 == jau::round_up( 7_u32, 8_u32) );
        REQUIRE(  8_u32 == jau::round_up( 8_u32, 8_u32) );
        REQUIRE( 16_u32 == jau::round_up( 9_u32, 8_u32) );
    }
    {
        REQUIRE(  0_u32 == jau::round_down( 0_u32, 1_u32) );
        REQUIRE(  1_u32 == jau::round_down( 1_u32, 1_u32) );
        REQUIRE(  2_u32 == jau::round_down( 2_u32, 1_u32) );

        REQUIRE(  0_u32 == jau::round_down( 0_u32, 8_u32) );
        REQUIRE(  0_u32 == jau::round_down( 1_u32, 8_u32) );
        REQUIRE(  0_u32 == jau::round_down( 7_u32, 8_u32) );
        REQUIRE(  8_u32 == jau::round_down( 8_u32, 8_u32) );
        REQUIRE(  8_u32 == jau::round_down( 9_u32, 8_u32) );
    }
}
TEST_CASE( "Int Math Test 02", "[abs][arithmetic][math]" ) {
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
        REQUIRE( 1 == jau::ct_abs( 1) );
        REQUIRE( 1 == jau::ct_abs(-1) );
        REQUIRE( 1_i64 == jau::abs( 1_i64) );
        REQUIRE( 1_i64 == jau::abs(-1_i64) );
        REQUIRE( 1_i64 == jau::ct_abs( 1_i64) );
        REQUIRE( 1_i64 == jau::ct_abs(-1_i64) );
        REQUIRE( std::numeric_limits<int64_t>::max()  == jau::abs( std::numeric_limits<int64_t>::max() ) );
        REQUIRE( std::numeric_limits<int64_t>::max()  == jau::abs( std::numeric_limits<int64_t>::min() ) );
        REQUIRE( std::numeric_limits<int64_t>::max()  == jau::ct_abs( std::numeric_limits<int64_t>::max() ) );
        REQUIRE( std::numeric_limits<int64_t>::min()  == jau::ct_abs( std::numeric_limits<int64_t>::min() ) );
        REQUIRE( std::numeric_limits<int64_t>::max()  == std::abs( std::numeric_limits<int64_t>::max() ) );
        REQUIRE( std::numeric_limits<int64_t>::min()  == std::abs( std::numeric_limits<int64_t>::min() ) );
        REQUIRE( INT32_MAX  == jau::abs( INT32_MIN ) );
        REQUIRE( INT32_MIN  == jau::ct_abs( INT32_MIN ) );
        REQUIRE( INT32_MIN  == std::abs( INT32_MIN ) );
    }
}

TEST_CASE( "Int Math Test 03a", "[min][max][clip][arithmetic][math]" ) {
    REQUIRE(         0  == jau::min( 0, INT32_MAX ) );
    REQUIRE( INT32_MAX  == jau::max( 0, INT32_MAX ) );
    REQUIRE( INT32_MAX-1== jau::min( INT32_MAX-1, INT32_MAX ) );
    REQUIRE( INT32_MAX  == jau::max( INT32_MAX-1, INT32_MAX ) );
    REQUIRE( INT32_MIN  == jau::min( 0, INT32_MIN ) );
    REQUIRE(         0  == jau::max( 0, INT32_MIN ) );
    REQUIRE( INT32_MIN  == jau::min( INT32_MIN+1, INT32_MIN ) );
    REQUIRE( INT32_MIN+1== jau::max( INT32_MIN+1, INT32_MIN ) );
    REQUIRE(         0  == jau::clamp( 0, -10, 10 ) );
    REQUIRE(       -10  == jau::clamp( INT32_MIN, -10, 10 ) );
    REQUIRE(        10  == jau::clamp( INT32_MAX, -10, 10 ) );
}

TEST_CASE( "Int Math Test 03b", "[ct_min][ct_max][clip2][arithmetic][math]" ) {
    REQUIRE(         0  == jau::ct_min( 0, INT32_MAX ) );
    REQUIRE( INT32_MAX  == jau::ct_max( 0, INT32_MAX ) );
    REQUIRE( INT32_MAX-1== jau::ct_min( INT32_MAX-1, INT32_MAX ) );
    REQUIRE( INT32_MAX  == jau::ct_max( INT32_MAX-1, INT32_MAX ) );
    REQUIRE( INT32_MIN+1  == jau::ct_min( 0, INT32_MIN+1 ) ); // limitation: `MIN <= x - y <= MAX`
    REQUIRE(         0  == jau::ct_max( 0, INT32_MIN+1 ) );   // limitation: `MIN <= x - y <= MAX`
    REQUIRE( INT32_MIN  == jau::ct_min( INT32_MIN+1, INT32_MIN ) );
    REQUIRE( INT32_MIN+1== jau::ct_max( INT32_MIN+1, INT32_MIN ) );
    REQUIRE(         0  == jau::ct_clamp( 0, -10, 10 ) );
    REQUIRE(       -10  == jau::ct_clamp( INT32_MIN+11, -10, 10 ) ); // limitation: `MIN <= x - y <= MAX`
    REQUIRE(        10  == jau::ct_clamp( INT32_MAX-11, -10, 10 ) ); // limitation: `MIN <= x - y <= MAX`
}


TEST_CASE( "Int Math Test 10", "[bits][arithmetic][math]" ) {
    {
        REQUIRE(  0b0000000000000000U == ct_masked_merge( 0b0000000000000000U, 0b0000000000000000U, 0b0000000000000000U ) );
        REQUIRE(  0b1100000000000011U == ct_masked_merge( 0b1111111100000000U, 0b1100000000000000U, 0b0000000000000011U ) );
        REQUIRE(               64_u32 == ct_masked_merge( 0b1111111111111111U,              64_u32, 256_u32 ) );
        REQUIRE(              256_u32 == ct_masked_merge( 0b0000000000000000U,              64_u32, 256_u32 ) );
    }
    {
        REQUIRE( true == is_power_of_2(  2_u32 ) );
        REQUIRE( true == is_power_of_2(  4_u32 ) );
        REQUIRE( true == is_power_of_2( 64_u32 ) );
    }
    {
        REQUIRE( 0 == round_to_power_of_2(0) );
        REQUIRE( 1 == round_to_power_of_2(1) );
        REQUIRE( 2 == round_to_power_of_2(2) );
        REQUIRE( 4 == round_to_power_of_2(3) );
        REQUIRE(64 == round_to_power_of_2(63) );
    }
    {
        REQUIRE(  0 == ct_bit_count( 0b00000000000000000000000000000000UL ) );
        REQUIRE(  1 == ct_bit_count( 0b00000000000000000000000000000001UL ) );
        REQUIRE(  1 == ct_bit_count( 0b10000000000000000000000000000000UL ) );
        REQUIRE( 16 == ct_bit_count( 0b10101010101010101010101010101010UL ) );
        REQUIRE( 16 == ct_bit_count( 0b01010101010101010101010101010101UL ) );
        REQUIRE( 32 == ct_bit_count( 0b11111111111111111111111111111111UL ) );
    }
    {
        REQUIRE( 0 == high_bit( 0b00000000U ) );
        REQUIRE( 1 == high_bit( 0b00000001U ) );
        REQUIRE( 2 == high_bit( 0b00000010U ) );
        REQUIRE( 2 == high_bit( 0b00000011U ) );
        REQUIRE( 8 == high_bit( 0b11000011U ) );

        REQUIRE( 64 == high_bit( 0b1100001111000011110000111100001111000011110000111100001111000011UL ) );
    }
}
TEST_CASE( "Int Math Test 20", "[add][sub][overflow][arithmetic][math]" ) {
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
    }
}
TEST_CASE( "Int Math Test 21", "[mul][overflow][arithmetic][math]" ) {
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
