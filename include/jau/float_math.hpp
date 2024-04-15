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
#include <type_traits>
#include <algorithm>

#include <jau/base_math.hpp>
#include <jau/string_util.hpp>

namespace jau {
    /** @defgroup Floats Float types and arithmetic
     *  Float types and arithmetic, see also and meta-group \ref Math
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

    using namespace jau::int_literals;

    typedef typename jau::uint_bytes<sizeof(float)>::type float_uint_t;
    typedef typename jau::uint_bytes<sizeof(double)>::type double_uint_t;

    /** Signed bit 31 of IEEE 754 (IEC 559) single float-point bit layout, i.e. `0x80000000`. */
    constexpr uint32_t const float_iec559_sign_bit = 1_u32 << 31; // 0x80000000_u32;

    /** Exponent mask bits 23-30 of IEEE 754 (IEC 559) single float-point bit layout, i.e. `0x7f800000`. */
    constexpr uint32_t const float_iec559_exp_mask = 0x7f800000_u32;

    /** Mantissa mask bits 0-22 of IEEE 754 (IEC 559) single float-point bit layout, i.e. `0x007fffff`. */
    constexpr uint32_t const float_iec559_mant_mask = 0x007fffff_u32;

    /** Positive infinity bit-value of IEEE 754 (IEC 559) single float-point bit layout, i.e. `0x7f800000`. */
    constexpr uint32_t const float_iec559_positive_inf_bitval = 0x7f800000_u32;

    /** Negative infinity bit-value of IEEE 754 (IEC 559) single float-point bit layout, i.e. `0xff800000`. */
    constexpr uint32_t const float_iec559_negative_inf_bitval = 0xff800000_u32;

    /** NaN bit-value of IEEE 754 (IEC 559) single float-point bit layout, i.e. `0x7fc00000`. */
    constexpr uint32_t const float_iec559_nan_bitval = 0x7fc00000_u32;

    /** Signed bit 63 of IEEE 754 (IEC 559) double double-point bit layout, i.e. `0x8000000000000000`. */
    constexpr uint64_t const double_iec559_sign_bit = 1_u64 << 63; // 0x8000000000000000_u64;

    /** Exponent mask bits 52-62 of IEEE 754 (IEC 559) double double-point bit layout, i.e. `0x7ff0000000000000`. */
    constexpr uint64_t const double_iec559_exp_mask = 0x7ff0000000000000_u64;

    /** Mantissa mask bits 0-51 of IEEE 754 (IEC 559) double double-point bit layout, i.e. `0x000fffffffffffff`. */
    constexpr uint64_t const double_iec559_mant_mask = 0x000fffffffffffff_u64;

    /** Positive infinity bit-value of IEEE 754 (IEC 559) double double-point bit layout, i.e. `0x7ff0000000000000`. */
    constexpr uint64_t const double_iec559_positive_inf_bitval = 0x7ff0000000000000_u64;

    /** Negative infinity bit-value of IEEE 754 (IEC 559) double double-point bit layout, i.e. `0xfff0000000000000`. */
    constexpr uint64_t const double_iec559_negative_inf_bitval = 0xfff0000000000000_u64;

    /** NaN bit-value of IEEE 754 (IEC 559) double double-point bit layout, i.e. `0x7ff8000000000000`. */
    constexpr uint64_t const double_iec559_nan_bitval = 0x7ff8000000000000_u64;

    /**
     * Returns the unsigned integer representation
     * according to IEEE 754 (IEC 559) floating-point bit layout.
     *
     * Meaningful semantics are only given if `true == std::numeric_limits<T>::is_iec559`.
     *
     * This raw method does not collapse all NaN values.
     *
     * The result is a functional unsigned integer that,
     * i.e. reversible to double via float_value() or double via double_value() depending on type `T`,
     *
     * See specific semantics of IEEE 754 (IEC 559) single floating-point bit layout:
     * - float_iec559_sign_bit
     * - float_iec559_exp_mask
     * - float_iec559_mant_mask
     * - float_iec559_positive_inf_bitval
     * - float_iec559_negative_inf_bitval
     * - float_iec559_nan_bitval
     *
     * See specific semantics of IEEE 754 (IEC 559) double doubleing-point bit layout:
     * - double_iec559_sign_bit
     * - double_iec559_exp_mask
     * - double_iec559_mant_mask
     * - double_iec559_positive_inf_bitval
     * - double_iec559_negative_inf_bitval
     * - double_iec559_nan_bitval
     *
     * @tparam T floating point type, e.g. float or double
     * @tparam matching floating point unsigned integer type, e.g. float_uint_t or double_uint_t
     * @param a float value
     * @return unsigned integer representation of IEEE 754 (IEC 559) floating-point bit layout
     * @see float_value()
     * @see double_value()
     */
    template<class T,
             std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
    typename jau::uint_bytes<sizeof(T)>::type
    bit_value_raw(const T a) noexcept
    {
        typedef typename jau::uint_bytes<sizeof(T)>::type T_uint;
        union { T_uint u; T f; } iec559 = { .f = a };
        return iec559.u;
    }

