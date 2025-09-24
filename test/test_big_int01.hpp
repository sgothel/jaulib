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
#include <cstring>
#include <iostream>

#include <jau/test/catch2_ext.hpp>

#include <jau/int_types.hpp>
#include <jau/mp/big_int.hpp>

using namespace jau;
using namespace jau::mp;
using namespace jau::int_literals;

TEST_CASE( "MP Big Int Test 00", "[big_int_t][arithmetic][math]" ) {
    std::cout << "big_int mp_word_bits " << std::to_string( mp_word_bits ) << std::endl;

    {
        BigInt a = 1_u64, b = 2_u64, c = 3_u64, d = 4_u64, e = 256_u64;
        std::cout << "big_int 1:: " << a.to_dec_string(true) << std::endl;
        std::cout << "big_int 1:: " << a.to_hex_string(true) << std::endl;
        std::cout << "big_int 2:: " << b.to_dec_string(true) << std::endl;
        std::cout << "big_int 3:: " << c.to_dec_string(true) << std::endl;
        std::cout << "big_int 256:: " << e.to_dec_string(true) << std::endl;
        std::cout << "big_int 256:: " << e.to_hex_string(true) << std::endl;
        REQUIRE( 1 == a.bits() );
        REQUIRE( 1 == a.bytes() );
        REQUIRE( 1 == a.sign() );
        {
            BigInt r = a + b;
            std::cout << "big_int 1+2:: " << r.to_dec_string(true) << std::endl;
            REQUIRE( c == r );
        }
        {
            BigInt r = b * b;
            std::cout << "big_int 2*2:: " << r.to_dec_string(true) << std::endl;
            REQUIRE( d == r );
        }
        {
            BigInt r = b / b; // NOLINT(misc-redundant-expression)
            std::cout << "big_int 2/2:: " << r.to_dec_string(true) << std::endl;
            REQUIRE( a == r );
        }

        REQUIRE( 2 == b.bits() );
        REQUIRE( 1 == b.bytes() );
        REQUIRE( 1 == b.sign() );

        REQUIRE( 2 == c.bits() );
        REQUIRE( 1 == c.bytes() );
        REQUIRE( 1 == c.sign() );

        REQUIRE( 9 == e.bits() );
        REQUIRE( 2 == e.bytes() );
        REQUIRE( 1 == e.sign() );
    }
    {
        BigInt a = BigInt::from_s32(-1), b = BigInt::from_s32(-2), c = BigInt::from_s32(-3), d = BigInt::from_s32(-256);
        std::cout << "big_int -1:: " << a.to_dec_string(true) << std::endl;
        std::cout << "big_int -2:: " << b.to_dec_string(true) << std::endl;
        std::cout << "big_int -3:: " << c.to_dec_string(true) << std::endl;
        std::cout << "big_int -256:: " << d.to_dec_string(true) << std::endl;
        REQUIRE(  1 == a.bits() );
        REQUIRE(  1 == a.bytes() );
        REQUIRE( BigInt::negative == a.sign() );

        REQUIRE( 2 == b.bits() );
        REQUIRE( 1 == b.bytes() );
        REQUIRE(BigInt::negative == b.sign() );

        REQUIRE( 2 == c.bits() );
        REQUIRE( 1 == c.bytes() );
        REQUIRE(BigInt::negative == c.sign() );

        REQUIRE( 9 == d.bits() );
        REQUIRE( 2 == d.bytes() );
        REQUIRE(BigInt::negative == d.sign() );
    }
}
TEST_CASE( "MP Big Int Test 01", "[big_int_t][arithmetic][math]" ) {
    {
        BigInt a = 0xffffffffffffffff_u64, b = 0x12000000ffffffff_u64;
        std::cout << "big_int a:: " << a.to_dec_string(true) << std::endl;
        std::cout << "big_int a:: " << a.to_hex_string(true) << std::endl;
        std::cout << "big_int b:: " << b.to_dec_string(true) << std::endl;
        std::cout << "big_int b:: " << b.to_hex_string(true) << std::endl;

        BigInt ab = a*b;
        std::cout << "big_int a*b:: " << ab.to_dec_string(true) << std::endl;
        std::cout << "big_int a*b:: " << ab.to_hex_string(true) << std::endl;
    }
    {
        BigInt zero, ten(10), thirty(30), forty(40);
        REQUIRE( zero < ten );
        REQUIRE( zero < thirty );
        REQUIRE( ten < thirty );

        REQUIRE( ten > zero );
        REQUIRE( thirty > ten );
        REQUIRE( thirty > zero );

        REQUIRE( ten <= ten );
        REQUIRE( ten <= thirty );

        REQUIRE( thirty >= thirty );
        REQUIRE( thirty >= ten );

        REQUIRE( thirty == thirty );
        REQUIRE( thirty != ten );

        REQUIRE( zero   == min(zero, ten) );
        REQUIRE( ten    == max(zero, ten) );
        REQUIRE( ten    == clamp(zero,  ten, thirty) );
        REQUIRE( thirty == clamp(forty, ten, thirty) );
    }
}
TEST_CASE( "MP Big Int Test 02", "[big_int_t][arithmetic][math]" ) {
    REQUIRE( BigInt(10) == BigInt( 5) + BigInt(5) );
    REQUIRE( BigInt(10) == BigInt( 2) * BigInt(5) );
    REQUIRE( BigInt( 5) == BigInt(10) / BigInt(2) );
    REQUIRE( BigInt( 1) == BigInt(10) % BigInt(3) );

    REQUIRE( BigInt(  1) == BigInt(10).pow( BigInt( 0) ) );
    REQUIRE( BigInt( 10) == BigInt(10).pow( BigInt( 1) ) );
    REQUIRE( BigInt(  100000000_u64 ) == BigInt(10).pow( BigInt( 8) ) );
    REQUIRE( BigInt( 4294967296_u64 ) == BigInt( 2).pow( BigInt(32) ) );

    REQUIRE( BigInt(  0) == BigInt(10).pow( BigInt::from_s32(-1) ) );
}
TEST_CASE( "MP Big Int Dec Test 10", "[big_int_t][inout][math]" ) {
    {
        uint8_t a_u8[] = { 0x10, 0x32, 0x54, 0x76, 0x98, 0xba, 0xdc, 0xfe,
                           0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
                           0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff

                         };
        BigInt a(a_u8, sizeof(a_u8), lb_endian_t::little);
        std::cout << "big_int zero:: " << a.to_dec_string(true) << std::endl;
        std::cout << "big_int zero:: " << a.to_hex_string(true) << std::endl;
        REQUIRE( 23 == sizeof(a_u8) );
        if ( false ) {
            for(size_t i=0; i<sizeof(a_u8); ++i) {
                std::string s1;
                jau::appendToHexString(s1, a.byte_at(i), true);
                std::cout << "zero.buf[" << std::to_string(i) << "]: 0x" << s1 << std::endl;
            }
        }
        REQUIRE( sizeof(a_u8)*8 == a.bits() );
        REQUIRE( sizeof(a_u8) == a.bytes() );

        uint8_t buf[sizeof(a_u8)];
        {
            for(size_t i=0; i<sizeof(a_u8); ++i) {
                buf[i] = 0;
            }
            REQUIRE( sizeof(a_u8) == a.binary_encode(buf, sizeof(buf), jau::lb_endian_t::little) );
            if ( false ) {
                for(size_t i=0; i<sizeof(a_u8); ++i) {
                    std::string s1;
                    jau::appendToHexString(s1, buf[i], true);
                    std::cout << "le.buf[" << std::to_string(i) << "]: 0x" << s1 << std::endl;
                }
            }
            for(size_t i=0; i<sizeof(a_u8); ++i) {
                REQUIRE( (uint32_t)a_u8[i] == (uint32_t)buf[i] );
            }
            BigInt b(buf, sizeof(buf), jau::lb_endian_t::little);
            std::cout << "big_int le:: " << b.to_dec_string(true) << std::endl;
            std::cout << "big_int le:: " << b.to_hex_string(true) << std::endl;
            REQUIRE( a == b );
        }
        {
            for(size_t i=0; i<sizeof(a_u8); ++i) {
                buf[i] = 0;
            }
            REQUIRE( sizeof(a_u8) == a.binary_encode(buf, sizeof(buf), jau::lb_endian_t::big) );
            if ( false ) {
                for(size_t i=0; i<sizeof(a_u8); ++i) {
                    std::string s1;
                    jau::appendToHexString(s1, buf[i], true);
                    std::cout << "be.buf[" << std::to_string(i) << "]: 0x" << s1 << std::endl;
                }
            }
            for(size_t i=0; i<sizeof(a_u8); ++i) {
                REQUIRE( (uint32_t)a_u8[sizeof(a_u8)-i-1] == (uint32_t)buf[i] );
            }
            BigInt b(buf, sizeof(buf), jau::lb_endian_t::big);
            std::cout << "big_int be:: " << b.to_dec_string(true) << std::endl;
            std::cout << "big_int be:: " << b.to_hex_string(true) << std::endl;
            REQUIRE( a == b );
        }
    }
}
TEST_CASE( "MP Big Int Dec Test 11", "[big_int_t][inout][math]" ) {
    {
        uint8_t a_u8[] = { 0x10, 0x32, 0x54, 0x76, 0x98, 0xba, 0xdc, 0xfe,
                           0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
                           0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff

                         };
        BigInt a("0xffeeddccbbaa998877665544332211fedcba9876543210");
        std::cout << "big_int zero:: " << a.to_dec_string(true) << std::endl;
        std::cout << "big_int zero:: " << a.to_hex_string(true) << std::endl;
        REQUIRE( 23 == sizeof(a_u8) );
        REQUIRE( sizeof(a_u8)*8 == a.bits() );
        REQUIRE( sizeof(a_u8) == a.bytes() );

        uint8_t buf[sizeof(a_u8)];
        {
            for(size_t i=0; i<sizeof(a_u8); ++i) {
                buf[i] = 0;
            }
            REQUIRE( sizeof(a_u8) == a.binary_encode(buf, sizeof(buf), jau::lb_endian_t::little) );
            for(size_t i=0; i<sizeof(a_u8); ++i) {
                REQUIRE( (uint32_t)a_u8[i] == (uint32_t)buf[i] );
            }
            BigInt b(buf, sizeof(buf), jau::lb_endian_t::little);
            std::cout << "big_int le:: " << b.to_dec_string(true) << std::endl;
            std::cout << "big_int le:: " << b.to_hex_string(true) << std::endl;
            REQUIRE( a == b );
        }
        {
            for(size_t i=0; i<sizeof(a_u8); ++i) {
                buf[i] = 0;
            }
            REQUIRE( sizeof(a_u8) == a.binary_encode(buf, sizeof(buf), jau::lb_endian_t::big) );
            for(size_t i=0; i<sizeof(a_u8); ++i) {
                REQUIRE( (uint32_t)a_u8[sizeof(a_u8)-i-1] == (uint32_t)buf[i] );
            }
            BigInt b(buf, sizeof(buf), jau::lb_endian_t::big);
            std::cout << "big_int be:: " << b.to_dec_string(true) << std::endl;
            std::cout << "big_int be:: " << b.to_hex_string(true) << std::endl;
            REQUIRE( a == b );
        }
    }
}
TEST_CASE( "MP Big Int Error Handling Test 88", "[big_int_t][exceptions][error][arithmetic][math]" ) {
    {
        BigInt a = 1, b = 0, r;
        REQUIRE_THROWS_MATCHES( r = a / b, jau::math::MathDivByZeroError, Catch::Matchers::ContainsSubstring("div_by_zero") );
        REQUIRE_THROWS_MATCHES( r = a % b, jau::math::MathDivByZeroError, Catch::Matchers::ContainsSubstring("div_by_zero") );
    }
    {
        BigInt a = BigInt::from_s32(-1), b = BigInt::from_s32(-1), r;
        REQUIRE_THROWS_MATCHES( r = a % b, jau::math::MathDomainError, Catch::Matchers::ContainsSubstring("invalid") );
    }
}
