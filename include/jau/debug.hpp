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



namespace jau::impl {
    /// This function is a possible cancellation point and therefore marked with noexcept (not marked with __THROW like ::fprintf).
    void dbgPrint0_tail(FILE *out, bool addErrno, bool addBacktrace) noexcept;
    /// This function is a possible cancellation point and therefore marked with noexcept (not marked with __THROW like ::fprintf).
    void dbgPrint1_prefix(FILE *out, const char *msg, const char *msgsep) noexcept;

    /// This function is a possible cancellation point and therefore marked with noexcept (not marked with __THROW like ::fprintf).
    template <typename... Args>
    inline __attribute__((always_inline))
    void dbgPrint0(FILE *out, bool addErrno, bool addBacktrace, std::string_view format, const Args &...args) noexcept {
        ::fputs(jau::format_string(format, args...).c_str(), out);
        jau::impl::dbgPrint0_tail(out, addErrno, addBacktrace);
    }

    /// This function is a possible cancellation point and therefore marked with noexcept (not marked with __THROW like ::fprintf).
    template <typename... Args>
    inline __attribute__((always_inline))
    void dbgPrint1(FILE *out, bool printPrefix, const char *msg, std::string_view format, const Args &...args) noexcept {
        if (printPrefix) {
            jau::impl::dbgPrint1_prefix(out, msg, ": ");
        }
        jau::impl::dbgPrint0(out, false, false, format, args...);
    }

    /// This function is a possible cancellation point and therefore marked with noexcept (not marked with __THROW like ::fprintf).
    template <typename... Args>
    inline __attribute__((always_inline))
    void dbgPrint2(FILE *out, const char *msg, bool addErrno, bool addBacktrace, const char *func, const char *file, const int line,
                              std::string_view format, const Args &...args) noexcept {
        jau::impl::dbgPrint1_prefix(out, msg, " ");
        ::fprintf(stderr, "@ %s:%d %s: ", file, line, func);
        jau::impl::dbgPrint0(out, addErrno, addBacktrace, format, args...);
    }
} // namespace jau::impl

#define jau_dbgPrint1(out, printPrefix, msg, fmt, ...) \
    jau::impl::dbgPrint1((out), (printPrefix), (msg), (fmt) __VA_OPT__(,) __VA_ARGS__);  \
    static_assert(0 <= jau::cfmt::check2< JAU_FOR_EACH1_LIST(JAU_NOREF_DECLTYPE_VALUE, __VA_ARGS__) >(fmt)); // compile time validation!

#define jau_dbgPrint1Line(out, printPrefix, msg, fmt, ...) \
    jau::impl::dbgPrint1((out), (printPrefix), (msg), (fmt) __VA_OPT__(,) __VA_ARGS__);  \
    static_assert(0 == jau::cfmt::check2Line< JAU_FOR_EACH1_LIST(JAU_NOREF_DECLTYPE_VALUE, __VA_ARGS__) >(fmt)); // compile time validation!

#define jau_dbgPrint2(out, msg, addErrno, addBacktrace, func, file, line, fmt, ...) \
    jau::impl::dbgPrint2((out), (msg), (addErrno), (addBacktrace), (func), (file), (line), (fmt) __VA_OPT__(,) __VA_ARGS__);  \
    static_assert(0 <= jau::cfmt::check2< JAU_FOR_EACH1_LIST(JAU_NOREF_DECLTYPE_VALUE, __VA_ARGS__) >(fmt)); // compile time validation!

#define jau_dbgPrint2Line(out, msg, addErrno, addBacktrace, func, file, line, fmt, ...) \
    jau::impl::dbgPrint2((out), (msg), (addErrno), (addBacktrace), (func), (file), (line), (fmt) __VA_OPT__(,) __VA_ARGS__);  \
    static_assert(0 == jau::cfmt::check2Line< JAU_FOR_EACH1_LIST(JAU_NOREF_DECLTYPE_VALUE, __VA_ARGS__) >(fmt)); // compile time validation!

/** Use for environment-variable environment::DEBUG conditional debug messages, prefix '[elapsed_time] Debug: '. */
#define jau_DBG_PRINT(fmt, ...) { if( jau::environment::get().debug ) { jau_dbgPrint1(stderr, true, "Debug", fmt __VA_OPT__(,) __VA_ARGS__); } }
#define jau_DBG_PRINT_LINE(fmt, ...) { if( jau::environment::get().debug ) { jau_dbgPrint1Line(stderr, true, "Debug", fmt __VA_OPT__(,) __VA_ARGS__); } }

