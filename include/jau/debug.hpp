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

#ifndef JAU_DEBUG_HPP_
#define JAU_DEBUG_HPP_

#include <cstdlib>

#include <cstdint>
#include <cstring>
#include <string>
#include <cstdio>
#include <cstdarg>

#include <jau/cpp_lang_util.hpp>
#include <jau/environment.hpp>
#include <jau/backtrace.hpp>

#include <jau/string_util.hpp>

// #define PERF_PRINT_ON 1

namespace jau {

    namespace impl {
        void dbgPrint1(const char * format, ...) noexcept;
        void dbgPrint2(const char *s) noexcept;

        /** Use for unconditional ::abort() call with given messages, prefix '[elapsed_time] ABORT @ file:line func: '. Function also appends last errno and strerror(errno). */
        void abortImpl(const char *func, const char *file, const int line, const char * format, ...) noexcept;

        void errPrint(const char *prefix, const bool backtrace, const char *func, const char *file, const int line, const char * format, ...) noexcept;
        void warnPrint(const char *func, const char *file, const int line, const char * format, ...) noexcept;
        void wordyPrint(const char * format, ...) noexcept;
    }

    /** Use for unconditional ::abort() call with given messages, prefix '[elapsed_time] ABORT @ file:line func: '. Function also appends last errno and strerror(errno). */
    #define ABORT(...) { jau::impl::abortImpl(__func__, __FILE__, __LINE__, __VA_ARGS__); }

    /** Use for environment-variable environment::DEBUG conditional debug messages, prefix '[elapsed_time] Debug: '. */
    template <typename... Args>
    void dbgPrint(std::string_view format, const Args &...args) {
        const jau::cfmt::PResult pr = jau::cfmt::checkR2<Args...>(format);
        if ( pr.argCount() < 0 ) {
            throw jau::IllegalArgumentError("format/arg mismatch `"+std::string(format)+"`: "+pr.toString(), E_FILE_LINE);
        }
        impl::dbgPrint2(impl::format_string_h(1023, format, args...).c_str());
    }
    /** Use for environment-variable environment::DEBUG conditional debug messages, prefix '[elapsed_time] Debug: '. */
    #define DBG_PRINT(...) { if( jau::environment::get().debug ) { jau::impl::dbgPrint1(__VA_ARGS__); } }

    /** Use for environment-variable environment::DEBUG conditional warning messages, prefix '[elapsed_time] Warning @ FILE:LINE FUNC: ' */
    #define DBG_WARN_PRINT(...) { if( jau::environment::get().debug ) { jau::impl::warnPrint(__func__, __FILE__, __LINE__, __VA_ARGS__); } }

    /** Use for environment-variable environment::DEBUG conditional error messages, prefix '[elapsed_time] Warning @ FILE:LINE FUNC: '. Function also appends last errno, strerror(errno) and full backtrace*/
    #define DBG_ERR_PRINT(...) { if( jau::environment::get().debug ) { jau::impl::errPrint("Debug", true /* backtrace */, __func__, __FILE__, __LINE__, __VA_ARGS__); } }

    /** Use for environment-variable environment::DEBUG_JNI conditional debug messages, prefix '[elapsed_time] Debug: '. */
    #define DBG_JNI_PRINT(...) { if( jau::environment::get().debug_jni ) { jau::impl::dbgPrint1(__VA_ARGS__); } }

    /**
     * Use for environment-variable environment::VERBOSE conditional verbose messages, prefix '[elapsed_time] Wordy: '.
     * <p>
     * 'Wordy' is the shorter English form of the Latin word 'verbosus', from which the word 'verbosity' is sourced.
     * </p>
     */
    #define WORDY_PRINT(...) { if( jau::environment::get().verbose ) { jau::impl::wordyPrint(__VA_ARGS__); } }


    #define PERF_TS_T0_BASE()  const uint64_t _t0 = jau::getCurrentMilliseconds()

