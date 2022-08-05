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
#include <cassert>
#include <cinttypes>
#include <cstring>
#include <memory>

#include <jau/ringbuffer.hpp>

#include "test_lfringbuffer_a.hpp"

using namespace jau;

typedef jau::snsize_t Integral_type;

typedef std::array<Integral_type, 6> Value_type;

template<>
Value_type getDefault() { return Value_type{-1, -1, -1, -1, -1, -1}; }

template<>
Value_type createValue(const Integral_type& v) { return Value_type{v, v+1, v+2, v+3, v+4, v+5}; }

template<>
Integral_type getValue(const Value_type& e) { return e[0]; }

TEST_CASE( "TestRingbuffer_A_04_a<Integral_type=jau::snsize_t, Value_type=array<jau::snsize_t, 6>, Size_type=jau::nsize_t, exp_memmove=true, exp_memcpy=true, exp_secmem=false>", "[ringbuffer]" ) {
    PerformRingbufferTests<Integral_type, Value_type, jau::nsize_t,
            true /* exp_memmove */, true /* exp_memcpy */, false /* exp_secmem */>();
}

TEST_CASE( "TestRingbuffer_A_04_b<Integral_type=jau::snsize_t, Value_type=array<jau::snsize_t, 6>, Size_type=jau::nsize_t, exp_memmove=true, exp_memcpy=true, exp_secmem=true>", "[ringbuffer]" ) {
    PerformRingbufferTests<Integral_type, Value_type, jau::nsize_t,
            true /* exp_memmove */, true /* exp_memcpy */, true /* exp_secmem */,
            true /* use_memmove */, true /* use_memcpy */, true /* use_secmem */>();
}

TEST_CASE( "TestRingbuffer_A_04_c<Integral_type=jau::snsize_t, Value_type=array<jau::snsize_t, 6>, Size_type=jau::nsize_t, exp_memmove=false, exp_memcpy=false, exp_secmem=true>", "[ringbuffer]" ) {
    PerformRingbufferTests<Integral_type, Value_type, jau::nsize_t,
            false /* exp_memmove */, false /* exp_memcpy */, true /* exp_secmem */,
            false /* use_memmove */, false /* use_memcpy */, true /* use_secmem */>();
}


