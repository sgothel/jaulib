/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020-2024 Gothel Software e.K.
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

#ifndef JAU_BASIC_TYPES_HPP_
#define JAU_BASIC_TYPES_HPP_

#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <ios>
#include <iostream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <thread>

#include <jau/byte_util.hpp>
#include <jau/cpp_lang_util.hpp>
#include <jau/fraction_type.hpp>
#include <jau/int_math.hpp>
#include <jau/int_types.hpp>
#include <jau/ordered_atomic.hpp>
#include <jau/packed_attribute.hpp>
#include <jau/type_traits_queries.hpp>

namespace jau {

    /** Simple pre-defined value pair [size_t, bool] for structured bindings to multi-values. */
    struct SizeBoolPair {
        /** a size_t value, e.g. index, length, etc */
        size_t s;
        /** a boolean value, e.g. success, etc */
        bool b;
    };

    /** Simple pre-defined value tuple [uint64_t, size_t, bool] for structured bindings to multi-values. */
    struct UInt64SizeBoolTuple {
        /** a uint64_t value, e.g. compute result value, etc */
        uint64_t v;
        /** a size_t value, e.g. index, length, etc */
        size_t s;
        /** a boolean value, e.g. success, etc */
        bool b;
    };

    /** Simple pre-defined value tuple [int64_t, size_t, bool] for structured bindings to multi-values. */
    struct Int64SizeBoolTuple {
        /** a int64_t value, e.g. compute result value, etc */
        int64_t v;
        /** a size_t value, e.g. index, length, etc */
        size_t s;
        /** a boolean value, e.g. success, etc */
        bool b;
    };

    /**
     * \ingroup Fractions
     *
     * Returns current monotonic time since Unix Epoch `00:00:00 UTC on 1970-01-01`.
     *
     * Returned fraction_timespec is passing machine precision and range of the underlying API.
     *
     * See fraction_timespec::to_fraction_i64() of how to measure duration in high range and precision:
     * <pre>
     *   fraction_timespec t0 = getMonotonicTime();
     *   // do something
     *
     *   // Exact duration
     *   fraction_timespec td_1 = getMonotonicTime() - t0;
     *
     *   // or for durations <= 292 years
     *   fraction_i64 td_2 = (getMonotonicTime() - t0).to_fraction_i64();
     * </pre>
     *
     * This is in stark contract to counting nanoseconds in int64_t which only lasts until `2262-04-12`,
     * since INT64_MAX is 9'223'372'036'854'775'807 for 9'223'372'036 seconds or 292 years.
     *
     * Monotonic time shall be used for high-performance measurements of durations,
     * since the underlying OS shall support fast calls.
     *
     * @see fraction_timespec
     * @see fraction_timespec::to_fraction_i64()
     * @see getWallClockTime()
     */
    fraction_timespec getMonotonicTime() noexcept;

    /**
     * \ingroup Fractions
     *
     * Returns current wall-clock real-time since Unix Epoch `00:00:00 UTC on 1970-01-01`.
     *
     * Returned fraction_timespec is passing machine precision and range of the underlying API.
     *
     * Wall-Clock time shall be used for accurate measurements of the actual time only,
     * since the underlying OS unlikely supports fast calls.
     *
     * @see fraction_timespec
     * @see fraction_timespec::to_fraction_i64()
     * @see getMonotonicTime()
     */
    fraction_timespec getWallClockTime() noexcept;

    /**
     * Returns current monotonic time in milliseconds.
     */
    uint64_t getCurrentMilliseconds() noexcept;

    /**
     * Returns current wall-clock system `time of day` in seconds since Unix Epoch
     * `00:00:00 UTC on 1 January 1970`.
     */
    uint64_t getWallClockSeconds() noexcept;

    /**
     * millisecond sleep using high precision monotonic timer,
     * useful for one-shot delays (only).
     *
     * Consider using jau::sleep_until or jau::sleep_for
     * utilizing absolute target time sleep when waiting
     * for an event, overcoming clock re-adjustments.
     *
     * @param td_ms duration to sleep in milliseconds
     * @param ignore_irq continue sleep when interrupted by a signal if true, defaults to true
     * @return true if completed waiting, otherwise false for interruption or error
     */
    bool milli_sleep(uint64_t td_ms, const bool ignore_irq = true) noexcept;

