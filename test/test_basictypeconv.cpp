/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021 Gothel Software e.K.
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

#include <jau/test/catch2_ext.hpp>

#include <jau/basic_types.hpp>

static constexpr inline bool VERBOSE = false;

using namespace jau::int_literals;

/**
 * Test private impl namespace
 */
namespace test_impl {
    template<class Dummy_type>
    constexpr bool isLittleEndian2_impl(std::enable_if_t<jau::has_endian_little_v<Dummy_type>, bool> = true) noexcept {
        return true;
    }

    template<class Dummy_type>
    constexpr bool isLittleEndian2_impl(std::enable_if_t<!jau::has_endian_little_v<Dummy_type>, bool> = true) noexcept {
        return false;
    }
}

/**
 * Just demonstrating usage of our type-traits
 * in a convenient API manner w/o requiring to add the dummy template type.
 */
constexpr bool isLittleEndian2() noexcept {
    return test_impl::isLittleEndian2_impl<bool>();
}


TEST_CASE( "Endianess Test 00", "[endian]" ) {
    fprintf(stderr, "********************************************************************************\n");
    fprintf(stderr, "is_builtin_bit_cast_available: %d\n", jau::is_builtin_bit_cast_available());
    fprintf(stderr, "endian: %s\n", jau::to_string(jau::endian::native).c_str());
    fprintf(stderr, "********************************************************************************\n");

    const bool cpp_is_little =
        #if BYTE_ORDER == LITTLE_ENDIAN
            true;
        #else
            false;
        #endif
    const bool cpp_is_big =
        #if BYTE_ORDER == BIG_ENDIAN
            true;
        #else
            false;
        #endif
    const bool is_little = jau::endian::little == jau::endian::native;
    const bool is_big = jau::endian::big == jau::endian::native;
    REQUIRE( cpp_is_little == is_little );
    REQUIRE( cpp_is_little == jau::isLittleEndian() );
    REQUIRE( cpp_is_big == is_big );
    REQUIRE( is_little == isLittleEndian2());
}

template<typename Value_type>
static void print(const Value_type a) {
    const uint8_t * pa = reinterpret_cast<const uint8_t *>(&a);
    for(std::size_t i=0; i<sizeof(Value_type); i++) {
        fprintf(stderr, "a[%zu] 0x%X, ", i, pa[i]);
    }
}

template<typename Value_type>
static bool compare(const Value_type a, const Value_type b) {
    const uint8_t * pa = reinterpret_cast<const uint8_t *>(&a);
    const uint8_t * pb = reinterpret_cast<const uint8_t *>(&b);
    bool res = true;
    for(std::size_t i=0; i<sizeof(Value_type) && res; i++) {
        res = pa[i] == pb[i];
        if( !res ) {
            fprintf(stderr, "pa[%zu] 0x%X != pb[%zu] 0x%X\n", i, pa[i], i, pb[i]);
        }
    }
    return res;
}

template<typename Value_type>
static void test_byteorder(const Value_type v_cpu,
                           const Value_type v_le,
                           const Value_type v_be)
{
    if( VERBOSE ) {
        fprintf(stderr, "test_byteorder: sizeof %zu; platform littleEndian %d", sizeof(Value_type), jau::isLittleEndian());
        fprintf(stderr, "\ncpu: %s: ", jau::to_hexstring(v_cpu).c_str()); print(v_cpu);
        fprintf(stderr, "\nle_: %s: ", jau::to_hexstring(v_le).c_str()); print(v_le);
        fprintf(stderr, "\nbe_: %s: ", jau::to_hexstring(v_be).c_str()); print(v_be);
        fprintf(stderr, "\n");
    }
    {
        Value_type r1_le = jau::bswap(v_be);
        REQUIRE( r1_le == v_le );
        Value_type r1_be = jau::bswap(v_le);
        REQUIRE( r1_be == v_be );
    }
    {
        #if BYTE_ORDER == LITTLE_ENDIAN
            REQUIRE( compare(v_le, v_cpu) == true );
            Value_type r1_cpu = jau::bswap(v_be);
            REQUIRE( r1_cpu == v_cpu );
        #else
            REQUIRE( compare(v_be, v_cpu) == true );
            Value_type r1_cpu = jau::bswap(v_le);
            REQUIRE( r1_cpu == v_cpu );
        #endif
    }
    {
        Value_type r1_cpu = jau::le_to_cpu(v_le);
        Value_type r2_cpu = jau::be_to_cpu(v_be);
        REQUIRE( r1_cpu == v_cpu );
        REQUIRE( r2_cpu == v_cpu );
    }
}

