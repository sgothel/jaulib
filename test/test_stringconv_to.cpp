/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020 Gothel Software e.K.
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
#include <limits>

#include "test_datatype01.hpp"

#include <jau/basic_types.hpp>
#include <jau/cpp_lang_util.hpp>
#include <jau/string_cfmt.hpp>
#include <jau/string_util.hpp>
#include <jau/test/catch2_ext.hpp>
#include <jau/type_traits_queries.hpp>

typedef std::vector<int> std_vec_int;

typedef std_vec_int::iterator std_vec_int_iter;

typedef std_vec_int::const_iterator std_vec_int_citer;

typedef std_vec_int_citer::pointer std_vec_int_citer_pointer;

typedef decltype( std::declval<std_vec_int_citer>().operator->() ) std_vec_int_citer_ptrop_retval;

using namespace jau::int_literals;

TEST_CASE( "Test 00 - to_string", "[jau][string][to_string]" ) {
    int i1 = 1;
    uint64_t u64_1 = 1116791496961ull;
    void * p_v_1 = (void *)0xAFFE;
    float float_1 = 1.65f;

    Addr48Bit addr48bit_1(u64_1);

    CHECK("1" == jau::to_string<int>(i1));
    CHECK("1116791496961" == jau::to_string(u64_1));
    CHECK("0xaffe" == jau::to_string(p_v_1));
    CHECK("0xaffe" == jau::toHexString(0xaffe_u32));
    {
        // radix, default: no-width, prefix, no-separator, '0' padding
        CHECK("0xaffe" == jau::to_string(0xaffe_u32, 16));               // hex
        CHECK("876543210" == jau::to_string(876543210_u64, 10));         // dec
        CHECK("077652" == jau::to_string(077652_u32, 8));                // oct
        CHECK("0b11010101101" == jau::to_string(0b11010101101_u32, 2));  // bin

        // no-prefix, radix, default: no-width, no-separator, '0' padding
        CHECK("affe" == jau::to_string(0xaffe_u32, 16, jau::LoUpCase::lower, jau::PrefixOpt::none));     // hex
        CHECK("876543210" == jau::to_string(876543210_u64, 10, jau::LoUpCase::lower, jau::PrefixOpt::none));       // dec
        CHECK("77652" == jau::to_string(077652_u32, 8, jau::LoUpCase::lower, jau::PrefixOpt::none));               // oct
        CHECK("11010101101" == jau::to_string(0b11010101101_u32, 2, jau::LoUpCase::lower, jau::PrefixOpt::none));  // bin

        // radix, width-expansion, default: prefix, no-separator, '0' padding
        CHECK("0x00affe" == jau::to_string(0xaffe_u32, 16, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 8));                 // hex
        CHECK("000876543210" == jau::to_string(876543210_u64, 10, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 12));         // dec
        CHECK("0000077652" == jau::to_string(077652_u32, 8, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 10));               // oct
        CHECK("0b00011010101101" == jau::to_string(0b11010101101_u32, 2, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 16));  // bin

        // no-prefix, radix, width-expansion, default: no-separator, '0' padding
        CHECK("0000affe" == jau::to_string(0xaffe_u32, 16, jau::LoUpCase::lower, jau::PrefixOpt::none, 8));                 // hex
        CHECK("000876543210" == jau::to_string(876543210_u64, 10, jau::LoUpCase::lower, jau::PrefixOpt::none, 12));         // dec
        CHECK("0000077652" == jau::to_string(077652_u32, 8, jau::LoUpCase::lower, jau::PrefixOpt::none, 10));               // oct
        CHECK("0000011010101101" == jau::to_string(0b11010101101_u32, 2, jau::LoUpCase::lower, jau::PrefixOpt::none, 16));  // bin

        // radix, separator, default: no-width, prefix, '0' padding
        CHECK("0xaffe" == jau::to_string(0xaffe_u32, 16, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 0, '\''));             // hex
        CHECK("0x1'affe" == jau::to_string(0x1affe_u32, 16, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 0, '\''));          // hex
        CHECK("876'543'210" == jau::to_string(876543210_u64, 10, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 0, '\''));     // dec
        CHECK("1'876'543'210" == jau::to_string(1876543210_u64, 10, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 0, '\''));  // dec
        CHECK("04321'7652" == jau::to_string(043217652_u32, 8, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 0, '\''));       // oct
        CHECK("01'4321'7652" == jau::to_string(0143217652_u32, 8, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 0, '\''));    // oct
        CHECK("0b1010'1101" == jau::to_string(0b10101101_u32, 2, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 0, '\''));     // bin
        CHECK("0b1'1010'1101" == jau::to_string(0b110101101_u32, 2, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 0, '\''));  // bin

        // no-prefix, radix, separator, default: no-width, '0' padding
        CHECK("affe" == jau::to_string(0xaffe_u32, 16, jau::LoUpCase::lower, jau::PrefixOpt::none, 0, '\''));               // hex
        CHECK("1'affe" == jau::to_string(0x1affe_u32, 16, jau::LoUpCase::lower, jau::PrefixOpt::none, 0, '\''));            // hex
        CHECK("876'543'210" == jau::to_string(876543210_u64, 10, jau::LoUpCase::lower, jau::PrefixOpt::none, 0, '\''));     // dec
        CHECK("1'876'543'210" == jau::to_string(1876543210_u64, 10, jau::LoUpCase::lower, jau::PrefixOpt::none, 0, '\''));  // dec
        CHECK("4321'7652" == jau::to_string(043217652_u32, 8, jau::LoUpCase::lower, jau::PrefixOpt::none, 0, '\''));        // oct
        CHECK("1'4321'7652" == jau::to_string(0143217652_u32, 8, jau::LoUpCase::lower, jau::PrefixOpt::none, 0, '\''));     // oct
        CHECK("1010'1101" == jau::to_string(0b10101101_u32, 2, jau::LoUpCase::lower, jau::PrefixOpt::none, 0, '\''));       // bin
        CHECK("1'1010'1101" == jau::to_string(0b110101101_u32, 2, jau::LoUpCase::lower, jau::PrefixOpt::none, 0, '\''));    // bin

        // radix, width-expansion, separator, default: prefix, '0' padding
        CHECK("0xaffe" == jau::to_string(0xaffe_u32, 16, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 6, '\''));    // hex
        CHECK("0x'affe" == jau::to_string(0xaffe_u32, 16, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 7, '\''));   // hex
        CHECK("0x0'affe" == jau::to_string(0xaffe_u32, 16, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 8, '\''));  // hex

        CHECK("876'543'210" == jau::to_string(876543210_u64, 10, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 11, '\''));    // dec
        CHECK("'876'543'210" == jau::to_string(876543210_u64, 10, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 12, '\''));   // dec
        CHECK("0'876'543'210" == jau::to_string(876543210_u64, 10, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 13, '\''));  // dec

        CHECK("07652" == jau::to_string(07652_u32, 8, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 5, '\''));    // oct
        CHECK("0'7652" == jau::to_string(07652_u32, 8, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 6, '\''));   // oct
        CHECK("00'7652" == jau::to_string(07652_u32, 8, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 7, '\''));  // oct

        CHECK("0b1110'1010'1101" == jau::to_string(0b111010101101_u32, 2, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 16, '\''));    // bin
        CHECK("0b'1110'1010'1101" == jau::to_string(0b111010101101_u32, 2, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 17, '\''));   // bin
        CHECK("0b0'1110'1010'1101" == jau::to_string(0b111010101101_u32, 2, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 18, '\''));  // bin

        // no-prefix, radix, width-expansion, separator, default: '0' padding
        CHECK("affe" == jau::to_string(0xaffe_u32, 16, jau::LoUpCase::lower, jau::PrefixOpt::none, 4, '\''));    // hex
        CHECK("'affe" == jau::to_string(0xaffe_u32, 16, jau::LoUpCase::lower, jau::PrefixOpt::none, 5, '\''));   // hex
        CHECK("0'affe" == jau::to_string(0xaffe_u32, 16, jau::LoUpCase::lower, jau::PrefixOpt::none, 6, '\''));  // hex

        CHECK("876'543'210" == jau::to_string(876543210_u64, 10, jau::LoUpCase::lower, jau::PrefixOpt::none, 11, '\''));    // dec
        CHECK("'876'543'210" == jau::to_string(876543210_u64, 10, jau::LoUpCase::lower, jau::PrefixOpt::none, 12, '\''));   // dec
        CHECK("0'876'543'210" == jau::to_string(876543210_u64, 10, jau::LoUpCase::lower, jau::PrefixOpt::none, 13, '\''));  // dec

        CHECK("7652" == jau::to_string(07652_u32, 8, jau::LoUpCase::lower, jau::PrefixOpt::none, 4, '\''));    // oct
        CHECK("'7652" == jau::to_string(07652_u32, 8, jau::LoUpCase::lower, jau::PrefixOpt::none, 5, '\''));   // oct
        CHECK("0'7652" == jau::to_string(07652_u32, 8, jau::LoUpCase::lower, jau::PrefixOpt::none, 6, '\''));  // oct

        CHECK("1110'1010'1101" == jau::to_string(0b111010101101_u32, 2, jau::LoUpCase::lower, jau::PrefixOpt::none, 14, '\''));    // bin
        CHECK("'1110'1010'1101" == jau::to_string(0b111010101101_u32, 2, jau::LoUpCase::lower, jau::PrefixOpt::none, 15, '\''));   // bin
        CHECK("0'1110'1010'1101" == jau::to_string(0b111010101101_u32, 2, jau::LoUpCase::lower, jau::PrefixOpt::none, 16, '\''));  // bin

        // no-prefix, radix, width-expansion, padding ' '
        CHECK("    affe" == jau::to_string(0xaffe_u32, 16, jau::LoUpCase::lower, jau::PrefixOpt::none, 8, '\'', ' '));
        CHECK("    876'543'210" == jau::to_string(876543210_u32, 10, jau::LoUpCase::lower, jau::PrefixOpt::none, 15, '\'', ' '));
        CHECK("    110'1010'1101" == jau::to_string(0b11010101101_u32, 2, jau::LoUpCase::lower, jau::PrefixOpt::none, 17, '\'', ' '));
        CHECK("    7'7652" == jau::to_string(077652_u32, 8, jau::LoUpCase::lower, jau::PrefixOpt::none, 10, '\'', ' '));
    }
    CHECK("1.650000" == jau::to_string(float_1));

    CHECK("01:04:05:F5:E1:01" == jau::to_string(addr48bit_1));

    //
    // Validating 'pointer std::vector::const_iterator.operator->()'
    // and the to_string type trait logic of it.
    //

    // jau::type_cue<std_vec_int_citer>::print("std_vec_int_citer", jau::TypeTraitGroup::ALL);
    // jau::type_cue<std_vec_int_citer_pointer>::print("std_vec_int_citer_pointer", jau::TypeTraitGroup::ALL);

    // jau::type_cue<std_vec_int_citer_ptrop_retval>::print("std_vec_int_citer_ptrop_retval", jau::TypeTraitGroup::ALL);
    printf("jau::has_member_of_pointer<std_vec_int_citer>) %d\n", jau::has_member_of_pointer<std_vec_int_citer>::value);

    std_vec_int vec_int_1;
    vec_int_1.push_back(1); vec_int_1.push_back(2); vec_int_1.push_back(3);
    std_vec_int_citer vec_int_citer_1B = vec_int_1.cbegin();
    uint8_t* vec_int_citer_1B_ptr = (uint8_t*)(vec_int_citer_1B.operator->());
    std::string vec_int_citer_1B_str = jau::toHexString(vec_int_citer_1B_ptr);

    std_vec_int_citer vec_int_citer_1E = vec_int_1.cend();
    uint8_t* vec_int_citer_1E_ptr = (uint8_t*)(vec_int_citer_1E.operator->());
    std::string vec_int_citer_1E_str = jau::toHexString(vec_int_citer_1E_ptr);

    std::ptrdiff_t vec_int_citer_1E_1B_ptrdiff = vec_int_citer_1E_ptr - vec_int_citer_1B_ptr;
    size_t vec_int_citer_1E_1B_ptr_count = vec_int_citer_1E_1B_ptrdiff / sizeof(int);
    size_t vec_int_citer_1E_1B_itr_count = vec_int_citer_1E - vec_int_citer_1B;

    printf("vec_int_citer_1E - vec_int_citer_1B = itr_count %zu, ptr_count %zu\n",
           vec_int_citer_1E_1B_itr_count, vec_int_citer_1E_1B_ptr_count);
    printf("vec_int_citer_1E - vec_int_citer_1B = %zu\n", vec_int_citer_1E_1B_itr_count);
    printf("vec_int_citer_1B_ptr %s, vec_int_citer_1E1_ptr = %s\n", vec_int_citer_1B_str.c_str(), vec_int_citer_1E_str.c_str());

    CHECK(vec_int_citer_1E_1B_itr_count == 3);
    CHECK(vec_int_citer_1E_1B_itr_count == vec_int_citer_1E_1B_ptr_count);

    CHECK(vec_int_citer_1E_str == jau::to_string(vec_int_citer_1E));
}

