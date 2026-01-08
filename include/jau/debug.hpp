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
#include <string_view>

#include <jau/cpp_lang_util.hpp>
#include <jau/environment.hpp>

#include <jau/string_util.hpp>
#include <jau/string_cfmt.hpp>

// #define PERF_PRINT_ON 1

namespace jau {

    namespace impl {
        template <typename... Args>
        void dbgPrint0(FILE *out, bool addErrno, bool addBacktrace, std::string_view format, const Args &...args) noexcept {
            std::exception_ptr eptr;
            try {
                ::fputs(jau::format_string(format, args...).c_str(), out);
                if (addErrno) {
                    ::fprintf(stderr, "; last errno %d %s", errno, strerror(errno));
                }
                ::fputs("\n", out);
                if (addBacktrace) {
                    ::fprintf(stderr, "%s", jau::get_backtrace(true /* skip_anon_frames */, 4 /* max_frames */, 2 /* skip_frames: this() + get_b*() */).c_str());
                }
                if (addErrno || addBacktrace) {
                    ::fflush(stderr);
                }
            } catch (...) {
                eptr = std::current_exception();
            }
            handle_exception(eptr);
        }

        template <typename... Args>
        void dbgPrint1(FILE *out, bool printPrefix, const char *msg, std::string_view format, const Args &...args) noexcept {
            if (printPrefix) {
                std::exception_ptr eptr;
                try {
                    ::fputc('[', out);
                    ::fputs(jau::to_decstring(environment::getElapsedMillisecond(), ',', 9).c_str(), out);
                    ::fputs("] ", out);
                    if (msg) {
                        ::fputs(msg, out);
                        ::fputs(": ", out);
                    }
                } catch (...) {
                    eptr = std::current_exception();
                }
                handle_exception(eptr);
            }
            jau::impl::dbgPrint0(out, false, false, format, args...);
        }

        template <typename... Args>
        void dbgPrint2(FILE *out, const char *msg, bool addErrno, bool addBacktrace, const char *func, const char *file, const int line,
                                  std::string_view format, const Args &...args) noexcept {
            std::exception_ptr eptr;
            try {
                ::fputc('[', out);
                ::fputs(jau::to_decstring(environment::getElapsedMillisecond(), ',', 9).c_str(), out);
                ::fputs("] ", out);
                if (msg) {
                    ::fputs(msg, out);
                    ::fputs(" ", out);
                }
                ::fprintf(stderr, "@ %s:%d %s: ", file, line, func);
            } catch (...) {
                eptr = std::current_exception();
            }
            handle_exception(eptr);
            jau::impl::dbgPrint0(out, addErrno, addBacktrace, format, args...);
        }
    }

    #define jau_dbgPrint1(out, printPrefix, msg, fmt, ...) \
        jau::impl::dbgPrint1((out), (printPrefix), (msg), (fmt) __VA_OPT__(,) __VA_ARGS__);  \
        static_assert(0 <= jau::cfmt::check2< JAU_FOR_EACH1_LIST(JAU_DECLTYPE_VALUE, __VA_ARGS__) >(fmt)); // compile time validation!

    #define jau_dbgPrint1Line(out, printPrefix, msg, fmt, ...) \
        jau::impl::dbgPrint1((out), (printPrefix), (msg), (fmt) __VA_OPT__(,) __VA_ARGS__);  \
        static_assert(0 == jau::cfmt::check2Line< JAU_FOR_EACH1_LIST(JAU_DECLTYPE_VALUE, __VA_ARGS__) >(fmt)); // compile time validation!

    #define jau_dbgPrint2(out, msg, addErrno, addBacktrace, func, file, line, fmt, ...) \
        jau::impl::dbgPrint2((out), (msg), (addErrno), (addBacktrace), (func), (file), (line), (fmt) __VA_OPT__(,) __VA_ARGS__);  \
        static_assert(0 <= jau::cfmt::check2< JAU_FOR_EACH1_LIST(JAU_DECLTYPE_VALUE, __VA_ARGS__) >(fmt)); // compile time validation!

