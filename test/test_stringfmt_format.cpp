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
#include "jau/ordered_atomic.hpp"

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

#ifdef HAS_STD_FORMAT
    #include <format>
#endif

using namespace std::literals;

using namespace jau::float_literals;

using namespace jau::int_literals;

class SomeClass1 { // NOLINT(misc-use-internal-linkage): intend
  public:
    std::string toString() const { return "SomeClass1 toString"; }
};
class SomeClass2 { // NOLINT(misc-use-internal-linkage): intend
  public:
    std::string_view to_string() const { return "SomeClass2 toString"; }
};
class SomeClass3 { // NOLINT(misc-use-internal-linkage): intend
  public:
};
inline std::string_view to_string(const SomeClass3&) { return "SomeClass3 toString"; } // NOLINT(misc-use-internal-linkage): intend

enum class game_t : uint16_t { // NOLINT(misc-use-internal-linkage): intend
    none,
    chess,
    pacman,
    mrdo
};
JAU_MAKE_ENUM_STRING(game_t, chess, pacman, mrdo); // NOLINT

enum class plain_scoped_unsigned_enum_t : unsigned { // NOLINT(misc-use-internal-linkage): intend
    none,
    one = 1,
    two = 2
};
enum class plain_scoped_signed_enum_t : signed { // NOLINT(misc-use-internal-linkage): intend
    none,
    minus_one = -1,
    one = 1,
    two = 2
};

TEST_CASE("jau::cfmt::cspec_t from type", "[jau][jau::cfmt]") {
    static_assert(jau::cfmt::cspec_t::unsigned_int      == jau::cfmt::to_cspec<unsigned char>());
    static_assert(jau::cfmt::cspec_t::character         == jau::cfmt::to_cspec<char>());
    static_assert(jau::cfmt::cspec_t::string            == jau::cfmt::to_cspec<bool>());
    static_assert(jau::cfmt::cspec_t::unsigned_int      == jau::cfmt::to_cspec<unsigned short>());
    static_assert(jau::cfmt::cspec_t::signed_int        == jau::cfmt::to_cspec<short>());
    static_assert(jau::cfmt::cspec_t::unsigned_int      == jau::cfmt::to_cspec<unsigned int>());
    static_assert(jau::cfmt::cspec_t::signed_int        == jau::cfmt::to_cspec<int>());
    static_assert(jau::cfmt::cspec_t::unsigned_int      == jau::cfmt::to_cspec<unsigned long>());
    static_assert(jau::cfmt::cspec_t::signed_int        == jau::cfmt::to_cspec<long>());
    static_assert(jau::cfmt::cspec_t::floating_point    == jau::cfmt::to_cspec<float>());
    static_assert(jau::cfmt::cspec_t::floating_point    == jau::cfmt::to_cspec<double>());
    static_assert(jau::cfmt::cspec_t::floating_point    == jau::cfmt::to_cspec<jau::float32_t>());
    static_assert(jau::cfmt::cspec_t::floating_point    == jau::cfmt::to_cspec<jau::float64_t>());

    static_assert(jau::cfmt::cspec_t::string            == jau::cfmt::to_cspec<decltype("Hello")>());
    static_assert(jau::cfmt::cspec_t::string            == jau::cfmt::to_cspec<std::string>());
    static_assert(jau::cfmt::cspec_t::string            == jau::cfmt::to_cspec<std::string_view>());

    static_assert(jau::cfmt::cspec_t::string            == jau::cfmt::to_cspec<SomeClass1>());
    static_assert(jau::cfmt::cspec_t::string            == jau::cfmt::to_cspec<game_t>());
    static_assert(jau::cfmt::cspec_t::unsigned_int      == jau::cfmt::to_cspec<plain_scoped_unsigned_enum_t>());
    static_assert(jau::cfmt::cspec_t::signed_int        == jau::cfmt::to_cspec<plain_scoped_signed_enum_t>());

    enum plain_unscoped_unsigned_enum_t : unsigned { // NOLINT(misc-use-internal-linkage): intend
        aa_none,
        aa_one = 1,
        aa_two = 2
    };
    enum plain_unscoped_signed_enum_t : signed { // NOLINT(misc-use-internal-linkage): intend
        bb_none,
        bb_minus_one = -1,
        bb_one = 1,
        bb_two = 2
    };
    static_assert(jau::cfmt::cspec_t::unsigned_int  == jau::cfmt::to_cspec<plain_unscoped_unsigned_enum_t>());
    static_assert(jau::cfmt::cspec_t::signed_int    == jau::cfmt::to_cspec<plain_unscoped_signed_enum_t>());
}

TEST_CASE("jau::cfmt::FormatOpts with auto", "[jau][jau::cfmt]") {
    {
        jau::cfmt::FormatOpts opts;
        opts.addFlag('\'');
        opts.setWidth(31);
        opts.setPrecision(27);
        opts.setConversion('u');
        std::cout << "opts-1: " << opts << "\n";
        REQUIRE( opts.conversion == jau::cfmt::cspec_t::unsigned_int);
#ifdef JAU_CFMT_TRACK_FORMAT_OPTS_LEN
        REQUIRE( opts.length_mod == jau::cfmt::plength_t::z);
#endif
    }
    {
        jau::cfmt::FormatOpts opts;
        opts.addFlag('\'');
        opts.setWidth(31);
        opts.setPrecision(27);
        opts.setConversion('?');
        std::cout << "opts-2.1: " << opts << "\n";
        REQUIRE( opts.conversion == jau::cfmt::cspec_t::any);
#ifdef JAU_CFMT_TRACK_FORMAT_OPTS_LEN
        REQUIRE( opts.length_mod == jau::cfmt::plength_t::z);
#endif
    }
    //
    // formatR
    //
    {
        using namespace jau::cfmt;
        std::string s;
        jau::cfmt::Result r = jau::cfmt::formatR(s,"%" PRIi64, (int64_t)1);
        std::cerr << "FormatResult " << r << "\n";
        std::cerr << "FormatString '" << s << "'\n";
        REQUIRE( true == r.success());
        REQUIRE( 1 == r.argumentCount());
        REQUIRE( flags_t::none == r.opts().flags);
        REQUIRE( false == r.opts().width_set);
        REQUIRE( 0 == r.opts().width);
        REQUIRE( false == r.opts().precision_set);
        REQUIRE( 0 == r.opts().precision);
        REQUIRE( jau::cfmt::cspec_t::signed_int == r.opts().conversion);
#ifdef JAU_CFMT_TRACK_FORMAT_OPTS_LEN
        REQUIRE( jau::cfmt::plength_t::l        <= r.opts().length_mod);
#endif
        REQUIRE( "1" == s);
    }
    {
        using namespace jau::cfmt;
        std::string s;
        jau::cfmt::Result r = jau::cfmt::formatR(s,"%?", (int64_t)1);
        std::cerr << "FormatResult " << r << "\n";
        std::cerr << "FormatString '" << s << "'\n";
        REQUIRE( true == r.success());
        REQUIRE( 1 == r.argumentCount());
        REQUIRE( flags_t::none == r.opts().flags);
        REQUIRE( false == r.opts().width_set);
        REQUIRE( 0 == r.opts().width);
        REQUIRE( false == r.opts().precision_set);
        REQUIRE( 0 == r.opts().precision);
        REQUIRE( jau::cfmt::cspec_t::signed_int == r.opts().conversion);
#ifdef JAU_CFMT_TRACK_FORMAT_OPTS_LEN
        REQUIRE( jau::cfmt::plength_t::any        == r.opts().length_mod);
#endif
        REQUIRE( "1" == s);
    }
    {
        using namespace jau::cfmt;
        char v=65;
        static_assert(jau::cfmt::cspec_t::character == jau::cfmt::to_cspec<decltype(v)>());

        std::string s;
        jau::cfmt::Result r = jau::cfmt::formatR(s,"%c", v);
        std::cerr << "FormatResult " << r << "\n";
        std::cerr << "FormatString '" << s << "'\n";
        REQUIRE( true == r.success());
        REQUIRE( 1 == r.argumentCount());
        REQUIRE( flags_t::none == r.opts().flags);
        REQUIRE( false == r.opts().width_set);
        REQUIRE( 0 == r.opts().width);
        REQUIRE( false == r.opts().precision_set);
        REQUIRE( 0 == r.opts().precision);
        REQUIRE( jau::cfmt::cspec_t::character == r.opts().conversion);
#ifdef JAU_CFMT_TRACK_FORMAT_OPTS_LEN
        REQUIRE( jau::cfmt::plength_t::none == r.opts().length_mod);
#endif
        REQUIRE( "A" == s);
    }
    {
        using namespace jau::cfmt;
        char v=65;
        static_assert(jau::cfmt::cspec_t::character == jau::cfmt::to_cspec<decltype(v)>());

        std::string s;
        jau::cfmt::Result r = jau::cfmt::formatR(s,"%?", v);
        std::cerr << "FormatResult " << r << "\n";
        std::cerr << "FormatString '" << s << "'\n";
        REQUIRE( true == r.success());
        REQUIRE( 1 == r.argumentCount());
        REQUIRE( flags_t::none == r.opts().flags);
        REQUIRE( false == r.opts().width_set);
        REQUIRE( 0 == r.opts().width);
        REQUIRE( false == r.opts().precision_set);
        REQUIRE( 0 == r.opts().precision);
        REQUIRE( jau::cfmt::cspec_t::character == r.opts().conversion);
#ifdef JAU_CFMT_TRACK_FORMAT_OPTS_LEN
        REQUIRE( jau::cfmt::plength_t::any == r.opts().length_mod);
#endif
        REQUIRE( "A" == s);
    }
    {
        using namespace jau::cfmt;
        unsigned char v=1;
        static_assert(jau::cfmt::cspec_t::unsigned_int == jau::cfmt::to_cspec<decltype(v)>());

        std::string s;
        jau::cfmt::Result r = jau::cfmt::formatR(s,"%?", v);
        std::cerr << "FormatResult " << r << "\n";
        std::cerr << "FormatString '" << s << "'\n";
        REQUIRE( true == r.success());
        REQUIRE( 1 == r.argumentCount());
        REQUIRE( flags_t::none == r.opts().flags);
        REQUIRE( false == r.opts().width_set);
        REQUIRE( 0 == r.opts().width);
        REQUIRE( false == r.opts().precision_set);
        REQUIRE( 0 == r.opts().precision);
        REQUIRE( jau::cfmt::cspec_t::unsigned_int == r.opts().conversion);
#ifdef JAU_CFMT_TRACK_FORMAT_OPTS_LEN
        REQUIRE( jau::cfmt::plength_t::any == r.opts().length_mod);
#endif
        REQUIRE( "1" == s);
    }
    {
        using namespace jau::cfmt;
        int v=65;
        static_assert(jau::cfmt::cspec_t::signed_int == jau::cfmt::to_cspec<decltype(v)>());

        std::string s;
        jau::cfmt::Result r = jau::cfmt::formatR(s,"%2.3d", v);
        std::cerr << "FormatResult " << r << "\n";
        std::cerr << "FormatString '" << s << "'\n";
        REQUIRE( true == r.success());
        REQUIRE( 1 == r.argumentCount());
        REQUIRE( flags_t::none == r.opts().flags);
        REQUIRE( true == r.opts().width_set);
        REQUIRE( 2 == r.opts().width);
        REQUIRE( true == r.opts().precision_set);
        REQUIRE( 3 == r.opts().precision);
        REQUIRE( jau::cfmt::cspec_t::signed_int == r.opts().conversion);
#ifdef JAU_CFMT_TRACK_FORMAT_OPTS_LEN
        REQUIRE( jau::cfmt::plength_t::none == r.opts().length_mod);
#endif
        REQUIRE( "065" == s);
    }
    {
        using namespace jau::cfmt;
        float v=12.34;
        static_assert(jau::cfmt::cspec_t::floating_point == jau::cfmt::to_cspec<decltype(v)>());

        std::string s;
        jau::cfmt::Result r = jau::cfmt::formatR(s,"%2.3f", v);
        std::cerr << "FormatResult " << r << "\n";
        std::cerr << "FormatString '" << s << "'\n";
        REQUIRE( true == r.success());
        REQUIRE( 1 == r.argumentCount());
        REQUIRE( flags_t::none == r.opts().flags);
        REQUIRE( true == r.opts().width_set);
        REQUIRE( 2 == r.opts().width);
        REQUIRE( true == r.opts().precision_set);
        REQUIRE( 3 == r.opts().precision);
        REQUIRE( jau::cfmt::cspec_t::floating_point == r.opts().conversion);
#ifdef JAU_CFMT_TRACK_FORMAT_OPTS_LEN
        REQUIRE( jau::cfmt::plength_t::none == r.opts().length_mod);
#endif
        REQUIRE( "12.340" == s);
    }
#if 0
    {
    short i3=-3;
    unsigned short i4=4;

    int i5=-5;
    unsigned int i6=6;

    long i7=-7;
    unsigned long i8=8;

    ssize_t i9 = -9;
    size_t i10 = 10;
    }
#endif
}

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
        REQUIRE( jau::cfmt::cspec_t::signed_int == r.opts().conversion);
