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

#ifndef JAU_INT_MATH_CT_HPP_
#define JAU_INT_MATH_CT_HPP_

#include <cstdint>
#include <climits>

#include <jau/int_types.hpp>
#include <jau/cpp_pragma.hpp>

namespace jau {

    /** @defgroup ConstantTime Constant Time (CT) Integral Operations
     * Integral integer operations in constant time (CT), see also and meta-group \ref Math
     *
     *  @{
     */

    /**
    // *************************************************
    // *************************************************
    // *************************************************
     */

    // Remember: constexpr specifier used in a function or static data member (since C++17) declaration implies inline.

    /**
     * Returns the value of the sign function (w/o branching) in O(1) and constant time (CT)
     * <pre>
     * -1 for x < 0
     *  0 for x = 0
     *  1 for x > 0
     * </pre>
     * Implementation is type safe.
     *
     * Branching may occur due to relational operator.
     *
     * @tparam T an integral number type
     * @param x the number
     * @return function result
     */
    template <typename T,
              std::enable_if_t< std::is_integral_v<T>, bool> = true>
    constexpr int ct_sign(const T x) noexcept
    {
        return (x != 0) | -(int)((std::make_unsigned_t<T>)((T)x) >> (sizeof(T) * CHAR_BIT - 1));
        // return (int) ( (T(0) < x) - (x < T(0)) );
    }

    /**
     * Returns the absolute value of an arithmetic number (w/o branching) in O(1) and constant time (CT),
     * while ct_abs(INT_MIN) is undefined behavior (UB) instead of being mapped correctly to INT_MAX like jau::abs() does, see above.
     *
     * This implementation is equivalent to std::abs(), i.e. unsafe
     *
     * - signed integral uses 2-complement branch-less conversion, [bithacks Integer-Abs](http://www.graphics.stanford.edu/~seander/bithacks.html#IntegerAbs)
     * - signed floating-point uses x * sign(x)
     * - unsigned just returns the value
     *
     * This implementation uses 2-complement branch-less conversion, [bithacks Integer-Abs](http://www.graphics.stanford.edu/~seander/bithacks.html#IntegerAbs)
     *
     * Note: On an x86_64 architecture abs() w/ branching is of equal speed or even faster.
     *
     * @tparam T an arithmetic number type
     * @param x the number
     * @return function result
     */
    template <typename T,
              std::enable_if_t< std::is_arithmetic_v<T> &&
                                std::is_integral_v<T> &&
                               !std::is_unsigned_v<T>, bool> = true>
    constexpr T ct_abs(const T x) noexcept
    {
        using unsigned_T = std::make_unsigned_t<T>;
        const T mask = x >> ( sizeof(T) * CHAR_BIT - 1 );
        PRAGMA_DISABLE_WARNING_PUSH
        PRAGMA_DISABLE_WARNING_INT_OVERFLOW
        return static_cast<unsigned_T>( ( x + mask ) ^ mask ); // clang 15: int overflow on UB (constrained in API doc)
        PRAGMA_DISABLE_WARNING_POP
    }
    template <typename T,
              std::enable_if_t< std::is_arithmetic_v<T> &&
                               !std::is_integral_v<T> &&
                               !std::is_unsigned_v<T>, bool> = true>
    constexpr T ct_abs(const T x) noexcept
    {
        return x * jau::ct_sign<T>(x);
    }
    template <typename T,
              std::enable_if_t< std::is_arithmetic_v<T> &&
                                std::is_unsigned_v<T>, bool> = true>
    constexpr T ct_abs(const T x) noexcept
    {
        return x;
    }

    /**
     * Returns the minimum of two integrals for `MIN <= x - y <= MAX` (w/o branching) in O(1) and constant time (CT).
     *
     * Source: [bithacks Test IntegerMinOrMax](http://www.graphics.stanford.edu/~seander/bithacks.html#IntegerMinOrMax)
     *
     * Note: On an x86_64 architecture min() w/ branching is of equal speed or even faster.
     *
     * @tparam T an integral number type
     * @param x one number
     * @param x the other number
     */
    template <typename T,
              std::enable_if_t< std::is_integral_v<T>, bool> = true>
    constexpr T ct_min(const T x, const T y) noexcept
    {
        return y + ( (x - y) & ( (x - y) >> ( sizeof(T) * CHAR_BIT - 1 ) ) );
    }

    /**
     * Returns the maximum of two integrals for `MIN <= x - y <= MAX` (w/o branching) in O(1) and constant time (CT).
     *
     * Source: [bithacks Test IntegerMinOrMax](http://www.graphics.stanford.edu/~seander/bithacks.html#IntegerMinOrMax)
     *
     * Note: On an x86_64 architecture max() w/ branching is of equal speed or even faster.
     *
     * @tparam T an integral number type
     * @param x one number
     * @param x the other number
     */
    template <typename T,
              std::enable_if_t< std::is_integral_v<T>, bool> = true>
    constexpr T ct_max(const T x, const T y) noexcept
    {
        return x - ( (x - y) & ( (x - y) >> ( sizeof(T) * CHAR_BIT - 1 ) ) );
    }

