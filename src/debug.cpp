/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020 Gothel Software e.K.
 * Copyright (c) 2020 ZAFENA AB
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <jau/debug.hpp>

#include <cstdarg>

#define UNW_LOCAL_ONLY
#include <libunwind.h>
#include <cxxabi.h>

using namespace jau;

/**
 * On aarch64 using g++ (Debian 10.2.1-6) 10.2.1 20210110
 * optimization of `jau::get_backtrace(..)` leads to a SIGSEGV
 * on libunwind version [1.3 - 1.6.2] function `unw_step(..)`
 */
#if defined(__GNUC__) && ! defined(__clang__)
    #pragma GCC push_options
    #pragma GCC optimize("-O0")
#endif

std::string jau::get_backtrace(const bool skip_anon_frames, const jau::snsize_t max_frames, const jau::snsize_t skip_frames) noexcept {
    // symbol:
    //  1: _ZN9direct_bt10DBTAdapter14startDiscoveryEbNS_19HCILEOwnAddressTypeEtt + 0x58d @ ip 0x7faa959d6daf, sp 0x7ffe38f301e0
    // de-mangled:
    //  1: direct_bt::DBTAdapter::startDiscovery(bool, direct_bt::HCILEOwnAddressType, unsigned short, unsigned short) + 0x58d @ ip 0x7f687b459071, sp 0x7fff2bf795d0
    std::string out;
    jau::snsize_t frame=0;
    int res;
    char cstr[256];
    unw_context_t uc;
    unw_word_t ip, sp;
    unw_cursor_t cursor;
    unw_word_t offset;

    if( 0 != ( res = unw_getcontext(&uc) ) ) {
        INFO_PRINT("unw_getcontext ERR: %d\n", res);
        return out;
    }
    if( 0 != ( res = unw_init_local(&cursor, &uc) ) ) {
        INFO_PRINT("unw_init_local ERR %d\n", res);
        return out;
    }
    bool last_frame_anon = false;
    while( unw_step(&cursor) > 0 && ( 0 > max_frames || ( max_frames + skip_frames ) > ( frame + 1 ) ) ) {
        frame++;
        if( skip_frames > frame ) {
            continue;
        }
        bool append_line;
        snprintf(cstr, sizeof(cstr), "%3zd: ", (ssize_t)frame);
        std::string line(cstr);

        if( 0 != ( res = unw_get_reg(&cursor, UNW_REG_IP, &ip) ) ) { // instruction pointer (pc)
            INFO_PRINT("unw_get_reg(IP): frame %zd, ERR %d\n", frame, res);
            ip = 0;
        }
        if( 0 != ( res = unw_get_reg(&cursor, UNW_REG_SP, &sp) ) ) { // stack pointer
            INFO_PRINT("unw_get_reg(SP): frame %zd, ERR %d\n", frame, res);
            sp = 0;
        }
        if( 0 == ( res = unw_get_proc_name(&cursor, cstr, sizeof(cstr), &offset) ) ) {
            int status;
            char *real_name;
            cstr[sizeof(cstr) -1] = 0; // fail safe
            if ( (real_name = abi::__cxa_demangle(cstr, nullptr, nullptr, &status)) == nullptr ) {
                line.append(cstr); // didn't work, use cstr
            } else {
                line.append(real_name);
                free( real_name );
            }
            snprintf(cstr, sizeof(cstr), " + 0x%lx @ ip %p, sp %p", (unsigned long)offset, (void*)ip, (void*)sp);
            append_line = true;
            last_frame_anon = false;
        } else {
            // anon frame w/o proc-name
            snprintf(cstr, sizeof(cstr), "__no_proc_name__ @ ip %p, sp %p", (void*)ip, (void*)sp);
            append_line = !skip_anon_frames || !last_frame_anon;
            last_frame_anon = true;
        }
        line.append(cstr);
        line.append("\n");
        if( append_line ) {
            out.append(line);
        }
    }
    return out;
}

#if defined(__GNUC__) && ! defined(__clang__)
    #pragma GCC pop_options
#endif

void jau::print_backtrace(const bool skip_anon_frames, const jau::snsize_t max_frames, const jau::snsize_t skip_frames) noexcept {
    fprintf(stderr, "%s", get_backtrace(skip_anon_frames, max_frames, skip_frames).c_str());
    fflush(stderr);
}

void jau::DBG_PRINT_impl(const char * format, ...) noexcept {
    fprintf(stderr, "[%s] Debug: ", jau::to_decstring(environment::getElapsedMillisecond(), ',', 9).c_str());
    va_list args;
    va_start (args, format);
    vfprintf(stderr, format, args);
    va_end (args);
    fprintf(stderr, "\n");
    fflush(stderr);
}

