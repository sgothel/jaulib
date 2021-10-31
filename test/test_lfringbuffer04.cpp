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

typedef std::array<Integral_type, 6> Value_type;

template<>
Value_type getDefault() { return Value_type{-1, -1, -1, -1, -1, -1}; }

template<>
Value_type createValue(const Integral_type& v) { return Value_type{v, v+1, v+2, v+3, v+4, v+5}; }

template<>
Integral_type getValue(const Value_type& e) { return e[0]; }

typedef TestRingbuffer_A<Integral_type, Value_type, jau::nsize_t,
        true /* exp_memmove */, true /* exp_memcpy */, false /* exp_secmem */> TestRingbuffer02a;

typedef TestRingbuffer_A<Integral_type, Value_type, jau::nsize_t,
        true /* exp_memmove */, true /* exp_memcpy */, true /* exp_secmem */,
        true /* use_memmove */, true /* use_memcpy */, true /* use_secmem */> TestRingbuffer02b;

typedef TestRingbuffer_A<Integral_type, Value_type, jau::nsize_t,
        false /* exp_memmove */, false /* exp_memcpy */, true /* exp_secmem */,
        false /* use_memmove */, false /* use_memcpy */, true /* use_secmem */> TestRingbuffer02c;

#if 1
METHOD_AS_TEST_CASE( TestRingbuffer02a::test00_PrintInfo,         "Test TestRingbuffer 02a- 00");
METHOD_AS_TEST_CASE( TestRingbuffer02a::test01_FullRead,          "Test TestRingbuffer 02a- 01");
METHOD_AS_TEST_CASE( TestRingbuffer02a::test02_EmptyWrite,        "Test TestRingbuffer 02a- 02");
METHOD_AS_TEST_CASE( TestRingbuffer02a::test03_EmptyWriteRange,   "Test TestRingbuffer 02a- 03");
METHOD_AS_TEST_CASE( TestRingbuffer02a::test04_FullReadReset,     "Test TestRingbuffer 02a- 04");
METHOD_AS_TEST_CASE( TestRingbuffer02a::test05_EmptyWriteClear,   "Test TestRingbuffer 02a- 05");
METHOD_AS_TEST_CASE( TestRingbuffer02a::test06_ReadResetMid01,    "Test TestRingbuffer 02a- 06");
METHOD_AS_TEST_CASE( TestRingbuffer02a::test07_ReadResetMid02,    "Test TestRingbuffer 02a- 07");
METHOD_AS_TEST_CASE( TestRingbuffer02a::test20_GrowFull01_Begin,  "Test TestRingbuffer 02a- 20");
METHOD_AS_TEST_CASE( TestRingbuffer02a::test21_GrowFull02_Begin1, "Test TestRingbuffer 02a- 21");
METHOD_AS_TEST_CASE( TestRingbuffer02a::test22_GrowFull03_Begin2, "Test TestRingbuffer 02a- 22");
METHOD_AS_TEST_CASE( TestRingbuffer02a::test23_GrowFull04_Begin3, "Test TestRingbuffer 02a- 23");
METHOD_AS_TEST_CASE( TestRingbuffer02a::test24_GrowFull05_End,    "Test TestRingbuffer 02a- 24");
METHOD_AS_TEST_CASE( TestRingbuffer02a::test25_GrowFull11_End1,   "Test TestRingbuffer 02a- 25");
METHOD_AS_TEST_CASE( TestRingbuffer02a::test26_GrowFull12_End2,   "Test TestRingbuffer 02a- 26");
METHOD_AS_TEST_CASE( TestRingbuffer02a::test27_GrowFull13_End3,   "Test TestRingbuffer 02a- 27");
#else
METHOD_AS_TEST_CASE( TestRingbuffer02a::test00_PrintInfo,         "Test TestRingbuffer 02a- 00");
METHOD_AS_TEST_CASE( TestRingbuffer02a::test03_EmptyWriteRange,   "Test TestRingbuffer 02a- 03");
#endif

