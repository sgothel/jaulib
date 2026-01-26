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

#ifndef JAU_FLOAT_MATH_HPP_
#define JAU_FLOAT_MATH_HPP_

#include <cmath>
#include <climits>
#include <concepts>
#include <type_traits>

#include <jau/float_types.hpp>
#include <jau/base_math.hpp>
#include <jau/string_util.hpp>

#include <jau/string_cfmt.hpp>
#include <jau/type_concepts.hpp>

namespace jau {
    /** @defgroup Floats Float types and arithmetic
     *  Float types and arithmetic, see also and meta-group \ref Math
     *  @{
     */

    /// Alias for `π` or half-circle radians (180 degrees), i.e. std::numbers::pi_v<T>
    template<std::floating_point T>
    inline constexpr T PI = std::numbers::pi_v<T>;

    /// Alias for `π/2` or right-angle radians (90 degrees), i.e. std::numbers::pi_v<T>/T(2)
    template<std::floating_point T>
    inline constexpr T PI_2 = std::numbers::pi_v<T>/T(2);

    /// Alias for `π/4` or half right-angle radians (45 degrees), i.e. std::numbers::pi_v<T>/T(4)
    template<std::floating_point T>
    inline constexpr T PI_4 = std::numbers::pi_v<T>/T(4);

    /// Alias for `1/π` or inverse of `π`, i.e. T(1)/std::numbers::pi_v<T> or std::numbers::inv_pi_v<T>
    template<std::floating_point T>
    inline constexpr T inv_PI = std::numbers::inv_pi_v<T>;

    /// Alias for epsilon constant, i.e. std::numeric_limits<T>::epsilon()
    template<std::floating_point T>
    inline constexpr T EPSILON = std::numeric_limits<T>::epsilon();

    /**
     * Calculates the smallest floating point value approximation
     * the given type T can represent, the machine epsilon of T.
     * @tparam T a non integer float type
     * @return machine epsilon of T
     */
    template<std::floating_point T>
    T machineEpsilon() noexcept
    {
      const T one(1);
      const T two(2);
      T x = one, res;
      do {
          res = x;
      } while (one + (x /= two) > one);
      return res;
    }

    /** Returns true if the given value is less than epsilon, w/ epsilon > 0. */
    template<std::floating_point T>
    constexpr bool is_zero(const T& a, const T& epsilon=std::numeric_limits<T>::epsilon()) noexcept {
        return std::abs(a) < epsilon;
    }

    /** Returns true if all given values a and b are less than epsilon, w/ epsilon > 0. */
    template<std::floating_point T>
    constexpr bool is_zero2f(const T& a, const T& b, const T& epsilon=std::numeric_limits<T>::epsilon()) noexcept {
        return std::abs(a) < epsilon && std::abs(b) < epsilon;
    }

    /** Returns true if all given values a, b and c are less than epsilon, w/ epsilon > 0. */
    template<std::floating_point T>
    constexpr bool is_zero3f(const T& a, const T& b, const T& c, const T& epsilon=std::numeric_limits<T>::epsilon()) noexcept {
        return std::abs(a) < epsilon && std::abs(b) < epsilon && std::abs(c) < epsilon;
    }

    /** Returns true if all given values a, b, c and d are less than epsilon, w/ epsilon > 0. */
    template<std::floating_point T>
    constexpr bool is_zero4f(const T& a, const T& b, const T& c, const T& d, const T& epsilon=std::numeric_limits<T>::epsilon()) noexcept {
        return std::abs(a) < epsilon && std::abs(b) < epsilon && std::abs(c) < epsilon && std::abs(d) < epsilon;
    }

    /**
     * Returns true if the given value is zero,
     * disregarding `epsilon` but considering `NaN`, `-Inf` and `+Inf`.
     */
    template<std::floating_point T>
    constexpr bool is_zero_raw(const T& a) noexcept {
        return ( bit_value(a) & ~float_iec559_sign_bit ) == 0;
    }

