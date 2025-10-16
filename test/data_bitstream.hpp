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
#include <jau/io/byte_stream.hpp>
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
    // MSB -> LSB over whole data, big-endian
    //
    constexpr static uint8_t testBytesMSB64_be[] = { 0xfa_u8, 0xde_u8, 0xaf_u8, 0xfe_u8, 0xde_u8, 0xaf_u8, 0xca_u8, 0xfe_u8 };
    constexpr static uint64_t testIntMSB64_be = 0xfadeaffedeafcafe_u64;
    // 11111010 11011110 10101111 11111110 11011110 10101111 11001010 11111110
    constexpr static std::string_view testStringsMSB64_be[] = { "11111010", "11011110", "10101111", "11111110",
                                                                "11011110", "10101111", "11001010", "11111110" };
    constexpr static std::string_view testStringMSB64_be = "11111010" "11011110" "10101111" "11111110"
                                                           "11011110" "10101111" "11001010" "11111110";

    //
    // MSB -> LSB, little-endian. Reverse byte-order of testBytesMSB64_be
    //
    constexpr static uint8_t testBytesMSB64_le[] = { 0xfe_u8, 0xca_u8, 0xaf_u8, 0xde_u8, 0xfe_u8, 0xaf_u8, 0xde_u8, 0xfa_u8 };
    constexpr static uint64_t testIntMSB64_le = 0xfecaafdefeafdefa_u64;
    // 11111110 11001010 10101111 11011110 11111110 10101111 11011110 11111010
    constexpr static std::string_view testStringsMSB64_le[] = { "11111110", "11001010", "10101111", "11011110",
                                                                "11111110", "10101111", "11011110", "11111010" };

    constexpr static std::string_view testStringMSB64_le = "11111110" "11001010" "10101111" "11011110"
                                                           "11111110" "10101111" "11011110" "11111010";
    //
    // LSB -> MSB over whole data, big-endian
    //
    constexpr static uint8_t testBytesLSB64_be[] = { 0x5F_u8, 0x7B_u8, 0xF5_u8, 0x7F_u8, 0x7B_u8, 0xF5_u8, 0x53_u8, 0x7F_u8 };
    constexpr static uint64_t testIntLSB64_be = 0x5F7BF57F7BF5537F_u64;
    // 01011111 01111011 11110101 01111111 01111011 11110101 01010011 01111111
    constexpr static std::string_view testStringsLSB64_be[] = { "01011111", "01111011", "11110101", "01111111",
                                                                "01111011", "11110101", "01010011", "01111111" };
    constexpr static std::string_view testStringLSB64_be = "01011111" "01111011" "11110101" "01111111"
                                                           "01111011" "11110101" "01010011" "01111111";
    //
    // LSB -> MSB, little endian. Reverse byte-order of testBytesLSB64_be and whole bit-reverse of testBytesMSB64_be
    //
    constexpr static uint8_t testBytesLSB64_le[] = { 0x7F_u8, 0x53_u8, 0xF5_u8, 0x7B_u8, 0x7F_u8, 0xF5_u8, 0x7B_u8, 0x5F_u8 };
    constexpr static uint64_t testIntLSB64_le = 0x7F53F57B7FF57B5F_u64;
    // 01111111 01010011 11110101 01111011 01111111 11110101 01111011 01011111
    constexpr static std::string_view testStringsLSB64_le[] = { "01111111" "01010011" "11110101" "01111011"
                                                                "01111111" "11110101" "01111011" "01011111" };
    constexpr static std::string_view testStringLSB64_le = "01111111" "01010011" "11110101" "01111011" "01111111" "11110101" "01111011" "01011111";

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
    static void dumpData(const std::string& prefix, jau::io::ByteStream& data, size_t len) {
        fprintf(stderr, "%s: Dump %s\n", prefix.c_str(), data.toString().c_str());
        bool err = false;
        size_t p = data.position();
        for( size_t i = 0; i < len && !err; ) {
            fprintf(stderr, "%s: %03zu: ", prefix.c_str(), i);
            for( size_t j = 0; j < 8 && i < len && !err; ++j, ++i ) {
                uint8_t v;
                err = data.read(v);
                fprintf(stderr, "%s, ", toHexBinaryString(v, 8).c_str());
            }
            fprintf(stderr, "\n");
        }
        if( p != data.seek(p) ) {
            throw jau::RuntimeException("couldn't rewind stream to "+std::to_string(p)+": "+data.toString(), E_FILE_LINE);
        }
    }
    static void dumpData(const std::string& prefix, jau::io::ByteStream& data) { dumpData(prefix, data, data.remaining()); }

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
        const std::string& s0 = jau::toBitString(v, jau::bit_order_t::msb, jau::PrefixOpt::none, bitCount);
        // std::string padding(strZeroPadding, bitCount > s0.length() ? bitCount - s0.length() : 0);
        // return padding + s0;
        return s0;
    }
    static std::string toHexBinaryString(const uint64_t v, const int bitCount) {
        return jau::toHexString(v) + " (" + toBinaryString(v, bitCount) + ")";
    }
};