static uint16_t compose(const uint8_t n1, const uint8_t n2) {
    uint16_t dest;
    uint8_t * p_dest = reinterpret_cast<uint8_t*>(&dest);
    p_dest[0] = n1;
    p_dest[1] = n2;
    return dest;
}
static uint32_t compose(const uint8_t n1, const uint8_t n2, const uint8_t n3, const uint8_t n4) {
    uint32_t dest;
    uint8_t * p_dest = reinterpret_cast<uint8_t*>(&dest);
    p_dest[0] = n1;
    p_dest[1] = n2;
    p_dest[2] = n3;
    p_dest[3] = n4;
    return dest;
}
static uint64_t compose(const uint8_t n1, const uint8_t n2, const uint8_t n3, const uint8_t n4,
                        const uint8_t n5, const uint8_t n6, const uint8_t n7, const uint8_t n8) {
    uint64_t dest;
    uint8_t * p_dest = reinterpret_cast<uint8_t*>(&dest);
    p_dest[0] = n1;
    p_dest[1] = n2;
    p_dest[2] = n3;
    p_dest[3] = n4;
    p_dest[4] = n5;
    p_dest[5] = n6;
    p_dest[6] = n7;
    p_dest[7] = n8;
    return dest;
}

template<typename Value_type>
static Value_type compose(const uint8_t lowest_value, const bool little_endian) {
    Value_type dest;
    uint8_t * p_dest = reinterpret_cast<uint8_t*>(&dest);
    uint8_t byte_value = lowest_value;
    if( little_endian ) {
        for(size_t i=0; i<sizeof(dest); i++, byte_value++) {
            p_dest[i] = byte_value;
        }
    } else {
        for(ssize_t i=sizeof(dest)-1; i>=0; i--, byte_value++) {
            p_dest[i] = byte_value;
        }
    }
    return dest;
}

TEST_CASE( "Integer Type Byte Order Test 01", "[byteorder][bswap]" ) {
    {
        uint16_t cpu = 0x3210U;
        uint16_t le = compose(0x10, 0x32); // stream: 1032
        uint16_t be = compose(0x32, 0x10); // stream: 3210
        test_byteorder(cpu, le, be);
    }
    {
        uint32_t cpu = 0x76543210U;
        uint32_t le = compose(0x10, 0x32, 0x54, 0x76); // stream: 10325476
        uint32_t be = compose(0x76, 0x54, 0x32, 0x10); // stream: 76543210
        test_byteorder(cpu, le, be);
    }
    {
        uint64_t cpu = 0xfedcba9876543210ULL;
        uint64_t le = compose(0x10, 0x32, 0x54, 0x76, 0x98, 0xba, 0xdc, 0xfe); // stream: 1032547698badcfe
        uint64_t be = compose(0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10); // stream: fedcba9876543210
        test_byteorder(cpu, le, be);
    }
    {
        jau::uint128_t le = compose<jau::uint128_t>(0x01, true /* little_endian */);
        jau::uint128_t be = compose<jau::uint128_t>(0x01, false /* little_endian */);
        jau::uint128_t cpu = jau::isLittleEndian() ? le : be;
        test_byteorder(cpu, le, be);
    }
    {
        jau::uint192_t le = compose<jau::uint192_t>(0x01, true /* little_endian */);
        jau::uint192_t be = compose<jau::uint192_t>(0x01, false /* little_endian */);
        jau::uint192_t cpu = jau::isLittleEndian() ? le : be;
        test_byteorder(cpu, le, be);
    }
    {
        jau::uint256_t le = compose<jau::uint256_t>(0x01, true /* little_endian */);
        jau::uint256_t be = compose<jau::uint256_t>(0x01, false /* little_endian */);
        jau::uint256_t cpu = jau::isLittleEndian() ? le : be;
        test_byteorder(cpu, le, be);
    }
}

template<typename Value_type>
static void test_value_cpu(const Value_type v1, const Value_type v2, const Value_type v3) {
    uint8_t buffer[3 * sizeof(Value_type)];
    jau::put_value(buffer, sizeof(Value_type)*0, v1);
    jau::put_value(buffer, sizeof(Value_type)*1, v2);
    jau::put_value(buffer, sizeof(Value_type)*2, v3);
    const Value_type r1 = jau::get_value<Value_type>(buffer, sizeof(Value_type)*0);
    const Value_type r2 = jau::get_value<Value_type>(buffer, sizeof(Value_type)*1);
    const Value_type r3 = jau::get_value<Value_type>(buffer, sizeof(Value_type)*2);
    REQUIRE( r1 == v1);
    REQUIRE( r2 == v2);
    REQUIRE( r3 == v3);
}

