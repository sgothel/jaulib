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
#include <iostream>

#include <jau/test/catch2_ext.hpp>

#include <jau/int_types.hpp>
#include <jau/mp/big_int_t.hpp>

using namespace jau;
using namespace jau::mp;
using namespace jau::int_literals;

TEST_CASE( "MP Big Int Test 00", "[big_int_t][arithmetic][math]" ) {
    std::cout << "big_int mp_word_bits " << std::to_string( mp_word_bits ) << std::endl;

    {
        big_int_t a = 1_u64, b = 2_u64, c = 3_u64, d = 4_u64, e = 256_u64;
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
            big_int_t r = a + b;
            std::cout << "big_int 1+2:: " << r.to_dec_string(true) << std::endl;
            REQUIRE( c == r );
        }
        {
            big_int_t r = b * b;
            std::cout << "big_int 2*2:: " << r.to_dec_string(true) << std::endl;
            REQUIRE( d == r );
        }
        {
            big_int_t r = b / b;
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
        big_int_t a = big_int_t::from_s32(-1), b = big_int_t::from_s32(-2), c = big_int_t::from_s32(-3), d = big_int_t::from_s32(-256);
        std::cout << "big_int -1:: " << a.to_dec_string(true) << std::endl;
        std::cout << "big_int -2:: " << b.to_dec_string(true) << std::endl;
        std::cout << "big_int -3:: " << c.to_dec_string(true) << std::endl;
        std::cout << "big_int -256:: " << d.to_dec_string(true) << std::endl;
        REQUIRE(  1 == a.bits() );
        REQUIRE(  1 == a.bytes() );
        REQUIRE( big_int_t::negative == a.sign() );

        REQUIRE( 2 == b.bits() );
        REQUIRE( 1 == b.bytes() );
        REQUIRE(big_int_t::negative == b.sign() );

        REQUIRE( 2 == c.bits() );
        REQUIRE( 1 == c.bytes() );
        REQUIRE(big_int_t::negative == c.sign() );

        REQUIRE( 9 == d.bits() );
        REQUIRE( 2 == d.bytes() );
        REQUIRE(big_int_t::negative == d.sign() );
    }
}
TEST_CASE( "MP Big Int Test 01", "[big_int_t][arithmetic][math]" ) {
    {
        big_int_t a = 0xffffffffffffffff_u64, b = 0x12000000ffffffff_u64;
        std::cout << "big_int a:: " << a.to_dec_string(true) << std::endl;
        std::cout << "big_int a:: " << a.to_hex_string(true) << std::endl;
        std::cout << "big_int b:: " << b.to_dec_string(true) << std::endl;
        std::cout << "big_int b:: " << b.to_hex_string(true) << std::endl;

        big_int_t ab = a*b;
        std::cout << "big_int a*b:: " << ab.to_dec_string(true) << std::endl;
        std::cout << "big_int a*b:: " << ab.to_hex_string(true) << std::endl;
    }
}
TEST_CASE( "MP Big Int Test 02", "[big_int_t][arithmetic][math]" ) {
    REQUIRE( big_int_t(10) == big_int_t( 5) + big_int_t(5) );
    REQUIRE( big_int_t(10) == big_int_t( 2) * big_int_t(5) );
    REQUIRE( big_int_t( 5) == big_int_t(10) / big_int_t(2) );
    REQUIRE( big_int_t( 1) == big_int_t(10) % big_int_t(3) );

    REQUIRE( big_int_t(  1) == big_int_t(10).pow( big_int_t( 0) ) );
    REQUIRE( big_int_t( 10) == big_int_t(10).pow( big_int_t( 1) ) );
    REQUIRE( big_int_t(  100000000_u64 ) == big_int_t(10).pow( big_int_t( 8) ) );
    REQUIRE( big_int_t( 4294967296_u64 ) == big_int_t( 2).pow( big_int_t(32) ) );

    REQUIRE( big_int_t(  0) == big_int_t(10).pow( big_int_t::from_s32(-1) ) );
}
TEST_CASE( "MP Big Int Dec Test 10", "[big_int_t][inout][math]" ) {
    {
        uint8_t a_u8[] = { 0x10, 0x32, 0x54, 0x76, 0x98, 0xba, 0xdc, 0xfe,
                           0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
                           0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff

                         };
        big_int_t a(a_u8, sizeof(a_u8), lb_endian::little);
        std::cout << "big_int a:: " << a.to_dec_string(true) << std::endl;
        std::cout << "big_int a:: " << a.to_hex_string(true) << std::endl;
        REQUIRE( 23 == sizeof(a_u8) );
        for(size_t i=0; i<sizeof(a_u8); ++i) {
            std::string s1;
            jau::byteHexString(s1, a.byte_at(i), true);
            std::cout << "a.buf[" << std::to_string(i) << "]: 0x" << s1 << std::endl;
        }
        REQUIRE( sizeof(a_u8)*8 == a.bits() );
        REQUIRE( sizeof(a_u8) == a.bytes() );

        uint8_t buf[sizeof(a_u8)];
        {
            for(size_t i=0; i<sizeof(a_u8); ++i) {
                buf[i] = 0;
            }
            REQUIRE( sizeof(a_u8) == a.binary_encode(buf, sizeof(buf), jau::lb_endian::little) );
            for(size_t i=0; i<sizeof(a_u8); ++i) {
                std::string s1;
                jau::byteHexString(s1, buf[i], true);
                std::cout << "le.buf[" << std::to_string(i) << "]: 0x" << s1 << std::endl;
            }
            for(size_t i=0; i<sizeof(a_u8); ++i) {
                REQUIRE( (uint32_t)a_u8[i] == (uint32_t)buf[i] );
            }
            big_int_t b(buf, sizeof(buf), jau::lb_endian::little);
            std::cout << "big_int le:: " << b.to_dec_string(true) << std::endl;
            std::cout << "big_int le:: " << b.to_hex_string(true) << std::endl;
            REQUIRE( a == b );
        }
        {
            for(size_t i=0; i<sizeof(a_u8); ++i) {
                buf[i] = 0;
            }
            REQUIRE( sizeof(a_u8) == a.binary_encode(buf, sizeof(buf), jau::lb_endian::big) );
            for(size_t i=0; i<sizeof(a_u8); ++i) {
                std::string s1;
                jau::byteHexString(s1, buf[i], true);
                std::cout << "be.buf[" << std::to_string(i) << "]: 0x" << s1 << std::endl;
            }
            for(size_t i=0; i<sizeof(a_u8); ++i) {
                REQUIRE( (uint32_t)a_u8[sizeof(a_u8)-i-1] == (uint32_t)buf[i] );
            }
            big_int_t b(buf, sizeof(buf), jau::lb_endian::big);
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
        big_int_t a("0xffeeddccbbaa998877665544332211fedcba9876543210");
        std::cout << "big_int a:: " << a.to_dec_string(true) << std::endl;
        std::cout << "big_int a:: " << a.to_hex_string(true) << std::endl;
        REQUIRE( 23 == sizeof(a_u8) );
        REQUIRE( sizeof(a_u8)*8 == a.bits() );
        REQUIRE( sizeof(a_u8) == a.bytes() );

        uint8_t buf[sizeof(a_u8)];
        {
            for(size_t i=0; i<sizeof(a_u8); ++i) {
                buf[i] = 0;
            }
            REQUIRE( sizeof(a_u8) == a.binary_encode(buf, sizeof(buf), jau::lb_endian::little) );
            for(size_t i=0; i<sizeof(a_u8); ++i) {
                REQUIRE( (uint32_t)a_u8[i] == (uint32_t)buf[i] );
            }
            big_int_t b(buf, sizeof(buf), jau::lb_endian::little);
            std::cout << "big_int le:: " << b.to_dec_string(true) << std::endl;
            std::cout << "big_int le:: " << b.to_hex_string(true) << std::endl;
            REQUIRE( a == b );
        }
        {
            for(size_t i=0; i<sizeof(a_u8); ++i) {
                buf[i] = 0;
            }
            REQUIRE( sizeof(a_u8) == a.binary_encode(buf, sizeof(buf), jau::lb_endian::big) );
            for(size_t i=0; i<sizeof(a_u8); ++i) {
                REQUIRE( (uint32_t)a_u8[sizeof(a_u8)-i-1] == (uint32_t)buf[i] );
            }
            big_int_t b(buf, sizeof(buf), jau::lb_endian::big);
            std::cout << "big_int be:: " << b.to_dec_string(true) << std::endl;
            std::cout << "big_int be:: " << b.to_hex_string(true) << std::endl;
            REQUIRE( a == b );
        }
    }
}
TEST_CASE( "MP Big Int Error Handling Test 88", "[big_int_t][error][arithmetic][math]" ) {
    {
        big_int_t a = 1, b = 0, r;
        REQUIRE_THROWS_MATCHES( r = a / b, MathDivByZeroError, Catch::Matchers::ContainsSubstring("div_by_zero") );
        REQUIRE_THROWS_MATCHES( r = a % b, MathDivByZeroError, Catch::Matchers::ContainsSubstring("div_by_zero") );
    }
    {
        big_int_t a = big_int_t::from_s32(-1), b = big_int_t::from_s32(-1), r;
        REQUIRE_THROWS_MATCHES( r = a % b, MathDomainError, Catch::Matchers::ContainsSubstring("invalid") );
    }
}
