/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020-2024 Gothel Software e.K.
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
#include <cstring>

#include <jau/test/catch2_ext.hpp>

#include <jau/basic_algos.hpp>
#include <jau/basic_types.hpp>
#include <jau/callocator.hpp>
#include <jau/counting_allocator.hpp>
#include <jau/counting_callocator.hpp>
#include <jau/darray.hpp>
#include <jau/float_types.hpp>

/**
 * Test general use of jau::darray, jau::cow_darray and jau::cow_vector.
 */
using namespace jau;

typedef jau::darray<int, jau::nsize_t, jau::callocator<int>, std::is_trivially_copyable_v<int>, true> secure_ints;
typedef jau::darray<int, jau::nsize_t, jau::callocator<int>, std::is_trivially_copyable_v<int>, false> normal_ints;

TEST_CASE( "JAU DArray Test 00 - ctti", "[ctti][datatype][jau][darray]" ) {
    typedef jau::darray<uint8_t> ByteBuffer;
    typedef jau::darray<int32_t> IntBuffer;
    typedef jau::darray<jau::float32_t> FloatBuffer;
    ByteBuffer b0(10);
    ByteBuffer b1(11);
    IntBuffer i0(10);
    IntBuffer i1(11);
    FloatBuffer f0(10);
    FloatBuffer f1(11);

    std::cout << "RTTI: " << jau::is_rtti_available() << std::endl;
    std::cout << std::endl;
    std::cout << "b0: " << b0 << std::endl;
    std::cout << "b1: " << b1 << std::endl;
    std::cout << "i0: " << i0 << std::endl;
    std::cout << "i1: " << i1 << std::endl;
    std::cout << "f0: " << f0 << std::endl;
    std::cout << "f1: " << f1 << std::endl;
    std::cout << std::endl;
    std::cout << "byte:     " << jau::int_ctti::u8() << std::endl;
    std::cout << "int:      " << jau::int_ctti::i32() << std::endl;
    std::cout << "float:    " << jau::float_ctti::f32() << std::endl;
    std::cout << std::endl;
    std::cout << "b0 value: " << b0.valueSignature() << std::endl;
    std::cout << "b1 value: " << b1.valueSignature() << std::endl;
    std::cout << "i0 value: " << i0.valueSignature() << std::endl;
    std::cout << "i1 value: " << i1.valueSignature() << std::endl;
    std::cout << "b0 self:  " << b0.classSignature() << std::endl;
    std::cout << "b1 self:  " << b1.classSignature() << std::endl;
    std::cout << "i0 self:  " << i0.classSignature() << std::endl;
    std::cout << "i1 self:  " << i1.classSignature() << std::endl;

    REQUIRE( b0.classSignature() == b1.classSignature() );
    REQUIRE( i0.classSignature() == i1.classSignature() );
    REQUIRE( b0.classSignature() != i1.classSignature() );

    REQUIRE( b0.valueSignature() == b1.valueSignature() );
    REQUIRE( i0.valueSignature() == i1.valueSignature() );
    REQUIRE( f0.valueSignature() == f1.valueSignature() );
    REQUIRE( b0.valueSignature() != i1.valueSignature() );
    REQUIRE( b0.valueSignature() != f1.valueSignature() );

    REQUIRE( b0.valueSignature() == jau::int_ctti::u8() );
    REQUIRE( b1.valueSignature() == jau::int_ctti::u8() );
    REQUIRE( i0.valueSignature() == jau::int_ctti::i32() );
    REQUIRE( i1.valueSignature() == jau::int_ctti::i32() );
    REQUIRE( f0.valueSignature() == jau::float_ctti::f32() );
    REQUIRE( f1.valueSignature() == jau::float_ctti::f32() );
    REQUIRE( b0.valueSignature() != jau::int_ctti::i32() );
    REQUIRE( i0.valueSignature() != jau::int_ctti::u8() );
    REQUIRE( f0.valueSignature() != jau::float_ctti::f64() );
    REQUIRE( f0.valueSignature() != jau::int_ctti::u8() );
}

