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
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>

#include <jau/basic_types.hpp>
#include <jau/bitfield.hpp>
#include <jau/bitheap.hpp>
#include <jau/debug.hpp>
#include <jau/int_types.hpp>
#include <jau/io/bit_stream.hpp>
#include <jau/io/byte_stream.hpp>
#include <jau/io/file_util.hpp>
#include <jau/io/io_util.hpp>
#include <jau/string_util.hpp>
#include <jau/test/catch2_ext.hpp>

// #include "test_httpd.hpp"
#include "data_bitstream.hpp"

extern "C" {
#include <unistd.h>
}

using namespace jau::fractions_i64_literals;
using jau::nsize_t;
// using size_type = jau::io::Bitstream::size_type;

TEST_CASE( "Bitstream Test 00", "[bitstream]" ) {
    REQUIRE( true == true );
}

static jau::bitheap getBitfield(const jau::nsize_t bitCount, const jau::bit_order_t bitOrder) {
    jau::bitheap source(bitCount);
    std::string_view in = BitDemoData::testStringMSB64_be;

    // msb 1111101011011110101011111111111011011110101011111100101011111110
    // lsb 0111111101010011111101010111101101111111111101010111101101011111
    //
    // msb 11111010 11011110 10101111 11111110 11011110 10101111 11001010 11111110
    // lsb 01111111 01010011 11110101 01111011 01111111 11110101 01111011 01011111
    for(size_t i=0; i<bitCount; ) {
        for(size_t j=64; i<bitCount && j-- > 0; ++i) {
            REQUIRE(true == source.put(i, in[j] == '1'));
        }
    }
    if( jau::bit_order_t::msb != bitOrder ) {
        source.reverse();
    }
    return source;
}

static jau::io::Bitstream getTestStream(const jau::bit_order_t dataBitOrder,
                                        const nsize_t preBits, const nsize_t skipBits, const nsize_t postBits) {
    const nsize_t bitCount = preBits + skipBits + postBits;
    const nsize_t byteCount = (bitCount + 7) / 8;
    jau::bitheap source = getBitfield(bitCount, dataBitOrder);
    jau::io::Bitstream bsTest(std::make_unique<jau::io::ByteStream_SecMemory>(byteCount, jau::io::iomode_t::rw), jau::io::ioaccess_t::write);
    fprintf(stderr, "TestStream.0: bitOrder[data %s], bits[pre %zu, skip %zu, post %zu = %zu]: %s\n",
            jau::to_string(dataBitOrder).c_str(),
            (size_t)preBits, (size_t)skipBits, (size_t)postBits, (size_t)bitCount, bsTest.toString().c_str());
    std::cerr << source << "\n";

    for( nsize_t i = 0; i < bitCount; i++ ) {
        REQUIRE(true == bsTest.writeBit(source[i]) );
        // fprintf(stderr, "TestData.1a: i %zu, %s\n", (size_t)i, bsTest.toString().c_str());
    }
    CHECK(preBits + skipBits + postBits == bsTest.position());

    REQUIRE(true == bsTest.setAccess(jau::io::ioaccess_t::read)); // switch to input-mode, implies flush()
    fprintf(stderr, "TestData.X: %s\n", bsTest.toString().c_str());
    REQUIRE(0 == bsTest.seek(0));
    BitDemoData::dumpData("TestStream.X", bsTest.byteStream());
    return bsTest;
}

static std::string getTestStreamResultAsString(const jau::bit_order_t dataBitOrder,
                                               const jau::nsize_t preBits, const jau::nsize_t skipBits, const jau::nsize_t postBits) {
    const jau::nsize_t totalBits = preBits+postBits;
    fprintf(stderr,"TestString: bitOrder %s, preBits %zu, skipBits %zu, postBits %zu, totalBits %zu\n",
            jau::to_string(dataBitOrder).c_str(), (size_t)preBits, (size_t)skipBits, (size_t)postBits, (size_t)totalBits);

    jau::bitheap source = getBitfield(preBits+skipBits+postBits, dataBitOrder);
    std::cerr << source << "\n";
    const auto [pre, preOK] = source.subbits(0, preBits);
    const auto [post, postOK] = source.subbits(preBits+skipBits, postBits);
    REQUIRE(true == preOK);
    REQUIRE(true == postOK);

    jau::bitheap r(preBits+postBits);
    REQUIRE(true == r.put(0, pre));
    REQUIRE(true == r.put(preBits, post));
    std::cerr << "ResultExp: <" << pre << "> + <" << post << "> = <" << r << ">\n";
    REQUIRE(totalBits == r.size());
    return r.toString();
}

