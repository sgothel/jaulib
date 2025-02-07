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
#include <cstring>

#include <jau/cpp_lang_util.hpp>
#include <jau/int_types.hpp>
#include <jau/float_types.hpp>
#include <jau/string_util.hpp>
#include <jau/test/catch2_ext.hpp>

#include <jau/basic_types.hpp>
#include <jau/mem_buffers.hpp>

using namespace jau::int_literals;

TEST_CASE( "MemBuffer CTTI Test 00", "[MemBuffer][ctti]" ) {
    typedef jau::DataBuffer<uint8_t> ByteBuffer;
    typedef jau::DataBuffer<int32_t> IntBuffer;
    typedef jau::DataBuffer<jau::float32_t> FloatBuffer;
    ByteBuffer::self_ref b0 = ByteBuffer::create(10);
    ByteBuffer::self_ref b1 = ByteBuffer::create(11);
    IntBuffer::self_ref i0 = IntBuffer::create(10);
    IntBuffer::self_ref i1 = IntBuffer::create(11);
    FloatBuffer::self_ref f0 = FloatBuffer::create(10);
    FloatBuffer::self_ref f1 = FloatBuffer::create(11);

    std::cout << "RTTI: " << jau::is_rtti_available() << std::endl;
    std::cout << std::endl;
    std::cout << "b0: " << *b0 << std::endl;
    std::cout << "b1: " << *b1 << std::endl;
    std::cout << "i0: " << *i0 << std::endl;
    std::cout << "i1: " << *i1 << std::endl;
    std::cout << "f0: " << *f0 << std::endl;
    std::cout << "f1: " << *f1 << std::endl;
    std::cout << std::endl;
    std::cout << "byte:     " << jau::int_ctti::u8() << std::endl;
    std::cout << "int:      " << jau::int_ctti::i32() << std::endl;
    std::cout << "float:    " << jau::float_ctti::f32() << std::endl;
    std::cout << std::endl;
    std::cout << "b0 value: " << b0->valueSignature() << std::endl;
    std::cout << "b1 value: " << b1->valueSignature() << std::endl;
    std::cout << "i0 value: " << i0->valueSignature() << std::endl;
    std::cout << "i1 value: " << i1->valueSignature() << std::endl;
    std::cout << "b0 self:  " << b0->classSignature() << std::endl;
    std::cout << "b1 self:  " << b1->classSignature() << std::endl;
    std::cout << "i0 self:  " << i0->classSignature() << std::endl;
    std::cout << "i1 self:  " << i1->classSignature() << std::endl;

    REQUIRE( b0->classSignature() == b1->classSignature() );
    REQUIRE( i0->classSignature() == i1->classSignature() );
    REQUIRE( b0->classSignature() != i1->classSignature() );

    REQUIRE( b0->valueSignature() == b1->valueSignature() );
    REQUIRE( i0->valueSignature() == i1->valueSignature() );
    REQUIRE( f0->valueSignature() == f1->valueSignature() );
    REQUIRE( b0->valueSignature() != i1->valueSignature() );
    REQUIRE( b0->valueSignature() != f1->valueSignature() );

    REQUIRE( b0->valueSignature() == jau::int_ctti::u8() );
    REQUIRE( b1->valueSignature() == jau::int_ctti::u8() );
    REQUIRE( i0->valueSignature() == jau::int_ctti::i32() );
    REQUIRE( i1->valueSignature() == jau::int_ctti::i32() );
    REQUIRE( f0->valueSignature() == jau::float_ctti::f32() );
    REQUIRE( f1->valueSignature() == jau::float_ctti::f32() );
    REQUIRE( b0->valueSignature() != jau::int_ctti::i32() );
    REQUIRE( i0->valueSignature() != jau::int_ctti::u8() );
    REQUIRE( f0->valueSignature() != jau::float_ctti::f64() );
    REQUIRE( f0->valueSignature() != jau::int_ctti::u8() );
}

