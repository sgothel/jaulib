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
#include <limits>
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

/// Execute with `test_stringfmt_perf --perf-analysis`
TEST_CASE("jau_cfmt_benchmark_str1", "[benchmark][jau][std::string][format_string]") {
    const size_t loops = 1000; // catch_auto_run ? 1000 : 1000;
    WARN("Benchmark with " + std::to_string(loops) + " loops");
    CHECK(true);

    static constexpr const char *format_check_exp = "format_check: '  Hi World'";
    BENCHMARK("fmt1.32 format       rsrved bench") {
        std::string str1 = "Hi World";

        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            std::string s = jau::format_string("format_check: '%10s'", str1);
            REQUIRE(format_check_exp == s);
            res = res + s.size();
        }
        return res;
    };
    BENCHMARK("fmtX.32 snprintf     rsrved bench") {
        std::string str1 = "Hi World";

        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            std::string s;
            const size_t bsz = jau::cfmt::default_string_capacity + 1; // including EOS
            s.reserve(bsz);         // incl. EOS
            s.resize(bsz - 1);      // excl. EOS
            size_t nchars = std::snprintf(&s[0], bsz, "format_check: '%10s'", str1.c_str());
            if( nchars < bsz ) {
                s.resize(nchars);
            }
            REQUIRE(format_check_exp == s);
            res = res + nchars;
        }
        return res;
    };
}

TEST_CASE("jau_cfmt_benchmark_str2", "[benchmark][jau][std::string][format_string]") {
    const size_t loops = 1000; // catch_auto_run ? 1000 : 1000;
    WARN("Benchmark with " + std::to_string(loops) + " loops");
    CHECK(true);

    static constexpr const char *format_check_exp = "format_check: 003, '  Hi World'";
    BENCHMARK("fmt1.32 format       rsrved bench") {
        int i1 = 3;
        std::string str1 = "Hi World";

        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            std::string s = jau::format_string("format_check: %03d, '%10s'", i1, str1);
            REQUIRE(format_check_exp == s);
            res = res + s.size();
        }
        return res;
    };
    BENCHMARK("fmtX.32 snprintf     rsrved bench") {
        int i1 = 3;
        std::string str1 = "Hi World";

        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            std::string s;
            const size_t bsz = jau::cfmt::default_string_capacity + 1; // including EOS
            s.reserve(bsz);         // incl. EOS
            s.resize(bsz - 1);      // excl. EOS
            size_t nchars = std::snprintf(&s[0], bsz, "format_check: %03d, '%10s'", i1, str1.c_str());
            if( nchars < bsz ) {
                s.resize(nchars);
            }
            REQUIRE(format_check_exp == s);
            res = res + nchars;
        }
        return res;
    };
}