#ifdef JAU_CFMT_TRACK_FORMAT_OPTS_LEN
        REQUIRE( jau::cfmt::plength_t::l == r.opts().length_mod);
#endif
    }
    {
        std::string s;
        jau::cfmt::Result r = jau::cfmt::formatR(s,"%23i", (int64_t)1);
        std::cerr << "FormatResult " << r << "\n";
        REQUIRE( true == r.success());
        REQUIRE( 1 == r.argumentCount());
        REQUIRE( flags_t::none == r.opts().flags);
        REQUIRE( true == r.opts().width_set);
        REQUIRE( 23 == r.opts().width);
        REQUIRE( false == r.opts().precision_set);
        REQUIRE( 0 == r.opts().precision);
        REQUIRE( jau::cfmt::cspec_t::signed_int == r.opts().conversion);
#ifdef JAU_CFMT_TRACK_FORMAT_OPTS_LEN
        REQUIRE( jau::cfmt::plength_t::none == r.opts().length_mod);
#endif
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
        REQUIRE( jau::cfmt::cspec_t::signed_int == r.opts().conversion);
#ifdef JAU_CFMT_TRACK_FORMAT_OPTS_LEN
        REQUIRE( jau::cfmt::plength_t::l == r.opts().length_mod);
#endif
    }
    {
        std::string s;
        jau::cfmt::Result r = jau::cfmt::formatR(s,"%23.12i", (int64_t)1);
        std::cerr << "FormatResult " << r << "\n";
        REQUIRE( true == r.success());
        REQUIRE( 1 == r.argumentCount());
        REQUIRE( flags_t::none == r.opts().flags);
        REQUIRE( true == r.opts().width_set);
        REQUIRE( 23 == r.opts().width);
        REQUIRE( true == r.opts().precision_set);
        REQUIRE( 12 == r.opts().precision);
        REQUIRE( jau::cfmt::cspec_t::signed_int == r.opts().conversion);
#ifdef JAU_CFMT_TRACK_FORMAT_OPTS_LEN
        REQUIRE( jau::cfmt::plength_t::none == r.opts().length_mod);
#endif
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
        REQUIRE( jau::cfmt::cspec_t::signed_int == r.opts().conversion);
#ifdef JAU_CFMT_TRACK_FORMAT_OPTS_LEN
        REQUIRE( jau::cfmt::plength_t::l == r.opts().length_mod);
#endif
    }
}

TEST_CASE("parse: width precision from arg", "[jau][std::string][jau::cfmt]") {
    using namespace jau::cfmt;

    {
        jau_format_check("%*" PRIi64, 21, (int64_t)1);
        jau_format_checkLine("%*" PRIi64, 21, (int64_t)1);
        std::string s;
        jau::cfmt::Result r = jau::cfmt::formatR(s,"%*i", 21, (int64_t)1);
        std::cerr << "FormatResult " << r << "\n";
        REQUIRE( true == r.success());
        REQUIRE( 2 == r.argumentCount());
        REQUIRE( flags_t::none == r.opts().flags);
        REQUIRE( true == r.opts().width_set);
        REQUIRE( 21 == r.opts().width);
        REQUIRE( false == r.opts().precision_set);
        REQUIRE( 0 == r.opts().precision);
        REQUIRE( jau::cfmt::cspec_t::signed_int == r.opts().conversion);
#ifdef JAU_CFMT_TRACK_FORMAT_OPTS_LEN
        REQUIRE( jau::cfmt::plength_t::none == r.opts().length_mod);
#endif
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
        REQUIRE( jau::cfmt::cspec_t::signed_int == r.opts().conversion);
#ifdef JAU_CFMT_TRACK_FORMAT_OPTS_LEN
        REQUIRE( jau::cfmt::plength_t::l == r.opts().length_mod);
#endif
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
        REQUIRE( jau::cfmt::cspec_t::signed_int == r.opts().conversion);
#ifdef JAU_CFMT_TRACK_FORMAT_OPTS_LEN
        REQUIRE( jau::cfmt::plength_t::l == r.opts().length_mod);
#endif
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
        REQUIRE( jau::cfmt::cspec_t::signed_int == r.opts().conversion);
#ifdef JAU_CFMT_TRACK_FORMAT_OPTS_LEN
        REQUIRE( jau::cfmt::plength_t::l == r.opts().length_mod);
#endif
    }
    {
        std::string s;
        jau::cfmt::Result r = jau::cfmt::formatR(s,"%+.*d", 12, (int64_t)1);
        std::cerr << "FormatResult " << r << "\n";
        REQUIRE( true == r.success());
        REQUIRE( 2 == r.argumentCount());
        REQUIRE( flags_t::plus == r.opts().flags);
        REQUIRE( false == r.opts().width_set);
        REQUIRE( 0 == r.opts().width);
        REQUIRE( true == r.opts().precision_set);
        REQUIRE( 12 == r.opts().precision);
        REQUIRE( jau::cfmt::cspec_t::signed_int == r.opts().conversion);
#ifdef JAU_CFMT_TRACK_FORMAT_OPTS_LEN
        REQUIRE( jau::cfmt::plength_t::none == r.opts().length_mod);
#endif
    }
}

template <typename... Args>
static void checkFormat(int line, const char *fmt, const Args &...args) {
    PRAGMA_DISABLE_WARNING_PUSH
    PRAGMA_DISABLE_WARNING_FORMAT_NONLITERAL
    PRAGMA_DISABLE_WARNING_FORMAT_SECURITY
    std::string exp = jau::unsafe::format_string(fmt, args...);
    PRAGMA_DISABLE_WARNING_POP

    // std::string has = jau::cfmt::format(fmt, args...);
    std::string has;
    jau::cfmt::Result r = jau::cfmt::formatR(has, fmt, args...);
    std::cerr << "FormatResult @ " << line << ": " << r << "\n";
    std::cerr << "FormatResult @ " << line << ": exp `" << exp << "`, has `" << has << "`\n\n";
    CHECK( true == r.success());
    CHECK( sizeof...(args) == r.argumentCount());
    CHECK(exp == has);
}

template <typename ArgStdPrint, typename ArgJauPrint>
static void checkFormat2(int line, const char *fmt, const ArgStdPrint &argStd, const ArgJauPrint &argJau) {
    PRAGMA_DISABLE_WARNING_PUSH
    PRAGMA_DISABLE_WARNING_FORMAT_NONLITERAL
    PRAGMA_DISABLE_WARNING_FORMAT_SECURITY
    std::string exp = jau::unsafe::format_string(fmt, argStd);
    PRAGMA_DISABLE_WARNING_POP

    // std::string has = jau::cfmt::format(fmt, args...);
    std::string has;
    jau::cfmt::Result r = jau::cfmt::formatR(has, fmt, argJau);
    std::cerr << "FormatResult @ " << line << ": " << r << "\n";
    std::cerr << "FormatResult @ " << line << ": exp `" << exp << "`, has `" << has << "`\n\n";
    CHECK( true == r.success());
    CHECK( 1 == r.argumentCount());
    CHECK(exp == has);
}