static std::string readBits(jau::io::Bitstream *copy, jau::io::Bitstream &input,
                            const nsize_t preCount, const nsize_t count) /* throws IOException */ {
    fprintf(stderr, "ReadBits.0: count[pre %zu, actual %zu]: %s\n", (size_t)preCount, (size_t)count, input.toString().c_str());
    if( copy ) {
        fprintf(stderr, "ReadBits.0c: %s\n", copy->toString().c_str());
    }
    std::string sbRead;
    nsize_t i = 0;
    while( i < count ) {
        const int bit = input.readBit();
        if( 0 > bit ) {
            // fprintf(stderr, "ReadBits.1: EOS: i %zu, %s\n", (size_t)i, input.toString().c_str());
            break;
        } else {
            const char c = ( 0 != bit ) ? '1' : '0';
            sbRead.insert(0, 1, c);
            i++;
            // fprintf(stderr, "ReadBits.1: i %zu, '%c' -> %s, %s\n", (size_t)i, c, sbRead.c_str(), input.toString().c_str());
            REQUIRE(i+preCount == input.position());
            if( copy ) {
                REQUIRE(true == copy->writeBit(bit));
                // fprintf(stderr, "ReadBits.1c: i %zu, %s\n", (size_t)i, copy->toString().c_str());
                REQUIRE(i+preCount == copy->position());
            }
        }
    }
    fprintf(stderr, "ReadBits.2: %s\n", input.toString().c_str());
    REQUIRE(i+preCount == input.position());
    if( copy ) {
        fprintf(stderr, "ReadBits.2c: %s\n", copy->toString().c_str());
        REQUIRE(i+preCount == copy->position());
    }
    return sbRead;
}

static void testLinearBitsImpl(const jau::bit_order_t bitOrder, const nsize_t preBits, const nsize_t skipBits, const nsize_t postBits) {
    const nsize_t totalBits = preBits+skipBits+postBits;
    fprintf(stderr,"XXX TestLinearBits: bitOrder %s, preBits %zu, skipBits %zu, postBits %zu, totalBits %zu\n",
        jau::to_string(bitOrder).c_str(), (size_t)preBits, (size_t)skipBits, (size_t)postBits, (size_t)totalBits);

    // prepare bitstream
    std::cerr << "Prepare bitstream\n";
    jau::io::Bitstream bsTest = getTestStream(bitOrder, preBits, skipBits, postBits);
    const std::string sTest = getTestStreamResultAsString(bitOrder, preBits, skipBits, postBits);

    // init copy-bitstream
    const nsize_t byteCount = ( totalBits + 7 ) / 8;
    jau::io::Bitstream bsCopy(std::make_unique<jau::io::ByteStream_SecMemory>(byteCount, jau::io::iomode_t::rw), jau::io::ioaccess_t::write);

    // read-bitstream .. and copy bits while reading
    std::cerr << "Reading bitstream: <" << sTest << "> from " << bsTest << "\n";
    {
        const std::string sReadPre = readBits(&bsCopy, bsTest, 0, preBits);
        REQUIRE(skipBits == bsTest.skip(skipBits));
        REQUIRE(skipBits == bsCopy.skip(skipBits));

        const std::string sReadPost = readBits(&bsCopy, bsTest, preBits+skipBits, postBits);
        const std::string sRead = sReadPost + sReadPre;
        std::cerr << "Read.Test: <" << sTest << "> == <" << sReadPre << "> + <" << sReadPost << "> = <" << sRead << ">\n";
        REQUIRE(sTest == sRead);
        REQUIRE(totalBits == bsTest.position());
        REQUIRE(totalBits == bsCopy.position());
    }

    // read copy ..
    REQUIRE(true == bsCopy.setImmutable()); // switch to input-mode, implies flush()
    BitDemoData::dumpData("Copy", bsCopy.byteStream());
    REQUIRE(0 == bsTest.seek(0));

    std::cerr << "Reading copy-bitstream: " << bsCopy << "\n";
    REQUIRE(true == bsCopy.setMark(0)); // mark at beginning
    REQUIRE(0 == bsCopy.position());
    {
        const std::string sReadPre1 = readBits( nullptr, bsCopy, 0, preBits);
        REQUIRE(skipBits == bsCopy.skip(skipBits));

        const std::string sReadPost1 = readBits(nullptr, bsCopy, preBits+skipBits, postBits);
        const std::string sRead1 = sReadPost1 + sReadPre1;
        REQUIRE(sTest == sRead1);

        REQUIRE(true == bsCopy.seekMark());
        const std::string sReadPre2 = readBits(nullptr, bsCopy, 0, preBits);
        REQUIRE(sReadPre1 == sReadPre2);
        REQUIRE(skipBits == bsCopy.skip(skipBits));

        const std::string sReadPost2 = readBits(nullptr, bsCopy, preBits+skipBits, postBits);
        REQUIRE(sReadPost1 == sReadPost2);
        const std::string sRead2 = sReadPost2 + sReadPre2;
        REQUIRE(sTest == sRead2);
        REQUIRE(totalBits == bsCopy.position());
    }
}

