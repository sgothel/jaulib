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

/// Execute with `test_stringfmt01_perf --perf_analysis`
TEST_CASE("jau::cfmt benchmark", "[benchmark][jau][std::string][format_string]") {
    const size_t loops = 1000; // catch_auto_run ? 1000 : 1000;
    WARN("Benchmark with " + std::to_string(loops) + " loops");
    CHECK(true);

    static constexpr const char *format_check_exp = "format_check: 1.10, 2.20, 1, 2, 003";
    // static constexpr const char *format_check_exp2 = "format_check: 1.100000, 2.200000, 1, 2, 003";

    BENCHMARK("fmt1.01 check               bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            float fa = 1.1f, fb = 2.2f;
            size_t sz1 = 1;
            uint64_t sz2 = 2;
            int i1 = 3;

            ssize_t r = jau::cfmt::check("format_check: %.2f, %2.2f, %zu, %" PRIu64 ", %03d", fa, fb, sz1, sz2, i1);
            REQUIRE(5 == r);
            res = res + r;
        }
        return res;
    };
    BENCHMARK("fmt1.02 checkR              bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            float fa = 1.1f, fb = 2.2f;
            size_t sz1 = 1;
            uint64_t sz2 = 2;
            int i1 = 3;

            jau::cfmt::Result pc = jau::cfmt::checkR("format_check: %.2f, %2.2f, %zu, %" PRIu64 ", %03d", fa, fb, sz1, sz2, i1);
            REQUIRE(5 == pc.argumentCount());
            res = res + pc.argumentCount();
        }
        return res;
    };
    BENCHMARK("fmt1.10 check      cnstexpr bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            constexpr ssize_t r = jau::cfmt::check("format_check: %.2f, %2.2f, %zu, %" PRIu64 ", %03d", 1.0f, 2.0f, 3_uz, 64_u64, 1_i32);
            REQUIRE(5 == r);
            res = res + r;
        }
        return res;
    };
    BENCHMARK("fmt1.11 checkR     cnstexpr bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            constexpr  jau::cfmt::Result pc = jau::cfmt::checkR("format_check: %.2f, %2.2f, %zu, %" PRIu64 ", %03d", 1.0f, 2.0f, 3_uz, 64_u64, 1_i32);
            REQUIRE(5 == pc.argumentCount());
            res = res + pc.argumentCount();
        }
        return res;
    };
    BENCHMARK("fmt1.12 checkR     cnstexpr bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            constexpr float fa = 1.1f, fb = 2.2f;
            constexpr size_t sz1 = 1;
            constexpr uint64_t sz2 = 2;
            constexpr int i1 = 3;

            constexpr  jau::cfmt::Result pc = jau::cfmt::checkR("format_check: %.2f, %2.2f, %zu, %" PRIu64 ", %03d", fa, fb, sz1, sz2, i1);
            REQUIRE(5 == pc.argumentCount());
            res = res + pc.argumentCount();
        }
        return res;
    };
    BENCHMARK("fmt1.20 format-ckd   rsrved bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            constexpr float fa = 1.1f, fb = 2.2f;
            constexpr size_t sz1 = 1;
            constexpr uint64_t sz2 = 2;
            constexpr int i1 = 3;

            std::string s = jau_format_string("format_check: %.2f, %2.2f, %zu, %" PRIu64 ", %03d", fa, fb, sz1, sz2, i1);
            REQUIRE(format_check_exp == s);
            res = res + s.size();
        }
        return res;
    };
    BENCHMARK("fmt1.30 formatR      rsrved bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            float fa = 1.1f, fb = 2.2f;
            size_t sz1 = 1;
            uint64_t sz2 = 2;
            int i1 = 3;
            std::string s;
            s.reserve(jau::cfmt::default_string_capacity);

            jau::cfmt::formatR(s, "format_check: %.2f, %2.2f, %zu, %" PRIu64 ", %03d", fa, fb, sz1, sz2, i1);
            REQUIRE(format_check_exp == s);
            res = res + s.size();
        }
        return res;
    };
    BENCHMARK("fmt1.31 format       rsrved bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            float fa = 1.1f, fb = 2.2f;
            size_t sz1 = 1;
            uint64_t sz2 = 2;
            int i1 = 3;

            std::string s = jau::format_string("format_check: %.2f, %2.2f, %zu, %" PRIu64 ", %03d", fa, fb, sz1, sz2, i1);
            REQUIRE(format_check_exp == s);
            res = res + s.size();
        }
        return res;
    };
    BENCHMARK("fmtX.30 snprintf     rsrved bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            float fa = 1.1f, fb = 2.2f;
            size_t sz1 = 1;
            uint64_t sz2 = 2;
            int i1 = 3;
            std::string s;
            const size_t bsz = jau::cfmt::default_string_capacity + 1; // including EOS
            s.reserve(bsz);         // incl. EOS
            s.resize(bsz - 1);      // excl. EOS
            size_t nchars = std::snprintf(&s[0], bsz, "format_check: %.2f, %2.2f, %zu, %" PRIu64 ", %03d", fa, fb, sz1, sz2, i1);
            if( nchars < bsz ) {
                s.resize(nchars);
                s.shrink_to_fit();
            }
            REQUIRE(format_check_exp == s);
            res = res + s.size();
            REQUIRE(strlen(format_check_exp) == nchars);
            res = res + nchars;
        }
        return res;
    };
    BENCHMARK("fmt1.41 format              bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            float fa = 1.1f, fb = 2.2f;
            size_t sz1 = 1;
            uint64_t sz2 = 2;
            int i1 = 3;

            std::string s = jau::cfmt::format("format_check: %.2f, %2.2f, %zu, %" PRIu64 ", %03d", fa, fb, sz1, sz2, i1);
            REQUIRE(format_check_exp == s);
            res = res + s.size();
        }
        return res;
    };
    BENCHMARK("fmtX.50 stringstream        bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            float fa = 1.1f, fb = 2.2f;
            size_t sz1 = 1;
            uint64_t sz2 = 2;
            int i1 = 3;
            std::ostringstream ss1;
            ss1 << "format_check: "
                << fa << ", "
                << fb << ", "
                << sz1 << ", "
                << sz2 << ", "
                << i1;
            std::string s = ss1.str();
            REQUIRE("format_check: 1.1, 2.2, 1, 2, 3" == s);
            res = res + s.size();
        }
        return res;
    };
#ifdef HAS_STD_FORMAT
    BENCHMARK("fmtX.60 stdformat           bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            float fa = 1.1f, fb = 2.2f;
            size_t sz1 = 1;
            uint64_t sz2 = 2;
            int i1 = 3;
            std::string s = std::format("format_040b: {0:.2f}, {1:2.2f}, {3}, {4}, {5:03d}", fa, fb, sz1, sz2, i1);
            REQUIRE(format_check_exp == s);
            res = res + s.size();
        }
        return res;
    };
#endif
}
