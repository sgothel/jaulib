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
#include <jau/exceptions.hpp>
#include <jau/backtrace.hpp>

#include <cstdarg>

#ifdef USE_LIBUNWIND
    #define UNW_LOCAL_ONLY
    #include <libunwind.h>
#endif /* USE_LIBUNWIND */

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

// gcc-1 CTTI mangled name: 'constexpr const char* jau::ctti_name() [with T = int(int)]'
static const std::string ctti_name_prefix_gcc1 = "constexpr const char* jau::ctti_name() [with T = ";

// clang-1 CTTI mangled name: 'const char *jau::ctti_name() [T = int (int)]`
static const std::string ctti_name_prefix_clang1 = "const char *jau::ctti_name() [T = ";

static const std::string* ctti_name_prefixes[] = { &ctti_name_prefix_gcc1, &ctti_name_prefix_clang1 };

std::string jau::demangle_name(const char* mangled_name) noexcept {
    const size_t len = ::strlen(mangled_name);
    if( nullptr == mangled_name || 0 == len ) {
        return std::string();
    }
    for(const std::string* ctti_name_prefix : ctti_name_prefixes) {
        if( len > ctti_name_prefix->length()+2 &&
            mangled_name == ::strstr(mangled_name, ctti_name_prefix->c_str()) )
        {
            std::string r( mangled_name + ctti_name_prefix->length() );
            r.resize(r.length() - 1);
            return r;
        }
    }

    int status;
    /**
     *   0: The demangling operation succeeded.
     *  -1: A memory allocation failure occurred.
     *  -2: @a mangled_name is not a valid name under the C++ ABI mangling rules.
     *  -3: One of the arguments is invalid.
     */
    char* real_name = abi::__cxa_demangle(mangled_name, nullptr, nullptr, &status);
    if ( nullptr == real_name ) {
        return std::string(mangled_name); // didn't work, use mangled_name
    } else {
        std::string res;
        if( 0 != status || 0 == ::strlen(real_name) ) {
            res = std::string(mangled_name);    // didn't work, use mangled_name
        } else {
            res = std::string(real_name);
        }
        free( real_name );
        return res;
    }
}

std::string jau::get_backtrace(const bool skip_anon_frames, const jau::snsize_t max_frames, const jau::snsize_t skip_frames) noexcept {
    std::string out;
    try {
#ifdef USE_LIBUNWIND
        // symbol:
        //  1: _ZN9direct_bt10DBTAdapter14startDiscoveryEbNS_19HCILEOwnAddressTypeEtt + 0x58d @ ip 0x7faa959d6daf, sp 0x7ffe38f301e0
        // de-mangled:
        //  1: direct_bt::DBTAdapter::startDiscovery(bool, direct_bt::HCILEOwnAddressType, unsigned short, unsigned short) + 0x58d @ ip 0x7f687b459071, sp 0x7fff2bf795d0
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
            if( 0 == unw_get_proc_name(&cursor, cstr, sizeof(cstr), &offset) ) {
                int status;
                char *real_name;
                cstr[sizeof(cstr) -1] = 0; // fail safe
                if ( (real_name = abi::__cxa_demangle(cstr, nullptr, nullptr, &status)) == nullptr ) {
                    line.append(cstr); // didn't work, use cstr
                } else {
                    line.append(real_name);
                    free( real_name );
                }
                snprintf(cstr, sizeof(cstr), " + 0x%lx @ ip %p, sp %p", (unsigned long)offset, (void*)ip, (void*)sp); // NOLINT(performance-no-int-to-ptr)
                append_line = true;
                last_frame_anon = false;
            } else {
                // anon frame w/o proc-name
                snprintf(cstr, sizeof(cstr), "__no_proc_name__ @ ip %p, sp %p", (void*)ip, (void*)sp); // NOLINT(performance-no-int-to-ptr)
                append_line = !skip_anon_frames || !last_frame_anon;
                last_frame_anon = true;
            }
            line.append(cstr);
            line.append("\n");
            if( append_line ) {
                out.append(line);
            }
        }
#else /* USE_LIBUNWIND */
        (void)skip_anon_frames;
        (void)max_frames;
        (void)skip_frames;
        out.append("0: backtrace disabled\n");
#endif /* USE_LIBUNWIND */
    } catch (...) { } // NOLINT(bugprone-empty-catch): intentional
    return out;
}

#if defined(__GNUC__) && ! defined(__clang__)
    #pragma GCC pop_options
#endif

void jau::print_backtrace(const bool skip_anon_frames, const jau::snsize_t max_frames, const jau::snsize_t skip_frames) noexcept {
    ::fprintf(stderr, "%s", get_backtrace(skip_anon_frames, max_frames, skip_frames).c_str());
    ::fflush(stderr);
}

extern "C" {
    int jaulib_id_entryfunc() { return 1; }
}
