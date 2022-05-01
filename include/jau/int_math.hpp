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

#ifndef JAU_BASIC_INT_MATH_HPP_
#define JAU_BASIC_INT_MATH_HPP_

#include <cstdint>
#include <cmath>

#include <jau/int_types.hpp>

namespace jau {

    #define JAU_USE_BUILDIN_OVERFLOW 1

    /**
    // *************************************************
    // *************************************************
    // *************************************************
     */

    // Remember: constexpr specifier used in a function or static data member (since C++17) declaration implies inline.

    /**
     * Returns the value of the sign function.
     * <pre>
     * -1 for x < 0
     *  0 for x = 0
     *  1 for x > 0
     * </pre>
     * Implementation is type safe.
     * @tparam T an arithmetic number type
     * @param x the arithmetic number
     * @return function result
     */
    template <typename T,
              std::enable_if_t< std::is_arithmetic_v<T>, bool> = true>
    constexpr snsize_t sign(const T x) noexcept
    {
        return (T(0) < x) - (x < T(0));
    }

    /**
     * Safely inverts the sign of an arithmetic number.
     *
     * Implementation takes special care to have T_MIN, i.e. std::numeric_limits<T>::min(),
     * converted to T_MAX, i.e. std::numeric_limits<T>::max().<br>
     * This is necessary since <code>T_MAX < | -T_MIN |</code> and the result would
     * not fit in the return type T otherwise.
     *
     * Hence for the extreme minimum case:
     * <pre>
     * jau::invert_sign<int32_t>(INT32_MIN) = | INT32_MIN | - 1 = INT32_MAX
     * </pre>
     *
     * Otherwise with x < 0:
     * <pre>
     * jau::invert_sign<int32_t>(x) = | x | = -x
     * </pre>
     * and x >= 0:
     * <pre>
     * jau::invert_sign<int32_t>(x) = -x
     * </pre>
     *
     * @tparam T an arithmetic number type
     * @param x the arithmetic number
     * @return function result
     */
    template <typename T,
              std::enable_if_t< std::is_arithmetic_v<T> &&
                               !std::is_unsigned_v<T>, bool> = true>
    constexpr T invert_sign(const T x) noexcept
    {
        return std::numeric_limits<T>::min() == x ? std::numeric_limits<T>::max() : -x;
    }

    template <typename T,
              std::enable_if_t< std::is_arithmetic_v<T> &&
                                std::is_unsigned_v<T>, bool> = true>
    constexpr T invert_sign(const T x) noexcept
    {
        return x;
    }

    /**
     * Returns the absolute value of an arithmetic number
     *
     * Implementation uses jau::invert_sign() to have a safe absolute value conversion, if required.
     *
     * @tparam T an arithmetic number type
     * @param x the arithmetic number
     * @return function result
     */
    template <typename T,
              std::enable_if_t< std::is_arithmetic_v<T> &&
                               !std::is_unsigned_v<T>, bool> = true>
    constexpr T abs(const T x) noexcept
    {
        return sign(x) < 0 ? invert_sign<T>( x ) : x;
    }

    template <typename T,
              std::enable_if_t< std::is_arithmetic_v<T> &&
                                std::is_unsigned_v<T>, bool> = true>
    constexpr T abs(const T x) noexcept
    {
        return x;
    }

    /**
     * Integer overflow aware addition returning true if overflow occurred,
     * otherwise false having the result stored in res.
     *
     * Implementation uses [Integer Overflow Builtins](https://gcc.gnu.org/onlinedocs/gcc/Integer-Overflow-Builtins.html)
     * if available, otherwise its own implementation.
     *
     * @tparam T an integral integer type
     * @tparam
     * @param a operand a
     * @param b operand b
     * @param res storage for result
     * @return true if overflow, otherwise false
     */
    template <typename T,
              std::enable_if_t< std::is_integral_v<T>, bool> = true>
    constexpr bool add_overflow(const T a, const T b, T& res) noexcept
    {
#if JAU_USE_BUILDIN_OVERFLOW && ( defined(__GNUC__) || defined(__clang__) )
        if ( __builtin_add_overflow(a, b, &res) )
#else
        // overflow:  a + b > R+ -> a > R+ - b, with b >= 0
        // underflow: a + b < R- -> a < R- - b, with b < 0
        if ( ( b >= 0 && a > std::numeric_limits<T>::max() - b ) ||
             ( b  < 0 && a < std::numeric_limits<T>::min() - b ) )
#endif
        {
            return true;
        } else {
            res = a * b;
            return false;
        }
    }