    /**
     * sleep using high precision monotonic timer,
     * useful for one-shot delays (only).
     *
     * Consider using jau::sleep_until or jau::sleep_for
     * utilizing absolute target time sleep when waiting
     * for an event, overcoming clock re-adjustments.
     *
     * @param relative_time an object of type fraction_timespec representing the time to sleep
     * @param ignore_irq continue sleep when interrupted by a signal if true, defaults to true
     * @return true if completed waiting, otherwise false for interruption or error
     */
    bool sleep(const fraction_timespec& relative_time, const bool ignore_irq = true) noexcept;

    /**
     * sleep_until causes the current thread to block until the  specific time is reached.
     *
     * Method works similar to std::this_thread::sleep_until(), but utilizes fraction_timespec instead of `int64_t nanoseconds counter`
     * for maintaining high-precision and infinite range.
     *
     * Implementation also uses ::clock_nanosleep(), with absolute time and either
     * monotonic- or wall-clok time depending on given monotonic flag.
     * This instead of ::nanosleep() with undefined clock-type and interruptions.
     *
     * @param absolute_time an object of type fraction_timespec representing the time when to stop waiting
     * @param monotonic if true, implementation uses the fast and steady monotonic clock (default), otherwise the wall-clock
     * @param ignore_irq continue sleep when interrupted by a signal if true, defaults to true
     * @return true if completed waiting, otherwise false for interruption or error
     * @see sleep_until()
     * @see sleep_for()
     * @see wait_until()
     * @see wait_for()
     */
    bool sleep_until(const fraction_timespec& absolute_time, const bool monotonic = true, const bool ignore_irq = true) noexcept;

    /**
     * sleep_for causes the current thread to block until a specific amount of time has passed.
     *
     * Implementation calls sleep_until() passing absolute time derived via getMonotonicTime() or getWallClockTime(), see:
     * <pre>
     *   fraction_timespec absolute_time = ( monotonic ? getMonotonicTime() : getWallClockTime() ) + relative_time;
     * </pre>
     *
     * Method works similar to std::this_thread::sleep_until(), but utilizes fraction_timespec instead of `int64_t nanoseconds counter`
     * for maintaining high-precision and infinite range.
     *
     * @param relative_time an object of type fraction_timespec representing the the maximum time to spend waiting
     * @param monotonic if true, implementation uses the fast and steady monotonic clock (default), otherwise the wall-clock
     * @param ignore_irq continue sleep when interrupted by a signal if true, defaults to true
     * @return true if completed waiting, otherwise false for interruption or error
     * @see sleep_until()
     * @see sleep_for()
     * @see wait_until()
     * @see wait_for()
     */
    bool sleep_for(const fraction_timespec& relative_time, const bool monotonic = true, const bool ignore_irq = true) noexcept;

    /**
     * sleep_for causes the current thread to block until a specific amount of time has passed.
     *
     * Implementation calls sleep_until() passing absolute time derived via getMonotonicTime() or getWallClockTime(), see:
     * <pre>
     *   fraction_timespec absolute_time = ( monotonic ? getMonotonicTime() : getWallClockTime() ) + relative_time;
     * </pre>
     *
     * Method works similar to std::this_thread::sleep_until(), but utilizes fraction_timespec instead of `int64_t nanoseconds counter`
     * for maintaining high-precision and infinite range.
     *
     * @param relative_time an object of type fraction_i64 representing the the maximum time to spend waiting, which is limited to 292 years if using nanoseconds fractions.
     * @param monotonic if true, implementation uses the fast and steady monotonic clock (default), otherwise the wall-clock
     * @param ignore_irq continue sleep when interrupted by a signal if true, defaults to true
     * @return true if completed waiting, otherwise false for interruption or error
     * @see sleep_until()
     * @see sleep_for()
     * @see wait_until()
     * @see wait_for()
     */
    bool sleep_for(const fraction_i64& relative_time, const bool monotonic = true, const bool ignore_irq = true) noexcept;

