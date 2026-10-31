/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2026 Gothel Software e.K.
 *
 * SPDX-License-Identifier: MIT
 *
 * This Source Code Form is subject to the terms of the MIT License
 * If a copy of the MIT was not distributed with this file,
 * you can obtain one at https://opensource.org/license/mit/.
 */
#include <cassert>
#include <cstring>
#include "jau/debug.hpp"
#include "jau/type_info.hpp"

#include <jau/cpp_lang_util.hpp>
#include <jau/test/catch2_ext.hpp>

#include <jau/int_types.hpp>
#include <jau/int_math.hpp>

using namespace jau;
using namespace jau::int_literals;
using namespace jau::float_literals;

static void print_typeinfo0(int line, const std::string& tname, const jau::type_info &t) {
    jau_printf("%-10s @ %3d, ctti: %u bytes: %s\n", tname, line, sizeof(t), t);
}
namespace copy {
    static void print_typeinfo0(int line, const std::string& tname, const jau::type_info &t) {
        jau_printf("%-10s @ %3d, ctti: %u bytes: %s\n", tname, line, sizeof(t), t);
    }
}

template<typename T>
static void print_typeinfo(int line, const std::string& tname) {
    constexpr jau::type_info t = jau::make_ctti<T>();
    print_typeinfo0(line, tname, t);
} // float64_t @ 76: ctti: TypeInfo[addr 0x7ffd812b29b0, hash 0x9b321e7a07a9504f, '_Float64', ident[]]]

TEST_CASE( "CTTI Base Type Test 00", "[ctti][ctor]" ) {
    {
        jau::type_info t1;
        jau::type_info t2 = t1;
        REQUIRE(t1 == t2);
    }
    {
        constexpr jau::type_info t1;
        constexpr jau::type_info t2 = t1;
        static_assert(t1 == t2);
    }
}

TEST_CASE( "CTTI Func Type Test 01", "[ctti][func]" ) {
    {
        jau::type_info f1 = jau::make_ctti<decltype(print_typeinfo0)>();
        jau::type_info f2 = jau::make_ctti<decltype(copy::print_typeinfo0)>();
        print_typeinfo0(__LINE__, "print_typeinfo0", f1);
        copy::print_typeinfo0(__LINE__, "print_typeinfo0", f2);
    }
}

TEST_CASE( "CTTI Int Type Test 02", "[ctti][int]" ) {
    // int        @  37, ctti: 32 bytes: TypeInfo[addr 0x7ffede347ac0, sig 0x5567d18950b8, hash 0xe624d85315011e0f,
    //
    // 'constexpr const char* jau::ctti_name() [with T = int]' -> int]_jau01', ident[]]]

    print_typeinfo<int>(__LINE__, "int");
    print_typeinfo<long>(__LINE__, "long");
    print_typeinfo<long long>(__LINE__, "long long");

    print_typeinfo<int8_t>(__LINE__, "int8_t");
    print_typeinfo<int16_t>(__LINE__, "int16_t");
    print_typeinfo<int32_t>(__LINE__, "int32_t");
    print_typeinfo<int64_t>(__LINE__, "int64_t");
    print_typeinfo<intptr_t>(__LINE__, "intptr_t");
    print_typeinfo<ssize_t>(__LINE__, "ssize_t");

    print_typeinfo<uint8_t>(__LINE__, "uint8_t");
    print_typeinfo<uint16_t>(__LINE__, "uint16_t");
    print_typeinfo<uint32_t>(__LINE__, "uint32_t");
    print_typeinfo<uint64_t>(__LINE__, "uint64_t");

    {
        const jau::type_info u32_01 = jau::make_ctti<uint32_t>();
        const jau::type_info u32_02 = int_ctti::u32();
        const jau::type_info &u32_03 = jau::static_ctti<uint32_t>();
        print_typeinfo0(__LINE__, "u32-01", u32_01);
        print_typeinfo0(__LINE__, "u32-02", u32_02);
        print_typeinfo0(__LINE__, "u32-03", u32_03);
    }

    [[maybe_unused]] int8_t i8_0 = 1_i8;
    [[maybe_unused]] int16_t i16_0 = 1_i16;
    [[maybe_unused]] int32_t i32_0 = 1_i32;
    [[maybe_unused]] int64_t i64_0 = 1_i64;

    [[maybe_unused]] uint8_t u8_0 = 1_u8;
    [[maybe_unused]] uint16_t u16_0 = 1_u16;
    [[maybe_unused]] uint32_t u32_0 = 1_u32;
    [[maybe_unused]] uint64_t u64_0 = 1_u64;

    static_assert( jau::make_ctti<decltype(i8_0)>() == jau::make_ctti<int8_t>() );
    static_assert( jau::make_ctti<decltype(i8_0)>() == jau::int_ctti::i8() );
    static_assert( jau::make_ctti<decltype(i16_0)>() == jau::int_ctti::i16() );
    static_assert( jau::make_ctti<decltype(i32_0)>() == jau::int_ctti::i32() );
    static_assert( jau::make_ctti<decltype(i64_0)>() == jau::int_ctti::i64() );

    static_assert( jau::make_ctti<decltype(u8_0)>() == jau::make_ctti<uint8_t>() );
    static_assert( jau::make_ctti<decltype(u8_0)>() == jau::int_ctti::u8() );
    static_assert( jau::make_ctti<decltype(u16_0)>() == jau::int_ctti::u16() );
    static_assert( jau::make_ctti<decltype(u32_0)>() == jau::int_ctti::u32() );
    static_assert( jau::make_ctti<decltype(u64_0)>() == jau::int_ctti::u64() );
}