    /**
     * Returns `-1`, `0` or `1` if `a` is less, equal or greater than `b`,
     * disregarding epsilon but considering `NaN`, `-Inf` and `+Inf`.
     *
     * Implementation considers following corner cases:
     * - NaN == NaN
     * - +Inf == +Inf
     * - -Inf == -Inf
     * - NaN > 0
     * - +Inf > -Inf
     *
     * @tparam T a non integer float type
     * @param a value to compare
     * @param b value to compare
     */
    template<std::floating_point T>
    constexpr int compare(const T a, const T b) noexcept {
        if( a < b ) {
            return -1; // Neither is NaN, a is smaller
        }
        if( a > b ) {
            return 1; // Neither is NaN, a is larger
        }
        // a == b: we compare the _signed_ int value
        typedef typename jau::uint_bytes_t<sizeof(T)> T_uint;
        typedef typename std::make_signed_t<T_uint> T_int;
        const T_int a_bits = static_cast<T_int>( bit_value(a) );
        const T_int b_bits = static_cast<T_int>( bit_value(b) );
        if( a_bits == b_bits ) {
            return 0;  // Values are equal (Inf, Nan .. )
        } else if( a_bits < b_bits ) {
            return -1; // (-0.0,  0.0) or (!NaN,  NaN)
        } else {
            return 1;  // ( 0.0, -0.0) or ( NaN, !NaN)
        }
    }

    /**
     * Returns `-1`, `0` or `1` if `a` is less, equal or greater than `b`,
     * considering epsilon and `NaN`, `-Inf` and `+Inf`.
     *
     * `epsilon` must be > 0.
     *
     * Implementation considers following corner cases:
     * - NaN == NaN
     * - +Inf == +Inf
     * - -Inf == -Inf
     * - NaN > 0
     * - +Inf > -Inf
     *
     * @tparam T a non integer float type
     * @param a value to compare
     * @param b value to compare
     * @param epsilon defaults to std::numeric_limits<T>::epsilon(), must be > 0
     */
    template<std::floating_point T>
    constexpr int compare(const T a, const T b, const T epsilon) noexcept {
        if( std::abs(a - b) < epsilon ) {
            return 0;
        } else {
            return compare(a, b);
        }
    }

    /**
     * Returns true if both values are equal
     * disregarding epsilon but considering `NaN`, `-Inf` and `+Inf`.
     *
     * Implementation considers following corner cases:
     * - NaN == NaN
     * - +Inf == +Inf
     * - -Inf == -Inf
     *
     * @tparam T a non integer float type
     * @param a value to compare
     * @param b value to compare
     */
    template<std::floating_point T>
    constexpr bool equals_raw(const T& a, const T& b) noexcept {
        // Values are equal (Inf, Nan .. )
        return bit_value(a) == bit_value(b);
    }

    /**
     * Returns true if both values are equal, i.e. their absolute delta < `epsilon`,
     * considering epsilon and `NaN`, `-Inf` and `+Inf`.
     *
     * `epsilon` must be > 0.
     *
     * Implementation considers following corner cases:
     * - NaN == NaN
     * - +Inf == +Inf
     * - -Inf == -Inf
     *
     * @tparam T a non integer float type
     * @param a value to compare
     * @param b value to compare
     * @param epsilon defaults to std::numeric_limits<T>::epsilon(), must be > 0
     */
    template<std::floating_point T>
    constexpr bool equals(const T& a, const T& b, const T& epsilon=std::numeric_limits<T>::epsilon()) noexcept {
        if( std::abs(a - b) < epsilon ) {
            return true;
        } else {
            // Values are equal (Inf, Nan .. )
            return bit_value(a) == bit_value(b);
        }
    }

    /**
     * Returns true if both values are equal, i.e. their absolute delta < `epsilon`,
     * considering epsilon but disregarding `NaN`, `-Inf` and `+Inf`.
     *
     * @tparam T a non integer float type
     * @param a value to compare
     * @param b value to compare
     */
    template<std::floating_point T>
    constexpr bool equals2(const T& a, const T& b, const T& epsilon=std::numeric_limits<T>::epsilon()) noexcept {
        return std::abs(a - b) < epsilon;
    }

    /**
     * Returns true, if both floating point values are equal
     * in the sense that their potential difference is less or equal <code>epsilon * ulp</code>.
     *
     * `epsilon` must be > 0.
     *
     * Implementation considers following corner cases:
     * - NaN == NaN
     * - +Inf == +Inf
     * - -Inf == -Inf
     *
     * @tparam T a non integer float type
     * @param a value to compare
     * @param b value to compare
     * @param ulp desired precision in ULPs (units in the last place)
     * @param epsilon the machine epsilon of type T, defaults to <code>std::numeric_limits<T>::epsilon()</code>
     */
    template<std::floating_point T>
    constexpr bool equals(const T& a, const T& b, int ulp, const T& epsilon=std::numeric_limits<T>::epsilon()) noexcept {
        return equals(a, b, epsilon * ulp);
    }

