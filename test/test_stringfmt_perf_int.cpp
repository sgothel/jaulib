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
            std::string s = jau::cfmt::format("format_check: %d", i1);
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
    BENCHMARK("fmt1.33 format       rsrved bench") {
        int i1 = 3;

        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            std::string s = jau::cfmt::format("format_check: %03d", i1);
            REQUIRE(format_check_exp == s);
            res = res + s.size();
        }
        return res;
    };
    BENCHMARK("fmtX.33 snprintf     rsrved bench") {
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

    BENCHMARK("fmt1.34 formatR      rsrved bench") {
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
    BENCHMARK("fmt1.34 format       rsrved bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            std::string s = jau::cfmt::format("format_check: %d, %zu", i1, i2);
            REQUIRE(format_check_exp1 == s);
            res = res + s.size();
        }
        return res;
    };
    BENCHMARK("fmtX.34 snprintf     rsrved bench") {
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
    BENCHMARK("fmt1.34 format              bench") {
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

    static constexpr const std::string_view format_check_exp1 = "format_check: -127, 127, -32000, 32000, -1073741823, 2147483647, -1073741823, 2147483647, -1073741823, 2147483647";
    static constexpr const std::string_view format_check_exp2 = "format_check: -0127, 00127, -000032000, 0000032000, -00001073741823, 000002147483647, -00001073741823, 000002147483647, -00001073741823, 000002147483647";

    static constexpr const std::string_view format_check_expB1 = "format_check: 2147483647, 2147483647, 2147483647, 2147483647, 2147483647, 2147483647, 2147483647, 2147483647, 2147483647, 2147483647";
    static constexpr const std::string_view format_check_expB2 = "format_check: 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff";
    static constexpr const std::string_view format_check_expC1 = "format_check: 000002147483647, 000002147483647, 000002147483647, 000002147483647, 000002147483647, 000002147483647, 000002147483647, 000002147483647, 000002147483647, 000002147483647";
    static constexpr const std::string_view format_check_expC2 = "format_check: 0x000007fffffff, 0x000007fffffff, 0x000007fffffff, 0x000007fffffff, 0x000007fffffff, 0x000007fffffff, 0x000007fffffff, 0x000007fffffff, 0x000007fffffff, 0x000007fffffff";
    char i1=-127;
    unsigned char i2=127;

    short i3=-32000;
    unsigned short i4=32000;

    int i5=-std::numeric_limits<int32_t>::max()/2;
    unsigned int i6=std::numeric_limits<uint32_t>::max()/2;

    long i7=-std::numeric_limits<int32_t>::max()/2;
    unsigned long i8=std::numeric_limits<uint32_t>::max()/2;

    ssize_t i9 = -std::numeric_limits<int32_t>::max()/2;
    size_t i10 = std::numeric_limits<uint32_t>::max()/2;

    uint32_t u32 = std::numeric_limits<uint32_t>::max()/2; // 2147483647
    uint64_t u64 = std::numeric_limits<uint32_t>::max()/2;

    const size_t bsz = jau::cfmt::default_string_capacity + 1; // including EOS
    std::string reserved;
    reserved.reserve(bsz);         // incl. EOS

    BENCHMARK("fmt1.130 format              bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            // fa += 0.01f; fb += 0.02f; ++sz1; ++i1; str1.append("X");
            std::string s = jau::cfmt::format("format_check: %d, %u, %d, %u, %d, %u, %d, %u, %d, %u", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
            REQUIRE(format_check_exp1 == s);
            res = res + s.size();
        }
        return res;
    };
    BENCHMARK("fmt1.130 formatR      rsrved bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            reserved.clear();
            jau::cfmt::formatR(reserved, "format_check: %d, %u, %d, %u, %d, %u, %d, %u, %d, %u", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
            REQUIRE(format_check_exp1 == reserved);
            res = res + reserved.size();
        }
        return res;
    };
    BENCHMARK("fmt1.130 append +len  rsrved bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            reserved.clear();
            jau::cfmt::append(reserved, "format_check: %hhd, %hhu, %hd, %hu, %d, %u, %ld, %lu, %zd, %zu", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
            REQUIRE(format_check_exp1 == reserved);
            res = res + reserved.size();
        }
        return res;
    };
    BENCHMARK("fmt1.130 append       rsrved bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            reserved.clear();
            jau::cfmt::append(reserved, "format_check: %d, %u, %d, %u, %d, %u, %d, %u, %d, %u", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
            REQUIRE(format_check_exp1 == reserved);
            res = res + reserved.size();
        }
        return res;
    };
    BENCHMARK("fmt1.130 append 32    rsrved bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            reserved.clear();
            jau::cfmt::append(reserved, "format_check: %u, %u, %u, %u, %u, %u, %u, %u, %u, %u", u32, u32, u32, u32, u32, u32, u32, u32, u32, u32);
            res = res + reserved.size();
            REQUIRE(format_check_expB1 == reserved);
        }
        return res;
    };
    BENCHMARK("fmt1.130 append 64    rsrved bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            reserved.clear();
            jau::cfmt::append(reserved, "format_check: %u, %u, %u, %u, %u, %u, %u, %u, %u, %u", u64, u64, u64, u64, u64, u64, u64, u64, u64, u64);
            res = res + reserved.size();
            REQUIRE(format_check_expB1 == reserved);
        }
        return res;
    };
    BENCHMARK("fmt1.131 append 32    rsrved bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            reserved.clear();
            jau::cfmt::append(reserved, "format_check: %#x, %#x, %#x, %#x, %#x, %#x, %#x, %#x, %#x, %#x", u32, u32, u32, u32, u32, u32, u32, u32, u32, u32);
            REQUIRE(format_check_expB2 == reserved);
            res = res + reserved.size();
        }
        return res;
    };
    BENCHMARK("fmt1.131 append 64    rsrved bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            reserved.clear();
            jau::cfmt::append(reserved, "format_check: %#x, %#x, %#x, %#x, %#x, %#x, %#x, %#x, %#x, %#x", u64, u64, u64, u64, u64, u64, u64, u64, u64, u64);
            REQUIRE(format_check_expB2 == reserved);
            res = res + reserved.size();
        }
        return res;
    };
    BENCHMARK("fmt1.130 append auto  rsrved bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            reserved.clear();
            jau::cfmt::append(reserved, "format_check: %d, %?, %?, %?, %?, %?, %?, %?, %?, %?", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
            REQUIRE(format_check_exp1 == reserved);
            res = res + reserved.size();
        }
        return res;
    };
    BENCHMARK("fmt1.130 append-cap   rsrved bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            reserved.clear();
            jau::cfmt::append_cap(reserved, "format_check: %d, %u, %d, %u, %d, %u, %d, %u, %d, %u", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
            REQUIRE(format_check_exp1 == reserved);
            res = res + reserved.size();
        }
        return res;
    };
    BENCHMARK("fmtX.130 snprintf     rsrved bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            reserved.clear();
            reserved.resize(bsz - 1);      // excl. EOS
            size_t nchars = std::snprintf(&reserved[0], bsz, "format_check: %hhd, %hhu, %hd, %hu, %d, %u, %ld, %lu, %zd, %zu", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
            if( nchars < bsz ) {
                reserved.resize(nchars);
            }
            REQUIRE(format_check_exp1 == reserved);
            res = res + nchars;
        }
        return res;
    };
    BENCHMARK("fmtX.130 stringstream        bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            std::ostringstream ss1;
            ss1 << "format_check: "
                << -127 << ", "
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

    BENCHMARK("fmt1.230 format              bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            std::string s = jau::cfmt::format("format_check: %05hhd, %05hhu, %010hd, %010hu, %015d, %015u, %015ld, %015lu, %015zd, %015zu", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
            REQUIRE(format_check_exp2 == s);
            res = res + s.size();
        }
        return res;
    };
    BENCHMARK("fmt1.230 formatR      rsrved bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            reserved.clear();

            jau::cfmt::formatR(reserved, "format_check: %05hhd, %05hhu, %010hd, %010hu, %015d, %015u, %015ld, %015lu, %015zd, %015zu", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
            REQUIRE(format_check_exp2 == reserved);
            res = res + reserved.size();
        }
        return res;
    };
    BENCHMARK("fmt1.230 append +len  rsrved bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            reserved.clear();
            jau::cfmt::append(reserved, "format_check: %05hhd, %05hhu, %010hd, %010hu, %015d, %015u, %015ld, %015lu, %015zd, %015zu", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
            REQUIRE(format_check_exp2 == reserved);
            res = res + reserved.size();
        }
        return res;
    };
    BENCHMARK("fmt1.230 append       rsrved bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            reserved.clear();
            jau::cfmt::append(reserved, "format_check: %05hhd, %05hhu, %010hd, %010hu, %015d, %015u, %015ld, %015lu, %015zd, %015zu", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
            REQUIRE(format_check_exp2 == reserved);
            res = res + reserved.size();
        }
        return res;
    };
    BENCHMARK("fmt1.230 append 32    rsrved bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            reserved.clear();
            jau::cfmt::append(reserved, "format_check: %015u, %015u, %015u, %015u, %015u, %015u, %015u, %015u, %015u, %015u", u32, u32, u32, u32, u32, u32, u32, u32, u32, u32);
            REQUIRE(format_check_expC1 == reserved);
            res = res + reserved.size();
        }
        return res;
    };
    BENCHMARK("fmt1.230 append 64    rsrved bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            reserved.clear();
            jau::cfmt::append(reserved, "format_check: %015u, %015u, %015u, %015u, %015u, %015u, %015u, %015u, %015u, %015u", u64, u64, u64, u64, u64, u64, u64, u64, u64, u64);
            REQUIRE(format_check_expC1 == reserved);
            res = res + reserved.size();
        }
        return res;
    };
    BENCHMARK("fmt1.231 append 32    rsrved bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            reserved.clear();
            jau::cfmt::append(reserved, "format_check: %#015x, %#015x, %#015x, %#015x, %#015x, %#015x, %#015x, %#015x, %#015x, %#015x", u32, u32, u32, u32, u32, u32, u32, u32, u32, u32);
            REQUIRE(format_check_expC2 == reserved);
            res = res + reserved.size();
        }
        return res;
    };
    BENCHMARK("fmt1.231 append 64    rsrved bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            reserved.clear();
            jau::cfmt::append(reserved, "format_check: %#015x, %#015x, %#015x, %#015x, %#015x, %#015x, %#015x, %#015x, %#015x, %#015x", u64, u64, u64, u64, u64, u64, u64, u64, u64, u64);
            REQUIRE(format_check_expC2 == reserved);
            res = res + reserved.size();
        }
        return res;
    };
    BENCHMARK("fmt1.232 append auto  rsrved bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            reserved.clear();
            jau::cfmt::append(reserved, "format_check: %05d, %05?, %010?, %010?, %015?, %015?, %015?, %015?, %015?, %015?", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
            REQUIRE(format_check_exp2 == reserved);
            res = res + reserved.size();
        }
        return res;
    };
    BENCHMARK("fmt1.230 append-cap   rsrved bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            reserved.clear();
            jau::cfmt::append_cap(reserved, "format_check: %05d, %05u, %010d, %010u, %015d, %015u, %015d, %015u, %015d, %015u", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
            REQUIRE(format_check_exp2 == reserved);
            res = res + reserved.size();
        }
        return res;
    };
    BENCHMARK("fmtX.232 snprintf     rsrved bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            reserved.clear();
            reserved.resize(bsz - 1);      // excl. EOS
            size_t nchars = std::snprintf(&reserved[0], bsz, "format_check: %05hhd, %05hhu, %010hd, %010hu, %015d, %015u, %015ld, %015lu, %015zd, %015zu", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
            if( nchars < bsz ) {
                reserved.resize(nchars);
            }
            REQUIRE(format_check_exp2 == reserved);
            res = res + nchars;
        }
        return res;
    };
    BENCHMARK("fmtX.232 stringstream        bench") {
        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            std::ostringstream ss1;
            ss1 << "format_check: "
                << std::setfill('0') // undefined with negative numbers, duh!
                << "-" << std::setw(5-1) << 127 << ", "
                << std::setw(5) << unsigned(i2) << ", "
                << "-" << std::setw(10-1) << jau::abs(i3) << ", "
                << std::setw(10) << i4 << ", "
                << "-" << std::setw(15-1) << jau::abs(i5) << ", "
                << std::setw(15) << i6 << ", "
                << "-" << std::setw(15-1) << jau::abs(i7) << ", "
                << std::setw(15) << i8 << ", "
                << "-" << std::setw(15-1) << jau::abs(i9) << ", "
                << std::setw(15) << i10;
            std::string s = ss1.str();
            REQUIRE(format_check_exp2 == s);
            res = res + s.size();
        }
        return res;
    };

}
