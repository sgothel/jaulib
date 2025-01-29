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

#include <jau/cpp_lang_util.hpp>
#include <jau/cpp_pragma.hpp>
#include <jau/float_types.hpp>
#include <jau/int_types.hpp>
#include <jau/string_cfmt.hpp>
#include <jau/string_util.hpp>
#include <jau/test/catch2_ext.hpp>
#include <jau/type_traits_queries.hpp>

#include "string_cfmt2.hpp"

#ifdef HAS_STD_FORMAT
    #include <format>
#endif

using namespace std::literals;

using namespace jau::float_literals;

using namespace jau::int_literals;

template <typename... Args>
constexpr std::string format_string000(const std::size_t maxStrLen, const std::string_view format, const Args &...args) {
    std::string str;
    str.reserve(maxStrLen + 1);  // incl. EOS
    str.resize(maxStrLen);       // excl. EOS

    // -Wformat=2 -> -Wformat -Wformat-nonliteral -Wformat-security -Wformat-y2k
    // -Wformat=2 -Wformat-overflow=2 -Wformat-signedness
    PRAGMA_DISABLE_WARNING_PUSH
    PRAGMA_DISABLE_WARNING_FORMAT_NONLITERAL
    const size_t nchars = std::snprintf(&str[0], maxStrLen + 1, format.data(), args...);
    PRAGMA_DISABLE_WARNING_POP
    if( nchars < maxStrLen + 1 ) {
        str.resize(nchars);
        str.shrink_to_fit();
    }  // else truncated w/ nchars > MaxStrLen
    return str;
}

std::string format_000a_vsnprintf(float fa, float fb, size_t sz1, uint64_t a_u64, int i) {
    size_t nchars;
    std::string str;
    {
        const size_t bsz = 1024;  // including EOS
        str.reserve(bsz);         // incl. EOS
        str.resize(bsz - 1);      // excl. EOS

        nchars = std::snprintf(&str[0], bsz,
                               "format_000a: %f, %f, %zu, %" PRIu64 ", %d\n",
                               fa + 1.0_f32, fb + 1.0_f32, sz1 + 1, a_u64 + 1_u64, i + 1);

        if( nchars < bsz ) {
            str.resize(nchars);
            str.shrink_to_fit();
            return str;
        }
    }
    str.clear();  // error
    return str;
}

std::string format_010a_vsnprintf(float fa, float fb, size_t sz1, uint64_t a_u64, int i) {
    return jau::format_string("format_010a: %f, %f, %zu, %" PRIu64 ", %d\n",
                              fa + 1.0_f32, fb + 1.0_f32, sz1 + 1, a_u64 + 1_u64, i + 1);
}

constexpr std::string format_020a_tsnprintf(float fa, float fb, size_t sz1, uint64_t a_u64, int i) {
    return jau::format_string_v(1023, "format_020a: %f, %f, %zu, %" PRIu64 ", %d\n",
                                fa + 1.0_f32, fb + 1.0_f32, sz1 + 1, a_u64 + 1_u64, i + 1);
}