    /**
     * Returns constrained integral value to lie between given min- and maximum value for `MIN <= x - y <= MAX`
     * (w/o branching) in O(1) and constant time (CT).
     *
     * Implementation returns `ct_min(ct_max(x, min_val), max_val)`, analog to GLSL's clamp()
     *
     * Note: On an x86_64 architecture clamp() w/ branching is of equal speed or even faster.
     *
     * @tparam T an integral number type
     * @param x one number
     * @param min_val the minimum limes, inclusive
     * @param max_val the maximum limes, inclusive
     */
    template <typename T,
              std::enable_if_t< std::is_integral_v<T>, bool> = true>
    constexpr T ct_clamp(const T x, const T min_val, const T max_val) noexcept
    {
        return jau::ct_min<T>(jau::ct_max<T>(x, min_val), max_val);
    }

    /**
     * Returns merged `a_if_masked` bits selected by `mask` `1` bits and `b_if_unmasked` bits selected by `mask` `0` bits
     * (w/o branching) in O(1) and constant time (CT).
     *
     * Source: [bithacks MaskedMerge](http://www.graphics.stanford.edu/~seander/bithacks.html#MaskedMerge)
     *
     * @tparam T an unsigned integral number type
     * @param mask 1 where bits from `a_if_masked` should be selected; 0 where from `b_if_unmasked`.
     * @param a_if_masked value to merge in masked bits
     * @param b_if_unmasked value to merge in non-masked bits
     */
    template <typename T,
              std::enable_if_t< std::is_integral_v<T> && std::is_unsigned_v<T>, bool> = true>
    constexpr T ct_masked_merge(T mask, T a_if_masked, T b_if_unmasked) {
        return b_if_unmasked ^ ( mask & ( a_if_masked ^ b_if_unmasked ) );
    }

    /**
     * Returns the next higher power of 2 of given unsigned 32-bit {@code n}
     * (w/o branching) in O(1) and constant time (CT).
     * <p>
     * Source: [bithacks RoundUpPowerOf2](http://www.graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2)
     * </p>
     */
    constexpr uint32_t ct_next_power_of_2(uint32_t n) {
        n--;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        return n + 1;
    }

    /**
     * Returns the number of set bits within given 32bit integer
     * (w/o branching) in O(1) and constant time (CT).
     *
     * Uses a <i>HAKEM 169 Bit Count</i> inspired implementation:
     * <pre>
     *   http://www.inwap.com/pdp10/hbaker/hakmem/hakmem.html
     *   http://home.pipeline.com/~hbaker1/hakmem/hacks.html#item169
     *   http://tekpool.wordpress.com/category/bit-count/
     *   https://github.com/aistrate/HackersDelight/blob/master/Original/HDcode/pop.c.txt
     *   https://github.com/aistrate/HackersDelight/blob/master/Original/HDcode/newCode/popDiff.c.txt
     * </pre>
     */
    constexpr uint32_t ct_bit_count(uint32_t n) noexcept {
        // Note: Original used 'unsigned int',
        // hence we use the unsigned right-shift '>>>'
        /**
         * Original using 'unsigned' right-shift and modulo
         *
        const uint32_t c = n
                         - ( (n >> 1) & 033333333333 )
                         - ( (n >> 2) & 011111111111 );
        return ( ( c + ( c >> 3 ) ) & 030707070707 ) % 63;
         *
         */
        // Hackers Delight, Figure 5-2, pop1 of pop.c.txt (or popDiff.c.txt in git repo)
        n = n - ((n >> 1) & 0x55555555);
        n = (n & 0x33333333) + ((n >> 2) & 0x33333333);
        n = (n + (n >> 4)) & 0x0f0f0f0f;
        n = n + (n >> 8);
        n = n + (n >> 16);
        return n & 0x3f;
    }

    /**
     * Returns ~0 (2-complement) if top bit of arg is set, otherwise 0
     * (w/o branching) in O(1) and constant time (CT).
     *
     * @tparam T an unsigned integral number type
     */
    template <typename T,
              std::enable_if_t< std::is_integral_v<T> && std::is_unsigned_v<T>, bool> = true>
    inline constexpr T ct_expand_top_bit(T x)
    {
       return T(0) - ( x >> ( sizeof(T) * CHAR_BIT - 1 ) );
    }

    /**
     * Returns ~0 (2-complement) if arg is zero, otherwise 0
     * (w/o branching) in O(1) and constant time (CT).
     *
     * @tparam T an unsigned integral number type
     */
    template <typename T,
              std::enable_if_t< std::is_integral_v<T> && std::is_unsigned_v<T>, bool> = true>
    inline constexpr T ct_is_zero(T x)
    {
       return jau::ct_expand_top_bit<T>( ~x & (x - 1) );
    }

    /**@}*/

} // namespace jau

#endif /* JAU_INT_MATH_CT_HPP_ */