    /**
     * wait_until causes the current thread to block until the condition variable is notified, a specific time is reached, or a spurious wakeup occurs.
     *
     * Method works similar to std::condition_variable::wait_until(), but utilizes fraction_timespec instead of `int64_t nanoseconds counter`
     * for maintaining high-precision and infinite range.
     *
     * @param cv std::condition_variable instance
     * @param lock an object of type std::unique_lock<std::mutex>, which must be locked by the current thread
     * @param absolute_time an object of type fraction_timespec representing the time when to stop waiting
     * @param monotonic if true, implementation uses the fast and steady monotonic clock (default), otherwise the wall-clock
     * @return std::cv_status::timeout if the relative timeout specified by rel_time expired, std::cv_status::no_timeout otherwise.
     * @see sleep_until()
     * @see sleep_for()
     * @see wait_until()
     * @see wait_for()
     */
    std::cv_status wait_until(std::condition_variable& cv, std::unique_lock<std::mutex>& lock, const fraction_timespec& absolute_time,
                              const bool monotonic = true) noexcept;

    /**
     * wait_for causes the current thread to block until the condition variable is notified, a specific amount of time has passed, or a spurious wakeup occurs.
     *
     * Implementation calls wait_until() passing absolute time derived via getMonotonicTime() or getWallClockTime(), see:
     * <pre>
     *   fraction_timespec absolute_time = ( monotonic ? getMonotonicTime() : getWallClockTime() ) + relative_time;
     * </pre>
     *
     * Method works similar to std::condition_variable::wait_for(), but utilizes fraction_timespec instead of `int64_t nanoseconds counter`
     * for maintaining high-precision and infinite range.
     *
     * When using a condition predicate loop to ensure no spurious wake-up preemptively ends waiting for the condition variable event or timeout,
     * it is always recommended to use wait_until() using the absolute timeout time computed once before said loop. Example from latch::wait_for():
     * <pre>
            std::unique_lock<std::mutex> lock(mtx_cd);
            const fraction_timespec timeout_time = getMonotonicTime() + timeout_duration;
            while( 0 < count ) {
                std::cv_status s = wait_until(cv, lock, timeout_time);
                if( 0 == count ) {
                    return true;
                }
                if( std::cv_status::timeout == s ) {
                    return false;
                }
            }
     * </pre>
     * @param cv std::condition_variable instance
     * @param lock an object of type std::unique_lock<std::mutex>, which must be locked by the current thread
     * @param relative_time an object of type fraction_timespec representing the the maximum time to spend waiting
     * @param monotonic if true, implementation uses the fast and steady monotonic clock (default), otherwise the wall-clock
     * @return std::cv_status::timeout if the relative timeout specified by rel_time expired, std::cv_status::no_timeout otherwise.
     * @see sleep_until()
     * @see sleep_for()
     * @see wait_until()
     * @see wait_for()
     */
    std::cv_status wait_for(std::condition_variable& cv, std::unique_lock<std::mutex>& lock, const fraction_timespec& relative_time,
                            const bool monotonic = true) noexcept;