METHOD_AS_TEST_CASE( TestRingbuffer02b::test00_PrintInfo,         "Test TestRingbuffer 02b- 00");
METHOD_AS_TEST_CASE( TestRingbuffer02b::test01_FullRead,          "Test TestRingbuffer 02b- 01");
METHOD_AS_TEST_CASE( TestRingbuffer02b::test02_EmptyWrite,        "Test TestRingbuffer 02b- 02");
METHOD_AS_TEST_CASE( TestRingbuffer02b::test03_EmptyWriteRange,   "Test TestRingbuffer 02b- 03");
METHOD_AS_TEST_CASE( TestRingbuffer02b::test04_FullReadReset,     "Test TestRingbuffer 02b- 04");
METHOD_AS_TEST_CASE( TestRingbuffer02b::test05_EmptyWriteClear,   "Test TestRingbuffer 02b- 05");
METHOD_AS_TEST_CASE( TestRingbuffer02b::test06_ReadResetMid01,    "Test TestRingbuffer 02b- 06");
METHOD_AS_TEST_CASE( TestRingbuffer02b::test07_ReadResetMid02,    "Test TestRingbuffer 02b- 07");
METHOD_AS_TEST_CASE( TestRingbuffer02b::test20_GrowFull01_Begin,  "Test TestRingbuffer 02b- 20");
METHOD_AS_TEST_CASE( TestRingbuffer02b::test21_GrowFull02_Begin1, "Test TestRingbuffer 02b- 21");
METHOD_AS_TEST_CASE( TestRingbuffer02b::test22_GrowFull03_Begin2, "Test TestRingbuffer 02b- 22");
METHOD_AS_TEST_CASE( TestRingbuffer02b::test23_GrowFull04_Begin3, "Test TestRingbuffer 02b- 23");
METHOD_AS_TEST_CASE( TestRingbuffer02b::test24_GrowFull05_End,    "Test TestRingbuffer 02b- 24");
METHOD_AS_TEST_CASE( TestRingbuffer02b::test25_GrowFull11_End1,   "Test TestRingbuffer 02b- 25");
METHOD_AS_TEST_CASE( TestRingbuffer02b::test26_GrowFull12_End2,   "Test TestRingbuffer 02b- 26");
METHOD_AS_TEST_CASE( TestRingbuffer02b::test27_GrowFull13_End3,   "Test TestRingbuffer 02b- 27");

METHOD_AS_TEST_CASE( TestRingbuffer02c::test00_PrintInfo,         "Test TestRingbuffer 02c- 00");
METHOD_AS_TEST_CASE( TestRingbuffer02c::test01_FullRead,          "Test TestRingbuffer 02c- 01");
METHOD_AS_TEST_CASE( TestRingbuffer02c::test02_EmptyWrite,        "Test TestRingbuffer 02c- 02");
METHOD_AS_TEST_CASE( TestRingbuffer02c::test03_EmptyWriteRange,   "Test TestRingbuffer 02c- 03");
METHOD_AS_TEST_CASE( TestRingbuffer02c::test04_FullReadReset,     "Test TestRingbuffer 02c- 04");
METHOD_AS_TEST_CASE( TestRingbuffer02c::test05_EmptyWriteClear,   "Test TestRingbuffer 02c- 05");
METHOD_AS_TEST_CASE( TestRingbuffer02c::test06_ReadResetMid01,    "Test TestRingbuffer 02c- 06");
METHOD_AS_TEST_CASE( TestRingbuffer02c::test07_ReadResetMid02,    "Test TestRingbuffer 02c- 07");
METHOD_AS_TEST_CASE( TestRingbuffer02c::test20_GrowFull01_Begin,  "Test TestRingbuffer 02c- 20");
METHOD_AS_TEST_CASE( TestRingbuffer02c::test21_GrowFull02_Begin1, "Test TestRingbuffer 02c- 21");
METHOD_AS_TEST_CASE( TestRingbuffer02c::test22_GrowFull03_Begin2, "Test TestRingbuffer 02c- 22");
METHOD_AS_TEST_CASE( TestRingbuffer02c::test23_GrowFull04_Begin3, "Test TestRingbuffer 02c- 23");
METHOD_AS_TEST_CASE( TestRingbuffer02c::test24_GrowFull05_End,    "Test TestRingbuffer 02c- 24");
METHOD_AS_TEST_CASE( TestRingbuffer02c::test25_GrowFull11_End1,   "Test TestRingbuffer 02c- 25");
METHOD_AS_TEST_CASE( TestRingbuffer02c::test26_GrowFull12_End2,   "Test TestRingbuffer 02c- 26");
METHOD_AS_TEST_CASE( TestRingbuffer02c::test27_GrowFull13_End3,   "Test TestRingbuffer 02c- 27");