template<class T>
static void int_test() {
    T data;
    // printf("COPY-0.1: %s\n", data.getInfo().c_str());
    CHECK( false == data.pinned() );
    CHECK( false == data.shared() );
    CHECK( 0 == data.position() );
    CHECK( 0 == data.limit() );
    CHECK( 0 == data.size() );
    CHECK( 0 == data.capacity() );
    data.reserve(2);
    CHECK( 0 == data.position() );
    CHECK( 0 == data.limit() );
    CHECK( 0 == data.size() );
    CHECK( 2 == data.capacity() );
    data.resize(2);
    CHECK( 0 == data.position() );
    CHECK( 2 == data.limit() );
    CHECK( 2 == data.size() );
    CHECK( 2 == data.capacity() );
    int j=0;
    for(int i : data) {
        CHECK(0 == i);
        ++j;
    }
    CHECK(2 == j);

    data.resize(4, 42);
    CHECK( 0 == data.position() );
    CHECK( 4 == data.limit() );
    CHECK( 4 == data.size() );
    CHECK( 4 == data.capacity() );
    j = 0;
    for(int i : data) {
        if( j < 2 ) {
            CHECK(0 == i);
        } else {
            CHECK(42 == i);
        }
        ++j;
    }
    CHECK(4 == j);

    data.erase(data.cbegin(), data.cend());
    CHECK( 0 == data.position() );
    CHECK( 0 == data.limit() );
    CHECK( 0 == data.size() );
    CHECK( 4 == data.capacity() );

    data.push_back(1);
    data.push_back(2);
    CHECK( 2 == data.position() );
    CHECK( 2 == data.limit() );
    CHECK( 2 == data.size() );
    CHECK( 4 == data.capacity() );
    CHECK(1 == data[0]);
    CHECK(2 == data[1]);

    data.shrink_to_fit();
    CHECK( 2 == data.position() );
    CHECK( 2 == data.limit() );
    CHECK( 2 == data.size() );
    CHECK( 2 == data.capacity() );
    CHECK(1 == data[0]);
    CHECK(2 == data[1]);

    data.setGrowthFactor(0);
    // printf("PINNED-0.6: %s\n", data.getInfo().c_str());
    CHECK( true  == data.pinned() );
    CHECK( false == data.shared() );
    REQUIRE_THROWS_AS( data.push_back(42), jau::IllegalStateError );
    CHECK( 2 == data.position() );
    CHECK( 2 == data.limit() );
    CHECK( 2 == data.size() );
    CHECK( 2 == data.capacity() );
    data.setGrowthFactor(2.0f);
    CHECK( false  == data.pinned() );
    CHECK( false == data.shared() );

    data.erase(data.cbegin(), data.cend());
    CHECK( 0 == data.position() );
    CHECK( 0 == data.limit() );
    CHECK( 0 == data.size() );
    CHECK( 2 == data.capacity() );

    data.shrink_to_fit();
    // printf("SHRINK-0.8: %s\n", data.getInfo().c_str());
    CHECK( 0 == data.position() );
    CHECK( 0 == data.limit() );
    CHECK( 0 == data.size() );
    CHECK( 0 == data.capacity() );

    data.push_back(42);
    CHECK( 1 == data.position() );
    CHECK( 1 == data.limit() );
    CHECK( 1  == data.size() );
    CHECK( 10 == data.capacity() );
    CHECK(42 == data[0]);

    data.push_back(43);
    CHECK( 2 == data.position() );
    CHECK( 2 == data.limit() );
    CHECK( 2  == data.size() );
    CHECK( 10 == data.capacity() );
    CHECK(42 == data[0]);
    CHECK(43 == data[1]);

    data.setPosition(0);
    // printf("POS-1.0: %s\n", data.getInfo().c_str());
    CHECK( 0 == data.position() );
    CHECK( 2 == data.limit() );
    CHECK( 2  == data.size() );
    CHECK( 10 == data.capacity() );

    data.setPosition(1);
    CHECK( 1 == data.position() );
    CHECK( 2 == data.limit() );
    CHECK( 2  == data.size() );
    CHECK( 10 == data.capacity() );

    data.setPosition(2);
    CHECK( 2 == data.position() );
    CHECK( 2 == data.limit() );
    CHECK( 2  == data.size() );
    CHECK( 10 == data.capacity() );

    REQUIRE_THROWS_AS( data.setPosition(3), jau::IndexOutOfBoundsError );
    REQUIRE_THROWS_AS( data.setLimit(3), jau::IndexOutOfBoundsError );

    data.resize(5, 42);
    CHECK( 2 == data.position() );
    CHECK( 5 == data.limit() );
    CHECK( 5  == data.size() );
    CHECK( 10 == data.capacity() );

    data.pop_back();
    CHECK( 2 == data.position() );
    CHECK( 4 == data.limit() );
    CHECK( 4  == data.size() );
    CHECK( 10 == data.capacity() );

    data.setLimit(3);
    CHECK( 2 == data.position() );
    CHECK( 3 == data.limit() );
    CHECK( 4  == data.size() );
    CHECK( 10 == data.capacity() );
    data.setLimit(1);
    CHECK( 1 == data.position() );
    CHECK( 1 == data.limit() );
    CHECK( 4  == data.size() );
    CHECK( 10 == data.capacity() );
    data.setLimit(4);
    data.setPosition(3);
    CHECK( 3 == data.position() );
    CHECK( 4 == data.limit() );
    CHECK( 4  == data.size() );
    CHECK( 10 == data.capacity() );

    typename T::value_type val = 0;
    for(auto it = data.begin(); it != data.end(); ++it) { *it=val++; }
    data.erase(data.begin()+1);
    // printf("ERASE-2.0: %s\n", data.getInfo().c_str());
    // printf("ERASE-2.0: %s\n", data.toString().c_str());
    CHECK( 2 == data.position() );
    CHECK( 3 == data.limit() );
    CHECK( 3  == data.size() );
    CHECK( 10 == data.capacity() );
    CHECK( data[0] == 0 );
    CHECK( data[1] == 2 );
    CHECK( data[2] == 3 );

    data.resize(10);
    val = 0;
    for(auto it = data.begin(); it != data.end(); ++it) { *it=val++; }
    CHECK( 2 == data.position() );
    CHECK( 10 == data.limit() );
    CHECK( 10 == data.size() );
    CHECK( 10 == data.capacity() );
    data.setPosition(8);
    CHECK( 8 == data.position() );

    data.erase(data.begin()+1, data.begin()+4);
    CHECK( 5 == data.position() );
    CHECK( 7 == data.limit() );
    CHECK( 7 == data.size() );
    CHECK( 10 == data.capacity() );
    CHECK( data[0] == 0 );
    CHECK( data[1] == 4 );
    CHECK( data[2] == 5 );

    data.resize(10);
    val = 0;
    for(auto it = data.begin(); it != data.end(); ++it) { *it=val++; }
    CHECK( 5 == data.position() );
    CHECK( 10 == data.limit() );
    CHECK( 10 == data.size() );
    CHECK( 10 == data.capacity() );

    data.erase(data.begin()+3, data.begin()+8);
    CHECK( 3 == data.position() );
    CHECK( 5 == data.limit() );
    CHECK( 5 == data.size() );
    CHECK( 10 == data.capacity() );
    CHECK( data[0] == 0 );
    CHECK( data[1] == 1 );
    CHECK( data[2] == 2 );
    CHECK( data[3] == 8 );
    CHECK( data[4] == 9 );

    data.clear();
    CHECK( 0 == data.position() );
    CHECK( 0 == data.limit() );
    CHECK( 0 == data.size() );
    CHECK( 10 == data.capacity() );
    data.resize(10);
    CHECK( 0 == data.position() );
    CHECK( 10 == data.limit() );
    CHECK( 10 == data.size() );
    CHECK( 10 == data.capacity() );
    data.clear(true);
    CHECK( 0 == data.position() );
    CHECK( 0 == data.limit() );
    CHECK( 0 == data.size() );
    CHECK( 0 == data.capacity() );
}