#if 0
TEST_CASE( "Test 01 - to_string(radix)", "[jau][string][to_string(radix)]" ) {
    REQUIRE( true == true );
}

TEST_CASE( "Test 02 - toHexString()", "[jau][string][toHexString]" ) {
    REQUIRE( true == true );
}
#endif

static void testToBitString(std::string_view prefix, std::string_view exp_be_s, const uint64_t &exp_be_v, size_t max_bits, bool check_value=true) {
    std::cout << prefix << ": max_bits " << max_bits << "\n";
    std::string has_be_s1 = jau::toBitString(exp_be_v, jau::bit_order_t::msb, jau::PrefixOpt::none, max_bits);
    std::cout << "  exp_be_s : " << exp_be_s << "\n";
    std::cout << "  has_be_s1: " << has_be_s1 << "\n";
    REQUIRE( exp_be_s == has_be_s1 );

    if( check_value ) {
        const auto [has_be_v, len_be, ok_be] = jau::fromBitString(exp_be_s);
        REQUIRE(true == ok_be);
        REQUIRE(exp_be_s.size() == len_be);
        std::string has_be_s2 = jau::toBitString(has_be_v, jau::bit_order_t::msb, jau::PrefixOpt::none, max_bits);
        std::cout << "  has_be_s2: " << has_be_s2 << "\n";
        REQUIRE(exp_be_v == has_be_v);
    }
}
static void testToBitString(std::string_view prefix, std::string_view s_be1, const uint32_t &v_be1) {
    testToBitString(prefix, s_be1, v_be1, s_be1.size(), true);
}

