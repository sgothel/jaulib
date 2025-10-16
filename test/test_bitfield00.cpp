/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2022-2025 Gothel Software e.K.
 *
 * SPDX-License-Identifier: MIT
 *
 * This Source Code Form is subject to the terms of the MIT License
 * If a copy of the MIT was not distributed with this file,
 * you can obtain one at https://opensource.org/license/mit/.
 */

#include <cassert>
#include <cstdlib>
#include <cstring>

#include <jau/basic_types.hpp>
#include <jau/bitfield.hpp>
#include <jau/bitheap.hpp>
#include <jau/debug.hpp>
#include <jau/int_math.hpp>
#include <jau/int_math_ct.hpp>
#include <jau/io/file_util.hpp>
#include <jau/io/io_util.hpp>
#include <jau/string_util.hpp>
#include <jau/test/catch2_ext.hpp>

// #include "test_httpd.hpp"
#include "data_bitstream.hpp"

using namespace jau::int_literals;

TEST_CASE( "Bitfield Test 00", "[bitfield]" ) {
    {
        const size_t bits = 3_uz*64_uz;
        jau::bitfield_t<uint64_t, bits> b1;
        REQUIRE(bits == b1.bit_size);
        REQUIRE(64 == b1.unit_bit_size);
        REQUIRE(8 == b1.unit_byte_size);
        REQUIRE(6 == b1.unit_shift);
        REQUIRE(3 == b1.unit_size);
        REQUIRE(0 == b1.count());
        REQUIRE(b1.bit_size == b1.flip().count());
        REQUIRE(0           == b1.flip().count());
        REQUIRE(b1.bit_size == b1.flip().count());
        REQUIRE(0           == b1.reset().count());
        REQUIRE(b1.bit_size == b1.setAll(true).count());
        REQUIRE(0           == b1.setAll(false).count());
        REQUIRE( true       == b1.set(64, 2_uz*64_uz, true));
        REQUIRE(2_uz*64_uz  == b1.count());
    }
    {
        const size_t bits = 3_uz*64_uz+4_uz;
        jau::bitfield_t<uint64_t, bits> b1;
        REQUIRE(bits == b1.bit_size);
        REQUIRE(64 == b1.unit_bit_size);
        REQUIRE(8 == b1.unit_byte_size);
        REQUIRE(6 == b1.unit_shift);
        REQUIRE(4 == b1.unit_size);
        REQUIRE(0 == b1.count());
        REQUIRE(b1.bit_size == b1.flip().count());
        REQUIRE(0           == b1.flip().count());
        REQUIRE(b1.bit_size == b1.flip().count());
        REQUIRE(0           == b1.reset().count());
        REQUIRE(b1.bit_size == b1.setAll(true).count());
        REQUIRE(0           == b1.setAll(false).count());
        REQUIRE( true       == b1.set(33, 2_uz*64_uz+2_uz, true));
        REQUIRE(2_uz*64_uz+2_uz == b1.count());
    }
    {
        const size_t bits = 3_uz*32_uz+4_uz;
        jau::bitfield_t<uint32_t, bits> b1;
        REQUIRE(bits == b1.bit_size);
        REQUIRE(32 == b1.unit_bit_size);
        REQUIRE(4 == b1.unit_byte_size);
        REQUIRE(5 == b1.unit_shift);
        REQUIRE(4 == b1.unit_size);
        REQUIRE(0 == b1.count());

        REQUIRE(b1.bit_size == b1.flip().count());
        REQUIRE(0           == b1.flip().count());
        REQUIRE(b1.bit_size == b1.flip().count());
        REQUIRE(0           == b1.reset().count());
        REQUIRE(b1.bit_size == b1.setAll(true).count());
        REQUIRE(0           == b1.setAll(false).count());
        REQUIRE(true        == b1.set(17, 2_uz*32_uz+2_uz, true));
        REQUIRE(2_uz*32_uz+2_uz == b1.count());
    }
    {
        const size_t bits = 3_uz*8_uz+4_uz;
        jau::bitfield_t<uint8_t, bits> b1;
        REQUIRE(bits == b1.bit_size);
        REQUIRE(8 == b1.unit_bit_size);
        REQUIRE(1 == b1.unit_byte_size);
        REQUIRE(3 == b1.unit_shift);
        REQUIRE(4 == b1.unit_size);
        REQUIRE(0 == b1.count());

        REQUIRE(b1.bit_size == b1.flip().count());
        REQUIRE(0           == b1.flip().count());
        REQUIRE(b1.bit_size == b1.flip().count());
        REQUIRE(0           == b1.reset().count());
        REQUIRE(b1.bit_size == b1.setAll(true).count());
        REQUIRE(0           == b1.setAll(false).count());
        REQUIRE(true        == b1.set(5, 2_uz*8_uz+2_uz, true));
        REQUIRE(2_uz*8_uz+2_uz == b1.count());
    }
}

