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

#include <jau/bitfield.hpp>
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
        REQUIRE(2_uz*64_uz == b1.set(64, 2_uz*64_uz, true).count());
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
        REQUIRE(2_uz*64_uz+2_uz == b1.set(33, 2_uz*64_uz+2_uz, true).count());
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
        REQUIRE(2_uz*32_uz+2_uz == b1.set(17, 2_uz*32_uz+2_uz, true).count());
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
        REQUIRE(2_uz*8_uz+2_uz == b1.set(5, 2_uz*8_uz+2_uz, true).count());
    }
}

TEST_CASE( "Bitfield Test 01 BitCount32_One", "[bitfield]" ) {
    const std::vector<std::string_view> &pyramid32bit_one  = BitDemoData::pyramid32bit_one;
    for(size_t i=0; i<pyramid32bit_one.size(); i++) {
        const uint32_t val0 = 1 << i;
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
    const uint32_t val;
    const std::string_view pattern;

    TestDataBF(size_t bitSize_, const uint64_t value_, const std::string_view pattern_)
    : bitSize(bitSize_), val(value_), pattern(pattern_) { }
};

static std::vector<TestDataBF> testDataBF32Bit = {
    TestDataBF(32, BitDemoData::testIntMSB, BitDemoData::testStringMSB),
    TestDataBF(32, BitDemoData::testIntMSB_rev, BitDemoData::testStringMSB_rev),
    TestDataBF(32, BitDemoData::testIntLSB, BitDemoData::testStringLSB),
    TestDataBF(32, BitDemoData::testIntLSB_revByte, BitDemoData::testStringLSB_revByte),

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

static void test_ValidateTestData(const TestDataBF& d) {
    const size_t oneBitCount0 = jau::bit_count(d.val);
    const size_t oneBitCount1 = BitDemoData::getOneBitCount(d.pattern);
    REQUIRE(oneBitCount0 == oneBitCount1);

    const std::string& pattern0 = BitDemoData::toBinaryString(d.val, d.bitSize);
    REQUIRE(d.pattern == pattern0);

    const uint64_t val1 = BitDemoData::toInteger(d.pattern);
    REQUIRE(d.val == val1);
    REQUIRE(d.bitSize == pattern0.length());
}

TEST_CASE( "Bitfield Test 20 ValidateTestData", "[bitfield]" ) {
    for(const TestDataBF& d : testDataBF32Bit) {
        test_ValidateTestData( d );
    }
    for(const TestDataBF& d : testDataBF16Bit) {
        test_ValidateTestData( d );
    }
    for(const TestDataBF& d : testDataBF3Bit) {
        test_ValidateTestData( d );
    }
}


/**
 * ***********************************************************************************
 * ***********************************************************************************
 * ***********************************************************************************
 */

template<jau::req::unsigned_integral StorageType, size_t BitSize>
static void assertEquals(const jau::bitfield_t<StorageType, BitSize>& bf, const size_t bf_off, const uint32_t v, const std::string_view pattern, size_t oneBitCount) {
    const size_t len = pattern.length();
    for(size_t i=0; i<len; i++) {
        const bool exp0 = 0 != ( v & ( 1 << i ) );
        const bool exp1 = '1' == pattern[len-1-i];
        const bool has = bf.get(i+bf_off);
        (void)oneBitCount;
        // fprintf(stderr, "Pos %04zu: Value 0x%08" PRIu64 "x / %s, c %zu\n", i, v, std::string(pattern).c_str(), oneBitCount);
        REQUIRE(exp0 == has);
        REQUIRE(exp1 == has);
    }
}

template<jau::req::unsigned_integral StorageType, size_t BitSize1, size_t BitSize2>
static void test_AlignedBits(const TestDataBF& d, jau::bitfield_t<StorageType, BitSize1>& bf1, jau::bitfield_t<StorageType, BitSize2>& bf2) {
    const size_t oneBitCount = jau::bit_count(d.val);

    REQUIRE(true == bf1.putUnit( 0, d.bitSize, d.val) );
    REQUIRE(d.val == bf1.getUnit(0, d.bitSize));
    REQUIRE(oneBitCount == bf1.count());
    assertEquals(bf1, 0, d.val, d.pattern, oneBitCount);

    bf2.putUnit( 0, d.bitSize, d.val);
    REQUIRE(d.val == bf2.getUnit( 0, d.bitSize));
    REQUIRE(oneBitCount*1 == bf2.count());
    assertEquals(bf2, 0, d.val, d.pattern, oneBitCount);

    bf2.putUnit(64, d.bitSize, d.val);
    REQUIRE(d.val == bf2.getUnit(64, d.bitSize));
    REQUIRE(oneBitCount*2 == bf2.count());
    assertEquals(bf2, 64, d.val, d.pattern, oneBitCount);

    REQUIRE(d.val == bf2.copyUnit(0, 96, d.bitSize));
    REQUIRE(d.val == bf2.getUnit(96, d.bitSize));
    REQUIRE(oneBitCount*3 == bf2.count());
    assertEquals(bf2, 96, d.val, d.pattern, oneBitCount);
}
static void test_AlignedBits(const TestDataBF& d) {
    {
        jau::bitfield_t<uint64_t, 64> bf1;
        jau::bitfield_t<uint64_t, 64 + 128> bf2;
        test_AlignedBits(d, bf1, bf2);
    }
    {
        jau::bitfield_t<uint32_t, 64> bf1;
        jau::bitfield_t<uint32_t, 64 + 128> bf2;
        test_AlignedBits(d, bf1, bf2);
    }
}

TEST_CASE( "Bitfield Test 21 Alignedbits", "[bitfield]" ) {
    for(const auto & i : testDataBF32Bit) {
        test_AlignedBits( i );
    }
    for(const auto & i : testDataBF16Bit) {
        test_AlignedBits( i );
    }
    for(const auto & i : testDataBF3Bit) {
        test_AlignedBits( i );
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
static void test_Unaligned(const TestDataBF d, jau::bitfield_t<StorageType, BitSize>& bf, const size_t lowBitnum) {
    const size_t maxBitpos = bf.size()-d.bitSize;
    const size_t oneBitCount = jau::bit_count(d.val);

    const std::string msg = jau::format_string("Value 0x%08x / %s, l %zu/%zu, c %zu, lbPos %zu -> %zu",
            d.val, std::string(d.pattern).c_str(), d.bitSize, bf.size(), oneBitCount, lowBitnum, maxBitpos);

    //
    // via putUnit
    //
    bf.putUnit( lowBitnum, d.bitSize, d.val);
    for(size_t i=0; i<d.bitSize; i++) {
        const bool exp = d.val & ( 1 << i );
        const bool has = bf.get(lowBitnum+i);
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
        REQUIRE_MSG(msg, d.val == bf.copyUnit(lowBitnum, lowBitnum+1, d.bitSize));
        bf.clr(lowBitnum);
        REQUIRE_MSG(msg, d.val == bf.getUnit(lowBitnum+1, d.bitSize));
        REQUIRE_MSG(msg, oneBitCount == bf.count());
        assertEquals(bf, lowBitnum+1, d.val, d.pattern, oneBitCount);
    }

    // test put/get
    bf.reset();
    REQUIRE_MSG(msg+", bitpos 0", false == bf.get(lowBitnum));
    bf.put(lowBitnum, true);
    REQUIRE_MSG(msg+", bitpos 0", true == bf.get(lowBitnum));
    bf.put(lowBitnum, false);
    REQUIRE_MSG(msg+", bitpos 0", false == bf.get(lowBitnum));

    //
    // via put/get
    //
    for(size_t i=0; i<d.bitSize; i++) {
        REQUIRE_MSG(msg+", bitpos "+std::to_string(i), false == bf.get(lowBitnum+i));
        const bool v = d.val & ( 1 << i );
        bf.put(lowBitnum+i, v);
        REQUIRE_MSG(msg+", bitpos "+std::to_string(i), v == bf.get(lowBitnum+i));
    }
    REQUIRE_MSG(msg, d.val == bf.getUnit(lowBitnum, d.bitSize));
    for(size_t i=0; i<d.bitSize; i++) {
        const bool exp = d.val & ( 1 << i );
        const bool has = bf.get(lowBitnum+i);
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
            const bool exp = d.val & (1 << i);
            const bool has = bf.copy(lowBitnum+i, lowBitnum+1+i);
            REQUIRE_MSG(msg+", bitpos "+std::to_string(i), exp == has);
        }
        bf.clr(lowBitnum);
        REQUIRE_MSG(msg, d.val == bf.getUnit( lowBitnum+1, d.bitSize));
        for(size_t i=0; i<d.bitSize; i++) {
            const bool exp = d.val & (1 << i);
            const bool has = bf.get(lowBitnum+1+i);
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
            if( d.val & ( 1 << i ) ) {
                bf.set(lowBitnum+i);
            } else {
                bf.clr(lowBitnum+i);
            }
        }
        REQUIRE_MSG(msg, d.val == bf.getUnit(lowBitnum, d.bitSize));
        for(size_t i=0; i<d.bitSize; i++) {
            const bool exp = d.val & (1 << i);
            const bool has = bf.get(lowBitnum+i);
            REQUIRE_MSG(msg+", bitpos "+std::to_string(i), exp == has);
        }
        REQUIRE_MSG(msg, oneBitCount == bf.count());
        assertEquals(bf, lowBitnum, d.val, d.pattern, oneBitCount);
    }
    {
        REQUIRE(bf.bit_size == bf.setAll(true).count());
        bf.set(0, lowBitnum, false);
        bf.set(lowBitnum+d.bitSize, bf.bit_size-(lowBitnum+d.bitSize), false);
        REQUIRE_MSG(msg, d.bitSize == bf.count());
        for(size_t i=0; i<d.bitSize; i++) {
            if( d.val & ( 1 << i ) ) {
                bf.set(lowBitnum+i);
            } else {
                bf.clr(lowBitnum+i);
            }
        }
        REQUIRE_MSG(msg, d.val == bf.getUnit(lowBitnum, d.bitSize));
        for(size_t i=0; i<d.bitSize; i++) {
            const bool exp = d.val & (1 << i);
            const bool has = bf.get(lowBitnum+i);
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
    bf.putUnit( lowBitnum, d.bitSize, d.val);
    checkOtherBits(d, bf, lowBitnum, msg, StorageType(0));

    bf.setAll(true);
    bf.putUnit( lowBitnum, d.bitSize, d.val);
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