TEST_CASE( "Test 03 - toBitString()", "[jau][string][toBitString]" ) {
    {
        testToBitString("Test 03.01.01", "000101100101110111011001",         0b101100101110111011001_u64, 0);
        testToBitString("Test 03.01.02", "000101100101110111011001",         0b101100101110111011001_u64);
        testToBitString("Test 03.01.03", "101110111011001",                  0b101100101110111011001_u64, 15, false);
        testToBitString("Test 03.01.04", "00000000000101100101110111011001",  0b101100101110111011001_u64);
        testToBitString("Test 03.01.05", "000000000000101100101110111011001", 0b101100101110111011001_u64, 33);

        testToBitString("Test 03.02.01", "11011001011101110110011110001101", 0b11011001011101110110011110001101_u64, 0);
        testToBitString("Test 03.02.02", "11011001011101110110011110001101", 0b11011001011101110110011110001101_u64, 32);
        testToBitString("Test 03.02.03", "01011001011101110110011110001101", 0b01011001011101110110011110001101_u64, 0);
        testToBitString("Test 03.02.04", "01011001011101110110011110001101", 0b01011001011101110110011110001101_u64, 32);
        testToBitString("Test 03.02.05", "0101110111011001",                 0b0101100101110111011001_u64, 16, false);

        testToBitString("Test 03.03.01", "1101100101110111011001111000110111011001011101110110011110001101",
                                        0b1101100101110111011001111000110111011001011101110110011110001101_u64, 0);
        testToBitString("Test 03.03.02", "1101100101110111011001111000110111011001011101110110011110001101",
                                        0b1101100101110111011001111000110111011001011101110110011110001101_u64, 64);

        testToBitString("Test 03.03.03", "0101100101110111011001111000110111011001011101110110011110001101",
                                        0b0101100101110111011001111000110111011001011101110110011110001101_u64, 0);
        testToBitString("Test 03.03.04", "0101100101110111011001111000110111011001011101110110011110001101",
                                        0b0101100101110111011001111000110111011001011101110110011110001101_u64, 64);

        testToBitString("Test 03.03.05", "0001100101110111011001111000110111011001011101110110011110001101",
                                        0b0001100101110111011001111000110111011001011101110110011110001101_u64, 0);
        testToBitString("Test 03.03.06", "0001100101110111011001111000110111011001011101110110011110001101",
                                        0b0001100101110111011001111000110111011001011101110110011110001101_u64, 64);

        testToBitString("Test 03.03.07", "1111111111101010111101101011111000000000000000000000000000000000",
                                         0b1111111111101010111101101011111000000000000000000000000000000000_u64, 0);
        testToBitString("Test 03.03.08", "1111111111101010111101101011111000000000000000000000000000000000",
                                         0b1111111111101010111101101011111000000000000000000000000000000000_u64, 64);

        testToBitString("Test 03.03.09", "11111110101001111110101011110110",
                                         0b0000000000000000000000000000000011111110101001111110101011110110_u64, 0);
        testToBitString("Test 03.03.10", "0000000000000000000000000000000011111110101001111110101011110110",
                                         0b0000000000000000000000000000000011111110101001111110101011110110_u64, 64);
        testToBitString("Test 03.03.11", "011111110101001111110101011110110",
                                         0b0000000000000000000000000000000011111110101001111110101011110110_u64, 33);

        testToBitString("Test 03.03.12", "00000000",
                                         0b0000000000000000000000000000000000000000000000000000000000000000_u64, 0);
        testToBitString("Test 03.03.13", "0000000000000000000000000000000000000000000000000000000000000000",
                                         0b0000000000000000000000000000000000000000000000000000000000000000_u64, 64);
    }
}

