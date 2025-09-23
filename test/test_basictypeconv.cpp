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

#include <jau/basic_types.hpp>
#include <jau/byte_util.hpp>
#include <jau/cpp_lang_util.hpp>
#include <jau/string_util.hpp>
#include <jau/test/catch2_ext.hpp>

extern "C" {
    // to test jau::endian_t::native against BYTE_ORDER macro
    #ifndef BYTE_ORDER
        #include <endian.h>
    #endif
}

static constexpr inline bool VERBOSE = false;

using namespace jau::int_literals;

/**
 * Test private impl namespace
 */
namespace test_impl {
    template<class Dummy_type>
    constexpr static bool isLittleEndian2_impl(std::enable_if_t<jau::has_endian_little_v<Dummy_type>, bool> = true) noexcept {
        return true;
    }

    template<class Dummy_type>
    constexpr static bool isLittleEndian2_impl(std::enable_if_t<!jau::has_endian_little_v<Dummy_type>, bool> = true) noexcept {
        return false;
    }
}  // namespace test_impl

/**
 * Just demonstrating usage of our type-traits
 * in a convenient API manner w/o requiring to add the dummy template type.
 */
constexpr static bool isLittleEndian2() noexcept {
    return test_impl::isLittleEndian2_impl<bool>();
}


TEST_CASE( "Endianess Test 00", "[endian]" ) {
    fprintf(stderr, "********************************************************************************\n");
    fprintf(stderr, "is_builtin_bit_cast_available: %d\n", jau::has_builtin_bit_cast());
    fprintf(stderr, "endian: %s\n", jau::to_string(jau::endian_t::native).c_str());
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
    const bool is_little = jau::endian_t::little == jau::endian_t::native;
    const bool is_big = jau::endian_t::big == jau::endian_t::native;
    REQUIRE( cpp_is_little == is_little );
    REQUIRE( cpp_is_little == jau::is_little_endian() );
    REQUIRE( cpp_is_big == is_big );
    REQUIRE( is_little == isLittleEndian2());
}

template<typename Value_type>
static void print(const Value_type a) {
    const uint8_t *pa = reinterpret_cast<const uint8_t *>(&a);
    for ( std::size_t i = 0; i < sizeof(Value_type); i++ ) {
        fprintf(stderr, "a[%zu] 0x%X, ", i, pa[i]);
    }
}

