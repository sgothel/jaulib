/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021-2025 Gothel Software e.K.
 *
 * SPDX-License-Identifier: MIT
 *
 * This Source Code Form is subject to the terms of the MIT License
 * If a copy of the MIT was not distributed with this file,
 * you can obtain one at https://opensource.org/license/mit/.
 */

#include <cassert>
#include <cstdint>
#include <cstring>
#include <string_view>
#include <vector>

#include <jau/basic_types.hpp>
#include <jau/debug.hpp>
#include <jau/string_util.hpp>

using namespace jau::fractions_i64_literals;
using namespace jau::int_literals;

extern "C" {
#include <unistd.h>
}

class BitDemoData {
  public:
    constexpr static uint32_t UNSIGNED_INT_MAX_VALUE = 0xffffffff_u32;

    /**
     * Returns the 32 bit mask of n-bits, i.e. n low order 1's.
     * <p>
     * Implementation handles n == 32.
     * </p>
     * @throws IndexOutOfBoundsError if {@code b} is out of bounds, i.e. &gt; 32
     */
    static uint32_t getBitMask(size_t n) {
        if( 32 > n ) {
            return ( 1_u32 << n ) - 1;
        } else if ( 32 == n ) {
            return UNSIGNED_INT_MAX_VALUE;
        } else {
            throw jau::IndexOutOfBoundsError("n <= 32 expected", n, 32, E_FILE_LINE);
        }
    }

    inline const static std::vector<std::string_view> pyramid32bit_one = {
        "00000000000000000000000000000001",
        "00000000000000000000000000000010",
        "00000000000000000000000000000100",
        "00000000000000000000000000001000",
        "00000000000000000000000000010000",
        "00000000000000000000000000100000",
        "00000000000000000000000001000000",
        "00000000000000000000000010000000",
        "00000000000000000000000100000000",
        "00000000000000000000001000000000",
        "00000000000000000000010000000000",
        "00000000000000000000100000000000",
        "00000000000000000001000000000000",
        "00000000000000000010000000000000",
        "00000000000000000100000000000000",
        "00000000000000001000000000000000",
        "00000000000000010000000000000000",
        "00000000000000100000000000000000",
        "00000000000001000000000000000000",
        "00000000000010000000000000000000",
        "00000000000100000000000000000000",
        "00000000001000000000000000000000",
        "00000000010000000000000000000000",
        "00000000100000000000000000000000",
        "00000001000000000000000000000000",
        "00000010000000000000000000000000",
        "00000100000000000000000000000000",
        "00001000000000000000000000000000",
        "00010000000000000000000000000000",
        "00100000000000000000000000000000",
        "01000000000000000000000000000000",
        "10000000000000000000000000000000"
    };

    //
    // MSB -> LSB over whole data
    //
    constexpr static uint8_t testBytesMSB[] = { 0xde_u8, 0xaf_u8, 0xca_u8, 0xfe_u8 };
    constexpr static uint64_t testIntMSB = 0xdeafcafe_u32;
    constexpr static std::string_view testStringsMSB[] = { "11011110", "10101111", "11001010", "11111110" };
    constexpr static std::string_view testStringMSB = "11011110"
                                                      "10101111"
                                                      "11001010"
                                                      "11111110";

    //
    // MSB -> LSB, reverse bit-order over each byte of testBytesLSB
    //
    constexpr static uint8_t testBytesMSB_rev[] = { 0xfe_u8, 0xca_u8, 0xaf_u8, 0xde_u8 };
    constexpr static uint64_t testIntMSB_rev = 0xfecaafde_u32;
    constexpr static std::string_view testStringsMSB_rev[] = { "11111110", "11001010", "10101111", "11011110" };
    constexpr static std::string_view testStringMSB_rev = "11111110"
                                                          "11001010"
                                                          "10101111"
                                                          "11011110";

    //
    // LSB -> MSB over whole data
    //
    constexpr static uint8_t testBytesLSB[] = { 0x7f_u8, 0x53_u8, 0xf5_u8, 0x7b_u8 };
    constexpr static uint64_t testIntLSB = 0x7f53f57b_u32;
    constexpr static std::string_view testStringsLSB[] = { "01111111", "01010011", "11110101", "01111011" };
    constexpr static std::string_view testStringLSB = "01111111"
                                                      "01010011"
                                                      "11110101"
                                                      "01111011";

    //
    // LSB -> MSB, reverse bit-order over each byte of testBytesMSB
    //
    constexpr static uint8_t testBytesLSB_revByte[] = { 0x7b_u8, 0xf5_u8, 0x53_u8, 0x7f_u8 };
    constexpr static uint64_t testIntLSB_revByte = 0x7bf5537f_u32;
    constexpr static std::string_view testStringsLSB_revByte[] = { "01111011", "11110101", "01010011", "01111111" };
    constexpr static std::string_view testStringLSB_revByte = "01111011"
                                                              "11110101"
                                                              "01010011"
                                                              "01111111";

    static void dumpData(const std::string& prefix, const uint8_t* data, size_t len) {
        for( size_t i = 0; i < len; ) {
            fprintf(stderr, "%s: %03zu: ", prefix.c_str(), i);
            for( size_t j = 0; j < 8 && i < len; ++j, ++i ) {
                const uint8_t v = 0xFF & data[i];
                fprintf(stderr, "%s, ", toHexBinaryString(v, 8).c_str());
            }
            fprintf(stderr, "\n");
        }
    }

    static size_t getOneBitCount(std::string_view pattern) {
        size_t c = 0;
        for( size_t i = 0; i < pattern.length(); ++i ) {
            if( '1' == pattern[i] ) {
                ++c;
            }
        }
        return c;
    }
    static uint64_t toLong(const std::string_view bitPattern) {
        const auto [result, len, ok] = jau::fromBitString(bitPattern);
        if (!ok) {
            throw jau::RuntimeException("parse error: " + std::string(bitPattern), E_FILE_LINE);
        }
        return (uint64_t)result;
    }
    static uint64_t toInteger(const std::string_view bitPattern) {
        const auto [result, len, ok] = jau::fromBitString(bitPattern);
        if (!ok) {
            throw jau::RuntimeException("parse error: " + std::string(bitPattern), E_FILE_LINE);
        }
        return result;
    }

    static std::string toHexString(const int v) {
        return jau::toHexString(v);
    }
    static std::string toHexString(const int64_t v) {
        return jau::toHexString(v);
    }
    static constexpr const char* strZeroPadding = "0000000000000000000000000000000000000000000000000000000000000000";  // 64
    static std::string toBinaryString(const uint64_t v, const size_t bitCount) {
        if( 0 == bitCount ) {
            return "";
        }
        const uint64_t mask = (1_u64 << bitCount) - 1_u64;
        const std::string& s0 = jau::to_string(mask & v, 2, jau::PrefixOpt::none, bitCount);
        std::string padding(strZeroPadding, bitCount > s0.length() ? bitCount - s0.length() : 0);
        return padding + s0;
    }
    static std::string toHexBinaryString(const uint64_t v, const int bitCount) {
        return jau::toHexString(v) + ", " + toBinaryString(v, bitCount);
    }
};