TEST_CASE( "Bitfield Test 01 BitCount32_One", "[bitfield]" ) {
    const std::vector<std::string_view> &pyramid32bit_one  = BitDemoData::pyramid32bit_one;
    for(size_t i=0; i<pyramid32bit_one.size(); i++) {
        const uint32_t val0 = 1_u32 << i;
        const size_t oneBitCountI1 = jau::ct_bit_count(val0);
        const size_t oneBitCountI2 = jau::bit_count(val0);
        const std::string_view pattern0 = pyramid32bit_one[i];
        const uint32_t val1 = BitDemoData::toInteger(pattern0);
        const std::string pattern1 = BitDemoData::toBinaryString(val0, 32);
        const size_t oneBitCount0 = BitDemoData::getOneBitCount(pattern0);
        // fprintf(stderr, "Round %02zu: 0x%08x %s, c %zu / %zu / %zu\n        : 0x%08x %s\n",
        //         i, val0, std::string(pattern0).c_str(), oneBitCount0, oneBitCountI1, oneBitCountI2,
        //         val1, pattern1.c_str());
        REQUIRE(val0 == val1);
        REQUIRE(pattern0 == pattern1);

        REQUIRE(oneBitCount0 == oneBitCountI1);
        REQUIRE(oneBitCount0 == oneBitCountI2);
    }
}

static jau::bitheap getBitheap(const jau::bit_order_t dataBitOrder,
                               const jau::nsize_t preBits, const jau::nsize_t skipBits, const jau::nsize_t postBits) {
    const jau::nsize_t totalBits = preBits+postBits;
    fprintf(stderr,"XXX getBitheap: bitOrder %s, preBits %zu, skipBits %zu, postBits %zu, totalBits %zu\n",
        jau::to_string(dataBitOrder).c_str(), (size_t)preBits, (size_t)skipBits, (size_t)postBits, (size_t)totalBits);

    // msb 11111010 11011110 10101111 11111110 11011110 10101111 11001010 11111110
    // lsb 01111111 01010011 11110101 01111011 01111111 11110101 01111011 01011111
    std::string_view in = BitDemoData::testStringMSB64_be;
    jau::bitheap source(in);
    if( jau::bit_order_t::msb != dataBitOrder ) {
        source.reverse();
        REQUIRE(BitDemoData::testStringLSB64_le == source.toString());
    }
    std::cerr << source << "\n";
    const auto [pre, preOK] = source.subbits(0, preBits);
    const auto [post, postOK] = source.subbits(preBits+skipBits, postBits);
    REQUIRE(true == preOK);
    REQUIRE(true == postOK);

    jau::bitheap r(preBits+postBits);
    REQUIRE(true == r.put(0, pre));
    REQUIRE(true == r.put(preBits, post));
    std::cerr << "ResultExp: <" << pre << "> + <" << post << "> = <" << r << ">\n";
    fprintf(stderr,"source0: <%s>\n", std::string(in).c_str());
    fprintf(stderr,"source1: <%s>\n", source.toString().c_str());
    REQUIRE(totalBits == r.size());
    return r;
}

TEST_CASE("Bitfield Test 01 subbits", "[bitfield][subbits]") {
    // msb 11111010 11011110 10101111 11111110 11011110 10101111 11001010 11111110
    REQUIRE(jau::bitheap("11111110") == getBitheap(jau::bit_order_t::msb, 0, 0, 8));
    REQUIRE(jau::bitheap("010") == getBitheap(jau::bit_order_t::msb, 0, 8, 3));
    REQUIRE(jau::bitheap("01011111110") == getBitheap(jau::bit_order_t::msb, 8, 0, 3));
    // msb 11111010 11011110 10101111 11111110 11011110 10101111 11001010 11111110
    // lsb 01111111 01010011 11110101 01111011 01111111 11110101 01111011 01011111
    REQUIRE(jau::bitheap("01011111") == getBitheap(jau::bit_order_t::lsb, 0, 0, 8));
    REQUIRE(jau::bitheap("011") == getBitheap(jau::bit_order_t::lsb, 0, 8, 3));
    REQUIRE(jau::bitheap("01101011111") == getBitheap(jau::bit_order_t::lsb, 8, 0, 3));
}

/**
 * ***********************************************************************************
 * ***********************************************************************************
 * ***********************************************************************************
 */

static void test_BitCount32_Samples(const uint32_t l) {
    const size_t oneBitCountL = jau::bit_count(l);
    const size_t oneBitCount1 = jau::ct_bit_count(l);
    // fprintf(stderr, "Round 0x%08x, c %zu / %zu\n", l, oneBitCountL, oneBitCount1);
    REQUIRE(oneBitCountL == oneBitCount1);
}

