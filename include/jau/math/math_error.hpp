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

namespace jau::math {

    /** @defgroup Math Math Support
     * Math Support Functionality, e.g. linear algebra, meta group
     *
     * Further support is coming from
     * - \ref Integer
     * - \ref ConstantTime
     * - \ref Floats
     *
     *  @{
     */

    /** Error types as specified by [C++ Math Error Handling](https://en.cppreference.com/w/cpp/numeric/math/math_errhandling) */
    enum class math_error_t : uint16_t {
            /** no math error */
            none = 0,
            /** See FE_INVALID, i.e. MathDomainError, std::domain_error : std::logic_error */
            invalid,
            /** See FE_DIVBYZERO, i.e. MathDivByZeroError, std::domain_error : std::logic_error*/
            div_by_zero,
            /** See FE_OVERFLOW, i.e. MathOverflowError, std::overflow_error : std::runtime_error */
            overflow,
            /** See FE_UNDERFLOW, i.e. MathUnderflowError, std::underflow_error : std::runtime_error */
            underflow,
            /** See FE_INEXACT, i.e. MathInexactError, std::runtime_error */
            inexact,
            /** undefined math error */
            undefined = 1U << 15,
    };
    /** Returns std::string representation of math_error_t */
    std::string to_string(const math_error_t v) noexcept;

    class MathErrorBase : public ExceptionBase {
      private:
        math_error_t m_error;

      protected:
        MathErrorBase(math_error_t err, std::string const& m, const char* file, int line) noexcept
        : ExceptionBase("MathError("+to_string(err)+")", m, file, line), m_error(err) {}
        
      public:        
        math_error_t error() const noexcept;                
    };
    class MathRuntimeErrorBase : public MathErrorBase {
      protected:
        MathRuntimeErrorBase(math_error_t err, std::string const& m, const char* file, int line) noexcept
        : MathErrorBase(err, m, file, line) {}
    };
    
    class MathError : public MathErrorBase, public std::exception {
      public:
        MathError(math_error_t err, std::string const& m, const char* file, int line) noexcept
        : MathErrorBase(err, m, file, line), exception() {}
                
        const char* what() const noexcept override {
            return whole_message().c_str();
        }
    };
    
    /** math_error_t::inexact */
    class MathInexactError : public MathRuntimeErrorBase, public std::runtime_error {
      public:
        MathInexactError(std::string const& m, const char* file, int line) noexcept
        : MathRuntimeErrorBase(math_error_t::inexact, m, file, line), runtime_error(whole_message()) {}
                
        const char* what() const noexcept override {
            return whole_message().c_str();
        }
    };
    
    /** math_error_t::overflow */
    class MathOverflowError : public MathRuntimeErrorBase, public std::overflow_error {
      public:
        MathOverflowError(std::string const& m, const char* file, int line) noexcept
        : MathRuntimeErrorBase(math_error_t::overflow, m, file, line), overflow_error(whole_message()) {}        
                
        const char* what() const noexcept override {
            return whole_message().c_str();
        }
    };
    
    /** math_error_t::underflow */
    class MathUnderflowError : public MathRuntimeErrorBase, public std::underflow_error {
      public:
        MathUnderflowError(std::string const& m, const char* file, int line) noexcept
        : MathRuntimeErrorBase(math_error_t::underflow, m, file, line), underflow_error(whole_message()) {}        
                
        const char* what() const noexcept override {
            return whole_message().c_str();
        }
    };

    /** math_error_t::invalid */
    class MathDomainError : public MathErrorBase, public std::domain_error {
      protected:
        MathDomainError(math_error_t err, std::string const& m, const char* file, int line) noexcept
        : MathErrorBase(err, m, file, line), domain_error(whole_message()) {}
        
      public:
        MathDomainError(std::string const& m, const char* file, int line) noexcept
        : MathErrorBase(math_error_t::invalid, m, file, line), domain_error(whole_message()) {}        
                
        const char* what() const noexcept override {
            return whole_message().c_str();
        }
    };
    
    /** math_error_t::div_by_zero, i.e. pole error */
    class MathDivByZeroError : public MathDomainError {
      public:
        MathDivByZeroError(std::string const& m, const char* file, int line) noexcept
        : MathDomainError(math_error_t::div_by_zero, m, file, line) {}
    };
    
    /**@}*/

} // namespace jau

#endif // JAU_MATH_HPP_