    /**
     * wait_for causes the current thread to block until the condition variable is notified, a specific amount of time has passed, or a spurious wakeup occurs.
     *
     * Implementation calls wait_until() passing absolute time derived via getMonotonicTime() or getWallClockTime(), see:
     * <pre>
     *   fraction_timespec absolute_time = ( monotonic ? getMonotonicTime() : getWallClockTime() ) + relative_time;
     * </pre>
     *
     * Method works similar to std::condition_variable::wait_for(), but utilizes fraction_timespec instead of `int64_t nanoseconds counter`
     * for maintaining high-precision and infinite range.
     *
     * When using a condition predicate loop to ensure no spurious wake-up preemptively ends waiting for the condition variable event or timeout,
     * it is always recommended to use wait_until() using the absolute timeout time computed once before said loop. Example from latch::wait_for():
     * <pre>
            std::unique_lock<std::mutex> lock(mtx_cd);
            const fraction_timespec timeout_time = getMonotonicTime() + timeout_duration;
            while( 0 < count ) {
                std::cv_status s = wait_until(cv, lock, timeout_time);
                if( 0 == count ) {
                    return true;
                }
                if( std::cv_status::timeout == s ) {
                    return false;
                }
            }
     * </pre>
     * @param cv std::condition_variable instance
     * @param lock an object of type std::unique_lock<std::mutex>, which must be locked by the current thread
     * @param relative_time an object of type fraction_i64 representing the the maximum time to spend waiting, which is limited to 292 years if using nanoseconds fractions.
     * @param monotonic if true, implementation uses the fast and steady monotonic clock (default), otherwise the wall-clock
     * @return std::cv_status::timeout if the relative timeout specified by rel_time expired, std::cv_status::no_timeout otherwise.
     * @see sleep_until()
     * @see sleep_for()
     * @see wait_until()
     * @see wait_for()
     */
    std::cv_status wait_for(std::condition_variable& cv, std::unique_lock<std::mutex>& lock, const fraction_i64& relative_time,
                            const bool monotonic = true) noexcept;

    std::string threadName(const std::thread::id id) noexcept;

    /**
    // *************************************************
    // *************************************************
    // *************************************************
     */

#define E_FILE_LINE __FILE__, __LINE__

    class ExceptionBase {
      private:
        // brief message
        std::string msg_;
        // optional whole backtrace
        std::string backtrace_;
        // brief message + optional whole backtrace
        std::string what_;

      protected:
        ExceptionBase(std::string&& type, std::string const& m, const char* file, int line) noexcept;

      public:
        virtual ~ExceptionBase() noexcept = default;
        ExceptionBase(const ExceptionBase& o) noexcept = default;
        ExceptionBase(ExceptionBase&& o) noexcept = default;
        ExceptionBase& operator=(const ExceptionBase& o) noexcept = default;
        ExceptionBase& operator=(ExceptionBase&& o) noexcept = default;

        /** Returns brief message. */
        const std::string& brief_message() const noexcept { return msg_; }
        /** Returns optional whole backtrace. */
        const std::string& backtrace() const noexcept { return backtrace_; }
        /** Returns brief message and optional whole backtrace, i.e. std::exception::what() string. */
        const std::string& whole_message() const noexcept { return what_; }

        /** Allow conversion to `const std::string&` using brief_message(), as required by Catch2's `REQUIRE_THROWS_MATCHES` */
        operator const std::string&() const noexcept { return brief_message(); };

        std::ostream& operator<<(std::ostream& out) noexcept {
            return out << what_;
        }

        virtual const char* what() const noexcept {
            return whole_message().c_str();
        }
    };
    class RuntimeExceptionBase : public ExceptionBase {
      protected:
        RuntimeExceptionBase(std::string&& type, std::string const& m, const char* file, int line) noexcept
        : ExceptionBase(std::move(type), m, file, line) { }

      public:
        ~RuntimeExceptionBase() noexcept override = default;

        RuntimeExceptionBase(const RuntimeExceptionBase& o) noexcept = default;
        RuntimeExceptionBase(RuntimeExceptionBase&& o) noexcept = default;
        RuntimeExceptionBase& operator=(const RuntimeExceptionBase& o) noexcept = default;
        RuntimeExceptionBase& operator=(RuntimeExceptionBase&& o) noexcept = default;
    };
    class LogicErrorBase : public ExceptionBase {
      protected:
        LogicErrorBase(std::string&& type, std::string const& m, const char* file, int line) noexcept
        : ExceptionBase(std::move(type), m, file, line) { }

      public:
        ~LogicErrorBase() noexcept override = default;

