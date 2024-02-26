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
    REQUIRE((nsize_t)std::numeric_limits<mp_word_t>::max() + (nsize_t)1 == big_int_t::base);

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
        REQUIRE( big_int_t::Negative == a.sign() );

        REQUIRE( 2 == b.bits() );
        REQUIRE( 1 == b.bytes() );
        REQUIRE(big_int_t::Negative == b.sign() );

        REQUIRE( 2 == c.bits() );
        REQUIRE( 1 == c.bytes() );
        REQUIRE(big_int_t::Negative == c.sign() );

        REQUIRE( 9 == d.bits() );
        REQUIRE( 2 == d.bytes() );
        REQUIRE(big_int_t::Negative == d.sign() );
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