TEST_CASE( "Bitfield Test 10 BitCount32_One", "[bitfield]" ) {
    const uint32_t MAX = BitDemoData::UNSIGNED_INT_MAX_VALUE;
    const uint32_t MAX_minus = MAX-0x1FF;
    const uint32_t MAX_half = MAX/2;
    const uint32_t MAX_half_minus = MAX_half-0x1FF;
    const uint32_t MAX_half_plus = MAX_half+0x1FF;

    for(uint32_t l=0; l<=0x1FF; ++l) {
        test_BitCount32_Samples(l);
    }
    for(uint32_t l=MAX_half_minus; l<=MAX_half_plus; ++l) {
        test_BitCount32_Samples(l);
    }
    for(uint32_t l=MAX_minus-1; l++ <MAX; ) {
        test_BitCount32_Samples(l);
    }
}

/**
 * ***********************************************************************************
 * ***********************************************************************************
 * ***********************************************************************************
 */

static std::vector<uint32_t> testDataOneBit = {
    0, 0, 1, 1, 2, 1, 3, 2, 4, 1, 5, 2, 6, 2, 7, 3,
    8, 1, 9, 2, 10, 2, 11, 3, 12, 2, 13, 3, 14, 3, 15, 4, 16, 1, 17, 2,
    0x3F, 6, 0x40, 1, 0x41, 2, 0x7f, 7, 0x80, 1, 0x81, 2, 0xfe, 7, 0xff, 8,
    0x4000, 1, 0x4001, 2, 0x7000, 3, 0x7fff, 15,
    0x0FFFFFF0, 24,
    0x55555555, 16,
    0x7F53F57B, 23,
    0xFEA7EAF6, 23, /* << 1 */
    0x80000000, 1,
    0xAAAAAAAA, 16,
    0xC0C0C0C0, 8,
    0xFF000000, 8,
    0xFFFFFFFF, 32
};

static void test_BitCount32_Data(const uint32_t i, const uint32_t expOneBits) {
    const size_t oneBitCountI = jau::bit_count(i);
    const size_t oneBitCount1 = jau::ct_bit_count(i);
    // fprintf(stderr, "Round 0x%08x, c %zu / %zu\n", i, oneBitCountI, oneBitCount1);
    REQUIRE(expOneBits == oneBitCountI);
    REQUIRE(oneBitCountI == oneBitCount1);
}

TEST_CASE( "Bitfield Test 11 BitCount32_Data", "[bitfield]" ) {
    for(size_t i = 0; i<testDataOneBit.size(); i+=2) {
        test_BitCount32_Data(testDataOneBit[i], testDataOneBit[i+1]);
    }
}


/**
 * ***********************************************************************************
 * ***********************************************************************************
 * ***********************************************************************************
 */

struct TestDataBF {
    const size_t bitSize;
    const uint64_t val;
    const std::string_view pattern;

    TestDataBF(const uint64_t bitSize_, const uint64_t value_, const std::string_view pattern_)
    : bitSize(bitSize_), val(value_), pattern(pattern_) { }
    std::string toString() const { return "BF[bitSize " + std::to_string(bitSize)+", val "+jau::toHexString(val)+", pattern '"+std::string(pattern)+"']"; }
};
static std::ostream &operator<<(std::ostream &out, const TestDataBF &v) {
    return out << v.toString();
}

static std::vector<TestDataBF> testDataBF64Bit = {
    TestDataBF(64, BitDemoData::testIntMSB64_be, BitDemoData::testStringMSB64_be),
    TestDataBF(64, BitDemoData::testIntMSB64_le, BitDemoData::testStringMSB64_le),
    TestDataBF(64, BitDemoData::testIntLSB64_be, BitDemoData::testStringLSB64_be),
    TestDataBF(64, BitDemoData::testIntLSB64_le, BitDemoData::testStringLSB64_le),

    TestDataBF(64, 0x04030201AFFECAFE, "0000010000000011000000100000000110101111111111101100101011111110"),
    TestDataBF(64, 0xAFFECAFE04030201, "1010111111111110110010101111111000000100000000110000001000000001"),
    TestDataBF(64, 0xDEADBEEFDEADBEEF, "1101111010101101101111101110111111011110101011011011111011101111")
};


static std::vector<TestDataBF> testDataBF32Bit = {
    // H->L    : 0x04030201: 00000100 00000011 00000010 00000001
    TestDataBF(32, 0x04030201, "00000100000000110000001000000001"),

    // H->L    : 0xAFFECAFE: 10101111 11111110 11001010 11111110
    TestDataBF(32, 0xAFFECAFE, "10101111111111101100101011111110"),
    // H->L    : 0xDEADBEEF: 11011110 10101101 10111110 11101111
    TestDataBF(32, 0xDEADBEEF, "11011110101011011011111011101111")
};

