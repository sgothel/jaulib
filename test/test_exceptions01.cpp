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
#include <cstring>
#include <exception>
#include <stdexcept>
#include <system_error>

#include <jau/test/catch2_ext.hpp>

#include <jau/basic_types.hpp>
#include <jau/math/math_error.hpp>
#include <jau/mp/big_int.hpp>

using namespace jau;

static void throwOutOfMemoryError() {
    throw jau::OutOfMemoryError("test", E_FILE_LINE);
}
static void throwRuntimeException() {
    throw jau::RuntimeException("test", E_FILE_LINE);
}
static void throwLogicError() {
    throw jau::LogicError("test", E_FILE_LINE);
}
static void throwIndexOutOfBoundsError() {
    throw jau::IndexOutOfBoundsError(10, 0, E_FILE_LINE);
}
static void throwIllegalArgumentError() {
    throw jau::IllegalArgumentError("test", E_FILE_LINE);
}
static void throwIllegalStateError() {
    throw jau::IllegalStateError("test", E_FILE_LINE);
}
static void throwRuntimeSystemException() {
    throw jau::RuntimeSystemException(std::error_code(), "test", E_FILE_LINE);
}
static void throwIOError() {
    throw jau::IOError("test", E_FILE_LINE);
}
static void throwInternalError() {
    throw jau::InternalError("test", E_FILE_LINE);
}
static void throwNotImplementedException() {
    throw jau::NotImplementedException("test", E_FILE_LINE);
}
static void throwNullPointerException() {
    throw jau::NullPointerException("test", E_FILE_LINE);
}
static void throwUnsupportedOperationException() {
    throw jau::UnsupportedOperationException("test", E_FILE_LINE);
}

