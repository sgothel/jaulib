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
#include <cstdio>
#include <cstring>
#include <iomanip>
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

TEST_CASE("jau_cfmt_benchmark_int0", "[benchmark][jau][std::string][format_int]") {
    const size_t loops = 1000; // catch_auto_run ? 1000 : 1000;
    WARN("Benchmark with " + std::to_string(loops) + " loops");
    CHECK(true);

    static constexpr const char *format_check_exp = "format_check: 3";
    BENCHMARK("fmt1.32 format       rsrved bench") {
        int i1 = 3;

        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            std::string s = jau::format_string("format_check: %d", i1);
            REQUIRE(format_check_exp == s);
            res = res + s.size();
        }
        return res;
    };
    BENCHMARK("fmtX.32 snprintf     rsrved bench") {
        int i1 = 3;

        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            std::string s;
            const size_t bsz = jau::cfmt::default_string_capacity + 1; // including EOS
            s.reserve(bsz);         // incl. EOS
            s.resize(bsz - 1);      // excl. EOS
            size_t nchars = std::snprintf(&s[0], bsz, "format_check: %d", i1);
            if( nchars < bsz ) {
                s.resize(nchars);
            }
            REQUIRE(format_check_exp == s);
            res = res + nchars;
        }
        return res;
    };
}

TEST_CASE("jau_cfmt_benchmark_int1", "[benchmark][jau][std::string][format_int]") {
    const size_t loops = 1000; // catch_auto_run ? 1000 : 1000;
    WARN("Benchmark with " + std::to_string(loops) + " loops");
    CHECK(true);

    static constexpr const char *format_check_exp = "format_check: 003";
    BENCHMARK("fmt1.32 format       rsrved bench") {
        int i1 = 3;

        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            std::string s = jau::format_string("format_check: %03d", i1);
            REQUIRE(format_check_exp == s);
            res = res + s.size();
        }
        return res;
    };
    BENCHMARK("fmtX.32 snprintf     rsrved bench") {
        int i1 = 3;

        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            std::string s;
            const size_t bsz = jau::cfmt::default_string_capacity + 1; // including EOS
            s.reserve(bsz);         // incl. EOS
            s.resize(bsz - 1);      // excl. EOS
            size_t nchars = std::snprintf(&s[0], bsz, "format_check: %03d", i1);
            if( nchars < bsz ) {
                s.resize(nchars);
            }
            REQUIRE(format_check_exp == s);
            res = res + nchars;
        }
        return res;
    };
}

/// Execute with `test_stringfmt_perf --perf-analysis`
TEST_CASE("jau_cfmt_benchmark_int2", "[benchmark][jau][std::string][format_int]") {
    const size_t loops = 1000; // catch_auto_run ? 1000 : 1000;
    WARN("Benchmark with " + std::to_string(loops) + " loops");
    CHECK(true);

    static constexpr const std::string_view format_check_exp1 = "format_check: -1, 2";
    int i1=-1;
    size_t i2=2;

    BENCHMARK("fmt1.130 formatR      rsrved bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            std::string s;
            s.reserve(jau::cfmt::default_string_capacity+1);

            jau::cfmt::formatR(s, "format_check: %d, %zu", i1, i2);
            REQUIRE(format_check_exp1 == s);
            res = res + s.size();
        }
        return res;
    };
    BENCHMARK("fmt1.132 format       rsrved bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            std::string s = jau::format_string("format_check: %d, %zu", i1, i2);
            REQUIRE(format_check_exp1 == s);
            res = res + s.size();
        }
        return res;
    };
    BENCHMARK("fmtX.132 snprintf     rsrved bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            std::string s;
            const size_t bsz = jau::cfmt::default_string_capacity + 1; // including EOS
            s.reserve(bsz);         // incl. EOS
            s.resize(bsz - 1);      // excl. EOS
            size_t nchars = std::snprintf(&s[0], bsz, "format_check: %d, %zu", i1, i2);
            if( nchars < bsz ) {
                s.resize(nchars);
            }
            REQUIRE(format_check_exp1 == s);
            res = res + nchars;
        }
        return res;
    };
    BENCHMARK("fmt1.142 format              bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            // fa += 0.01f; fb += 0.02f; ++sz1; ++i1; str1.append("X");
            std::string s = jau::cfmt::format("format_check: %d, %zu", i1, i2);
            REQUIRE(format_check_exp1 == s);
            res = res + s.size();
        }
        return res;
    };

}