TEST_CASE("jau_cfmt_benchmark_all", "[benchmark][jau][std::string][format_string]") {
    const size_t loops = 1000; // catch_auto_run ? 1000 : 1000;
    WARN("Benchmark with " + std::to_string(loops) + " loops");
    CHECK(true);

    static constexpr const char *format_check_exp = "format_check: 1.10, 2.20, 1, 2, 003,   Hi World";

    BENCHMARK("fmt1.01 check               bench") {
        float fa = 1.1f, fb = 2.2f;
        size_t sz1 = 1;
        uint64_t sz2 = 2;
        int i1 = 3;
        std::string str1 = "Hi World";

        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            ssize_t r = jau::cfmt::check("format_check: %.2f, %2.2f, %zu, %" PRIu64 ", %03d, %10s", fa, fb, sz1, sz2, i1, str1);
            REQUIRE(6 == r);
            res = res + r;
        }
        return res;
    };
    BENCHMARK("fmt1.02 checkR              bench") {
        float fa = 1.1f, fb = 2.2f;
        size_t sz1 = 1;
        uint64_t sz2 = 2;
        int i1 = 3;
        std::string str1 = "Hi World";

        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            jau::cfmt::Result pc = jau::cfmt::checkR("format_check: %.2f, %2.2f, %zu, %" PRIu64 ", %03d, %10s", fa, fb, sz1, sz2, i1, str1);
            REQUIRE(6 == pc.argumentCount());
            res = res + pc.argumentCount();
        }
        return res;
    };
    BENCHMARK("fmt1.20 format-ckd   rsrved bench") {
        float fa = 1.1f, fb = 2.2f;
        size_t sz1 = 1;
        uint64_t sz2 = 2;
        int i1 = 3;
        std::string str1 = "Hi World";

        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            std::string s = jau_format_string("format_check: %.2f, %2.2f, %zu, %" PRIu64 ", %03d, %10s", fa, fb, sz1, sz2, i1, str1);
            REQUIRE(format_check_exp == s);
            res = res + s.size();
        }
        return res;
    };
    BENCHMARK("fmt1.30 formatR      rsrved bench") {
        float fa = 1.1f, fb = 2.2f;
        size_t sz1 = 1;
        uint64_t sz2 = 2;
        int i1 = 3;
        std::string str1 = "Hi World";

        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            std::string s;
            s.reserve(jau::cfmt::default_string_capacity+1);

            jau::cfmt::formatR(s, "format_check: %.2f, %2.2f, %zu, %" PRIu64 ", %03d, %10s", fa, fb, sz1, sz2, i1, str1);
            REQUIRE(format_check_exp == s);
            res = res + s.size();
        }
        return res;
    };
    BENCHMARK("fmt1.32 format       rsrved bench") {
        float fa = 1.1f, fb = 2.2f;
        size_t sz1 = 1;
        uint64_t sz2 = 2;
        int i1 = 3;
        std::string str1 = "Hi World";

        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            std::string s = jau::format_string("format_check: %.2f, %2.2f, %zu, %" PRIu64 ", %03d, %10s", fa, fb, sz1, sz2, i1, str1);
            REQUIRE(format_check_exp == s);
            res = res + s.size();
        }
        return res;
    };
    BENCHMARK("fmtX.32 snprintf     rsrved bench") {
        float fa = 1.1f, fb = 2.2f;
        size_t sz1 = 1;
        uint64_t sz2 = 2;
        int i1 = 3;
        std::string str1 = "Hi World";

        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            std::string s;
            const size_t bsz = jau::cfmt::default_string_capacity + 1; // including EOS
            s.reserve(bsz);         // incl. EOS
            s.resize(bsz - 1);      // excl. EOS
            size_t nchars = std::snprintf(&s[0], bsz, "format_check: %.2f, %2.2f, %zu, %" PRIu64 ", %03d, %10s", fa, fb, sz1, sz2, i1, str1.c_str());
            if( nchars < bsz ) {
                s.resize(nchars);
            }
            REQUIRE(format_check_exp == s);
            res = res + nchars;
        }
        return res;
    };
    BENCHMARK("fmt1.42 format              bench") {
        float fa = 1.1f, fb = 2.2f;
        size_t sz1 = 1;
        uint64_t sz2 = 2;
        int i1 = 3;
        std::string str1 = "Hi World";

        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            // fa += 0.01f; fb += 0.02f; ++sz1; ++i1; str1.append("X");
            std::string s = jau::cfmt::format("format_check: %.2f, %2.2f, %zu, %" PRIu64 ", %03d, %10s", fa, fb, sz1, sz2, i1, str1);
            REQUIRE(format_check_exp == s);
            res = res + s.size();
        }
        return res;
    };
    BENCHMARK("fmtX.50 stringstream        bench") {
        float fa = 1.1f, fb = 2.2f;
        size_t sz1 = 1;
        uint64_t sz2 = 2;
        int i1 = 3;
        std::string str1 = "Hi World";

        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            std::ostringstream ss1;
            ss1 << "format_check: "
                << fa << ", "
                << fb << ", "
                << sz1 << ", "
                << sz2 << ", "
                << i1 << ", "
                << str1;
            std::string s = ss1.str();
            REQUIRE("format_check: 1.1, 2.2, 1, 2, 3, Hi World" == s);
            res = res + s.size();
        }
        return res;
    };
#ifdef HAS_STD_FORMAT
    BENCHMARK("fmtX.60 stdformat           bench") {
        float fa = 1.1f, fb = 2.2f;
        size_t sz1 = 1;
        uint64_t sz2 = 2;
        int i1 = 3;
        std::string str1 = "Hi World";

        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            std::string s = std::format("format_040b: {0:.2f}, {1:2.2f}, {3}, {4}, {5:03d}, {6:10s}", fa, fb, sz1, sz2, i1, str1);
            REQUIRE(format_check_exp == s);
            res = res + s.size();
        }
        return res;
    };
#endif
}

