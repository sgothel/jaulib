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

#ifndef JAU_INT_MATH_HPP_
#define JAU_INT_MATH_HPP_

#include <cstdint>
#include <cmath>
#include <climits>

#include <jau/base_math.hpp>
#include <jau/int_math_ct.hpp>

namespace jau {

    #define JAU_USE_BUILDIN_OVERFLOW 1

    /** \addtogroup Integer
     *
     *  @{
     */

    /**
     * base_math: arithmetic types, i.e. integral + floating point types
     * int_math: integral types
     * float_math: floating point types
    // *************************************************
    // *************************************************
    // *************************************************
     */

    // Remember: constexpr specifier used in a function or static data member (since C++17) declaration implies inline.

    /** Returns true of the given integer value is zero. */
    template<class T>
    typename std::enable_if<std::is_integral_v<T>, bool>::type
    constexpr is_zero(const T& a) noexcept {
        return 0 == a;
    }

    /**
     * Returns true if both values are equal.
     *
     * @tparam T an integral type
     * @param a value to compare
     * @param b value to compare
     */
    template<class T>
    typename std::enable_if<std::is_integral_v<T>, bool>::type
    constexpr equals(const T& a, const T& b) noexcept {
        return a == b;
    }

    /**
     * Returns true if both values are equal, i.e. their absolute delta <= `allowed_deviation`.
     *
     * @tparam T an integral type
     * @param a value to compare
     * @param b value to compare
     * @param allowed_deviation allowed deviation
     */
    template<class T>
    typename std::enable_if<std::is_integral_v<T>, bool>::type
    constexpr equals(const T& a, const T& b, const T& allowed_deviation) noexcept {
        return std::abs(a - b) <= allowed_deviation;
    }

    /**
     * Round up w/ branching in O(1)
     *
     * @tparam T an unsigned integral number type
     * @tparam U an unsigned integral number type
     * @param n to be aligned number
     * @param align_to alignment boundary, must not be 0
     * @return n rounded up to a multiple of align_to
     */
    template <typename T, typename U,
              std::enable_if_t< std::is_integral_v<T> && std::is_unsigned_v<T> &&
                                std::is_integral_v<U> && std::is_unsigned_v<U>, bool> = true>
    constexpr T round_up(const T n, const U align_to) {
       assert(align_to != 0); // align_to must not be 0

       if(n % align_to) {
          return n + ( align_to - ( n % align_to ) );
       } else {
           return n;
       }
    }

    /**
     * Round down w/ branching in O(1)
     *
     * @tparam T an unsigned integral number type
     * @tparam U an unsigned integral number type
     * @param n to be aligned number
     * @param align_to alignment boundary
     * @return n rounded down to a multiple of align_to
     */
    template <typename T, typename U,
              std::enable_if_t< std::is_integral_v<T> && std::is_unsigned_v<T> &&
                                std::is_integral_v<U> && std::is_unsigned_v<U>, bool> = true>
    constexpr T round_down(T n, U align_to) {
       return align_to == 0 ? n : ( n - ( n % align_to ) );
    }

    /**
     * Power of 2 test (w/o branching ?) in O(1)
     *
     * Source: [bithacks Test PowerOf2](http://www.graphics.stanford.edu/~seander/bithacks.html#DetermineIfPowerOf2)
     *
     * Branching may occur due to relational operator.
     *
     * @tparam T an unsigned integral number type
     * @param x the unsigned integral number
     * @return true if arg is 2^n for some n > 0
     */
    template <typename T,
              std::enable_if_t< std::is_integral_v<T> && std::is_unsigned_v<T>, bool> = true>
    constexpr bool is_power_of_2(const T x) noexcept
    {
       return 0<x && 0 == ( x & static_cast<T>( x - 1 ) );
    }

    /**
     * If the given {@code n} is not is_power_of_2() return next_power_of_2(),
     * otherwise return {@code n} unchanged.
     * <pre>
     * return is_power_of_2(n) ? n : next_power_of_2(n);
     * </pre>
     */
    constexpr uint32_t round_to_power_of_2(const uint32_t n) {
        return is_power_of_2(n) ? n : ct_next_power_of_2(n);
    }

