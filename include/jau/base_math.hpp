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

#ifndef JAU_BASE_MATH_HPP_
#define JAU_BASE_MATH_HPP_

#include <climits>
#include <cmath>
#include <concepts>

#include <jau/int_math_ct.hpp>
#include <jau/int_types.hpp>
#include <jau/type_concepts.hpp>

namespace jau {

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

    /**
     * Returns true, if both integer point values differ less than the given range.
     * @tparam T an arithmetic type
     * @param a value to compare
     * @param b value to compare
     * @param range the maximum difference both values may differ
     */
    template<jau::req::arithmetic T>
    bool in_range(const T& a, const T& b, const T& range) {
        return std::abs(a-b) <= range;
    }

    /** Returns true of the given integral is positive, i.e. >= 0. */
    template<typename T>
    requires jau::req::signed_integral<T>
    constexpr bool is_positive(const T a) noexcept {
        return a >= 0;
    }

    template<typename T>
    requires std::floating_point<T>
    constexpr bool is_positive(const T a) noexcept {
        return a >= 0;
    }

    template<typename T>
    requires jau::req::unsigned_integral<T>
    constexpr bool is_positive(const T) noexcept {
        return true;
    }

    /**
     * Returns the value of the sign function (w/o branching ?) in O(1).
     * <pre>
     * -1 for x < 0
     *  0 for x = 0
     *  1 for x > 0
     * </pre>
     * Implementation is type safe.
     *
     * Branching may occur due to relational operator.
     *
     * @tparam T an arithmetic number type
     * @param x the arithmetic number
     * @return function result
     */
    template <jau::req::signed_arithmetic T>
    constexpr int sign(const T x) noexcept
    {
        return (int) ( (T(0) < x) - (x < T(0)) );
    }

    template <jau::req::unsigned_arithmetic T>
    constexpr int sign(const T x) noexcept
    {
        return (int) ( T(0) < x );
    }

    /**
     * Safely inverts the sign of an arithmetic number w/ branching in O(1)
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
     * @tparam T an unsigned arithmetic number type
     * @param x the number
     * @return function result
     */
    template <jau::req::signed_arithmetic T>
    constexpr T invert_sign(const T x) noexcept
    {
        return std::numeric_limits<T>::min() == x ? std::numeric_limits<T>::max() : -x;
    }

    template <jau::req::unsigned_arithmetic T>
    constexpr T invert_sign(const T x) noexcept
    {
        return x;
    }

    /**
     * Returns the absolute value of an arithmetic number (w/ branching) in O(1)
     *
     * - signed uses jau::invert_sign() to have a safe absolute value conversion
     * - unsigned just returns the value
     * - 2-complement branch-less is not used due to lack of INT_MIN -> INT_MAX conversion, [bithacks Integer-Abs](http://www.graphics.stanford.edu/~seander/bithacks.html#IntegerAbs)
     *
     * This implementation uses jau::invert_sign() to have a safe absolute value conversion, if required.
     *
     * @tparam T an arithmetic number type
     * @param x the number
     * @return function result
     */
    template <jau::req::signed_arithmetic T>
    constexpr T abs(const T x) noexcept
    {
        return jau::sign<T>(x) < 0 ? jau::invert_sign<T>( x ) : x;
    }

    template <jau::req::unsigned_arithmetic T>
    constexpr T abs(const T x) noexcept
    {
        return x;
    }

    /**
     * Returns the minimum of two integrals (w/ branching) in O(1)
     *
     * @tparam T an arithmetic number type
     * @param x one number
     * @param x the other number
     */
    template <jau::req::arithmetic T>
    constexpr T min(const T x, const T y) noexcept
    {
        return x < y ? x : y;
    }

    /**
     * Returns the maximum of two integrals (w/ branching) in O(1)
     *
     * @tparam T an arithmetic number type
     * @param x one number
     * @param x the other number
     */
    template <jau::req::arithmetic T>
    constexpr T max(const T x, const T y) noexcept
    {
        return x > y ? x : y;
    }

    /**
     * Returns constrained integral value to lie between given min- and maximum value (w/ branching) in O(1).
     *
     * Implementation returns `min(max(x, min_val), max_val)`, analog to GLSL's clamp()
     *
     * @tparam T an arithmetic number type
     * @param x one number
     * @param min_val the minimum limes, inclusive
     * @param max_val the maximum limes, inclusive
     */
    template <jau::req::arithmetic T>
    constexpr T clamp(const T x, const T min_val, const T max_val) noexcept
    {
        return jau::min<T>(jau::max<T>(x, min_val), max_val);
    }

    /**@}*/

} // namespace jau

#endif /* JAU_BASE_MATH_HPP_ */