TEST_CASE( "JAU DArray Test 01 - basics", "[basics][datatype][jau][darray]" ) {
    int_test<normal_ints>();
    int_test<secure_ints>();
}

TEST_CASE( "JAU DArray Test 02 - slice", "[slice][datatype][jau][darray]" ) {
    normal_ints d0(nullptr, 10, 0);
    CHECK( false == d0.pinned() );
    CHECK( false == d0.shared() );
    CHECK(  0 == d0.position() );
    CHECK( 10 == d0.limit() );
    CHECK( 10 == d0.size() );
    CHECK( 10 == d0.capacity() );

    {
        normal_ints s0 = d0.duplicate();
        CHECK( true == d0.pinned() );
        CHECK( false == d0.shared() );
        CHECK( true == s0.pinned() );
        CHECK( true == s0.shared() );
        CHECK(  0 == s0.position() );
        CHECK( 10 == s0.limit() );
        CHECK( 10 == s0.size() );
        CHECK( 10 == s0.capacity() );
        CHECK( d0.begin() == s0.begin() );
        CHECK( d0.position_ptr() == s0.position_ptr() );
        CHECK( d0.position() == s0.position() );
        CHECK( d0.limit_ptr() == s0.limit_ptr() );
        CHECK( d0.limit() == s0.limit() );
        CHECK( d0.end() == s0.end() );
        CHECK( d0.size() == s0.size() );
        CHECK( d0.capacity() == s0.capacity() );
        REQUIRE_THROWS_AS( d0.push_back(42), jau::IllegalStateError );
        REQUIRE_THROWS_AS( s0.push_back(42), jau::IllegalStateError );
    }

    {
        normal_ints s0 = d0.slice();
        CHECK( true == d0.pinned() );
        CHECK( false == d0.shared() );
        CHECK( true == s0.pinned() );
        CHECK( true == s0.shared() );
        CHECK(  0 == s0.position() );
        CHECK( 10 == s0.limit() );
        CHECK( 10 == s0.size() );
        CHECK( 10 == s0.capacity() );
        CHECK( d0.begin() == s0.begin() );
        CHECK( d0.position_ptr() == s0.position_ptr() );
        CHECK( d0.position() == s0.position() );
        CHECK( d0.limit_ptr() == s0.limit_ptr() );
        CHECK( d0.limit() == s0.limit() );
        CHECK( d0.end() == s0.end() );
        CHECK( d0.size() == s0.size() );
        CHECK( d0.capacity() == s0.capacity() );
        REQUIRE_THROWS_AS( d0.push_back(42), jau::IllegalStateError );
        REQUIRE_THROWS_AS( s0.push_back(42), jau::IllegalStateError );
    }

    d0.setPosition(5);
    {
        normal_ints s0 = d0.duplicate();
        CHECK( true == d0.pinned() );
        CHECK( false == d0.shared() );
        CHECK( true == s0.pinned() );
        CHECK( true == s0.shared() );
        CHECK(  5 == s0.position() );
        CHECK( 10 == s0.limit() );
        CHECK( 10 == s0.size() );
        CHECK( 10 == s0.capacity() );
        CHECK( d0.begin() == s0.begin() );
        CHECK( d0.position_ptr() == s0.position_ptr() );
        CHECK( d0.position() == s0.position() );
        CHECK( d0.limit_ptr() == s0.limit_ptr() );
        CHECK( d0.limit() == s0.limit() );
        CHECK( d0.end() == s0.end() );
        CHECK( d0.size() == s0.size() );
        CHECK( d0.capacity() == s0.capacity() );
        REQUIRE_THROWS_AS( d0.push_back(42), jau::IllegalStateError );
        REQUIRE_THROWS_AS( s0.push_back(42), jau::IllegalStateError );
    }
    {
        normal_ints s0 = d0.slice();
        CHECK( true == d0.pinned() );
        CHECK( false == d0.shared() );
        CHECK( true == s0.pinned() );
        CHECK( true == s0.shared() );
        CHECK(  0 == s0.position() );
        CHECK(  5 == s0.limit() );
        CHECK(  5 == s0.size() );
        CHECK(  5 == s0.capacity() );
        CHECK( d0.position_ptr() == s0.begin() );
        CHECK( d0.position_ptr() == s0.position_ptr() );
        CHECK( d0.limit_ptr() == s0.limit_ptr() );
        CHECK(  5 == s0.limit() );
        CHECK( d0.end() == s0.end() );
        CHECK(  5 == s0.size() );
        CHECK(  5 == s0.capacity() );
        REQUIRE_THROWS_AS( d0.push_back(42), jau::IllegalStateError );
        REQUIRE_THROWS_AS( s0.push_back(42), jau::IllegalStateError );
    }
}