TEST_CASE( "CTTI Float Type Test 03", "[ctti][float]" ) {
    print_typeinfo<float>(__LINE__, "float");
    print_typeinfo<jau::float32_t>(__LINE__, "float32_t");
    print_typeinfo<double>(__LINE__, "double");
    print_typeinfo<jau::float64_t>(__LINE__, "float64_t");

    [[maybe_unused]] float f_0 = 1.1_f;
    [[maybe_unused]] float f_1 = 2_f;
    [[maybe_unused]] double d_0 = 1.1_d;
    [[maybe_unused]] double d_1 = 2_d;

    [[maybe_unused]] jau::float32_t f32_0 = 1.1_f32;
    [[maybe_unused]] jau::float32_t f32_1 = 2_f32;
    [[maybe_unused]] jau::float64_t f64_0 = 1.1_f64;
    [[maybe_unused]] jau::float64_t f64_1 = 2_f64;

    static_assert( jau::make_ctti<decltype(f_0)>() == jau::make_ctti<float>() );
    static_assert( jau::make_ctti<decltype(f_0)>() == jau::float_ctti::f() );
    static_assert( jau::make_ctti<decltype(f_1)>() == jau::float_ctti::f() );
    static_assert( jau::make_ctti<decltype(f32_0)>() == jau::float_ctti::f32() );
    static_assert( jau::make_ctti<decltype(f32_1)>() == jau::float_ctti::f32() );
    static_assert( jau::make_ctti<decltype(d_0)>() == jau::float_ctti::d() );
    static_assert( jau::make_ctti<decltype(d_1)>() == jau::float_ctti::d() );
    static_assert( jau::make_ctti<decltype(f64_0)>() == jau::float_ctti::f64() );
    static_assert( jau::make_ctti<decltype(f64_1)>() == jau::float_ctti::f64() );
}

