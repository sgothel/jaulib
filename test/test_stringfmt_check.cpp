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
#include <sstream>
#include <string_view>
#include <type_traits>

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
#include <jau/debug.hpp>

#ifdef HAS_STD_FORMAT
    #include <format>
#endif

using namespace std::literals;

using namespace jau::float_literals;

using namespace jau::int_literals;

static std::string format_snprintf_ffszu64d(float fa, float fb, size_t sz1, uint64_t a_u64, int i) {
    size_t nchars;
    std::string str;
    {
        const size_t bsz = 1024;  // including EOS
        str.reserve(bsz);         // incl. EOS
        str.resize(bsz - 1);      // excl. EOS

        nchars = std::snprintf(&str[0], bsz,
                               "format_000a: %f, %f, %zu, %" PRIu64 ", %" PRIu64 ", %" PRIx64 ", %06" PRIu64 ", %06" PRIx64 ", %d\n",
                               fa + 1.0_f32, fb + 1.0_f32, sz1 + 1,
                               a_u64 + 1_u64, a_u64 + 1_u64, a_u64 + 1_u64, a_u64 + 1_u64, a_u64 + 1_u64,
                               i + 1);

        if( nchars < bsz ) {
            str.resize(nchars);
            str.shrink_to_fit();
            return str;
        }
    }
    str.clear();  // error
    return str;
}

static std::string format_010a_jaufmtstr(float fa, float fb, size_t sz1, uint64_t a_u64, int i) {
    return jau::format_string("format_010a: %f, %f, %zu, %" PRIu64 ", %" PRIu64 ", %" PRIx64 ", %06" PRIu64 ", %06" PRIx64 ", %d\n",
                              fa + 1.0_f32, fb + 1.0_f32, sz1 + 1,
                              a_u64 + 1_u64, a_u64 + 1_u64, a_u64 + 1_u64, a_u64 + 1_u64, a_u64 + 1_u64,
                              i + 1);
}

static std::string format_020a_jaufmtstr_n(float fa, float fb, size_t sz1, uint64_t a_u64, int i) {
    return jau::format_string_n(1023, "format_020a: %f, %f, %zu, %" PRIu64 ", %" PRIu64 ", %" PRIx64 ", %06" PRIu64 ", %06" PRIx64 ", %d\n",
                                fa + 1.0_f32, fb + 1.0_f32, sz1 + 1,
                                a_u64 + 1_u64, a_u64 + 1_u64, a_u64 + 1_u64, a_u64 + 1_u64, a_u64 + 1_u64,
                                i + 1);
}

static std::string format_030a_strstream(float fa, float fb, size_t sz1, uint64_t a_u64, int i) {
    std::ostringstream ss1;
    ss1 << "format_030a: "
        << fa + 1.0_f32 << ", "
        << fb + 1.0_f32 << ", "
        << sz1 + 1 << ", "
        << a_u64 + 1_u64 << ", "
        << i + 1 << "\n";
    return ss1.str();
}

#ifdef HAS_STD_FORMAT
std::string format_040a_stdformat(float fa, float fb, size_t sz1, uint64_t a_u64, int i) {
    return std::format("format_040a: {0}, {1}, {3}, {4}, {5}\n",
                       fa + 1.0_f32, fb + 1.0_f32, sz1 + 1, a_u64 + 1_u64, i + 1);
}
#endif