static void testLinearBitsImpl(const jau::bit_order_t bitOrder) {
    testLinearBitsImpl(bitOrder,  0,  0,  1);
    testLinearBitsImpl(bitOrder,  0,  0,  3);
    testLinearBitsImpl(bitOrder,  0,  0,  7);
    testLinearBitsImpl(bitOrder,  0,  0,  8);
    testLinearBitsImpl(bitOrder,  0,  0,  9);
    testLinearBitsImpl(bitOrder,  0,  0, 20);
    testLinearBitsImpl(bitOrder,  0,  0, 31);
    testLinearBitsImpl(bitOrder,  0,  0, 32);
    testLinearBitsImpl(bitOrder,  0,  0, 33);
    testLinearBitsImpl(bitOrder,  0,  0, 63);
    testLinearBitsImpl(bitOrder,  0,  0, 64);
    testLinearBitsImpl(bitOrder,  0,  0, 65);
    testLinearBitsImpl(bitOrder,  0,  0, 80);
    testLinearBitsImpl(bitOrder,  0,  0, 127);
    testLinearBitsImpl(bitOrder,  0,  0, 128);
    testLinearBitsImpl(bitOrder,  0,  0, 129);
    testLinearBitsImpl(bitOrder,  0,  0, 140);

    testLinearBitsImpl(bitOrder,  3,  0,  3);
    testLinearBitsImpl(bitOrder,  8,  0,  3);
    testLinearBitsImpl(bitOrder,  9,  0,  3);

    testLinearBitsImpl(bitOrder,  0,  1,  1);
    testLinearBitsImpl(bitOrder,  0,  1,  3);
    testLinearBitsImpl(bitOrder,  0,  2,  8);
    testLinearBitsImpl(bitOrder,  0,  8, 10);
    testLinearBitsImpl(bitOrder,  0, 12, 20);
    testLinearBitsImpl(bitOrder,  0, 23,  9);

    testLinearBitsImpl(bitOrder,  1,  1,  1);
    testLinearBitsImpl(bitOrder,  2,  1,  3);
    testLinearBitsImpl(bitOrder,  7,  2,  8);
    testLinearBitsImpl(bitOrder,  8,  8,  8);
    testLinearBitsImpl(bitOrder, 15, 12,  5);
    testLinearBitsImpl(bitOrder, 16, 11,  5);
}

TEST_CASE( "Bitstream Test 01 LinearBitsMSBFirst", "[bitstream]" ) {
    testLinearBitsImpl(jau::bit_order_t::msb);
}

TEST_CASE( "Bitstream Test 02 LinearBitsLSBFirst", "[bitstream]" ) {
    testLinearBitsImpl(jau::bit_order_t::lsb);
}

//
//
//

