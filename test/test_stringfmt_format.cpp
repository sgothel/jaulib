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
#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string_view>

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

TEST_CASE("parse: width precision from format", "[jau][std::string][jau::cfmt]") {
    using namespace jau::cfmt;

    //
    // Single feature: Width / Precision
    //
    {
        std::string s;
        jau::cfmt::Result r = jau::cfmt::formatR(s,"%" PRIi64, (int64_t)1);
        std::cerr << "FormatResult " << r << "\n";
        REQUIRE( true == r.success());
        REQUIRE( 1 == r.argumentCount());
        REQUIRE( flags_t::none == r.opts().flags);
        REQUIRE( false == r.opts().width_set);
        REQUIRE( 0 == r.opts().width);
        REQUIRE( false == r.opts().precision_set);
        REQUIRE( 0 == r.opts().precision);
        REQUIRE( jau::cfmt::plength_t::l == r.opts().length_mod);
    }
    {
        std::string s;
        jau::cfmt::Result r = jau::cfmt::formatR(s,"%23" PRIi64, (int64_t)1);
        std::cerr << "FormatResult " << r << "\n";
        REQUIRE( true == r.success());
        REQUIRE( 1 == r.argumentCount());
        REQUIRE( flags_t::none == r.opts().flags);
        REQUIRE( true == r.opts().width_set);
        REQUIRE( 23 == r.opts().width);
        REQUIRE( false == r.opts().precision_set);
        REQUIRE( 0 == r.opts().precision);
        REQUIRE( jau::cfmt::plength_t::l == r.opts().length_mod);
    }
    {
        std::string s;
        jau::cfmt::Result r = jau::cfmt::formatR(s,"%.12" PRIi64, (int64_t)1);
        std::cerr << "FormatResult " << r << "\n";
        REQUIRE( true == r.success());
        REQUIRE( 1 == r.argumentCount());
        REQUIRE( flags_t::none == r.opts().flags);
        REQUIRE( false == r.opts().width_set);
        REQUIRE( 0 == r.opts().width);
        REQUIRE( true == r.opts().precision_set);
        REQUIRE( 12 == r.opts().precision);
        REQUIRE( jau::cfmt::plength_t::l == r.opts().length_mod);
    }
    {
        std::string s;
        jau::cfmt::Result r = jau::cfmt::formatR(s,"%23.12" PRIi64, (int64_t)1);
        std::cerr << "FormatResult " << r << "\n";
        REQUIRE( true == r.success());
        REQUIRE( 1 == r.argumentCount());
        REQUIRE( flags_t::none == r.opts().flags);
        REQUIRE( true == r.opts().width_set);
        REQUIRE( 23 == r.opts().width);
        REQUIRE( true == r.opts().precision_set);
        REQUIRE( 12 == r.opts().precision);
        REQUIRE( jau::cfmt::plength_t::l == r.opts().length_mod);
    }
    {
        std::string s;
        jau::cfmt::Result r = jau::cfmt::formatR(s,"%#-+0 23.12" PRIi64, (int64_t)1);
        std::cerr << "FormatResult " << r << "\n";
        REQUIRE( true == r.success());
        REQUIRE( 1 == r.argumentCount());
        REQUIRE( ( flags_t::left | flags_t::plus ) == r.opts().flags);
        REQUIRE( true == r.opts().width_set);
        REQUIRE( 23 == r.opts().width);
        REQUIRE( true == r.opts().precision_set);
        REQUIRE( 12 == r.opts().precision);
        REQUIRE( jau::cfmt::plength_t::l == r.opts().length_mod);
    }
}

TEST_CASE("parse: width precision from arg", "[jau][std::string][jau::cfmt]") {
    using namespace jau::cfmt;

    {
        jau_format_check("%*" PRIi64, 21, (int64_t)1);
        jau_format_checkLine("%*" PRIi64, 21, (int64_t)1);
        std::string s;
        jau::cfmt::Result r = jau::cfmt::formatR(s,"%*" PRIi64, 21, (int64_t)1);
        std::cerr << "FormatResult " << r << "\n";
        REQUIRE( true == r.success());
        REQUIRE( 2 == r.argumentCount());
        REQUIRE( flags_t::none == r.opts().flags);
        REQUIRE( true == r.opts().width_set);
        REQUIRE( 21 == r.opts().width);
        REQUIRE( false == r.opts().precision_set);
        REQUIRE( 0 == r.opts().precision);
        REQUIRE( jau::cfmt::plength_t::l == r.opts().length_mod);
    }
    {
        std::string s;
        jau::cfmt::Result r = jau::cfmt::formatR(s,"%.*" PRIi64, 12, (int64_t)1);
        std::cerr << "FormatResult " << r << "\n";
        REQUIRE( true == r.success());
        REQUIRE( 2 == r.argumentCount());
        REQUIRE( flags_t::none == r.opts().flags);
        REQUIRE( false == r.opts().width_set);
        REQUIRE( 0 == r.opts().width);
        REQUIRE( true == r.opts().precision_set);
        REQUIRE( 12 == r.opts().precision);
        REQUIRE( jau::cfmt::plength_t::l == r.opts().length_mod);
    }
    {
        std::string s;
        jau::cfmt::Result r = jau::cfmt::formatR(s,"%*.*" PRIi64, 23, 12, (int64_t)1);
        std::cerr << "FormatResult " << r << "\n";
        REQUIRE( true == r.success());
        REQUIRE( 3 == r.argumentCount());
        REQUIRE( flags_t::none == r.opts().flags);
        REQUIRE( true == r.opts().width_set);
        REQUIRE( 23 == r.opts().width);
        REQUIRE( true == r.opts().precision_set);
        REQUIRE( 12 == r.opts().precision);
        REQUIRE( jau::cfmt::plength_t::l == r.opts().length_mod);
    }

    {
        std::string s;
        jau::cfmt::Result r = jau::cfmt::formatR(s,"%-*.12" PRIi64, 23, (int64_t)1);
        std::cerr << "FormatResult " << r << "\n";
        REQUIRE( true == r.success());
        REQUIRE( 2 == r.argumentCount());
        REQUIRE( flags_t::left == r.opts().flags);
        REQUIRE( true == r.opts().width_set);
        REQUIRE( 23 == r.opts().width);
        REQUIRE( true == r.opts().precision_set);
        REQUIRE( 12 == r.opts().precision);
        REQUIRE( jau::cfmt::plength_t::l == r.opts().length_mod);
    }
    {
        std::string s;
        jau::cfmt::Result r = jau::cfmt::formatR(s,"%+.*" PRIi64, 12, (int64_t)1);
        std::cerr << "FormatResult " << r << "\n";
        REQUIRE( true == r.success());
        REQUIRE( 2 == r.argumentCount());
        REQUIRE( flags_t::plus == r.opts().flags);
        REQUIRE( false == r.opts().width_set);
        REQUIRE( 0 == r.opts().width);
        REQUIRE( true == r.opts().precision_set);
        REQUIRE( 12 == r.opts().precision);
        REQUIRE( jau::cfmt::plength_t::l == r.opts().length_mod);
    }
}

template <typename... Args>
static void checkFormat(int line, const char *fmt, const Args &...args) {
    std::string exp = jau::unsafe::format_string(fmt, args...);
    // std::string has = jau::format_string(fmt, args...);
    std::string has;
    jau::cfmt::Result r = jau::cfmt::formatR(has, fmt, args...);
    std::cerr << "FormatResult @ " << line << ": " << r << "\n";
    std::cerr << "FormatResult @ " << line << ": exp `" << exp << "`, has `" << has << "`\n\n";
    CHECK( true == r.success());
    CHECK( sizeof...(args) == r.argumentCount());
    CHECK(exp == has);
}

