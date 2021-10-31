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

#define CATCH_CONFIG_MAIN
#include <catch2/catch_amalgamated.hpp>
#include <jau/test/catch2_ext.hpp>

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
        Integral_type getValue() const { return value; }
        static Integer valueOf(const Integral_type i) { return Integer(i); }
};
typedef std::shared_ptr<Integer> Value_type;

template<>
Value_type getDefault() { return std::make_shared<Integer>(); }

template<>
Value_type createValue(const Integral_type& v) { return std::make_shared<Integer>(v); }

template<>
Integral_type getValue(const Value_type& e) { return e->getValue(); }

typedef TestRingbuffer_A<Integral_type, Value_type, jau::nsize_t,
        false /* exp_memmove */, false /* exp_memcpy */, false /* exp_secmem */> TestRingbuffer03a;

typedef TestRingbuffer_A<Integral_type, Value_type, jau::nsize_t,
        false /* exp_memmove */, false /* exp_memcpy */, true /* exp_secmem */,
        false /* use_memmove */, false /* use_memcpy */, true /* use_secmem */> TestRingbuffer03b;


METHOD_AS_TEST_CASE( TestRingbuffer03a::test00_PrintInfo,         "Test TestRingbuffer 03a- 00");
METHOD_AS_TEST_CASE( TestRingbuffer03a::test01_FullRead,          "Test TestRingbuffer 03a- 01");
METHOD_AS_TEST_CASE( TestRingbuffer03a::test02_EmptyWrite,        "Test TestRingbuffer 03a- 02");
METHOD_AS_TEST_CASE( TestRingbuffer03a::test03_EmptyWriteRange,   "Test TestRingbuffer 03a- 03");
METHOD_AS_TEST_CASE( TestRingbuffer03a::test04_FullReadReset,     "Test TestRingbuffer 03a- 04");
METHOD_AS_TEST_CASE( TestRingbuffer03a::test05_EmptyWriteClear,   "Test TestRingbuffer 03a- 05");
METHOD_AS_TEST_CASE( TestRingbuffer03a::test06_ReadResetMid01,    "Test TestRingbuffer 03a- 06");
METHOD_AS_TEST_CASE( TestRingbuffer03a::test07_ReadResetMid02,    "Test TestRingbuffer 03a- 07");
METHOD_AS_TEST_CASE( TestRingbuffer03a::test20_GrowFull01_Begin,  "Test TestRingbuffer 03a- 20");
METHOD_AS_TEST_CASE( TestRingbuffer03a::test21_GrowFull02_Begin1, "Test TestRingbuffer 03a- 21");
METHOD_AS_TEST_CASE( TestRingbuffer03a::test22_GrowFull03_Begin2, "Test TestRingbuffer 03a- 22");
METHOD_AS_TEST_CASE( TestRingbuffer03a::test23_GrowFull04_Begin3, "Test TestRingbuffer 03a- 23");
METHOD_AS_TEST_CASE( TestRingbuffer03a::test24_GrowFull05_End,    "Test TestRingbuffer 03a- 24");
METHOD_AS_TEST_CASE( TestRingbuffer03a::test25_GrowFull11_End1,   "Test TestRingbuffer 03a- 25");
METHOD_AS_TEST_CASE( TestRingbuffer03a::test26_GrowFull12_End2,   "Test TestRingbuffer 03a- 26");
METHOD_AS_TEST_CASE( TestRingbuffer03a::test27_GrowFull13_End3,   "Test TestRingbuffer 03a- 27");

METHOD_AS_TEST_CASE( TestRingbuffer03b::test00_PrintInfo,         "Test TestRingbuffer 03b- 00");
METHOD_AS_TEST_CASE( TestRingbuffer03b::test01_FullRead,          "Test TestRingbuffer 03b- 01");
METHOD_AS_TEST_CASE( TestRingbuffer03b::test02_EmptyWrite,        "Test TestRingbuffer 03b- 02");
METHOD_AS_TEST_CASE( TestRingbuffer03b::test03_EmptyWriteRange,   "Test TestRingbuffer 03b- 03");
METHOD_AS_TEST_CASE( TestRingbuffer03b::test04_FullReadReset,     "Test TestRingbuffer 03b- 04");
METHOD_AS_TEST_CASE( TestRingbuffer03b::test05_EmptyWriteClear,   "Test TestRingbuffer 03b- 05");
METHOD_AS_TEST_CASE( TestRingbuffer03b::test06_ReadResetMid01,    "Test TestRingbuffer 03b- 06");
METHOD_AS_TEST_CASE( TestRingbuffer03b::test07_ReadResetMid02,    "Test TestRingbuffer 03b- 07");
METHOD_AS_TEST_CASE( TestRingbuffer03b::test20_GrowFull01_Begin,  "Test TestRingbuffer 03b- 20");
METHOD_AS_TEST_CASE( TestRingbuffer03b::test21_GrowFull02_Begin1, "Test TestRingbuffer 03b- 21");
METHOD_AS_TEST_CASE( TestRingbuffer03b::test22_GrowFull03_Begin2, "Test TestRingbuffer 03b- 22");
METHOD_AS_TEST_CASE( TestRingbuffer03b::test23_GrowFull04_Begin3, "Test TestRingbuffer 03b- 23");
METHOD_AS_TEST_CASE( TestRingbuffer03b::test24_GrowFull05_End,    "Test TestRingbuffer 03b- 24");
METHOD_AS_TEST_CASE( TestRingbuffer03b::test25_GrowFull11_End1,   "Test TestRingbuffer 03b- 25");
METHOD_AS_TEST_CASE( TestRingbuffer03b::test26_GrowFull12_End2,   "Test TestRingbuffer 03b- 26");
METHOD_AS_TEST_CASE( TestRingbuffer03b::test27_GrowFull13_End3,   "Test TestRingbuffer 03b- 27");