TEST_CASE( "Integer Get/Put in CPU Byte Order Test 02", "[byteorder][get][put]" ) {
    {
        uint8_t a = 0x01, b = 0x11, c = 0xff;
        test_value_cpu(a, b, c);
    }
    {
        uint16_t a = 0x0123, b = 0x1122, c = 0xffee;
        test_value_cpu(a, b, c);
    }
    {
        uint32_t a = 0x01234567U, b = 0x11223344U, c = 0xffeeddccU;
        test_value_cpu(a, b, c);
    }
    {
        uint64_t a = 0x0123456789abcdefULL, b = 0x1122334455667788ULL, c = 0xffeeddcc99887766ULL;
        test_value_cpu(a, b, c);
    }
    {
        jau::uint128_t a = compose<jau::uint128_t>(0x01, jau::isLittleEndian());
        jau::uint128_t b = compose<jau::uint128_t>(0x20, jau::isLittleEndian());
        jau::uint128_t c = compose<jau::uint128_t>(0x40, jau::isLittleEndian());
        test_value_cpu(a, b, c);
    }
    {
        jau::uint192_t a = compose<jau::uint192_t>(0x01, jau::isLittleEndian());
        jau::uint192_t b = compose<jau::uint192_t>(0x20, jau::isLittleEndian());
        jau::uint192_t c = compose<jau::uint192_t>(0x40, jau::isLittleEndian());
        test_value_cpu(a, b, c);
    }
    {
        jau::uint256_t a = compose<jau::uint256_t>(0x01, jau::isLittleEndian());
        jau::uint256_t b = compose<jau::uint256_t>(0x20, jau::isLittleEndian());
        jau::uint256_t c = compose<jau::uint256_t>(0x40, jau::isLittleEndian());
        test_value_cpu(a, b, c);
    }
}

template<typename Value_type>
static void test_value_littlebig(const Value_type v_cpu, const Value_type v_le, const Value_type v_be) {
    if( VERBOSE ) {
        fprintf(stderr, "test_value_littlebig: sizeof %zu; platform littleEndian %d", sizeof(Value_type), jau::isLittleEndian());
        fprintf(stderr, "\ncpu: %s: ", jau::to_hexstring(v_cpu).c_str()); print(v_cpu);
        fprintf(stderr, "\nle_: %s: ", jau::to_hexstring(v_le).c_str()); print(v_le);
        fprintf(stderr, "\nbe_: %s: ", jau::to_hexstring(v_be).c_str()); print(v_be);
        fprintf(stderr, "\n");
    }
    uint8_t buffer[2 * sizeof(Value_type)];

    jau::put_value(buffer, sizeof(Value_type)*0, v_cpu, true /* little_endian */);
    jau::put_value(buffer, sizeof(Value_type)*1, v_cpu, false /* little_endian */);

    const Value_type rle_raw = jau::get_value<Value_type>(buffer, sizeof(Value_type)*0);
    const Value_type rle_cpu = jau::get_value<Value_type>(buffer, sizeof(Value_type)*0, true /* little_endian */);
    REQUIRE( rle_raw == v_le);
    REQUIRE( rle_cpu == v_cpu);

    const Value_type rbe_raw = jau::get_value<Value_type>(buffer, sizeof(Value_type)*1);
    const Value_type rbe_cpu = jau::get_value<Value_type>(buffer, sizeof(Value_type)*1, false /* little_endian */);
    REQUIRE( rbe_raw == v_be);
    REQUIRE( rbe_cpu == v_cpu);
}

