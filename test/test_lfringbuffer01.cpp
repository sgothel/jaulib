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

typedef uint8_t Integral_type;
typedef uint8_t Value_type;

template<>
Value_type getDefault() { return (Value_type)0xff; }

template<>
Value_type createValue(const Integral_type& v) { return v; }

template<>
Integral_type getValue(const Value_type& e) { return e; }

typedef TestRingbuffer_A<Integral_type, Value_type, jau::nsize_t,
        true /* exp_memmove */, true /* exp_memcpy */, false /* exp_secmem */> TestRingbuffer01a;

typedef TestRingbuffer_A<Integral_type, Value_type, jau::nsize_t,
        true /* exp_memmove */, true /* exp_memcpy */, true /* exp_secmem */,
        true /* use_memmove */, true /* use_memcpy */, true /* use_secmem */> TestRingbuffer01b;

typedef TestRingbuffer_A<Integral_type, Value_type, jau::nsize_t,
        false /* exp_memmove */, false /* exp_memcpy */, true /* exp_secmem */,
        false /* use_memmove */, false /* use_memcpy */, true /* use_secmem */> TestRingbuffer01c;

#if 1
METHOD_AS_TEST_CASE( TestRingbuffer01a::test00_PrintInfo,         "Test TestRingbuffer 01a- 00");
METHOD_AS_TEST_CASE( TestRingbuffer01a::test01_FullRead,          "Test TestRingbuffer 01a- 01");
METHOD_AS_TEST_CASE( TestRingbuffer01a::test02_EmptyWrite,        "Test TestRingbuffer 01a- 02");
METHOD_AS_TEST_CASE( TestRingbuffer01a::test03_EmptyWriteRange,   "Test TestRingbuffer 01a- 03");
METHOD_AS_TEST_CASE( TestRingbuffer01a::test04_FullReadReset,     "Test TestRingbuffer 01a- 04");
METHOD_AS_TEST_CASE( TestRingbuffer01a::test05_EmptyWriteClear,   "Test TestRingbuffer 01a- 05");
METHOD_AS_TEST_CASE( TestRingbuffer01a::test06_ReadResetMid01,    "Test TestRingbuffer 01a- 06");
METHOD_AS_TEST_CASE( TestRingbuffer01a::test07_ReadResetMid02,    "Test TestRingbuffer 01a- 07");
METHOD_AS_TEST_CASE( TestRingbuffer01a::test20_GrowFull01_Begin,  "Test TestRingbuffer 01a- 20");
METHOD_AS_TEST_CASE( TestRingbuffer01a::test21_GrowFull02_Begin1, "Test TestRingbuffer 01a- 21");
METHOD_AS_TEST_CASE( TestRingbuffer01a::test22_GrowFull03_Begin2, "Test TestRingbuffer 01a- 22");
METHOD_AS_TEST_CASE( TestRingbuffer01a::test23_GrowFull04_Begin3, "Test TestRingbuffer 01a- 23");
METHOD_AS_TEST_CASE( TestRingbuffer01a::test24_GrowFull05_End,    "Test TestRingbuffer 01a- 24");
METHOD_AS_TEST_CASE( TestRingbuffer01a::test25_GrowFull11_End1,   "Test TestRingbuffer 01a- 25");
METHOD_AS_TEST_CASE( TestRingbuffer01a::test26_GrowFull12_End2,   "Test TestRingbuffer 01a- 26");
METHOD_AS_TEST_CASE( TestRingbuffer01a::test27_GrowFull13_End3,   "Test TestRingbuffer 01a- 27");
#else
METHOD_AS_TEST_CASE( TestRingbuffer01a::test00_PrintInfo,         "Test TestRingbuffer 01a- 00");
METHOD_AS_TEST_CASE( TestRingbuffer01a::test03_EmptyWriteRange,   "Test TestRingbuffer 01a- 03");
#endif

