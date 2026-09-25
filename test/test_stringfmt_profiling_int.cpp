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
    if (!tstnum || 3==tstnum) {
        jau_fprintf_td(stderr, "fmt1.130 append       rsrved bench\n");
        for( size_t i = 0; i < loops; ++i ) {
            reserved.clear();
            jau::cfmt::append(reserved, "format_check: %hhd, %hhu, %hd, %hu, %d, %u, %ld, %lu, %zd, %zu", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
            res = res + reserved.size();
        }
        jau_fprintf_td(stderr, "Test End\n");
    }
    if (!tstnum || 4==tstnum) {
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
            std::string s = jau::cfmt::format("format_check: %01hhd, %02hhu, %03hd, %04hu, %05d, %06u, %07ld, %08lu, %09zd, %010zu", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
            res = res + s.size();
        }
        jau_fprintf_td(stderr, "Test End\n");
    }
    if (!tstnum || 11==tstnum) {
        jau_fprintf_td(stderr, "fmt1.230 formatR      rsrved bench\n");
        for( size_t i = 0; i < loops; ++i ) {
            reserved.clear();

            jau::cfmt::formatR(reserved, "format_check: %01hhd, %02hhu, %03hd, %04hu, %05d, %06u, %07ld, %08lu, %09zd, %010zu", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
            res = res + reserved.size();
        }
        jau_fprintf_td(stderr, "Test End\n");
    }
    if (!tstnum || 12==tstnum) {
        jau_fprintf_td(stderr, "fmt1.230 append-ckd   rsrved bench\n");
        for( size_t i = 0; i < loops; ++i ) {
            reserved.clear();
            jau_append_string(reserved, "format_check: %01hhd, %02hhu, %03hd, %04hu, %05d, %06u, %07ld, %08lu, %09zd, %010zu", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
            res = res + reserved.size();
        }
        jau_fprintf_td(stderr, "Test End\n");
    }
    if (!tstnum || 13==tstnum) {
        jau_fprintf_td(stderr, "fmt1.230 append       rsrved bench\n");
        for( size_t i = 0; i < loops; ++i ) {
            reserved.clear();
            jau::cfmt::append(reserved, "format_check: %01hhd, %02hhu, %03hd, %04hu, %05d, %06u, %07ld, %08lu, %09zd, %010zu", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
            res = res + reserved.size();
        }
        jau_fprintf_td(stderr, "Test End\n");
    }
    if (!tstnum || 14==tstnum) {
        jau_fprintf_td(stderr, "fmt1.232 append auto  rsrved bench\n");
        for( size_t i = 0; i < loops; ++i ) {
            reserved.clear();
            jau::cfmt::append(reserved, "format_check: %01hhd, %02?, %03?, %04?, %05?, %06?, %07?, %08?, %09?, %010?", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
            res = res + reserved.size();
        }
        jau_fprintf_td(stderr, "Test End\n");
    };
    if (!tstnum || 15==tstnum) {
        jau_fprintf_td(stderr, "fmtX.232 snprintf     rsrved bench\n");
        for( size_t i = 0; i < loops; ++i ) {
            reserved.clear();
            reserved.resize(bsz - 1);      // excl. EOS
            size_t nchars = std::snprintf(&reserved[0], bsz, "format_check: %01hhd, %02hhu, %03hd, %04hu, %05d, %06u, %07ld, %08lu, %09zd, %010zu", i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);
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