void jau::WORDY_PRINT_impl(const char * format, ...) noexcept {
    fprintf(stderr, "[%s] Wordy: ", jau::to_decstring(environment::getElapsedMillisecond(), ',', 9).c_str());
    va_list args;
    va_start (args, format);
    vfprintf(stderr, format, args);
    va_end (args);
    fprintf(stderr, "\n");
    fflush(stderr);
}

void jau::ABORT_impl(const char *func, const char *file, const int line, const char * format, ...) noexcept {
    fprintf(stderr, "[%s] ABORT @ %s:%d %s: ", jau::to_decstring(environment::getElapsedMillisecond(), ',', 9).c_str(), file, line, func);
    va_list args;
    va_start (args, format);
    vfprintf(stderr, format, args);
    va_end (args);
    fprintf(stderr, "; last errno %d %s\n", errno, strerror(errno));
    fflush(stderr);
    jau::print_backtrace(true /* skip_anon_frames */, -1 /* max_frames -> all */, 3 /* skip_frames: this() + print_b*() + get_b*() */);
    abort();
}

void jau::ERR_PRINTv(const char *func, const char *file, const int line, const char * format, va_list args) noexcept {
    fprintf(stderr, "[%s] Error @ %s:%d %s: ", jau::to_decstring(environment::getElapsedMillisecond(), ',', 9).c_str(), file, line, func);
    vfprintf(stderr, format, args);
    fprintf(stderr, "; last errno %d %s\n", errno, strerror(errno));
    fflush(stderr);
    jau::print_backtrace(true /* skip_anon_frames */, 4 /* max_frames */, 3 /* skip_frames: this() + print_b*() + get_b*() */);
}

void jau::ERR_PRINT_impl(const char *prefix, const bool backtrace, const char *func, const char *file, const int line, const char * format, ...) noexcept {
    fprintf(stderr, "[%s] %s @ %s:%d %s: ", jau::to_decstring(environment::getElapsedMillisecond(), ',', 9).c_str(), prefix, file, line, func);
    va_list args;
    va_start (args, format);
    vfprintf(stderr, format, args);
    va_end (args);
    fprintf(stderr, "; last errno %d %s\n", errno, strerror(errno));
    fflush(stderr);
    if( backtrace ) {
        jau::print_backtrace(true /* skip_anon_frames */, 4 /* max_frames */, 3 /* skip_frames: this() + print_b*() + get_b*() */);
    }
}

void jau::WARN_PRINTv(const char *func, const char *file, const int line, const char * format, va_list args) noexcept {
    fprintf(stderr, "[%s] Warning @ %s:%d %s: ", jau::to_decstring(environment::getElapsedMillisecond(), ',', 9).c_str(), file, line, func);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    fflush(stderr);
}

void jau::WARN_PRINT_impl(const char *func, const char *file, const int line, const char * format, ...) noexcept {
    fprintf(stderr, "[%s] Warning @ %s:%d %s: ", jau::to_decstring(environment::getElapsedMillisecond(), ',', 9).c_str(), file, line, func);
    va_list args;
    va_start (args, format);
    vfprintf(stderr, format, args);
    va_end (args);
    fprintf(stderr, "\n");
    fflush(stderr);
}

void jau::INFO_PRINT(const char * format, ...) noexcept {
    fprintf(stderr, "[%s] Info: ", jau::to_decstring(environment::getElapsedMillisecond(), ',', 9).c_str());
    va_list args;
    va_start (args, format);
    vfprintf(stderr, format, args);
    va_end (args);
    fprintf(stderr, "\n");
    fflush(stderr);
}

void jau::PLAIN_PRINT(const bool printPrefix, const char * format, ...) noexcept {
    if( printPrefix ) {
        fprintf(stderr, "[%s] ", jau::to_decstring(environment::getElapsedMillisecond(), ',', 9).c_str());
    }
    va_list args;
    va_start (args, format);
    vfprintf(stderr, format, args);
    va_end (args);
    fprintf(stderr, "\n");
    fflush(stderr);
}

int jau::fprintf_td(FILE* stream, const char * format, ...) noexcept {
    int res = ::fprintf(stderr, "[%s] ", jau::to_decstring(environment::getElapsedMillisecond(), ',', 9).c_str());
    va_list args;
    va_start (args, format);
    res += ::vfprintf(stream, format, args);
    va_end (args);
    return res;
}

void jau::COND_PRINT_impl(const char * format, ...) noexcept {
    fprintf(stderr, "[%s] ", jau::to_decstring(environment::getElapsedMillisecond(), ',', 9).c_str());
    va_list args;
    va_start (args, format);
    vfprintf(stderr, format, args);
    va_end (args);
    fprintf(stderr, "\n");
    fflush(stderr);
}