static std::vector<TestDataBF> testDataBF16Bit = {
    // H->L    : 0x0201: 00000100 00000011 00000010 00000001
    TestDataBF(16, 0x0201, "0000001000000001"),
    // H->L    : 0x0403: 00000100 00000011
    TestDataBF(16, 0x0403, "0000010000000011"),

    // H->L    : 0xAFFE: 10101111 11111110
    TestDataBF(16, 0xAFFE, "1010111111111110"),
    // H->L    : 0xCAFE: 11001010 11111110
    TestDataBF(16, 0xCAFE, "1100101011111110"),

    // H->L    : 0xDEADBEEF: 11011110 10101101 10111110 11101111
    TestDataBF(16, 0xDEAD, "1101111010101101"),
    TestDataBF(16, 0xBEEF, "1011111011101111")
};

static std::vector<TestDataBF> testDataBF3Bit = {
    TestDataBF(3, 0x01, "001"),
    TestDataBF(3, 0x02, "010"),
    TestDataBF(3, 0x05, "101")
};

static void test_ValidateTestData(std::string_view prefix, const TestDataBF& d) {
    std::cout << "Test " << prefix << ": " << d << "\n";
    const size_t oneBitCount0 = jau::bit_count(d.val);
    const size_t oneBitCount1 = BitDemoData::getOneBitCount(d.pattern);
    REQUIRE(oneBitCount0 == oneBitCount1);
    // Test: BF[bitSize 64, val 0xdeafcafe, pattern '1111101011011110101011111111111011011110101011111100101011111110'

    const std::string& pattern0 = BitDemoData::toBinaryString(d.val, d.bitSize);
    REQUIRE(d.pattern == pattern0);

    const uint64_t val1 = BitDemoData::toInteger(d.pattern);
    REQUIRE(d.val == val1);
    REQUIRE(d.bitSize == pattern0.length());
}

TEST_CASE( "Bitfield Test 20 ValidateTestData", "[bitfield]" ) {
    for(size_t i=0; i<testDataBF64Bit.size(); ++i) {
        test_ValidateTestData( "BF64Bit."+std::to_string(i), testDataBF64Bit[i] );
    }
    for(size_t i=0; i<testDataBF32Bit.size(); ++i) {
        test_ValidateTestData( "BF32Bit."+std::to_string(i), testDataBF32Bit[i] );
    }
    for(size_t i=0; i<testDataBF16Bit.size(); ++i) {
        test_ValidateTestData( "BF16Bit."+std::to_string(i), testDataBF16Bit[i] );
    }
    for(size_t i=0; i<testDataBF3Bit.size(); ++i) {
        test_ValidateTestData( "BF03Bit."+std::to_string(i), testDataBF3Bit[i] );
    }
}


/**
 * ***********************************************************************************
 * ***********************************************************************************
 * ***********************************************************************************
 */

template<jau::req::unsigned_integral StorageType, size_t BitSize>
static void assertEquals(const jau::bitfield_t<StorageType, BitSize>& bf, const size_t bf_off, const uint64_t v,
                         const std::string_view pattern, size_t oneBitCount)
{
    const size_t len = pattern.length();
    for(size_t i=0; i<len; i++) {
        const bool exp0 = 0 != ( v & ( 1_u64 << i ) );
        const bool exp1 = '1' == pattern[len-1-i];
        const bool has = bf[i+bf_off];
        (void)oneBitCount;
        // fprintf(stderr, "Pos %04zu: Value 0x%08" PRIu64 "x / %s, c %zu\n", i, v, std::string(pattern).c_str(), oneBitCount);
        REQUIRE(exp0 == has);
        REQUIRE(exp1 == has);
    }
}

