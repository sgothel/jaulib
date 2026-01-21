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
#include <limits>

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

/// Execute with `test_stringfmt_perf_impl --perf-analysis`

static uint32_t digits10_loop0(uint64_t v) noexcept {
    static constexpr const size_t char32buf_maxlen = 32;
    char buf_[char32buf_maxlen];
    char * d = buf_;
    const char * const d_end_num = d + char32buf_maxlen;
    uint32_t l = 0;
    do {
        ++l;
        v /= 10;
    } while( v && d < d_end_num);
    return l;
}
static uint32_t digits10_loop1(uint64_t v) noexcept {
    static constexpr const size_t char32buf_maxlen = 32;
    char buf_[char32buf_maxlen];
    char * d = buf_;
    const char * const d_end_num = d + char32buf_maxlen;
    do {
        *(d++) = char('0' + (v % 10_u64));
        v /= 10;
    } while( v && d < d_end_num);
    return uint32_t(d - buf_);
}

TEST_CASE("fast_log_benchmark_digits10", "[benchmark][jau][math][log]") {
    const size_t loops = 1000; // catch_auto_run ? 1000 : 1000;
    WARN("Benchmark with " + std::to_string(loops) + " loops");
    CHECK(true);

    const double log2_10 = std::log2<uint64_t>(10);
    const uint64_t i1 = std::numeric_limits<uint64_t>::max(); // Value = 18446744073709551615 (0xffffffffffffffff)
    const uint32_t i1_d10 = 20;

    BENCHMARK("O(n) loop0            bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            uint32_t l = digits10_loop0(i1);
            REQUIRE(i1_d10 == l);
            res = res + l;
        }
        return res;
    };
    BENCHMARK("O(n) loop1            bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            uint32_t l = digits10_loop1(i1);
            REQUIRE(i1_d10 == l);
            res = res + l;
        }
        return res;
    };
    BENCHMARK("log10(x)              bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            uint32_t l = 1 + static_cast<uint32_t>(std::log10<uint64_t>(i1));
            REQUIRE(i1_d10 == l);
            res = res + l;
        }
        return res;
    };
    BENCHMARK("log2(x)/log2(10)      bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            uint32_t l = 1 + static_cast<uint32_t>(std::log2<uint64_t>(i1) / log2_10);
            REQUIRE(i1_d10 == l);
            res = res + l;
        }
        return res;
    };
}

TEST_CASE("jau_cfmt_benchmark_append_integral00", "[benchmark][jau][std::string][format_string]") {
    const size_t loops = 1000; // catch_auto_run ? 1000 : 1000;
    WARN("Benchmark with " + std::to_string(loops) + " loops");
    CHECK(true);

    const uint64_t i1 = std::numeric_limits<uint64_t>::max(); // Value = 18446744073709551615 (0xffffffffffffffff)
    static constexpr const char *format_check_exp = "18446744073709551615";
    jau::cfmt::FormatOpts o1;
    o1.length_mod = jau::cfmt::plength_t::z;
    // opts.addFlag('0');
    o1.setConversion('u');
    std::cout << "flags: " << o1 << "\n";

    {
        std::string s;
        s.reserve(jau::cfmt::default_string_capacity + 1);

        jau::cfmt::impl::append_integral(s, s.max_size(), i1, false, o1, false);
        REQUIRE(format_check_exp == s);
    }

    BENCHMARK("append_integral      rsrved bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            std::string s;
            s.reserve(jau::cfmt::default_string_capacity+1);

            jau::cfmt::impl::append_integral(s, s.max_size(), i1, false, o1, false);
            REQUIRE(format_check_exp == s);
            res = res + s.size();
        }
        return res;
    };

    BENCHMARK("snprintf             rsrved bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            std::string s;
            const size_t bsz = jau::cfmt::default_string_capacity + 1; // including EOS
            s.reserve(bsz);         // incl. EOS
            s.resize(bsz - 1);      // excl. EOS
            size_t nchars = std::snprintf(&s[0], bsz, "%zu", i1);
            if( nchars < bsz ) {
                s.resize(nchars);
            }
            REQUIRE(format_check_exp == s);
            res = res + nchars;
        }
        return res;
    };
}

TEST_CASE("jau_cfmt_benchmark_append_integral01", "[benchmark][jau][std::string][format_string]") {
    const size_t loops = 1000; // catch_auto_run ? 1000 : 1000;
    WARN("Benchmark with " + std::to_string(loops) + " loops");
    CHECK(true);

    const uint64_t i1 = std::numeric_limits<uint64_t>::max(); // Value = 18446744073709551615 (0xffffffffffffffff)
    static constexpr const char *format_check_exp1 = "    0000000018'446'744'073'709'551'615";
    static constexpr const char *format_check_exp0 = "    0000000000000018446744073709551615";
    jau::cfmt::FormatOpts o1;
    o1.length_mod = jau::cfmt::plength_t::z;
    o1.addFlag('\'');
    o1.setWidth(38);
    o1.setPrecision(34);
    o1.setConversion('u');
    std::cout << "flags: " << o1 << "\n";

    {
        std::string s;
        s.reserve(jau::cfmt::default_string_capacity + 1);

        jau::cfmt::impl::append_integral(s, s.max_size(), i1, false, o1, false);
        REQUIRE(format_check_exp1 == s);
    }

    BENCHMARK("append_integral      rsrved bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            std::string s;
            s.reserve(jau::cfmt::default_string_capacity+1);

            jau::cfmt::impl::append_integral(s, s.max_size(), i1, false, o1, false);
            REQUIRE(format_check_exp1 == s);
            res = res + s.size();
        }
        return res;
    };

    BENCHMARK("snprintf             rsrved bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            std::string s;
            const size_t bsz = jau::cfmt::default_string_capacity + 1; // including EOS
            s.reserve(bsz);         // incl. EOS
            s.resize(bsz - 1);      // excl. EOS
            size_t nchars = std::snprintf(&s[0], bsz, "%38.34zu", i1);
            if( nchars < bsz ) {
                s.resize(nchars);
            }
            REQUIRE(format_check_exp0 == s);
            res = res + nchars;
        }
        return res;
    };
}
