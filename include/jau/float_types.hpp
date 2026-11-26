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

#if __cplusplus > 202002L && __STDCPP_FLOAT32_T__ && __STDCPP_FLOAT64_T__
    #include <stdfloat>
#endif

#include <jau/cpp_lang_util.hpp>
#include <jau/type_info.hpp>
#include <jau/type_concepts.hpp>
#include <jau/int_types.hpp>

namespace jau {

    /** \addtogroup Floats
     *
     *  @{
     */
    #if __cplusplus > 202002L && __STDCPP_FLOAT32_T__ && __STDCPP_FLOAT64_T__
        /** https://en.cppreference.com/w/cpp/types/floating-point */
        typedef std::float32_t float32_t;
        typedef std::float64_t float64_t;
    #else
        static_assert(4 == sizeof(float));
        static_assert(8 == sizeof(double));
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

    typedef jau::uint_bytes_t<sizeof(float)> float_uint_t;
    typedef jau::uint_bytes_t<sizeof(float64_t)> double_uint_t;

    /**
     * IEC559 (IEEE 754) floating-point type handling.
     *
     * @see https://en.wikipedia.org/wiki/IEEE_754
     * @see https://en.wikipedia.org/wiki/Single-precision_floating-point_format
     * @see https://en.wikipedia.org/wiki/Double-precision_floating-point_format
     * @see https://digisim.io/blog/ieee-754-floating-point-from-bits-to-numbers
     */
    namespace iec559 {
        /** Signed bit 31 of IEEE 754 (IEC 559) single float-point bit layout, i.e. `0x80000000`. */
        constexpr uint32_t const binary32_sign_bit = 1_u32 << 31; // 0x80000000_u32;

        /** Exponent mask 8 bits 23-30 w/ bias of 127 of IEEE 754 (IEC 559) single float-point bit layout, i.e. `0x7f800000`. */
        constexpr uint32_t const binary32_exp_mask = 0x7f800000_u32;

        /** Significand mask 23 bits 0-22 of IEEE 754 (IEC 559) single float-point bit layout, i.e. `0x007fffff`. */
        constexpr uint32_t const binary32_significand_mask = 0x007fffff_u32;

        /** Significand divisor pow(2, 23) for IEEE 754 (IEC 559) single float-point. */
        constexpr uint32_t binary32_raw_significand_divisor = 8388608_u32;

        /** Positive infinity bit-value of IEEE 754 (IEC 559) single float-point bit layout, i.e. `0x7f800000`. */
        constexpr uint32_t const binary32_positive_inf_bitval = 0x7f800000_u32;

        /** Negative infinity bit-value of IEEE 754 (IEC 559) single float-point bit layout, i.e. `0xff800000`. */
        constexpr uint32_t const binary32_negative_inf_bitval = 0xff800000_u32;

        /** NaN bit-value of IEEE 754 (IEC 559) single float-point bit layout, i.e. `0x7fc00000`. */
        constexpr uint32_t const binary32_nan_bitval = 0x7fc00000_u32;

        /** Signed bit 63 of IEEE 754 (IEC 559) double float-point bit layout, i.e. `0x8000000000000000`. */
        constexpr uint64_t const binary64_sign_bit = 1_u64 << 63; // 0x8000000000000000_u64;

        /** Exponent mask 11 bits 52-62 w/ bias of 1023 of IEEE 754 (IEC 559) double float-point bit layout, i.e. `0x7ff0000000000000`. */
        constexpr uint64_t const binary64_exp_mask = 0x7ff0000000000000_u64;

        /** Mantissa mask 52 bits 0-51 of IEEE 754 (IEC 559) double float-point bit layout, i.e. `0x000fffffffffffff`. */
        constexpr uint64_t const binary64_significand_mask = 0x000fffffffffffff_u64;

        /** Significand divisor pow(2, 52) for IEEE 754 (IEC 559) double float-point. */
        constexpr uint64_t binary64_raw_significand_divisor = 4503599627370496_u64;

        /** Positive infinity bit-value of IEEE 754 (IEC 559) double float-point bit layout, i.e. `0x7ff0000000000000`. */
        constexpr uint64_t const binary64_positive_inf_bitval = 0x7ff0000000000000_u64;