template<jau::req::unsigned_integral StorageType, size_t BitSize1, size_t BitSize2>
static void test_AlignedBits(std::string_view prefix, const TestDataBF& d,
                             jau::bitfield_t<StorageType, BitSize1>& bf1, jau::bitfield_t<StorageType, BitSize2>& bf2)
{
    std::cout << "Test " << prefix << ": " << d << "\n";

    const size_t oneBitCount = jau::bit_count(d.val);

    REQUIRE(true == bf1.putUnit( 0, d.bitSize, d.val) );
    REQUIRE(d.val == bf1.getUnit(0, d.bitSize));
    REQUIRE(oneBitCount == bf1.count());
    // std::cout << "Test " << prefix << ".1 - bf1: " << bf1 << "\n";
    assertEquals(bf1, 0, d.val, d.pattern, oneBitCount);

    REQUIRE(true == bf2.putUnit( 0, d.bitSize, d.val));
    REQUIRE(d.val == bf2.getUnit( 0, d.bitSize));
    REQUIRE(oneBitCount*1 == bf2.count());
    // std::cout << "Test " << prefix << ".2 - bf2: " << bf2 << "\n";
    assertEquals(bf2, 0, d.val, d.pattern, oneBitCount);

    REQUIRE(true == bf2.putUnit(128, d.bitSize, d.val));
    REQUIRE(d.val == bf2.getUnit(128, d.bitSize));
    REQUIRE(oneBitCount*2 == bf2.count());
    // std::cout << "Test " << prefix << ".3 - bf2: " << bf2 << "\n";
    assertEquals(bf2, 128, d.val, d.pattern, oneBitCount);

    REQUIRE(true == bf2.copyUnit(0, 233, d.bitSize));
    REQUIRE(d.val == bf2.getUnit(233, d.bitSize));

    // std::cout << "Test " << prefix << ".4 - bf2: " << bf2 << "\n";
    REQUIRE(oneBitCount*3 == bf2.count());
    assertEquals(bf2, 233, d.val, d.pattern, oneBitCount);
}
static void test_AlignedBits(std::string_view prefix, const TestDataBF& d) {
    {
        jau::bitfield_t<uint64_t, 64_u64> bf1;
        jau::bitfield_t<uint64_t, 64_u64 * 5> bf2;
        test_AlignedBits(std::string(prefix)+".a", d, bf1, bf2);
    }
    if( d.bitSize <= 32 ) {
        jau::bitfield_t<uint32_t, 64_u64> bf1;
        jau::bitfield_t<uint32_t, 64_u64 * 5> bf2;
        test_AlignedBits(std::string(prefix)+".b", d, bf1, bf2);
    }
}

TEST_CASE( "Bitfield Test 21 Alignedbits", "[bitfield]" ) {
    for(size_t i=0; i<testDataBF64Bit.size(); ++i) {
        test_AlignedBits( "BF64Bit."+std::to_string(i), testDataBF64Bit[i] );
    }
    for(size_t i=0; i<testDataBF32Bit.size(); ++i) {
        test_AlignedBits( "BF32Bit."+std::to_string(i), testDataBF32Bit[i] );
    }
    for(size_t i=0; i<testDataBF16Bit.size(); ++i) {
        test_AlignedBits( "BF16Bit."+std::to_string(i), testDataBF16Bit[i] );
    }
    for(size_t i=0; i<testDataBF3Bit.size(); ++i) {
        test_AlignedBits( "BF03Bit."+std::to_string(i), testDataBF3Bit[i] );
    }
}


/**
 * ***********************************************************************************
 * ***********************************************************************************
 * ***********************************************************************************
 */

template<jau::req::unsigned_integral StorageType, size_t BitSize>
static void checkOtherBits(const TestDataBF& d, const jau::bitfield_t<StorageType, BitSize>& bf, const size_t lowBitnum,
                           const std::string& msg, const StorageType expBits) {
    const size_t highBitnum = lowBitnum + d.bitSize - 1;
    // fprintf(stderr,msg+": [0"+".."+"("+lowBitnum+".."+highBitnum+").."+(bf.size()-1)+"]");
    for(size_t i=0; i<lowBitnum; i+=32) {
        const size_t len = std::min<size_t>(32_uz, lowBitnum-i);
        const StorageType val = bf.getUnit(i, len);
        const StorageType exp = expBits & BitDemoData::getBitMask(len);
        // fprintf(stderr,"    <"+i+".."+(i+len-1)+">, exp "+BitDemoData.toHexString(exp));
        REQUIRE_MSG(msg+", bitpos "+std::to_string(i), exp == val);
    }
    for(size_t i=highBitnum+1; i<bf.size(); i+=32) {
        const size_t len = std::min(32_uz, bf.size() - i);
        const StorageType val = bf.getUnit(i, len);
        const StorageType exp = expBits & BitDemoData::getBitMask(len);
        // fprintf(stderr,"        <"+i+".."+(i+len-1)+">, exp "+BitDemoData.toHexString(exp));
        REQUIRE_MSG(msg+", bitpos "+std::to_string(i), exp == val);
    }
}