template<typename T>
void test_putget01() {
    typedef jau::DataBuffer<T> TBuffer;
    typename TBuffer::self_ref b0 = TBuffer::create(10);

    std::cout << "RTTI: " << jau::is_rtti_available() << std::endl;
    std::cout << std::endl;
    std::cout << "b0: " << *b0 << std::endl;
    std::cout << std::endl;

    REQUIRE( 10 == b0->capacity());
    REQUIRE( 10 == b0->limit());
    REQUIRE(  0 == b0->position());
    REQUIRE( 10 == b0->remaining());

    size_t c=0;
    for(uint8_t w=0; w<5; ++w) {
        REQUIRE(   c == b0->position());
        REQUIRE(10-c == b0->remaining());
        b0->put(w);
        ++c;
    }
    REQUIRE( 10 == b0->capacity());
    REQUIRE( 10 == b0->limit());
    REQUIRE(  5 == b0->position());
    REQUIRE(  5 == b0->remaining());

    T v=5;
    while( b0->hasRemaining() ) {
        REQUIRE(   c == b0->position());
        REQUIRE(10-c == b0->remaining());
        b0->put(v++);
        ++c;
    }
    REQUIRE( 10 == b0->capacity());
    REQUIRE( 10 == b0->limit());
    REQUIRE( 10 == b0->position());
    REQUIRE(  0 == b0->remaining());

    b0->flip();
    REQUIRE( 10 == b0->capacity());
    REQUIRE( 10 == b0->limit());
    REQUIRE(  0 == b0->position());
    REQUIRE( 10 == b0->remaining());

    c=0;
    v=0;
    while( b0->hasRemaining() ) {
        REQUIRE(   c == b0->position());
        REQUIRE(10-c == b0->remaining());
        REQUIRE(v++ == b0->get());
        ++c;
    }
    REQUIRE( 10 == c);
    REQUIRE( 10 == b0->capacity());
    REQUIRE( 10 == b0->limit());
    REQUIRE( 10 == b0->position());
    REQUIRE(  0 == b0->remaining());

    //
    //
    //

    b0->clear();
    REQUIRE( 10 == b0->capacity());
    REQUIRE( 10 == b0->limit());
    REQUIRE(  0 == b0->position());
    REQUIRE( 10 == b0->remaining());

    c=0;
    for(uint8_t w=0; w<5; ++w) {
        REQUIRE(   c == b0->position());
        REQUIRE(10-c == b0->remaining());
        b0->put(w);
        ++c;
    }
    REQUIRE( 10 == b0->capacity());
    REQUIRE( 10 == b0->limit());
    REQUIRE(  5 == b0->position());
    REQUIRE(  5 == b0->remaining());

    b0->flip();
    REQUIRE( 10 == b0->capacity());
    REQUIRE(  5 == b0->limit());
    REQUIRE(  0 == b0->position());
    REQUIRE(  5 == b0->remaining());

    c=0;
    v=0;
    while( b0->hasRemaining() ) {
        REQUIRE(   c == b0->position());
        REQUIRE( 5-c == b0->remaining());
        REQUIRE(v++ == b0->get());
        ++c;
    }
    REQUIRE(  5 == c);
    REQUIRE( 10 == b0->capacity());
    REQUIRE(  5 == b0->limit());
    REQUIRE(  5 == b0->position());
    REQUIRE(  0 == b0->remaining());
}
TEST_CASE( "MemBuffer IO Test 01", "[MemBuffer][io]" ) {
    test_putget01<uint8_t>();
    test_putget01<uint16_t>();
    test_putget01<int32_t>();
    test_putget01<float>();
}