TEST_CASE("single_conversion", "[jau][std::string][jau::cfmt]") {
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
    const char sl1[] = "Hallo";
    std::string s2 = "World";
    std::string_view s2sv = s2;
    const char *s2p = s2.c_str();

    {
        double value = 123.45;
        const int expval = std::ilogb(value);
        const double frac = value / std::scalbn(1.0, expval);
        const uint64_t significand = jau::significand_raw(value);
        fprintf(stderr, "JAU:10 v %f = %f * 2^%d -> 0x%0" PRIx64 "p%d\n", value, frac, expval, significand, expval);

        const int32_t expval2 = jau::exponent_unbiased(value);
        fprintf(stderr, "JAU:11 v %f = %f * 2^%d -> 0x%0" PRIx64 "p%d\n", value, frac, expval2, significand, expval2);
    }
    {
        float value = 123.45f;
        const int expval = std::ilogb(value);
        const double frac = value / std::scalbn(1.0, expval);
        const uint32_t significand = jau::significand_raw(value);
        fprintf(stderr, "JAU:20 v %f = %f * 2^%d -> 0x%0" PRIx32 "p%d\n", value, frac, expval, significand, expval);

        const int32_t expval2 = jau::exponent_unbiased(value);
        fprintf(stderr, "JAU:21 v %f = %f * 2^%d -> 0x%0" PRIx32 "p%d\n", value, frac, expval2, significand, expval2);
    }
    {
        float ivalue = 123.45f;
        double value = ivalue;
        const int expval = std::ilogb(value);
        const double frac = value / std::scalbn(1.0, expval);
        const uint64_t significand = jau::significand_raw(value) >> (32-4);
        fprintf(stderr, "JAU:30 v %f = %f * 2^%d -> 0x%0" PRIx64 "p%d\n", value, frac, expval, significand, expval);

        const int32_t expval2 = jau::exponent_unbiased(value);
        fprintf(stderr, "JAU:31 v %f = %f * 2^%d -> 0x%0" PRIx64 "p%d\n", value, frac, expval2, significand, expval2);
    }
    checkFormat(__LINE__, "%%");

    checkFormat(__LINE__, "%c", 'Z');
    checkFormat(__LINE__, "%s", "Hello World");
    checkFormat(__LINE__, "%s", sl1);
    {
        // impossible for vsnprintf (via jau::unsafe::format_string)
        CHECK( 1 == jau::cfmt::check("%s", s2));
        CHECK( "World" == jau::format_string("%s", s2));
        CHECK( 1 == jau::cfmt::check("%s", s2sv));
        CHECK( "World" == jau::format_string("%s", s2sv));
    }
    checkFormat(__LINE__, "%p", &i32);
    checkFormat(__LINE__, "p1a %p %0p", p1a, p1a);
    checkFormat(__LINE__, "p1b %p %0p", p1b, p1b);
    checkFormat(__LINE__, "p2a %p %0p", p2a, p2a);
    checkFormat(__LINE__, "p2b %p %0p", p2b, p2b);
    checkFormat(__LINE__, "p3a %p %0p", p3a, p3a);
    checkFormat(__LINE__, "p3b %p %0p", p3b, p3b);
    checkFormat(__LINE__, "p3b %p %0p", &i32_u, &i32_u);
    checkFormat(__LINE__, "p3b %p %0p", &sl1, &sl1);
    checkFormat(__LINE__, "p3b %p %0p", s2p, s2p);
    checkFormat(__LINE__, "%p", (void *)nullptr);
    checkFormat(__LINE__, "%s", (char *)nullptr);

    checkFormat(__LINE__, "%d", i32);

    checkFormat(__LINE__, "%o", u32);
    checkFormat(__LINE__, "%x", u32);
    checkFormat(__LINE__, "%X", u32);
    checkFormat(__LINE__, "%u", u32);
    checkFormat(__LINE__, "%o", i32_u);
    checkFormat(__LINE__, "%x", i32_u);
    checkFormat(__LINE__, "%X", i32_u);
    checkFormat(__LINE__, "%u", i32_u);

    checkFormat(__LINE__, "%f", f64);
    checkFormat(__LINE__, "%e", f64);
    checkFormat(__LINE__, "%E", f64);
    checkFormat(__LINE__, "%a", f64);
    checkFormat(__LINE__, "%A", f64);
    // checkFormat(__LINE__, "%g", f64);
    // checkFormat(__LINE__, "%G", f64);

    checkFormat(__LINE__, "%f", f32);
    checkFormat(__LINE__, "%e", f32);
    checkFormat(__LINE__, "%E", f32);
    checkFormat(__LINE__, "%a", f32);
    checkFormat(__LINE__, "%A", f32);
    // checkFormat(__LINE__, "%g", f32);
    // checkFormat(__LINE__, "%G", f32);

    checkFormat(__LINE__, "%dZZZ", i32);
    checkFormat(__LINE__, "%dZZ", i32);
    checkFormat(__LINE__, "%dZ", i32);
    checkFormat(__LINE__, "Z%dZ Z%dZ", i32, i32);
    checkFormat(__LINE__, "Z%-6dZ Z%6dZ", i32, i32);

    checkFormat(__LINE__, "%#020x", 305441741);
    checkFormat(__LINE__, "%zd", 2147483647L);

    static_assert(0 < jau::cfmt::checkLine("%zd", 2147483647UL)); // failed intentionally unsigned -> signed
    checkFormat(__LINE__, "%zu", 2147483647UL);

    static_assert(0 == jau::cfmt::checkLine("%s", (const char*)"Test"));
    static_assert(0 == jau::cfmt::checkLine("%s", "Test"));
    checkFormat(__LINE__, "%s", "Test");
    {
        const char *str = nullptr;
        size_t str_len = 2;
        const char limiter = '3';
        const char *limiter_pos = nullptr;
        char *endptr = nullptr;

        jau_format_check("Value end not '%c' @ idx %zd, %p != %p, in: %p '%s' len %zu", limiter, endptr - str, endptr, limiter_pos, str, str, str_len);
        jau_format_checkLine("Value end not '%c' @ idx %zd, %p != %p, in: %p '%s' len %zu", limiter, endptr - str, endptr, limiter_pos, str, str, str_len);
    }
    // bool
    {
        jau_format_check("%d", (bool)true);
        jau_format_checkLine("%d", (bool)true);
        jau_format_check("%u", (bool)true);
        jau_format_checkLine("%u", (bool)true);
        jau_format_check("%s", (bool)true);
        jau_format_checkLine("%s", (bool)true);
        CHECK("1" == jau::format_string("%d", (bool)true));
        CHECK("0" == jau::format_string("%d", (bool)false));
        CHECK("1" == jau::format_string("%u", (bool)true));
        CHECK("0" == jau::format_string("%u", (bool)false));
        CHECK("true" == jau::format_string("%s", (bool)true));
        CHECK("false" == jau::format_string("%s", (bool)false));
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

        static_assert(true == std::is_enum_v<decltype(e1_u)>);
        static_assert(true == std::is_unsigned_v<std::underlying_type_t<decltype(e1_u)>>);

        static_assert(true == std::is_enum_v<decltype(e2_s)>);
        static_assert(true == std::is_signed_v<std::underlying_type_t<decltype(e2_s)>>);

        static_assert(true == std::is_enum_v<decltype(e3_s)>);
        static_assert(true == std::is_signed_v<std::underlying_type_t<decltype(e3_s)>>);

        static_assert(true == std::is_enum_v<decltype(e4_u)>);
        static_assert(true == std::is_unsigned_v<std::underlying_type_t<decltype(e4_u)>>);

        jau_format_string("Enum %u, %d, %d, %u\n", e1_u, e2_s, e3_s, e4_u);
        jau_format_checkLine("%u, %d, %d, %u\n", e1_u, e2_s, e3_s, e4_u);

        static_assert(4 == jau::cfmt::check("%u, %d, %d, %u\n", e1_u, e2_s, e3_s, e4_u));
        static_assert(0 == jau::cfmt::checkLine("%u, %u, %d, %u\n", e1_u, e2_s, e3_s, e4_u));

        static_assert(0 == jau::cfmt::checkLine("%u\n", e1_u)); // unsigned -> unsigned OK
        static_assert(0 < jau::cfmt::checkLine("%d\n", e1_u));  // unsigned -> signed ERROR
        static_assert(0 == jau::cfmt::checkLine("%u\n", e2_s)); // signed -> unsigned OK
    }
}