template<jau::req::unsigned_integral StorageType, size_t BitSize>
static void test_Unaligned(const TestDataBF &d, jau::bitfield_t<StorageType, BitSize>& bf, const size_t lowBitnum) {
    const size_t maxBitpos = bf.size()-d.bitSize;
    const size_t oneBitCount = jau::bit_count(d.val);

    const std::string msg = jau::format_string("Value 0x%08" PRIx64 " / %s, l %zu/%zu, c %zu, lbPos %zu -> %zu",
            d.val, std::string(d.pattern).c_str(), d.bitSize, bf.size(), oneBitCount, lowBitnum, maxBitpos);

    //
    // via putUnit
    //
    REQUIRE( true == bf.putUnit( lowBitnum, d.bitSize, d.val) );
    for(size_t i=0; i<d.bitSize; i++) {
        const bool exp = d.val & ( 1_u64 << i );
        const bool has = bf[lowBitnum+i];
        REQUIRE_MSG(msg+", bitpos "+std::to_string(i), exp == has);
    }
    REQUIRE_MSG(msg, d.val == bf.getUnit( lowBitnum, d.bitSize));
    REQUIRE_MSG(msg, oneBitCount == bf.count());
    assertEquals(bf, lowBitnum, d.val, d.pattern, oneBitCount);

    //
    // via copyUnit
    //
    if( lowBitnum < maxBitpos ) {
        // copy bits 1 forward
        // clear trailing orig bit
        REQUIRE_MSG(msg, true == bf.copyUnit(lowBitnum, lowBitnum+1, d.bitSize));
        REQUIRE(true == bf.clr(lowBitnum));
        REQUIRE_MSG(msg, d.val == bf.getUnit(lowBitnum+1, d.bitSize));
        REQUIRE_MSG(msg, oneBitCount == bf.count());
        assertEquals(bf, lowBitnum+1, d.val, d.pattern, oneBitCount);
    }

    // test put/get
    bf.reset();
    REQUIRE_MSG(msg+", bitpos 0", false == bf[lowBitnum]);
    REQUIRE(true == bf.put(lowBitnum, true));
    REQUIRE_MSG(msg+", bitpos 0", true == bf[lowBitnum]);
    REQUIRE(true == bf.put(lowBitnum, false));
    REQUIRE_MSG(msg+", bitpos 0", false == bf[lowBitnum]);

    //
    // via put/get
    //
    for(size_t i=0; i<d.bitSize; i++) {
        REQUIRE_MSG(msg+", bitpos "+std::to_string(i), false == bf[lowBitnum+i]);
        const bool v = d.val & ( 1_u64 << i );
        REQUIRE(true == bf.put(lowBitnum+i, v));
        REQUIRE_MSG(msg+", bitpos "+std::to_string(i), v == bf[lowBitnum+i]);
    }
    REQUIRE_MSG(msg, d.val == bf.getUnit(lowBitnum, d.bitSize));
    for(size_t i=0; i<d.bitSize; i++) {
        const bool exp = d.val & ( 1_u64 << i );
        const bool has = bf[lowBitnum+i];
        REQUIRE_MSG(msg+", bitpos "+std::to_string(i), exp == has);
    }
    REQUIRE_MSG(msg, oneBitCount == bf.count());
    assertEquals(bf, lowBitnum, d.val, d.pattern, oneBitCount);

    //
    // via copy
    //
    if( lowBitnum < maxBitpos ) {
        // copy bits 1 forward
        // clear trailing orig bit
        for(size_t i=d.bitSize; i-- >0; ) {
            const bool exp = d.val & (1_u64 << i);
            REQUIRE(true == bf.copy(lowBitnum+i, lowBitnum+1+i));
            const bool has = bf.get(lowBitnum+1+i);
            REQUIRE_MSG(msg+", bitpos "+std::to_string(i), exp == has);
        }
        REQUIRE(true == bf.clr(lowBitnum));
        REQUIRE_MSG(msg, d.val == bf.getUnit( lowBitnum+1, d.bitSize));
        for(size_t i=0; i<d.bitSize; i++) {
            const bool exp = d.val & (1_u64 << i);
            const bool has = bf[lowBitnum+1+i];
            REQUIRE_MSG(msg+", bitpos "+std::to_string(i), exp == has);
        }
        REQUIRE_MSG(msg, oneBitCount == bf.count());
        assertEquals(bf, lowBitnum+1, d.val, d.pattern, oneBitCount);
    }

    //
    // via set/clear
    //
    {
        REQUIRE(0 == bf.setAll(false).count());
        for(size_t i=0; i<d.bitSize; i++) {
            if( d.val & ( 1_u64 << i ) ) {
                REQUIRE(true == bf.set(lowBitnum+i));
            } else {
                REQUIRE(true == bf.clr(lowBitnum+i));
            }
        }
        REQUIRE_MSG(msg, d.val == bf.getUnit(lowBitnum, d.bitSize));
        for(size_t i=0; i<d.bitSize; i++) {
            const bool exp = d.val & (1_u64 << i);
            const bool has = bf[lowBitnum+i];
            REQUIRE_MSG(msg+", bitpos "+std::to_string(i), exp == has);
        }
        REQUIRE_MSG(msg, oneBitCount == bf.count());
        assertEquals(bf, lowBitnum, d.val, d.pattern, oneBitCount);
    }
    {
        REQUIRE(bf.bit_size == bf.setAll(true).count());
        REQUIRE(true == bf.set(0, lowBitnum, false));
        REQUIRE(true == bf.set(lowBitnum+d.bitSize, bf.bit_size-(lowBitnum+d.bitSize), false));
        REQUIRE_MSG(msg, d.bitSize == bf.count());
        for(size_t i=0; i<d.bitSize; i++) {
            if( d.val & ( 1_u64 << i ) ) {
                REQUIRE(true == bf.set(lowBitnum+i));
            } else {
                REQUIRE(true == bf.clr(lowBitnum+i));
            }
        }
        REQUIRE_MSG(msg, d.val == bf.getUnit(lowBitnum, d.bitSize));
        for(size_t i=0; i<d.bitSize; i++) {
            const bool exp = d.val & (1_u64 << i);
            const bool has = bf[lowBitnum+i];
            REQUIRE_MSG(msg+", bitpos "+std::to_string(i), exp == has);
        }
        if( oneBitCount != bf.count() ) {
            REQUIRE_MSG(msg, oneBitCount == bf.count());
        }
        assertEquals(bf, lowBitnum, d.val, d.pattern, oneBitCount);
    }

    //
    // Validate 'other bits' put32/get32
    //
    bf.setAll(false);
    REQUIRE(true == bf.putUnit( lowBitnum, d.bitSize, d.val));
    checkOtherBits(d, bf, lowBitnum, msg, StorageType(0));

    bf.setAll(true);
    REQUIRE(true == bf.putUnit( lowBitnum, d.bitSize, d.val));
    checkOtherBits(d, bf, lowBitnum, msg, StorageType(BitDemoData::UNSIGNED_INT_MAX_VALUE));
}