    /**
     * Returns the unsigned integer representation
     * according to IEEE 754 (IEC 559) single floating-point bit layout.
     *
     * See bit_value() for details.
     *
     * This raw method does not collapse all NaN values to float_iec559_nan_bitval.
     */
    constexpr uint32_t bit_value_raw(const float a) noexcept {
        union { uint32_t u; float f; } iec559 = { .f = a };
        return iec559.u;
    }
    /**
     * Returns the unsigned integer representation
     * according to IEEE 754 (IEC 559) single floating-point bit layout.
     *
     * Meaningful semantics are only given if `true == std::numeric_limits<float>::is_iec559`.
     *
     * All NaN values which are represented by float_iec559_nan_bitval.
     *
     * The result is a functional unsigned integer that, i.e. reversible to double via float_value().
     *
     * See specific semantics of IEEE 754 (IEC 559) single floating-point bit layout:
     * - float_iec559_sign_bit
     * - float_iec559_exp_mask
     * - float_iec559_mant_mask
     * - float_iec559_positive_inf_bitval
     * - float_iec559_negative_inf_bitval
     * - float_iec559_nan_bitval
     *
     * The result is a functional unsigned integer that, i.e. reversible to float via float_value(),
     * except all NaN values which are represented by float_iec559_nan_bitval.
     *
     * @param a single float value
     * @return unsigned integer representation of IEEE 754 (IEC 559) single floating-point bit layout
     * @see float_value()
     * @see bit_value_raw()
     */
    constexpr uint32_t bit_value(const float a) noexcept {
        if( std::isnan(a) ) {
            return float_iec559_nan_bitval;
        }
        return bit_value_raw(a);
    }

    /** Converting IEEE 754 (IEC 559) single floating-point bit layout to float, see bit_value() */
    constexpr float float_value(const uint32_t a) noexcept {
        union { uint32_t u; float f; } iec559 = { .u = a };
        return iec559.f;
    }
    /**
     * Returns the unsigned integer representation
     * according to IEEE 754 (IEC 559) double floating-point bit layout.
     *
     * See bit_value() for details.
     *
     * This raw method does not collapse all NaN values to double_iec559_nan_bitval.
     */
    constexpr uint64_t bit_value_raw(const double a) noexcept {
        union { uint64_t u; double f; } iec559 = { .f = a };
        return iec559.u;
    }
    /**
     * Returns the unsigned integer representation
     * according to IEEE 754 (IEC 559) double floating-point bit layout.
     *
     * Meaningful semantics are only given if `true == std::numeric_limits<double>::is_iec559`.
     *
     * All NaN values which are represented by double_iec559_nan_bitval.
     *
     * The result is a functional unsigned integer that, i.e. reversible to double via double_value().
     *
     * See specific semantics of IEEE 754 (IEC 559) double floating-point bit layout:
     * - double_iec559_sign_bit
     * - double_iec559_exp_mask
     * - double_iec559_mant_mask
     * - double_iec559_positive_inf_bitval
     * - double_iec559_negative_inf_bitval
     * - double_iec559_nan_bitval
     *
     * @param a double float value
     * @return unsigned integer representation of IEEE 754 (IEC 559) double floating-point bit layout
     * @see double_value()
     * @see bit_value_raw()
     */
    constexpr uint64_t bit_value(const double a) noexcept {
        if( std::isnan(a) ) {
            return double_iec559_nan_bitval;
        }
        return bit_value_raw(a);
    }
    /** Converting IEEE 754 (IEC 559) double floating-point bit layout to double, see bit_value() */
    constexpr double double_value(const uint64_t a) noexcept {
        union { uint64_t u; double f; } iec559 = { .u = a };
        return iec559.f;
    }

    /**
     * Calculates the smallest floating point value approximation
     * the given type T can represent, the machine epsilon of T.
     * @tparam T a non integer float type
     * @return machine epsilon of T
     */
    template<class T>
    typename std::enable_if<std::is_floating_point_v<T>, T>::type
    machineEpsilon() noexcept
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
    template<class T>
    typename std::enable_if<std::is_floating_point_v<T>, bool>::type
    constexpr is_zero(const T& a, const T& epsilon=std::numeric_limits<T>::epsilon()) noexcept {
        return std::abs(a) < epsilon;
    }

