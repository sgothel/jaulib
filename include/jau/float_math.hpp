/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020 Gothel Software e.K.
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

#ifndef JAU_BASIC_FLOAT_MATH_HPP_
#define JAU_BASIC_FLOAT_MATH_HPP_

#include <cmath>
#include <type_traits>
#include <algorithm>

namespace jau {
    /**
    // *************************************************
    // *************************************************
    // *************************************************
     */

    /**
     * Returns true, if both integer point values differ less than the given delta.
     * @tparam T an integral type
     * @param a value to compare
     * @param b value to compare
     * @param delta the maximum difference both values may differ
     */
    template<class T>
    bool in_range(const T& a, const T& b, const T& delta)
    {
        const T diff = std::fabs(a-b);
        return diff <= delta ||
               diff < std::numeric_limits<T>::min(); // subnormal limit
    }

    /**
     * Calculates the smallest floating point value approximation
     * the given type T can represent, the machine epsilon of T.
     * @tparam T a non integer float type
     * @return machine epsilon of T
     */
    template<class T>
    typename std::enable_if<!std::numeric_limits<T>::is_integer, T>::type
        machineEpsilon() {
      const T one(1);
      const T two(2);
      T x = one, res;
      do {
          res = x;
      } while (one + (x /= two) > one);
      return res;
    }

    /**
     * Returns true, if both floating point values are equal
     * in the sense that their potential difference is less or equal <code>epsilon * ulp</code>.
     * @tparam T a non integer float type
     * @param a value to compare
     * @param b value to compare
     * @param ulp desired precision in ULPs (units in the last place), defaults to 1
     * @param epsilon the machine epsilon of type T, defaults to <code>std::numeric_limits<T>::epsilon()</code>
     */
    template<class T>
    typename std::enable_if<!std::numeric_limits<T>::is_integer, bool>::type
        machine_equal(const T& a, const T& b, int ulp=1, const T& epsilon=std::numeric_limits<T>::epsilon())
    {
        const T diff = std::fabs(a-b);
        return diff <= epsilon * ulp ||
               diff < std::numeric_limits<T>::min(); // subnormal limit
    }

    /**
     * Returns true, if both floating point values are equal
     * in the sense that their potential difference is less or equal <code>epsilon * |a+b| * ulp</code>,
     * where <code>|a+b|</code> scales epsilon to the magnitude of used values.
     * @tparam T a non integer float type
     * @param a value to compare
     * @param b value to compare
     * @param ulp desired precision in ULPs (units in the last place), defaults to 1
     * @param epsilon the machine epsilon of type T, defaults to <code>std::numeric_limits<T>::epsilon()</code>
     */
    template<class T>
    typename std::enable_if<!std::numeric_limits<T>::is_integer, bool>::type
        almost_equal(const T& a, const T& b, int ulp=1, const T& epsilon=std::numeric_limits<T>::epsilon())
    {
        const T diff = std::fabs(a-b);
        return diff <= epsilon * std::fabs(a+b) * ulp ||
               diff < std::numeric_limits<T>::min(); // subnormal limit
    }

} // namespace jau

#endif /* JAU_BASIC_FLOAT_MATH_HPP_ */