TEST_CASE("integral_conversion", "[jau][std::string][jau::cfmt]") {
    static constexpr const char *format_check_exp1 = "format_check: -1, 2, -3, 4, -5, 6, -7, 8, -9, 10";
    static constexpr const char *format_check_exp2 = "format_check: -1, 02, -03, 0004, -0005, 000006, -000007, 00000008, -00000009, 0000000010";
    char i1=-1;
    unsigned char i2=2;

    short i3=-3;
    unsigned short i4=4;

    int i5=-5;
    unsigned int i6=6;

    long i7=-7;
    unsigned long i8=8;

    ssize_t i9 = -9;
    size_t i10 = 10;

    jau_format_check("format_check: %hhd, %hhu, %hd, %hu, %d, %u, %ld, %lu, %zd, %zu", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
    CHECK(format_check_exp1 == jau::format_string("format_check: %hhd, %hhu, %hd, %hu, %d, %u, %ld, %lu, %zd, %zu", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10));

    jau_format_check("format_check: %01hhd, %02hhu, %03hd, %04hu, %05d, %06u, %07ld, %08lu, %09zd, %010zu", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
    CHECK(format_check_exp2 == jau::format_string("format_check: %01hhd, %02hhu, %03hd, %04hu, %05d, %06u, %07ld, %08lu, %09zd, %010zu", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10));
}

TEST_CASE("thousands_flag", "[jau][std::string][jau::cfmt][flags]" ) {
    //
    // thousand flag ' or ,
    //
    jau_format_checkLine("%'d", 1);
    jau_format_checkLine("%,d", 1);

    CHECK("1" == jau::format_string("%'d", 1));
    CHECK("10" == jau::format_string("%#'d", 10));
    CHECK("100" == jau::format_string("%,d", 100));
    CHECK("1'000" == jau::format_string("%#'d", 1000));
    CHECK("1'000'000" == jau::format_string("%,d", 1000000));
    CHECK("+1'000'000" == jau::format_string("%'+d", 1000000));
    CHECK("+1'000'000" == jau::format_string("%#'+d", 1000000));
    CHECK("-1'000'000" == jau::format_string("%,d", -1000000));
    CHECK("-1'000'000" == jau::format_string("%#'d", -1000000));

    CHECK("ff" == jau::format_string("%'x", 0xff_u32));
    CHECK("0xff" == jau::format_string("%#'x", 0xff_u32));
    CHECK("ffff" == jau::format_string("%,x", 0xffff_u32));
    CHECK("0x1'ffff" == jau::format_string("%#'x", 0x1ffff_u32));
    CHECK("1'ffff'ffff" == jau::format_string("%,lx", 0x1ffffffff_i64));
    CHECK("0x1'ffff'ffff" == jau::format_string("%#'lx", 0x1ffffffff_u64));
    // negative types not allowed for hex-conversion

    // separator, space-padding
    CHECK(" 876'543" == jau::format_string("%,8d", 876543));
    CHECK("9'876'543" == jau::format_string("%,8d", 9876543));
    CHECK("9'876'543" == jau::format_string("%,9d", 9876543));
    CHECK(" 9'876'543" == jau::format_string("%,10d", 9876543));
    CHECK("    9'876'543" == jau::format_string("%,13d", 9876543));

    CHECK("0xaffe" == jau::format_string("%#'x", 0xaffe_u32));
    CHECK("0xaffe" == jau::format_string("%#'6x", 0xaffe_u32));
    CHECK(" 0xaffe" == jau::format_string("%#'7x", 0xaffe_u32));
    CHECK("  0xaffe" == jau::format_string("%#'8x", 0xaffe_u32));
    CHECK("0x1'affe" == jau::format_string("%#'7x", 0x1affe_u32));
    CHECK("    0x1'affe" == jau::format_string("%#'12x", 0x1affe_u32));

    // separator, zero-padding
    CHECK("'876'543" == jau::format_string("%,08d", 876543));
    CHECK("9'876'543" == jau::format_string("%,08d", 9876543));
    CHECK("9'876'543" == jau::format_string("%,09d", 9876543));
    CHECK("09'876'543" == jau::format_string("%,010d", 9876543));
    CHECK("0'009'876'543" == jau::format_string("%,013d", 9876543));

    CHECK("0xaffe" == jau::format_string("%#'x", 0xaffe_u32));
    CHECK("0xaffe" == jau::format_string("%#'06x", 0xaffe_u32));
    CHECK("0x'affe" == jau::format_string("%#'07x", 0xaffe_u32));
    CHECK("0x0'affe" == jau::format_string("%#'08x", 0xaffe_u32));
    CHECK("0x1'affe" == jau::format_string("%#'07x", 0x1affe_u32));
    CHECK("0x'0001'affe" == jau::format_string("%#'012x", 0x1affe_u32));
}

TEST_CASE("binary", "[jau][std::string][jau::cfmt][flags]" ) {
    jau_format_checkLine("%b", 1_u32);

    CHECK("0b1" == jau::format_string("%#b", 1_u32));
    CHECK("0b1010111111111110" == jau::format_string("%#b", 0xaffe_u32));
    CHECK("1011111011101111" == jau::format_string("%b", 0xbeef_u32));
}

TEST_CASE("space_flag", "[jau][std::string][jau::cfmt][flags]" ) {
  std::string buffer;

  buffer = jau::format_string("% d", 42);
  CHECK(buffer == " 42");

  buffer = jau::format_string("% d", -42);
  CHECK(buffer == "-42");

  buffer = jau::format_string("% 5d", 42);
  CHECK(buffer == "   42");

  buffer = jau::format_string("% 5d", -42);
  CHECK(buffer == "  -42"); //   "-  42" == "  -42"

  buffer = jau::format_string("% 15d", 42);
  CHECK(buffer == "             42");

  buffer = jau::format_string("% 15d", -42);
  CHECK(buffer == "            -42");

  buffer = jau::format_string("% 15d", -42);
  CHECK(buffer == "            -42");

  buffer = jau::format_string("% 15.3f", -42.987);
  CHECK(buffer == "        -42.987");

  buffer = jau::format_string("% 15.3f", 42.987);
  CHECK(buffer == "         42.987");

  buffer = jau::format_string("% s", "Hello testing");
  CHECK(buffer == "Hello testing");

  buffer = jau::format_string("% d", 1024);
  CHECK(buffer == " 1024");

  buffer = jau::format_string("% d", -1024);
  CHECK(buffer == "-1024");

  buffer = jau::format_string("% i", 1024);
  CHECK(buffer == " 1024");

  buffer = jau::format_string("% i", -1024);
  CHECK(buffer == "-1024");

  buffer = jau::format_string("% u", 1024);
  CHECK(buffer == "1024");

  buffer = jau::format_string("% u", 4294966272U);
  CHECK(buffer == "4294966272");

  buffer = jau::format_string("% o", 511);
  CHECK(buffer == "777");

  buffer = jau::format_string("% o", 4294966785U);
  CHECK(buffer == "37777777001");

  buffer = jau::format_string("% x", 305441741);
  CHECK(buffer == "1234abcd");

  buffer = jau::format_string("% x", 3989525555U);
  CHECK(buffer == "edcb5433");

  buffer = jau::format_string("% X", 305441741);
  CHECK(buffer == "1234ABCD");

  buffer = jau::format_string("% X", 3989525555U);
  CHECK(buffer == "EDCB5433");

  buffer = jau::format_string("% c", 'x');
  CHECK(buffer == "x");
}

TEST_CASE("plus_flag", "[jau][std::string][jau::cfmt][flags]" ) {
  std::string buffer;

  buffer = jau::format_string("%+d", 42);
  CHECK(buffer == "+42");

  buffer = jau::format_string("%+d", -42);
  CHECK(buffer == "-42");

  buffer = jau::format_string("%+5d", 42);
  CHECK(buffer == "  +42");

  buffer = jau::format_string("%+5d", -42);
  CHECK(buffer == "  -42");

  buffer = jau::format_string("%+15d", 42);
  CHECK(buffer == "            +42");

  buffer = jau::format_string("%+15d", -42);
  CHECK(buffer == "            -42");

  buffer = jau::format_string("%+s", "Hello testing");
  CHECK(buffer == "Hello testing");

  buffer = jau::format_string("%+d", 1024);
  CHECK(buffer == "+1024");

  buffer = jau::format_string("%+d", -1024);
  CHECK(buffer == "-1024");

  buffer = jau::format_string("%+i", 1024);
  CHECK(buffer == "+1024");

  buffer = jau::format_string("%+i", -1024);
  CHECK(buffer == "-1024");

  buffer = jau::format_string("%+u", 1024);
  CHECK(buffer == "1024");

  buffer = jau::format_string("%+u", 4294966272U);
  CHECK(buffer == "4294966272");

  buffer = jau::format_string("%+o", 511);
  CHECK(buffer == "777");

  buffer = jau::format_string("%+o", 4294966785U);
  CHECK(buffer == "37777777001");

  buffer = jau::format_string("%+x", 305441741);
  CHECK(buffer == "1234abcd");

  buffer = jau::format_string("%+x", 3989525555U);
  CHECK(buffer == "edcb5433");

  buffer = jau::format_string("%+X", 305441741);
  CHECK(buffer == "1234ABCD");

  buffer = jau::format_string("%+X", 3989525555U);
  CHECK(buffer == "EDCB5433");

  buffer = jau::format_string("%+c", 'x');
  CHECK(buffer == "x");

  buffer = jau::format_string("%+.0d", 0);
  CHECK(buffer == "+");
}


TEST_CASE("zero_flag", "[jau][std::string][jau::cfmt][flags]" ) {
  std::string buffer;

  buffer = jau::format_string("%0d", 42);
  CHECK(buffer == "42");

  buffer = jau::format_string("%0ld", 42L);
  CHECK(buffer == "42");

  buffer = jau::format_string("%0d", -42);
  CHECK(buffer == "-42");

  buffer = jau::format_string("%05d", 42);
  CHECK(buffer == "00042");

  buffer = jau::format_string("%05d", -42);
  CHECK(buffer == "-0042");

  buffer = jau::format_string("%015d", 42);
  CHECK(buffer == "000000000000042");

  buffer = jau::format_string("%015d", -42);
  CHECK(buffer == "-00000000000042");

  buffer = jau::format_string("%015.2f", 42.1234);
  CHECK(buffer == "000000000042.12");

  buffer = jau::format_string("%015.3f", 42.9876);
  CHECK(buffer == "00000000042.988");

  buffer = jau::format_string("%015.5f", -42.9876);
  CHECK(buffer == "-00000042.98760");
}


TEST_CASE("left_flag", "[jau][std::string][jau::cfmt][flags]" ) {
  std::string buffer;

  buffer = jau::format_string("%-d", 42);
  CHECK(buffer == "42");

  buffer = jau::format_string("%-d", -42);
  CHECK(buffer == "-42");

  buffer = jau::format_string("%-5d", 42);
  CHECK(buffer == "42   ");

  buffer = jau::format_string("%-5d", -42);
  CHECK(buffer == "-42  ");

  buffer = jau::format_string("%-15d", 42);
  CHECK(buffer == "42             ");

  buffer = jau::format_string("%-15d", -42);
  CHECK(buffer == "-42            ");

  buffer = jau::format_string("%-0d", 42);
  CHECK(buffer == "42");

  buffer = jau::format_string("%-0d", -42);
  CHECK(buffer == "-42");

  buffer = jau::format_string("%-05d", 42);
  CHECK(buffer == "42   ");

  buffer = jau::format_string("%-05d", -42);
  CHECK(buffer == "-42  ");

  buffer = jau::format_string("%-015d", 42);
  CHECK(buffer == "42             ");

  buffer = jau::format_string("%-015d", -42);
  CHECK(buffer == "-42            ");

  buffer = jau::format_string("%0-d", 42);
  CHECK(buffer == "42");

  buffer = jau::format_string("%0-d", -42);
  CHECK(buffer == "-42");

  buffer = jau::format_string("%0-5d", 42);
  CHECK(buffer == "42   ");

  buffer = jau::format_string("%0-5d", -42);
  CHECK(buffer == "-42  ");

  buffer = jau::format_string("%0-15d", 42);
  CHECK(buffer == "42             ");

  buffer = jau::format_string("%0-15d", -42);
  CHECK(buffer == "-42            ");

  buffer = jau::format_string("%0-15.3e", -42.);
  CHECK(buffer == "-4.200e+01     ");

  buffer = jau::format_string("%0-15.3g", -42.);
  CHECK(buffer == "-42.0          ");
}


TEST_CASE("hash_flag", "[jau][std::string][jau::cfmt][flags]" ) {
  std::string buffer;

  buffer = jau::format_string("%#.0x", 0);
  CHECK(buffer == "");
  buffer = jau::format_string("%#.1x", 0);
  CHECK(buffer == "0");
  buffer = jau::format_string("%#.0llx", (long long)0);
  CHECK(buffer == "");
  buffer = jau::format_string("%#.8x", 0x614e);
  CHECK(buffer == "0x0000614e");
  buffer = jau::format_string("%#b", 6);
  CHECK(buffer == "0b110");
}


TEST_CASE("specifier", "[jau][std::string][jau::cfmt][specifier]" ) {
  std::string buffer;

  buffer = jau::format_string("Hello testing");
  CHECK(buffer == "Hello testing");

  buffer = jau::format_string("%s", "Hello testing");
  CHECK(buffer == "Hello testing");

  buffer = jau::format_string("%d", 1024);
  CHECK(buffer == "1024");

  buffer = jau::format_string("%d", -1024);
  CHECK(buffer == "-1024");

  buffer = jau::format_string("%i", 1024);
  CHECK(buffer == "1024");

  buffer = jau::format_string("%i", -1024);
  CHECK(buffer == "-1024");

  buffer = jau::format_string("%u", 1024);
  CHECK(buffer == "1024");

  buffer = jau::format_string("%u", 4294966272U);
  CHECK(buffer == "4294966272");

  buffer = jau::format_string("%o", 511);
  CHECK(buffer == "777");

  buffer = jau::format_string("%o", 4294966785U);
  CHECK(buffer == "37777777001");

  buffer = jau::format_string("%x", 305441741);
  CHECK(buffer == "1234abcd");

  buffer = jau::format_string("%x", 3989525555U);
  CHECK(buffer == "edcb5433");

  buffer = jau::format_string("%X", 305441741);
  CHECK(buffer == "1234ABCD");

  buffer = jau::format_string("%X", 3989525555U);
  CHECK(buffer == "EDCB5433");

  buffer = jau::format_string("%%");
  CHECK(buffer == "%");
}


TEST_CASE("width", "[jau][std::string][jau::cfmt][width]" ) {
  std::string buffer;

  buffer = jau::format_string("%1s", "Hello testing");
  CHECK(buffer == "Hello testing");

  buffer = jau::format_string("%1d", 1024);
  CHECK(buffer == "1024");

  buffer = jau::format_string("%1d", -1024);
  CHECK(buffer == "-1024");

  buffer = jau::format_string("%1i", 1024);
  CHECK(buffer == "1024");

  buffer = jau::format_string("%1i", -1024);
  CHECK(buffer == "-1024");

  buffer = jau::format_string("%1u", 1024);
  CHECK(buffer == "1024");

  buffer = jau::format_string("%1u", 4294966272U);
  CHECK(buffer == "4294966272");

  buffer = jau::format_string("%1o", 511);
  CHECK(buffer == "777");

  buffer = jau::format_string("%1o", 4294966785U);
  CHECK(buffer == "37777777001");

  buffer = jau::format_string("%1x", 305441741);
  CHECK(buffer == "1234abcd");

  buffer = jau::format_string("%1x", 3989525555U);
  CHECK(buffer == "edcb5433");

  buffer = jau::format_string("%1X", 305441741);
  CHECK(buffer == "1234ABCD");

  buffer = jau::format_string("%1X", 3989525555U);
  CHECK(buffer == "EDCB5433");

  buffer = jau::format_string("%1c", 'x');
  CHECK(buffer == "x");
}


TEST_CASE("width_20", "[jau][std::string][jau::cfmt][width]" ) {
  std::string buffer;

  buffer = jau::format_string("%20s", "Hello");
  CHECK(buffer == "               Hello");

  buffer = jau::format_string("%20d", 1024);
  CHECK(buffer == "                1024");

  buffer = jau::format_string("%20d", -1024);
  CHECK(buffer == "               -1024");

  buffer = jau::format_string("%20i", 1024);
  CHECK(buffer == "                1024");

  buffer = jau::format_string("%20i", -1024);
  CHECK(buffer == "               -1024");

  buffer = jau::format_string("%20u", 1024);
  CHECK(buffer == "                1024");

  buffer = jau::format_string("%20u", 4294966272U);
  CHECK(buffer == "          4294966272");

  buffer = jau::format_string("%20o", 511);
  CHECK(buffer == "                 777");

  buffer = jau::format_string("%20o", 4294966785U);
  CHECK(buffer == "         37777777001");

  buffer = jau::format_string("%20x", 305441741);
  CHECK(buffer == "            1234abcd");

  buffer = jau::format_string("%20x", 3989525555U);
  CHECK(buffer == "            edcb5433");

  buffer = jau::format_string("%20X", 305441741);
  CHECK(buffer == "            1234ABCD");

  buffer = jau::format_string("%20X", 3989525555U);
  CHECK(buffer == "            EDCB5433");

  buffer = jau::format_string("%20c", 'x');
  CHECK(buffer == "                   x");
}


TEST_CASE("width_star_20", "[jau][std::string][jau::cfmt][width]" ) {
  std::string buffer;

  buffer = jau::format_string("%*s", 20, "Hello");
  CHECK(buffer == "               Hello");

  buffer = jau::format_string("%*d", 20, 1024);
  CHECK(buffer == "                1024");

  buffer = jau::format_string("%*d", 20, -1024);
  CHECK(buffer == "               -1024");

  buffer = jau::format_string("%*i", 20, 1024);
  CHECK(buffer == "                1024");

  buffer = jau::format_string("%*i", 20, -1024);
  CHECK(buffer == "               -1024");

  buffer = jau::format_string("%*u", 20, 1024);
  CHECK(buffer == "                1024");

  buffer = jau::format_string("%*u", 20, 4294966272U);
  CHECK(buffer == "          4294966272");

  buffer = jau::format_string("%*o", 20, 511);
  CHECK(buffer == "                 777");

  buffer = jau::format_string("%*o", 20, 4294966785U);
  CHECK(buffer == "         37777777001");

  buffer = jau::format_string("%*x", 20, 305441741);
  CHECK(buffer == "            1234abcd");

  buffer = jau::format_string("%*x", 20, 3989525555U);
  CHECK(buffer == "            edcb5433");

  buffer = jau::format_string("%*X", 20, 305441741);
  CHECK(buffer == "            1234ABCD");

  buffer = jau::format_string("%*X", 20, 3989525555U);
  CHECK(buffer == "            EDCB5433");

  buffer = jau::format_string("%*c", 20,'x');
  CHECK(buffer == "                   x");
}


TEST_CASE("width_left_20", "[jau][std::string][jau::cfmt][width]" ) {
  std::string buffer;

  buffer = jau::format_string("%-20s", "Hello");
  CHECK(buffer == "Hello               ");

  buffer = jau::format_string("%-20d", 1024);
  CHECK(buffer == "1024                ");

  buffer = jau::format_string("%-20d", -1024);
  CHECK(buffer == "-1024               ");

  buffer = jau::format_string("%-20i", 1024);
  CHECK(buffer == "1024                ");

  buffer = jau::format_string("%-20i", -1024);
  CHECK(buffer == "-1024               ");

  buffer = jau::format_string("%-20u", 1024);
  CHECK(buffer == "1024                ");

  buffer = jau::format_string("%-20.4f", 1024.1234);
  CHECK(buffer == "1024.1234           ");

  buffer = jau::format_string("%-20u", 4294966272U);
  CHECK(buffer == "4294966272          ");

  buffer = jau::format_string("%-20o", 511);
  CHECK(buffer == "777                 ");

  buffer = jau::format_string("%-20o", 4294966785U);
  CHECK(buffer == "37777777001         ");

  buffer = jau::format_string("%-20x", 305441741);
  CHECK(buffer == "1234abcd            ");

  buffer = jau::format_string("%-20x", 3989525555U);
  CHECK(buffer == "edcb5433            ");

  buffer = jau::format_string("%-20X", 305441741);
  CHECK(buffer == "1234ABCD            ");

  buffer = jau::format_string("%-20X", 3989525555U);
  CHECK(buffer == "EDCB5433            ");

  buffer = jau::format_string("%-20c", 'x');
  CHECK(buffer == "x                   ");

  buffer = jau::format_string("|%5d| |%-2d| |%5d|", 9, 9, 9);
  CHECK(buffer == "|    9| |9 | |    9|");

  buffer = jau::format_string("|%5d| |%-2d| |%5d|", 10, 10, 10);
  CHECK(buffer == "|   10| |10| |   10|");

  buffer = jau::format_string("|%5d| |%-12d| |%5d|", 9, 9, 9);
  CHECK(buffer == "|    9| |9           | |    9|");

  buffer = jau::format_string("|%5d| |%-12d| |%5d|", 10, 10, 10);
  CHECK(buffer == "|   10| |10          | |   10|");
}


TEST_CASE("zero_width_left_20", "[jau][std::string][jau::cfmt][width]" ) {
  std::string buffer;

  buffer = jau::format_string("%0-20s", "Hello");
  CHECK(buffer == "Hello               ");

  buffer = jau::format_string("%0-20d", 1024);
  CHECK(buffer == "1024                ");

  buffer = jau::format_string("%0-20d", -1024);
  CHECK(buffer == "-1024               ");

  buffer = jau::format_string("%0-20i", 1024);
  CHECK(buffer == "1024                ");

  buffer = jau::format_string("%0-20i", -1024);
  CHECK(buffer == "-1024               ");

  buffer = jau::format_string("%0-20u", 1024);
  CHECK(buffer == "1024                ");

  buffer = jau::format_string("%0-20u", 4294966272U);
  CHECK(buffer == "4294966272          ");

  buffer = jau::format_string("%0-20o", 511);
  CHECK(buffer == "777                 ");

  buffer = jau::format_string("%0-20o", 4294966785U);
  CHECK(buffer == "37777777001         ");

  buffer = jau::format_string("%0-20x", 305441741);
  CHECK(buffer == "1234abcd            ");

  buffer = jau::format_string("%0-20x", 3989525555U);
  CHECK(buffer == "edcb5433            ");

  buffer = jau::format_string("%0-20X", 305441741);
  CHECK(buffer == "1234ABCD            ");

  buffer = jau::format_string("%0-20X", 3989525555U);
  CHECK(buffer == "EDCB5433            ");

  buffer = jau::format_string("%0-20c", 'x');
  CHECK(buffer == "x                   ");
}


TEST_CASE("width_20", "[jau][std::string][jau::cfmt][padding]" ) {
  std::string buffer;

  buffer = jau::format_string("%020d", 1024);
  CHECK(buffer == "00000000000000001024");

  buffer = jau::format_string("%020d", -1024);
  CHECK(buffer == "-0000000000000001024");

  buffer = jau::format_string("%020i", 1024);
  CHECK(buffer == "00000000000000001024");

  buffer = jau::format_string("%020i", -1024);
  CHECK(buffer == "-0000000000000001024");

  buffer = jau::format_string("%020u", 1024);
  CHECK(buffer == "00000000000000001024");

  buffer = jau::format_string("%020u", 4294966272U);
  CHECK(buffer == "00000000004294966272");

  buffer = jau::format_string("%020o", 511);
  CHECK(buffer == "00000000000000000777");

  buffer = jau::format_string("%020o", 4294966785U);
  CHECK(buffer == "00000000037777777001");

  buffer = jau::format_string("%020x", 305441741);
  CHECK(buffer == "0000000000001234abcd");

  buffer = jau::format_string("%020x", 3989525555U);
  CHECK(buffer == "000000000000edcb5433");

  buffer = jau::format_string("%020X", 305441741);
  CHECK(buffer == "0000000000001234ABCD");

  buffer = jau::format_string("%020X", 3989525555U);
  CHECK(buffer == "000000000000EDCB5433");
}


TEST_CASE("precision_20", "[jau][std::string][jau::cfmt][padding]" ) {
  std::string buffer;

  buffer = jau::format_string("%.20d", 1024);
  CHECK(buffer == "00000000000000001024");

  buffer = jau::format_string("%.20d", -1024);
  CHECK(buffer == "-00000000000000001024");

  buffer = jau::format_string("%.20i", 1024);
  CHECK(buffer == "00000000000000001024");

  buffer = jau::format_string("%.20i", -1024);
  CHECK(buffer == "-00000000000000001024");

  buffer = jau::format_string("%.20u", 1024);
  CHECK(buffer == "00000000000000001024");

  buffer = jau::format_string("%.20u", 4294966272U);
  CHECK(buffer == "00000000004294966272");

  buffer = jau::format_string("%.20o", 511);
  CHECK(buffer == "00000000000000000777");

  buffer = jau::format_string("%.20o", 4294966785U);
  CHECK(buffer == "00000000037777777001");

  buffer = jau::format_string("%.20x", 305441741);
  CHECK(buffer == "0000000000001234abcd");

  buffer = jau::format_string("%.20x", 3989525555U);
  CHECK(buffer == "000000000000edcb5433");

  buffer = jau::format_string("%.20X", 305441741);
  CHECK(buffer == "0000000000001234ABCD");

  buffer = jau::format_string("%.20X", 3989525555U);
  CHECK(buffer == "000000000000EDCB5433");
}


TEST_CASE("hash_zero_width_20", "[jau][std::string][jau::cfmt][padding]" ) {
  std::string buffer;

  buffer = jau::format_string("%#020d", 1024);
  CHECK(buffer == "00000000000000001024");

  buffer = jau::format_string("%#020d", -1024);
  CHECK(buffer == "-0000000000000001024");

  buffer = jau::format_string("%#020i", 1024);
  CHECK(buffer == "00000000000000001024");

  buffer = jau::format_string("%#020i", -1024);
  CHECK(buffer == "-0000000000000001024");

  buffer = jau::format_string("%#020u", 1024);
  CHECK(buffer == "00000000000000001024");

  buffer = jau::format_string("%#020u", 4294966272U);
  CHECK(buffer == "00000000004294966272");

  buffer = jau::format_string("%#020o", 511);
  CHECK(buffer == "00000000000000000777");

  buffer = jau::format_string("%#020o", 4294966785U);
  CHECK(buffer == "00000000037777777001");

  buffer = jau::format_string("%#020x", 305441741);
  CHECK(buffer == "0x00000000001234abcd");

  buffer = jau::format_string("%#020x", 3989525555U);
  CHECK(buffer == "0x0000000000edcb5433");

  buffer = jau::format_string("%#020X", 305441741);
  CHECK(buffer == "0X00000000001234ABCD");

  buffer = jau::format_string("%#020X", 3989525555U);
  CHECK(buffer == "0X0000000000EDCB5433");
}


TEST_CASE("hash_width_20", "[jau][std::string][jau::cfmt][padding]" ) {
  std::string buffer;

  buffer = jau::format_string("%#20d", 1024);
  CHECK(buffer == "                1024");

  buffer = jau::format_string("%#20d", -1024);
  CHECK(buffer == "               -1024");

  buffer = jau::format_string("%#20i", 1024);
  CHECK(buffer == "                1024");

  buffer = jau::format_string("%#20i", -1024);
  CHECK(buffer == "               -1024");

  buffer = jau::format_string("%#20u", 1024);
  CHECK(buffer == "                1024");

  buffer = jau::format_string("%#20u", 4294966272U);
  CHECK(buffer == "          4294966272");

  buffer = jau::format_string("%#20o", 511);
  CHECK(buffer == "                0777");

  buffer = jau::format_string("%#20o", 4294966785U);
  CHECK(buffer == "        037777777001");

  buffer = jau::format_string("%#20x", 305441741);
  CHECK(buffer == "          0x1234abcd");

  buffer = jau::format_string("%#20x", 3989525555U);
  CHECK(buffer == "          0xedcb5433");

  buffer = jau::format_string("%#20X", 305441741);
  CHECK(buffer == "          0X1234ABCD");

  buffer = jau::format_string("%#20X", 3989525555U);
  CHECK(buffer == "          0XEDCB5433");
}


TEST_CASE("width_20_precision_5", "[jau][std::string][jau::cfmt][padding]" ) {
  std::string buffer;

  buffer = jau::format_string("%20.5d", 1024);
  CHECK(buffer == "               01024");

  buffer = jau::format_string("%20.5d", -1024);
  CHECK(buffer == "              -01024");

  buffer = jau::format_string("%20.5i", 1024);
  CHECK(buffer == "               01024");

  buffer = jau::format_string("%20.5i", -1024);
  CHECK(buffer == "              -01024");

  buffer = jau::format_string("%20.5u", 1024);
  CHECK(buffer == "               01024");

  buffer = jau::format_string("%20.5u", 4294966272U);
  CHECK(buffer == "          4294966272");

  buffer = jau::format_string("%20.5o", 511);
  CHECK(buffer == "               00777");

  buffer = jau::format_string("%20.5o", 4294966785U);
  CHECK(buffer == "         37777777001");

  buffer = jau::format_string("%20.5x", 305441741);
  CHECK(buffer == "            1234abcd");

  buffer = jau::format_string("%20.10x", 3989525555U);
  CHECK(buffer == "          00edcb5433");

  buffer = jau::format_string("%20.5X", 305441741);
  CHECK(buffer == "            1234ABCD");

  buffer = jau::format_string("%20.10X", 3989525555U);
  CHECK(buffer == "          00EDCB5433");
}


TEST_CASE("padding neg_numbers", "[jau][std::string][jau::cfmt][padding]" ) {
  std::string buffer;

  // space padding
  buffer = jau::format_string("% 1d", -5);
  CHECK(buffer == "-5");

  buffer = jau::format_string("% 2d", -5);
  CHECK(buffer == "-5");

  buffer = jau::format_string("% 3d", -5);
  CHECK(buffer == " -5");

  buffer = jau::format_string("% 4d", -5);
  CHECK(buffer == "  -5");

  // zero padding
  buffer = jau::format_string("%01d", -5);
  CHECK(buffer == "-5");

  buffer = jau::format_string("%02d", -5);
  CHECK(buffer == "-5");

  buffer = jau::format_string("%03d", -5);
  CHECK(buffer == "-05");

  buffer = jau::format_string("%04d", -5);
  CHECK(buffer == "-005");
}


TEST_CASE("float padding_neg_numbers", "[jau][std::string][jau::cfmt][float]" ) {
  std::string buffer;

  // space padding
  buffer = jau::format_string("% 3.1f", -5.);
  CHECK(buffer == "-5.0");

  buffer = jau::format_string("% 4.1f", -5.);
  CHECK(buffer == "-5.0");

  buffer = jau::format_string("% 5.1f", -5.);
  CHECK(buffer == " -5.0");

  buffer = jau::format_string("% 6.1g", -5.);
  CHECK(buffer == "    -5");

  buffer = jau::format_string("% 6.1e", -5.);
  CHECK(buffer == "-5.0e+00");

  buffer = jau::format_string("% 10.1e", -5.);
  CHECK(buffer == "  -5.0e+00");

  // zero padding
  buffer = jau::format_string("%03.1f", -5.);
  CHECK(buffer == "-5.0");

  buffer = jau::format_string("%04.1f", -5.);
  CHECK(buffer == "-5.0");

  buffer = jau::format_string("%05.1f", -5.);
  CHECK(buffer == "-05.0");

  // zero padding no decimal point
  buffer = jau::format_string("%01.0f", -5.);
  CHECK(buffer == "-5");

  buffer = jau::format_string("%02.0f", -5.);
  CHECK(buffer == "-5");

  buffer = jau::format_string("%03.0f", -5.);
  CHECK(buffer == "-05");

  buffer = jau::format_string("%010.1e", -5.);
  CHECK(buffer == "-005.0e+00");

  buffer = jau::format_string("%07.0E", -5.);
  CHECK(buffer == "-05E+00");

  buffer = jau::format_string("%03.0g", -5.);
  CHECK(buffer == "-05");
}

TEST_CASE("length", "[jau][std::string][jau::cfmt][length]" ) {
  std::string buffer;

  buffer = jau::format_string("%.0s", "Hello testing");
  CHECK(buffer == "");

  buffer = jau::format_string("%20.0s", "Hello testing");
  CHECK(buffer == "                    ");

  buffer = jau::format_string("%.s", "Hello testing");
  CHECK(buffer == "");

  buffer = jau::format_string("%20.s", "Hello testing");
  CHECK(buffer == "                    ");

  buffer = jau::format_string("%20.0d", 1024);
  CHECK(buffer == "                1024");

  buffer = jau::format_string("%20.0d", -1024);
  CHECK(buffer == "               -1024");

  buffer = jau::format_string("%20.d", 0);
  CHECK(buffer == "                    ");

  buffer = jau::format_string("%20.0i", 1024);
  CHECK(buffer == "                1024");

  buffer = jau::format_string("%20.i", -1024);
  CHECK(buffer == "               -1024");

  buffer = jau::format_string("%20.i", 0);
  CHECK(buffer == "                    ");

  buffer = jau::format_string("%20.u", 1024);
  CHECK(buffer == "                1024");

  buffer = jau::format_string("%20.0u", 4294966272U);
  CHECK(buffer == "          4294966272");

  buffer = jau::format_string("%20.u", 0U);
  CHECK(buffer == "                    ");

  buffer = jau::format_string("%20.o", 511);
  CHECK(buffer == "                 777");

  buffer = jau::format_string("%20.0o", 4294966785U);
  CHECK(buffer == "         37777777001");

  buffer = jau::format_string("%20.o", 0U);
  CHECK(buffer == "                    ");

  buffer = jau::format_string("%20.x", 305441741);
  CHECK(buffer == "            1234abcd");

  buffer = jau::format_string("%50.x", 305441741);
  CHECK(buffer == "                                          1234abcd");

  buffer = jau::format_string("%50.x%10.u", 305441741, 12345);
  CHECK(buffer == "                                          1234abcd     12345");

  buffer = jau::format_string("%20.0x", 3989525555U);
  CHECK(buffer == "            edcb5433");

  buffer = jau::format_string("%20.x", 0U);
  CHECK(buffer == "                    ");

  buffer = jau::format_string("%20.X", 305441741);
  CHECK(buffer == "            1234ABCD");

  buffer = jau::format_string("%20.0X", 3989525555U);
  CHECK(buffer == "            EDCB5433");

  buffer = jau::format_string("%20.X", 0U);
  CHECK(buffer == "                    ");

  buffer = jau::format_string("%02.0u", 0U);
  CHECK(buffer == "  ");

  buffer = jau::format_string("%02.0d", 0);
  CHECK(buffer == "  ");
}


TEST_CASE("float", "[jau][std::string][jau::cfmt][float]" ) {
  std::string buffer;

  // test special-case floats using math.h macros
  buffer = jau::format_string("%8f", NAN);
  CHECK(buffer == "     nan");

  buffer = jau::format_string("%8f", INFINITY);
  CHECK(buffer == "     inf");

  buffer = jau::format_string("%-8f", -INFINITY);
  CHECK(buffer == "-inf    ");

  buffer = jau::format_string("%+8e", INFINITY);
  CHECK(buffer == "    +inf");

  buffer = jau::format_string("%.4f", 3.1415354); // NOLINT
  CHECK(buffer == "3.1415");

  buffer = jau::format_string("%.3f", 30343.1415354);
  CHECK(buffer == "30343.142");

  buffer = jau::format_string("%.0f", 34.1415354);
  CHECK(buffer == "34");

  buffer = jau::format_string("%.0f", 1.3);
  CHECK(buffer == "1");

  buffer = jau::format_string("%.0f", 1.55);
  CHECK(buffer == "2");

  buffer = jau::format_string("%.1f", 1.64);
  CHECK(buffer == "1.6");

  buffer = jau::format_string("%.2f", 42.8952);
  CHECK(buffer == "42.90");

  buffer = jau::format_string("%.9f", 42.8952);
  CHECK(buffer == "42.895200000");

  buffer = jau::format_string("%.10f", 42.895223);
  CHECK(buffer == "42.8952230000");

  // assuming not being truncated to 9 digits. (19)
  buffer = jau::format_string("%.12f", 42.987654321098);
  CHECK(buffer == "42.987654321098");

  // assuming not being truncated to 9 digits, but rounded
  buffer = jau::format_string("%.12f", 42.98765432109899);
  CHECK(buffer == "42.987654321099");

  // 14
  buffer = jau::format_string("%.14f", 42.98765432109876);
  CHECK(buffer == "42.98765432109876");
  // 14 rounded
  buffer = jau::format_string("%.14f", 42.9876543210987699);
  CHECK(buffer == "42.98765432109877");

  // 16 truncated to 14 (max precision)
  buffer = jau::format_string("%.16f", 42.9876543210987612);
  CHECK(buffer == "42.9876543210987600");

  // 16 truncated to 14 (max precision) and rounded
  buffer = jau::format_string("%.16f", 42.9876543210987654);
  CHECK(buffer == "42.9876543210987700");

  buffer = jau::format_string("%6.2f", 42.8952);
  CHECK(buffer == " 42.90");

  buffer = jau::format_string("%+6.2f", 42.8952);
  CHECK(buffer == "+42.90");

  buffer = jau::format_string("%+5.1f", 42.9252);
  CHECK(buffer == "+42.9");

  buffer = jau::format_string("%f", 42.5);
  CHECK(buffer == "42.500000");

  buffer = jau::format_string("%.1f", 42.5);
  CHECK(buffer == "42.5");

  buffer = jau::format_string("%f", 42167.0);
  CHECK(buffer == "42167.000000");

  buffer = jau::format_string("%.9f", -12345.987654321);
  CHECK(buffer == "-12345.987654321");

  buffer = jau::format_string("%.1f", 3.999);
  CHECK(buffer == "4.0");

  buffer = jau::format_string("%.0f", 3.5);
  CHECK(buffer == "4");

  buffer = jau::format_string("%.0f", 4.5);
  CHECK(buffer == "4");

  buffer = jau::format_string("%.0f", 3.49);
  CHECK(buffer == "3");

  buffer = jau::format_string("%.1f", 3.49);
  CHECK(buffer == "3.5");

  buffer = jau::format_string("a%-5.1f", 0.5);
  CHECK(buffer == "a0.5  ");

  buffer = jau::format_string("a%-5.1fend", 0.5);
  CHECK(buffer == "a0.5  end");

  buffer = jau::format_string("%G", 12345.678);
  CHECK(buffer == "12345.7");

  buffer = jau::format_string("%.7G", 12345.678);
  CHECK(buffer == "12345.68");

  buffer = jau::format_string("%.5G", 123456789.);
  CHECK(buffer == "1.2346E+08");

  buffer = jau::format_string("%.6G", 12345.);
  CHECK(buffer == "12345.0");

  buffer = jau::format_string("%+12.4g", 123456789.);
  CHECK(buffer == "  +1.235e+08");

  buffer = jau::format_string("%.2G", 0.001234);
  CHECK(buffer == "0.0012");

  buffer = jau::format_string("%+10.4G", 0.001234);
  CHECK(buffer == " +0.001234");

  buffer = jau::format_string("%+012.4g", 0.00001234);
  CHECK(buffer == "+001.234e-05");

  buffer = jau::format_string("%.3g", -1.2345e-308);
  CHECK(buffer == "-1.23e-308");

  buffer = jau::format_string("%+.3E", 1.23e+308);
  CHECK(buffer == "+1.230E+308");

  // out of range for float: should switch to exp notation if supported, else empty
  buffer = jau::format_string("%.1f", 1E20);
  CHECK(buffer == "1.0e+20");

  buffer = jau::format_string("%.5f", -1.12345);
  CHECK(buffer == "-1.12345");

  buffer = jau::format_string("%.5f", -1.00000e20);
  CHECK(buffer == "-1.00000e+20");

  // brute force float
  bool fail = false;
  std::stringstream str;
  str.precision(5);
  for (float i = -100000; i < 100000; i += 1) { // NOLINT
    buffer = jau::format_string("%.5f", i / 10000);
    str.str("");
    str << std::fixed << i / 10000;
    fail = fail || buffer != str.str();
  }
  CHECK(!fail);

  // brute force exp
  fail = false;
  str.setf(std::ios::scientific, std::ios::floatfield);
  for (float i = -1e20; i < 1e20; i += 1e15) { // NOLINT
    buffer = jau::format_string("%.5f", i);
    buffer.shrink_to_fit();
    str.str("");
    str << i;
    REQUIRE(buffer == str.str());
    fail = fail || buffer != str.str();
  }
  CHECK(!fail);
}


TEST_CASE("types", "[jau][std::string][jau::cfmt][types]" ) {
  std::string buffer;

  buffer = jau::format_string("%i", 0);
  CHECK(buffer == "0");

  buffer = jau::format_string("%i", 1234);
  CHECK(buffer == "1234");

  buffer = jau::format_string("%i", 32767);
  CHECK(buffer == "32767");

  buffer = jau::format_string("%i", -32767);
  CHECK(buffer == "-32767");

  buffer = jau::format_string("%li", 30L);
  CHECK(buffer == "30");

  buffer = jau::format_string("%li", -2147483647L);
  CHECK(buffer == "-2147483647");

  buffer = jau::format_string("%li", 2147483647L);
  CHECK(buffer == "2147483647");

  buffer = jau::format_string("%lli", 30LL);
  CHECK(buffer == "30");

  buffer = jau::format_string("%lli", -9223372036854775807LL);
  CHECK(buffer == "-9223372036854775807");

  buffer = jau::format_string("%lli", 9223372036854775807LL);
  CHECK(buffer == "9223372036854775807");

  buffer = jau::format_string("%lu", 100000L);
  CHECK(buffer == "100000");

  buffer = jau::format_string("%lu", 0xFFFFFFFFL);
  CHECK(buffer == "4294967295");

  buffer = jau::format_string("%llu", 281474976710656LLU);
  CHECK(buffer == "281474976710656");

  buffer = jau::format_string("%llu", 18446744073709551615LLU);
  CHECK(buffer == "18446744073709551615");

  buffer = jau::format_string("%zu", 2147483647UL);
  CHECK(buffer == "2147483647");

  buffer = jau::format_string("%zd", 2147483647L);
  CHECK(buffer == "2147483647");

  // failed intentionally unsigned -> signed
  static_assert(0 < jau::cfmt::checkLine("%zd", 2147483647UL));
  // buffer = jau::format_string("%zd", 2147483647UL);
  // CHECK(buffer == "2147483647");

  if (sizeof(size_t) == sizeof(long)) {
    buffer = jau::format_string("%zi", -2147483647L);
    CHECK(buffer == "-2147483647");
  }
  else {
    buffer = jau::format_string("%zi", -2147483647LL);
    CHECK(buffer == "-2147483647");
  }

  buffer = jau::format_string("%b", 60000);
  CHECK(buffer == "1110101001100000");

  buffer = jau::format_string("%lb", 12345678L);
  CHECK(buffer == "101111000110000101001110");

  buffer = jau::format_string("%o", 60000);
  CHECK(buffer == "165140");

  buffer = jau::format_string("%lo", 12345678L);
  CHECK(buffer == "57060516");

  buffer = jau::format_string("%lx", 0x12345678L);
  CHECK(buffer == "12345678");

  buffer = jau::format_string("%llx", 0x1234567891234567LLU);
  CHECK(buffer == "1234567891234567");

  buffer = jau::format_string("%lx", 0xabcdefabL);
  CHECK(buffer == "abcdefab");

  buffer = jau::format_string("%lX", 0xabcdefabL);
  CHECK(buffer == "ABCDEFAB");

  buffer = jau::format_string("%c", 'v');
  CHECK(buffer == "v");

  buffer = jau::format_string("%cv", 'w');
  CHECK(buffer == "wv");

  buffer = jau::format_string("%s", "A Test");
  CHECK(buffer == "A Test");

  static_assert(0  < jau::cfmt::checkLine("%hhu", 0xFFU)); // size unsigned int > unsigned char (intentional failure)
  static_assert(0 == jau::cfmt::checkLine("%hhu", 0xFF_u8));
  buffer = jau::format_string("%hhu", 0xFF_u8);
  CHECK(buffer == "255");

  // intentionally fails: given arg size > hh char
  static_assert(0 < jau::cfmt::checkLine("%hhu", 0xFFFFUL)); // size unsigned long > unsigned char (intentional failure)
  // buffer = jau::format_string("%hhu", 0xFFFFUL);
  // CHECK(buffer == "255");

  static_assert(0  < jau::cfmt::checkLine("%hu", 0x123456UL)); // size unsigned long > unsigned short (intentional failure)
  static_assert(0 == jau::cfmt::checkLine("%hu", 0x1234_u16));
  buffer = jau::format_string("%hu", 0x1234_u16); // size unsigned long > unsigned short
  CHECK(buffer == "4660");

  static_assert(0  < jau::cfmt::checkLine("%s%hhi %hu", "Test", 10000, 0xFFFFFFFF));
  static_assert(0 == jau::cfmt::checkLine("%s%hhi %hu", "Test", 16_i8, 0xFFFF_u16));
  buffer = jau::format_string("%s%hhi %hu", "Test", (char)16, (unsigned short)0xFFFF);
  CHECK(buffer == "Test16 65535");

  buffer = jau::format_string("%tx", &buffer[10] - &buffer[0]);
  CHECK(buffer == "a");

// TBD
  if (sizeof(intmax_t) == sizeof(long)) {
    buffer = jau::format_string("%ji", -2147483647L);
    CHECK(buffer == "-2147483647");
  }
  else {
    buffer = jau::format_string("%ji", -2147483647LL);
    CHECK(buffer == "-2147483647");
  }
}


TEST_CASE("pointer", "[jau][std::string][jau::cfmt][pointer]" ) {
  std::string buffer;

#if 0
  buffer = jau::format_string("%p", (void*)0x1234U);
  if (sizeof(void*) == 4U) {
    CHECK(buffer == "00001234");
  }
  else {
    CHECK(buffer == "0000000000001234");
  }

  buffer = jau::format_string("%p", (void*)0x12345678U);
  if (sizeof(void*) == 4U) {
    CHECK(buffer == "12345678");
  }
  else {
    CHECK(buffer == "0000000012345678");
  }

  buffer = jau::format_string("%p-%p", (void*)0x12345678U, (void*)0x7EDCBA98U);
  if (sizeof(void*) == 4U) {
    CHECK(buffer == "12345678-7EDCBA98");
  }
  else {
    CHECK(buffer == "0000000012345678-000000007EDCBA98");
  }

  if (sizeof(uintptr_t) == sizeof(uint64_t)) {
    buffer = jau::format_string("%p", (void*)(uintptr_t)0xFFFFFFFFU); // NOLINT
    CHECK(buffer == "00000000FFFFFFFF");
  }
  else {
    buffer = jau::format_string("%p", (void*)(uintptr_t)0xFFFFFFFFU); // NOLINT
    CHECK(buffer == "FFFFFFFF");
  }
#else
  // %#x or %#lx
  buffer = jau::format_string("%p", (void*)0x1234U);
  CHECK(buffer == "0x1234");

  buffer = jau::format_string("%p", (void*)0x12345678U);
  CHECK(buffer == "0x12345678");

  buffer = jau::format_string("%p-%p", (void*)0x12345678U, (void*)0x7EDCBA98U);
  CHECK(buffer == "0x12345678-0x7edcba98");

  buffer = jau::format_string("%p", (void*)(uintptr_t)0xFFFFFFFFU); // NOLINT
  CHECK(buffer == "0xffffffff");
#endif
}


TEST_CASE("unknown flag", "[jau][std::string][jau::cfmt][error]" ) {
  std::string buffer;

  // we inject an error message
  buffer = jau::format_string("%kmarco", 42, 37); // orig "kmarco"
  const size_t q = buffer.find("<E#", 0);
  CHECK( q != std::string::npos );
}


TEST_CASE("string length", "[jau][std::string][jau::cfmt][stringlen]" ) {
  std::string buffer;

  buffer = jau::format_string("%.4s", "This is a test");
  CHECK(buffer == "This");

  buffer = jau::format_string("%.4s", "test");
  CHECK(buffer == "test");

  buffer = jau::format_string("%.7s", "123");
  CHECK(buffer == "123");

  buffer = jau::format_string("%.7s", "");
  CHECK(buffer == "");

  buffer = jau::format_string("%.4s%.2s", "123456", "abcdef");
  CHECK(buffer == "1234ab");

  // we inject an error message
  buffer = jau::format_string("%.4.2s", "123456"); // orig ".2s"
  const size_t q = buffer.find("<E#", 0);
  CHECK( q != std::string::npos );

  buffer = jau::format_string("%.*s", 3, "123456");
  CHECK(buffer == "123");
}

TEST_CASE("misc", "[jau][std::string][jau::cfmt][misc]" ) {
  std::string buffer;

  buffer = jau::format_string("%u%u%ctest%d %s", 5, 3000, 'a', -20, "bit");
  CHECK(buffer == "53000atest-20 bit");

  buffer = jau::format_string("%.*f", 2, 0.33333333);
  CHECK(buffer == "0.33");

  buffer = jau::format_string("%.*d", -1, 1);
  CHECK(buffer == "1");

  buffer = jau::format_string("%.3s", "foobar");
  CHECK(buffer == "foo");

  buffer = jau::format_string("% .0d", 0);
  CHECK(buffer == " ");

  buffer = jau::format_string("%10.5d", 4);
  CHECK(buffer == "     00004");

  buffer = jau::format_string("%*sx", -3, "hi");
  CHECK(buffer == "hi x");

  buffer = jau::format_string("%.*g", 2, 0.33333333);
  CHECK(buffer == "0.33");

  buffer = jau::format_string("%.*e", 2, 0.33333333);
  CHECK(buffer == "3.33e-01");
}