static std::string format_000b_vsnprintf(float fa, float fb, size_t sz1, uint64_t a_u64, int i) {
    size_t nchars;
    std::string str;
    {
        const size_t bsz = 1024;  // including EOS
        str.reserve(bsz);         // incl. EOS
        str.resize(bsz - 1);      // excl. EOS

        nchars = std::snprintf(&str[0], bsz,
                               "format_000b: %.2f, %2.2f, %zu, %" PRIu64 ", %" PRIu64 ", %" PRIx64 ", %06" PRIu64 ", %06" PRIx64 ", %03d\n",
                               fa + 1.0_f32, fb + 1.0_f32, sz1 + 1,
                               a_u64 + 1_u64, a_u64 + 1_u64, a_u64 + 1_u64, a_u64 + 1_u64, a_u64 + 1_u64,
                               i + 1);

        if( nchars < bsz ) {
            str.resize(nchars);
            str.shrink_to_fit();
            return str;
        }
    }
    str.clear();  // error
    return str;
}
static std::string format_010b_jaufmtstr(float fa, float fb, size_t sz1, uint64_t a_u64, int i) {
    return jau::format_string("format_010b: %.2f, %2.2f, %zu, %" PRIu64 ", %" PRIu64 ", %" PRIx64 ", %06" PRIu64 ", %06" PRIx64 ", %03d\n",
                              fa + 1.0_f32, fb + 1.0_f32, sz1 + 1,
                              a_u64 + 1_u64, a_u64 + 1_u64, a_u64 + 1_u64, a_u64 + 1_u64, a_u64 + 1_u64,
                              i + 1);
}

static std::string format_020b_jaufmtstr_n(float fa, float fb, size_t sz1, uint64_t a_u64, int i) {
    return jau::format_string_n(1023, "format_020b: %.2f, %2.2f, %zu, %" PRIu64 ", %" PRIu64 ", %" PRIx64 ", %06" PRIu64 ", %06" PRIx64 ", %03d\n",
                                fa + 1.0_f32, fb + 1.0_f32, sz1 + 1,
                                a_u64 + 1_u64, a_u64 + 1_u64, a_u64 + 1_u64, a_u64 + 1_u64, a_u64 + 1_u64,
                                i + 1);
}

static std::string format_030b_strstream(float fa, float fb, size_t sz1, uint64_t a_u64, int i) {
    std::ostringstream ss1;
    ss1.precision(3);
    ss1 << "format_030b: "
        << fa + 1.0_f32 << ", ";
    ss1.width(3);
    ss1 << fb + 1.0_f32 << ", "
        << sz1 + 1 << ", "
        << a_u64 + 1_u64 << ", ";
    ss1.width(3);
    ss1 << i + 1 << "\n";
    return ss1.str();
}

#ifdef HAS_STD_FORMAT
std::string format_040b_stdformat(float fa, float fb, size_t sz1, uint64_t a_u64, int i) {
    return std::format("format_040b: {0:.2f}, {1:2.2f}, {3}, {4}, {5:03d}\n",
                       fa + 1.0_f32, fb + 1.0_f32, sz1 + 1, a_u64 + 1_u64, i + 1);
}
#endif

template <typename Func>
static size_t test_format(const Func func, bool output) {
    constexpr float fa = 1.1f, fb = 2.2f;
    constexpr size_t sz1 = 1;
    constexpr uint64_t sz2 = 2;
    constexpr int i = 3;

    const std::string s = func(fa, fb, sz1, sz2, i);
    volatile size_t l = s.length();
    if( output ) {
        std::cout << s;
    }
    REQUIRE(0 < l);
    REQUIRE(l < 1024);
    return l;
}

static void format_0a() {
    test_format(format_snprintf_ffszu64d, true);
    test_format(format_010a_jaufmtstr, true);
    test_format(format_020a_jaufmtstr_n, true);
    test_format(format_030a_strstream, true);

#ifdef HAS_STD_FORMAT
    test_format(format_040a_stdformat, true);
#endif
}
static void format_0b() {
    test_format(format_000b_vsnprintf, true);
    test_format(format_010b_jaufmtstr, true);
    test_format(format_020b_jaufmtstr_n, true);
    test_format(format_030b_strstream, true);

#ifdef HAS_STD_FORMAT
    test_format(format_040b_stdformat, true);
#endif
}

