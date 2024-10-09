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

#include <jau/string_util.hpp>
#include <jau/test/catch2_ext.hpp>

#include <jau/basic_types.hpp>

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
    const uint8_t * pa = reinterpret_cast<const uint8_t *>(&a);
    for(std::size_t i=0; i<sizeof(Value_type); i++) {
        fprintf(stderr, "a[%zu] 0x%X, ", i, pa[i]);
    }
}

template<typename Value_type>
static bool compare_values(const Value_type a, const Value_type b) {
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
static Value_type compose(const uint8_t lowest_value, const jau::lb_endian_t le_or_be) {
    Value_type dest;
    uint8_t * p_dest = reinterpret_cast<uint8_t*>(&dest);
    uint8_t byte_value = lowest_value;
    if( jau::is_little_endian( le_or_be ) ) {
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
template<typename Value_type>
static Value_type compose(const uint8_t lowest_value, const jau::endian_t le_or_be) {
    return compose<Value_type>(lowest_value, to_lb_endian( le_or_be ) );
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
    jau::put_value(buffer + sizeof(Value_type)*0, v1);
    jau::put_value(buffer + sizeof(Value_type)*1, v2);
    jau::put_value(buffer + sizeof(Value_type)*2, v3);
    const Value_type r1 = jau::get_value<Value_type>(buffer + sizeof(Value_type)*0);
    const Value_type r2 = jau::get_value<Value_type>(buffer + sizeof(Value_type)*1);
    const Value_type r3 = jau::get_value<Value_type>(buffer + sizeof(Value_type)*2);
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
    if( VERBOSE ) {
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

TEST_CASE( "HexString from and to byte vector conversion - Test 04", "[hexstring]" ) {
    {
        std::cout << "Little Endian Representation: " << std::endl;
        const std::vector<uint8_t> lalaSink1 = { 0x1a, 0x1b, 0x2a, 0x2b, 0xff };
        const uint64_t v0_be = 0x000000ff2b2a1b1aU;
        const uint64_t v0_le = 0x0000001a1b2a2bffU;

        const std::string value_s0 = "1a1b2a2bff"; // LE
        const std::string value_s1 = jau::bytesHexString(lalaSink1.data(), lalaSink1.size(), true /* lsbFirst */);
        const uint64_t v1_le = jau::from_hexstring(value_s1, !jau::is_little_endian() /* lsbFirst */); // LE -> LE on native
        const uint64_t v1_be = jau::from_hexstring(value_s1,  jau::is_little_endian() /* lsbFirst */); // LE -> BE on native

        std::vector<uint8_t> lalaSink2;
        jau::hexStringBytes(lalaSink2, value_s1, true /* lsbFirst */, false);
        const std::string value_s2 = jau::bytesHexString(lalaSink2.data(), lalaSink2.size(), true /* lsbFirst */);
        const uint64_t v2_le = jau::from_hexstring(value_s2, !jau::is_little_endian() /* lsbFirst */); // LE -> LE on native
        const uint64_t v2_be = jau::from_hexstring(value_s2,  jau::is_little_endian() /* lsbFirst */); // LE -> BE on native

        REQUIRE( value_s0 == value_s1 );
        REQUIRE( value_s0 == value_s2 );

        std::cout << "v0_le " << value_s1 << " (2) " << value_s2 << std::endl;
        {
            std::string v0_be_s = jau::to_hexstring(v0_be);
            std::cout << "v0_be_s " << v0_be_s << std::endl;
            std::string v1_be_s = jau::to_hexstring(v1_be);
            std::string v2_be_s = jau::to_hexstring(v2_be);
            std::cout << "v1_be_s " << v1_be_s << std::endl;
            std::cout << "v2_be_s " << v2_be_s << std::endl;

            std::string v0_le_s = jau::to_hexstring(v0_le);
            std::cout << "v0_le_s " << v0_le_s << std::endl;
            std::string v1_le_s = jau::to_hexstring(v1_le);
            std::string v2_le_s = jau::to_hexstring(v2_le);
            std::cout << "v1_le_s " << v1_le_s << std::endl;
            std::cout << "v2_le_s " << v2_le_s << std::endl;
        }
        REQUIRE( v0_le == v1_le );
        REQUIRE( v0_le == v2_le );
        REQUIRE( v0_be == v1_be );
        REQUIRE( v0_be == v2_be );

        REQUIRE(lalaSink1 == lalaSink2);
        std::cout << std::endl;
    }
    {
        std::cout << "Big Endian Representation: " << std::endl;
        const std::vector<uint8_t> lalaSink1 = { 0x1a, 0x1b, 0x2a, 0x2b, 0xff };
        const uint64_t v0_be = 0x000000ff2b2a1b1aU;
        const uint64_t v0_le = 0x0000001a1b2a2bffU;

        const std::string value_s0 = "0xff2b2a1b1a";
        const std::string value_s1 = jau::bytesHexString(lalaSink1.data(), lalaSink1.size(), false /* lsbFirst */);
        const uint64_t v1_le = jau::from_hexstring(value_s1,  jau::is_little_endian() /* lsbFirst */); // BE -> LE on native
        const uint64_t v1_be = jau::from_hexstring(value_s1, !jau::is_little_endian() /* lsbFirst */); // BE -> BE on native

        std::vector<uint8_t> lalaSink2;
        jau::hexStringBytes(lalaSink2, value_s1, false /* lsbFirst */, true);
        const std::string value_s2 = jau::bytesHexString(lalaSink2.data(), lalaSink2.size(), false /* lsbFirst */);
        const uint64_t v2_le = jau::from_hexstring(value_s2,  jau::is_little_endian() /* lsbFirst */); // BE -> LE on native
        const uint64_t v2_be = jau::from_hexstring(value_s2, !jau::is_little_endian() /* lsbFirst */); // BE -> BE on native
        REQUIRE( value_s0 == value_s1 );
        REQUIRE( value_s0 == value_s2 );

        std::cout << "v0_be " << value_s1 << " (2) " << value_s2 << std::endl;
        {
            std::string v0_be_s = jau::to_hexstring(v0_be);
            std::cout << "v0_be_s " << v0_be_s << std::endl;
            std::string v1_be_s = jau::to_hexstring(v1_be);
            std::string v2_be_s = jau::to_hexstring(v2_be);
            std::cout << "v1_be_s " << v1_be_s << std::endl;
            std::cout << "v2_be_s " << v2_be_s << std::endl;

            std::string v0_le_s = jau::to_hexstring(v0_le);
            std::cout << "v0_le_s " << v0_le_s << std::endl;
            std::string v1_le_s = jau::to_hexstring(v1_le);
            std::string v2_le_s = jau::to_hexstring(v2_le);
            std::cout << "v1_le_s " << v1_le_s << std::endl;
            std::cout << "v2_le_s " << v2_le_s << std::endl;
        }
        REQUIRE( v0_le == v1_le );
        REQUIRE( v0_le == v2_le );
        REQUIRE( v0_be == v1_be );
        REQUIRE( v0_be == v2_be );

        REQUIRE(lalaSink1 == lalaSink2);
        std::cout << std::endl;
    }
    {
        // even digits
        std::cout << "Even digits (1): " << std::endl;
        const std::vector<uint8_t> v0_b = { 0x1a, 0x1b, 0x2a, 0x2b, 0xff };
        const uint64_t v0 = 0xff2b2a1b1aU;
        const std::string v0_s_msb = "0xff2b2a1b1a";
        const std::string v0_s_lsb =   "1a1b2a2bff";
        std::cout << "v0   " << jau::to_hexstring(v0) << std::endl;
        std::cout << "v0_b " << jau::to_string(v0_b) << std::endl;
        std::cout << "v0_s (msb) " << v0_s_msb << std::endl;
        std::cout << "v0_s (lsb)   " << v0_s_lsb << std::endl;

        std::vector<uint8_t> v1_b_msb_str;
        std::vector<uint8_t> v1_b_lsb_str;
        jau::hexStringBytes(v1_b_msb_str, v0_s_msb, false /* lsbFirst */, true);
        jau::hexStringBytes(v1_b_lsb_str, v0_s_lsb, true  /* lsbFirst */, true);
        const std::string v1_bs_msb_str = jau::bytesHexString(v1_b_msb_str, false /* lsbFirst */);
        const std::string v1_bs_lsb_str = jau::bytesHexString(v1_b_lsb_str, false /* lsbFirst */);
        std::cout << "v1_b  (msb str) " << jau::to_string(v1_b_msb_str) << std::endl;
        std::cout << "v1_bs (msb str) " << v1_bs_msb_str << std::endl;
        std::cout << "v1_b  (lsb str) " << jau::to_string(v1_b_lsb_str) << std::endl;
        std::cout << "v1_bs (lsb str) " << v1_bs_lsb_str << std::endl;

        const uint64_t v1_msb_str = jau::from_hexstring(v0_s_msb, false);
        const uint64_t v1_lsb_str = jau::from_hexstring(v0_s_lsb, true);
        std::cout << "v1   (msb) " << jau::to_hexstring(v1_msb_str) << std::endl;
        std::cout << "v1   (lsb) " << jau::to_hexstring(v1_lsb_str) << std::endl;

        REQUIRE( v0 == v1_msb_str );
        REQUIRE( v0 == v1_lsb_str );
        REQUIRE( v0_b == v1_b_msb_str );
        REQUIRE( v0_b == v1_b_lsb_str );
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
        const std::string v0_s_lsb =   "1a1b2a2bf";
        std::cout << "v0   (msb) " << jau::to_hexstring(v0_msb) << std::endl;
        std::cout << "v0_b (msb) " << jau::to_string(v0_b_msb) << std::endl;
        std::cout << "v0_s (msb) " << v0_s_msb << std::endl;
        std::cout << "v0   (lsb) " << jau::to_hexstring(v0_lsb) << std::endl;
        std::cout << "v0_b (lsb) " << jau::to_string(v0_b_lsb) << std::endl;
        std::cout << "v0_s (lsb) " << v0_s_lsb << std::endl;

        std::vector<uint8_t> v1_b_msb_str;
        std::vector<uint8_t> v1_b_lsb_str;
        jau::hexStringBytes(v1_b_msb_str, v0_s_msb, false /* lsbFirst */, true);
        jau::hexStringBytes(v1_b_lsb_str, v0_s_lsb, true  /* lsbFirst */, true);
        const std::string v1_bs_msb_str = jau::bytesHexString(v1_b_msb_str, false /* lsbFirst */);
        const std::string v1_bs_lsb_str = jau::bytesHexString(v1_b_lsb_str, false /* lsbFirst */);
        std::cout << "v1_b  (msb str) " << jau::to_string(v1_b_msb_str) << std::endl;
        std::cout << "v1_bs (msb str) " << v1_bs_msb_str << std::endl;
        std::cout << "v1_b  (lsb str) " << jau::to_string(v1_b_lsb_str) << std::endl;
        std::cout << "v1_bs (lsb str) " << v1_bs_lsb_str << std::endl;

        const uint64_t v1_msb_str = jau::from_hexstring(v0_s_msb, false);
        const uint64_t v1_lsb_str = jau::from_hexstring(v0_s_lsb, true);
        std::cout << "v1   (msb) " << jau::to_hexstring(v1_msb_str) << std::endl;
        std::cout << "v1   (lsb) " << jau::to_hexstring(v1_lsb_str) << std::endl;

        REQUIRE( v0_msb == v1_msb_str );
        REQUIRE( v0_lsb == v1_lsb_str );
        REQUIRE( v0_b_msb == v1_b_msb_str );
        REQUIRE( v0_b_lsb == v1_b_lsb_str );
        std::cout << std::endl;
    }
    {
        std::cout << "Even digits (2): " << std::endl;
        const uint64_t v0 = 0x000000ff2b2a1b1aU;
        std::string v0_s = jau::to_hexstring(v0);
        const uint64_t v0_2 = jau::from_hexstring(v0_s);
        std::cout << "v0_s " << v0_s << std::endl;
        std::cout << "v0_2  " << jau::to_hexstring(v0_2) << std::endl;
        REQUIRE( v0 == v0_2 );
        std::cout << std::endl;
    }
    {
        std::cout << "Even digits (3): " << std::endl;
        std::string v0_0s1 = "0xff2b2a1b1a";
        const uint64_t v0_0 = 0xff2b2a1b1aU;
        std::string v0_0s2 = jau::to_hexstring(v0_0);

        const uint64_t i0_0s1 = jau::from_hexstring(v0_0s1);
        const uint64_t i0_0s2 = jau::from_hexstring(v0_0s2);

        std::cout << "v0_0s  " << v0_0s1 << std::endl;
        std::cout << "v0_0s2 " << v0_0s2 << std::endl;

        std::cout << "i0_0s1 " << jau::to_hexstring(i0_0s1) << std::endl;
        std::cout << "i0_0s2 " << jau::to_hexstring(i0_0s2) << std::endl;

        REQUIRE( v0_0 == i0_0s1 );
        REQUIRE( v0_0 == i0_0s2 );
        std::cout << std::endl;
    }
    {
        std::cout << "Odd digits (3): " << std::endl;
        std::string v0_0s1 = "0xf2b2a1b1a";
        const uint64_t v0_0 = 0xf2b2a1b1aU;
        std::string v0_0s2 = jau::to_hexstring(v0_0);

        const uint64_t i0_0s1 = jau::from_hexstring(v0_0s1);
        const uint64_t i0_0s2 = jau::from_hexstring(v0_0s2);

        std::cout << "v0_0s  " << v0_0s1 << std::endl;
        std::cout << "v0_0s2 " << v0_0s2 << std::endl;

        std::cout << "i0_0s1 " << jau::to_hexstring(i0_0s1) << std::endl;
        std::cout << "i0_0s2 " << jau::to_hexstring(i0_0s2) << std::endl;

        REQUIRE( v0_0 == i0_0s1 );
        REQUIRE( v0_0 == i0_0s2 );
        std::cout << std::endl;
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