TEST_CASE( "MemBuffer IO Test 02", "[MemBuffer][io][exception]" ) {
    typedef jau::DataBuffer<int32_t> IntBuffer;

    // 1
    IntBuffer::self_ref i0 = IntBuffer::create(10);
    REQUIRE(  0 == i0->position() );
    REQUIRE( 10 == i0->limit() );
    REQUIRE( 10 == i0->remaining() );

    i0->flip();
    REQUIRE(  0 == i0->position() );
    REQUIRE(  0 == i0->limit() );
    REQUIRE(  0 == i0->remaining() );
    REQUIRE_THROWS_AS( i0->get(), jau::IndexOutOfBoundsError );
    REQUIRE_THROWS_AS( i0->put(1), jau::IndexOutOfBoundsError );

    // 2
    i0->clear();
    REQUIRE(  0 == i0->position() );
    REQUIRE( 10 == i0->limit() );
    REQUIRE( 10 == i0->remaining() );

    i0->put(1);
    REQUIRE(  1 == i0->position() );
    REQUIRE( 10 == i0->limit() );
    REQUIRE(  9 == i0->remaining() );

    i0->flip();
    REQUIRE(  0 == i0->position() );
    REQUIRE(  1 == i0->limit() );
    REQUIRE(  1 == i0->remaining() );

    REQUIRE(  1 == i0->get() );
    REQUIRE(  1 == i0->position() );
    REQUIRE(  1 == i0->limit() );
    REQUIRE(  0 == i0->remaining() );
    REQUIRE_THROWS_AS( i0->get(), jau::IndexOutOfBoundsError );

    // 3
    i0->clear();
    REQUIRE(  0 == i0->position() );
    REQUIRE( 10 == i0->limit() );
    REQUIRE( 10 == i0->remaining() );

    REQUIRE_THROWS_AS( i0->putPri(1_i64), jau::IllegalArgumentError );
    REQUIRE_THROWS_AS( i0->putPri(1.0), jau::IllegalArgumentError );

    i0->putPri(1_i8);
    REQUIRE(  1 == i0->position() );
    REQUIRE( 10 == i0->limit() );
    REQUIRE(  9 == i0->remaining() );

    i0->flip();
    REQUIRE(  0 == i0->position() );
    REQUIRE(  1 == i0->limit() );
    REQUIRE(  1 == i0->remaining() );

    REQUIRE_THROWS_AS( i0->getPri<uint8_t>(), jau::IllegalArgumentError );
    REQUIRE(  0 == i0->position() );
    REQUIRE(  1 == i0->limit() );
    REQUIRE(  1 == i0->remaining() );

    REQUIRE(  1_i64 == i0->getPri<int64_t>() );
    REQUIRE(  1 == i0->position() );
    REQUIRE(  1 == i0->limit() );
    REQUIRE(  0 == i0->remaining() );
}

template<typename T>
void test_put4_01_impl(jau::MemBuffer& b) {
    std::cout << "RTTI: " << jau::is_rtti_available() << std::endl;
    std::cout << std::endl;
    std::cout << "0: b: " << b << std::endl;

    REQUIRE( 16 == b.capacity());
    REQUIRE( 16 == b.limit());
    REQUIRE(  0 == b.position());
    REQUIRE( 16 == b.remaining());

    b.putPri( T( 0) );
    b.putPri( T( 1), T( 2) );
    b.putPri( T( 3), T( 4), T( 5) );
    b.putPri( T( 6), T( 7));
    b.putPri( T( 8), T( 9), T(10), T(11));
    b.putPri( T(12), T(13), T(14), T(15));
    REQUIRE( 16 == b.capacity());
    REQUIRE( 16 == b.limit());
    REQUIRE( 16 == b.position());
    REQUIRE(  0 == b.remaining());

    b.flip();
    REQUIRE( 16 == b.capacity());
    REQUIRE( 16 == b.limit());
    REQUIRE(  0 == b.position());
    REQUIRE( 16 == b.remaining());

    T v=0;
    while( b.hasRemaining() ) {
        // size_t p = b.position();
        T w = b.getPri<T>();
        REQUIRE(v++ == w);
        // std::cout << "[" << std::to_string(p) << "]: " << jau::to_hexstring(w) << "\n";
    }
    REQUIRE( 16 == b.capacity());
    REQUIRE( 16 == b.limit());
    REQUIRE( 16 == b.position());
    REQUIRE(  0 == b.remaining());

    b.rewind();
    REQUIRE( 16 == b.capacity());
    REQUIRE( 16 == b.limit());
    REQUIRE(  0 == b.position());
    REQUIRE( 16 == b.remaining());

    size_t c=0;
    v=0;
    while( b.hasRemaining() ) {
        REQUIRE(   c == b.position());
        REQUIRE(16-c == b.remaining());
        REQUIRE(v++ == b.getPri<T>());
        ++c;
    }
    REQUIRE( 16 == c);
    REQUIRE( 16 == b.capacity());
    REQUIRE( 16 == b.limit());
    REQUIRE( 16 == b.position());
    REQUIRE(  0 == b.remaining());
}
template<typename T>
void test_put4_01() {
    typedef jau::DataBuffer<T> TBuffer;
    typename TBuffer::self_ref b0 = TBuffer::create(4*4);
    test_put4_01_impl<T>(*b0);
}

TEST_CASE( "MemBuffer IO Test 11", "[MemBuffer][io][abstract]" ) {
    test_put4_01<uint8_t>();
    test_put4_01<uint16_t>();
    test_put4_01<int32_t>();
    test_put4_01<float>();
}