    #define jau_dbgPrint2Line(out, msg, addErrno, addBacktrace, func, file, line, fmt, ...) \
        jau::impl::dbgPrint2((out), (msg), (addErrno), (addBacktrace), (func), (file), (line), (fmt) __VA_OPT__(,) __VA_ARGS__);  \
        static_assert(0 == jau::cfmt::check2Line< JAU_FOR_EACH1_LIST(JAU_DECLTYPE_VALUE, __VA_ARGS__) >(fmt)); // compile time validation!

    /** Use for environment-variable environment::DEBUG conditional debug messages, prefix '[elapsed_time] Debug: '. */
    #define DBG_PRINT(fmt, ...) { if( jau::environment::get().debug ) { jau_dbgPrint1(stderr, true, "Debug", fmt __VA_OPT__(,) __VA_ARGS__); } }
    #define DBG_PRINT_LINE(fmt, ...) { if( jau::environment::get().debug ) { jau_dbgPrint1Line(stderr, true, "Debug", fmt __VA_OPT__(,) __VA_ARGS__); } }

    /** Use for environment-variable environment::DEBUG_JNI conditional debug messages, prefix '[elapsed_time] Debug: '. */
    #define DBG_JNI_PRINT(...) { if( jau::environment::get().debug_jni ) { jau_dbgPrint1(stderr, true, "Debug", __VA_ARGS__); } }

    /** Use for environment-variable environment::DEBUG conditional warning messages, prefix '[elapsed_time] Warning @ FILE:LINE FUNC: ' */
    #define DBG_WARN_PRINT(...) { if( jau::environment::get().debug ) { jau_dbgPrint2(stderr, "Warning", false /* errno */, false /* backtrace */, __func__, __FILE__, __LINE__, __VA_ARGS__); } }

    /** Use for environment-variable environment::DEBUG conditional error messages, prefix '[elapsed_time] Debug @ FILE:LINE FUNC: '. Function also appends last errno, strerror(errno) and full backtrace*/
    #define DBG_ERR_PRINT(...) { if( jau::environment::get().debug ) { jau_dbgPrint2(stderr, "Debug", true /* errno */, true /* backtrace */, __func__, __FILE__, __LINE__, __VA_ARGS__); } }

    /**
     * Use for environment-variable environment::VERBOSE conditional verbose messages, prefix '[elapsed_time] Wordy: '.
     * <p>
     * 'Wordy' is the shorter English form of the Latin word 'verbosus', from which the word 'verbosity' is sourced.
     * </p>
     */
    #define WORDY_PRINT(...) { if( jau::environment::get().verbose ) { jau_dbgPrint1(stderr, true, "Wordy", __VA_ARGS__); } }

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

    /** Use for unconditional ::abort() call with given messages, prefix '[elapsed_time] ABORT @ file:line func: '. Function also appends last errno and strerror(errno). */
    #define ABORT(...) { jau_dbgPrint2(stderr, "ABORT", true /* errno */, true /* backtrace */, __func__, __FILE__, __LINE__, __VA_ARGS__); abort(); }

    /** Use for unconditional error messages, prefix '[elapsed_time] Error @ FILE:LINE FUNC: '. Function also appends last errno, strerror(errno) and full backtrace*/
    #define ERR_PRINT(...) { jau_dbgPrint2(stderr, "Error", true /* errno */, true /* backtrace */, __func__, __FILE__, __LINE__, __VA_ARGS__); }

    /** Use for unconditional error messages, prefix '[elapsed_time] Error @ FILE:LINE FUNC: '. Function also appends last errno and strerror(errno). No backtrace. */
    #define ERR_PRINT2(...) { jau_dbgPrint2(stderr, "Error", true /* errno */, false /* backtrace */, __func__, __FILE__, __LINE__, __VA_ARGS__); }