    /**
     * Return the index of the highest set bit w/ branching (loop) in O(n), actually O(n/2).
     *
     * @tparam T an unsigned integral number type
     * @param x value
     */
    template <typename T,
              std::enable_if_t< std::is_integral_v<T> && std::is_unsigned_v<T>, bool> = true>
    inline constexpr nsize_t high_bit(T x)
    {
        nsize_t hb = 0;
        for(nsize_t s = ( CHAR_BIT * sizeof(T) ) >> 1; s > 0; s >>= 1) {
            const nsize_t z = s * ( ( ~jau::ct_is_zero( x >> s ) ) & 1 );
            hb += z;
            x >>= z;
        }
        return hb + x;
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
        return __builtin_add_overflow(a, b, &res);
#else
        // overflow:  a + b > R+ -> a > R+ - b, with b >= 0
        // underflow: a + b < R- -> a < R- - b, with b < 0
        if ( ( b >= 0 && a > std::numeric_limits<T>::max() - b ) ||
             ( b  < 0 && a < std::numeric_limits<T>::min() - b ) )
        {
            return true;
        } else {
            res = a + b;
            return false;
        }
#endif
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
        return __builtin_sub_overflow(a, b, &res);
#else
        // overflow:  a - b > R+ -> a > R+ + b, with b < 0
        // underflow: a - b < R- -> a < R- + b, with b >= 0
        if ( ( b  < 0 && a > std::numeric_limits<T>::max() + b ) ||
             ( b >= 0 && a < std::numeric_limits<T>::min() + b ) )
        {
            return true;
        } else {
            res = a - b;
            return false;
        }
#endif
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
        return __builtin_mul_overflow(a, b, &res);
#else
        // overflow: a * b > R+ -> a > R+ / b
        if ( ( b > 0 && abs(a) > std::numeric_limits<T>::max() / b ) ||
             ( b < 0 && abs(a) > std::numeric_limits<T>::min() / b ) )
        {
            return true;
        } else {
            res = a * b;
            return false;
        }
#endif
    }

    /**
     * Returns the greatest common divisor (GCD) of the two given integer values following Euclid's algorithm from Euclid's Elements ~300 BC,
     * using the absolute positive value of given integers.
     *
     * Returns zero if a and b is zero.
     *
     * Note implementation uses modulo operator `(a/b)*b + a % b = a `,
     * i.e. remainder of the integer division - hence implementation uses `abs(a) % abs(b)`
     * in case the integral T is a signed type (dropped for unsigned).
     *
     * Implementation is similar to std::gcd(), however, it uses a fixed common type T
     * and a while loop instead of compile time evaluation via recursion.
     *
     * @tparam T integral type
     * @tparam
     * @param a integral value a
     * @param b integral value b
     * @return zero if a and b are zero, otherwise the greatest common divisor (GCD) of a and b,
     */
    template <typename T,
              std::enable_if_t<  std::is_integral_v<T> &&
                                !std::is_unsigned_v<T>, bool> = true>
    constexpr T gcd(T a, T b) noexcept
    {
        T a_ = abs(a);
        T b_ = abs(b);
        while( b_ != 0 ) {
            const T t = b_;
            b_ = a_ % b_;
            a_ = t;
        }
        return a_;
    }

    template <typename T,
              std::enable_if_t< std::is_integral_v<T> &&
                                std::is_unsigned_v<T>, bool> = true>
    constexpr T gcd(T a, T b) noexcept
    {
        while( b != 0 ) {
            const T t = b;
            b = a % b;
            a = t;
        }
        return a;
    }

    /**
     * Integer overflow aware calculation of least common multiple (LCM) following Euclid's algorithm from Euclid's Elements ~300 BC.
     * @tparam T integral type
     * @tparam
     * @param result storage for lcm result: zero if a and b are zero, otherwise lcm of a and b
     * @param a integral value a
     * @param b integral value b
     * @return true if overflow, otherwise false for success
     */
    template <typename T,
              std::enable_if_t<  std::is_integral_v<T>, bool> = true>
    constexpr bool lcm_overflow(const T a, const T b, T& result) noexcept
    {
        const T _gcd = gcd<T>( a, b );
        if( 0 < _gcd ) {
            T r;
            if( mul_overflow(a, b, r) ) {
                return true;
            } else {
                result = r / _gcd;
                return false;
            }
        } else {
            result = 0;
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

    /**@}*/

} // namespace jau

#endif /* JAU_INT_MATH_HPP_ */