static void test_refs(const size_t &sz, const int64_t &i64, const float &f) {
    // references can't be passed as arguments
    //   static_assert( 0 == jau::cfmt::checkLine("lala %zu, %" PRIi64 ", %f", sz, i64, f) );
    //   static_assert( 0 == jau::cfmt::check("lala %zu, %" PRIi64 ", %f", sz, i64, f) );
    //
    // however, after removing the reference and feeding it into the check2 variant, it works
    static_assert( 0 == jau::cfmt::check2Line<std::remove_reference_t<decltype(sz)>,
            std::remove_reference_t<decltype(i64)>,
            std::remove_reference_t<decltype(f)>>("lala %zu, %" PRIi64 ", %f") );
    //
    // which is what is done in the macros
    //
    jau_format_checkLine("lala %zu, %" PRIi64 ", %f", sz, i64, f);
    jau_format_check("lala %zu, %" PRIi64 ", %f", sz, i64, f);
}

TEST_CASE("jau::cfmt_10", "[jau][std::string][jau::cfmt]") {
    char buf[1024];
    constexpr float fa = 1.123456f, fb = 2.2f;
    constexpr size_t sz1 = 1;
    constexpr int64_t v_i64 = 2;
    constexpr uint64_t v_u64 = 3;
    constexpr int i = 3;
    const float *pf = &fa;

    test_refs(sz1, v_i64, fa);
    {
        std::string s;
        jau::cfmt::Result r = jau::cfmt::formatR(s, "lala %d", 2);
        std::cerr << "XXX: " << __LINE__ << ": " << r << std::endl;
        REQUIRE( 1 == r.argumentCount() );
    }
    {   // 'format_check: %.2f, %2.2f, %zu, %lu, %03d'
        std::string s;
        jau::cfmt::Result r = jau::cfmt::formatR(s, "format_check: %.2f, %2.2f, %zu, %" PRIi64 ", %" PRIi64 "d, %" PRIi64 "X, %06" PRIu64 "d, %06" PRIu64 "X, %03d\n",
                                                       fa, fb, sz1, v_i64, v_i64, v_i64, v_u64, v_u64, i);
        std::cerr << "XXX: " << __LINE__ << ": " << r << std::endl;
        REQUIRE(9 == r.argumentCount());
    }
    {
        std::string s;
        jau::cfmt::Result r = jau::cfmt::formatR(s, "lala %d - end", 2);
        std::cerr << "XXX: " << __LINE__ << ": " << r << std::endl;
        REQUIRE(1 == r.argumentCount());
    }
    {
        static_assert( 0 == jau::cfmt::checkLine("lala %d", 2) );
        static_assert( 1 == jau::cfmt::check("lala %d", 2) );

        int i2 = 2;
        constexpr const bool b2 = jau::cfmt::check("lala %d", i2);
        static_assert( b2 );
    }
    {
        static_assert(false == std::is_signed_v<const float*>);
        static_assert(false == std::is_unsigned_v<const float*>);

        static_assert(true == std::is_signed_v<float>);
        static_assert(false == std::is_unsigned_v<float>);
    }
    {
        using E = int;
        using T = unsigned int;
        // using U = std::conditional_t<std::is_unsigned_v<T>, std::make_signed_t<T>, T>; // triggers the instantiating the 'other' case and hence fails
        using U = typename std::conditional_t<std::is_unsigned_v<T>, std::make_signed<T>, std::type_identity<T>>::type; // NOLINT
        static_assert( std::is_same_v<T, T> );
        static_assert( std::is_same_v<E, U> );
    }
    {
        using E = float;
        using T = float;
        // using U = std::conditional_t<std::is_unsigned_v<T>, std::make_signed_t<T>, T>; // triggers the instantiating the 'other' case and hence fails
        using U = typename std::conditional_t<std::is_unsigned_v<T>, std::make_signed<T>, std::type_identity<T>>::type; // NOLINT
        static_assert( std::is_same_v<T, T> );
        static_assert( std::is_same_v<E, U> );
    }
    {
        static_assert(0 == jau::cfmt::checkLine(" lala %d", 1));
        static_assert(0 == jau::cfmt::checkLine(" lala %ld", 1));
        static_assert(0 == jau::cfmt::checkLine(" lala %zd", 1));
        static_assert(0 == jau::cfmt::checkLine(" lala %8d", 1));
        static_assert(0 == jau::cfmt::checkLine(" lala %08d", 1));
        static_assert(0 == jau::cfmt::checkLine(" lala %08.2d", 1));
        static_assert(0 == jau::cfmt::checkLine(" %" PRIi64 ", %" PRIi64 ", %08" PRIi64 ".", (int64_t)1, (int64_t)1, (int64_t)1));
    }
    {
        // we support safe signedness conversion
        static_assert( 1  == jau::cfmt::check("         int -> int %d", 1));
        static_assert(-1 == jau::cfmt::check("unsigned int  -> int %d", (unsigned int)1));  // error: sizeof(unsigned) == sizeof(signed)
        static_assert( 1 == jau::cfmt::check("unsigned char -> int %d", (unsigned char)1)); // OK: sizeof(unsigned) < sizeof(signed)
        static_assert( 1  == jau::cfmt::check("unsigned int -> unsigned int %u", (unsigned int)1));
        static_assert( 1 == jau::cfmt::check("         int -> unsigned int %u",  1));            // OK: +signed -> unsigned
        static_assert(-1 == jau::cfmt::check("    uint64_t -> int64_t %" PRIi64, (uint64_t)1)); // error: sizeof(unsigned) == sizeof(signed)
        static_assert( 1 == jau::cfmt::check("     int64_t -> uint64_t %" PRIu64, (int64_t)1)); // OK: +signed -> unsigned

        // given type <= integral target type
        static_assert( 1 == jau::cfmt::check("        char -> int %d", (char)1));     // OK  given type <= integral target type
        static_assert(-1 == jau::cfmt::check("        char -> int %d", (uint64_t)1)); // ERR given type  > integral target type
        {
            jau::cfmt::Result res = jau::cfmt::checkR(" error long -> int %d", (long)1);
            // if( res.error() ) {
            printf("XXX: sizeof(long) %zu, %s\n", sizeof(long), res.toString().c_str());
        }
        if constexpr ( sizeof(long) <= sizeof(int) ) {
            REQUIRE( 1 == jau::cfmt::check(" OK long(4) -> int %d", (long)1)); // NOLINT(misc-static-assert)
        } else {
            REQUIRE(-1 == jau::cfmt::check(" error long(8) -> int %d", (long)1)); // NOLINT(misc-static-assert); error: given type > integral target type
        }

        static_assert(1 == jau::cfmt::check(" %d", i));
        static_assert(1 == jau::cfmt::check(" %f", fa));
        static_assert(1 == jau::cfmt::check(" %zd", (ssize_t)1));
        static_assert(1 == jau::cfmt::check(" %zu", (size_t)1));
        static_assert(3 == jau::cfmt::check(" %" PRIi64 ", %" PRIi64 ", %08" PRIi64 ".", (int64_t)1, (int64_t)1, (int64_t)1));
        static_assert(3 == jau::cfmt::check(" %" PRIi64 ", %" PRIi64 ", %08" PRIi64 ".", v_i64, v_i64, v_i64));
        static_assert(1 == jau::cfmt::check(" %p", pf));
        const char* cs1 = "lala";
        static_assert(1 == jau::cfmt::check(" %s", (const char*)"lala"));
        static_assert(1 == jau::cfmt::check(" %s", cs1));
        static_assert(0 == jau::cfmt::checkLine(" %s", cs1));
        (void)cs1;

        static_assert(0 == jau::cfmt::checkR("Hello World").argumentCount());
        static_assert(1 == jau::cfmt::checkR("Hello World %d", 1).argumentCount());
        static_assert(1 == jau::cfmt::checkR("Hello 1 %d", i).argumentCount());
        static_assert(0 == jau::cfmt::check("Hello World"));
        static_assert(1 == jau::cfmt::check("Hello World %d", 1));
        static_assert(1 == jau::cfmt::check("Hello 1 %d", i));

        static_assert(1 == jau::cfmt::checkR("Hello 1 %.2f", fa).argumentCount());
        static_assert(1 == jau::cfmt::checkR("Hello 1 %.2f - end", fa).argumentCount());
        static_assert(2 == jau::cfmt::checkR("Hello 1 %.2f, 2 %2.2f - end", fa, fb).argumentCount());
        static_assert(3 == jau::cfmt::checkR("Hello 1 %.2f , 2 %2.2f, 3 %zu - end", fa, fb, sz1).argumentCount());
        static_assert(4 == jau::cfmt::checkR("Hello 1 %.2f, 2 %2.2f, 3 %zu, 4 %" PRIi64 " - end", fa, fb, sz1, v_i64).argumentCount());
        static_assert(5 == jau::cfmt::checkR("Hello 1 %.2f, 2 %2.2f, 3 %zu, 4 %" PRIi64 ", 5 %03d - end", fa, fb, sz1, v_i64, i).argumentCount());

        static_assert(5 == jau::cfmt::checkR("Hello %" PRIi64 ", %" PRIu64 ", %" PRIx64 ", %06" PRIu64 ", %06" PRIx64 "",
                                             v_i64, v_u64, v_u64, v_u64, v_u64).argumentCount());

        // static_assert(6 == jau::cfmt::checkR("Hello 1 %.2f, 2 %2.2f, 3 %zu, 4 %" PRIi64 ", 5 %03d, 6 %p - end", fa, fb, sz1, sz2, i, pf).argumentCount());
        // static_assert(1 == jau::cfmt::checkR(" %s", "lala").argumentCount());

        static_assert(0 > jau::cfmt::check("Hello World %"));
        static_assert(0 > jau::cfmt::checkR("Hello World %").argumentCount());
        static_assert(0 > jau::cfmt::checkR("Hello 1 %d").argumentCount());
        static_assert(-1 == jau::cfmt::checkR("Hello 1 %d", fa).argumentCount());
        if constexpr ( sizeof(long) <= 4 ) {
            assert( 1 == jau::cfmt::checkR("Hello 1 %d", sz1).argumentCount()); // NOLINT(misc-static-assert)
        } else {
            assert(-1 == jau::cfmt::checkR("Hello 1 %d", sz1).argumentCount()); // NOLINT(misc-static-assert)
        }
        static_assert(-6 == jau::cfmt::checkR("Hello 1 %.2f, 2 %2.2f, 3 %zu, 4 %" PRIi64 ", 5 %03d, 6 %p - end",
                                              fa, fb, sz1, v_i64, i, i).argumentCount());
        static_assert(-6 == jau::cfmt::check("Hello 1 %.2f, 2 %2.2f, 3 %zu, 4 %" PRIi64 ", 5 %03d, 6 %p - end",
                                              fa, fb, sz1, v_i64, i, i));

        {
            static_assert(0 <= jau::cfmt::checkR("format_020a: %f, %f, %zu, %" PRIu64 ", %d\n",
                                                 fa + 1.0_f32, fb + 1.0_f32, sz1 + 1, v_i64 + 1_u64, i + 1).argumentCount()); // compile time validation!
            const std::string s = jau_format_string("format_020a: %f, %f, %zu, %" PRIu64 ", %d\n",
                                                    fa + 1.0_f32, fb + 1.0_f32, sz1 + 1, v_i64 + 1_u64, i + 1);
            REQUIRE( s.size() > 0 );
        }
        {
            std::string s0 = jau::format_string("Hello %d", 1);
            REQUIRE( s0.size() > 0 );
            // FIXME
            // const std::string s1 = jau::format_string<"Hello %d">(1);
            // REQUIRE_THROWS_AS( s0 = jau::format_string("Hello %d", 1.0f), jau::IllegalArgumentError);
        }
    }
    {
        std::string s1 = jau_format_string("Hello %d", 1);
        REQUIRE("Hello 1" == s1);

        constexpr jau::cfmt::Result c1 = jau::cfmt::checkR("Hello %u", (unsigned int)1);
        std::cerr << "XXX: " << __LINE__ << ": " << c1 << std::endl;
        REQUIRE(true == c1.success());
    }
    {
        constexpr jau::cfmt::Result c1 = jau::cfmt::checkR("Hello World");
        REQUIRE(true == c1.success());
        REQUIRE(0 == c1.argumentCount());
        REQUIRE(0 == jau::cfmt::checkR("Hello World").argumentCount());
        constexpr jau::cfmt::Result c3 = jau::cfmt::checkR("Hello 1 %d", i);
        REQUIRE(true == c3.success());
        REQUIRE(1 == c3.argumentCount());
        REQUIRE(9 == std::snprintf(buf, sizeof(buf), "Hello 1 %d", i));

        // `Hello 1 %.2f, 2 %2.2f, 3 %zu, 4 %li, 5 %03d, 6 %p - end`
        REQUIRE(1 == jau::cfmt::checkR("Hello 1 %.2f", fa).argumentCount());
        REQUIRE(1 == jau::cfmt::checkR("Hello 1 %.2f - end", fa).argumentCount());

        // 'Hello 1 %.2f, 2 %2.2f - end'
        jau::cfmt::Result pc = jau::cfmt::checkR("Hello 1 %.2f, 2 %2.2f - end", fa, fb);
        std::cerr << "XXX: " << __LINE__ << ": " << pc << std::endl;
        REQUIRE(2 == jau::cfmt::checkR("Hello 1 %.2f, 2 %2.2f - end", fa, fb).argumentCount());

        // `Hello 1 %.2f, 2 %2.2f, 3 %zu - end`
        pc = jau::cfmt::checkR("Hello 1 %.2f, 2 %2.2f, 3 %zu - end", fa, fb, sz1);
        std::cerr << "XXX: " << __LINE__ << ": " << pc << std::endl;
        REQUIRE(3 == jau::cfmt::checkR("Hello 1 %.2f, 2 %2.2f, 3 %zu - end", fa, fb, sz1).argumentCount());

        REQUIRE(4 == jau::cfmt::checkR("Hello 1 %.2f, 2 %2.2f, 3 %zu, 4 %" PRIi64 " - end", fa, fb, sz1, v_i64).argumentCount());
        REQUIRE(5 == jau::cfmt::checkR("Hello 1 %.2f, 2 %2.2f, 3 %zu, 4 %" PRIi64 ", 5 %03d - end", fa, fb, sz1, v_i64, i).argumentCount());
        REQUIRE(6 == jau::cfmt::checkR("Hello 1 %.2f, 2 %2.2f, 3 %zu, 4 %" PRIi64 ", 5 %03d, 6 %p - end", fa, fb, sz1, v_i64, i, pf).argumentCount());

        // REQUIRE(13 == std::snprintf(buf, sizeof(buf), "Hello World %").argumentCount());
        REQUIRE(0 > jau::cfmt::checkR("Hello World %").argumentCount());
        REQUIRE(0 > jau::cfmt::checkR("Hello 1 %d").argumentCount());
        REQUIRE(-1 == jau::cfmt::checkR("Hello 1 %d", fa).argumentCount());
        if constexpr ( sizeof(long) <= 4 ) {
            REQUIRE( 1 == jau::cfmt::checkR("Hello 1 %d", sz1).argumentCount());
        } else {
            REQUIRE(-1 == jau::cfmt::checkR("Hello 1 %d", sz1).argumentCount());
        }
        REQUIRE(-6 == jau::cfmt::checkR("Hello 1 %.2f, 2 %2.2f, 3 %zu, 4 %" PRIi64 ", 5 %03d, 6 %p - end",
                                        fa, fb, sz1, v_i64, i, i).argumentCount());
    }
}

TEST_CASE("jau::cfmt_01", "[jau][std::string][format_string]") {
    format_0a();
    format_0b();
}

TEST_CASE("jau::cfmt_10 debug", "[jau][std::string][format_string][debug]") {
    jau_INFO_PRINT("lala001");
    jau_INFO_PRINT("lala002 %d, %f, %s", 1, 3.14, "hello world");
    std::string s1 = "Hello";
    std::string_view sv1 = s1;
    const char * s1p = s1.c_str();
    jau_INFO_PRINT("lala003 %s, %s, %s", s1, sv1, s1p);

    jau_ERR_PRINT3("error 01: '%s', %d", s1, 88);
}