    #define PERF_TS_TD_BASE(m)  { const uint64_t _td = jau::getCurrentMilliseconds() - _t0; \
                                  fprintf(stderr, "[%s] PERF %s done in %d ms,\n", jau::to_decstring(jau::environment::getElapsedMillisecond(), ',', 9).c_str(), (m), (int)_td); }
    #ifdef PERF_PRINT_ON
        #define PERF_TS_T0() PERF_TS_T0_BASE()
        #define PERF_TS_TD(m) PERF_TS_TD_BASE(m)
    #else
        #define PERF_TS_T0()
        #define PERF_TS_TD(m)
    #endif
    #ifdef PERF2_PRINT_ON
        #define PERF2_TS_T0() PERF_TS_T0_BASE()
        #define PERF2_TS_TD(m) PERF_TS_TD_BASE(m)
    #else
        #define PERF2_TS_T0()
        #define PERF2_TS_TD(m)
    #endif
    #ifdef PERF3_PRINT_ON
        #define PERF3_TS_T0() PERF_TS_T0_BASE()
        #define PERF3_TS_TD(m) PERF_TS_TD_BASE(m)
    #else
        #define PERF3_TS_T0()
        #define PERF3_TS_TD(m)
    #endif

    /** Use for unconditional error messages, prefix '[elapsed_time] Error @ file:line func: '. Function also appends last errno and strerror(errno). */
    void ERR_PRINTv(const char *func, const char *file, const int line, const char * format, va_list args) noexcept;

    /** Use for unconditional error messages, prefix '[elapsed_time] Error @ FILE:LINE FUNC: '. Function also appends last errno, strerror(errno) and full backtrace*/
    #define ERR_PRINT(...) { jau::impl::errPrint("Error", true /* backtrace */, __func__, __FILE__, __LINE__, __VA_ARGS__); }

    /** Use for unconditional error messages, prefix '[elapsed_time] Error @ FILE:LINE FUNC: '. Function also appends last errno and strerror(errno). No backtrace. */
    #define ERR_PRINT2(...) { jau::impl::errPrint("Error", false /* backtrace */, __func__, __FILE__, __LINE__, __VA_ARGS__); }

    /** Use for unconditional interruption messages, prefix '[elapsed_time] Interrupted @ FILE:LINE FUNC: '. Function also appends last errno and strerror(errno). */
    #define IRQ_PRINT(...) { jau::impl::errPrint("Interrupted", false /* backtrace */, __func__, __FILE__, __LINE__, __VA_ARGS__); }

    /** Use for unconditional warning messages, prefix '[elapsed_time] Warning @ file:line func: ' */
    void WARN_PRINTv(const char *func, const char *file, const int line, const char * format, va_list args) noexcept;

    /** Use for unconditional warning messages, prefix '[elapsed_time] Warning @ FILE:LINE FUNC: ' */
    #define WARN_PRINT(...) { jau::impl::warnPrint(__func__, __FILE__, __LINE__, __VA_ARGS__); }

    /** Use for unconditional informal messages, prefix '[elapsed_time] Info: '. */
    void INFO_PRINT(const char * format, ...) noexcept;

    /** Use for unconditional plain messages, prefix '[elapsed_time] ' if printPrefix == true. */
    void PLAIN_PRINT(const bool printPrefix, const char * format, ...) noexcept;

    /**
     * Convenient fprintf() invocation, prepending the given elapsed_ms timestamp.
     * @param elapsed_ms the given elapsed time in milliseconds
     * @param stream the output stream
     * @param format the format
     * @param args the optional arguments
     */
    int fprintf_td(const uint64_t elapsed_ms, FILE* stream, const char * format, ...) noexcept;

    /**
     * Convenient fprintf() invocation, prepending the environment::getElapsedMillisecond() timestamp.
     * @param stream the output stream
     * @param format the format
     * @param args the optional arguments
     */
    int fprintf_td(FILE* stream, const char * format, ...) noexcept;

    void COND_PRINT_impl(const char * format, ...) noexcept;

    /** Use for conditional plain messages, prefix '[elapsed_time] '. */
    #define COND_PRINT(C, ...) { if( C ) { jau::COND_PRINT_impl(__VA_ARGS__); } }


    template<class List>
    void printSharedPtrList(const std::string& prefix, List & list) noexcept {
        fprintf(stderr, "%s: Start: %zu elements\n", prefix.c_str(), (size_t)list.size());
        int idx = 0;
        for (auto it = list.begin(); it != list.end(); idx++) {
            typename List::value_type & e = *it;
            if ( nullptr != e ) {
                fprintf(stderr, "%s[%d]: useCount %zu, mem %p\n", prefix.c_str(), idx, (size_t)e.use_count(), e.get());
            } else {
                fprintf(stderr, "%s[%d]: NULL\n", prefix.c_str(), idx);
            }
            ++it;
        }
    }

} // namespace jau

#endif /* JAU_DEBUG_HPP_ */