        LogicErrorBase(const LogicErrorBase& o) noexcept = default;
        LogicErrorBase(LogicErrorBase&& o) noexcept = default;
        LogicErrorBase& operator=(const LogicErrorBase& o) noexcept = default;
        LogicErrorBase& operator=(LogicErrorBase&& o) noexcept = default;
    };
    class RuntimeSystemExceptionBase : public RuntimeExceptionBase {
      protected:
        std::error_code m_ec;
        RuntimeSystemExceptionBase(std::string&& type, const std::error_code& ec, std::string const& m, const char* file, int line) noexcept
        : RuntimeExceptionBase(std::move(type), m, file, line), m_ec(ec) { }

      public:
        ~RuntimeSystemExceptionBase() noexcept override = default;

        RuntimeSystemExceptionBase(const RuntimeSystemExceptionBase& o) noexcept = default;
        RuntimeSystemExceptionBase(RuntimeSystemExceptionBase&& o) noexcept = default;
        RuntimeSystemExceptionBase& operator=(const RuntimeSystemExceptionBase& o) noexcept = default;
        RuntimeSystemExceptionBase& operator=(RuntimeSystemExceptionBase&& o) noexcept = default;

        const std::error_code& code() const noexcept { return m_ec; }
    };

    class OutOfMemoryError : public ExceptionBase,
                             public std::bad_alloc {
      public:
        OutOfMemoryError(std::string const& m, const char* file, int line)
        : ExceptionBase("OutOfMemoryError", m, file, line), bad_alloc() { }

        const char* what() const noexcept override {
            return whole_message().c_str();
        }
    };

    class RuntimeException : public RuntimeExceptionBase,
                             public std::runtime_error {
      protected:
        RuntimeException(std::string&& type, std::string const& m, const char* file, int line) noexcept
        : RuntimeExceptionBase(std::move(type), m, file, line), runtime_error(whole_message()) { }

      public:
        RuntimeException(std::string const& m, const char* file, int line) noexcept
        : RuntimeException("RuntimeException", m, file, line) { }

        ~RuntimeException() noexcept override = default;

        RuntimeException(const RuntimeException& o) noexcept = default;
        RuntimeException(RuntimeException&& o) noexcept = default;
        RuntimeException& operator=(const RuntimeException& o) noexcept = default;
        RuntimeException& operator=(RuntimeException&& o) noexcept = default;

        // base class std::exception:
        const char* what() const noexcept override {
            return whole_message().c_str();
        }
    };
    class LogicError : public LogicErrorBase,
                       public std::logic_error {
      protected:
        LogicError(std::string&& type, std::string const& m, const char* file, int line) noexcept
        : LogicErrorBase(std::move(type), m, file, line), logic_error(whole_message()) { }

      public:
        LogicError(std::string const& m, const char* file, int line) noexcept
        : LogicError("LogicErrorStd", m, file, line) { }

        ~LogicError() noexcept override = default;

        LogicError(const LogicError& o) noexcept = default;
        LogicError(LogicError&& o) noexcept = default;
        LogicError& operator=(const LogicError& o) noexcept = default;
        LogicError& operator=(LogicError&& o) noexcept = default;

        const char* what() const noexcept override {
            return whole_message().c_str();
        }
    };
    class RuntimeSystemException : public RuntimeSystemExceptionBase,
                                   public std::system_error {
      protected:
        RuntimeSystemException(std::string&& type, const std::error_code& ec, std::string const& m, const char* file, int line) noexcept
        : RuntimeSystemExceptionBase(std::move(type), ec, m, file, line), system_error(ec, whole_message()) { }

      public:
        RuntimeSystemException(const std::error_code& ec, std::string const& m, const char* file, int line) noexcept
        : RuntimeSystemException("RuntimeSystemExceptionStd", ec, m, file, line) { }

        ~RuntimeSystemException() noexcept override = default;

        RuntimeSystemException(const RuntimeSystemException& o) noexcept = default;
        RuntimeSystemException(RuntimeSystemException&& o) noexcept = default;
        RuntimeSystemException& operator=(const RuntimeSystemException& o) noexcept = default;
        RuntimeSystemException& operator=(RuntimeSystemException&& o) noexcept = default;

        const char* what() const noexcept override {
            return whole_message().c_str();
        }
    };

