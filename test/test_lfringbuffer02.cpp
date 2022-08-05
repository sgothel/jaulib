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

class Integer {
    public:
        Integral_type value;

        Integer(Integral_type v) : value(v) {}

        Integer() noexcept : value(-1) { }

        Integer(const Integer &o) noexcept = default;
        Integer(Integer &&o) noexcept = default;
        Integer& operator=(const Integer &o) noexcept = default;
        Integer& operator=(Integer &&o) noexcept = default;

        operator Integral_type() const {
            return value;
        }
        constexpr Integral_type getValue() const { return value; }
        static Integer valueOf(const Integral_type i) { return Integer(i); }
};
typedef Integer Value_type;

template<>
Value_type getDefault() { return Integer(); }

template<>
Value_type createValue(const Integral_type& v) { return Integer(v); }

template<>
Integral_type getValue(const Value_type& e) { return e.getValue(); }

TEST_CASE( "TestRingbuffer_A_02_a<Integral_type=jau::snsize_t, Value_type=Integer, Size_type=jau::nsize_t, exp_memmove=true, exp_memcpy=true, exp_secmem=false>", "[ringbuffer]" ) {
    PerformRingbufferTests<Integral_type, Value_type, jau::nsize_t,
            true /* exp_memmove */, true /* exp_memcpy */, false /* exp_secmem */>();
}

TEST_CASE( "TestRingbuffer_A_02_b<Integral_type=jau::snsize_t, Value_type=Integer, Size_type=jau::nsize_t, exp_memmove=true, exp_memcpy=true, exp_secmem=true>", "[ringbuffer]" ) {
    PerformRingbufferTests<Integral_type, Value_type, jau::nsize_t,
            true /* exp_memmove */, true /* exp_memcpy */, true /* exp_secmem */,
            true /* use_memmove */, true /* use_memcpy */, true /* use_secmem */>();
}

TEST_CASE( "TestRingbuffer_A_02_c<Integral_type=jau::snsize_t, Value_type=Integer, Size_type=jau::nsize_t, exp_memmove=false, exp_memcpy=false, exp_secmem=true>", "[ringbuffer]" ) {
    PerformRingbufferTests<Integral_type, Value_type, jau::nsize_t,
            false /* exp_memmove */, false /* exp_memcpy */, true /* exp_secmem */,
            false /* use_memmove */, false /* use_memcpy */, true /* use_secmem */>();
}