std::string format_030a_strstream(float fa, float fb, size_t sz1, uint64_t a_u64, int i) {
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

std::string format_000b_vsnprintf(float fa, float fb, size_t sz1, uint64_t a_u64, int i) {
    size_t nchars;
    std::string str;
    {
        const size_t bsz = 1024;  // including EOS
        str.reserve(bsz);         // incl. EOS
        str.resize(bsz - 1);      // excl. EOS

        nchars = std::snprintf(&str[0], bsz,
                               "format_000b: %.2f, %2.2f, %zu, %" PRIu64 ", %03d\n",
                               fa + 1.0_f32, fb + 1.0_f32, sz1 + 1, a_u64 + 1_u64, i + 1);

        if( nchars < bsz ) {
            str.resize(nchars);
            str.shrink_to_fit();
            return str;
        }
    }
    str.clear();  // error
    return str;
}
std::string format_010b_vsnprintf(float fa, float fb, size_t sz1, uint64_t a_u64, int i) {
    return jau::format_string("format_010b: %.2f, %2.2f, %zu, %" PRIu64 ", %03d\n",
                              fa + 1.0_f32, fb + 1.0_f32, sz1 + 1, a_u64 + 1_u64, i + 1);
}

std::string format_020b_tsnprintf(float fa, float fb, size_t sz1, uint64_t a_u64, int i) {
    return jau::format_string_v(1023, "format_020b: %.2f, %2.2f, %zu, %" PRIu64 ", %03d\n",
                                fa + 1.0_f32, fb + 1.0_f32, sz1 + 1, a_u64 + 1_u64, i + 1);
}

std::string format_030b_strstream(float fa, float fb, size_t sz1, uint64_t a_u64, int i) {
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
size_t test_format(const Func func, bool output) {
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

void format_0a() {
    test_format(format_000a_vsnprintf, true);
    test_format(format_010a_vsnprintf, true);
    test_format(format_020a_tsnprintf, true);
    test_format(format_030a_strstream, true);

#ifdef HAS_STD_FORMAT
    test_format(format_040a_stdformat, true);
#endif
}
void format_0b() {
    test_format(format_000b_vsnprintf, true);
    test_format(format_010b_vsnprintf, true);
    test_format(format_020b_tsnprintf, true);
    test_format(format_030b_strstream, true);

#ifdef HAS_STD_FORMAT
    test_format(format_040b_stdformat, true);
#endif
}

template <typename... Args>
constexpr std::string format_string_static2(const std::string_view fmt, const Args &...) {
    // constexpr const std::string format2(format);
    // constexpr const bool b = jau::cfmt::check2<Args...>("Haus"sv);
    constexpr const bool b = jau::cfmt::check3<Args...>(fmt);
    static_assert( b );
    (void)b;
    return ""; // jau::format_string_v(1024, format, args...);
}


template <typename... Targs>
constexpr jau::cfmt2::PResult check(const std::string_view fmt, const Targs &...) noexcept {
    return jau::cfmt2::impl::checkRec<Targs...>( jau::cfmt2::PResult(fmt) );
}

template <typename... Targs>
constexpr std::string format_string_static3(const std::string_view format, const Targs &...args) {
    // constexpr const jau::cfmt2::PResult ctx2 = jau::cfmt2::impl::checkRec<Targs...>( jau::cfmt2::PResult(format) );
    // static_assert( 0 <= jau::cfmt2::impl::checkRec<Targs...>( jau::cfmt2::PResult(format)).argCount() );
    if( 0 <= jau::cfmt2::impl::checkRec<Targs...>( jau::cfmt2::PResult(format)).argCount() ) {
        return jau::format_string_v(1024, format, args...);
    } else {
        return "";
    }
}


TEST_CASE("jau::cfmt_00", "[jau][std::string][jau::cfmt]") {
    char buf[1024];
    constexpr float fa = 1.123456f, fb = 2.2f;
    constexpr size_t sz1 = 1;
    constexpr int64_t sz2 = 2;
    constexpr int i = 3;
    const float *pf = &fa;

    {
        constexpr jau::cfmt2::PResult pr = jau::cfmt2::checkR("lala %d", 2);
        std::cerr << "XXX: " << __LINE__ << ": " << pr << std::endl;
        static_assert( 0 <= pr.argCount() );
    }
    {   // 'format_check: %.2f, %2.2f, %zu, %lu, %03d'
        constexpr  jau::cfmt2::PResult pc = jau::cfmt2::checkR("format_check: %.2f, %2.2f, %zu, %" PRIu64 ", %03d\n",
                                                               fa, fb, sz1, sz2, i);
        std::cerr << "XXX: " << __LINE__ << ": " << pc << std::endl;
        // static_assert( 5 == pc.argCount() );
        REQUIRE(5 == pc.argCount());
    }
    {
        constexpr jau::cfmt2::PResult pr = jau::cfmt2::checkR("lala %d - end", 2);
        std::cerr << "XXX: " << __LINE__ << ": " << pr << std::endl;
        // static_assert( 0 <= pr.argCount() );
        REQUIRE(0 <= pr.argCount());
    }
    {
        constexpr const bool b1 = jau::cfmt::check("lala %d", 2);
        static_assert( b1 );

        constexpr const bool b2 = jau::cfmt::check2<int>("lala %d");
        static_assert( b2 );

        // constexpr jau::cfmt2::PResult pr1 = check(fmt3, 2);
        constexpr jau::cfmt2::PResult pr1 = check("Hello %d", 2);
        std::cerr << "XXX: " << __LINE__ << ": " << pr1 << std::endl;
        static_assert( 0 <= pr1.argCount() );

        std::string s3 = format_string_static3("Hello %d", 2);
        std::cerr << "XXX: " << __LINE__ << ": " << s3 << std::endl;
        REQUIRE( s3.length() > 0 );

    // constexpr const std::string f2 = "Hello %d"s;
    // constexpr const std::string_view f2v = "Hello %d"sv;
    // format_string_static2(f2v, (int)2);
    // constexpr const std::string_view f3 = "Hello %d"sv;
    // constexpr const char f3[] = "Hello %d";
    // format_string_static2("Hello %d"sv, (int)2);
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
        // we shall ignore signedness like snprintf
        static_assert(true == jau::cfmt::check("         int -> int %d", 1));
        static_assert(true == jau::cfmt::check("unsigned int -> int %d", (unsigned int)1));
        static_assert(true == jau::cfmt::check("unsigned int -> unsigned int %u", (unsigned int)1));
        static_assert(true == jau::cfmt::check("         int -> unsigned int %u", 1));
        static_assert(true == jau::cfmt::check("        char -> int %d", (char)1)); // integral target type > given type
        #if !defined(__EMSCRIPTEN__)
            static_assert(false == jau::cfmt::check(" error long -> int %d", (long)1)); // error: integral target type < given type
        #endif

        static_assert(true == jau::cfmt::check(" %d", i));
        static_assert(true == jau::cfmt::check(" %f", fa));
        static_assert(true == jau::cfmt::check(" %zd", (ssize_t)1));
        static_assert(true == jau::cfmt::check(" %zu", (size_t)1));
        static_assert(true == jau::cfmt::check(" %" PRIi64 ".", (int64_t)1));
        static_assert(true == jau::cfmt::check(" %" PRIi64 ".", sz2));
        static_assert(true == jau::cfmt::check(" %p", pf));

        static_assert(0 == jau::cfmt::checkR("Hello World").argCount());
        static_assert(1 == jau::cfmt::checkR("Hello World %d", 1).argCount());
        static_assert(1 == jau::cfmt::checkR("Hello 1 %d", i).argCount());
        static_assert(true == jau::cfmt::check("Hello World"));
        static_assert(true == jau::cfmt::check("Hello World %d", 1));
        static_assert(true == jau::cfmt::check("Hello 1 %d", i));

        static_assert(1 == jau::cfmt::checkR("Hello 1 %.2f", fa).argCount());
        static_assert(1 == jau::cfmt::checkR("Hello 1 %.2f - end", fa).argCount());
        static_assert(2 == jau::cfmt::checkR("Hello 1 %.2f, 2 %2.2f - end", fa, fb).argCount());
        static_assert(3 == jau::cfmt::checkR("Hello 1 %.2f , 2 %2.2f, 3 %zu - end", fa, fb, sz1).argCount());
        static_assert(4 == jau::cfmt::checkR("Hello 1 %.2f, 2 %2.2f, 3 %zu, 4 %" PRIi64 " - end", fa, fb, sz1, sz2).argCount());
        static_assert(5 == jau::cfmt::checkR("Hello 1 %.2f, 2 %2.2f, 3 %zu, 4 %" PRIi64 ", 5 %03d - end", fa, fb, sz1, sz2, i).argCount());
        // static_assert(6 == jau::cfmt::checkR("Hello 1 %.2f, 2 %2.2f, 3 %zu, 4 %" PRIi64 ", 5 %03d, 6 %p - end", fa, fb, sz1, sz2, i, pf).argCount());

        static_assert(false == jau::cfmt::check("Hello World %"));
        static_assert(0 > jau::cfmt::checkR("Hello World %").argCount());
        static_assert(0 > jau::cfmt::checkR("Hello 1 %d").argCount());
        static_assert(-1 == jau::cfmt::checkR("Hello 1 %d", fa).argCount());
        #if !defined(__EMSCRIPTEN__)
            static_assert(-1 == jau::cfmt::checkR("Hello 1 %d", sz1).argCount());
        #endif
        static_assert(-6 == jau::cfmt::checkR("Hello 1 %.2f, 2 %2.2f, 3 %zu, 4 %" PRIi64 ", 5 %03d, 6 %p - end",
                                              fa, fb, sz1, sz2, i, i).argCount());
        static_assert(false == jau::cfmt::check("Hello 1 %.2f, 2 %2.2f, 3 %zu, 4 %" PRIi64 ", 5 %03d, 6 %p - end",
                                              fa, fb, sz1, sz2, i, i));

        const std::string s = jau_format_string_static("format_020a: %f, %f, %zu, %" PRIu64 ", %d\n",
                                                       fa + 1.0_f32, fb + 1.0_f32, sz1 + 1, sz2 + 1_u64, i + 1);
        (void)s;
    }
    {
        constexpr bool b1 = std::is_nothrow_assignable_v<int&, uint64_t>;
        static_assert( b1, "Not Assignable" );
        constexpr bool b2 = std::is_nothrow_assignable_v<unsigned int&, long double>;
        static_assert( b2, "Not Assignable" );
        jau_format_string_static("Hello");
        jau_format_string_static("Hello %d", (int)2);
        jau_format_string_static("Hello %d", (unsigned int)2);
        jau_format_string_static("Hello %u", (unsigned int)2);
        jau_format_string_static("Hello %u", (int)2);

        // constexpr std::string_view fmt2 = "Hello %d"sv;
        // format_string_static2("Hello %d"sv, (int)2);
        // format_string_static2("Hello %d"s, (int)2);

        constexpr jau::cfmt::PResult c1 = jau::cfmt::checkR("Hello %u", (unsigned int)1);
        std::cerr << "XXX: " << __LINE__ << ": " << c1 << std::endl;
        REQUIRE(false == c1.error());
    }
    {
        constexpr jau::cfmt::PResult c1 = jau::cfmt::checkR("Hello World");
        REQUIRE(false == c1.error());
        REQUIRE(0 == c1.argCount());
        REQUIRE(0 == jau::cfmt::checkR("Hello World").argCount());
        constexpr jau::cfmt::PResult c3 = jau::cfmt::checkR("Hello 1 %d", i);
        REQUIRE(false == c3.error());
        REQUIRE(1 == c3.argCount());
        REQUIRE(9 == std::snprintf(buf, sizeof(buf), "Hello 1 %d", i));

        // `Hello 1 %.2f, 2 %2.2f, 3 %zu, 4 %li, 5 %03d, 6 %p - end`
        REQUIRE(1 == jau::cfmt::checkR("Hello 1 %.2f", fa).argCount());
        REQUIRE(1 == jau::cfmt::checkR("Hello 1 %.2f - end", fa).argCount());

        // 'Hello 1 %.2f, 2 %2.2f - end'
        jau::cfmt::PResult pc = jau::cfmt::checkR("Hello 1 %.2f, 2 %2.2f - end", fa, fb);
        std::cerr << "XXX: " << __LINE__ << ": " << pc << std::endl;
        REQUIRE(2 == jau::cfmt::checkR("Hello 1 %.2f, 2 %2.2f - end", fa, fb).argCount());

        // `Hello 1 %.2f, 2 %2.2f, 3 %zu - end`
        pc = jau::cfmt::checkR("Hello 1 %.2f, 2 %2.2f, 3 %zu - end", fa, fb, sz1);
        std::cerr << "XXX: " << __LINE__ << ": " << pc << std::endl;
        REQUIRE(3 == jau::cfmt::checkR("Hello 1 %.2f, 2 %2.2f, 3 %zu - end", fa, fb, sz1).argCount());

        REQUIRE(4 == jau::cfmt::checkR("Hello 1 %.2f, 2 %2.2f, 3 %zu, 4 %" PRIi64 " - end", fa, fb, sz1, sz2).argCount());
        REQUIRE(5 == jau::cfmt::checkR("Hello 1 %.2f, 2 %2.2f, 3 %zu, 4 %" PRIi64 ", 5 %03d - end", fa, fb, sz1, sz2, i).argCount());
        REQUIRE(6 == jau::cfmt::checkR("Hello 1 %.2f, 2 %2.2f, 3 %zu, 4 %" PRIi64 ", 5 %03d, 6 %p - end", fa, fb, sz1, sz2, i, pf).argCount());

        // REQUIRE(13 == std::snprintf(buf, sizeof(buf), "Hello World %").argCount());
        REQUIRE(0 > jau::cfmt::checkR("Hello World %").argCount());
        REQUIRE(0 > jau::cfmt::checkR("Hello 1 %d").argCount());
        REQUIRE(-1 == jau::cfmt::checkR("Hello 1 %d", fa).argCount());
        REQUIRE(-1 == jau::cfmt::checkR("Hello 1 %d", sz1).argCount());
        REQUIRE(-6 == jau::cfmt::checkR("Hello 1 %.2f, 2 %2.2f, 3 %zu, 4 %" PRIi64 ", 5 %03d, 6 %p - end",
                                        fa, fb, sz1, sz2, i, i).argCount());
    }
}

TEST_CASE("jau::cfmt_01", "[jau][std::string][format_string]") {
    format_0a();
    format_0b();
}

/// Execute with `test_stringfmt --perf_analysis`
TEST_CASE("jau::cfmt_10", "[benchmark][jau][std::string][format_string]") {
    const size_t loops = 1000; // catch_auto_run ? 1000 : 1000;
    WARN("Benchmark with " + std::to_string(loops) + " loops");
    CHECK(true);

    BENCHMARK("fmt__check            bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            float fa = 1.1f, fb = 2.2f;
            size_t sz1 = 1;
            uint64_t sz2 = 2;
            int i1 = 3;

            size_t r = jau::cfmt::checkR("format_check: %.2f, %2.2f, %zu, %" PRIu64 ", %03d\n",
                                         fa, fb, sz1, sz2, i1).argCount();
            REQUIRE(5 == r);
            res = res + r;
        }
        return res;
    };
    BENCHMARK("fmt__check cnstexpr   bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            constexpr float fa = 1.1f, fb = 2.2f;
            constexpr size_t sz1 = 1;
            constexpr uint64_t sz2 = 2;
            constexpr int i1 = 3;

            constexpr  jau::cfmt::PResult pc = jau::cfmt::checkR("format_check: %.2f, %2.2f, %zu, %" PRIu64 ", %03d\n",
                                                                  fa, fb, sz1, sz2, i1);
            constexpr size_t r = pc.argCount();
            REQUIRE(5 == r);
            res = res + r;
        }
        return res;
    };
    BENCHMARK("fmt__check cnstexp2   bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            constexpr float fa = 1.1f, fb = 2.2f;
            constexpr size_t sz1 = 1;
            constexpr uint64_t sz2 = 2;
            constexpr int i1 = 3;

            constexpr  jau::cfmt2::PResult pc = jau::cfmt2::checkR("format_check: %.2f, %2.2f, %zu, %" PRIu64 ", %03d\n",
                                                                   fa, fb, sz1, sz2, i1);
            constexpr size_t r = pc.argCount();
            REQUIRE(5 == r);
            res = res + r;
        }
        return res;
    };
    BENCHMARK("format_000a_vsnprintf bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            res = res + test_format(format_000a_vsnprintf, false);
        }
        return res;
    };
    BENCHMARK("format_010a_vsnprintf bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            res = res + test_format(format_010a_vsnprintf, false);
        }
        return res;
    };
    BENCHMARK("fmt__020a macro       bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            {
                constexpr float fa = 1.1f, fb = 2.2f;
                constexpr size_t sz1 = 1;
                constexpr uint64_t a_u64 = 2;
                constexpr int j = 3;
                const std::string s = jau_format_string_static("format_020a: %f, %f, %zu, %" PRIu64 ", %d\n",
                                                               fa + 1.0_f32, fb + 1.0_f32, sz1 + 1, a_u64 + 1_u64, j + 1);
                res = res + s.length();
            }
        }
        return res;
    };
    BENCHMARK("fmt__020a cnstexpr-in bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            {
                constexpr float fa = 1.1f, fb = 2.2f;
                constexpr size_t sz1 = 1;
                constexpr uint64_t a_u64 = 2;
                constexpr int j = 3;
                if constexpr( jau::cfmt::check("format_020a: %f, %f, %zu, %" PRIu64 ", %d\n",
                                               fa + 1.0_f32, fb + 1.0_f32, sz1 + 1, a_u64 + 1_u64, j + 1) ) {
                    std::string s;
                    s.reserve(1023 + 1);  // incl. EOS
                    s.resize(1023);       // excl. EOS

                    // -Wformat=2 -> -Wformat -Wformat-nonliteral -Wformat-security -Wformat-y2k
                    // -Wformat=2 -Wformat-overflow=2 -Wformat-signedness
                    PRAGMA_DISABLE_WARNING_PUSH
                    PRAGMA_DISABLE_WARNING_FORMAT_NONLITERAL
                    const size_t nchars = std::snprintf(&s[0], 1023 + 1, "format_020a: %f, %f, %zu, %" PRIu64 ", %d\n",
                                                        fa + 1.0_f32, fb + 1.0_f32, sz1 + 1, a_u64 + 1_u64, j + 1);
                    PRAGMA_DISABLE_WARNING_POP
                    if( nchars < 1023 + 1 ) {
                        s.resize(nchars);
                        s.shrink_to_fit();
                    }  // else truncated w/ nchars > MaxStrLen
                    res = res + s.length();
                }
            }
        }
        return res;
    };
    BENCHMARK("fmt__020a_tsnprintf   bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            res = res + test_format(format_020a_tsnprintf, false);
        }
        return res;
    };
    BENCHMARK("format_030a_strstream bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            res = res + test_format(format_030a_strstream, false);
        }
        return res;
    };
#ifdef HAS_STD_FORMAT
    BENCHMARK("format_040a_stdformat bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < iterations; ++i ) {
            res = res + test_format(format_040a_stdformat, false);
        }
        return res;
    };
#endif

    BENCHMARK("format_000b_vsnprintf bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            res = res + test_format(format_000b_vsnprintf, false);
        }
        return res;
    };
    BENCHMARK("format_010b_vsnprintf bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            res = res + test_format(format_010b_vsnprintf, false);
        }
        return res;
    };
    BENCHMARK("format_020b__snprintf bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            res = res + test_format(format_020b_tsnprintf, false);
        }
        return res;
    };
    BENCHMARK("format_030b_strstream bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            res = res + test_format(format_030b_strstream, false);
        }
        return res;
    };
#ifdef HAS_STD_FORMAT
    BENCHMARK("format_040b_stdformat bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < iterations; ++i ) {
            res = res + test_format(format_040b_stdformat, false);
        }
        return res;
    };
#endif
}