    /** Use for unconditional error messages, prefix '[elapsed_time] Error @ FILE:LINE FUNC: '. Function also appends last errno and strerror(errno). Full backtrace. */
    #define ERR_PRINT3(...) { jau_dbgPrint2(stderr, "Error", true /* errno */, true /* backtrace */, __func__, __FILE__, __LINE__, __VA_ARGS__); }

    /** Use for unconditional interruption messages, prefix '[elapsed_time] Interrupted @ FILE:LINE FUNC: '. Function also appends last errno and strerror(errno). */
    #define IRQ_PRINT(...) { jau_dbgPrint2(stderr, "Interrupted", true /* errno */, false /* backtrace */, __func__, __FILE__, __LINE__, __VA_ARGS__); }

    /** Use for unconditional warning messages, prefix '[elapsed_time] Warning @ FILE:LINE FUNC: ' */
    #define WARN_PRINT(...) { jau_dbgPrint2(stderr, "Warning", false /* errno */, false /* backtrace */, __func__, __FILE__, __LINE__, __VA_ARGS__); }

    /** Use for unconditional informal messages, prefix '[elapsed_time] Info: '. */
    #define INFO_PRINT(fmt, ...) { jau_dbgPrint1(stderr, true, "Info", fmt __VA_OPT__(,) __VA_ARGS__); }
    #define INFO_PRINT_LINE(fmt, ...) { jau_dbgPrint1Line(stderr, true, "Info", fmt __VA_OPT__(,) __VA_ARGS__); }

    /** Use for unconditional plain messages, prefix '[elapsed_time] ' if printPrefix == true. */
    #define PLAIN_PRINT(printPrefix, fmt, ...) { jau_dbgPrint1(stderr, (printPrefix), nullptr, fmt __VA_OPT__(,) __VA_ARGS__); }

    /**
     * Convenient fprintf() invocation, prepending the given elapsed_ms timestamp.
     * @param elapsed_ms the given elapsed time in milliseconds
     * @param stream the output stream
     * @param format the format
     * @param args the optional arguments
     */
    template <typename... Args>
    int fprintf_td(const uint64_t elapsed_ms, FILE* stream, std::string_view format, const Args &...args) noexcept {
        int res = 0;
        std::exception_ptr eptr;
        try {
            res = ::fprintf(stream, "[%s] ", jau::to_decstring(elapsed_ms, ',', 9).c_str());
            const int r = ::fputs(jau::format_string(format, args...).c_str(), stream);
            if (r >= 0) {
                res += r;
            }
        } catch (...) {
            eptr = std::current_exception();
        }
        handle_exception(eptr);
        return res;
    }
    /**
     * Convenient fprintf() invocation, prepending the environment::getElapsedMillisecond() timestamp.
     * @param stream the output stream
     * @param format the format
     * @param args the optional arguments
     */
    template <typename... Args>
    inline int fprintf_td(FILE* stream, std::string_view format, const Args &...args) noexcept {
        return fprintf_td(environment::getElapsedMillisecond(), stream, format, args...);
    }

    /** Use for conditional plain messages, prefix '[elapsed_time] '. */
    #define COND_PRINT(C, ...) { if( C ) { jau::impl::dbgPrint0(stderr, false, false, __VA_ARGS__); } }

    template<class List>
    void printSharedPtrList(const std::string& prefix, List & list) noexcept {
        ::fprintf(stderr, "%s: Start: %zu elements\n", prefix.c_str(), (size_t)list.size());
        int idx = 0;
        for (auto it = list.begin(); it != list.end(); idx++) {
            typename List::value_type & e = *it;
            if ( nullptr != e ) {
                ::fprintf(stderr, "%s[%d]: useCount %zu, mem %p\n", prefix.c_str(), idx, (size_t)e.use_count(), e.get());
            } else {
                ::fprintf(stderr, "%s[%d]: NULL\n", prefix.c_str(), idx);
            }
            ++it;
        }
    }

} // namespace jau

#endif /* JAU_DEBUG_HPP_ */
