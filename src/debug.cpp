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

std::string jau::get_backtrace(const bool skip_anon_frames, const jau::snsize_t max_frames, const jau::nsize_t skip_frames) noexcept {
    // symbol:
    //  1: _ZN9direct_bt10DBTAdapter14startDiscoveryEbNS_19HCILEOwnAddressTypeEtt + 0x58d @ ip 0x7faa959d6daf, sp 0x7ffe38f301e0
    // de-mangled:
    //  1: direct_bt::DBTAdapter::startDiscovery(bool, direct_bt::HCILEOwnAddressType, unsigned short, unsigned short) + 0x58d @ ip 0x7f687b459071, sp 0x7fff2bf795d0
    std::string out;
    jau::nsize_t frame=0;
    int res;
    char cstr[256];
    unw_context_t uc;
    unw_word_t ip, sp;
    unw_cursor_t cursor;
    unw_word_t offset;
    unw_getcontext(&uc);
    unw_init_local(&cursor, &uc);
    bool last_frame_anon = false;
    while( unw_step(&cursor) > 0 && ( max_frames - skip_frames ) > frame ) {
        frame++;
        if( skip_frames > frame ) {
            continue;
        }
        bool append_line;
        snprintf(cstr, sizeof(cstr), "%3zu: ", (size_t)frame);
        std::string line(cstr);

        unw_get_reg(&cursor, UNW_REG_IP, &ip); // instruction pointer (pc)
        unw_get_reg(&cursor, UNW_REG_SP, &sp); // stack pointer
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

void jau::print_backtrace(const bool skip_anon_frames, const jau::snsize_t max_frames, const jau::nsize_t skip_frames) noexcept {
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
    jau::print_backtrace(true, 2);
    abort();
}

void jau::ERR_PRINTv(const char *func, const char *file, const int line, const char * format, va_list args) noexcept {
    fprintf(stderr, "[%s] Error @ %s:%d %s: ", jau::to_decstring(environment::getElapsedMillisecond(), ',', 9).c_str(), file, line, func);
    vfprintf(stderr, format, args);
    fprintf(stderr, "; last errno %d %s\n", errno, strerror(errno));
    fflush(stderr);
    jau::print_backtrace(true, 2);
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
        jau::print_backtrace(true, 2);
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