TEST_CASE("single_conversion", "[jau][std::string][jau::cfmt]") {
    // type conversion
    bool b0 = false;
    bool b1 = true;
    int32_t  i32 = -1234;
    int32_t  i32_u = 1234;
    uint32_t u32 =  1234;
    float    f32 = 123.45f; // 42.14f;
    double   f64 = 123.45; // 42.1456;
    jau::float32_t f32_2 = 123.45f; // 42.14f;
    jau::float64_t f64_2 = 123.45; // 42.1456;
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
        jau_format_checkLine("%s", s2);
        CHECK( 1 == jau::cfmt::check("%s", s2));
        CHECK( "World" == jau::cfmt::format("%s", s2));
        jau_format_checkLine("%?", s2);
        CHECK( 1 == jau::cfmt::check("%?", s2));
        CHECK( "World" == jau::cfmt::format("%?", s2));

        jau_format_checkLine("%s", s2sv);
        CHECK( 1 == jau::cfmt::check("%s", s2sv));
        CHECK( "World" == jau::cfmt::format("%s", s2sv));
        jau_format_checkLine("%?", s2sv);
        CHECK( 1 == jau::cfmt::check("%?", s2sv));
        CHECK( "World" == jau::cfmt::format("%?", s2sv));
    }
    {
        // jau_format_checkLine("%s", (int)0);
        CHECK( -1 == jau::cfmt::check("%s", (int)0));
    }
    {
        const char *cstr0 = nullptr;
        const char *cstr1 = "Hello World";
        jau_format_checkLine("%s", cstr0);
        CHECK( "(null)" == jau::cfmt::format("%s", cstr0));
        CHECK( "Hello World" == jau::cfmt::format("%s", cstr1));
        jau_format_checkLine("%?", cstr0);
        CHECK( "(null)" == jau::cfmt::format("%?", cstr0));
        CHECK( "Hello World" == jau::cfmt::format("%?", cstr1));
    }
    {
        const void *handle = (void *)0x12345678;
        const void *nil = nullptr;
        jau_format_checkLine("%p", handle);
        CHECK( "0x12345678" == jau::cfmt::format("%p", handle));
        CHECK( "(nil)" == jau::cfmt::format("%p", nil));
        jau_format_checkLine("%?", handle);
        CHECK( "0x12345678" == jau::cfmt::format("%?", handle));
        CHECK( "(nil)" == jau::cfmt::format("%?", nil));
        jau_format_checkLine("%#p", handle);
        CHECK( "0x12345678" == jau::cfmt::format("%#p", handle));
        CHECK( "(nil)" == jau::cfmt::format("%#p", nil));

        // only `char*` for string allowed
        CHECK( -1 == jau::cfmt::check("%s", handle));
        CHECK( true == jau::cfmt::format("%s", handle).starts_with("<E#1"));
        CHECK( true == jau::cfmt::format("%s", nil).starts_with("<E#1"));
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

    checkFormat(__LINE__, "%d", b0);
    checkFormat(__LINE__, "%d", b1);
    checkFormat(__LINE__, "%u", b0);
    checkFormat(__LINE__, "%u", b1);
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
    checkFormat2(__LINE__, "%f", f64, f64_2);
    checkFormat2(__LINE__, "%e", f64, f64_2);
    checkFormat2(__LINE__, "%E", f64, f64_2);
    checkFormat2(__LINE__, "%a", f64, f64_2);
    checkFormat2(__LINE__, "%A", f64, f64_2);
    // checkFormat2(__LINE__, "%g", f64, f64_2);
    // checkFormat2(__LINE__, "%G", f64, f64_2);

    checkFormat(__LINE__, "%f", f32);
    checkFormat(__LINE__, "%e", f32);
    checkFormat(__LINE__, "%E", f32);
    checkFormat(__LINE__, "%a", f32);
    checkFormat(__LINE__, "%A", f32);
    // checkFormat(__LINE__, "%g", f32);
    // checkFormat(__LINE__, "%G", f32);
    checkFormat2(__LINE__, "%f", f32, f32_2);
    checkFormat2(__LINE__, "%e", f32, f32_2);
    checkFormat2(__LINE__, "%E", f32, f32_2);
    checkFormat2(__LINE__, "%a", f32, f32_2);
    checkFormat2(__LINE__, "%A", f32, f32_2);
    // checkFormat2(__LINE__, "%g", f32, f32_2);
    // checkFormat2(__LINE__, "%G", f32, f32_2);

    checkFormat(__LINE__, "%dZZZ", i32);
    checkFormat(__LINE__, "%dZZ", i32);
    checkFormat(__LINE__, "%dZ", i32);
    checkFormat(__LINE__, "Z%dZ Z%dZ", i32, i32);
    checkFormat(__LINE__, "Z%-6dZ Z%6dZ", i32, i32);

    checkFormat(__LINE__, "%#020x", 305441741);
    checkFormat(__LINE__, "%zd", 2147483647L);

    #if !JAU_CFMT_IGNORE_LENGTH_MODIFIER
        static_assert(0 < jau::cfmt::checkLine("%zd", 2147483647UL)); // failed intentionally unsigned -> signed
    #endif
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

        jau_format_check("Value end not '%?' @ idx %?, %? != %?, in: %? '%?' len %?", limiter, endptr - str, endptr, limiter_pos, str, str, str_len);
    }
    // bool
    {
        jau_format_check("%?", b0);
        jau_format_checkLine("%?", b0);
        jau_format_check("%d", b0);
        jau_format_checkLine("%d", b0);
        jau_format_check("%u", b0);
        jau_format_checkLine("%u", b0);
        jau_format_check("%s", b0);
        jau_format_checkLine("%s", b0);
        CHECK("0" == jau::cfmt::format("%d", b0));
        CHECK("1" == jau::cfmt::format("%d", b1));
        CHECK("0" == jau::cfmt::format("%u", b0));
        CHECK("1" == jau::cfmt::format("%u", b1));
        CHECK("false" == jau::cfmt::format("%s", b0));
        CHECK("true" == jau::cfmt::format("%s", b1));
        CHECK("false" == jau::cfmt::format("%?", b0));
        CHECK("true" == jau::cfmt::format("%?", b1));
    }

    // enums: integral value
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
        static_assert(jau::cfmt::cspec_t::unsigned_int == jau::cfmt::to_cspec<decltype(e1_u)>());

        static_assert(true == std::is_enum_v<decltype(e2_s)>);
        static_assert(true == std::is_signed_v<std::underlying_type_t<decltype(e2_s)>>);
        static_assert(jau::cfmt::cspec_t::signed_int == jau::cfmt::to_cspec<decltype(e2_s)>());

        static_assert(true == std::is_enum_v<decltype(e3_s)>);
        static_assert(true == std::is_signed_v<std::underlying_type_t<decltype(e3_s)>>);
        static_assert(jau::cfmt::cspec_t::signed_int == jau::cfmt::to_cspec<decltype(e3_s)>());

        static_assert(true == std::is_enum_v<decltype(e4_u)>);
        static_assert(true == std::is_unsigned_v<std::underlying_type_t<decltype(e4_u)>>);
        static_assert(jau::cfmt::cspec_t::unsigned_int == jau::cfmt::to_cspec<decltype(e4_u)>());

        static_assert(4 == jau::cfmt::check("%u, %d, %d, %u", e1_u, e2_s, e3_s, e4_u));
        static_assert(0 == jau::cfmt::checkLine("%u, %u, %d, %u", e1_u, e2_s, e3_s, e4_u));
        static_assert(4 == jau::cfmt::check("%?, %?, %?, %?", e1_u, e2_s, e3_s, e4_u));
        static_assert(0 == jau::cfmt::checkLine("%?, %?, %?, %?", e1_u, e2_s, e3_s, e4_u));

        static_assert(0 == jau::cfmt::checkLine("%u", e1_u)); // unsigned -> unsigned OK
        // !JAU_CFMT_IGNORE_LENGTH_MODIFIER
        // static_assert(0 < jau::cfmt::checkLine("%d", e1_u));  // unsigned -> signed ERROR
        static_assert(0 == jau::cfmt::checkLine("%u", e2_s)); // signed -> unsigned OK

        jau_format_checkLine("%u, %d, %d, %u\n", e1_u, e2_s, e3_s, e4_u);
        CHECK("Enum 0, -1, -1, 0" == jau::cfmt::format("Enum %u, %d, %d, %u", e1_u, e2_s, e3_s, e4_u));
        jau_format_checkLine("%?, %?, %?, %?\n", e1_u, e2_s, e3_s, e4_u);
        CHECK("Enum 0, -1, -1, 0" == jau::cfmt::format("Enum %?, %?, %?, %?", e1_u, e2_s, e3_s, e4_u));

        jau_format_checkLine("%u, %d, %d, %u\n",
            plain_scoped_unsigned_enum_t::none,
            plain_scoped_signed_enum_t::minus_one, plain_scoped_signed_enum_t::one,
            plain_scoped_unsigned_enum_t::one);
        CHECK("Enum 0, -1, 1, 2" == jau::cfmt::format("Enum %u, %d, %d, %u",
            plain_scoped_unsigned_enum_t::none,
            plain_scoped_signed_enum_t::minus_one, plain_scoped_signed_enum_t::one,
            plain_scoped_unsigned_enum_t::two));
        CHECK("Enum 0, -1, 1, 2" == jau::cfmt::format("Enum %?, %?, %?, %?",
            plain_scoped_unsigned_enum_t::none,
            plain_scoped_signed_enum_t::minus_one, plain_scoped_signed_enum_t::one,
            plain_scoped_unsigned_enum_t::two));

        CHECK("jau::to_string<T> n/a for type" == jau::to_string(plain_scoped_unsigned_enum_t::one).substr(0, 30)); // no to_string available
        CHECK( -1 == jau::cfmt::check("%s", plain_scoped_unsigned_enum_t::one));    // no to_string available
    }
    // enums: string value
    {
        CHECK("chess" == name(game_t::chess));
        CHECK("chess" == to_string(game_t::chess));
        CHECK("chess" == jau::to_string(game_t::chess));
        CHECK("pacman" == jau::to_string(game_t::pacman));
        jau_format_checkLine("%s", game_t::chess);
        jau_format_checkLine("%?", game_t::chess);
        CHECK( "chess" == jau::cfmt::format("%s", game_t::chess));
        CHECK( "pacman" == jau::cfmt::format("%s", game_t::pacman));
        CHECK( "chess" == jau::cfmt::format("%?", game_t::chess));
        CHECK( "pacman" == jau::cfmt::format("%?", game_t::pacman));

        CHECK("little" == jau::to_string(jau::lb_endian_t::little));
        jau_format_checkLine("%s", jau::lb_endian_t::little);
        CHECK( "little" == jau::cfmt::format("%s", jau::lb_endian_t::little));
        jau_format_checkLine("%?", jau::lb_endian_t::little);
        CHECK( "little" == jau::cfmt::format("%?", jau::lb_endian_t::little));
    }
    // class w/ toString to_string and free to_string (SomeClass3)
    {
        CHECK("SomeClass1 toString" == jau::to_string(SomeClass1()));
        jau_format_checkLine("%s", SomeClass1());
        CHECK( "SomeClass1 toString" == jau::cfmt::format("%s", SomeClass1()));

        CHECK("SomeClass2 toString" == SomeClass2().to_string());
        jau_format_checkLine("%s", SomeClass2());
        CHECK("SomeClass2 toString" == jau::cfmt::format("%s", SomeClass2()));

        CHECK("SomeClass3 toString" == to_string(SomeClass3()));
        jau_format_checkLine("%s", SomeClass3());
        CHECK("SomeClass3 toString" == jau::cfmt::format("%s", SomeClass3()));

        SomeClass1 sc1;
        CHECK("SomeClass1 toString" == jau::to_string(sc1));
        CHECK("SomeClass1 toString" == jau::cfmt::format("%s", sc1));
        CHECK("SomeClass1 toString" != jau::cfmt::format("%p", &sc1));

        SomeClass2 sc2;
        CHECK("SomeClass2 toString" == jau::to_string(sc2));
        CHECK("SomeClass2 toString" == jau::cfmt::format("%s", sc2));
        CHECK("SomeClass2 toString" != jau::cfmt::format("%p", &sc2));

        SomeClass3 sc3;
        CHECK("SomeClass3 toString" == jau::to_string(sc3));
        CHECK("SomeClass3 toString" == jau::cfmt::format("%s", sc3));
        CHECK("SomeClass3 toString" != jau::cfmt::format("%p", &sc3));
    }
    // jau::fraction_i64 has both, member toString and free to_string
    {
        jau::fraction_i64 timeout(10, 1);
        jau_format_check("Timeout %ld ms, %s", timeout.to_ms(), timeout);
        CHECK("10/1" == jau::to_string(timeout));
        CHECK("Timeout 10000 ms, 10/1" == jau::cfmt::format("Timeout %ld ms, %s", timeout.to_ms(), timeout));
    }
    // atomic wrapper
    {
        jau::ordered_atomic<SomeClass1, std::memory_order_relaxed> sc_clz1;
        jau::ordered_atomic<SomeClass3, std::memory_order_relaxed> sc_clz3;
        jau::ordered_atomic<uint16_t, std::memory_order_relaxed> sc_u16 = 12;
        jau::ordered_atomic<bool, std::memory_order_relaxed> sc_bool = true;
        CHECK("SomeClass1 toString" == jau::to_string(sc_clz1));
        CHECK("SomeClass3 toString" == jau::to_string(sc_clz3));
        CHECK("12" == jau::to_string(sc_u16));
        CHECK("T" == jau::to_string(sc_bool));
        CHECK("SomeClass1 toString" == jau::cfmt::format("%s", sc_clz1));
        CHECK("SomeClass1 toString, 12, true, SomeClass3 toString" == jau::cfmt::format("%s, %u, %s, %s", sc_clz1, sc_u16, sc_bool, sc_clz3));
    }
    {
        jau::ordered_atomic<SomeClass3, std::memory_order_relaxed> sc_clz3;
        CHECK("SomeClass3 toString" == jau::to_string(sc_clz3));
        CHECK("SomeClass3 toString" == jau::cfmt::format("%s", sc_clz3));
    }
    {
        jau::relaxed_atomic_int sc_int = 11;
        CHECK("11" == jau::to_string(sc_int));
        CHECK("11" == jau::cfmt::format("%d", sc_int));
    }
}