    /**
     * Returns true, if both floating point values are equal
     * in the sense that their potential difference is less or equal <code>epsilon * |a+b| * ulp</code>,
     * where <code>|a+b|</code> scales epsilon to the magnitude of used values.
     *
     * `epsilon` must be > 0.
     *
     * Implementation considers following corner cases:
     * - NaN == NaN
     * - +Inf == +Inf
     * - -Inf == -Inf
     *
     * @tparam T a non integer float type
     * @param a value to compare
     * @param b value to compare
     * @param ulp desired precision in ULPs (units in the last place), defaults to 1
     * @param epsilon the machine epsilon of type T, defaults to <code>std::numeric_limits<T>::epsilon()</code>
     */
    template<std::floating_point T>
    constexpr bool almost_equal(const T& a, const T& b, int ulp=1, const T& epsilon=std::numeric_limits<T>::epsilon()) noexcept
    {
        const T diff = std::fabs(a-b);
        if( ( diff <= epsilon * std::fabs(a+b) * ulp ) ||
            ( diff < std::numeric_limits<T>::min() ) ) { // subnormal limit
            return true;
        } else {
            // Values are equal (Inf, Nan .. )
            return bit_value(a) == bit_value(b);
        }
    }

    /** Returns the rounded value cast to signed int. */
    template<std::floating_point T>
    constexpr typename jau::sint_bytes_t<sizeof(T)> round_to_int(const T v) noexcept {
        return static_cast<typename jau::sint_bytes_t<sizeof(T)>>( std::round(v) );
    }

    /** Returns the rounded value cast to unsigned int. */
    template<std::floating_point T>
    constexpr typename jau::uint_bytes_t<sizeof(T)> round_to_uint(const T v) noexcept {
        return static_cast<typename jau::uint_bytes_t<sizeof(T)>>( std::round(v) );
    }

    /** Converts arc-degree to radians */
    template<std::floating_point T>
    constexpr T adeg_to_rad(const T arc_degree) noexcept {
        return arc_degree * PI<T> / T(180.0);
    }

    /** Converts radians to arc-degree */
    template<std::floating_point T>
    constexpr T rad_to_adeg(const T rad) noexcept {
        return rad * T(180.0) * inv_PI<T>;
    }

    /**
     * Appends a row of floating points to the given string `sb`
     * @param sb string buffer to appends to
     * @param f format string for each float element, e.g. "%10.5f"
     * @param a the float data of size rows x columns
     * @param rows float data `a` size row factor
     * @param columns float data `a` size column factor
     * @param rowMajorOrder if true floats are laid out in row-major-order, otherwise column-major-order (OpenGL)
     * @param row selected row of float data `a`
     * @return given string buffer `sb` for chaining
     */
    template<std::floating_point T>
    std::string& row_to_string(std::string& sb, const std::string_view f,
                               const T a[],
                               const jau::nsize_t rows, const jau::nsize_t columns,
                               const bool rowMajorOrder, const jau::nsize_t row) noexcept {
      if(rowMajorOrder) {
          for(jau::nsize_t c=0; c<columns; ++c) {
              sb.append( jau::format_string(f, a[ row*columns + c ] ) );
              sb.append(", ");
          }
      } else {
          for(jau::nsize_t c=0; c<columns; ++c) {
              sb.append( jau::format_string(f, a[ row + c*rows ] ) );
              sb.append(", ");
          }
      }
      return sb;
    }

    /**
     * Appends a matrix of floating points to the given string `sb`
     * @param sb string buffer to appends to
     * @param rowPrefix prefix for each row
     * @param f format string for each float element, e.g. "%10.5f"
     * @param a the float data of size rows x columns
     * @param rows float data `a` size row factor
     * @param columns float data `a` size column factor
     * @param rowMajorOrder if true floats are laid out in row-major-order, otherwise column-major-order (OpenGL)
     * @return given string buffer `sb` for chaining
     */
    template<std::floating_point T>
    std::string& mat_to_string(std::string& sb, const std::string& rowPrefix, const std::string_view f,
                               const T a[], const jau::nsize_t rows, const jau::nsize_t columns,
                               const bool rowMajorOrder) noexcept {
        sb.append(rowPrefix).append("{\n");
        for(jau::nsize_t i=0; i<rows; ++i) {
            sb.append(rowPrefix).append("  ");
            row_to_string(sb, f, a, rows, columns, rowMajorOrder, i);
            sb.append("\n");
        }
        sb.append(rowPrefix).append("}").append("\n");
        return sb;
    }

    /**@}*/

} // namespace jau

#endif /* JAU_FLOAT_MATH_HPP_ */
