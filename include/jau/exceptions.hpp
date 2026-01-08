/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020-2026 Gothel Software e.K.
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

#ifndef JAU_EXCEPTIONS_HPP_
#define JAU_EXCEPTIONS_HPP_

#include <ios>
#include <stdexcept>
#include <string>
#include <system_error>

#include <jau/cpp_lang_util.hpp>
#include <jau/functional.hpp>

namespace jau {

    /** @defgroup Exceptions Exception types and functions
     * Exception types and functions.
     *
     *  @{
     */

    /**
     * Handle given optional exception (nullable std::exception_ptr) and send std::exception::what() message to `stderr`
     * @param eptr contains optional exception, may be `nullptr`
     * @return true if `eptr` contained an exception pointer, message was not `nullptr`
     */
    inline bool handle_exception(std::exception_ptr eptr) { // NOLINT(performance-unnecessary-value-param) passing by value is OK
        try {
            if (eptr) {
                std::rethrow_exception(eptr);
            }
        } catch (const std::exception &e) {
            ::fprintf(stderr, "Exception caught: %s\n", e.what());
            return true;
        }
        return false;
    }

    typedef jau::function<bool(const std::exception &)> exception_handler_t;

    /**
     * Handle given optional exception (nullable std::exception_ptr) and calls exception_handler_t
     * @param eptr contains optional exception, may be `nullptr`
     * @param eh exception_handler_t to process an eventual exception
     * @return true if `eptr` contained an exception pointer, exception_handler_t result is returned. Otherwise false (no exception).
     */
    inline bool handle_exception(std::exception_ptr eptr, jau::exception_handler_t &eh) { // NOLINT(performance-unnecessary-value-param) passing by value is OK
        try {
            if (eptr) {
                std::rethrow_exception(eptr);
            }
        } catch (const std::exception &e) {
            return eh(e);
        }
        return false;
    }

    /**
    // *************************************************
    // *************************************************
    // *************************************************
     */

    /** \addtogroup Exceptions
     *
     *  @{
     */

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

    /**@}*/

}  // namespace jau

#endif /* JAU_EXCEPTIONS_HPP_ */