TEST_CASE("integral_conversion", "[jau][std::string][jau::cfmt]") {
    static constexpr const char *format_check_exp1a = "format_check: -1, 2, -3, 4, -5, 6, -7, 8, -9, 10";
    static constexpr const char *format_check_exp1b = "format_check: A, 2, -3, 4, -5, 6, -7, 8, -9, 10";
    static constexpr const char *format_check_exp2a = "format_check: -1, 02, -03, 0004, -0005, 000006, -000007, 00000008, -00000009, 0000000010";
    static constexpr const char *format_check_exp2b = "format_check: A, 02, -03, 0004, -0005, 000006, -000007, 00000008, -00000009, 0000000010";
    char i1=-1;
    char i1b=65;
    unsigned char i2=2;

    short i3=-3;
    unsigned short i4=4;

    int i5=-5;
    unsigned int i6=6;

    long i7=-7;
    unsigned long i8=8;

    ssize_t i9 = -9;
    size_t i10 = 10;

    static_assert(jau::cfmt::cspec_t::character == jau::cfmt::to_cspec<decltype(i1)>());
    static_assert(jau::cfmt::cspec_t::unsigned_int == jau::cfmt::to_cspec<decltype(i2)>());

    jau_format_check("format_check: %?", i1);
    CHECK("format_check: A" == jau::cfmt::format("format_check: %?", i1b));

    jau_format_check("format_check: %hhd, %hhu, %hd, %hu, %d, %u, %ld, %lu, %zd, %zu", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
    CHECK(format_check_exp1a == jau::cfmt::format("format_check: %hhd, %hhu, %hd, %hu, %d, %u, %ld, %lu, %zd, %zu", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10));
    jau_format_check("format_check: %?, %?, %?, %?, %?, %?, %?, %?, %?, %?", i1b, i2, i3, i4, i5, i6, i7, i8, i9, i10);
    CHECK(format_check_exp1b == jau::cfmt::format("format_check: %?, %?, %?, %?, %?, %?, %?, %?, %?, %?", i1b, i2, i3, i4, i5, i6, i7, i8, i9, i10));

    jau_format_check("format_check: %01hhd, %02hhu, %03hd, %04hu, %05d, %06u, %07ld, %08lu, %09zd, %010zu", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
    CHECK(format_check_exp2a == jau::cfmt::format("format_check: %01hhd, %02hhu, %03hd, %04hu, %05d, %06u, %07ld, %08lu, %09zd, %010zu", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10));
    jau_format_check("format_check: %01?, %02?, %03?, %04?, %05?, %06?, %07?, %08?, %09?, %010?", i1b, i2, i3, i4, i5, i6, i7, i8, i9, i10);
    CHECK(format_check_exp2b == jau::cfmt::format("format_check: %01?, %02?, %03?, %04?, %05?, %06?, %07?, %08?, %09?, %010?", i1b, i2, i3, i4, i5, i6, i7, i8, i9, i10));
}

TEST_CASE("thousands_flag", "[jau][std::string][jau::cfmt][flags]" ) {
    //
    // thousand flag ' or ,
    //
    jau_format_checkLine("%'d", 1);
    jau_format_checkLine("%,d", 1);

    CHECK("1" == jau::cfmt::format("%'d", 1));
    CHECK("10" == jau::cfmt::format("%#'d", 10));
    CHECK("100" == jau::cfmt::format("%,d", 100));
    CHECK("1'000" == jau::cfmt::format("%#'d", 1000));
    CHECK("1'000'000" == jau::cfmt::format("%,d", 1000000));
    CHECK("+1'000'000" == jau::cfmt::format("%'+d", 1000000));
    CHECK("+1'000'000" == jau::cfmt::format("%#'+d", 1000000));
    CHECK("-1'000'000" == jau::cfmt::format("%,d", -1000000));
    CHECK("-1'000'000" == jau::cfmt::format("%#'d", -1000000));

    CHECK("1" == jau::cfmt::format("%'?", 1));
    CHECK("10" == jau::cfmt::format("%#'?", 10));
    CHECK("100" == jau::cfmt::format("%,?", 100));
    CHECK("1'000" == jau::cfmt::format("%#'?", 1000));
    CHECK("1'000'000" == jau::cfmt::format("%,?", 1000000));
    CHECK("+1'000'000" == jau::cfmt::format("%'+?", 1000000));
    CHECK("+1'000'000" == jau::cfmt::format("%#'+?", 1000000));
    CHECK("-1'000'000" == jau::cfmt::format("%,?", -1000000));
    CHECK("-1'000'000" == jau::cfmt::format("%#'?", -1000000));

    CHECK("ff" == jau::cfmt::format("%'x", 0xff_u32));
    CHECK("0xff" == jau::cfmt::format("%#'x", 0xff_u32));
    CHECK("ffff" == jau::cfmt::format("%,x", 0xffff_u32));
    CHECK("0x1'ffff" == jau::cfmt::format("%#'x", 0x1ffff_u32));
    CHECK("1'ffff'ffff" == jau::cfmt::format("%,lx", 0x1ffffffff_i64));
    CHECK("0x1'ffff'ffff" == jau::cfmt::format("%#'lx", 0x1ffffffff_u64));
    // negative types not allowed for hex-conversion
    CHECK("255" == jau::cfmt::format("%'?", 0xff_u32));
    CHECK("255" == jau::cfmt::format("%#'?", 0xff_u32));
    CHECK("65'535" == jau::cfmt::format("%,?", 0xffff_u32));
    // negative types not allowed for hex-conversion

    // separator, space-padding
    CHECK(" 876'543" == jau::cfmt::format("%,8d", 876543));
    CHECK("9'876'543" == jau::cfmt::format("%,8d", 9876543));
    CHECK("9'876'543" == jau::cfmt::format("%,9d", 9876543));
    CHECK(" 9'876'543" == jau::cfmt::format("%,10d", 9876543));
    CHECK("    9'876'543" == jau::cfmt::format("%,13d", 9876543));

    CHECK("0xaffe" == jau::cfmt::format("%#'x", 0xaffe_u32));
    CHECK("0xaffe" == jau::cfmt::format("%#'6x", 0xaffe_u32));
    CHECK(" 0xaffe" == jau::cfmt::format("%#'7x", 0xaffe_u32));
    CHECK("  0xaffe" == jau::cfmt::format("%#'8x", 0xaffe_u32));
    CHECK("0x1'affe" == jau::cfmt::format("%#'7x", 0x1affe_u32));
    CHECK("    0x1'affe" == jau::cfmt::format("%#'12x", 0x1affe_u32));

    // separator, zero-padding
    CHECK("'876'543" == jau::cfmt::format("%,08d", 876543));
    CHECK("9'876'543" == jau::cfmt::format("%,08d", 9876543));
    CHECK("9'876'543" == jau::cfmt::format("%,09d", 9876543));
    CHECK("09'876'543" == jau::cfmt::format("%,010d", 9876543));
    CHECK("0'009'876'543" == jau::cfmt::format("%,013d", 9876543));

    CHECK("0xaffe" == jau::cfmt::format("%#'x", 0xaffe_u32));
    CHECK("0xaffe" == jau::cfmt::format("%#'06x", 0xaffe_u32));
    CHECK("0x'affe" == jau::cfmt::format("%#'07x", 0xaffe_u32));
    CHECK("0x0'affe" == jau::cfmt::format("%#'08x", 0xaffe_u32));
    CHECK("0x1'affe" == jau::cfmt::format("%#'07x", 0x1affe_u32));
    CHECK("0x'0001'affe" == jau::cfmt::format("%#'012x", 0x1affe_u32));
}

TEST_CASE("binary", "[jau][std::string][jau::cfmt][flags]" ) {
    jau_format_checkLine("%b", 1_u32);

    CHECK("0b1" == jau::cfmt::format("%#b", 1_u32));
    CHECK("0b1010111111111110" == jau::cfmt::format("%#b", 0xaffe_u32));
    CHECK("1011111011101111" == jau::cfmt::format("%b", 0xbeef_u32));
}

TEST_CASE("space_flag", "[jau][std::string][jau::cfmt][flags]" ) {
  std::string buffer;

  buffer = jau::cfmt::format("% d", 42);
  CHECK(buffer == " 42");

  buffer = jau::cfmt::format("% d", -42);
  CHECK(buffer == "-42");

  buffer = jau::cfmt::format("% 5d", 42);
  CHECK(buffer == "   42");

  buffer = jau::cfmt::format("% 5d", -42);
  CHECK(buffer == "  -42"); //   "-  42" == "  -42"

  buffer = jau::cfmt::format("% 15d", 42);
  CHECK(buffer == "             42");

  buffer = jau::cfmt::format("% 15d", -42);
  CHECK(buffer == "            -42");

  buffer = jau::cfmt::format("% 15d", -42);
  CHECK(buffer == "            -42");

  buffer = jau::cfmt::format("% 15.3f", -42.987);
  CHECK(buffer == "        -42.987");

  buffer = jau::cfmt::format("% 15.3f", 42.987);
  CHECK(buffer == "         42.987");

  buffer = jau::cfmt::format("% s", "Hello testing");
  CHECK(buffer == "Hello testing");

  buffer = jau::cfmt::format("% d", 1024);
  CHECK(buffer == " 1024");

  buffer = jau::cfmt::format("% d", -1024);
  CHECK(buffer == "-1024");

  buffer = jau::cfmt::format("% i", 1024);
  CHECK(buffer == " 1024");

  buffer = jau::cfmt::format("% i", -1024);
  CHECK(buffer == "-1024");

  buffer = jau::cfmt::format("% u", 1024);
  CHECK(buffer == "1024");

  buffer = jau::cfmt::format("% u", 4294966272U);
  CHECK(buffer == "4294966272");

  buffer = jau::cfmt::format("% o", 511);
  CHECK(buffer == "777");

  buffer = jau::cfmt::format("% o", 4294966785U);
  CHECK(buffer == "37777777001");

  buffer = jau::cfmt::format("% x", 305441741);
  CHECK(buffer == "1234abcd");

  buffer = jau::cfmt::format("% x", 3989525555U);
  CHECK(buffer == "edcb5433");

  buffer = jau::cfmt::format("% X", 305441741);
  CHECK(buffer == "1234ABCD");

  buffer = jau::cfmt::format("% X", 3989525555U);
  CHECK(buffer == "EDCB5433");

  buffer = jau::cfmt::format("% c", 'x');
  CHECK(buffer == "x");
}

TEST_CASE("plus_flag", "[jau][std::string][jau::cfmt][flags]" ) {
  std::string buffer;

  buffer = jau::cfmt::format("%+d", 42);
  CHECK(buffer == "+42");

  buffer = jau::cfmt::format("%+d", -42);
  CHECK(buffer == "-42");

  buffer = jau::cfmt::format("%+5d", 42);
  CHECK(buffer == "  +42");

  buffer = jau::cfmt::format("%+5d", -42);
  CHECK(buffer == "  -42");

  buffer = jau::cfmt::format("%+15d", 42);
  CHECK(buffer == "            +42");

  buffer = jau::cfmt::format("%+15d", -42);
  CHECK(buffer == "            -42");

  buffer = jau::cfmt::format("%+s", "Hello testing");
  CHECK(buffer == "Hello testing");

  buffer = jau::cfmt::format("%+d", 1024);
  CHECK(buffer == "+1024");

  buffer = jau::cfmt::format("%+d", -1024);
  CHECK(buffer == "-1024");

  buffer = jau::cfmt::format("%+i", 1024);
  CHECK(buffer == "+1024");

  buffer = jau::cfmt::format("%+i", -1024);
  CHECK(buffer == "-1024");

  buffer = jau::cfmt::format("%+u", 1024);
  CHECK(buffer == "1024");

  buffer = jau::cfmt::format("%+u", 4294966272U);
  CHECK(buffer == "4294966272");

  buffer = jau::cfmt::format("%+o", 511);
  CHECK(buffer == "777");

  buffer = jau::cfmt::format("%+o", 4294966785U);
  CHECK(buffer == "37777777001");

  buffer = jau::cfmt::format("%+x", 305441741);
  CHECK(buffer == "1234abcd");

  buffer = jau::cfmt::format("%+x", 3989525555U);
  CHECK(buffer == "edcb5433");

  buffer = jau::cfmt::format("%+X", 305441741);
  CHECK(buffer == "1234ABCD");

  buffer = jau::cfmt::format("%+X", 3989525555U);
  CHECK(buffer == "EDCB5433");

  buffer = jau::cfmt::format("%+c", 'x');
  CHECK(buffer == "x");

  buffer = jau::cfmt::format("%+.0d", 0);
  CHECK(buffer == "+");
}


TEST_CASE("zero_flag", "[jau][std::string][jau::cfmt][flags]" ) {
  std::string buffer;

  buffer = jau::cfmt::format("%0d", 42);
  CHECK(buffer == "42");

  buffer = jau::cfmt::format("%0ld", 42L);
  CHECK(buffer == "42");

  buffer = jau::cfmt::format("%0d", -42);
  CHECK(buffer == "-42");

  buffer = jau::cfmt::format("%05d", 42);
  CHECK(buffer == "00042");

  buffer = jau::cfmt::format("%05d", -42);
  CHECK(buffer == "-0042");

  buffer = jau::cfmt::format("%015d", 42);
  CHECK(buffer == "000000000000042");

  buffer = jau::cfmt::format("%015d", -42);
  CHECK(buffer == "-00000000000042");

  buffer = jau::cfmt::format("%015.2f", 42.1234);
  CHECK(buffer == "000000000042.12");

  buffer = jau::cfmt::format("%015.3f", 42.9876);
  CHECK(buffer == "00000000042.988");

  buffer = jau::cfmt::format("%015.5f", -42.9876);
  CHECK(buffer == "-00000042.98760");
}


TEST_CASE("left_flag", "[jau][std::string][jau::cfmt][flags]" ) {
  std::string buffer;

  buffer = jau::cfmt::format("%-d", 42);
  CHECK(buffer == "42");

  buffer = jau::cfmt::format("%-d", -42);
  CHECK(buffer == "-42");

  buffer = jau::cfmt::format("%-5d", 42);
  CHECK(buffer == "42   ");

  buffer = jau::cfmt::format("%-5d", -42);
  CHECK(buffer == "-42  ");

  buffer = jau::cfmt::format("%-15d", 42);
  CHECK(buffer == "42             ");

  buffer = jau::cfmt::format("%-15d", -42);
  CHECK(buffer == "-42            ");

  buffer = jau::cfmt::format("%-0d", 42);
  CHECK(buffer == "42");

  buffer = jau::cfmt::format("%-0d", -42);
  CHECK(buffer == "-42");

  buffer = jau::cfmt::format("%-05d", 42);
  CHECK(buffer == "42   ");

  buffer = jau::cfmt::format("%-05d", -42);
  CHECK(buffer == "-42  ");

  buffer = jau::cfmt::format("%-015d", 42);
  CHECK(buffer == "42             ");

  buffer = jau::cfmt::format("%-015d", -42);
  CHECK(buffer == "-42            ");

  buffer = jau::cfmt::format("%0-d", 42);
  CHECK(buffer == "42");

  buffer = jau::cfmt::format("%0-d", -42);
  CHECK(buffer == "-42");

  buffer = jau::cfmt::format("%0-5d", 42);
  CHECK(buffer == "42   ");

  buffer = jau::cfmt::format("%0-5d", -42);
  CHECK(buffer == "-42  ");

  buffer = jau::cfmt::format("%0-15d", 42);
  CHECK(buffer == "42             ");

  buffer = jau::cfmt::format("%0-15d", -42);
  CHECK(buffer == "-42            ");

  buffer = jau::cfmt::format("%0-15.3e", -42.);
  CHECK(buffer == "-4.200e+01     ");

  buffer = jau::cfmt::format("%0-15.3g", -42.);
  CHECK(buffer == "-42.0          ");
}


TEST_CASE("hash_flag", "[jau][std::string][jau::cfmt][flags]" ) {
  std::string buffer;

  buffer = jau::cfmt::format("%#.0x", 0);
  CHECK(buffer == "");
  buffer = jau::cfmt::format("%#.1x", 0);
  CHECK(buffer == "0");
  buffer = jau::cfmt::format("%#.0llx", (long long)0);
  CHECK(buffer == "");
  buffer = jau::cfmt::format("%#.8x", 0x614e);
  CHECK(buffer == "0x0000614e");
  buffer = jau::cfmt::format("%#b", 6);
  CHECK(buffer == "0b110");
}


TEST_CASE("specifier", "[jau][std::string][jau::cfmt][specifier]" ) {
  std::string buffer;

  buffer = jau::cfmt::format("Hello testing");
  CHECK(buffer == "Hello testing");

  buffer = jau::cfmt::format("%s", "Hello testing");
  CHECK(buffer == "Hello testing");

  buffer = jau::cfmt::format("%d", 1024);
  CHECK(buffer == "1024");

  buffer = jau::cfmt::format("%d", -1024);
  CHECK(buffer == "-1024");

  buffer = jau::cfmt::format("%i", 1024);
  CHECK(buffer == "1024");

  buffer = jau::cfmt::format("%i", -1024);
  CHECK(buffer == "-1024");

  buffer = jau::cfmt::format("%u", 1024);
  CHECK(buffer == "1024");

  buffer = jau::cfmt::format("%u", 4294966272U);
  CHECK(buffer == "4294966272");

  buffer = jau::cfmt::format("%o", 511);
  CHECK(buffer == "777");

  buffer = jau::cfmt::format("%o", 4294966785U);
  CHECK(buffer == "37777777001");

  buffer = jau::cfmt::format("%x", 305441741);
  CHECK(buffer == "1234abcd");

  buffer = jau::cfmt::format("%x", 3989525555U);
  CHECK(buffer == "edcb5433");

  buffer = jau::cfmt::format("%X", 305441741);
  CHECK(buffer == "1234ABCD");

  buffer = jau::cfmt::format("%X", 3989525555U);
  CHECK(buffer == "EDCB5433");

  buffer = jau::cfmt::format("%%");
  CHECK(buffer == "%");
}


TEST_CASE("width", "[jau][std::string][jau::cfmt][width]" ) {
  std::string buffer;

  buffer = jau::cfmt::format("%1s", "Hello testing");
  CHECK(buffer == "Hello testing");

  buffer = jau::cfmt::format("%1d", 1024);
  CHECK(buffer == "1024");

  buffer = jau::cfmt::format("%1d", -1024);
  CHECK(buffer == "-1024");

  buffer = jau::cfmt::format("%1i", 1024);
  CHECK(buffer == "1024");

  buffer = jau::cfmt::format("%1i", -1024);
  CHECK(buffer == "-1024");

  buffer = jau::cfmt::format("%1u", 1024);
  CHECK(buffer == "1024");

  buffer = jau::cfmt::format("%1u", 4294966272U);
  CHECK(buffer == "4294966272");

  buffer = jau::cfmt::format("%1o", 511);
  CHECK(buffer == "777");

  buffer = jau::cfmt::format("%1o", 4294966785U);
  CHECK(buffer == "37777777001");

  buffer = jau::cfmt::format("%1x", 305441741);
  CHECK(buffer == "1234abcd");

  buffer = jau::cfmt::format("%1x", 3989525555U);
  CHECK(buffer == "edcb5433");

  buffer = jau::cfmt::format("%1X", 305441741);
  CHECK(buffer == "1234ABCD");

  buffer = jau::cfmt::format("%1X", 3989525555U);
  CHECK(buffer == "EDCB5433");

  buffer = jau::cfmt::format("%1c", 'x');
  CHECK(buffer == "x");
}


TEST_CASE("width_20", "[jau][std::string][jau::cfmt][width]" ) {
  std::string buffer;

  buffer = jau::cfmt::format("%20s", "Hello");
  CHECK(buffer == "               Hello");

  buffer = jau::cfmt::format("%20d", 1024);
  CHECK(buffer == "                1024");

  buffer = jau::cfmt::format("%20d", -1024);
  CHECK(buffer == "               -1024");

  buffer = jau::cfmt::format("%20i", 1024);
  CHECK(buffer == "                1024");

  buffer = jau::cfmt::format("%20i", -1024);
  CHECK(buffer == "               -1024");

  buffer = jau::cfmt::format("%20u", 1024);
  CHECK(buffer == "                1024");

  buffer = jau::cfmt::format("%20u", 4294966272U);
  CHECK(buffer == "          4294966272");

  buffer = jau::cfmt::format("%20o", 511);
  CHECK(buffer == "                 777");

  buffer = jau::cfmt::format("%20o", 4294966785U);
  CHECK(buffer == "         37777777001");

  buffer = jau::cfmt::format("%20x", 305441741);
  CHECK(buffer == "            1234abcd");

  buffer = jau::cfmt::format("%20x", 3989525555U);
  CHECK(buffer == "            edcb5433");

  buffer = jau::cfmt::format("%20X", 305441741);
  CHECK(buffer == "            1234ABCD");

  buffer = jau::cfmt::format("%20X", 3989525555U);
  CHECK(buffer == "            EDCB5433");

  buffer = jau::cfmt::format("%20c", 'x');
  CHECK(buffer == "                   x");
}


TEST_CASE("width_star_20", "[jau][std::string][jau::cfmt][width]" ) {
  std::string buffer;

  buffer = jau::cfmt::format("%*s", 20, "Hello");
  CHECK(buffer == "               Hello");

  buffer = jau::cfmt::format("%*d", 20, 1024);
  CHECK(buffer == "                1024");

  buffer = jau::cfmt::format("%*d", 20, -1024);
  CHECK(buffer == "               -1024");

  buffer = jau::cfmt::format("%*i", 20, 1024);
  CHECK(buffer == "                1024");

  buffer = jau::cfmt::format("%*i", 20, -1024);
  CHECK(buffer == "               -1024");

  buffer = jau::cfmt::format("%*u", 20, 1024);
  CHECK(buffer == "                1024");

  buffer = jau::cfmt::format("%*u", 20, 4294966272U);
  CHECK(buffer == "          4294966272");

  buffer = jau::cfmt::format("%*o", 20, 511);
  CHECK(buffer == "                 777");

  buffer = jau::cfmt::format("%*o", 20, 4294966785U);
  CHECK(buffer == "         37777777001");

  buffer = jau::cfmt::format("%*x", 20, 305441741);
  CHECK(buffer == "            1234abcd");

  buffer = jau::cfmt::format("%*x", 20, 3989525555U);
  CHECK(buffer == "            edcb5433");

  buffer = jau::cfmt::format("%*X", 20, 305441741);
  CHECK(buffer == "            1234ABCD");

  buffer = jau::cfmt::format("%*X", 20, 3989525555U);
  CHECK(buffer == "            EDCB5433");

  buffer = jau::cfmt::format("%*c", 20,'x');
  CHECK(buffer == "                   x");
}


TEST_CASE("width_left_20", "[jau][std::string][jau::cfmt][width]" ) {
  std::string buffer;

  buffer = jau::cfmt::format("%-20s", "Hello");
  CHECK(buffer == "Hello               ");

  buffer = jau::cfmt::format("%-20d", 1024);
  CHECK(buffer == "1024                ");

  buffer = jau::cfmt::format("%-20d", -1024);
  CHECK(buffer == "-1024               ");

  buffer = jau::cfmt::format("%-20i", 1024);
  CHECK(buffer == "1024                ");

  buffer = jau::cfmt::format("%-20i", -1024);
  CHECK(buffer == "-1024               ");

  buffer = jau::cfmt::format("%-20u", 1024);
  CHECK(buffer == "1024                ");

  buffer = jau::cfmt::format("%-20.4f", 1024.1234);
  CHECK(buffer == "1024.1234           ");

  buffer = jau::cfmt::format("%-20u", 4294966272U);
  CHECK(buffer == "4294966272          ");

  buffer = jau::cfmt::format("%-20o", 511);
  CHECK(buffer == "777                 ");

  buffer = jau::cfmt::format("%-20o", 4294966785U);
  CHECK(buffer == "37777777001         ");

  buffer = jau::cfmt::format("%-20x", 305441741);
  CHECK(buffer == "1234abcd            ");

  buffer = jau::cfmt::format("%-20x", 3989525555U);
  CHECK(buffer == "edcb5433            ");

  buffer = jau::cfmt::format("%-20X", 305441741);
  CHECK(buffer == "1234ABCD            ");

  buffer = jau::cfmt::format("%-20X", 3989525555U);
  CHECK(buffer == "EDCB5433            ");

  buffer = jau::cfmt::format("%-20c", 'x');
  CHECK(buffer == "x                   ");

  buffer = jau::cfmt::format("|%5d| |%-2d| |%5d|", 9, 9, 9);
  CHECK(buffer == "|    9| |9 | |    9|");

  buffer = jau::cfmt::format("|%5d| |%-2d| |%5d|", 10, 10, 10);
  CHECK(buffer == "|   10| |10| |   10|");

  buffer = jau::cfmt::format("|%5d| |%-12d| |%5d|", 9, 9, 9);
  CHECK(buffer == "|    9| |9           | |    9|");

  buffer = jau::cfmt::format("|%5d| |%-12d| |%5d|", 10, 10, 10);
  CHECK(buffer == "|   10| |10          | |   10|");
}


TEST_CASE("zero_width_left_20", "[jau][std::string][jau::cfmt][width]" ) {
  std::string buffer;

  buffer = jau::cfmt::format("%0-20s", "Hello");
  CHECK(buffer == "Hello               ");

  buffer = jau::cfmt::format("%0-20d", 1024);
  CHECK(buffer == "1024                ");

  buffer = jau::cfmt::format("%0-20d", -1024);
  CHECK(buffer == "-1024               ");

  buffer = jau::cfmt::format("%0-20i", 1024);
  CHECK(buffer == "1024                ");

  buffer = jau::cfmt::format("%0-20i", -1024);
  CHECK(buffer == "-1024               ");

  buffer = jau::cfmt::format("%0-20u", 1024);
  CHECK(buffer == "1024                ");

  buffer = jau::cfmt::format("%0-20u", 4294966272U);
  CHECK(buffer == "4294966272          ");

  buffer = jau::cfmt::format("%0-20o", 511);
  CHECK(buffer == "777                 ");

  buffer = jau::cfmt::format("%0-20o", 4294966785U);
  CHECK(buffer == "37777777001         ");

  buffer = jau::cfmt::format("%0-20x", 305441741);
  CHECK(buffer == "1234abcd            ");

  buffer = jau::cfmt::format("%0-20x", 3989525555U);
  CHECK(buffer == "edcb5433            ");

  buffer = jau::cfmt::format("%0-20X", 305441741);
  CHECK(buffer == "1234ABCD            ");

  buffer = jau::cfmt::format("%0-20X", 3989525555U);
  CHECK(buffer == "EDCB5433            ");

  buffer = jau::cfmt::format("%0-20c", 'x');
  CHECK(buffer == "x                   ");
}


TEST_CASE("width_20", "[jau][std::string][jau::cfmt][padding]" ) {
  std::string buffer;

  buffer = jau::cfmt::format("%020d", 1024);
  CHECK(buffer == "00000000000000001024");

  buffer = jau::cfmt::format("%020d", -1024);
  CHECK(buffer == "-0000000000000001024");

  buffer = jau::cfmt::format("%020i", 1024);
  CHECK(buffer == "00000000000000001024");

  buffer = jau::cfmt::format("%020i", -1024);
  CHECK(buffer == "-0000000000000001024");

  buffer = jau::cfmt::format("%020u", 1024);
  CHECK(buffer == "00000000000000001024");

  buffer = jau::cfmt::format("%020u", 4294966272U);
  CHECK(buffer == "00000000004294966272");

  buffer = jau::cfmt::format("%020o", 511);
  CHECK(buffer == "00000000000000000777");

  buffer = jau::cfmt::format("%020o", 4294966785U);
  CHECK(buffer == "00000000037777777001");

  buffer = jau::cfmt::format("%020x", 305441741);
  CHECK(buffer == "0000000000001234abcd");

  buffer = jau::cfmt::format("%020x", 3989525555U);
  CHECK(buffer == "000000000000edcb5433");

  buffer = jau::cfmt::format("%020X", 305441741);
  CHECK(buffer == "0000000000001234ABCD");

  buffer = jau::cfmt::format("%020X", 3989525555U);
  CHECK(buffer == "000000000000EDCB5433");
}


TEST_CASE("precision_20", "[jau][std::string][jau::cfmt][padding]" ) {
  std::string buffer;

  buffer = jau::cfmt::format("%.20d", 1024);
  CHECK(buffer == "00000000000000001024");

  buffer = jau::cfmt::format("%.20d", -1024);
  CHECK(buffer == "-00000000000000001024");

  buffer = jau::cfmt::format("%.20i", 1024);
  CHECK(buffer == "00000000000000001024");

  buffer = jau::cfmt::format("%.20i", -1024);
  CHECK(buffer == "-00000000000000001024");

  buffer = jau::cfmt::format("%.20u", 1024);
  CHECK(buffer == "00000000000000001024");

  buffer = jau::cfmt::format("%.20u", 4294966272U);
  CHECK(buffer == "00000000004294966272");

  buffer = jau::cfmt::format("%.20o", 511);
  CHECK(buffer == "00000000000000000777");

  buffer = jau::cfmt::format("%.20o", 4294966785U);
  CHECK(buffer == "00000000037777777001");

  buffer = jau::cfmt::format("%.20x", 305441741);
  CHECK(buffer == "0000000000001234abcd");

  buffer = jau::cfmt::format("%.20x", 3989525555U);
  CHECK(buffer == "000000000000edcb5433");

  buffer = jau::cfmt::format("%.20X", 305441741);
  CHECK(buffer == "0000000000001234ABCD");

  buffer = jau::cfmt::format("%.20X", 3989525555U);
  CHECK(buffer == "000000000000EDCB5433");
}


TEST_CASE("hash_zero_width_20", "[jau][std::string][jau::cfmt][padding]" ) {
  std::string buffer;

  buffer = jau::cfmt::format("%#020d", 1024);
  CHECK(buffer == "00000000000000001024");

  buffer = jau::cfmt::format("%#020d", -1024);
  CHECK(buffer == "-0000000000000001024");

  buffer = jau::cfmt::format("%#020i", 1024);
  CHECK(buffer == "00000000000000001024");

  buffer = jau::cfmt::format("%#020i", -1024);
  CHECK(buffer == "-0000000000000001024");

  buffer = jau::cfmt::format("%#020u", 1024);
  CHECK(buffer == "00000000000000001024");

  buffer = jau::cfmt::format("%#020u", 4294966272U);
  CHECK(buffer == "00000000004294966272");

  buffer = jau::cfmt::format("%#020o", 511);
  CHECK(buffer == "00000000000000000777");

  buffer = jau::cfmt::format("%#020o", 4294966785U);
  CHECK(buffer == "00000000037777777001");

  buffer = jau::cfmt::format("%#020x", 305441741);
  CHECK(buffer == "0x00000000001234abcd");

  buffer = jau::cfmt::format("%#020x", 3989525555U);
  CHECK(buffer == "0x0000000000edcb5433");

  buffer = jau::cfmt::format("%#020X", 305441741);
  CHECK(buffer == "0X00000000001234ABCD");

  buffer = jau::cfmt::format("%#020X", 3989525555U);
  CHECK(buffer == "0X0000000000EDCB5433");
}


TEST_CASE("hash_width_20", "[jau][std::string][jau::cfmt][padding]" ) {
  std::string buffer;

  buffer = jau::cfmt::format("%#20d", 1024);
  CHECK(buffer == "                1024");

  buffer = jau::cfmt::format("%#20d", -1024);
  CHECK(buffer == "               -1024");

  buffer = jau::cfmt::format("%#20i", 1024);
  CHECK(buffer == "                1024");

  buffer = jau::cfmt::format("%#20i", -1024);
  CHECK(buffer == "               -1024");

  buffer = jau::cfmt::format("%#20u", 1024);
  CHECK(buffer == "                1024");

  buffer = jau::cfmt::format("%#20u", 4294966272U);
  CHECK(buffer == "          4294966272");

  buffer = jau::cfmt::format("%#20o", 511);
  CHECK(buffer == "                0777");

  buffer = jau::cfmt::format("%#20o", 4294966785U);
  CHECK(buffer == "        037777777001");

  buffer = jau::cfmt::format("%#20x", 305441741);
  CHECK(buffer == "          0x1234abcd");

  buffer = jau::cfmt::format("%#20x", 3989525555U);
  CHECK(buffer == "          0xedcb5433");

  buffer = jau::cfmt::format("%#20X", 305441741);
  CHECK(buffer == "          0X1234ABCD");

  buffer = jau::cfmt::format("%#20X", 3989525555U);
  CHECK(buffer == "          0XEDCB5433");
}


TEST_CASE("width_20_precision_5", "[jau][std::string][jau::cfmt][padding]" ) {
  std::string buffer;

  buffer = jau::cfmt::format("%20.5d", 1024);
  CHECK(buffer == "               01024");

  buffer = jau::cfmt::format("%20.5d", -1024);
  CHECK(buffer == "              -01024");

  buffer = jau::cfmt::format("%20.5i", 1024);
  CHECK(buffer == "               01024");

  buffer = jau::cfmt::format("%20.5i", -1024);
  CHECK(buffer == "              -01024");

  buffer = jau::cfmt::format("%20.5u", 1024);
  CHECK(buffer == "               01024");

  buffer = jau::cfmt::format("%20.5u", 4294966272U);
  CHECK(buffer == "          4294966272");

  buffer = jau::cfmt::format("%20.5o", 511);
  CHECK(buffer == "               00777");

  buffer = jau::cfmt::format("%20.5o", 4294966785U);
  CHECK(buffer == "         37777777001");

  buffer = jau::cfmt::format("%20.5x", 305441741);
  CHECK(buffer == "            1234abcd");

  buffer = jau::cfmt::format("%20.10x", 3989525555U);
  CHECK(buffer == "          00edcb5433");

  buffer = jau::cfmt::format("%20.5X", 305441741);
  CHECK(buffer == "            1234ABCD");

  buffer = jau::cfmt::format("%20.10X", 3989525555U);
  CHECK(buffer == "          00EDCB5433");
}


TEST_CASE("padding neg_numbers", "[jau][std::string][jau::cfmt][padding]" ) {
  std::string buffer;

  // space padding
  buffer = jau::cfmt::format("% 1d", -5);
  CHECK(buffer == "-5");

  buffer = jau::cfmt::format("% 2d", -5);
  CHECK(buffer == "-5");

  buffer = jau::cfmt::format("% 3d", -5);
  CHECK(buffer == " -5");

  buffer = jau::cfmt::format("% 4d", -5);
  CHECK(buffer == "  -5");

  // zero padding
  buffer = jau::cfmt::format("%01d", -5);
  CHECK(buffer == "-5");

  buffer = jau::cfmt::format("%02d", -5);
  CHECK(buffer == "-5");

  buffer = jau::cfmt::format("%03d", -5);
  CHECK(buffer == "-05");

  buffer = jau::cfmt::format("%04d", -5);
  CHECK(buffer == "-005");
}


TEST_CASE("float padding_neg_numbers", "[jau][std::string][jau::cfmt][float]" ) {
  std::string buffer;

  // space padding
  buffer = jau::cfmt::format("% 3.1f", -5.);
  CHECK(buffer == "-5.0");

  buffer = jau::cfmt::format("% 4.1f", -5.);
  CHECK(buffer == "-5.0");

  buffer = jau::cfmt::format("% 5.1f", -5.);
  CHECK(buffer == " -5.0");

  buffer = jau::cfmt::format("% 6.1g", -5.);
  CHECK(buffer == "    -5");

  buffer = jau::cfmt::format("% 6.1e", -5.);
  CHECK(buffer == "-5.0e+00");

  buffer = jau::cfmt::format("% 10.1e", -5.);
  CHECK(buffer == "  -5.0e+00");

  // zero padding
  buffer = jau::cfmt::format("%03.1f", -5.);
  CHECK(buffer == "-5.0");

  buffer = jau::cfmt::format("%04.1f", -5.);
  CHECK(buffer == "-5.0");

  buffer = jau::cfmt::format("%05.1f", -5.);
  CHECK(buffer == "-05.0");

  // zero padding no decimal point
  buffer = jau::cfmt::format("%01.0f", -5.);
  CHECK(buffer == "-5");

  buffer = jau::cfmt::format("%02.0f", -5.);
  CHECK(buffer == "-5");

  buffer = jau::cfmt::format("%03.0f", -5.);
  CHECK(buffer == "-05");

  buffer = jau::cfmt::format("%010.1e", -5.);
  CHECK(buffer == "-005.0e+00");

  buffer = jau::cfmt::format("%07.0E", -5.);
  CHECK(buffer == "-05E+00");

  buffer = jau::cfmt::format("%03.0g", -5.);
  CHECK(buffer == "-05");
}

TEST_CASE("length", "[jau][std::string][jau::cfmt][length]" ) {
  std::string buffer;

  buffer = jau::cfmt::format("%.0s", "Hello testing");
  CHECK(buffer == "");

  buffer = jau::cfmt::format("%20.0s", "Hello testing");
  CHECK(buffer == "                    ");

  buffer = jau::cfmt::format("%.s", "Hello testing");
  CHECK(buffer == "");

  buffer = jau::cfmt::format("%20.s", "Hello testing");
  CHECK(buffer == "                    ");

  buffer = jau::cfmt::format("%20.0d", 1024);
  CHECK(buffer == "                1024");

  buffer = jau::cfmt::format("%20.0d", -1024);
  CHECK(buffer == "               -1024");

  buffer = jau::cfmt::format("%20.d", 0);
  CHECK(buffer == "                    ");

  buffer = jau::cfmt::format("%20.0i", 1024);
  CHECK(buffer == "                1024");

  buffer = jau::cfmt::format("%20.i", -1024);
  CHECK(buffer == "               -1024");

  buffer = jau::cfmt::format("%20.i", 0);
  CHECK(buffer == "                    ");

  buffer = jau::cfmt::format("%20.u", 1024);
  CHECK(buffer == "                1024");

  buffer = jau::cfmt::format("%20.0u", 4294966272U);
  CHECK(buffer == "          4294966272");

  buffer = jau::cfmt::format("%20.u", 0U);
  CHECK(buffer == "                    ");

  buffer = jau::cfmt::format("%20.o", 511);
  CHECK(buffer == "                 777");

  buffer = jau::cfmt::format("%20.0o", 4294966785U);
  CHECK(buffer == "         37777777001");

  buffer = jau::cfmt::format("%20.o", 0U);
  CHECK(buffer == "                    ");

  buffer = jau::cfmt::format("%20.x", 305441741);
  CHECK(buffer == "            1234abcd");

  buffer = jau::cfmt::format("%50.x", 305441741);
  CHECK(buffer == "                                          1234abcd");

  buffer = jau::cfmt::format("%50.x%10.u", 305441741, 12345);
  CHECK(buffer == "                                          1234abcd     12345");

  buffer = jau::cfmt::format("%20.0x", 3989525555U);
  CHECK(buffer == "            edcb5433");

  buffer = jau::cfmt::format("%20.x", 0U);
  CHECK(buffer == "                    ");

  buffer = jau::cfmt::format("%20.X", 305441741);
  CHECK(buffer == "            1234ABCD");

  buffer = jau::cfmt::format("%20.0X", 3989525555U);
  CHECK(buffer == "            EDCB5433");

  buffer = jau::cfmt::format("%20.X", 0U);
  CHECK(buffer == "                    ");

  buffer = jau::cfmt::format("%02.0u", 0U);
  CHECK(buffer == "  ");

  buffer = jau::cfmt::format("%02.0d", 0);
  CHECK(buffer == "  ");
}


TEST_CASE("float", "[jau][std::string][jau::cfmt][float]" ) {
  std::string buffer;

  // test special-case floats using math.h macros
  buffer = jau::cfmt::format("%8f", NAN);
  CHECK(buffer == "     nan");

  buffer = jau::cfmt::format("%8f", INFINITY);
  CHECK(buffer == "     inf");

  buffer = jau::cfmt::format("%-8f", -INFINITY);
  CHECK(buffer == "-inf    ");

  buffer = jau::cfmt::format("%+8e", INFINITY);
  CHECK(buffer == "    +inf");

  buffer = jau::cfmt::format("%.4f", 3.1415354); // NOLINT
  CHECK(buffer == "3.1415");

  buffer = jau::cfmt::format("%.3f", 30343.1415354);
  CHECK(buffer == "30343.142");

  buffer = jau::cfmt::format("%.0f", 34.1415354);
  CHECK(buffer == "34");

  buffer = jau::cfmt::format("%.0f", 1.3);
  CHECK(buffer == "1");

  buffer = jau::cfmt::format("%.0f", 1.55);
  CHECK(buffer == "2");

  buffer = jau::cfmt::format("%.1f", 1.64);
  CHECK(buffer == "1.6");

  buffer = jau::cfmt::format("%.2f", 42.8952);
  CHECK(buffer == "42.90");

  buffer = jau::cfmt::format("%.9f", 42.8952);
  CHECK(buffer == "42.895200000");

  buffer = jau::cfmt::format("%.10f", 42.895223);
  CHECK(buffer == "42.8952230000");

  // assuming not being truncated to 9 digits. (19)
  buffer = jau::cfmt::format("%.12f", 42.987654321098);
  CHECK(buffer == "42.987654321098");

  // assuming not being truncated to 9 digits, but rounded
  buffer = jau::cfmt::format("%.12f", 42.98765432109899);
  CHECK(buffer == "42.987654321099");

  // 14
  buffer = jau::cfmt::format("%.14f", 42.98765432109876);
  CHECK(buffer == "42.98765432109876");
  // 14 rounded
  buffer = jau::cfmt::format("%.14f", 42.9876543210987699);
  CHECK(buffer == "42.98765432109877");

  // 16 truncated to 14 (max precision)
  buffer = jau::cfmt::format("%.16f", 42.9876543210987612);
  CHECK(buffer == "42.9876543210987600");

  // 16 truncated to 14 (max precision) and rounded
  buffer = jau::cfmt::format("%.16f", 42.9876543210987654);
  CHECK(buffer == "42.9876543210987700");

  buffer = jau::cfmt::format("%6.2f", 42.8952);
  CHECK(buffer == " 42.90");

  buffer = jau::cfmt::format("%+6.2f", 42.8952);
  CHECK(buffer == "+42.90");

  buffer = jau::cfmt::format("%+5.1f", 42.9252);
  CHECK(buffer == "+42.9");

  buffer = jau::cfmt::format("%f", 42.5_f64);
  CHECK(buffer == "42.500000");
  buffer = jau::cfmt::format("%f", 42.5);
  CHECK(buffer == "42.500000");

  buffer = jau::cfmt::format("%.1f", 42.5_f64);
  CHECK(buffer == "42.5");
  buffer = jau::cfmt::format("%.1f", 42.5);
  CHECK(buffer == "42.5");

  buffer = jau::cfmt::format("%f", 42167.0_f64);
  CHECK(buffer == "42167.000000");
  buffer = jau::cfmt::format("%f", 42167.0);
  CHECK(buffer == "42167.000000");

  buffer = jau::cfmt::format("%.9f", -12345.987654321_f64);
  CHECK(buffer == "-12345.987654321");
  buffer = jau::cfmt::format("%.9f", -12345.987654321);
  CHECK(buffer == "-12345.987654321");

  buffer = jau::cfmt::format("%.1f", 3.999_f64);
  CHECK(buffer == "4.0");
  buffer = jau::cfmt::format("%.1f", 3.999);
  CHECK(buffer == "4.0");

  buffer = jau::cfmt::format("%.0f", 3.5_f64);
  CHECK(buffer == "4");
  buffer = jau::cfmt::format("%.0f", 3.5);
  CHECK(buffer == "4");

  buffer = jau::cfmt::format("%.0f", 4.5);
  CHECK(buffer == "4");

  buffer = jau::cfmt::format("%.0f", 3.49);
  CHECK(buffer == "3");

  buffer = jau::cfmt::format("%.1f", 3.49);
  CHECK(buffer == "3.5");

  buffer = jau::cfmt::format("a%-5.1f", 0.5);
  CHECK(buffer == "a0.5  ");

  buffer = jau::cfmt::format("a%-5.1fend", 0.5);
  CHECK(buffer == "a0.5  end");

  buffer = jau::cfmt::format("%G", 12345.678);
  CHECK(buffer == "12345.7");

  buffer = jau::cfmt::format("%.7G", 12345.678);
  CHECK(buffer == "12345.68");

  buffer = jau::cfmt::format("%.5G", 123456789.);
  CHECK(buffer == "1.2346E+08");

  buffer = jau::cfmt::format("%.6G", 12345.);
  CHECK(buffer == "12345.0");

  buffer = jau::cfmt::format("%+12.4g", 123456789.);
  CHECK(buffer == "  +1.235e+08");

  buffer = jau::cfmt::format("%.2G", 0.001234);
  CHECK(buffer == "0.0012");

  buffer = jau::cfmt::format("%+10.4G", 0.001234);
  CHECK(buffer == " +0.001234");

  buffer = jau::cfmt::format("%+012.4g", 0.00001234);
  CHECK(buffer == "+001.234e-05");

  buffer = jau::cfmt::format("%.3g", -1.2345e-308);
  CHECK(buffer == "-1.23e-308");

  buffer = jau::cfmt::format("%+.3E", 1.23e+308);
  CHECK(buffer == "+1.230E+308");

  // out of range for float: should switch to exp notation if supported, else empty
  buffer = jau::cfmt::format("%.1f", 1E20);
  CHECK(buffer == "1.0e+20");

  buffer = jau::cfmt::format("%.5f", -1.12345);
  CHECK(buffer == "-1.12345");

  buffer = jau::cfmt::format("%.5f", -1.00000e20_f64);
  CHECK(buffer == "-1.00000e+20");
  buffer = jau::cfmt::format("%.5f", -1.00000e20);
  CHECK(buffer == "-1.00000e+20");

  // brute force float
  bool fail = false;
  std::stringstream str;
  str.precision(5);
  for (float i = -100000; i < 100000; i += 1) { // NOLINT
    buffer = jau::cfmt::format("%.5f", i / 10000);
    str.str("");
    str << std::fixed << i / 10000;
    fail = fail || buffer != str.str();
  }
  CHECK(!fail);

  // brute force exp
  fail = false;
  str.setf(std::ios::scientific, std::ios::floatfield);
  for (float i = -1e20; i < 1e20; i += 1e15) { // NOLINT
    buffer = jau::cfmt::format("%.5f", i);
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

  buffer = jau::cfmt::format("%i", 0);
  CHECK(buffer == "0");

  buffer = jau::cfmt::format("%i", 1234);
  CHECK(buffer == "1234");

  buffer = jau::cfmt::format("%i", 32767);
  CHECK(buffer == "32767");

  buffer = jau::cfmt::format("%i", -32767);
  CHECK(buffer == "-32767");

  buffer = jau::cfmt::format("%li", 30L);
  CHECK(buffer == "30");

  buffer = jau::cfmt::format("%li", -2147483647L);
  CHECK(buffer == "-2147483647");

  buffer = jau::cfmt::format("%li", 2147483647L);
  CHECK(buffer == "2147483647");

  buffer = jau::cfmt::format("%lli", 30LL);
  CHECK(buffer == "30");

  buffer = jau::cfmt::format("%lli", -9223372036854775807LL);
  CHECK(buffer == "-9223372036854775807");

  buffer = jau::cfmt::format("%lli", 9223372036854775807LL);
  CHECK(buffer == "9223372036854775807");

  buffer = jau::cfmt::format("%lu", 100000L);
  CHECK(buffer == "100000");

  buffer = jau::cfmt::format("%lu", 0xFFFFFFFFL);
  CHECK(buffer == "4294967295");

  buffer = jau::cfmt::format("%llu", 281474976710656LLU);
  CHECK(buffer == "281474976710656");

  buffer = jau::cfmt::format("%llu", 18446744073709551615LLU);
  CHECK(buffer == "18446744073709551615");

  buffer = jau::cfmt::format("%zu", 2147483647UL);
  CHECK(buffer == "2147483647");

  buffer = jau::cfmt::format("%zd", 2147483647L);
  CHECK(buffer == "2147483647");

  // failed intentionally unsigned -> signed 64-bit
  static_assert(0 < jau::cfmt::checkLine("%zd", 2147483647UL));
  // buffer = jau::cfmt::format("%zd", 2147483647UL);
  // CHECK(buffer == "2147483647");

  if (sizeof(size_t) == sizeof(long)) {
    buffer = jau::cfmt::format("%zi", -2147483647L);
    CHECK(buffer == "-2147483647");
  }
  else {
    buffer = jau::cfmt::format("%zi", -2147483647LL);
    CHECK(buffer == "-2147483647");
  }

  buffer = jau::cfmt::format("%b", 60000);
  CHECK(buffer == "1110101001100000");

  buffer = jau::cfmt::format("%lb", 12345678L);
  CHECK(buffer == "101111000110000101001110");

  buffer = jau::cfmt::format("%o", 60000);
  CHECK(buffer == "165140");

  buffer = jau::cfmt::format("%lo", 12345678L);
  CHECK(buffer == "57060516");

  buffer = jau::cfmt::format("%lx", 0x12345678L);
  CHECK(buffer == "12345678");

  buffer = jau::cfmt::format("%llx", 0x1234567891234567LLU);
  CHECK(buffer == "1234567891234567");

  buffer = jau::cfmt::format("%lx", 0xabcdefabL);
  CHECK(buffer == "abcdefab");

  buffer = jau::cfmt::format("%lX", 0xabcdefabL);
  CHECK(buffer == "ABCDEFAB");

  buffer = jau::cfmt::format("%c", 'v');
  CHECK(buffer == "v");

  buffer = jau::cfmt::format("%cv", 'w');
  CHECK(buffer == "wv");

  buffer = jau::cfmt::format("%s", "A Test");
  CHECK(buffer == "A Test");

  // !JAU_CFMT_IGNORE_LENGTH_MODIFIER
  // static_assert(0  < jau::cfmt::checkLine("%hhu", 0xFFU)); // size unsigned int > unsigned char (intentional failure)
  static_assert(0 == jau::cfmt::checkLine("%hhu", 0xFF_u8));
  buffer = jau::cfmt::format("%hhu", 0xFF_u8);
  CHECK(buffer == "255");

  // intentionally fails: given arg size > hh char
  // !JAU_CFMT_IGNORE_LENGTH_MODIFIER
  // static_assert(0 < jau::cfmt::checkLine("%hhu", 0xFFFFUL)); // size unsigned long > unsigned char (intentional failure)
  // buffer = jau::cfmt::format("%hhu", 0xFFFFUL);
  // CHECK(buffer == "255");

  // !JAU_CFMT_IGNORE_LENGTH_MODIFIER
  // static_assert(0  < jau::cfmt::checkLine("%hu", 0x123456UL)); // size unsigned long > unsigned short (intentional failure)
  static_assert(0 == jau::cfmt::checkLine("%hu", 0x1234_u16));
  buffer = jau::cfmt::format("%hu", 0x1234_u16); // size unsigned long > unsigned short
  CHECK(buffer == "4660");

  // !JAU_CFMT_IGNORE_LENGTH_MODIFIER
  // static_assert(0  < jau::cfmt::checkLine("%s%hhi %hu", "Test", 10000, 0xFFFFFFFF));
  static_assert(0 == jau::cfmt::checkLine("%s%hhi %hu", "Test", 16_i8, 0xFFFF_u16));
  buffer = jau::cfmt::format("%s%hhi %hu", "Test", (char)16, (unsigned short)0xFFFF);
  CHECK(buffer == "Test16 65535");

  buffer = jau::cfmt::format("%tx", &buffer[10] - &buffer[0]);
  CHECK(buffer == "a");

// TBD
  if (sizeof(intmax_t) == sizeof(long)) {
    buffer = jau::cfmt::format("%ji", -2147483647L);
    CHECK(buffer == "-2147483647");
  }
  else {
    buffer = jau::cfmt::format("%ji", -2147483647LL);
    CHECK(buffer == "-2147483647");
  }
}


TEST_CASE("pointer", "[jau][std::string][jau::cfmt][pointer]" ) {
  std::string buffer;

#if 0
  buffer = jau::cfmt::format("%p", (void*)0x1234U);
  if (sizeof(void*) == 4U) {
    CHECK(buffer == "00001234");
  }
  else {
    CHECK(buffer == "0000000000001234");
  }

  buffer = jau::cfmt::format("%p", (void*)0x12345678U);
  if (sizeof(void*) == 4U) {
    CHECK(buffer == "12345678");
  }
  else {
    CHECK(buffer == "0000000012345678");
  }

  buffer = jau::cfmt::format("%p-%p", (void*)0x12345678U, (void*)0x7EDCBA98U);
  if (sizeof(void*) == 4U) {
    CHECK(buffer == "12345678-7EDCBA98");
  }
  else {
    CHECK(buffer == "0000000012345678-000000007EDCBA98");
  }

  if (sizeof(uintptr_t) == sizeof(uint64_t)) {
    buffer = jau::cfmt::format("%p", (void*)(uintptr_t)0xFFFFFFFFU); // NOLINT
    CHECK(buffer == "00000000FFFFFFFF");
  }
  else {
    buffer = jau::cfmt::format("%p", (void*)(uintptr_t)0xFFFFFFFFU); // NOLINT
    CHECK(buffer == "FFFFFFFF");
  }
#else
  // %#x or %#lx
  buffer = jau::cfmt::format("%p", (void*)0x1234U);
  CHECK(buffer == "0x1234");

  buffer = jau::cfmt::format("%p", (void*)0x12345678U);
  CHECK(buffer == "0x12345678");

  buffer = jau::cfmt::format("%p-%p", (void*)0x12345678U, (void*)0x7EDCBA98U);
  CHECK(buffer == "0x12345678-0x7edcba98");

  buffer = jau::cfmt::format("%p", (void*)(uintptr_t)0xFFFFFFFFU); // NOLINT
  CHECK(buffer == "0xffffffff");
#endif
}


TEST_CASE("unknown flag", "[jau][std::string][jau::cfmt][error]" ) {
  std::string buffer;

  // we inject an error message
  CHECK(0 > jau::cfmt::check("%kmarco", 42, 37)); // orig "kmarco"
  buffer = jau::cfmt::format("%kmarco", 42, 37); // orig "kmarco"
  std::cerr << "buffer @ " << __LINE__ << ": '" << buffer << "'\n";
  const size_t q = buffer.find("<E#", 0);
  CHECK( q != std::string::npos );
}


TEST_CASE("string length", "[jau][std::string][jau::cfmt][stringlen]" ) {
  std::string buffer;

  buffer = jau::cfmt::format("%.4s", "This is a test");
  CHECK(buffer == "This");

  buffer = jau::cfmt::format("%.4s", "test");
  CHECK(buffer == "test");

  buffer = jau::cfmt::format("%.7s", "123");
  CHECK(buffer == "123");

  buffer = jau::cfmt::format("%.7s", "");
  CHECK(buffer == "");

  buffer = jau::cfmt::format("%.4s%.2s", "123456", "abcdef");
  CHECK(buffer == "1234ab");

  // we inject an error message
  buffer = jau::cfmt::format("%.4.2s", "123456"); // orig ".2s"
  const size_t q = buffer.find("<E#", 0);
  CHECK( q != std::string::npos );

  buffer = jau::cfmt::format("%.*s", 3, "123456");
  CHECK(buffer == "123");
}

TEST_CASE("misc", "[jau][std::string][jau::cfmt][misc]" ) {
  std::string buffer;

  buffer = jau::cfmt::format("%u%u%ctest%d %s", 5, 3000, 'a', -20, "bit");
  CHECK(buffer == "53000atest-20 bit");

  buffer = jau::cfmt::format("%.*f", 2, 0.33333333);
  CHECK(buffer == "0.33");

  buffer = jau::cfmt::format("%.*d", -1, 1);
  CHECK(buffer == "1");

  buffer = jau::cfmt::format("%.3s", "foobar");
  CHECK(buffer == "foo");

  buffer = jau::cfmt::format("% .0d", 0);
  CHECK(buffer == " ");

  buffer = jau::cfmt::format("%10.5d", 4);
  CHECK(buffer == "     00004");

  buffer = jau::cfmt::format("%*sx", -3, "hi");
  CHECK(buffer == "hi x");

  buffer = jau::cfmt::format("%.*g", 2, 0.33333333);
  CHECK(buffer == "0.33");

  buffer = jau::cfmt::format("%.*e", 2, 0.33333333);
  CHECK(buffer == "3.33e-01");
}