    class IndexOutOfBoundsError : public LogicErrorBase,
                                  public std::out_of_range {
      protected:
        IndexOutOfBoundsError(const char* file, int line, std::string&& type, std::string const& m) noexcept
        : LogicErrorBase(std::move(type), m, file, line), out_of_range(whole_message()) { }

      public:
        IndexOutOfBoundsError(const std::size_t index, const std::size_t length, const char* file, int line) noexcept
        : IndexOutOfBoundsError(file, line, "IndexOutOfBoundsError", "Index " + std::to_string(index) + ", data length " + std::to_string(length)) { }

        IndexOutOfBoundsError(const std::string& msg, const std::size_t index, const std::size_t length, const char* file, int line) noexcept
        : IndexOutOfBoundsError(file, line, "IndexOutOfBoundsError", msg + ": index " + std::to_string(index) + ", data length " + std::to_string(length)) { }

        IndexOutOfBoundsError(const std::string& index_s, const std::string& length_s, const char* file, int line) noexcept
        : IndexOutOfBoundsError(file, line, "IndexOutOfBoundsError", "Index " + index_s + ", data length " + length_s) { }

        IndexOutOfBoundsError(const std::size_t index, const std::size_t count, const std::size_t length, const char* file, int line) noexcept
        : IndexOutOfBoundsError(file, line, "IndexOutOfBoundsError", "Index " + std::to_string(index) + ", count " + std::to_string(count) + ", data length " + std::to_string(length)) { }

        const char* what() const noexcept override {
            return whole_message().c_str();
        }
    };

    class IllegalArgumentError : public LogicErrorBase,
                                 public std::invalid_argument {
      protected:
        IllegalArgumentError(std::string&& type, std::string const& m, const char* file, int line) noexcept
        : LogicErrorBase(std::move(type), m, file, line), invalid_argument(whole_message()) { }

      public:
        IllegalArgumentError(std::string const& m, const char* file, int line) noexcept
        : IllegalArgumentError("IllegalArgumentError", m, file, line) { }

        const char* what() const noexcept override {
            return whole_message().c_str();
        }
    };

    class IllegalStateError : public LogicErrorBase,
                              public std::domain_error {
      protected:
        IllegalStateError(std::string&& type, std::string const& m, const char* file, int line) noexcept
        : LogicErrorBase(std::move(type), m, file, line), domain_error(whole_message()) { }

      public:
        IllegalStateError(std::string const& m, const char* file, int line) noexcept
        : IllegalStateError("IllegalStateError", m, file, line) { }

        const char* what() const noexcept override {
            return whole_message().c_str();
        }
    };

    class IOError : public RuntimeSystemExceptionBase,
                    public std::ios_base::failure {
      public:
        IOError(std::string const& m, const char* file, int line, const std::error_code& ec = std::io_errc::stream) noexcept
        : RuntimeSystemExceptionBase("IOError", ec, m, file, line), failure(whole_message(), ec) { }

        const char* what() const noexcept override {
            return whole_message().c_str();
        }
    };

    class InternalError : public RuntimeException {
      public:
        InternalError(std::string const& m, const char* file, int line) noexcept
        : RuntimeException("InternalError", m, file, line) { }
    };

    class NotImplementedException : public RuntimeException {
      public:
        NotImplementedException(std::string const& m, const char* file, int line) noexcept
        : RuntimeException("NotImplementedException", m, file, line) { }
    };

    class NullPointerException : public RuntimeException {
      public:
        NullPointerException(std::string const& m, const char* file, int line) noexcept
        : RuntimeException("NullPointerException", m, file, line) { }
    };

    class UnsupportedOperationException : public RuntimeException {
      public:
        UnsupportedOperationException(std::string const& m, const char* file, int line) noexcept
        : RuntimeException("UnsupportedOperationException", m, file, line) { }
    };

    /**
    // *************************************************
    // *************************************************
    // *************************************************
     */

    /** \addtogroup Integer
     *
     *  @{
     */