        /** Negative infinity bit-value of IEEE 754 (IEC 559) double float-point bit layout, i.e. `0xfff0000000000000`. */
        constexpr uint64_t const binary64_negative_inf_bitval = 0xfff0000000000000_u64;

        /** NaN bit-value of IEEE 754 (IEC 559) double float-point bit layout, i.e. `0x7ff8000000000000`. */
        constexpr uint64_t const binary64_nan_bitval = 0x7ff8000000000000_u64;

        /**
         * Returns the unsigned integer representation
         * according to IEEE 754 (IEC 559) binary-float bit layout.
         *
         * Meaningful semantics are only given if `true == std::numeric_limits<T>::is_iec559`.
         *
         * This raw method does not collapse all NaN values.
         *
         * The result is a functional unsigned integer that,
         * i.e. reversible to double via float_value() or double via double_value() depending on type `T`,
         *
         * See specific semantics of IEEE 754 (IEC 559) single floating-point bit layout:
         * - binary32_sign_bit
         * - binary32_exp_mask
         * - binary32_mant_mask
         * - binary32_positive_inf_bitval
         * - binary32_negative_inf_bitval
         * - binary32_nan_bitval
         *
         * See specific semantics of IEEE 754 (IEC 559) double doubleing-point bit layout:
         * - binary64_sign_bit
         * - binary64_exp_mask
         * - binary64_mant_mask
         * - binary64_positive_inf_bitval
         * - binary64_negative_inf_bitval
         * - binary64_nan_bitval
         *
         * @tparam T floating point type, e.g. float or double
         * @tparam matching floating point unsigned integer type, e.g. float_uint_t or double_uint_t
         * @param a float value
         * @return unsigned integer representation of IEEE 754 (IEC 559) floating-point bit layout
         * @see float_value()
         * @see double_value()
         */
        template<std::floating_point T>
        requires (!jau::req::float32_point<T>) && (!jau::req::float64_point<T>)
        jau::uint_bytes_t<sizeof(T)>
        bit_value_raw(const T a) noexcept
        {
            typedef jau::uint_bytes_t<sizeof(T)> T_uint;
            union { T_uint u; T f; } iec559 = { .f = a };
            return iec559.u;
        }

        /**
         * Returns the unsigned integer representation
         * according to IEEE 754 (IEC 559) binary-float 32-bit layout.
         *
         * See bit_value() for details.
         *
         * This raw method does not collapse all NaN values to binary32_nan_bitval.
         */
        template<jau::req::float32_point T>
        constexpr uint32_t bit_value_raw(const T a) noexcept {
            union { uint32_t u; T f; } iec559 = { .f = a };
            return iec559.u;
        }
        /** Extracts the 23-bit raw significand (fraction, mantissa) from the given IEEE 754 (IEC 559) binary-float 32-bit */
        template<jau::req::float32_point T>
        constexpr uint32_t significand_raw(T a) noexcept {
            return bit_value_raw(a) & binary32_significand_mask;
        }
        /** Returns the whole significand (fraction, mantissa) from the raw significand given IEEE 754 (IEC 559) binary-float 32-bit */
        template<jau::req::float32_point T>
        constexpr float significand(T a) noexcept {
            return 1.0f + (float)significand_raw(a) / (float)binary32_raw_significand_divisor;
        }
        /** Extracts the 8-bit exponent from the given IEEE 754 (IEC 559) binary-float 32-bit */
        template<jau::req::float32_point T>
        constexpr uint32_t exponent_raw(T a) noexcept {
            return ( bit_value_raw(a) & binary32_exp_mask ) >> 23U;
        }
        /** Extracts the unbiased 8-bit exponent from the given IEEE 754 (IEC 559) binary-float 32-bit and subtracts 127, i.e. exponent_raw(a)-127 */
        template<jau::req::float32_point T>
        constexpr int32_t exponent(T a) noexcept {
            return int32_t(exponent_raw(a))-127;
        }
        /** Extracts the sign-bit from the given IEEE 754 (IEC 559) binary-float 32-bit and returns -1 or 1. */
        template<jau::req::float32_point T>
        constexpr int sign(T a) noexcept {
            return bit_value_raw(a) & binary32_sign_bit ? -1 : 1;
        }
        template<jau::req::float32_point T>
        constexpr bool is_nan(T a) noexcept {
            return bit_value_raw(a) == binary32_nan_bitval;
        }
        template<jau::req::float32_point T>
        constexpr bool is_positive_inf(T a) noexcept {
            return bit_value_raw(a) == binary32_positive_inf_bitval;
        }
        template<jau::req::float32_point T>
        constexpr bool is_negative_inf(T a) noexcept {
            return bit_value_raw(a) == binary32_negative_inf_bitval;
        }
        template<jau::req::float32_point T>
        constexpr bool is_inf(T a) noexcept {
            const uint32_t b = bit_value_raw(a);
            return b == binary32_positive_inf_bitval || b == binary32_negative_inf_bitval;
        }