TEST_CASE( "Exception 00", "[exceptions][error]" ) {
    static_assert( true == std::is_base_of_v<jau::ExceptionBase, jau::OutOfMemoryError>);
    static_assert( true == std::is_base_of_v<std::bad_alloc, jau::OutOfMemoryError>);
    static_assert( true == std::is_base_of_v<std::exception, jau::OutOfMemoryError>);
    REQUIRE_THROWS_MATCHES( throwOutOfMemoryError(), jau::OutOfMemoryError, Catch::Matchers::ContainsSubstring("OutOfMemoryError") );
    REQUIRE_THROWS_AS(      throwOutOfMemoryError(), jau::ExceptionBase);
    REQUIRE_THROWS_AS(      throwOutOfMemoryError(), std::bad_alloc);
    REQUIRE_THROWS_AS(      throwOutOfMemoryError(), std::exception);
    
    static_assert( true == std::is_base_of_v<jau::ExceptionBase, jau::RuntimeExceptionBase>);
    static_assert( true == std::is_base_of_v<jau::RuntimeExceptionBase, jau::RuntimeException>);
    static_assert( true == std::is_base_of_v<std::runtime_error, jau::RuntimeException>);
    static_assert( true == std::is_base_of_v<std::exception, jau::RuntimeException>);
    REQUIRE_THROWS_MATCHES( throwRuntimeException(), jau::RuntimeException, Catch::Matchers::ContainsSubstring("RuntimeException") );
    REQUIRE_THROWS_AS(      throwRuntimeException(), jau::RuntimeExceptionBase);
    REQUIRE_THROWS_AS(      throwRuntimeException(), std::runtime_error);
    REQUIRE_THROWS_AS(      throwRuntimeException(), std::exception);
    
    static_assert( true == std::is_base_of_v<jau::ExceptionBase, jau::LogicErrorBase>);
    static_assert( true == std::is_base_of_v<jau::LogicErrorBase, jau::LogicError>);
    static_assert( true == std::is_base_of_v<std::logic_error, jau::LogicError>);
    static_assert( true == std::is_base_of_v<std::exception, jau::LogicError>);
    REQUIRE_THROWS_MATCHES( throwLogicError(), jau::LogicError, Catch::Matchers::ContainsSubstring("LogicError") );
    REQUIRE_THROWS_AS(      throwLogicError(), jau::LogicErrorBase);
    REQUIRE_THROWS_AS(      throwLogicError(), std::logic_error);
    REQUIRE_THROWS_AS(      throwLogicError(), std::exception);
    
    static_assert( true == std::is_base_of_v<std::out_of_range, jau::IndexOutOfBoundsError>);
    static_assert( true == std::is_base_of_v<jau::LogicErrorBase, jau::IndexOutOfBoundsError>);
    static_assert( true == std::is_base_of_v<std::logic_error, jau::IndexOutOfBoundsError>);
    REQUIRE_THROWS_MATCHES( throwIndexOutOfBoundsError(), jau::IndexOutOfBoundsError, Catch::Matchers::ContainsSubstring("IndexOutOfBoundsError") );
    REQUIRE_THROWS_AS(      throwIndexOutOfBoundsError(), std::out_of_range);
    REQUIRE_THROWS_MATCHES( throwIndexOutOfBoundsError(), jau::LogicErrorBase, Catch::Matchers::ContainsSubstring("IndexOutOfBoundsError") );
    REQUIRE_THROWS_AS(      throwIndexOutOfBoundsError(), std::logic_error);
    REQUIRE_THROWS_AS(      throwIndexOutOfBoundsError(), std::exception);  
    
    static_assert( true == std::is_base_of_v<std::invalid_argument, jau::IllegalArgumentError>);
    static_assert( true == std::is_base_of_v<jau::LogicErrorBase, jau::IllegalArgumentError>);
    static_assert( true == std::is_base_of_v<std::logic_error, jau::IllegalArgumentError>);
    REQUIRE_THROWS_MATCHES( throwIllegalArgumentError(), jau::IllegalArgumentError, Catch::Matchers::ContainsSubstring("IllegalArgumentError") );
    REQUIRE_THROWS_AS(      throwIllegalArgumentError(), std::invalid_argument);
    REQUIRE_THROWS_MATCHES( throwIllegalArgumentError(), jau::LogicErrorBase, Catch::Matchers::ContainsSubstring("IllegalArgumentError") );
    REQUIRE_THROWS_AS(      throwIllegalArgumentError(), std::logic_error);
    REQUIRE_THROWS_AS(      throwIllegalArgumentError(), std::exception);
    
    static_assert( true == std::is_base_of_v<std::domain_error, jau::IllegalStateError>);
    static_assert( true == std::is_base_of_v<jau::LogicErrorBase, jau::IllegalStateError>);
    static_assert( true == std::is_base_of_v<std::logic_error, jau::IllegalStateError>);
    REQUIRE_THROWS_MATCHES( throwIllegalStateError(), jau::IllegalStateError, Catch::Matchers::ContainsSubstring("IllegalStateError") );
    REQUIRE_THROWS_AS(      throwIllegalStateError(), std::domain_error);
    REQUIRE_THROWS_MATCHES( throwIllegalStateError(), jau::LogicErrorBase, Catch::Matchers::ContainsSubstring("IllegalStateError") );
    REQUIRE_THROWS_AS(      throwIllegalStateError(), std::logic_error);
    REQUIRE_THROWS_AS(      throwIllegalStateError(), std::exception);
    
    static_assert( true == std::is_base_of_v<jau::RuntimeExceptionBase, jau::RuntimeSystemExceptionBase>);
    static_assert( true == std::is_base_of_v<jau::ExceptionBase, jau::RuntimeSystemExceptionBase>);
    static_assert( true == std::is_base_of_v<jau::RuntimeSystemExceptionBase, jau::RuntimeSystemException>);
    static_assert( true == std::is_base_of_v<std::system_error, jau::RuntimeSystemException>);
    static_assert( true == std::is_base_of_v<std::runtime_error, jau::RuntimeSystemException>);
    static_assert( true == std::is_base_of_v<std::exception, jau::RuntimeSystemException>);
    REQUIRE_THROWS_MATCHES( throwRuntimeSystemException(), jau::RuntimeSystemExceptionBase, Catch::Matchers::ContainsSubstring("RuntimeSystemException") );
    REQUIRE_THROWS_AS(      throwRuntimeSystemException(), std::system_error);
    REQUIRE_THROWS_MATCHES( throwRuntimeSystemException(), jau::RuntimeExceptionBase, Catch::Matchers::ContainsSubstring("RuntimeSystemException") );
    REQUIRE_THROWS_AS(      throwRuntimeSystemException(), std::runtime_error);
    REQUIRE_THROWS_AS(      throwRuntimeSystemException(), std::exception);
           
    static_assert( true == std::is_base_of_v<std::ios_base::failure, jau::IOError>);
    static_assert( true == std::is_base_of_v<jau::RuntimeSystemExceptionBase, jau::IOError>);
    static_assert( true == std::is_base_of_v<std::system_error, jau::IOError>);
    static_assert( true == std::is_base_of_v<jau::RuntimeExceptionBase, jau::IOError>);
    static_assert( true == std::is_base_of_v<std::runtime_error, jau::IOError>);
    REQUIRE_THROWS_MATCHES( throwIOError(), jau::IOError, Catch::Matchers::ContainsSubstring("IOError") );
    REQUIRE_THROWS_AS( throwIOError(), std::ios_base::failure );
    REQUIRE_THROWS_MATCHES( throwIOError(), jau::RuntimeSystemExceptionBase, Catch::Matchers::ContainsSubstring("IOError") );
    REQUIRE_THROWS_AS( throwIOError(), std::system_error);
    REQUIRE_THROWS_MATCHES( throwIOError(), jau::RuntimeExceptionBase, Catch::Matchers::ContainsSubstring("IOError") );
    REQUIRE_THROWS_AS( throwIOError(), std::runtime_error );
    REQUIRE_THROWS_AS( throwIOError(), std::exception );
    
    REQUIRE_THROWS_MATCHES( throwInternalError(), jau::RuntimeExceptionBase, Catch::Matchers::ContainsSubstring("InternalError") );
    REQUIRE_THROWS_AS( throwInternalError(), std::runtime_error );
    REQUIRE_THROWS_MATCHES( throwInternalError(), jau::InternalError, Catch::Matchers::ContainsSubstring("InternalError") );
    REQUIRE_THROWS_AS( throwInternalError(), std::exception );
    
    REQUIRE_THROWS_MATCHES( throwNotImplementedException(), jau::RuntimeExceptionBase, Catch::Matchers::ContainsSubstring("NotImplementedException") );
    REQUIRE_THROWS_AS( throwNotImplementedException(), std::runtime_error );
    REQUIRE_THROWS_MATCHES( throwNotImplementedException(), jau::NotImplementedException, Catch::Matchers::ContainsSubstring("NotImplementedException") );
    REQUIRE_THROWS_AS( throwNotImplementedException(), std::exception );
    
    REQUIRE_THROWS_MATCHES( throwNullPointerException(), jau::RuntimeExceptionBase, Catch::Matchers::ContainsSubstring("NullPointerException") );
    REQUIRE_THROWS_AS( throwNullPointerException(), std::runtime_error );
    REQUIRE_THROWS_MATCHES( throwNullPointerException(), jau::NullPointerException, Catch::Matchers::ContainsSubstring("NullPointerException") );
    REQUIRE_THROWS_AS( throwNullPointerException(), std::exception );
    
    REQUIRE_THROWS_MATCHES( throwUnsupportedOperationException(), jau::RuntimeExceptionBase, Catch::Matchers::ContainsSubstring("UnsupportedOperationException") );
    REQUIRE_THROWS_AS( throwUnsupportedOperationException(), std::runtime_error );
    REQUIRE_THROWS_MATCHES( throwUnsupportedOperationException(), jau::UnsupportedOperationException, Catch::Matchers::ContainsSubstring("UnsupportedOperationException") );
    REQUIRE_THROWS_AS( throwUnsupportedOperationException(), std::exception );
}

