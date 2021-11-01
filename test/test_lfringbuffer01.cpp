/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020 Gothel Software e.K.
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
#include <iostream>
#include <cassert>
#include <cinttypes>
#include <cstring>
#include <memory>

#include <jau/ringbuffer.hpp>

#include "test_lfringbuffer_a.hpp"

using namespace jau;

typedef uint8_t Integral_type;
typedef uint8_t Value_type;

template<>
Value_type getDefault() { return (Value_type)0xff; }

template<>
Value_type createValue(const Integral_type& v) { return v; }

template<>
Integral_type getValue(const Value_type& e) { return e; }

TEST_CASE( "TestRingbuffer_A_01_a<Integral_type=uint8_t, Value_type=uint8_t, Size_type=jau::nsize_t, exp_memmove=true, exp_memcpy=true, exp_secmem=false>", "[ringbuffer_A_01a]" ) {
    PerformRingbufferTests<Integral_type, Value_type, jau::nsize_t,
            true /* exp_memmove */, true /* exp_memcpy */, false /* exp_secmem */>();
}

TEST_CASE( "TestRingbuffer_A_01_b<Integral_type=uint8_t, Value_type=uint8_t, Size_type=jau::nsize_t, exp_memmove=true, exp_memcpy=true, exp_secmem=true>", "[ringbuffer_A_01b]" ) {
    PerformRingbufferTests<Integral_type, Value_type, jau::nsize_t,
            true /* exp_memmove */, true /* exp_memcpy */, true /* exp_secmem */,
            true /* use_memmove */, true /* use_memcpy */, true /* use_secmem */>();
}

TEST_CASE( "TestRingbuffer_A_01_c<Integral_type=uint8_t, Value_type=uint8_t, Size_type=jau::nsize_t, exp_memmove=false, exp_memcpy=false, exp_secmem=true>", "[ringbuffer_A_01c]" ) {
    PerformRingbufferTests<Integral_type, Value_type, jau::nsize_t,
            false /* exp_memmove */, false /* exp_memcpy */, true /* exp_secmem */,
            false /* use_memmove */, false /* use_memcpy */, true /* use_secmem */>();
}

