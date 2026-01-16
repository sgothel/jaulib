/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2024 Gothel Software e.K.
 *
 * ***
 *
 * SPDX-License-Identifier: MIT
 *
 * This Source Code Form is subject to the terms of the MIT License
 * If a copy of the MIT was not distributed with this
 * file, You can obtain one at https://opensource.org/license/mit/.
 *
 */
#include <sys/types.h>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>

#include <jau/basic_types.hpp>
#include <jau/cpp_lang_util.hpp>
#include <jau/cpp_pragma.hpp>
#include <jau/float_types.hpp>
#include <jau/int_types.hpp>
#include <jau/string_cfmt.hpp>
#include <jau/string_literal.hpp>
#include <jau/string_util.hpp>
#include <jau/test/catch2_ext.hpp>
#include <jau/type_concepts.hpp>
#include <jau/type_traits_queries.hpp>

#ifdef HAS_STD_FORMAT
    #include <format>
#endif

using namespace std::literals;

using namespace jau::float_literals;

using namespace jau::int_literals;

//
// Explore memory footprint of using jau::cfmt, this unit uses jau::cfmt
//

template <typename... Args>
static void printFormat(int line, const char *fmt, const Args &...args) {
    // std::string exp = jau::unsafe::format_string(fmt, args...);
    // std::string has = jau::format_string(fmt, args...);
    std::string has;
    jau::cfmt::Result r = jau::cfmt::formatR(has, fmt, args...);
    std::cerr << "FormatResult @ " << line << ": " << r << "\n";
    std::cerr << "FormatResult @ " << line << ": has `" << has << "`\n\n";
    CHECK( true == r.success());
    CHECK( sizeof...(args) == r.argumentCount());
}

TEST_CASE("format: std::cfmt footprint", "[jau][std::string][jau::cfmt][footprint]") {
    // type conversion
    int32_t  i32 = -1234;
    int32_t  i32_u = 1234;
    uint32_t u32 =  1234;
    float    f32 = 123.45f; // 42.14f;
    double   f64 = 123.45; // 42.1456;
    void *p1a = (void *)0xaabbccdd_u64; // NOLINT
    void *p1b = (void *)0x11223344aabbccdd_u64; // NOLINT
    void *p2a = (void *)0x112233aabbccdd_u64; // NOLINT
    void *p2b = (void *)0xaabbcc_u64; // NOLINT
    void *p3a = (void *)0x112233aabbccdd_u64; // NOLINT
    void *p3b = (void *)0xaabbcc_u64; // NOLINT

    printFormat(__LINE__, "%%");

    printFormat(__LINE__, "%c", 'Z');
    printFormat(__LINE__, "%s", "Hello World");
    printFormat(__LINE__, "%p", &i32);
    printFormat(__LINE__, "p1a %p %0p", p1a, p1a);
    printFormat(__LINE__, "p1b %p %0p", p1b, p1b);
    printFormat(__LINE__, "p2a %p %0p", p2a, p2a);
    printFormat(__LINE__, "p2b %p %0p", p2b, p2b);
    printFormat(__LINE__, "p3a %p %0p", p3a, p3a);
    printFormat(__LINE__, "p3b %p %0p", p3b, p3b);

    printFormat(__LINE__, "%d", i32);

    printFormat(__LINE__, "%o", u32);
    printFormat(__LINE__, "%x", u32);
    printFormat(__LINE__, "%X", u32);
    printFormat(__LINE__, "%u", u32);
    printFormat(__LINE__, "%o", i32_u);
    printFormat(__LINE__, "%x", i32_u);
    printFormat(__LINE__, "%X", i32_u);
    printFormat(__LINE__, "%u", i32_u);

    printFormat(__LINE__, "%f", f64);
    printFormat(__LINE__, "%e", f64);
    printFormat(__LINE__, "%E", f64);
    printFormat(__LINE__, "%a", f64);
    printFormat(__LINE__, "%A", f64);
    // checkFormat(__LINE__, "%g", f64);
    // checkFormat(__LINE__, "%G", f64);

    printFormat(__LINE__, "%f", f32);
    printFormat(__LINE__, "%e", f32);
    printFormat(__LINE__, "%E", f32);
    printFormat(__LINE__, "%a", f32);
    printFormat(__LINE__, "%A", f32);
    // checkFormat(__LINE__, "%g", f32);
    // checkFormat(__LINE__, "%G", f32);

    printFormat(__LINE__, "%dZZZ", i32);
    printFormat(__LINE__, "%dZZ", i32);
    printFormat(__LINE__, "%dZ", i32);
    printFormat(__LINE__, "Z%dZ Z%dZ", i32, i32);
    printFormat(__LINE__, "Z%-6dZ Z%6dZ", i32, i32);

    printFormat(__LINE__, "%#020x", 305441741);
    printFormat(__LINE__, "%zd", 2147483647L);

    printFormat(__LINE__, "%zu", 2147483647UL);

    printFormat(__LINE__, "%s", "Test");
    {
        const char *str = nullptr;
        size_t str_len = 2;
        const char limiter = '3';
        const char *limiter_pos = nullptr;
        char *endptr = nullptr;

        printFormat(__LINE__, "Value end not '%c' @ idx %zd, %p != %p, in: %p '%s' len %zu", limiter, endptr - str, endptr, limiter_pos, str, str, str_len);
    }

    // enums
    {
        enum enum1_unsigned_t { jau1_alpha, jau1_beta, jau1_gamma }; ///< unsigned
        enum1_unsigned_t e1_u = jau1_alpha;

        enum enum2_signed_t { jau2_alpha=-1, jau_beta, jau_gamma }; ///< signed
        enum2_signed_t e2_s = jau2_alpha;

        enum class enum3_signed_t { alpha=-1, beta, gamma }; ///< signed
        enum3_signed_t e3_s = enum3_signed_t::alpha;

        typedef enum { ///< unsigned
            jau_CAP_CLEAR=0,
            jau_CAP_SET=1
        } enum4_unsigned_t;
        enum4_unsigned_t e4_u = jau_CAP_CLEAR;

        printFormat(__LINE__, "Enum %u, %d, %d, %u\n", e1_u, e2_s, e3_s, e4_u);
    }

}
