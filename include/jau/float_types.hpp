/*
 * Author: Svenson Han Gothel <shg@jausoft.com>
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

#ifndef JAU_FLOAT_TYPES_HPP_
#define JAU_FLOAT_TYPES_HPP_

#include <cmath>
#include <jau/int_types.hpp>
#if __cplusplus > 202002L
    #include <stdfloat>
#endif

#include <jau/cpp_lang_util.hpp>
#include <jau/type_info.hpp>

namespace jau {

    /** \addtogroup Floats
     *
     *  @{
     */

    #if __cplusplus > 202002L
        /** https://en.cppreference.com/w/cpp/types/floating-point */
        typedef std::float32_t float32_t;
        typedef std::float64_t float64_t;
    #else
        static_assert(32 == sizeof(float)<<3);
        static_assert(64 == sizeof(double)<<3);
        typedef float float32_t;
        typedef double float64_t;
    #endif

    /**
     * base_math: arithmetic types, i.e. integral + floating point types
     * int_math: integral types
     * float_math: floating point types
    // *************************************************
    // *************************************************
    // *************************************************
     */

    using namespace jau::int_literals;

    typedef typename jau::uint_bytes_t<sizeof(float)> float_uint_t;
    typedef typename jau::uint_bytes_t<sizeof(float64_t)> double_uint_t;

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
    template<std::floating_point T>
    typename jau::uint_bytes_t<sizeof(T)>
    bit_value_raw(const T a) noexcept
    {
        typedef typename jau::uint_bytes_t<sizeof(T)> T_uint;
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
    /** Extracts the 23-bit significand (fraction, mantissa) from the given IEEE 754 (IEC 559) float32_t */
    constexpr uint32_t significand_raw(float32_t a) noexcept {
        return bit_value_raw(a) & (((uint32_t)1 << 24) - 1);
    }
    /** Extracts the 8-bit exponent from the given IEEE 754 (IEC 559) float32_t */
    constexpr uint32_t exponent_raw(float32_t a) noexcept {
        constexpr const uint32_t m = uint32_t(0b11111111) << 23U;
        return ( bit_value_raw(a) & m ) >> 23U;
    }
    /** Extracts the unbiased 8-bit exponent from the given IEEE 754 (IEC 559) float64_t and subtracts 127, i.e. exponent_raw(a)-127 */
    constexpr int32_t exponent_unbiased(float32_t a) noexcept {
        return int32_t(exponent_raw(a))-127;
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
     * according to IEEE 754 (IEC 559) float64_t floating-point bit layout.
     *
     * See bit_value() for details.
     *
     * This raw method does not collapse all NaN values to double_iec559_nan_bitval.
     */
    constexpr uint64_t bit_value_raw(const float64_t a) noexcept {
        union { uint64_t u; float64_t f; } iec559 = { .f = a };
        return iec559.u;
    }
    /** Extracts the 52-bit significand (fraction, mantissa) from the given IEEE 754 (IEC 559) float64_t */
    constexpr uint64_t significand_raw(float64_t a) noexcept {
        return bit_value_raw(a) & (((uint64_t)1 << 53) - 1);
    }
    /** Extracts the 11-bit exponent from the given IEEE 754 (IEC 559) float64_t */
    constexpr uint32_t exponent_raw(float64_t a) noexcept {
        constexpr const uint64_t m = uint64_t(0b11111111111) << 52;
        return uint32_t(( bit_value_raw(a) & m ) >> 52);
    }
    /** Extracts the unbiased 11-bit exponent from the given IEEE 754 (IEC 559) float64_t and subtracts 1023, i.e. exponent_raw(a)-1023 */
    constexpr int32_t exponent_unbiased(float64_t a) noexcept {
        return int32_t(exponent_raw(a))-1023;
    }

    /**
     * Returns the unsigned integer representation
     * according to IEEE 754 (IEC 559) float64_t floating-point bit layout.
     *
     * Meaningful semantics are only given if `true == std::numeric_limits<float64_t>::is_iec559`.
     *
     * All NaN values which are represented by double_iec559_nan_bitval.
     *
     * The result is a functional unsigned integer that, i.e. reversible to float64_t via double_value().
     *
     * See specific semantics of IEEE 754 (IEC 559) float64_t floating-point bit layout:
     * - double_iec559_sign_bit
     * - double_iec559_exp_mask
     * - double_iec559_mant_mask
     * - double_iec559_positive_inf_bitval
     * - double_iec559_negative_inf_bitval
     * - double_iec559_nan_bitval
     *
     * @param a float64_t float value
     * @return unsigned integer representation of IEEE 754 (IEC 559) float64_t floating-point bit layout
     * @see double_value()
     * @see bit_value_raw()
     */
    constexpr uint64_t bit_value(const float64_t a) noexcept {
        if( std::isnan(a) ) {
            return double_iec559_nan_bitval;
        }
        return bit_value_raw(a);
    }
    /** Converting IEEE 754 (IEC 559) float64_t floating-point bit layout to float64_t, see bit_value() */
    constexpr float64_t double_value(const uint64_t a) noexcept {
        union { uint64_t u; float64_t f; } iec559 = { .u = a };
        return iec559.f;
    }

    namespace float_literals {
        constexpr float32_t operator ""_f32(long double __v)            { return (float32_t)__v; }
        constexpr float32_t operator ""_f32(unsigned long long int __v) { return (float32_t)__v; }
        constexpr float64_t operator ""_f64(long double __v)            { return (float64_t)__v; }
        constexpr float64_t operator ""_f64(unsigned long long int __v) { return (float64_t)__v; }
    } // float_literals

    class float_ctti {
      public:
        /// jau::float_32_t or just float
        static const jau::type_info& f32() { return jau::static_ctti<float32_t>(); }
        /// jau::float_64_t or just float64_t
        static const jau::type_info& f64() { return jau::static_ctti<float64_t>(); }
    };

    /**@}*/

} // namespace jau

#endif /* JAU_FLOAT_TYPES_HPP_ */
