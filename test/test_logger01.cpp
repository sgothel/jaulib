/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2024 Gothel Software e.K.
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

#include <cstdio>
#include <cstring>

#include <jau/test/catch2_ext.hpp>

#include <jau/cpp_lang_util.hpp>
#include <jau/debug.hpp>
#include <jau/functional.hpp>
#include <jau/string_cfmt.hpp>

/////
///// BEGIN logger.hpp
/////

namespace jau {
    enum class log_level : uint32_t {
        none        =   0,
        fatal       =  10,
        critical    =  20,
        error       =  30,
        warning     =  40,
        info        = 100,
        debug       = 200,
        trace       = 300
    };
    constexpr log_level log_level_default = log_level::warning;

    enum class time_format {
        elapsed_millis  = 0,
        timestamp       = 1
    };

    template<typename... Args>
    constexpr size_t check_cfmt(std::string_view fmt) { // NOLINT
        return jau::cfmt::check2<Args...>(fmt);
    }

    template<log_level Level=log_level_default>
    class Logger {
      public:
        typedef jau::function<void(std::string &sink)> append_func;

        static constexpr log_level level = Level;
        using self_t = Logger<Level>;

      private:
        FILE *m_out;
        time_format m_tf;
        std::string m_str;

        template <typename... Args>
        void debugImpl(std::string_view format, const Args &...args) noexcept {
            if constexpr (level >= log_level::debug) {
                m_str.clear();
                // jau::impl::dbgPrint1(out, true, "Debug", format, args...);
                if (!jau::impl::dbgPrint_pre(jau::cfmt::default_string_capacity, m_str, true, "Debug", ": ", nullptr /* func */, nullptr /* file */, 0 /* line */)) {
                    return;
                }
                jau::cfmt::append(m_str, std::numeric_limits<size_t>::max(), format, args...);
                if (m_out) {
                    jau::impl::fprintf_tail(m_out, m_str);
                }
            }
        }

      public:
        Logger(FILE *o, time_format tf=time_format::elapsed_millis) noexcept
        : m_out(o), m_tf(tf)
        { }

        std::string_view last() noexcept { return m_str; }

#if 0
        template <typename... Args>
        CXX_ALWAYS_INLINE
        void debug(std::string_view format, const Args &...args) noexcept {
            if constexpr (level >= log_level::debug) {
                debugImpl(format, args...);
            }
        }

        template <typename... Args>
        void debug(append_func af) noexcept {
            if constexpr (level >= log_level::debug) {
                m_str.clear();
                // jau::impl::dbgPrint1(out, true, "Debug", format, args...);
                if (!jau::impl::dbgPrint_pre(jau::cfmt::default_string_capacity, m_str, true, "Debug", ": ", nullptr /* func */, nullptr /* file */, 0 /* line */)) {
                    return;
                }
                af(m_str);
                if (m_out) {
                    jau::impl::fprintf_tail(m_out, m_str);
                }
            }
        }
#else
        template <typename Dummy = bool, typename... Args,
                  typename std::enable_if_t<jau::enums::compare(level, jau::log_level::debug) >= 0, Dummy> = false >
        void debug(std::string_view format, const Args &...args) noexcept {
            m_str.clear();
            // jau::impl::dbgPrint1(out, true, "Debug", format, args...);
            if (!jau::impl::dbgPrint_pre(jau::cfmt::default_string_capacity, m_str, true, "Debug", ": ", nullptr /* func */, nullptr /* file */, 0 /* line */)) {
                return;
            }
            jau::cfmt::append(m_str, std::numeric_limits<size_t>::max(), format, args...);
            if (m_out) {
                jau::impl::fprintf_tail(m_out, m_str);
            }
        }

        template <typename Dummy = bool, typename... Args,
                  typename std::enable_if_t<jau::enums::compare(level, jau::log_level::debug) < 0, Dummy> = false >
        CXX_ALWAYS_INLINE
        void debug(std::string_view, const Args &...) noexcept {
        }

