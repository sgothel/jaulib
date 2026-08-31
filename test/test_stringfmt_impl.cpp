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
#include <jau/debug.hpp>
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

TEST_CASE("jau_cfmt_append_integral01", "[benchmark][jau][std::string][format_string]") {
    // const uint64_t i1 = std::numeric_limits<uint64_t>::max(); // Value = 18446744073709551615 (0xffffffffffffffff)
    // static constexpr const char *format_check_exp1 = "    0000000018'446'744'073'709'551'615";
    // static constexpr const char *format_check_exp0 = "    0000000000000018446744073709551615";

    {
        // Case 1: clang-analyzer-core.uninitialized.Assign: buf_ was written up-to val_digits! (Assigned value is garbage or undefined)
        // This analysis issue was only produced with preset `debug-clang`, not `release-clang`.
        static constexpr const char *format_exp = "-0";
        // radix 2
        // no flags_t::uppercase
        // no flags_t::thousands
        // no width
        // no precision
        // value = 0 ??? -> val_digits 1
        // negative = true
        // no inject_dot
        const uint64_t value = 0;
        const bool negative = true;
        const bool inject_dot = false;
        jau::cfmt::FormatOpts opts;
        opts.length_mod = jau::cfmt::plength_t::z;
        opts.setConversion('b');
        std::cout << "FormatOpts: " << opts << "\n";
        REQUIRE(2 == opts.radix);

        {
            std::string s;
            s.reserve(jau::cfmt::default_string_capacity + 1);

            jau::cfmt::impl::append_integral(s, s.max_size(), value, negative, opts, inject_dot);
            REQUIRE(format_exp == s);
        }
        {
            std::string s;
            s.reserve(jau::cfmt::default_string_capacity + 1);

            jau::cfmt::impl::append_integral_simple(s, s.max_size(), value, negative, opts);
            REQUIRE(format_exp == s);
        }
    }
}

template<typename F>
static void testASSafeCappedAppend(std::string_view tag, F func) {
    constexpr size_t init_cap = 20;
    std::string s_cap;
    s_cap.reserve(init_cap);
    const void * const s_cap_data = (void*)s_cap.data();
    const size_t       s_cap_cap  = s_cap.capacity();
    jau_fprintf(stdout, "%s 01: s_cap: len %3zu, cap %4zu, data %p, '%s'\n", tag, s_cap.length(), s_cap.capacity(), s_cap.data(), s_cap);
    REQUIRE(s_cap_cap >= init_cap);

    // jau::cfmtppend_cap(s_cap, "0123456789ABCDEFGHIJKLMNOPQR
    // jau::cfmt::append_cap(s_cap, "0123456789ABCDEFGHIJKL %d", 1234567890); // `_0123456789ABCDEFGHIJKLMNOPQR` 29
    func(s_cap);
    jau_fprintf(stdout, "%s 10: s_cap: len %3zu, cap %4zu, data %p, '%s'\n", tag, s_cap.length(), s_cap.capacity(), s_cap.data(), s_cap);

    REQUIRE(s_cap.length()   == s_cap_cap-1);
    REQUIRE(s_cap.capacity() == s_cap_cap);
    REQUIRE((void*)s_cap.data() == s_cap_data);
}