template<jau::req::unsigned_integral StorageType, size_t BitSize>
static void test_Unaligned(const TestDataBF& d, jau::bitfield_t<StorageType, BitSize>& bf) {
    const size_t maxBitpos = bf.size()-d.bitSize;
    for(size_t i=0; i<=maxBitpos; i++) {
        bf.setAll(false);
        test_Unaligned(d, bf, i);
    }
}

template<size_t BitSize1, size_t BitSize2>
static void test_Unaligned(const TestDataBF& d) {
    jau::bitfield_t<uint64_t, BitSize1> bf1;
    jau::bitfield_t<uint64_t, BitSize2> bf2;
    test_Unaligned( d, bf1 );
    test_Unaligned( d, bf2 );
}

TEST_CASE( "Bitfield Test 22 Unalignedbits", "[bitfield]" ) {
    for(const auto & i : testDataBF64Bit) {
        test_Unaligned<64, 64+196>(i);
    }
    for(const auto & i : testDataBF32Bit) {
        test_Unaligned<32, 32+128>(i);
    }
    for(const auto & i : testDataBF16Bit) {
        test_Unaligned<16, 16+128>(i);
    }
    for(const auto & i : testDataBF3Bit) {
        test_Unaligned<3, 3+128>( i );
    }
}


template<jau::req::unsigned_integral StorageType, size_t BitSize>
static void testAlignedBitReverse(std::string_view prefix, const TestDataBF& d, jau::bitfield_t<StorageType, BitSize>& bf) {
    const bool verbose = true;
    if( verbose ) {
        std::cout << prefix << ": " << d << "\n";
        std::cout << " bf bit-size[unit " << bf.unit_bit_size << ", total " << bf.bit_size << "], units " << bf.unit_size << "\n";
    }
    bf.clear();
    REQUIRE( true == bf.put(0, d.pattern) );

    std::string s_be2 = bf.toString();
    std::string s_be2_rev = s_be2;
    std::ranges::reverse(s_be2_rev);
    std::string s_be3_rev = bf.reverse().toString();
    if( verbose ) {
        std::cout << "  data:      " << d.pattern << "\n";
        std::cout << "  s_be2:     " << s_be2 << "\n";
        std::cout << "  s_be2_rev: " << s_be2_rev << "\n";
        std::cout << "  s_be3_rev: " << s_be3_rev << "\n";
    }
    REQUIRE( d.pattern == s_be2 );
    REQUIRE( s_be2_rev == s_be3_rev );
}

template<size_t BitSize>
static void testAlignedBitReverse(std::string_view prefix, const TestDataBF& d) {
    jau::bitfield_t<uint64_t, BitSize> bf1;
    jau::bitfield_t<uint32_t, BitSize> bf2;
    jau::bitfield_t<uint16_t, BitSize> bf3;
    jau::bitfield_t<uint8_t, BitSize> bf4;
    testAlignedBitReverse( prefix, d, bf1 );
    testAlignedBitReverse( prefix, d, bf2 );
    testAlignedBitReverse( prefix, d, bf3 );
    testAlignedBitReverse( prefix, d, bf4 );
}