    inline void set_bit_uint32(const uint8_t nr, uint32_t& mask) {
        using namespace jau::int_literals;
        if ( nr > 31 ) { throw IndexOutOfBoundsError(nr, 32, E_FILE_LINE); }
        mask |= 1_u32 << (nr & 31);
    }

    inline void clear_bit_uint32(const uint8_t nr, uint32_t& mask) {
        using namespace jau::int_literals;
        if ( nr > 31 ) { throw IndexOutOfBoundsError(nr, 32, E_FILE_LINE); }
        mask |= ~(1_u32 << (nr & 31));
    }

    inline uint32_t test_bit_uint32(const uint8_t nr, const uint32_t mask) {
        using namespace jau::int_literals;
        if ( nr > 31 ) { throw IndexOutOfBoundsError(nr, 32, E_FILE_LINE); }
        return mask & (1_u32 << (nr & 31));
    }

    inline void set_bit_uint64(const uint8_t nr, uint64_t& mask) {
        using namespace jau::int_literals;
        if ( nr > 63 ) { throw IndexOutOfBoundsError(nr, 64, E_FILE_LINE); }
        mask |= 1_u64 << (nr & 63);
    }

    inline void clear_bit_uint64(const uint8_t nr, uint64_t& mask) {
        using namespace jau::int_literals;
        if ( nr > 63 ) { throw IndexOutOfBoundsError(nr, 64, E_FILE_LINE); }
        mask |= ~(1_u64 << (nr & 63));
    }

    inline uint64_t test_bit_uint64(const uint8_t nr, const uint64_t mask) {
        using namespace jau::int_literals;
        if ( nr > 63 ) { throw IndexOutOfBoundsError(nr, 64, E_FILE_LINE); }
        return mask & (1_u64 << (nr & 63));
    }

    /**
    // *************************************************
    // *************************************************
    // *************************************************
     */

    /**
     * Merge the given 'uuid16' into a 'base_uuid' copy at the given little endian 'uuid16_le_octet_index' position.
     * <p>
     * The given 'uuid16' value will be added with the 'base_uuid' copy at the given position.
     * </p>
     * <pre>
     * base_uuid: 00000000-0000-1000-8000-00805F9B34FB
     *    uuid16: DCBA
     * uuid16_le_octet_index: 12
     *    result: 0000DCBA-0000-1000-8000-00805F9B34FB
     *
     * LE: low-mem - FB349B5F8000-0080-0010-0000-ABCD0000 - high-mem
     *                                           ^ index 12
     * LE: uuid16 -> value.data[12+13]
     *
     * BE: low-mem - 0000DCBA-0000-1000-8000-00805F9B34FB - high-mem
     *                   ^ index 2
     * BE: uuid16 -> value.data[2+3]
     * </pre>
     */
    uint128dp_t merge_uint128(uint16_t const uuid16, uint128dp_t const& base_uuid, nsize_t const uuid16_le_octet_index);

    /**
     * Merge the given 'uuid32' into a 'base_uuid' copy at the given little endian 'uuid32_le_octet_index' position.
     * <p>
     * The given 'uuid32' value will be added with the 'base_uuid' copy at the given position.
     * </p>
     * <pre>
     * base_uuid: 00000000-0000-1000-8000-00805F9B34FB
     *    uuid32: 87654321
     * uuid32_le_octet_index: 12
     *    result: 87654321-0000-1000-8000-00805F9B34FB
     *
     * LE: low-mem - FB349B5F8000-0080-0010-0000-12345678 - high-mem
     *                                           ^ index 12
     * LE: uuid32 -> value.data[12..15]
     *
     * BE: low-mem - 87654321-0000-1000-8000-00805F9B34FB - high-mem
     *               ^ index 0
     * BE: uuid32 -> value.data[0..3]
     * </pre>
     */
    uint128dp_t merge_uint128(uint32_t const uuid32, uint128dp_t const& base_uuid, nsize_t const uuid32_le_octet_index);

    /**@}*/

}  // namespace jau

/** \example test_intdecstring01.cpp
 * This C++ unit test validates the jau::to_decstring implementation
 */

#endif /* JAU_BASIC_TYPES_HPP_ */