static void testBulkBitsImpl(const nsize_t preBits, const nsize_t skipBits, const nsize_t postBits) /* throws IOException */ {
    const nsize_t totalBits = preBits+skipBits+postBits;
    fprintf(stderr,"XXX TestBulkBits: preBits %zu, skipBits %zu, postBits %zu, totalBits %zu\n",
        (size_t)preBits, (size_t)skipBits, (size_t)postBits, (size_t)totalBits);

    // prepare bitstream
    std::cerr << "Prepare bitstream\n";
    jau::io::Bitstream bsTest = getTestStream(jau::bit_order_t::msb, preBits, skipBits, postBits);
    const std::string sTest = getTestStreamResultAsString(jau::bit_order_t::msb, preBits, skipBits, postBits);

    // init copy-bitstream
    const nsize_t byteCount = ( totalBits + 7 ) / 8;
    jau::io::Bitstream bsCopy(std::make_unique<jau::io::ByteStream_SecMemory>(byteCount, jau::io::iomode_t::rw), jau::io::ioaccess_t::write);

    // read-bitstream .. and copy bits while reading
    std::cerr << "Reading bitstream: <" << sTest << "> from " << bsTest << "\n";
    {
        uint64_t readBitsPre;
        REQUIRE(preBits == bsTest.readBits64(preBits, readBitsPre));
        REQUIRE(preBits == bsCopy.writeBits64(preBits, readBitsPre));

        REQUIRE(skipBits == bsTest.skip(skipBits));
        REQUIRE(skipBits == bsCopy.skip(skipBits));

        uint64_t readBitsPost;
        REQUIRE(postBits == bsTest.readBits64(postBits, readBitsPost));
        REQUIRE(postBits == bsCopy.writeBits64(postBits, readBitsPost));

        const std::string sReadPreLo = preBits > 0 ? jau::toBitString(readBitsPre, jau::bit_order_t::msb,jau::PrefixOpt::none, preBits) : "";
        const std::string sReadPostHi = postBits > 0 ? jau::toBitString(readBitsPost, jau::bit_order_t::msb, jau::PrefixOpt::none, postBits) : "";
        const std::string sRead = sReadPostHi + sReadPreLo;
        std::cerr << "Read.Test: <" << sTest << "> == <" << sReadPreLo << "> + <" << sReadPostHi << "> = <" << sRead << ">\n";

        REQUIRE(sTest == sRead);
        REQUIRE(totalBits == bsTest.position());
        REQUIRE(totalBits == bsCopy.position());
    }

    // read copy ..
    REQUIRE(true == bsCopy.setImmutable()); // switch to input-mode, implies flush()
    BitDemoData::dumpData("Copy: ", bsCopy.byteStream());

    std::cerr << "Reading copy-bitstream: " << bsCopy << "\n";
    REQUIRE(true == bsCopy.setMark(0)); // mark at beginning
    REQUIRE(0 == bsCopy.position());
    {
        uint64_t copyBitsPre;
        REQUIRE(preBits == bsCopy.readBits64(preBits, copyBitsPre));
        REQUIRE(skipBits == bsCopy.skip(skipBits));

        uint64_t copyBitsPost;
        REQUIRE(postBits == bsCopy.readBits64(postBits, copyBitsPost));

        const std::string sReadPreLo = preBits > 0 ? jau::toBitString(copyBitsPre, jau::bit_order_t::msb,jau::PrefixOpt::none, preBits) : "";
        const std::string sReadPostHi = postBits > 0 ? jau::toBitString(copyBitsPost, jau::bit_order_t::msb, jau::PrefixOpt::none, postBits) : "";
        const std::string sRead = sReadPostHi + sReadPreLo;
        std::cerr << "Copy.Test: <" << sTest << "> == <" << sReadPreLo << "> + <" << sReadPostHi << "> = <" << sRead << ">\n";

        REQUIRE(sTest == sRead);
        REQUIRE(totalBits == bsCopy.position());
    }
}

TEST_CASE( "Bitstream Test 11 BulkBitsLSBFirst", "[bitstream]" ) {
    testBulkBitsImpl(0,  0,  1);
    testBulkBitsImpl(0,  0,  3);
    testBulkBitsImpl(0,  0,  8);
    testBulkBitsImpl(0,  0,  10);
    testBulkBitsImpl(0,  0,  30);
    testBulkBitsImpl(0,  0,  31);

    testBulkBitsImpl(3,  0,  3);
    testBulkBitsImpl(8,  0,  3);
    testBulkBitsImpl(9,  0,  3);
    testBulkBitsImpl(5,  0,  6);
    testBulkBitsImpl(5,  0,  8);

    testBulkBitsImpl(0,  1,  1);
    testBulkBitsImpl(3,  6,  4);

    testBulkBitsImpl(0,  1,  3);
    testBulkBitsImpl(0,  2,  8);
    testBulkBitsImpl(0,  8,  10);
    testBulkBitsImpl(0,  12, 20);
    testBulkBitsImpl(0,  23, 9);
    testBulkBitsImpl(0,  1,  31);

    testBulkBitsImpl(1,  1,  1);
    testBulkBitsImpl(2,  1,  3);
    testBulkBitsImpl(7,  2,  8);
    testBulkBitsImpl(8,  8,  8);
    testBulkBitsImpl(15, 12, 5);
    testBulkBitsImpl(16, 11, 5);
    testBulkBitsImpl(5,  6,  5);
    testBulkBitsImpl(5,  6,  8);
}

TEST_CASE( "Bitstream Test 21 ErrorHandling", "[bitstream]" ) {
    jau::io::Bitstream bsTest(std::make_unique<jau::io::ByteStream_SecMemory>(64, jau::io::iomode_t::rw), jau::io::ioaccess_t::write);
    std::cerr << "x0 " << bsTest << "\n";
    std::cerr << "x0 " << bsTest.byteStream() << "\n";

    REQUIRE(true == bsTest.canWrite());
    REQUIRE(-1 == bsTest.readBit());

    REQUIRE(true == bsTest.setAccess(jau::io::ioaccess_t::read));
    REQUIRE(false == bsTest.canWrite());
    REQUIRE(false == bsTest.writeBit(1));

    REQUIRE(true == bsTest.setAccess(jau::io::ioaccess_t::write));
    REQUIRE(true == bsTest.canWrite());
    REQUIRE(true == bsTest.writeBit(1));

    REQUIRE(true == bsTest.setAccess(jau::io::ioaccess_t::read));
    REQUIRE(false == bsTest.canWrite());
    REQUIRE(1 == bsTest.readBit());
}