TEST_CASE("jau_cfmt_append_cap", "[as-safe][jau][std::string][format_string]") {
    constexpr size_t init_cap = 20;
    {
        std::string s_grw, s_cap;
        jau_fprintf(stdout, "00: s_grw: len %3zu, cap %4zu, data %p, '%s'\n", s_grw.length(), s_grw.capacity(), s_grw.data(), s_grw);
        jau_fprintf(stdout, "00: s_cap: len %3zu, cap %4zu, data %p, '%s'\n", s_cap.length(), s_cap.capacity(), s_cap.data(), s_cap);
        s_grw.reserve(init_cap);
        s_cap.reserve(init_cap);
        const size_t       s_grw_cap  = s_grw.capacity();
        const void * const s_cap_data = (void*)s_cap.data();
        const size_t       s_cap_cap  = s_cap.capacity();
        jau_fprintf(stdout, "01: s_grw: len %3zu, cap %4zu, data %p, '%s'\n", s_grw.length(), s_grw.capacity(), s_grw.data(), s_grw);
        jau_fprintf(stdout, "01: s_cap: len %3zu, cap %4zu, data %p, '%s'\n", s_cap.length(), s_cap.capacity(), s_cap.data(), s_cap);
        REQUIRE(s_grw_cap >= init_cap);
        REQUIRE(s_cap_cap >= init_cap);

        s_grw.resize(1, '_');
        s_cap.resize(1, '_');
        jau_fprintf(stdout, "02: s_grw: len %3zu, cap %4zu, data %p, '%s'\n", s_grw.length(), s_grw.capacity(), s_grw.data(), s_grw);
        jau_fprintf(stdout, "02: s_cap: len %3zu, cap %4zu, data %p, '%s'\n", s_cap.length(), s_cap.capacity(), s_cap.data(), s_cap);

        REQUIRE(s_grw.length() == 1);
        REQUIRE(s_cap.length() == 1);
        REQUIRE(s_grw.capacity() >= s_grw_cap);
        REQUIRE(s_cap.capacity() == s_cap_cap);
        REQUIRE((void*)s_cap.data() == s_cap_data);

        jau::cfmt::append(    s_grw, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
        jau::cfmt::append_cap(s_cap, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"); // `_0123456789ABCDEFGHIJKLMNOPQR` 29
        jau_fprintf(stdout, "10: s_grw: len %3zu, cap %4zu, data %p, '%s'\n", s_grw.length(), s_grw.capacity(), s_grw.data(), s_grw);
        jau_fprintf(stdout, "10: s_cap: len %3zu, cap %4zu, data %p, '%s'\n", s_cap.length(), s_cap.capacity(), s_cap.data(), s_cap);

        REQUIRE(s_grw.length()   > s_grw_cap);
        REQUIRE(s_grw.capacity() > s_grw_cap);
        REQUIRE(s_cap.length()   == s_cap_cap-1);
        REQUIRE(s_cap.capacity() == s_cap_cap);
        REQUIRE((void*)s_cap.data() == s_cap_data);
    }
    testASSafeCappedAppend("a01", [](std::string &s){
        jau::cfmt::append_cap(s, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");  // `_0123456789ABCDEFGHIJKLMNOPQR` 29
    });
    testASSafeCappedAppend("a02", [](std::string &s){
        jau::cfmt::append_cap(s, "0123456789ABCDEFGHIJKL %s", "abcdefghijklmnopqrstuvwxyz");
    });
    testASSafeCappedAppend("a03", [](std::string &s){
        jau::cfmt::append_cap(s, "0123456789ABCDEFGHIJKL %10.10s", "abcdefghijklmnopqrstuvwxyz");
    });
    testASSafeCappedAppend("i01", [](std::string &s){
        jau::cfmt::append_cap(s, "0123456789ABCDEFGHIJKL %d", 1234567890);
    });
    testASSafeCappedAppend("i02", [](std::string &s){
        jau::cfmt::append_cap(s, "0123456789ABCDEFGHIJKL %10.10d", 1234567890);
    });
    testASSafeCappedAppend("i03", [](std::string &s){
        jau::cfmt::append_cap(s, "0123456789ABCDEFGHIJKL %-10.10d", 1234567890);
    });
    testASSafeCappedAppend("f01", [](std::string &s){
        jau::cfmt::append_cap(s, "0123456789ABCDEFGHIJKLMN %.2f", 1.23);
    });
    testASSafeCappedAppend("f01", [](std::string &s){
        jau::cfmt::append_cap(s, "0123456789ABCDEFGHIJKLMNO %.2f", 1.23);
    });
    testASSafeCappedAppend("f02", [](std::string &s){
        jau::cfmt::append_cap(s, "0123456789ABCDEFGHIJKLMNO %5.3f", 1.23f);
    });

    testASSafeCappedAppend("f03", [](std::string &s){
        jau::cfmt::append_cap(s, "0123456789ABCDEFGHIJKL %f", 1234567890.0f);
    });
    testASSafeCappedAppend("f04", [](std::string &s){
        jau::cfmt::append_cap(s, "0123456789ABCDEFGHIJKL %10.10f", 1234567890.0f);
    });


}