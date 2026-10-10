/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2024-2026 Gothel Software e.K.
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
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <limits>
#include <string_view>
#include "jau/debug.hpp"

#include <jau/basic_types.hpp>
#include <jau/cpp_lang_util.hpp>
#include <jau/cpp_pragma.hpp>
#include <jau/float_types.hpp>
#include <jau/int_types.hpp>
#include <jau/string_cfmt.hpp>
#include <jau/string_literal.hpp>
#include <jau/string_util.hpp>
#include <jau/type_concepts.hpp>

#ifdef HAS_STD_FORMAT
    #include <format>
#endif

using namespace std::literals;

using namespace jau::float_literals;

using namespace jau::int_literals;

/**
 * append: 42 (regular), 43 (auto-conversion), 64 (32/64 bit), 88 (all append)
 * - no width/prec
 *   - 3 regular (42)
 *   - 4 auto-conversion (43)
 * - with/prec
 *   - 13 regular (42)
 *   - 14 auto-conversion (43)
 */
int main(int argc, char *argv[]) {
    int tstnum = 0;
    size_t loops = 1000;

    for (int i = 0; i < argc; i++) {
        if (0 == strcmp("--test", argv[i]) && i+1<argc) {
            jau::fromIntString(tstnum, std::string_view(argv[++i]));
        } else if (0 == strcmp("--loops", argv[i]) && i+1<argc) {
            jau::fromIntString(loops, std::string_view(argv[++i]));
        }
    }
    jau_fprintf_td(stderr, "XXX: tstnum %d, loops %'zu\n", tstnum, loops);

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
    volatile size_t res = 0;

    if (!tstnum || 1==tstnum) {
        jau_fprintf_td(stderr, "fmt1.130 format              bench\n");
        for( size_t i = 0; i < loops; ++i ) {
            // fa += 0.01f; fb += 0.02f; ++sz1; ++i1; str1.append("X");
            std::string s = jau::cfmt::format("format_check: %hhd, %hhu, %hd, %hu, %d, %u, %ld, %lu, %zd, %zu", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
            res = res + s.size();
        }
        jau_fprintf_td(stderr, "Test End\n");
    }
    if (!tstnum || 2==tstnum) {
        jau_fprintf_td(stderr, "fmt1.130 formatR      rsrved bench\n");
        for( size_t i = 0; i < loops; ++i ) {
            reserved.clear();
            jau::cfmt::formatR(reserved, "format_check: %hhd, %hhu, %hd, %hu, %d, %u, %ld, %lu, %zd, %zu", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
            res = res + reserved.size();
        }
        jau_fprintf_td(stderr, "Test End\n");
    }
    if (!tstnum || 3==tstnum || 42==tstnum || 88==tstnum) {
        jau_fprintf_td(stderr, "fmt1.130 append       rsrved bench\n");
        for( size_t i = 0; i < loops; ++i ) {
            reserved.clear();
            jau::cfmt::append(reserved, "format_check: %hhd, %hhu, %hd, %hu, %d, %u, %ld, %lu, %zd, %zu", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
            res = res + reserved.size();
        }
        jau_fprintf_td(stderr, "Test End\n");
    }
    if (!tstnum || 3==tstnum || 42==tstnum || 64==tstnum) {
        jau_fprintf_td(stderr, "fmt1.130 append 32/64 rsrved bench\n");
        for( size_t i = 0; i < loops; ++i ) {
            reserved.clear();
            jau::cfmt::append(reserved, "format_check: %u, %u, %u, %u, %u, %u, %u, %u, %u, %u", u32, u32, u32, u32, u32, u64, u64, u64, u64, u64);
            res = res + reserved.size();
        }
        jau_fprintf_td(stderr, "Test End\n");
    }
    if (!tstnum || 3==tstnum || 42==tstnum || 64==tstnum) {
        jau_fprintf_td(stderr, "fmt1.131 append 32/64 rsrved bench\n");
        for( size_t i = 0; i < loops; ++i ) {
            reserved.clear();
            jau::cfmt::append(reserved, "format_check: %#x, %#x, %#x, %#x, %#x, %#x, %#x, %#x, %#x, %#x", u32, u32, u32, u32, u32, u64, u64, u64, u64, u64);
            res = res + reserved.size();
        }
        jau_fprintf_td(stderr, "Test End\n");
    }
    if (!tstnum || 4==tstnum || 43==tstnum || 88==tstnum) {
        jau_fprintf_td(stderr, "fmt1.130 append auto  rsrved bench\n");
        for( size_t i = 0; i < loops; ++i ) {
            reserved.clear();
            jau::cfmt::append(reserved, "format_check: %hhd, %?, %?, %?, %?, %?, %?, %?, %?, %?", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
            res = res + reserved.size();
        }
        jau_fprintf_td(stderr, "Test End\n");
    }
    if (!tstnum || 5==tstnum) {
        jau_fprintf_td(stderr, "fmtX.130 snprintf     rsrved bench\n");
        for( size_t i = 0; i < loops; ++i ) {
            reserved.clear();
            reserved.resize(bsz - 1);      // excl. EOS
            size_t nchars = std::snprintf(&reserved[0], bsz, "format_check: %hhd, %hhu, %hd, %hu, %d, %u, %ld, %lu, %zd, %zu", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
            if( nchars < bsz ) {
                reserved.resize(nchars);
            }
            res = res + nchars;
        }
        jau_fprintf_td(stderr, "Test End\n");
    }
    if (!tstnum || 6==tstnum) {
        jau_fprintf_td(stderr, "fmtX.130 stringstream        bench\n");
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
            res = res + s.size();
        }
        jau_fprintf_td(stderr, "Test End\n");
    }

    ///
    ///

    if (!tstnum || 10==tstnum) {
        jau_fprintf_td(stderr, "fmt1.230 format              bench\n");
        for( size_t i = 0; i < loops; ++i ) {
            std::string s = jau::cfmt::format("format_check: %05hhd, %05hhu, %010hd, %010hu, %015d, %015u, %015ld, %015lu, %015zd, %015zu", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
            res = res + s.size();
        }
        jau_fprintf_td(stderr, "Test End\n");
    }
    if (!tstnum || 11==tstnum) {
        jau_fprintf_td(stderr, "fmt1.230 formatR      rsrved bench\n");
        for( size_t i = 0; i < loops; ++i ) {
            reserved.clear();

            jau::cfmt::formatR(reserved, "format_check: %05hhd, %05hhu, %010hd, %010hu, %015d, %015u, %015ld, %015lu, %015zd, %015zu", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
            res = res + reserved.size();
        }
        jau_fprintf_td(stderr, "Test End\n");
    }
    if (!tstnum || 13==tstnum || 42==tstnum || 88==tstnum) {
        jau_fprintf_td(stderr, "fmt1.230 append       rsrved bench\n");
        for( size_t i = 0; i < loops; ++i ) {
            reserved.clear();
            jau::cfmt::append(reserved, "format_check: %05hhd, %05hhu, %010hd, %010hu, %015d, %015u, %015ld, %015lu, %015zd, %015zu", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
            res = res + reserved.size();
        }
        jau_fprintf_td(stderr, "Test End\n");
    }
    if (!tstnum || 42==tstnum || 64==tstnum) {
        jau_fprintf_td(stderr, "fmt1.230 append 32/64 rsrved bench\n");
        for( size_t i = 0; i < loops; ++i ) {
            reserved.clear();
            jau::cfmt::append(reserved, "format_check: %015u, %015u, %015u, %015u, %015u, %015u, %015u, %015u, %015u, %015u", u32, u32, u32, u32, u32, u64, u64, u64, u64, u64);
            res = res + reserved.size();
        }
        jau_fprintf_td(stderr, "Test End\n");
    }
    if (!tstnum || 42==tstnum || 64==tstnum) {
        jau_fprintf_td(stderr, "fmt1.231 append 32/64 rsrved bench\n");
        for( size_t i = 0; i < loops; ++i ) {
            reserved.clear();
            jau::cfmt::append(reserved, "format_check: %#015x, %#015x, %#015x, %#015x, %#015x, %#015x, %#015x, %#015x, %#015x, %#015x", u32, u32, u32, u32, u32, u64, u64, u64, u64, u64);
            res = res + reserved.size();
        }
        jau_fprintf_td(stderr, "Test End\n");
    }
    if (!tstnum || 14==tstnum || 43==tstnum || 88==tstnum) {
        jau_fprintf_td(stderr, "fmt1.232 append auto  rsrved bench\n");
        for( size_t i = 0; i < loops; ++i ) {
            reserved.clear();
            jau::cfmt::append(reserved, "format_check: %05d, %05?, %010?, %010?, %015?, %015?, %015?, %015?, %015?, %015?", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
            res = res + reserved.size();
        }
        jau_fprintf_td(stderr, "Test End\n");
    };
    if (!tstnum || 15==tstnum) {
        jau_fprintf_td(stderr, "fmtX.232 snprintf     rsrved bench\n");
        for( size_t i = 0; i < loops; ++i ) {
            reserved.clear();
            reserved.resize(bsz - 1);      // excl. EOS
            size_t nchars = std::snprintf(&reserved[0], bsz, "format_check: %05hhd, %05hhu, %010hd, %010hu, %015d, %015u, %015ld, %015lu, %015zd, %015zu", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
            if( nchars < bsz ) {
                reserved.resize(nchars);
            }
            res = res + nchars;
        }
        jau_fprintf_td(stderr, "Test End\n");
    }
    if (!tstnum || 16==tstnum) {
        jau_fprintf_td(stderr, "fmtX.232 stringstream        bench\n");
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
            res = res + s.size();
        }
        jau_fprintf_td(stderr, "Test End\n");
    }
    size_t res2 = res;
    jau_fprintf_td(stderr, "Exit (res %zu)\n", res2);
    return 0;
}