/** Use for environment-variable environment::DEBUG_JNI conditional debug messages, prefix '[elapsed_time] Debug: '. */
#define jau_DBG_JNI_PRINT(...) { if( jau::environment::get().debug_jni ) { jau_dbgPrint1(stderr, true, "Debug", __VA_ARGS__); } }

/** Use for environment-variable environment::DEBUG conditional warning messages, prefix '[elapsed_time] Warning @ FILE:LINE FUNC: ' */
#define jau_DBG_WARN_PRINT(...) { if( jau::environment::get().debug ) { jau_dbgPrint2(stderr, "Warning", false /* errno */, false /* backtrace */, __func__, __FILE__, __LINE__, __VA_ARGS__); } }

/** Use for environment-variable environment::DEBUG conditional error messages, prefix '[elapsed_time] Debug @ FILE:LINE FUNC: '. Function also appends last errno, strerror(errno) and full backtrace*/
#define jau_DBG_ERR_PRINT(...) { if( jau::environment::get().debug ) { jau_dbgPrint2(stderr, "Debug", true /* errno */, true /* backtrace */, __func__, __FILE__, __LINE__, __VA_ARGS__); } }

/**
 * Use for environment-variable environment::VERBOSE conditional verbose messages, prefix '[elapsed_time] Wordy: '.
 * <p>
 * 'Wordy' is the shorter English form of the Latin word 'verbosus', from which the word 'verbosity' is sourced.
 * </p>
 */
#define jau_WORDY_PRINT(...) { if( jau::environment::get().verbose ) { jau_dbgPrint1(stderr, true, "Wordy", __VA_ARGS__); } }

#define jau_PERF_TS_T0_BASE()  const uint64_t _t0 = jau::getCurrentMilliseconds()

#define jau_PERF_TS_TD_BASE(m)  { const uint64_t _td = jau::getCurrentMilliseconds() - _t0; \
                                  fprintf(stderr, "[%s] PERF %s done in %d ms,\n", jau::to_decstring(jau::environment::getElapsedMillisecond(), ',', 9).c_str(), (m), (int)_td); }
#ifdef PERF_PRINT_ON
    #define jau_PERF_TS_T0() jau_PERF_TS_T0_BASE()
    #define jau_PERF_TS_TD(m) jau_PERF_TS_TD_BASE(m)
#else
    #define jau_PERF_TS_T0()
    #define jau_PERF_TS_TD(m)
#endif
#ifdef PERF2_PRINT_ON
    #define jau_PERF2_TS_T0() jau_PERF_TS_T0_BASE()
    #define jau_PERF2_TS_TD(m) jau_PERF_TS_TD_BASE(m)
#else
    #define jau_PERF2_TS_T0()
    #define jau_PERF2_TS_TD(m)
#endif
#ifdef PERF3_PRINT_ON
    #define jau_PERF3_TS_T0() jau_PERF_TS_T0_BASE()
    #define jau_PERF3_TS_TD(m) jau_PERF_TS_TD_BASE(m)
#else
    #define jau_PERF3_TS_T0()
    #define jau_PERF3_TS_TD(m)
#endif

/** Use for unconditional ::abort() call with given messages, prefix '[elapsed_time] ABORT @ file:line func: '. Function also appends last errno and strerror(errno). */
#define jau_ABORT(...) { jau_dbgPrint2(stderr, "ABORT", true /* errno */, true /* backtrace */, __func__, __FILE__, __LINE__, __VA_ARGS__); abort(); }

/** Use for unconditional error messages, prefix '[elapsed_time] Error @ FILE:LINE FUNC: '. Function also appends last errno, strerror(errno) and full backtrace*/
#define jau_ERR_PRINT(...) { jau_dbgPrint2(stderr, "Error", true /* errno */, true /* backtrace */, __func__, __FILE__, __LINE__, __VA_ARGS__); }