TEST_CASE( "Integer Get/Put in explicit Byte Order Test 03", "[byteorder][get][put]" ) {
    {
        uint16_t cpu = 0x3210U;
        uint16_t le = compose(0x10, 0x32); // stream: 1032
        uint16_t be = compose(0x32, 0x10); // stream: 3210
        test_value_littlebig(cpu, le, be);
    }
    {
        uint32_t cpu = 0x76543210U;
        uint32_t le = compose(0x10, 0x32, 0x54, 0x76); // stream: 10325476
        uint32_t be = compose(0x76, 0x54, 0x32, 0x10); // stream: 76543210
        test_value_littlebig(cpu, le, be);
    }
    {
        uint64_t cpu = 0xfedcba9876543210ULL;
        uint64_t le = compose(0x10, 0x32, 0x54, 0x76, 0x98, 0xba, 0xdc, 0xfe); // stream: 1032547698badcfe
        uint64_t be = compose(0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10); // stream: fedcba9876543210
        test_value_littlebig(cpu, le, be);
    }
    {
        jau::uint128_t le = compose<jau::uint128_t>(0x01, true /* little_endian */);
        jau::uint128_t be = compose<jau::uint128_t>(0x01, false /* little_endian */);
        jau::uint128_t cpu = jau::isLittleEndian() ? le : be;
        test_value_littlebig(cpu, le, be);
    }
    {
        jau::uint192_t le = compose<jau::uint192_t>(0x01, true /* little_endian */);
        jau::uint192_t be = compose<jau::uint192_t>(0x01, false /* little_endian */);
        jau::uint192_t cpu = jau::isLittleEndian() ? le : be;
        test_value_littlebig(cpu, le, be);
    }
    {
        jau::uint256_t le = compose<jau::uint256_t>(0x01, true /* little_endian */);
        jau::uint256_t be = compose<jau::uint256_t>(0x01, false /* little_endian */);
        jau::uint256_t cpu = jau::isLittleEndian() ? le : be;
        test_value_littlebig(cpu, le, be);
    }
}

TEST_CASE( "HexString from and to byte vector conversion - Test 04", "[hexstring]" ) {
    const std::vector<uint8_t> lalaSink1 = { 0x1a, 0x1b, 0x2a, 0x2b, 0xff };
    {
        const std::string value_s0 = "1a1b2a2bff";
        const std::string value_s1 = jau::bytesHexString(lalaSink1.data(), 0, lalaSink1.size(), true /* lsbFirst */);
        std::vector<uint8_t> lalaSink2;
        jau::hexStringBytes(lalaSink2, value_s1, true /* lsbFirst */, false);
        const std::string value_s2 = jau::bytesHexString(lalaSink2.data(), 0, lalaSink2.size(), true /* lsbFirst */);
        REQUIRE( value_s0 == value_s1 );
        REQUIRE( value_s0 == value_s2 );
        // Assert.assertArrayEquals(lalaSink1, lalaSink2);
    }
    {
        const std::string value_s0 = "0xff2b2a1b1a";
        const std::string value_s1 = jau::bytesHexString(lalaSink1.data(), 0, lalaSink1.size(), false /* lsbFirst */);
        std::vector<uint8_t> lalaSink2;
        jau::hexStringBytes(lalaSink2, value_s1, false /* lsbFirst */, true);
        const std::string value_s2 = jau::bytesHexString(lalaSink2.data(), 0, lalaSink2.size(), false /* lsbFirst */);
        REQUIRE( value_s0 == value_s1 );
        REQUIRE( value_s0 == value_s2 );
        // Assert.assertArrayEquals(lalaSink1, lalaSink2);
    }
}

TEST_CASE( "Integer Type Test Test 05", "[integer][type]" ) {
    REQUIRE( 3_i8 == (int8_t)3 );
    REQUIRE( 3_u8 == (uint8_t)3 );

    REQUIRE( 3_i16 == (int16_t)3 );
    REQUIRE( 3_u16 == (uint16_t)3 );

    REQUIRE( 3_i32 == (int32_t)3 );
    REQUIRE( 3_u32 == (uint32_t)3 );

    REQUIRE( 3_i64 == (int64_t)3 );
    REQUIRE( 3_u64 == (uint64_t)3 );

    REQUIRE( 3_iz == (ssize_t)3 );
    REQUIRE( 3_uz == (size_t)3 );

    REQUIRE( 3_inz == (jau::snsize_t)3 );
    REQUIRE( 3_unz == (jau::nsize_t)3 );
}