METHOD_AS_TEST_CASE( TestRingbuffer01b::test00_PrintInfo,         "Test TestRingbuffer 01b- 00");
METHOD_AS_TEST_CASE( TestRingbuffer01b::test01_FullRead,          "Test TestRingbuffer 01b- 01");
METHOD_AS_TEST_CASE( TestRingbuffer01b::test02_EmptyWrite,        "Test TestRingbuffer 01b- 02");
METHOD_AS_TEST_CASE( TestRingbuffer01b::test03_EmptyWriteRange,   "Test TestRingbuffer 01b- 03");
METHOD_AS_TEST_CASE( TestRingbuffer01b::test04_FullReadReset,     "Test TestRingbuffer 01b- 04");
METHOD_AS_TEST_CASE( TestRingbuffer01b::test05_EmptyWriteClear,   "Test TestRingbuffer 01b- 05");
METHOD_AS_TEST_CASE( TestRingbuffer01b::test06_ReadResetMid01,    "Test TestRingbuffer 01b- 06");
METHOD_AS_TEST_CASE( TestRingbuffer01b::test07_ReadResetMid02,    "Test TestRingbuffer 01b- 07");
METHOD_AS_TEST_CASE( TestRingbuffer01b::test20_GrowFull01_Begin,  "Test TestRingbuffer 01b- 20");
METHOD_AS_TEST_CASE( TestRingbuffer01b::test21_GrowFull02_Begin1, "Test TestRingbuffer 01b- 21");
METHOD_AS_TEST_CASE( TestRingbuffer01b::test22_GrowFull03_Begin2, "Test TestRingbuffer 01b- 22");
METHOD_AS_TEST_CASE( TestRingbuffer01b::test23_GrowFull04_Begin3, "Test TestRingbuffer 01b- 23");
METHOD_AS_TEST_CASE( TestRingbuffer01b::test24_GrowFull05_End,    "Test TestRingbuffer 01b- 24");
METHOD_AS_TEST_CASE( TestRingbuffer01b::test25_GrowFull11_End1,   "Test TestRingbuffer 01b- 25");
METHOD_AS_TEST_CASE( TestRingbuffer01b::test26_GrowFull12_End2,   "Test TestRingbuffer 01b- 26");
METHOD_AS_TEST_CASE( TestRingbuffer01b::test27_GrowFull13_End3,   "Test TestRingbuffer 01b- 27");

METHOD_AS_TEST_CASE( TestRingbuffer01c::test00_PrintInfo,         "Test TestRingbuffer 01c- 00");
METHOD_AS_TEST_CASE( TestRingbuffer01c::test01_FullRead,          "Test TestRingbuffer 01c- 01");
METHOD_AS_TEST_CASE( TestRingbuffer01c::test02_EmptyWrite,        "Test TestRingbuffer 01c- 02");
METHOD_AS_TEST_CASE( TestRingbuffer01c::test03_EmptyWriteRange,   "Test TestRingbuffer 01c- 03");
METHOD_AS_TEST_CASE( TestRingbuffer01c::test04_FullReadReset,     "Test TestRingbuffer 01c- 04");
METHOD_AS_TEST_CASE( TestRingbuffer01c::test05_EmptyWriteClear,   "Test TestRingbuffer 01c- 05");
METHOD_AS_TEST_CASE( TestRingbuffer01c::test06_ReadResetMid01,    "Test TestRingbuffer 01c- 06");
METHOD_AS_TEST_CASE( TestRingbuffer01c::test07_ReadResetMid02,    "Test TestRingbuffer 01c- 07");
METHOD_AS_TEST_CASE( TestRingbuffer01c::test20_GrowFull01_Begin,  "Test TestRingbuffer 01c- 20");
METHOD_AS_TEST_CASE( TestRingbuffer01c::test21_GrowFull02_Begin1, "Test TestRingbuffer 01c- 21");
METHOD_AS_TEST_CASE( TestRingbuffer01c::test22_GrowFull03_Begin2, "Test TestRingbuffer 01c- 22");
METHOD_AS_TEST_CASE( TestRingbuffer01c::test23_GrowFull04_Begin3, "Test TestRingbuffer 01c- 23");
METHOD_AS_TEST_CASE( TestRingbuffer01c::test24_GrowFull05_End,    "Test TestRingbuffer 01c- 24");
METHOD_AS_TEST_CASE( TestRingbuffer01c::test25_GrowFull11_End1,   "Test TestRingbuffer 01c- 25");
METHOD_AS_TEST_CASE( TestRingbuffer01c::test26_GrowFull12_End2,   "Test TestRingbuffer 01c- 26");
METHOD_AS_TEST_CASE( TestRingbuffer01c::test27_GrowFull13_End3,   "Test TestRingbuffer 01c- 27");