/** Use for unconditional error messages, prefix '[elapsed_time] Error @ FILE:LINE FUNC: '. Function also appends last errno and strerror(errno). No backtrace. */
#define jau_ERR_PRINT2(...) { jau_dbgPrint2(stderr, "Error", true /* errno */, false /* backtrace */, __func__, __FILE__, __LINE__, __VA_ARGS__); }

/** Use for unconditional error messages, prefix '[elapsed_time] Error @ FILE:LINE FUNC: '. Function also appends last errno and strerror(errno). Full backtrace. */
#define jau_ERR_PRINT3(...) { jau_dbgPrint2(stderr, "Error", true /* errno */, true /* backtrace */, __func__, __FILE__, __LINE__, __VA_ARGS__); }

/** Use for unconditional interruption messages, prefix '[elapsed_time] Interrupted @ FILE:LINE FUNC: '. Function also appends last errno and strerror(errno). */
#define jau_IRQ_PRINT(...) { jau_dbgPrint2(stderr, "Interrupted", true /* errno */, false /* backtrace */, __func__, __FILE__, __LINE__, __VA_ARGS__); }

/** Use for unconditional warning messages, prefix '[elapsed_time] Warning @ FILE:LINE FUNC: ' */
#define jau_WARN_PRINT(...) { jau_dbgPrint2(stderr, "Warning", false /* errno */, false /* backtrace */, __func__, __FILE__, __LINE__, __VA_ARGS__); }

/** Use for unconditional informal messages, prefix '[elapsed_time] Info: '. */
#define jau_INFO_PRINT(fmt, ...) { jau_dbgPrint1(stderr, true, "Info", fmt __VA_OPT__(,) __VA_ARGS__); }
#define jau_INFO_PRINT_LINE(fmt, ...) { jau_dbgPrint1Line(stderr, true, "Info", fmt __VA_OPT__(,) __VA_ARGS__); }

/** Use for unconditional plain messages, prefix '[elapsed_time] ' if printPrefix == true. */
#define jau_PLAIN_PRINT(printPrefix, fmt, ...) { jau_dbgPrint1(stderr, (printPrefix), nullptr, fmt __VA_OPT__(,) __VA_ARGS__); }

/** Use for conditional plain messages, prefix '[elapsed_time] '. */
#define jau_COND_PRINT(C, ...) { if( C ) { jau::impl::dbgPrint0(stderr, false, false, __VA_ARGS__); } }

namespace jau {
    /**
     * Convenient fprintf() invocation, prepending the given elapsed_ms timestamp.
     * @param elapsed_ms the given elapsed time in milliseconds
     * @param stream the output stream
     * @param format the format
     * @param args the optional arguments
     */
    template <typename... Args>
    inline __attribute__((always_inline))
    int fprintf_td(const uint64_t elapsed_ms, FILE* stream, std::string_view format, const Args &...args) noexcept {
        int res = ::fprintf(stream, "[%s] ", jau::to_decstring(elapsed_ms, ',', 9).c_str());
        const int r = ::fputs(jau::format_string(format, args...).c_str(), stream);
        if (r >= 0) {
            res += r;
        }
        return res;
    }

    /**
     * Convenient fprintf() invocation, prepending the environment::getElapsedMillisecond() timestamp.
     * @param stream the output stream
     * @param format the format
     * @param args the optional arguments
     */
    template <typename... Args>
    inline __attribute__((always_inline))
    int fprintf_td(FILE* stream, std::string_view format, const Args &...args) noexcept {
        return fprintf_td(environment::getElapsedMillisecond(), stream, format, args...);
    }

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

#define jau_fprintf_td2(elapsed_ms, stream, fmt, ...) \
    jau::fprintf_td((elapsed_ms), (stream), (fmt) __VA_OPT__(,) __VA_ARGS__);  \
    static_assert(0 <= jau::cfmt::check2< JAU_FOR_EACH1_LIST(JAU_NOREF_DECLTYPE_VALUE, __VA_ARGS__) >(fmt)); // compile time validation!

#define jau_fprintf_td(stream, fmt, ...) \
    jau::fprintf_td((stream), (fmt) __VA_OPT__(,) __VA_ARGS__);  \
    static_assert(0 <= jau::cfmt::check2< JAU_FOR_EACH1_LIST(JAU_NOREF_DECLTYPE_VALUE, __VA_ARGS__) >(fmt)); // compile time validation!

#endif /* JAU_DEBUG_HPP_ */