static void testRadix32(const int base) {
    {
        // UTF-8 (or codepage 437) <-> ASCII collision
        const std::string s1 = "Ç";
        const std::string s2 = "é";
        fprintf(stderr, "test.s1 '%s' %03d, %03d, size %zu\n", s1.c_str(), s1[0], (int)(0xFF & s1[0]), sizeof(s1[0]));
        fprintf(stderr, "test.s2 '%s' %03d, %03d, size %zu\n", s2.c_str(), s2[0], (int)(0xFF & s2[0]), sizeof(s2[0]));
    }
    {
        // UTF-8 (or codepage 437) <-> ASCII collision
        const std::wstring s1 = L"Ç";
        const std::wstring s2 = L"é";
        fprintf(stderr, "test.s1 %03d, %03d, size %zu\n", s1[0], (int)(0xFFFF & s1[0]), sizeof(s1[0]));
        fprintf(stderr, "test.s2 %03d, %03d, size %zu\n", s2[0], (int)(0xFFFF & s2[0]), sizeof(s2[0]));
    }

    const std::string r1_max = jau::dec_to_radix(base-1, base, 3, '0');
    const std::string r1_max_s = jau::dec_to_radix(base-1, base);
    const std::string r3_max = r1_max_s + r1_max_s + r1_max_s;
    const int min = jau::radix_to_dec("0", base);
    const int max = jau::radix_to_dec(r3_max, base);

    fprintf(stderr, "Test base %d: [%d .. %d] <-> ['%s' .. '%s'], %d years (max/365d) \n",
            base, min, max, jau::dec_to_radix(min, base).c_str(), jau::dec_to_radix(max, base).c_str(), (max/365));

    REQUIRE(0 == min);
    REQUIRE(0 == jau::radix_to_dec("000", base));
    REQUIRE("0" == jau::dec_to_radix(0, base));
    REQUIRE("000" == jau::dec_to_radix(0, base, 3, '0'));

    REQUIRE(1 == jau::radix_to_dec("001", base));
    REQUIRE("1" == jau::dec_to_radix(1, base));
    REQUIRE("001" == jau::dec_to_radix(1, base, 3, '0'));
    {
        const int v0_d = jau::radix_to_dec(r1_max, base);
        const std::string v1_s = jau::dec_to_radix(base-1, base, 3, '0');
        REQUIRE(r1_max == v1_s);
        REQUIRE(base-1 == v0_d);
    }
    {
        const int v0_d = jau::radix_to_dec(r3_max, base);
        const std::string v1_s = jau::dec_to_radix(max, base, 3, '0');
        REQUIRE(r3_max == v1_s);
        REQUIRE(max == v0_d);
    }
    for(int iter=min; iter<=max; ++iter) {
        const std::string rad = jau::dec_to_radix(iter, base, 3, '0');
        const int dec = jau::radix_to_dec(rad, base);
#if 0
        fprintf(stderr, "test base %d: iter %d, rad '%s' %03d %03d %03d, dec %d\n",
                base, iter, rad.c_str(), (int)(0xFF & rad[0]), (int)(0xFF & rad[1]), (int)(0xFF & rad[2]), dec);
#endif
        REQUIRE(iter == dec);
    }
}

static void testRadix64(const int base, const int64_t min, const int64_t max) {
    const int padding = 9;
    const std::string r1_max = jau::dec_to_radix(base-1, base, padding, '0');

    fprintf(stderr, "Test base %d: [%" PRIi64 " .. %" PRIi64 "] <-> ['%s' .. '%s'], %" PRIi64 " years (max/365d) \n",
            base, min, max, jau::dec_to_radix(min, base).c_str(), jau::dec_to_radix(max, base).c_str(), (max/365));

    REQUIRE(0 == jau::radix_to_dec("000", base));
    REQUIRE("0" == jau::dec_to_radix(0, base));

    REQUIRE(1 == jau::radix_to_dec("001", base));
    REQUIRE("1" == jau::dec_to_radix(1, base));
    {
        const int64_t v0_d = jau::radix_to_dec(r1_max, base);
        const std::string v1_s = jau::dec_to_radix(base-1, base, padding, '0');
        REQUIRE(r1_max == v1_s);
        REQUIRE(base-1 == v0_d);
    }
    for(int64_t iter=std::max(0_i64, min-1); iter<max; ) {
        ++iter;
        const std::string rad = jau::dec_to_radix(iter, base, padding, '0');
        const int64_t dec = jau::radix_to_dec(rad, base);
#if 0
        fprintf(stderr, "test base %d: iter %" PRIi64 ", rad '%s', dec %" PRIi64 "\n", base, iter, rad.c_str(), dec);
#endif
        REQUIRE(iter == dec);
    }
}

TEST_CASE( "Radix Base 62 Test 05", "[integer][type]" ) {
    testRadix32(62);
    testRadix64(62, 0x7fffff00, 0x80000100_i64);
    testRadix64(62, 0xFFFFFFF0_i64, 0x100000010_i64);
    testRadix64(62, 0x7FFFFFFFFFFFFFF0_i64, 0x7FFFFFFFFFFFFFFF_i64);
}

TEST_CASE( "Radix Base 82 Test 05", "[integer][type]" ) {
    testRadix32(82);
    testRadix64(82, 0x7fffff00, 0x80000100_i64);
    testRadix64(82, 0xFFFFFFF0_i64, 0x100000010_i64);
    testRadix64(82, 0x7FFFFFFFFFFFFFF0_i64, 0x7FFFFFFFFFFFFFFF_i64);
}

// TEST_CASE( "Radix Base 143 Test 05", "[integer][type]" ) {
//    testRadix(143);
// }