static void throwMathError() {
    throw jau::math::MathError(jau::math::math_error_t::undefined, "test", E_FILE_LINE);
}
static void throwMathInexactError() {
    throw jau::math::MathInexactError("test", E_FILE_LINE);
}
static void throwMathDomainError() {
    throw jau::math::MathDomainError("test", E_FILE_LINE);
}
static void throwMathDivByZeroError() {
    throw jau::math::MathDivByZeroError("test", E_FILE_LINE);
}
static void throwMathOverflowError() {
    throw jau::math::MathOverflowError("test", E_FILE_LINE);
}
static void throwMathUnderflowError() {
    throw jau::math::MathUnderflowError("test", E_FILE_LINE);
}

TEST_CASE( "Exception 10 Math", "[big_int_t][exceptions][error][math]" ) {
    // MathError
    static_assert( true == std::is_base_of_v<jau::math::MathErrorBase, jau::math::MathError>);
    static_assert( true == std::is_base_of_v<std::exception, jau::math::MathError>);
    REQUIRE_THROWS_MATCHES( throwMathError(), jau::math::MathErrorBase, Catch::Matchers::ContainsSubstring("MathError(undefined)") );
    REQUIRE_THROWS_AS(      throwMathError(), std::exception);
    
    static_assert( true == std::is_base_of_v<jau::math::MathRuntimeErrorBase, jau::math::MathInexactError>);
    static_assert( true == std::is_base_of_v<jau::math::MathErrorBase, jau::math::MathInexactError>);
    static_assert( true == std::is_base_of_v<std::runtime_error, jau::math::MathInexactError>);
    static_assert( true == std::is_base_of_v<std::exception, jau::math::MathInexactError>);
    REQUIRE_THROWS_MATCHES( throwMathInexactError(), jau::math::MathInexactError, Catch::Matchers::ContainsSubstring("MathError(inexact)") );
    REQUIRE_THROWS_AS(      throwMathInexactError(), jau::math::MathRuntimeErrorBase);
    REQUIRE_THROWS_AS(      throwMathInexactError(), jau::math::MathErrorBase);
    REQUIRE_THROWS_AS(      throwMathInexactError(), std::runtime_error);
    REQUIRE_THROWS_AS(      throwMathInexactError(), std::exception);
    
    static_assert( true == std::is_base_of_v<jau::math::MathRuntimeErrorBase, jau::math::MathOverflowError>);
    static_assert( true == std::is_base_of_v<jau::math::MathErrorBase, jau::math::MathOverflowError>);
    static_assert( true == std::is_base_of_v<std::overflow_error, jau::math::MathOverflowError>);
    static_assert( true == std::is_base_of_v<std::runtime_error, jau::math::MathOverflowError>);
    static_assert( true == std::is_base_of_v<std::exception, jau::math::MathOverflowError>);
    REQUIRE_THROWS_MATCHES( throwMathOverflowError(), jau::math::MathOverflowError, Catch::Matchers::ContainsSubstring("MathError(overflow)") );
    REQUIRE_THROWS_AS(      throwMathOverflowError(), jau::math::MathRuntimeErrorBase);
    REQUIRE_THROWS_AS(      throwMathOverflowError(), jau::math::MathErrorBase);
    REQUIRE_THROWS_AS(      throwMathOverflowError(), std::overflow_error);
    REQUIRE_THROWS_AS(      throwMathOverflowError(), std::runtime_error);
    REQUIRE_THROWS_AS(      throwMathOverflowError(), std::exception);
    
    static_assert( true == std::is_base_of_v<jau::math::MathRuntimeErrorBase, jau::math::MathUnderflowError>);
    static_assert( true == std::is_base_of_v<jau::math::MathErrorBase, jau::math::MathUnderflowError>);
    static_assert( true == std::is_base_of_v<std::underflow_error, jau::math::MathUnderflowError>);
    static_assert( true == std::is_base_of_v<std::runtime_error, jau::math::MathUnderflowError>);
    static_assert( true == std::is_base_of_v<std::exception, jau::math::MathUnderflowError>);
    REQUIRE_THROWS_MATCHES( throwMathUnderflowError(), jau::math::MathUnderflowError, Catch::Matchers::ContainsSubstring("MathError(underflow)") );
    REQUIRE_THROWS_AS(      throwMathUnderflowError(), jau::math::MathRuntimeErrorBase);
    REQUIRE_THROWS_AS(      throwMathUnderflowError(), jau::math::MathErrorBase);
    REQUIRE_THROWS_AS(      throwMathUnderflowError(), std::underflow_error);
    REQUIRE_THROWS_AS(      throwMathUnderflowError(), std::runtime_error);
    REQUIRE_THROWS_AS(      throwMathUnderflowError(), std::exception);

    static_assert( true == std::is_base_of_v<jau::math::MathErrorBase, jau::math::MathDomainError>);
    static_assert( true == std::is_base_of_v<std::domain_error, jau::math::MathDomainError>);
    static_assert( true == std::is_base_of_v<std::exception, jau::math::MathDomainError>);
    REQUIRE_THROWS_MATCHES( throwMathDomainError(), jau::math::MathDomainError, Catch::Matchers::ContainsSubstring("MathError(invalid)") );
    REQUIRE_THROWS_AS(      throwMathDomainError(), jau::math::MathErrorBase);
    REQUIRE_THROWS_AS(      throwMathDomainError(), std::domain_error);
    REQUIRE_THROWS_AS(      throwMathDomainError(), std::exception);
    
    static_assert( true == std::is_base_of_v<jau::math::MathErrorBase, jau::math::MathDivByZeroError>);
    static_assert( true == std::is_base_of_v<jau::math::MathDomainError, jau::math::MathDivByZeroError>);
    static_assert( true == std::is_base_of_v<std::domain_error, jau::math::MathDivByZeroError>);
    static_assert( true == std::is_base_of_v<std::exception, jau::math::MathDivByZeroError>);
    REQUIRE_THROWS_MATCHES( throwMathDivByZeroError(), jau::math::MathDivByZeroError, Catch::Matchers::ContainsSubstring("MathError(div_by_zero)") );
    REQUIRE_THROWS_AS(      throwMathDivByZeroError(), jau::math::MathDomainError);
    REQUIRE_THROWS_AS(      throwMathDivByZeroError(), jau::math::MathErrorBase);
    REQUIRE_THROWS_AS(      throwMathDivByZeroError(), std::domain_error);
    REQUIRE_THROWS_AS(      throwMathDivByZeroError(), std::exception);
    REQUIRE_THROWS_AS(      throwMathDivByZeroError(), std::exception);    
}

TEST_CASE( "Exception 11 Math", "[big_int_t][exceptions][error][arithmetic][math]" ) {
    {
        jau::mp::BigInt a = 1, b = 0, r;
        REQUIRE_THROWS_MATCHES( r = a / b, jau::math::MathDivByZeroError, Catch::Matchers::ContainsSubstring("div_by_zero") );
        REQUIRE_THROWS_MATCHES( r = a % b, jau::math::MathDivByZeroError, Catch::Matchers::ContainsSubstring("div_by_zero") );
    }
    {
        jau::mp::BigInt a = jau::mp::BigInt::from_s32(-1), b = jau::mp::BigInt::from_s32(-1), r;
        REQUIRE_THROWS_MATCHES( r = a % b, jau::math::MathDomainError, Catch::Matchers::ContainsSubstring("invalid") );
    }
}