        /**
         * Returns the unsigned integer representation
         * according to IEEE 754 (IEC 559) single floating-point bit layout.
         *
         * Meaningful semantics are only given if `true == std::numeric_limits<T>::is_iec559`.
         *
         * All NaN values which are represented by binary32_nan_bitval.
         *
         * The result is a functional unsigned integer that, i.e. reversible to double via float_value().
         *
         * See specific semantics of IEEE 754 (IEC 559) single floating-point bit layout:
         * - binary32_sign_bit
         * - binary32_exp_mask
         * - binary32_mant_mask
         * - binary32_positive_inf_bitval
         * - binary32_negative_inf_bitval
         * - binary32_nan_bitval
         *
         * The result is a functional unsigned integer that, i.e. reversible to float via float_value(),
         * except all NaN values which are represented by binary32_nan_bitval.
         *
         * @param a single float value
         * @return unsigned integer representation of IEEE 754 (IEC 559) single floating-point bit layout
         * @see float_value()
         * @see bit_value_raw()
         */
        template<jau::req::float32_point T>
        constexpr uint32_t bit_value(const T a) noexcept { // NOLINT(bugprone-exception-escape): std::isnan not throwing
            if( std::isnan(a) ) {
                return binary32_nan_bitval;
            }
            return bit_value_raw(a);
        }

        /** Converting IEEE 754 (IEC 559) 32-bit floating-point bit layout to binary-float 32-bit (float or float32_t), see bit_value() */
        template<jau::req::float32_point T>
        constexpr T float32_value(const uint32_t a) noexcept {
            union { uint32_t u; T f; } iec559 = { .u = a };
            return iec559.f;
        }

        /**
         * Returns the unsigned integer representation
         * according to IEEE 754 (IEC 559) binary-float 64-bit floating-point bit layout.
         *
         * See bit_value() for details.
         *
         * This raw method does not collapse all NaN values to binary64_nan_bitval.
         */
        template<jau::req::float64_point T>
        constexpr uint64_t bit_value_raw(const T a) noexcept {
            union { uint64_t u; T f; } iec559 = { .f = a };
            return iec559.u;
        }
        /** Extracts the 52-bit significand (fraction, mantissa) from the given IEEE 754 (IEC 559) binary-binary-float 64-bit */
        template<jau::req::float64_point T>
        constexpr uint64_t significand_raw(T a) noexcept {
            return bit_value_raw(a) & binary64_significand_mask;
        }
        /** Returns the whole significand (fraction, mantissa) from the raw significand given IEEE 754 (IEC 559) binary-float 64-bit */
        template<jau::req::float64_point T>
        constexpr double significand(T a) noexcept {
            return 1.0 + (double)significand_raw(a) / (double)binary64_raw_significand_divisor;
        }
        /** Extracts the 11-bit exponent from the given IEEE 754 (IEC 559) binary-float 64-bit */
        template<jau::req::float64_point T>
        constexpr uint32_t exponent_raw(T a) noexcept {
            return uint32_t(( bit_value_raw(a) & binary64_exp_mask ) >> 52);
        }
        /** Extracts the unbiased 11-bit exponent from the given IEEE 754 (IEC 559) binary-float 64-bit and subtracts 1023, i.e. exponent_raw(a)-1023 */
        template<jau::req::float64_point T>
        constexpr int32_t exponent(T a) noexcept {
            return int32_t(exponent_raw(a))-1023;
        }
        /** Extracts the sign-bit from the given IEEE 754 (IEC 559) binary-float 64-bit and returns -1 or 1. */
        template<jau::req::float64_point T>
        constexpr int sign(T a) noexcept {
            return bit_value_raw(a) & binary64_sign_bit ? -1 : 1;
        }
        template<jau::req::float64_point T>
        constexpr bool is_nan(T a) noexcept {
            return bit_value_raw(a) == binary64_nan_bitval;
        }
        template<jau::req::float64_point T>
        constexpr bool is_positive_inf(T a) noexcept {
            return bit_value_raw(a) == binary64_positive_inf_bitval;
        }
        template<jau::req::float64_point T>
        constexpr bool is_negative_inf(T a) noexcept {
            return bit_value_raw(a) == binary64_negative_inf_bitval;
        }
        template<jau::req::float64_point T>
        constexpr bool is_inf(T a) noexcept {
            const uint64_t b = bit_value_raw(a);
            return b == binary64_positive_inf_bitval || b == binary64_negative_inf_bitval;
        }

