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

#ifndef JAU_MATH_HPP_
#define JAU_MATH_HPP_

#include <stdexcept>
#include <jau/int_math.hpp>
#include <jau/basic_types.hpp>

namespace jau {

    /** @defgroup Math Mathematical Operations
     * Mathematical operations, meta group
     *
     * Further support is coming from
     * - \ref Integer
     * - \ref ConstantTime
     * - \ref Floats
     *
     *  @{
     */

    /** Error types as specified by [C++ Math Error Handling](https://en.cppreference.com/w/cpp/numeric/math/math_errhandling) */
    enum class math_error_t {
            /** See FE_INVALID */
            invalid,
            /** See FE_DIVBYZERO */
            div_by_zero,
            /** See FE_OVERFLOW */
            overflow,
            /** See FE_UNDERFLOW */
            underflow,
            /** See FE_INEXACT */
            inexact
    };
    /** Returns std::string representation of math_error_t */
    std::string to_string(const math_error_t v) noexcept;

    class MathError : public RuntimeException {
      private:
        math_error_t error;

      public:
        MathError(math_error_t err, std::string const& m, const char* file, int line) noexcept
        : RuntimeException("MathError("+to_string(err)+")", m, file, line), error(err) {}
    };
    /** math_error_t::invalid */
    class MathDomainError : public MathError {
      public:
        MathDomainError(std::string const& m, const char* file, int line) noexcept
        : MathError(math_error_t::invalid, m, file, line) {}
    };
    /** math_error_t::div_by_zero, i.e. pole error */
    class MathDivByZeroError : public MathError {
      public:
        MathDivByZeroError(std::string const& m, const char* file, int line) noexcept
        : MathError(math_error_t::div_by_zero, m, file, line) {}
    };
    /** math_error_t::overflow */
    class MathOverflowError : public MathError {
      public:
        MathOverflowError(std::string const& m, const char* file, int line) noexcept
        : MathError(math_error_t::overflow, m, file, line) {}
    };
    /** math_error_t::underflow */
    class MathUnderflowError : public MathError {
      public:
        MathUnderflowError(std::string const& m, const char* file, int line) noexcept
        : MathError(math_error_t::underflow, m, file, line) {}
    };
    /** math_error_t::inexact */
    class MathInexactError : public MathError {
      public:
        MathInexactError(std::string const& m, const char* file, int line) noexcept
        : MathError(math_error_t::inexact, m, file, line) {}
    };

    /**@}*/

} // namespace jau

#endif // JAU_MATH_HPP_