TEST_CASE("Bitfield Test 30 Aligned Reverse", "[bitfield][bitreverse]") {
    {
        jau::bitfield<64> exp(BitDemoData::testStringLSB64_le);
        jau::bitfield<64> has(BitDemoData::testStringMSB64_be);
        has.reverse();
        REQUIRE(exp == has);
        REQUIRE(BitDemoData::testStringLSB64_le == has.toString());
    }
    {
        jau::bitheap source(BitDemoData::testStringMSB64_be);
        source.reverse();
        REQUIRE(jau::bitheap(BitDemoData::testStringLSB64_le) == source);
        REQUIRE(BitDemoData::testStringLSB64_le == source.toString());
    }

    for(size_t i=0; i<testDataBF64Bit.size(); ++i) {
        testAlignedBitReverse<64>( "BF64Bit."+std::to_string(i), testDataBF64Bit[i] );
    }
    for(size_t i=0; i<testDataBF32Bit.size(); ++i) {
        testAlignedBitReverse<32>( "BF32Bit."+std::to_string(i), testDataBF32Bit[i] );
    }
    for(size_t i=0; i<testDataBF16Bit.size(); ++i) {
        testAlignedBitReverse<16>( "BF16Bit."+std::to_string(i), testDataBF16Bit[i] );
    }
    for(size_t i=0; i<testDataBF3Bit.size(); ++i) {
        testAlignedBitReverse<3>( "BF03Bit."+std::to_string(i), testDataBF3Bit[i] );
    }
}

template<jau::req::unsigned_integral StorageType, size_t BitSize>
static void testUnalignedBitReverse(std::string_view prefix, const size_t offset, const TestDataBF& d, jau::bitfield_t<StorageType, BitSize>& bf) {
    const bool verbose = true;
    if( verbose ) {
        std::cout << prefix << ", offset " << offset << ": " << d << "\n";
        std::cout << " bf bit-size[unit " << bf.unit_bit_size << ", total " << bf.bit_size << "], units " << bf.unit_size << "\n";
    }
    bf.clear();
    REQUIRE( true == bf.put(offset, d.pattern) );

    std::string s_be2 = bf.toString(offset, d.bitSize);
    std::string s_be2_rev = s_be2;
    std::ranges::reverse(s_be2_rev);
    std::string s_be3_rev = bf.reverse().toString(bf.size()-d.bitSize-offset, d.bitSize);
    if( verbose ) {
        std::cout << "  data:      " << d.pattern << "\n";
        std::cout << "  s_be2:     " << s_be2 << "\n";
        std::cout << "  s_be2_rev: " << s_be2_rev << "\n";
        std::cout << "  s_be3_rev: " << s_be3_rev << "\n";
    }
    REQUIRE( d.pattern == s_be2 );
    REQUIRE( s_be2_rev == s_be3_rev );
}

template<size_t BitSize>
static void testUnalignedBitReverse(std::string_view prefix, const TestDataBF& d) {
    jau::bitfield_t<uint64_t, BitSize> bf1;
    jau::bitfield_t<uint32_t, BitSize> bf2;
    jau::bitfield_t<uint16_t, BitSize> bf3;
    jau::bitfield_t<uint8_t, BitSize> bf4;
    testUnalignedBitReverse( prefix,  0, d, bf1 );
    testUnalignedBitReverse( prefix,  1, d, bf1 );
    testUnalignedBitReverse( prefix, 32, d, bf1 );
    testUnalignedBitReverse( prefix, 33, d, bf1 );

    testUnalignedBitReverse( prefix,  0, d, bf2 );
    testUnalignedBitReverse( prefix,  1, d, bf2 );
    testUnalignedBitReverse( prefix, 32, d, bf2 );
    testUnalignedBitReverse( prefix, 33, d, bf2 );

    testUnalignedBitReverse( prefix,  0, d, bf3 );
    testUnalignedBitReverse( prefix,  1, d, bf3 );
    testUnalignedBitReverse( prefix, 32, d, bf3 );
    testUnalignedBitReverse( prefix, 33, d, bf3 );

    testUnalignedBitReverse( prefix,  0, d, bf4 );
    testUnalignedBitReverse( prefix,  1, d, bf4 );
    testUnalignedBitReverse( prefix, 32, d, bf4 );
    testUnalignedBitReverse( prefix, 33, d, bf4 );
}

TEST_CASE("Bitfield Test 31 Unaligned Reverse", "[bitfield][bitreverse]") {
    for(size_t i=0; i<testDataBF64Bit.size(); ++i) {
        testUnalignedBitReverse<64*2+33>( "BF64Bit."+std::to_string(i), testDataBF64Bit[i] );
    }
    for(size_t i=0; i<testDataBF32Bit.size(); ++i) {
        testUnalignedBitReverse<32*2+33>( "BF32Bit."+std::to_string(i), testDataBF32Bit[i] );
    }
    for(size_t i=0; i<testDataBF16Bit.size(); ++i) {
        testUnalignedBitReverse<16*2+33>( "BF16Bit."+std::to_string(i), testDataBF16Bit[i] );
    }
    for(size_t i=0; i<testDataBF3Bit.size(); ++i) {
        testUnalignedBitReverse<3*2+33>( "BF03Bit."+std::to_string(i), testDataBF3Bit[i] );
    }
}