        /**
         * Returns the unsigned integer representation
         * according to IEEE 754 (IEC 559) binary-float 64-bit floating-point bit layout.
         *
         * Meaningful semantics are only given if `true == std::numeric_limits<T>::is_iec559`.
         *
         * All NaN values which are represented by binary64_nan_bitval.
         *
         * The result is a functional unsigned integer that, i.e. reversible to binary-float 64-bit via double_value().
         *
         * See specific semantics of IEEE 754 (IEC 559) binary-float 64-bit floating-point bit layout:
         * - binary64_sign_bit
         * - binary64_exp_mask
         * - binary64_mant_mask
         * - binary64_positive_inf_bitval
         * - binary64_negative_inf_bitval
         * - binary64_nan_bitval
         *
         * @param a binary-float 64-bit float value
         * @return unsigned integer representation of IEEE 754 (IEC 559) binary-float 64-bit floating-point bit layout
         * @see double_value()
         * @see bit_value_raw()
         */
        template<jau::req::float64_point T>
        constexpr uint64_t bit_value(const T a) noexcept { // NOLINT(bugprone-exception-escape): std::isnan not throwing
            if( std::isnan(a) ) {
                return binary64_nan_bitval;
            }
            return bit_value_raw(a);
        }
        /** Converting IEEE 754 (IEC 559) 64-bit floating-point bit layout to binary-float 64-bit (double or float64_t), see bit_value() */
        template<jau::req::float64_point T>
        constexpr T float64_value(const uint64_t a) noexcept {
            union { uint64_t u; T f; } iec559 = { .u = a };
            return iec559.f;
        }
    }

    namespace float_literals {
        constexpr float     operator ""_f(long double __v)              noexcept { return (float)__v; }
        constexpr float     operator ""_f(unsigned long long int __v)   noexcept { return (float)__v; }
        constexpr double    operator ""_d(long double __v)              noexcept { return (double)__v; }
        constexpr double    operator ""_d(unsigned long long int __v)   noexcept { return (double)__v; }
        constexpr float32_t operator ""_f32(long double __v)            noexcept { return (float32_t)__v; }
        constexpr float32_t operator ""_f32(unsigned long long int __v) noexcept { return (float32_t)__v; }
        constexpr float64_t operator ""_f64(long double __v)            noexcept { return (float64_t)__v; }
        constexpr float64_t operator ""_f64(unsigned long long int __v) noexcept { return (float64_t)__v; }
    } // float_literals

    namespace float_ctti {
        /// just float
        consteval jau::type_info f() { return jau::make_ctti<float>(); }
        /// just double
        consteval jau::type_info d() { return jau::make_ctti<double>(); }
        /// jau::float_32_t or just float
        consteval jau::type_info f32() { return jau::make_ctti<float32_t>(); }
        /// jau::float_64_t or just double
        consteval jau::type_info f64() { return jau::make_ctti<float64_t>(); }
    };

    /**@}*/

} // namespace jau

#endif /* JAU_FLOAT_TYPES_HPP_ */