TEST_CASE( "JAU DArray Test 03 - put/get", "[put][get][datatype][jau][darray]" ) {
    normal_ints d0(nullptr, 10);
    CHECK( false == d0.pinned() );
    CHECK( false == d0.shared() );
    CHECK(  0 == d0.position() );
    CHECK( 10 == d0.remaining() );
    CHECK( 10 == d0.limit() );
    CHECK( 10 == d0.size() );
    CHECK( 10 == d0.capacity() );
    int v = 0;
    for(normal_ints::size_type i=0; i<6; ++i) {
        d0.put(v++);
    }
    CHECK(  6 == d0.position() );
    CHECK(  4 == d0.remaining() );
    CHECK( 10 == d0.limit() );
    CHECK( 10 == d0.size() );
    CHECK( 10 == d0.capacity() );
    d0.flip();
    CHECK(  0 == d0.position() );
    CHECK(  6 == d0.remaining() );
    CHECK(  6 == d0.limit() );
    CHECK( 10 == d0.size() );
    CHECK( 10 == d0.capacity() );
    v = 0;
    while( d0.hasRemaining() ) {
        CHECK( v++ == d0.get() );
    }
    CHECK(  6 == d0.position() );
    CHECK(  0 == d0.remaining() );
    CHECK(  6 == d0.limit() );
    CHECK( 10 == d0.size() );
    CHECK( 10 == d0.capacity() );
    REQUIRE_THROWS_AS( d0.get(), jau::IndexOutOfBoundsError );
    CHECK(  6 == d0.position() );
    CHECK(  0 == d0.remaining() );
    CHECK(  6 == d0.limit() );
    CHECK( 10 == d0.size() );
    CHECK( 10 == d0.capacity() );

    d0.clearPosition();
    CHECK(  0 == d0.position() );
    CHECK( 10 == d0.remaining() );
    CHECK( 10 == d0.limit() );
    CHECK( 10 == d0.size() );
    CHECK( 10 == d0.capacity() );
    d0.putN(False(), 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 );
    CHECK( 10 == d0.position() );
    CHECK(  0 == d0.remaining() );
    CHECK( 10 == d0.limit() );
    CHECK( 10 == d0.size() );
    CHECK( 10 == d0.capacity() );

    d0.flip();
    CHECK(  0 == d0.position() );
    CHECK( 10 == d0.remaining() );
    CHECK( 10 == d0.limit() );
    CHECK( 10 == d0.size() );
    CHECK( 10 == d0.capacity() );
    v = 10;
    while(d0.hasRemaining()) {
        CHECK( v++ == d0.get() );
    }

    d0.rewind();
    CHECK(  0 == d0.position() );
    CHECK( 10 == d0.remaining() );
    CHECK( 10 == d0.limit() );
    CHECK( 10 == d0.size() );
    CHECK( 10 == d0.capacity() );
    REQUIRE_THROWS_AS( d0.putN(False(), 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 ), jau::IndexOutOfBoundsError );
    CHECK(  0 == d0.position() );
    CHECK( 10 == d0.remaining() );
    CHECK( 10 == d0.limit() );
    CHECK( 10 == d0.size() );
    CHECK( 10 == d0.capacity() );

    d0.putN(True(), 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 );
    std::cout << "p1: " << d0.getInfo() << "\n";
    std::cout << "p1: " << d0 << "\n";
    CHECK( 11 == d0.position() );
    CHECK(  0 == d0.remaining() );
    CHECK( 11 == d0.limit() );
    CHECK( 11 == d0.size() );
    CHECK( 11 <  d0.capacity() );
    auto c1 = d0.capacity();

    REQUIRE_THROWS_AS( d0.put(21), jau::IndexOutOfBoundsError );
    d0.put(21, True());
    std::cout << "p2: " << d0.getInfo() << "\n";
    std::cout << "p2: " << d0 << "\n";
    CHECK( 12 == d0.position() );
    CHECK(  0 == d0.remaining() );
    CHECK( 12 == d0.limit() );
    CHECK( 12 == d0.size() );
    CHECK( c1 == d0.capacity() );

    REQUIRE_THROWS_AS( d0.putN(False(), 22, 23, 24, 25 ), jau::IndexOutOfBoundsError );
    d0.putN(True(), 22, 23, 24, 25 );
    std::cout << "p3: " << d0.getInfo() << "\n";
    std::cout << "p3: " << d0 << "\n";
    CHECK( 16 == d0.position() );
    CHECK(  0 == d0.remaining() );
    CHECK( 16 == d0.limit() );
    CHECK( 16 == d0.size() );
    CHECK( 16 == d0.capacity() );

    d0.setGrowthFactor(1.0f);
    CHECK( false == d0.pinned() );
    CHECK( false == d0.shared() );
    d0.put(26, True());
    std::cout << "p4: " << d0.getInfo() << "\n";
    std::cout << "p4: " << d0 << "\n";
    CHECK( 17 == d0.position() );
    CHECK(  0 == d0.remaining() );
    CHECK( 17 == d0.limit() );
    CHECK( 17 == d0.size() );
    CHECK( 17 == d0.capacity() );

    d0.push_back(27);
    std::cout << "p5: " << d0.getInfo() << "\n";
    std::cout << "p5: " << d0 << "\n";
    CHECK( 18 == d0.position() );
    CHECK(  0 == d0.remaining() );
    CHECK( 18 == d0.limit() );
    CHECK( 18 == d0.size() );
    CHECK( 18 == d0.capacity() );

    d0.flip();
    CHECK(  0 == d0.position() );
    CHECK( 18 == d0.remaining() );
    CHECK( 18 == d0.limit() );
    CHECK( 18 == d0.size() );
    CHECK( 18 == d0.capacity() );
    v = 10;
    while(d0.hasRemaining()) {
        CHECK( v++ == d0.get() );
    }
    CHECK( 18 == d0.position() );
    CHECK(  0 == d0.remaining() );
    CHECK( 18 == d0.limit() );
    CHECK( 18 == d0.size() );
    CHECK( 18 == d0.capacity() );
}