        template <typename Dummy = bool, typename... Args,
                  typename std::enable_if_t<jau::enums::compare(level, jau::log_level::debug) >= 0, Dummy> = false >
        void debug(append_func &&af) noexcept {
            m_str.clear();
            // jau::impl::dbgPrint1(out, true, "Debug", format, args...);
            if (!jau::impl::dbgPrint_pre(jau::cfmt::default_string_capacity, m_str, true, "Debug", ": ", nullptr /* func */, nullptr /* file */, 0 /* line */)) {
                return;
            }
            af(m_str);
            if (m_out) {
                jau::impl::fprintf_tail(m_out, m_str);
            }
        }

        template <typename Dummy = bool, typename... Args,
                  typename std::enable_if_t<jau::enums::compare(level, jau::log_level::debug) < 0, Dummy> = false >
        CXX_ALWAYS_INLINE
        void debug(append_func &&) noexcept {
        }

#endif
    };

    using DefaultLogger = Logger<log_level_default>;
    using WarningLogger = Logger<log_level::warning>;
    using DebugLogger = Logger<log_level::debug>;
    using TraceLogger = Logger<log_level::trace>;

} // namespace jau

#define jau_log_debug(logger, fmt, ...) \
    if (jau::enums::compare(logger.level, jau::log_level::debug)>=0) { (logger).debug((fmt) __VA_OPT__(,) __VA_ARGS__); } \
    static_assert(0 <= jau::cfmt::check2< JAU_FOR_EACH1_LIST(JAU_NOREF_DECLTYPE_VALUE, __VA_ARGS__) >(fmt)); // compile time validation!

/////
///// END logger.hpp
/////

static int count_int_trace = 0;
static int getIntTrace() noexcept {
    ++count_int_trace;
    return 1;
}

static int count_int_warning = 0;
static int getIntWarning() noexcept {
    ++count_int_warning;
    return 2;
}

static void reset_stats() noexcept {
    count_int_trace = 0;
    count_int_warning = 0;
}

TEST_CASE( "Logger 10", "[logger][jau::cfmt]" ) {
    // lambda: worst: not fast due to setup and passing functor
    {
        reset_stats();
        jau::TraceLogger log(stderr);
        log.debug([](std::string &sink) noexcept { jau::cfmt::append(sink, "Hello World %d", getIntTrace()); });
        REQUIRE(1 == count_int_trace);
        // REQUIRE("Debug: Hallo World 1\n" == log.last().substr(12));
    }
    {
        reset_stats();
        jau::DefaultLogger log(stderr);
        log.debug([](std::string &sink) noexcept { jau::cfmt::append(sink, "Hello World %d", getIntWarning()); });
        REQUIRE(0 == count_int_warning);
        // REQUIRE("Debug: Hallo World 1\n" == log.last().substr(12)); // `[        2] `
    }
    // macro: best: fast + sure
    {
        reset_stats();
        jau::TraceLogger log(stderr);
        jau_log_debug(log, "Hello World %d", getIntTrace());
        REQUIRE(1 == count_int_trace);
        // REQUIRE("Debug: Hallo World 1\n" == log.last().substr(12));
    }
    {
        reset_stats();
        jau::DefaultLogger log(stderr);
        jau_log_debug(log, "Hello World %d", getIntWarning());
        REQUIRE(0 == count_int_warning);
        // REQUIRE("Debug: Hallo World 1\n" == log.last().substr(12)); // `[        2] `
    }
    // direct: good: fast + not so sure (format arguments)
    {
        reset_stats();
        jau::TraceLogger log(stderr);
        log.debug("Hello World %d", getIntTrace());
        REQUIRE(1 == count_int_trace);
        // REQUIRE("Debug: Hallo World 1\n" == log.last().substr(12));
    }
    {
        reset_stats();
        jau::DefaultLogger log(stderr);
        log.debug("Hello World %d", getIntWarning()); // FIXME: getIntWarning() gets called
        REQUIRE(1 == count_int_warning);
        // REQUIRE("Debug: Hallo World 1\n" == log.last().substr(12)); // `[        2] `
    }
}