template<typename Value_type>
static bool compare_values(const Value_type a, const Value_type b) {
    const uint8_t *pa = reinterpret_cast<const uint8_t *>(&a);
    const uint8_t *pb = reinterpret_cast<const uint8_t *>(&b);
    bool res = true;
    for ( std::size_t i = 0; i < sizeof(Value_type) && res; i++ ) {
        res = pa[i] == pb[i];
        if ( !res ) {
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
        fprintf(stderr, "test_byteorder: sizeof %zu; platform littleEndian %d", sizeof(Value_type), jau::is_little_endian());
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
            REQUIRE( compare_values(v_le, v_cpu) == true );
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

static uint16_t composeU16(const uint8_t n1, const uint8_t n2) {
    uint16_t dest;
    uint8_t *p_dest = reinterpret_cast<uint8_t *>(&dest);
    p_dest[0] = n1;
    p_dest[1] = n2;
    return dest;
}
static int16_t composeI16(const uint8_t n1, const uint8_t n2) {
    int16_t dest;
    uint8_t *p_dest = reinterpret_cast<uint8_t *>(&dest);
    p_dest[0] = n1;
    p_dest[1] = n2;
    return dest;
}
static uint32_t composeU32(const uint8_t n1, const uint8_t n2, const uint8_t n3, const uint8_t n4) {
    uint32_t dest;
    uint8_t *p_dest = reinterpret_cast<uint8_t *>(&dest);
    p_dest[0] = n1;
    p_dest[1] = n2;
    p_dest[2] = n3;
    p_dest[3] = n4;
    return dest;
}
static int32_t composeI32(const uint8_t n1, const uint8_t n2, const uint8_t n3, const uint8_t n4) {
    int32_t dest;
    uint8_t *p_dest = reinterpret_cast<uint8_t *>(&dest);
    p_dest[0] = n1;
    p_dest[1] = n2;
    p_dest[2] = n3;
    p_dest[3] = n4;
    return dest;
}
static uint64_t composeU64(const uint8_t n1, const uint8_t n2, const uint8_t n3, const uint8_t n4,
                           const uint8_t n5, const uint8_t n6, const uint8_t n7, const uint8_t n8) {
    uint64_t dest;
    uint8_t *p_dest = reinterpret_cast<uint8_t *>(&dest);
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
static int64_t composeI64(const uint8_t n1, const uint8_t n2, const uint8_t n3, const uint8_t n4,
                          const uint8_t n5, const uint8_t n6, const uint8_t n7, const uint8_t n8) {
    int64_t dest;
    uint8_t *p_dest = reinterpret_cast<uint8_t *>(&dest);
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
static Value_type compose(const uint8_t lowest_value, const jau::lb_endian_t le_or_be) {
    Value_type dest;
    uint8_t *p_dest = reinterpret_cast<uint8_t *>(&dest);
    uint8_t byte_value = lowest_value;
    if ( jau::is_little_endian(le_or_be) ) {
        for ( size_t i = 0; i < sizeof(dest); i++, byte_value++ ) {
            p_dest[i] = byte_value;
        }
    } else {
        for ( ssize_t i = sizeof(dest) - 1; i >= 0; i--, byte_value++ ) {
            p_dest[i] = byte_value;
        }
    }
    return dest;
}
template<typename Value_type>
static Value_type compose(const uint8_t lowest_value, const jau::endian_t le_or_be) {
    return compose<Value_type>(lowest_value, to_lb_endian(le_or_be));
}

TEST_CASE("Integer Type Byte Order Test 01", "[byteorder][bswap]") {
    {
        constexpr uint16_t cpu = 0x3210U;
        uint16_t le = composeU16(0x10, 0x32);  // stream: 1032
        uint16_t be = composeU16(0x32, 0x10);  // stream: 3210
        test_byteorder(cpu, le, be);
    }
    {
        constexpr uint16_t cpu = 0xFEDCU;
        uint16_t le = composeU16(0xDC, 0xFE);  // stream: DCFE
        uint16_t be = composeU16(0xFE, 0xDC);  // stream: FEDC
        test_byteorder(cpu, le, be);
    }
    {
        constexpr int16_t cpu = 0x3210;
        int16_t le = composeI16(0x10, 0x32);  // stream: 1032
        int16_t be = composeI16(0x32, 0x10);  // stream: 3210
        test_byteorder(cpu, le, be);
    }
    {
        constexpr int16_t cpu = -292_i16;     // int16_t(0xFEDC);
        int16_t le = composeI16(0xDC, 0xFE);  // stream: DCFE
        int16_t be = composeI16(0xFE, 0xDC);  // stream: FEDC
        test_byteorder(cpu, le, be);
    }
    {
        constexpr uint32_t cpu = 0x76543210U;
        uint32_t le = composeU32(0x10, 0x32, 0x54, 0x76);  // stream: 10325476
        uint32_t be = composeU32(0x76, 0x54, 0x32, 0x10);  // stream: 76543210
        test_byteorder(cpu, le, be);
    }
    {
        constexpr uint32_t cpu = 0xFEDCBA98U;
        uint32_t le = composeU32(0x98, 0xBA, 0xDC, 0xFE);  // stream: 98BADCFE
        uint32_t be = composeU32(0xFE, 0xDC, 0xBA, 0x98);  // stream: FEDCBA98
        test_byteorder(cpu, le, be);
    }
    {
        constexpr int32_t cpu = int32_t(0x76543210);
        int32_t le = composeI32(0x10, 0x32, 0x54, 0x76);  // stream: 10325476
        int32_t be = composeI32(0x76, 0x54, 0x32, 0x10);  // stream: 76543210
        test_byteorder(cpu, le, be);
    }
    {
        constexpr int32_t cpu = -19088744_i32;            // int32_t(0xFEDCBA98U);
        int32_t le = composeI32(0x98, 0xBA, 0xDC, 0xFE);  // stream: 98BADCFE
        int32_t be = composeI32(0xFE, 0xDC, 0xBA, 0x98);  // stream: FEDCBA98
        test_byteorder(cpu, le, be);
    }
    {
        constexpr uint64_t cpu = 0xfedcba9876543210ULL;
        uint64_t le = composeU64(0x10, 0x32, 0x54, 0x76, 0x98, 0xba, 0xdc, 0xfe);  // stream: 1032547698badcfe
        uint64_t be = composeU64(0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10);  // stream: fedcba9876543210
        test_byteorder(cpu, le, be);
    }
    {
        constexpr int64_t cpu = -81985529216486896_i64;                           // int64_t(0xfedcba9876543210ULL);
        int64_t le = composeI64(0x10, 0x32, 0x54, 0x76, 0x98, 0xba, 0xdc, 0xfe);  // stream: 1032547698badcfe
        int64_t be = composeI64(0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10);  // stream: fedcba9876543210
        test_byteorder(cpu, le, be);
    }
    {
        jau::uint128dp_t le = compose<jau::uint128dp_t>(0x01, jau::lb_endian_t::little);
        jau::uint128dp_t be = compose<jau::uint128dp_t>(0x01, jau::lb_endian_t::big);
        jau::uint128dp_t cpu = jau::is_little_endian() ? le : be;
        test_byteorder(cpu, le, be);
    }
    {
        jau::uint192dp_t le = compose<jau::uint192dp_t>(0x01, jau::lb_endian_t::little);
        jau::uint192dp_t be = compose<jau::uint192dp_t>(0x01, jau::lb_endian_t::big);
        jau::uint192dp_t cpu = jau::is_little_endian() ? le : be;
        test_byteorder(cpu, le, be);
    }
    {
        jau::uint256dp_t le = compose<jau::uint256dp_t>(0x01, jau::lb_endian_t::little);
        jau::uint256dp_t be = compose<jau::uint256dp_t>(0x01, jau::lb_endian_t::big);
        jau::uint256dp_t cpu = jau::is_little_endian() ? le : be;
        test_byteorder(cpu, le, be);
    }
}

template<typename Value_type>
static void test_value_cpu(const Value_type v1, const Value_type v2, const Value_type v3) {
    uint8_t buffer[3 * sizeof(Value_type)];
    jau::put_value(buffer + sizeof(Value_type) * 0, v1);
    jau::put_value(buffer + sizeof(Value_type) * 1, v2);
    jau::put_value(buffer + sizeof(Value_type) * 2, v3);
    const Value_type r1 = jau::get_value<Value_type>(buffer + sizeof(Value_type) * 0);
    const Value_type r2 = jau::get_value<Value_type>(buffer + sizeof(Value_type) * 1);
    const Value_type r3 = jau::get_value<Value_type>(buffer + sizeof(Value_type) * 2);
    REQUIRE(r1 == v1);
    REQUIRE(r2 == v2);
    REQUIRE(r3 == v3);
}

TEST_CASE("Integer Get/Put in CPU Byte Order Test 02", "[byteorder][get][put]") {
    {
        constexpr uint8_t a = 0x01, b = 0x11, c = 0xff;
        test_value_cpu(a, b, c);
    }
    {
        constexpr uint16_t a = 0x0123, b = 0x1122, c = 0xffee;
        test_value_cpu(a, b, c);
    }
    {
        constexpr int16_t a = 0x0123, b = 0x1122, c = -18_i16;  // (int16_t)0xffee;
        test_value_cpu(a, b, c);
    }
    {
        constexpr uint32_t a = 0x01234567U, b = 0x11223344U, c = 0xffeeddccU;
        test_value_cpu(a, b, c);
    }
    {
        constexpr int32_t a = 0x01234567, b = 0x11223344, c = -1122868_i32;  // (int32_t)0xffeeddcc;
        test_value_cpu(a, b, c);
    }
    {
        constexpr uint64_t a = 0x0123456789abcdefULL, b = 0x1122334455667788ULL, c = 0xffeeddcc99887766ULL;
        test_value_cpu(a, b, c);
    }
    {
        constexpr int64_t a = 0x0123456789abcdefLL, b = 0x1122334455667788LL, c = -4822678761867418_i64;  // (int64_t)0xffeeddcc99887766LL;
        test_value_cpu(a, b, c);
    }
    {
        jau::uint128dp_t a = compose<jau::uint128dp_t>(0x01, jau::endian_t::native);
        jau::uint128dp_t b = compose<jau::uint128dp_t>(0x20, jau::endian_t::native);
        jau::uint128dp_t c = compose<jau::uint128dp_t>(0x40, jau::endian_t::native);
        test_value_cpu(a, b, c);
    }
    {
        jau::uint192dp_t a = compose<jau::uint192dp_t>(0x01, jau::endian_t::native);
        jau::uint192dp_t b = compose<jau::uint192dp_t>(0x20, jau::endian_t::native);
        jau::uint192dp_t c = compose<jau::uint192dp_t>(0x40, jau::endian_t::native);
        test_value_cpu(a, b, c);
    }
    {
        jau::uint256dp_t a = compose<jau::uint256dp_t>(0x01, jau::endian_t::native);
        jau::uint256dp_t b = compose<jau::uint256dp_t>(0x20, jau::endian_t::native);
        jau::uint256dp_t c = compose<jau::uint256dp_t>(0x40, jau::endian_t::native);
        test_value_cpu(a, b, c);
    }
}

template<typename Value_type>
static void test_value_littlebig(const Value_type v_cpu, const Value_type v_le, const Value_type v_be) {
    if ( VERBOSE ) {
        fprintf(stderr, "test_value_littlebig: sizeof %zu; platform littleEndian %d", sizeof(Value_type), jau::is_little_endian());
        fprintf(stderr, "\ncpu: %s: ", jau::to_hexstring(v_cpu).c_str()); print(v_cpu);
        fprintf(stderr, "\nle_: %s: ", jau::to_hexstring(v_le).c_str()); print(v_le);
        fprintf(stderr, "\nbe_: %s: ", jau::to_hexstring(v_be).c_str()); print(v_be);
        fprintf(stderr, "\n");
    }
    uint8_t buffer[2 * sizeof(Value_type)];

    jau::put_value(buffer + sizeof(Value_type)*0, v_cpu, jau::lb_endian_t::little);
    jau::put_value(buffer + sizeof(Value_type)*1, v_cpu, jau::lb_endian_t::big);

    const Value_type rle_raw = jau::get_value<Value_type>(buffer + sizeof(Value_type)*0);
    const Value_type rle_cpu = jau::get_value<Value_type>(buffer + sizeof(Value_type)*0, jau::lb_endian_t::little);
    REQUIRE( rle_raw == v_le);
    REQUIRE( rle_cpu == v_cpu);

    const Value_type rbe_raw = jau::get_value<Value_type>(buffer + sizeof(Value_type)*1);
    const Value_type rbe_cpu = jau::get_value<Value_type>(buffer + sizeof(Value_type)*1, jau::lb_endian_t::big);
    REQUIRE( rbe_raw == v_be);
    REQUIRE( rbe_cpu == v_cpu);
}

TEST_CASE( "Integer Get/Put in explicit Byte Order Test 03", "[byteorder][get][put]" ) {
    {
        constexpr uint16_t cpu = 0x3210U;
        uint16_t le = composeU16(0x10, 0x32); // stream: 1032
        uint16_t be = composeU16(0x32, 0x10); // stream: 3210
        test_value_littlebig(cpu, le, be);
    }
    {
        constexpr uint16_t cpu = 0xFEDCU;
        uint16_t le = composeU16(0xDC, 0xFE); // stream: DCFE
        uint16_t be = composeU16(0xFE, 0xDC); // stream: FEDC
        test_value_littlebig(cpu, le, be);
    }
    {
        constexpr int16_t cpu = 0x3210;
        int16_t le = composeI16(0x10, 0x32); // stream: 1032
        int16_t be = composeI16(0x32, 0x10); // stream: 3210
        test_value_littlebig(cpu, le, be);
    }
    {
        constexpr int16_t cpu = -292_i16; // int16_t(0xFEDC);
        int16_t le = composeI16(0xDC, 0xFE); // stream: DCFE
        int16_t be = composeI16(0xFE, 0xDC); // stream: FEDC
        test_value_littlebig(cpu, le, be);
    }
    {
        constexpr uint32_t cpu = 0x76543210U;
        uint32_t le = composeU32(0x10, 0x32, 0x54, 0x76); // stream: 10325476
        uint32_t be = composeU32(0x76, 0x54, 0x32, 0x10); // stream: 76543210
        test_value_littlebig(cpu, le, be);
    }
    {
        constexpr uint32_t cpu = 0xFEDCBA98U;
        uint32_t le = composeU32(0x98, 0xBA, 0xDC, 0xFE); // stream: 98BADCFE
        uint32_t be = composeU32(0xFE, 0xDC, 0xBA, 0x98); // stream: FEDCBA98
        test_value_littlebig(cpu, le, be);
    }
    {
        constexpr int32_t cpu = int32_t(0x76543210);
        int32_t le = composeI32(0x10, 0x32, 0x54, 0x76); // stream: 10325476
        int32_t be = composeI32(0x76, 0x54, 0x32, 0x10); // stream: 76543210
        test_value_littlebig(cpu, le, be);
    }
    {
        constexpr int32_t cpu = -19088744_i32; // int32_t(0xFEDCBA98U);
        int32_t le = composeI32(0x98, 0xBA, 0xDC, 0xFE); // stream: 98BADCFE
        int32_t be = composeI32(0xFE, 0xDC, 0xBA, 0x98); // stream: FEDCBA98
        test_value_littlebig(cpu, le, be);
    }
    {
        constexpr uint64_t cpu = 0xfedcba9876543210ULL;
        uint64_t le = composeU64(0x10, 0x32, 0x54, 0x76, 0x98, 0xba, 0xdc, 0xfe); // stream: 1032547698badcfe
        uint64_t be = composeU64(0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10); // stream: fedcba9876543210
        test_value_littlebig(cpu, le, be);
    }
    {
        constexpr int64_t cpu = -81985529216486896_i64; // int64_t(0xfedcba9876543210ULL);
        int64_t le = composeI64(0x10, 0x32, 0x54, 0x76, 0x98, 0xba, 0xdc, 0xfe); // stream: 1032547698badcfe
        int64_t be = composeI64(0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10); // stream: fedcba9876543210
        test_value_littlebig(cpu, le, be);
    }
    {
        jau::uint128dp_t le = compose<jau::uint128dp_t>(0x01, jau::lb_endian_t::little);
        jau::uint128dp_t be = compose<jau::uint128dp_t>(0x01, jau::lb_endian_t::big);
        jau::uint128dp_t cpu = jau::is_little_endian() ? le : be;
        test_value_littlebig(cpu, le, be);
    }
    {
        jau::uint192dp_t le = compose<jau::uint192dp_t>(0x01, jau::lb_endian_t::little);
        jau::uint192dp_t be = compose<jau::uint192dp_t>(0x01, jau::lb_endian_t::big);
        jau::uint192dp_t cpu = jau::is_little_endian() ? le : be;
        test_value_littlebig(cpu, le, be);
    }
    {
        jau::uint256dp_t le = compose<jau::uint256dp_t>(0x01, jau::lb_endian_t::little);
        jau::uint256dp_t be = compose<jau::uint256dp_t>(0x01, jau::lb_endian_t::big);
        jau::uint256dp_t cpu = jau::is_little_endian() ? le : be;
        test_value_littlebig(cpu, le, be);
    }
}

TEST_CASE("HexString from and to byte vector conversion - Test 04", "[hexstring]") {
    {
        std::cout << "Little Endian Representation: " << std::endl;
        const std::vector<uint8_t> source_le = { 0x1a, 0x1b, 0x2a, 0x2b, 0xff };
        const uint64_t v0_cpu = 0x000000ff2b2a1b1aU;

        const std::string value_s0_le = "1a1b2a2bff";  // LE
        const std::string value_s1_le = jau::bytesHexString(source_le.data(), source_le.size(), true /* lsbFirst */);
        {
            std::vector<uint8_t> out;
            jau::hexStringBytes(out, value_s1_le, true);
            const uint64_t v_cpu = jau::le_to_cpu(*jau::pointer_cast<const uint64_t *>(out.data()));
            std::string v_cpu_s0 = jau::format_string("%lx", v_cpu);
            std::string v_cpu_s1 = jau::to_hexstring(v_cpu);
            std::string v_cpu_s2 = jau::to_string(v_cpu, 16);
            std::cout << "v0_le " << value_s1_le << ", is_le " << jau::is_little_endian() << std::endl;
            std::cout << "- out " << jau::to_string(out, 16) << std::endl;
            std::cout << "- v_cpu0 0x" << v_cpu_s0 << std::endl;
            std::cout << "- v_cpu1 " << v_cpu_s1 << std::endl;
            std::cout << "- v_cpu2 " << v_cpu_s2 << std::endl;
            REQUIRE(source_le == out);
            REQUIRE(v0_cpu == v_cpu);
            REQUIRE(v_cpu_s1 == v_cpu_s2);
        }
        const auto [v1_cpu, consumed1, complete1] = jau::from_hexstring(value_s1_le, true /* lsbFirst */);
        REQUIRE(true == complete1);
        REQUIRE(value_s1_le.length() == consumed1);

        std::vector<uint8_t> pass2_le;
        jau::hexStringBytes(pass2_le, value_s1_le, true /* lsbFirst */, jau::False());
        const std::string value_s2_le = jau::bytesHexString(pass2_le, true /* lsbFirst */);
        const auto [v2_cpu, consumed2, complete2] = jau::from_hexstring(value_s2_le, true /* lsbFirst */);
        REQUIRE(true == complete2);
        REQUIRE(value_s2_le.length() == consumed2);

        REQUIRE(value_s0_le == value_s1_le);
        REQUIRE(value_s0_le == value_s2_le);

        std::cout << "v0_le " << value_s1_le << " (2) " << value_s2_le << std::endl;
        {
            std::string v1_cpu_s = jau::to_hexstring(v1_cpu);
            std::cout << "v1_cpu_s " << v1_cpu_s << std::endl;
            std::string v2_cpu_s = jau::to_hexstring(v2_cpu);
            std::cout << "v2_cpu_s " << v2_cpu_s << std::endl;
        }
        REQUIRE(v0_cpu == v1_cpu);
        REQUIRE(v0_cpu == v2_cpu);

        REQUIRE(source_le == pass2_le);
        std::cout << std::endl;
    }
    {
        std::cout << "Big Endian Representation: " << std::endl;
        const std::vector<uint8_t> source_le = { 0x1a, 0x1b, 0x2a, 0x2b, 0xff };
        const uint64_t v0_cpu = 0x000000ff2b2a1b1aU;

        const std::string value_s0_be = "0xff2b2a1b1a";
        const std::string value_s1_be = jau::bytesHexString(source_le.data(), source_le.size(), false /* lsbFirst */);
        {
            std::vector<uint8_t> out;
            jau::hexStringBytes(out, value_s1_be, false);
            const uint64_t v_cpu = jau::le_to_cpu(*jau::pointer_cast<const uint64_t *>(out.data()));
            std::string v_cpu_s0 = jau::format_string("%lx", v_cpu);
            std::string v_cpu_s1 = jau::to_hexstring(v_cpu);
            std::string v_cpu_s2 = jau::to_string(v_cpu, 16);
            std::cout << "v0_be " << value_s1_be << ", is_le " << jau::is_little_endian() << std::endl;
            std::cout << "- out " << jau::to_string(out, 16) << std::endl;
            std::cout << "- v_cpu0 0x" << v_cpu_s0 << std::endl;
            std::cout << "- v_cpu1 " << v_cpu_s1 << std::endl;
            std::cout << "- v_cpu2 " << v_cpu_s2 << std::endl;
            REQUIRE(source_le == out);
            REQUIRE(v0_cpu == v_cpu);
            REQUIRE(v_cpu_s1 == v_cpu_s2);
        }
        const auto [v1_cpu, consumed1, complete1] = jau::from_hexstring(value_s1_be, false /* lsbFirst */);
        REQUIRE(true == complete1);
        REQUIRE(value_s1_be.length() == consumed1);

        std::vector<uint8_t> pass2_le;
        jau::hexStringBytes(pass2_le, value_s1_be, false /* lsbFirst */);
        const std::string value_s2_be = jau::bytesHexString(pass2_le.data(), pass2_le.size(), false /* lsbFirst */);
        const auto [v2_cpu, consumed2, complete2] = jau::from_hexstring(value_s2_be, false /* lsbFirst */);
        REQUIRE(true == complete2);
        REQUIRE(value_s2_be.length() == consumed2);
        REQUIRE(value_s0_be == value_s1_be);
        REQUIRE(value_s0_be == value_s2_be);

        std::cout << "v0_be " << value_s1_be << " (2) " << value_s2_be << std::endl;
        {
            std::string v1_cpu_s = jau::to_hexstring(v1_cpu);
            std::cout << "v1_cpu_s " << v1_cpu_s << std::endl;
            std::string v2_cpu_s = jau::to_hexstring(v2_cpu);
            std::cout << "v2_cpu_s " << v2_cpu_s << std::endl;
        }
        REQUIRE(v0_cpu == v1_cpu);
        REQUIRE(v0_cpu == v2_cpu);

        REQUIRE(source_le == pass2_le);
        std::cout << std::endl;
    }
    {
        // even digits
        std::cout << "Even digits (1): " << std::endl;
        const std::vector<uint8_t> v0_b = { 0x1a, 0x1b, 0x2a, 0x2b, 0xff };
        const uint64_t v0 = 0xff2b2a1b1aU;
        const std::string v0_s_msb = "0xff2b2a1b1a";
        const std::string v0_s_lsb = "1a1b2a2bff";
        std::cout << "v0   " << jau::to_hexstring(v0) << std::endl;
        std::cout << "v0_b " << jau::to_string(v0_b, 16) << std::endl;
        std::cout << "v0_s (msb) " << v0_s_msb << std::endl;
        std::cout << "v0_s (lsb)   " << v0_s_lsb << std::endl;

        std::vector<uint8_t> v1_b_msb;
        std::vector<uint8_t> v1_b_lsb;
        jau::hexStringBytes(v1_b_msb, v0_s_msb, false /* lsbFirst */);
        jau::hexStringBytes(v1_b_lsb, v0_s_lsb, true /* lsbFirst */);
        const std::string v1_bs_msb_str = jau::bytesHexString(v1_b_msb, false /* lsbFirst */);
        const std::string v1_bs_lsb_str = jau::bytesHexString(v1_b_lsb, false /* lsbFirst */);
        std::cout << "v1_b  (msb str) " << jau::to_string(v1_b_msb, 16) << std::endl;
        std::cout << "v1_bs (msb str) " << v1_bs_msb_str << std::endl;
        std::cout << "v1_b  (lsb str) " << jau::to_string(v1_b_lsb, 16) << std::endl;
        std::cout << "v1_bs (lsb str) " << v1_bs_lsb_str << std::endl;

        const auto [v1_msb, consumed1, complete1] = jau::from_hexstring(v0_s_msb, false /* lsbFirst */);
        REQUIRE(true == complete1);
        REQUIRE(v0_s_msb.length() == consumed1);

        const auto [v1_lsb, consumed2, complete2] = jau::from_hexstring(v0_s_lsb, true /* lsbFirst */);
        REQUIRE(true == complete2);
        REQUIRE(v0_s_lsb.length() == consumed2);

        std::cout << "v1   (msb) " << jau::to_hexstring(v1_msb) << std::endl;
        std::cout << "v1   (lsb) " << jau::to_hexstring(v1_lsb) << std::endl;

        REQUIRE(v0 == v1_msb);
        REQUIRE(v0 == v1_lsb);
        REQUIRE(v0_b == v1_b_msb);
        REQUIRE(v0_b == v1_b_lsb);
        std::cout << std::endl;
    }
    {
        // odd digits
        std::cout << "Odd digits (1): " << std::endl;
        const std::vector<uint8_t> v0_b_msb = { 0x1a, 0x1b, 0x2a, 0x2b, 0x0f };
        const std::vector<uint8_t> v0_b_lsb = { 0x1a, 0x1b, 0x2a, 0x2b, 0xf0 };
        const uint64_t v0_msb = 0x0f2b2a1b1aU;
        const uint64_t v0_lsb = 0xf02b2a1b1aU;
        const std::string v0_s_msb = "0xf2b2a1b1a";
        const std::string v0_s_lsb = "1a1b2a2bf";
        std::cout << "v0   (msb) " << jau::to_hexstring(v0_msb) << std::endl;
        std::cout << "v0_b (msb) " << jau::to_string(v0_b_msb, 16) << std::endl;
        std::cout << "v0_s (msb) " << v0_s_msb << std::endl;
        std::cout << "v0   (lsb) " << jau::to_hexstring(v0_lsb) << std::endl;
        std::cout << "v0_b (lsb) " << jau::to_string(v0_b_lsb, 16) << std::endl;
        std::cout << "v0_s (lsb) " << v0_s_lsb << std::endl;

        std::vector<uint8_t> v1_b_msb;
        std::vector<uint8_t> v1_b_lsb;
        jau::hexStringBytes(v1_b_msb, v0_s_msb, false /* lsbFirst */);
        jau::hexStringBytes(v1_b_lsb, v0_s_lsb, true /* lsbFirst */);
        const std::string v1_bs_msb_str = jau::bytesHexString(v1_b_msb, false /* lsbFirst */);
        const std::string v1_bs_lsb_str = jau::bytesHexString(v1_b_lsb, false /* lsbFirst */);
        std::cout << "v1_b  (msb str) " << jau::to_string(v1_b_msb, 16) << std::endl;
        std::cout << "v1_bs (msb str) " << v1_bs_msb_str << std::endl;
        std::cout << "v1_b  (lsb str) " << jau::to_string(v1_b_lsb, 16) << std::endl;
        std::cout << "v1_bs (lsb str) " << v1_bs_lsb_str << std::endl;

        const jau::UInt64SizeBoolTuple v1_msb = jau::from_hexstring(v0_s_msb, false);
        REQUIRE(true == v1_msb.b);
        const jau::UInt64SizeBoolTuple v1_lsb = jau::from_hexstring(v0_s_lsb, true);
        REQUIRE(true == v1_lsb.b);
        std::cout << "v1   (msb) " << jau::to_hexstring(v1_msb.v) << std::endl;
        std::cout << "v1   (lsb) " << jau::to_hexstring(v1_lsb.v) << std::endl;

        REQUIRE(v0_msb == v1_msb.v);
        REQUIRE(v0_lsb == v1_lsb.v);
        REQUIRE(v0_b_msb == v1_b_msb);
        REQUIRE(v0_b_lsb == v1_b_lsb);
        std::cout << std::endl;
    }
    {
        std::cout << "Even digits (2): " << std::endl;
        const uint64_t v0 = 0x000000ff2b2a1b1aU;
        std::string v0_s = jau::to_hexstring(v0);
        const jau::UInt64SizeBoolTuple v0_2 = jau::from_hexstring(v0_s);
        REQUIRE(true == v0_2.b);
        std::cout << "v0_s " << v0_s << std::endl;
        std::cout << "v0_2  " << jau::to_hexstring(v0_2.v) << std::endl;
        REQUIRE(v0 == v0_2.v);
        std::cout << std::endl;
    }
    {
        std::cout << "Even digits (3): " << std::endl;
        std::string v0_0s1 = "0xff2b2a1b1a";
        const uint64_t v0_0 = 0xff2b2a1b1aU;
        std::string v0_0s2 = jau::to_hexstring(v0_0);

        const jau::UInt64SizeBoolTuple i0_0s1 = jau::from_hexstring(v0_0s1);
        REQUIRE(true == i0_0s1.b);
        const jau::UInt64SizeBoolTuple i0_0s2 = jau::from_hexstring(v0_0s2);
        REQUIRE(true == i0_0s2.b);

        std::cout << "v0_0s  " << v0_0s1 << std::endl;
        std::cout << "v0_0s2 " << v0_0s2 << std::endl;

        std::cout << "i0_0s1 " << jau::to_hexstring(i0_0s1.v) << std::endl;
        std::cout << "i0_0s2 " << jau::to_hexstring(i0_0s2.v) << std::endl;

        REQUIRE(v0_0 == i0_0s1.v);
        REQUIRE(v0_0 == i0_0s2.v);
        std::cout << std::endl;
    }
    {
        std::cout << "Odd digits (3): " << std::endl;
        std::string v0_0s1 = "0xf2b2a1b1a";
        const uint64_t v0_0 = 0xf2b2a1b1aU;
        std::string v0_0s2 = jau::to_hexstring(v0_0);

        const auto [i0_0s1, len1, ok1] = jau::from_hexstring(v0_0s1);
        REQUIRE(true == ok1);
        const auto [i0_0s2, len2, ok2] = jau::from_hexstring(v0_0s2);
        REQUIRE(true == ok2);

        std::cout << "v0_0s  " << v0_0s1 << std::endl;
        std::cout << "v0_0s2 " << v0_0s2 << std::endl;

        std::cout << "i0_0s1 " << jau::to_hexstring(i0_0s1) << std::endl;
        std::cout << "i0_0s2 " << jau::to_hexstring(i0_0s2) << std::endl;

        REQUIRE(v0_0 == i0_0s1);
        REQUIRE(v0_0 == i0_0s2);
        std::cout << std::endl;
    }
}

TEST_CASE("BitString from and to byte vector conversion - Test 05", "[bitstring]") {
    {
        std::cout << "Little Endian Representation: " << std::endl;
        const std::vector<uint8_t> source_le = { 0x1a, 0x1b, 0x2a, 0x2b, 0xff };
        // const uint64_t v0_cpu = 0x000000ff2b2a1b1aU;
        const uint64_t v0_cpu = 0b1111111100101011001010100001101100011010U;

        // const uint64_t v0_le = 0x1a1b2a2bff;
        const std::string value_s0_le = "0001101000011011001010100010101111111111";  // LE
        const std::string value_s1_le = jau::toBitString(source_le.data(), source_le.size(), true /* lsbFirst */);
        {
            std::vector<uint8_t> out;
            jau::fromBitString(out, value_s1_le, true);
            const uint64_t v_cpu = jau::le_to_cpu(*jau::pointer_cast<const uint64_t *>(out.data()));
            std::string v_cpu_s1 = jau::toBitString(v_cpu);
            std::string v_cpu_s2 = jau::to_string(v_cpu, 2);
            std::cout << "v0_le " << value_s1_le << ", is_le " << jau::is_little_endian() << std::endl;
            std::cout << "- out " << jau::to_string(out, 16) << std::endl;
            std::cout << "- v_cpu1 " << v_cpu_s1 << std::endl;
            std::cout << "- v_cpu2 " << v_cpu_s2 << std::endl;
            REQUIRE(source_le == out);
            REQUIRE(v0_cpu == v_cpu);
            REQUIRE(v_cpu_s1 == v_cpu_s2);
        }
        const auto [v1_cpu, len1, ok1] = jau::fromBitString(value_s1_le, true /* lsbFirst */);

        std::vector<uint8_t> pass2_le;
        jau::fromBitString(pass2_le, value_s1_le, true /* lsbFirst */, jau::False());
        const std::string value_s2_le = jau::toBitString(pass2_le, true /* lsbFirst */);
        const auto [v2_cpu, len2, ok2] = jau::fromBitString(value_s2_le, true /* lsbFirst */);

        REQUIRE(value_s0_le == value_s1_le);
        REQUIRE(value_s0_le == value_s2_le);

        std::cout << "v0_le " << value_s1_le << " (2) " << value_s2_le << std::endl;
        {
            std::string v1_cpu_s = jau::toBitString(v1_cpu);
            std::cout << "v1_cpu_s " << v1_cpu_s << std::endl;
            std::string v2_cpu_s = jau::toBitString(v2_cpu);
            std::cout << "v2_cpu_s " << v2_cpu_s << std::endl;
        }
        REQUIRE(v0_cpu == v1_cpu);
        REQUIRE(v0_cpu == v2_cpu);

        REQUIRE(source_le == pass2_le);
        std::cout << std::endl;
    }
    {
        std::cout << "Big Endian Representation: " << std::endl;
        const std::vector<uint8_t> source_le = { 0x1a, 0x1b, 0x2a, 0x2b, 0xff };
        // const uint64_t v0_cpu = 0x000000ff2b2a1b1aU;
        const uint64_t v0_cpu = 0b1111111100101011001010100001101100011010U;

        const std::string value_s0_be = "0b1111111100101011001010100001101100011010";
        const std::string value_s1_be = jau::toBitString(source_le.data(), source_le.size(), false /* lsbFirst */);
        {
            std::vector<uint8_t> out;
            jau::fromBitString(out, value_s1_be, false);
            const uint64_t v_cpu = jau::le_to_cpu(*jau::pointer_cast<const uint64_t *>(out.data()));
            std::string v_cpu_s1 = jau::toBitString(v_cpu);
            std::string v_cpu_s2 = jau::to_string(v_cpu, 2);
            std::cout << "v0_be " << value_s1_be << ", is_le " << jau::is_little_endian() << std::endl;
            std::cout << "- out " << jau::to_string(out, 16) << std::endl;
            std::cout << "- v_cpu1 " << v_cpu_s1 << std::endl;
            std::cout << "- v_cpu2 " << v_cpu_s2 << std::endl;
            REQUIRE(source_le == out);
            REQUIRE(v0_cpu == v_cpu);
            REQUIRE(v_cpu_s1 == v_cpu_s2);
        }
        const auto [v1_cpu, len1, ok1] = jau::fromBitString(value_s1_be, false /* lsbFirst */);

        std::vector<uint8_t> pass2_le;
        jau::fromBitString(pass2_le, value_s1_be, false /* lsbFirst */);
        const std::string value_s2_be = jau::toBitString(pass2_le.data(), pass2_le.size(), false /* lsbFirst */);
        const auto [v2_cpu, len2, ok2] = jau::fromBitString(value_s2_be, false /* lsbFirst */);
        REQUIRE(value_s0_be == value_s1_be);
        REQUIRE(value_s0_be == value_s2_be);

        std::cout << "v0_be " << value_s1_be << " (2) " << value_s2_be << std::endl;
        {
            std::string v1_cpu_s = jau::to_hexstring(v1_cpu);
            std::cout << "v1_cpu_s " << v1_cpu_s << std::endl;
            std::string v2_cpu_s = jau::to_hexstring(v2_cpu);
            std::cout << "v2_cpu_s " << v2_cpu_s << std::endl;
        }
        REQUIRE(v0_cpu == v1_cpu);
        REQUIRE(v0_cpu == v2_cpu);

        REQUIRE(source_le == pass2_le);
        std::cout << std::endl;
    }
    {
        // even digits
        std::cout << "Even digits (1): " << std::endl;
        const std::vector<uint8_t> v0_b = { 0x1a, 0x1b, 0x2a, 0x2b, 0xff };
        const uint64_t v0 = 0xff2b2a1b1aU;
        const std::string v0_s_msb = "0b1111111100101011001010100001101100011010";
        const std::string v0_s_lsb = "0001101000011011001010100010101111111111";
        std::cout << "v0   " << jau::toBitString(v0) << std::endl;
        std::cout << "v0_b " << jau::to_string(v0_b, 16) << std::endl;
        std::cout << "v0_s (msb) " << v0_s_msb << std::endl;
        std::cout << "v0_s (lsb)   " << v0_s_lsb << std::endl;

        std::vector<uint8_t> v1_b_msb;
        std::vector<uint8_t> v1_b_lsb;
        jau::fromBitString(v1_b_msb, v0_s_msb, false /* lsbFirst */);
        jau::fromBitString(v1_b_lsb, v0_s_lsb, true /* lsbFirst */);
        const std::string v1_bs_msb_str = jau::toBitString(v1_b_msb, false /* lsbFirst */);
        const std::string v1_bs_lsb_str = jau::toBitString(v1_b_lsb, false /* lsbFirst */);
        std::cout << "v1_b  (msb str) " << jau::to_string(v1_b_msb, 16) << std::endl;
        std::cout << "v1_bs (msb str) " << v1_bs_msb_str << std::endl;
        std::cout << "v1_b  (lsb str) " << jau::to_string(v1_b_lsb, 16) << std::endl;
        std::cout << "v1_bs (lsb str) " << v1_bs_lsb_str << std::endl;

        const auto [v1_msb, len1, ok1] = jau::fromBitString(v0_s_msb, false);
        const auto [v1_lsb, len2, ok2] = jau::fromBitString(v0_s_lsb, true);
        std::cout << "v1   (msb) " << jau::toBitString(v1_msb) << std::endl;
        std::cout << "v1   (lsb) " << jau::toBitString(v1_lsb) << std::endl;

        REQUIRE(v0 == v1_msb);
        REQUIRE(v0 == v1_lsb);
        REQUIRE(v0_b == v1_b_msb);
        REQUIRE(v0_b == v1_b_lsb);
        std::cout << std::endl;
    }
    {
        // odd digits
        std::cout << "Odd digits (1): " << std::endl;
        // 0x3F2B2A1B1A
        const std::vector<uint8_t> v0_b_msb = { 0x1a, 0x1b, 0x2a, 0x2b, 0x3f };
        const std::vector<uint8_t> v0_b_lsb = { 0xd0, 0xd9, 0x51, 0x59, 0xf8 };
        const uint64_t v0_msb = 0x3F2B2A1B1AU;
        const uint64_t v0_lsb = 0xf85951d9d0U;
        const std::string v0_s_msb = "0b11111100101011001010100001101100011010";  // 0x3F2B2A1B1A
        const std::string v0_s_lsb = "1101000011011001010100010101100111111";     // 0x1A1B2A2B3F -> 0xf85951d9d0 (due to odd nibbles)
        // v0_s_msb:
        // BE:   [111111] 00101011 00101010 00011011 00011010
        // BE: [00111111] 00101011 00101010 00011011 00011010   11111100101011001010100001101100011010  0x3F2B2A1B1A
        //
        // v0_s_lsb:
        // LE: 11010000 11011001 01010001 01011001 [11111]
        // BE: [11111]000 01011001 01010001 11011001 11010000
        std::cout << "v0   (msb) " << jau::toBitString(v0_msb) << std::endl;
        std::cout << "v0_b (msb) " << jau::to_string(v0_b_msb) << std::endl;
        std::cout << "v0_s (msb) " << v0_s_msb << std::endl;
        std::cout << "v0   (lsb) " << jau::toBitString(v0_lsb) << std::endl;
        std::cout << "v0_b (lsb) " << jau::to_string(v0_b_lsb) << std::endl;
        std::cout << "v0_s (lsb) " << v0_s_lsb << std::endl;

        std::vector<uint8_t> v1_b_msb;
        std::vector<uint8_t> v1_b_lsb;
        jau::fromBitString(v1_b_msb, v0_s_msb, false /* lsbFirst */);
        jau::fromBitString(v1_b_lsb, v0_s_lsb, true /* lsbFirst */);
        const std::string v1_bs_msb_str = jau::toBitString(v1_b_msb, false /* lsbFirst */);
        const std::string v1_bs_lsb_str = jau::toBitString(v1_b_lsb, false /* lsbFirst */);
        std::cout << "v1_b  (msb str) " << jau::to_string(v1_b_msb, 16) << std::endl;
        std::cout << "v1_bs (msb str) " << v1_bs_msb_str << std::endl;
        std::cout << "v1_b  (lsb str) " << jau::to_string(v1_b_lsb, 16) << std::endl;
        std::cout << "v1_bs (lsb str) " << v1_bs_lsb_str << std::endl;

        const auto [v1_msb, len1, ok1] = jau::fromBitString(v0_s_msb, false);
        const auto [v1_lsb, len2, ok2] = jau::fromBitString(v0_s_lsb, true);
        std::cout << "v1   (msb) " << jau::toBitString(v1_msb) << ", " << jau::to_hexstring(v1_msb, false) << std::endl;
        std::cout << "v1   (lsb) " << jau::toBitString(v1_lsb) << ", " << jau::to_hexstring(v1_lsb, true) << std::endl;

        REQUIRE(v0_msb == v1_msb);
        REQUIRE(v0_lsb == v1_lsb);
        REQUIRE(v0_b_msb == v1_b_msb);
        REQUIRE(v0_b_lsb == v1_b_lsb);
        std::cout << std::endl;
    }
    {
        std::cout << "Even digits (2): " << std::endl;
        const uint64_t v0 = 0b1111111100101011001010100001101100011010U;  // 0xFF2B2A1B1A
        std::string v0_s = jau::toBitString(v0);
        const auto [v0_2, len1, ok1] = jau::fromBitString(v0_s);
        std::cout << "v0_s " << v0_s << std::endl;
        std::cout << "v0_2  " << jau::toBitString(v0_2) << std::endl;
        REQUIRE(v0 == v0_2);
        std::cout << std::endl;
    }
    {
        std::cout << "Even digits (3): " << std::endl;
        std::string v0_0s1 = "0b1111111100101011001010100001101100011010";  // "0xff2b2a1b1a";
        const uint64_t v0_0 = 0b1111111100101011001010100001101100011010U;  // 0xff2b2a1b1aU;
        std::string v0_0s2 = jau::toBitString(v0_0);

        const auto [i0_0s1, len1, ok1] = jau::fromBitString(v0_0s1);
        const auto [i0_0s2, len2, ok2] = jau::fromBitString(v0_0s2);

        std::cout << "v0_0s  " << v0_0s1 << std::endl;
        std::cout << "v0_0s2 " << v0_0s2 << std::endl;

        std::cout << "i0_0s1 " << jau::toBitString(i0_0s1) << std::endl;
        std::cout << "i0_0s2 " << jau::toBitString(i0_0s2) << std::endl;

        REQUIRE(v0_0 == i0_0s1);
        REQUIRE(v0_0 == i0_0s2);
        std::cout << std::endl;
    }
    {
        std::cout << "Odd digits (3): " << std::endl;
        std::string v0_0s1 = "0b111100101011001010100001101100011010";  // 0xf2b2a1b1a";
        const uint64_t v0_0 = 0b111100101011001010100001101100011010U;  // 0xf2b2a1b1aU;
        std::string v0_0s2 = jau::toBitString(v0_0);

        const auto [i0_0s1, len1, ok1] = jau::fromBitString(v0_0s1);
        const auto [i0_0s2, len2, ok2] = jau::fromBitString(v0_0s2);

        std::cout << "v0_0s  " << v0_0s1 << std::endl;
        std::cout << "v0_0s2 " << v0_0s2 << std::endl;

        std::cout << "i0_0s1 " << jau::toBitString(i0_0s1) << std::endl;
        std::cout << "i0_0s2 " << jau::toBitString(i0_0s2) << std::endl;

        REQUIRE(v0_0 == i0_0s1);
        REQUIRE(v0_0 == i0_0s2);
        std::cout << std::endl;
    }
}

TEST_CASE("Integer Type Test Test 06", "[integer][type]") {
    REQUIRE(3_i8 == (int8_t)3);
    REQUIRE(3_u8 == (uint8_t)3);

    REQUIRE(3_i16 == (int16_t)3);
    REQUIRE(3_u16 == (uint16_t)3);

    REQUIRE(3_i32 == (int32_t)3);
    REQUIRE(3_u32 == (uint32_t)3);

    REQUIRE(3_i64 == (int64_t)3);
    REQUIRE(3_u64 == (uint64_t)3);

    REQUIRE(3_iz == (ssize_t)3);
    REQUIRE(3_uz == (size_t)3);

    REQUIRE(3_inz == (jau::snsize_t)3);
    REQUIRE(3_unz == (jau::nsize_t)3);
}