TEST_CASE("CTTI Benchmark 11", "[ctti][type]") {
    BENCHMARK("CTTI Static Equal   Benchmark") {
        volatile size_t r;
        const jau::type_info &t1a = jau::static_ctti<int32_t>();
        const jau::type_info &t1b = jau::static_ctti<int32_t>();
        r  = t1a.hash_code();
        r += t1b.hash_code();
        REQUIRE( t1a == t1b );
        REQUIRE( t1b == t1a );
        return r;
    };
    BENCHMARK("CTTI Static Unequal Benchmark") {
        volatile size_t r;
        const jau::type_info &t1  = jau::static_ctti<int32_t>();
        const jau::type_info &t2  = jau::static_ctti<int64_t>();
        r  = t1.hash_code();
        r += t2.hash_code();
        REQUIRE( t1 != t2  );
        REQUIRE( t2 != t1  );
        return r;
    };
    BENCHMARK("CTTI Normal Equal   Benchmark") {
        volatile size_t r;
        const jau::type_info t1a = int_ctti::i32();
        const jau::type_info t1b = int_ctti::i32();
        r  = t1a.hash_code();
        r += t1b.hash_code();
        REQUIRE( t1a == t1b );
        REQUIRE( t1b == t1a );
        return r;
    };
    BENCHMARK("CTTI Normal Unequal Benchmark") {
        volatile size_t r;
        const jau::type_info t1  = int_ctti::i32();
        const jau::type_info t2  = int_ctti::i64();
        r  = t1.hash_code();
        r += t2.hash_code();
        REQUIRE( t1 != t2  );
        REQUIRE( t2 != t1  );
        return r;
    };
}

namespace {
    struct SomeVeryLongTypeNameIsBeingUsedHereForType1_addthistagtomakeitevenlonger76543210 {
        int value;
    };
    struct SomeVeryLongTypeNameIsBeingUsedHereForType2_addthistagtomakeitevenlonger76543210 {
        int value;
    };
}

TEST_CASE("CTTI Benchmark 12", "[ctti][type]") {
    BENCHMARK("CTTI Static Equal   Benchmark") {
        volatile size_t r;
        const jau::type_info &t1a = jau::static_ctti<SomeVeryLongTypeNameIsBeingUsedHereForType1_addthistagtomakeitevenlonger76543210>();
        const jau::type_info &t1b = jau::static_ctti<SomeVeryLongTypeNameIsBeingUsedHereForType1_addthistagtomakeitevenlonger76543210>();
        r  = t1a.hash_code();
        r += t1b.hash_code();
        REQUIRE( t1a == t1b );
        REQUIRE( t1b == t1a );
        return r;
    };
    BENCHMARK("CTTI Static Unequal Benchmark") {
        volatile size_t r;
        const jau::type_info &t1  = jau::static_ctti<SomeVeryLongTypeNameIsBeingUsedHereForType1_addthistagtomakeitevenlonger76543210>();
        const jau::type_info &t2  = jau::static_ctti<SomeVeryLongTypeNameIsBeingUsedHereForType2_addthistagtomakeitevenlonger76543210>();
        r  = t1.hash_code();
        r += t2.hash_code();
        REQUIRE( t1 != t2  );
        REQUIRE( t2 != t1  );
        return r;
    };
    BENCHMARK("CTTI Normal Equal   Benchmark") {
        volatile size_t r;
        const jau::type_info t1a = jau::make_ctti<SomeVeryLongTypeNameIsBeingUsedHereForType1_addthistagtomakeitevenlonger76543210>();
        const jau::type_info t1b = jau::make_ctti<SomeVeryLongTypeNameIsBeingUsedHereForType1_addthistagtomakeitevenlonger76543210>();
        r  = t1a.hash_code();
        r += t1b.hash_code();
        REQUIRE( t1a == t1b );
        REQUIRE( t1b == t1a );
        return r;
    };
    BENCHMARK("CTTI Normal Unequal Benchmark") {
        volatile size_t r;
        const jau::type_info t1 = jau::make_ctti<SomeVeryLongTypeNameIsBeingUsedHereForType1_addthistagtomakeitevenlonger76543210>();
        const jau::type_info t2 = jau::make_ctti<SomeVeryLongTypeNameIsBeingUsedHereForType2_addthistagtomakeitevenlonger76543210>();
        r  = t1.hash_code();
        r += t2.hash_code();
        REQUIRE( t1 != t2  );
        REQUIRE( t2 != t1  );
        return r;
    };
}