TEST_CASE( "Logger 88 Perf", "[logger][jau::cfmt]" ) {
    const size_t loops = 1000; // catch_auto_run ? 1000 : 1000;

    BENCHMARK("fmt1.32 append       Trace Direct") {
        jau::TraceLogger log(nullptr);
        float fa = 1.1f, fb = 2.2f;
        size_t sz1 = 1;
        uint64_t sz2 = 2;
        int i1 = 3;
        std::string str1 = "Hi World";

        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            log.debug("format_check: %.2f, %2.2f, %zu, %" PRIu64 ", %03d, %10s", fa, fb, sz1, sz2, i1, str1);
            res = res + log.last().size();
        }
        return res;
    };
    BENCHMARK("fmt1.32 append       Trace Lambda") {
        jau::TraceLogger log(nullptr);
        float fa = 1.1f, fb = 2.2f;
        size_t sz1 = 1;
        uint64_t sz2 = 2;
        int i1 = 3;
        std::string str1 = "Hi World";

        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            log.debug([&](std::string &sink) noexcept { jau::cfmt::append(sink, "format_check: %.2f, %2.2f, %zu, %" PRIu64 ", %03d, %10s", fa, fb, sz1, sz2, i1, str1); });
            res = res + log.last().size();
        }
        return res;
    };
    BENCHMARK("fmt1.32 append       Trace Macro") {
        jau::TraceLogger log(nullptr);
        float fa = 1.1f, fb = 2.2f;
        size_t sz1 = 1;
        uint64_t sz2 = 2;
        int i1 = 3;
        std::string str1 = "Hi World";

        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            jau_log_debug(log, "format_check: %.2f, %2.2f, %zu, %" PRIu64 ", %03d, %10s", fa, fb, sz1, sz2, i1, str1);
            res = res + log.last().size();
        }
        return res;
    };
    BENCHMARK("fmt1.32 append       Warning Direct") {
        jau::WarningLogger log(nullptr);
        float fa = 1.1f, fb = 2.2f;
        size_t sz1 = 1;
        uint64_t sz2 = 2;
        int i1 = 3;
        std::string str1 = "Hi World";

        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            log.debug("format_check: %.2f, %2.2f, %zu, %" PRIu64 ", %03d, %10s", fa, fb, sz1, sz2, i1, str1);
            res = res + log.last().size();
        }
        return res;
    };
    BENCHMARK("fmt1.32 append       Warning Lambda") {
        jau::WarningLogger log(nullptr);
        float fa = 1.1f, fb = 2.2f;
        size_t sz1 = 1;
        uint64_t sz2 = 2;
        int i1 = 3;
        std::string str1 = "Hi World";

        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            log.debug([&](std::string &sink) noexcept { jau::cfmt::append(sink, "format_check: %.2f, %2.2f, %zu, %" PRIu64 ", %03d, %10s", fa, fb, sz1, sz2, i1, str1); });
            res = res + log.last().size();
        }
        return res;
    };
    BENCHMARK("fmt1.32 append       Warning Macro") {
        jau::WarningLogger log(nullptr);
        float fa = 1.1f, fb = 2.2f;
        size_t sz1 = 1;
        uint64_t sz2 = 2;
        int i1 = 3;
        std::string str1 = "Hi World";

        volatile size_t res = 0;
        for( size_t i = 0; i < loops; ++i ) {
            jau_log_debug(log, "format_check: %.2f, %2.2f, %zu, %" PRIu64 ", %03d, %10s", fa, fb, sz1, sz2, i1, str1);
            res = res + log.last().size();
        }
        return res;
    };
}