    /** Returns true if all given values a and b are less than epsilon, w/ epsilon > 0. */
    template<class T>
    typename std::enable_if<std::is_floating_point_v<T>, bool>::type
    constexpr is_zero2f(const T& a, const T& b, const T& epsilon=std::numeric_limits<T>::epsilon()) noexcept {
        return std::abs(a) < epsilon && std::abs(b) < epsilon;
    }

    /** Returns true if all given values a, b and c are less than epsilon, w/ epsilon > 0. */
    template<class T>
    typename std::enable_if<std::is_floating_point_v<T>, bool>::type
    constexpr is_zero3f(const T& a, const T& b, const T& c, const T& epsilon=std::numeric_limits<T>::epsilon()) noexcept {
        return std::abs(a) < epsilon && std::abs(b) < epsilon && std::abs(c) < epsilon;
    }

    /** Returns true if all given values a, b, c and d are less than epsilon, w/ epsilon > 0. */
    template<class T>
    typename std::enable_if<std::is_floating_point_v<T>, bool>::type
    constexpr is_zero4f(const T& a, const T& b, const T& c, const T& d, const T& epsilon=std::numeric_limits<T>::epsilon()) noexcept {
        return std::abs(a) < epsilon && std::abs(b) < epsilon && std::abs(c) < epsilon && std::abs(d) < epsilon;
    }

    /**
     * Returns true if the given value is zero,
     * disregarding `epsilon` but considering `NaN`, `-Inf` and `+Inf`.
     */
    template<class T>
    typename std::enable_if<std::is_floating_point_v<T>, bool>::type
    constexpr is_zero_raw(const T& a) noexcept {
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
    template<class T,
             std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
    constexpr int compare(const T a, const T b) noexcept {
        if( a < b ) {
            return -1; // Neither is NaN, a is smaller
        }
        if( a > b ) {
            return 1; // Neither is NaN, a is larger
        }
        // a == b: we compare the _signed_ int value
        typedef typename jau::uint_bytes<sizeof(T)>::type T_uint;
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
    template<class T,
             std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
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
    template<class T>
    typename std::enable_if<std::is_floating_point_v<T>, bool>::type
    constexpr equals_raw(const T& a, const T& b) noexcept {
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
    template<class T>
    typename std::enable_if<std::is_floating_point_v<T>, bool>::type
    constexpr equals(const T& a, const T& b, const T& epsilon=std::numeric_limits<T>::epsilon()) noexcept {
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
    template<class T>
    typename std::enable_if<std::is_floating_point_v<T>, bool>::type
    constexpr equals2(const T& a, const T& b, const T& epsilon=std::numeric_limits<T>::epsilon()) noexcept {
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
    template<class T>
    typename std::enable_if<std::is_floating_point_v<T>, bool>::type
    constexpr equals(const T& a, const T& b, int ulp, const T& epsilon=std::numeric_limits<T>::epsilon()) noexcept {
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
    template<class T>
    typename std::enable_if<std::is_floating_point_v<T>, bool>::type
    almost_equal(const T& a, const T& b, int ulp=1, const T& epsilon=std::numeric_limits<T>::epsilon()) noexcept
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

    /** Returns the rounded value cast to int. */
    template<class T,
             std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
    constexpr typename jau::sint_bytes<sizeof(T)>::type round_to_int(const T v) noexcept {
        return static_cast<typename jau::sint_bytes<sizeof(T)>::type>( std::round(v) );
    }

    /** Converts arc-degree to radians */
    template<class T>
    typename std::enable_if<std::is_floating_point_v<T>, bool>::type
    constexpr adeg_to_rad(const T arc_degree) noexcept {
        return arc_degree * (T)M_PI / (T)180.0;
    }

    /** Converts radians to arc-degree */
    template<class T>
    typename std::enable_if<std::is_floating_point_v<T>, bool>::type
    constexpr rad_to_adeg(const T rad) noexcept {
        return rad * (T)180.0 / (T)M_PI;
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
    template<typename T,
        std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
    std::string& row_to_string(std::string& sb, const std::string& f,
                               const T a[],
                               const jau::nsize_t rows, const jau::nsize_t columns,
                               const bool rowMajorOrder, const jau::nsize_t row) noexcept {
      if(rowMajorOrder) {
          for(jau::nsize_t c=0; c<columns; ++c) {
              sb.append( jau::format_string(f.c_str(), a[ row*columns + c ] ) );
              sb.append(", ");
          }
      } else {
          for(jau::nsize_t c=0; c<columns; ++c) {
              sb.append( jau::format_string(f.c_str(), a[ row + c*rows ] ) );
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
    template<typename T,
        std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
    std::string& mat_to_string(std::string& sb, const std::string& rowPrefix, const std::string& f,
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