    /**
     * Integer overflow aware subtraction returning true if overflow occurred,
     * otherwise false having the result stored in res.
     *
     * Implementation uses [Integer Overflow Builtins](https://gcc.gnu.org/onlinedocs/gcc/Integer-Overflow-Builtins.html)
     * if available, otherwise its own implementation.
     *
     * @tparam T an integral integer type
     * @tparam
     * @param a operand a
     * @param b operand b
     * @param res storage for result
     * @return true if overflow, otherwise false
     */
    template <typename T,
              std::enable_if_t< std::is_integral_v<T>, bool> = true>
    constexpr bool sub_overflow(const T a, const T b, T& res) noexcept
    {
#if JAU_USE_BUILDIN_OVERFLOW && ( defined(__GNUC__) || defined(__clang__) )
        if ( __builtin_sub_overflow(a, b, &res) )
#else
        // overflow:  a - b > R+ -> a > R+ + b, with b < 0
        // underflow: a - b < R- -> a < R- + b, with b >= 0
        if ( ( b  < 0 && a > std::numeric_limits<T>::max() + b ) ||
             ( b >= 0 && a < std::numeric_limits<T>::min() + b ) )
#endif
        {
            return true;
        } else {
            res = a * b;
            return false;
        }
    }

    /**
     * Integer overflow aware multiplication returning true if overflow occurred,
     * otherwise false having the result stored in res.
     *
     * Implementation uses [Integer Overflow Builtins](https://gcc.gnu.org/onlinedocs/gcc/Integer-Overflow-Builtins.html)
     * if available, otherwise its own implementation.
     *
     * @tparam T an integral integer type
     * @tparam
     * @param a operand a
     * @param b operand b
     * @param res storage for result
     * @return true if overflow, otherwise false
     */
    template <typename T,
              std::enable_if_t< std::is_integral_v<T>, bool> = true>
    constexpr bool mul_overflow(const T a, const T b, T& res) noexcept
    {
#if JAU_USE_BUILDIN_OVERFLOW && ( defined(__GNUC__) || defined(__clang__) )
        if ( __builtin_mul_overflow(a, b, &res) )
#else
        // overflow: a * b > R+ -> a > R+ / b
        if ( ( b > 0 && abs(a) > std::numeric_limits<T>::max() / b ) ||
             ( b < 0 && abs(a) > std::numeric_limits<T>::min() / b ) )
#endif
        {
            return true;
        } else {
            res = a * b;
            return false;
        }
    }

    /**
     * Returns the number of decimal digits of the given integral value number using std::log10<T>().<br>
     * If sign_is_digit == true (default), treats a potential negative sign as a digit.
     * <pre>
     * x < 0: 1 + (int) ( log10( -x ) ) + ( sign_is_digit ? 1 : 0 )
     * x = 0: 1
     * x > 0: 1 + (int) ( log10(  x ) )
     * </pre>
     * Implementation uses jau::invert_sign() to have a safe absolute value conversion, if required.
     * <p>
     * Convenience method, reusing precomputed sign of value to avoid redundant computations.
     * </p>
     * @tparam T an integral integer type
     * @param x the integral integer
     * @param x_sign the pre-determined sign of the given value x
     * @param sign_is_digit if true and value is negative, adds one to result for sign. Defaults to true.
     * @return digit count
     */
    template <typename T,
              std::enable_if_t<  std::is_integral_v<T>, bool> = true>
    constexpr nsize_t digits10(const T x, const snsize_t x_sign, const bool sign_is_digit=true) noexcept
    {
        if( x_sign == 0 ) {
            return 1;
        }
        if( x_sign < 0 ) {
            return 1 + static_cast<nsize_t>( std::log10<T>( invert_sign<T>( x ) ) ) + ( sign_is_digit ? 1 : 0 );
        } else {
            return 1 + static_cast<nsize_t>( std::log10<T>(                 x   ) );
        }
    }

    /**
     * Returns the number of decimal digits of the given integral value number using std::log10<T>().
     * If sign_is_digit == true (default), treats a potential negative sign as a digit.
     * <pre>
     * x < 0: 1 + (int) ( log10( -x ) ) + ( sign_is_digit ? 1 : 0 )
     * x = 0: 1
     * x > 0: 1 + (int) ( log10(  x ) )
     * </pre>
     * Implementation uses jau::invert_sign() to have a safe absolute value conversion, if required.
     * @tparam T an integral integer type
     * @param x the integral integer
     * @param sign_is_digit if true and value is negative, adds one to result for sign. Defaults to true.
     * @return digit count
     */
    template <typename T,
              std::enable_if_t<  std::is_integral_v<T>, bool> = true>
    constexpr nsize_t digits10(const T x, const bool sign_is_digit=true) noexcept
    {
        return digits10<T>(x, jau::sign<T>(x), sign_is_digit);
    }

} // namespace jau

#endif /* JAU_BASIC_INT_MATH_HPP_ */