TEST_CASE("jau_cfmt_benchmark_int_all", "[benchmark][jau][std::string][format_int]") {
    const size_t loops = 1000; // catch_auto_run ? 1000 : 1000;
    WARN("Benchmark with " + std::to_string(loops) + " loops");
    CHECK(true);

    static constexpr const std::string_view format_check_exp1 = "format_check: -1, 2, -3, 4, -5, 6, -7, 8, -9, 10";
    static constexpr const std::string_view format_check_exp2 = "format_check: -1, 02, -03, 0004, -0005, 000006, -000007, 00000008, -00000009, 0000000010";
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

    BENCHMARK("fmt1.130 formatR      rsrved bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            std::string s;
            s.reserve(jau::cfmt::default_string_capacity+1);

            jau::cfmt::formatR(s, "format_check: %hhd, %hhu, %hd, %hu, %d, %u, %ld, %lu, %zd, %zu", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
            REQUIRE(format_check_exp1 == s);
            res = res + s.size();
        }
        return res;
    };
    BENCHMARK("fmt1.132 format       rsrved bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            std::string s = jau::format_string("format_check: %hhd, %hhu, %hd, %hu, %d, %u, %ld, %lu, %zd, %zu", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
            REQUIRE(format_check_exp1 == s);
            res = res + s.size();
        }
        return res;
    };
    BENCHMARK("fmtX.132 snprintf     rsrved bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            std::string s;
            const size_t bsz = jau::cfmt::default_string_capacity + 1; // including EOS
            s.reserve(bsz);         // incl. EOS
            s.resize(bsz - 1);      // excl. EOS
            size_t nchars = std::snprintf(&s[0], bsz, "format_check: %hhd, %hhu, %hd, %hu, %d, %u, %ld, %lu, %zd, %zu", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
            if( nchars < bsz ) {
                s.resize(nchars);
            }
            REQUIRE(format_check_exp1 == s);
            res = res + nchars;
        }
        return res;
    };
    BENCHMARK("fmt1.142 format              bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            // fa += 0.01f; fb += 0.02f; ++sz1; ++i1; str1.append("X");
            std::string s = jau::cfmt::format("format_check: %hhd, %hhu, %hd, %hu, %d, %u, %ld, %lu, %zd, %zu", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
            REQUIRE(format_check_exp1 == s);
            res = res + s.size();
        }
        return res;
    };

    BENCHMARK("fmtX.150 stringstream        bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            std::ostringstream ss1;
            ss1 << "format_check: "
                << int(i1) << ", "
                << unsigned(i2) << ", "
                << i3 << ", "
                << i4 << ", "
                << i5 << ", "
                << i6 << ", "
                << i7 << ", "
                << i8 << ", "
                << i9 << ", "
                << i10;
            std::string s = ss1.str();
            REQUIRE(format_check_exp1 == s);
            res = res + s.size();
        }
        return res;
    };

    ///
    ///

    BENCHMARK("fmt1.230 formatR      rsrved bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            std::string s;
            s.reserve(jau::cfmt::default_string_capacity+1);

            jau::cfmt::formatR(s, "format_check: %01hhd, %02hhu, %03hd, %04hu, %05d, %06u, %07ld, %08lu, %09zd, %010zu", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
            REQUIRE(format_check_exp2 == s);
            res = res + s.size();
        }
        return res;
    };
    BENCHMARK("fmt1.232 format       rsrved bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            std::string s = jau::format_string("format_check: %01hhd, %02hhu, %03hd, %04hu, %05d, %06u, %07ld, %08lu, %09zd, %010zu", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
            REQUIRE(format_check_exp2 == s);
            res = res + s.size();
        }
        return res;
    };
    BENCHMARK("fmtX.232 snprintf     rsrved bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            std::string s;
            const size_t bsz = jau::cfmt::default_string_capacity + 1; // including EOS
            s.reserve(bsz);         // incl. EOS
            s.resize(bsz - 1);      // excl. EOS
            size_t nchars = std::snprintf(&s[0], bsz, "format_check: %01hhd, %02hhu, %03hd, %04hu, %05d, %06u, %07ld, %08lu, %09zd, %010zu", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
            if( nchars < bsz ) {
                s.resize(nchars);
            }
            REQUIRE(format_check_exp2 == s);
            res = res + nchars;
        }
        return res;
    };
    BENCHMARK("fmt1.242 format              bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            // fa += 0.01f; fb += 0.02f; ++sz1; ++i1; str1.append("X");
            std::string s = jau::cfmt::format("format_check: %01hhd, %02hhu, %03hd, %04hu, %05d, %06u, %07ld, %08lu, %09zd, %010zu", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
            REQUIRE(format_check_exp2 == s);
            res = res + s.size();
        }
        return res;
    };

    BENCHMARK("fmtX.250 stringstream        bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            std::ostringstream ss1;
            ss1 << "format_check: "
                << std::setfill('0') // undefined with negative numbers, duh!
                << "-" << std::setw(0) << int(jau::abs(i1)) << ", "
                << std::setw(2) << unsigned(i2) << ", "
                << "-" << std::setw(3-1) << jau::abs(i3) << ", "
                << std::setw(4) << i4 << ", "
                << "-" << std::setw(5-1) << jau::abs(i5) << ", "
                << std::setw(6) << i6 << ", "
                << "-" << std::setw(7-1) << jau::abs(i7) << ", "
                << std::setw(8) << i8 << ", "
                << "-" << std::setw(9-1) << jau::abs(i9) << ", "
                << std::setw(10) << i10;
            std::string s = ss1.str();
            REQUIRE(format_check_exp2 == s);
            res = res + s.size();
        }
        return res;
    };

}
