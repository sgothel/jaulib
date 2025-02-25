/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2022 Gothel Software e.K.
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

#ifndef JAU_FRACTION_TYPE_HPP_
#define JAU_FRACTION_TYPE_HPP_

#include <limits>
#include <chrono>

#include <cstdio>

#include <cstdint>
#include <cmath>
#include "jau/cpp_lang_util.hpp"

#include <jau/int_types.hpp>
#include <jau/int_math.hpp>
#include <jau/ordered_atomic.hpp>
#include <jau/cpp_pragma.hpp>
#include <jau/backtrace.hpp>

namespace jau {

    // Remember: constexpr specifier used in a function or static data member (since C++17) declaration implies inline.

    /** @defgroup Fractions Fraction Arithmetic and Time
     *  Fraction type and arithmetic support
     *  inclusive its utilization for time without loss of precision nor range.
     *
     *  Adds support for \ref Concurrency with general time functionality
     *  like sleep_until(), sleep_for(), wait_until() and wait_for(),
     *  completed with getMonotonicTime() and getWallClockTime().
     *
     *  @{
     */

    /**
     * Fraction template type using integral values, evaluated at runtime.
     *
     * All operations reduce its fraction to the lowest terms using
     * the greatest common divisor (gcd()) following Euclid's algorithm from Euclid's Elements ~300 BC,
     * see reduce().
     *
     * fraction provides similar properties like C++11's `std::ratio`,
     * but is evaluated at runtime time without `constexpr` constraints using a common integral template type.
     * std::ratio is evaluated at compile time and must use `constexpr` literal values.
     *
     * fraction provides similar properties like C++11's `std::chrono::duration`,
     * but is flexible with its denominator and always reduce() its fraction to the lowest terms.
     * `std::chrono::duration` uses a fixed `std::ratio` denominator and hence is inflexible.
     *
     * Further, fraction can be converted to std::chrono::duration,
     * matching the selected duration's period, see to_duration_count() and to_duration().
     * However, it is recommended to use fraction_timespec, see below.
     *
     * The following properties are exposed:
     * - Numerator carries sign and hence can be negative and of signed type
     * - Denominator is always positive and is an unsigned type
     * - All operations incl. construction will result in a reduced fraction using the greatest common denominator, see gcd().
     * - No exceptions are thrown, a zero denominator is undefined behavior (UB), implementation will return zero { n=0, d=1 }.
     *
     * See usable fixed typedef's
     * - fraction_i64
     * - fraction_ui64
     *
     * fraction_timespec covers high precision and an almost infinite range of time
     * similar to `struct timespec_t`.
     *
     * Counting nanoseconds in int64_t only lasts until `2262-04-12`,
     * since INT64_MAX is 9'223'372'036'854'775'807 for 9'223'372'036 seconds or 292 years.
     *
     * Hence one may use fraction_i64 for durations up to 292 years
     * and fraction_timespec for almost infinite range of time-points or durations beyond 292 years.
     *
     * Constants are provided in namespace jau::fractions_i64,
     * from fractions_i64::pico to fractions_i64::tera, including fractions_i64::seconds to fractions_i64::years, etc.
     *
     * Literal operators are provided in namespace jau::fractions_i64_literals,
     * e.g. for `3_s`, `100_ns` ... literals.
     *
     * @tparam int_type
     * @tparam
     */
    template<typename Int_type,
             std::enable_if_t< std::is_integral_v<Int_type>, bool> = true>
    class fraction {
        public:
            /** User defined integral integer template type, used for numerator and may be signed. */
            typedef Int_type                        int_type;

            /** unsigned variant of template int_type, used for denominator. */
            typedef std::make_unsigned_t<int_type> uint_type;

            /** Numerator, carries the sign. */
            int_type num;
            /** Denominator, always positive. */
            uint_type denom;
            /** Overflow flag. If set, last arithmetic operation produced an overflow. Must be cleared manually. */
            bool overflow;

        private:
            void set_overflow() noexcept {
                overflow = true;
                print_backtrace(true /* skip_anon_frames */, 6 /* max_frames */, 2 /* skip_frames */);
                num = std::numeric_limits<int_type>::max();
                denom = std::numeric_limits<uint_type>::max();
            }

        public:

            /**
             * Constructs a zero fraction instance { 0, 1 }
             */
            constexpr fraction() noexcept
            : num(0), denom(1), overflow(false) { }


            /**
             * Constructs a fraction instance with smallest numerator and denominator using gcd()
             *
             * Note: sign is always stored in fraction's numerator, i.e. the denominator is always positive.
             *
             * @param n the given numerator
             * @param d the given denominator
             */
            template <typename T,
                      std::enable_if_t<  std::is_same_v<int_type, T> &&
                                        !std::is_unsigned_v<T>, bool> = true>
            constexpr fraction(const int_type& n, const T& d) noexcept
            : num(0), denom(1), overflow(false)
            {
                if( n != 0 && d != 0 ) {
                    // calculate smallest num and denom, both arguments 'n' and 'd' may be negative
                    const uint_type abs_d = jau::abs(d);
                    const uint_type _gcd = gcd<uint_type>( (uint_type)jau::abs(n), abs_d );
                    num = ( n * jau::sign(d) ) / (int_type)_gcd;
                    denom = abs_d / _gcd;
                }
            }


            /**
             * Constructs a fraction instance with smallest numerator and denominator using gcd()
             *
             * Note: sign is always stored in fraction's numerator, i.e. the denominator is always positive and hence unsigned.
             *
             * @param n the given numerator
             * @param d the given denominator
             */
            constexpr fraction(const int_type& n, const uint_type& abs_d) noexcept
            : num(0), denom(1), overflow(false)
            {
                if( n != 0 && abs_d != 0 ) {
                    // calculate smallest num and denom, only given argument 'n' can be negative
                    const uint_type _gcd = gcd<uint_type>( (uint_type)jau::abs(n), abs_d );
                    num = n / (int_type)_gcd;
                    denom = abs_d / _gcd;
                }
            }

            // We use the implicit default copy- and move constructor and assignment operations,
            // rendering fraction TriviallyCopyable
#if 0
            constexpr fraction(const fraction<int_type> &o) noexcept
            : num(o.num), denom(o.denom) { }

            constexpr fraction(fraction<int_type> &&o) noexcept
            : num(std::move(o.num)), denom(std::move(o.denom)) { }

            constexpr fraction& operator=(const fraction<int_type> &o) noexcept {
                num = o.num;
                denom = o.denom;
                return *this;
            }
            constexpr fraction& operator=(fraction<int_type> &&o) noexcept {
                num = std::move( o.num );
                denom = std::move( o.denom );
                return *this;
            }
#endif

            /**
             * Reduce this fraction to the lowest terms using the greatest common denominator, see gcd(), i.e. normalization.
             *
             * Might need to be called after manual modifications on numerator or denominator.
             *
             * Not required after applying any provided operation as they normalize the fraction.
             */
            constexpr fraction<int_type>& reduce() noexcept {
                if( num != 0 && denom != 0 ) {
                    const uint_type _gcd = gcd<uint_type>( (uint_type)jau::abs(num), denom );
                    num /= static_cast<int_type>(_gcd);
                    denom /= _gcd;
                }
                return *this;
            }

            /**
             * Converts this this fraction to a numerator for the given new base fraction.
             *
             * If overflow_ptr is not nullptr, true is stored if an overflow occurred otherwise false.
             *
             * @param new_base the new base fraction for conversion
             * @param overflow_ptr optional pointer to overflow result, defaults to nullptr
             * @return numerator representing this fraction on the new base, or std::numeric_limits<int_type>::max() if an overflow occurred.
             */
            constexpr int_type to_num_of(const fraction<int_type>& new_base, bool * overflow_ptr=nullptr) const noexcept {
                // const uint_type _lcm = lcm<uint_type>( denom, new_base.denom );
                // return ( num * (int_type)( _lcm / denom ) ) / new_base.num;
                //
                if( 0 != denom && 0 != new_base.num ) {
                    int_type r;
                    if( mul_overflow(num, (int_type)new_base.denom, r) ) {
                        if( nullptr != overflow_ptr ) {
                            *overflow_ptr = true;
                        }
                        return std::numeric_limits<int_type>::max();
                    } else {
                        if( nullptr != overflow_ptr ) {
                            *overflow_ptr = false;
                        }
                        return r / (int_type)denom / new_base.num;
                    }
                } else {
                    // div-by-zero -> max value as we don't throw
                    return std::numeric_limits<int_type>::max();
                }
            }

            /**
             * Converts this this fraction to a numerator for the given new base fraction.
             *
             * If overflow_ptr is not nullptr, true is stored if an overflow occurred otherwise false.
             *
             * @param new_base_num the new base numerator for conversion
             * @param new_base_denom the new base denominator for conversion
             * @param new_base the new base fraction for conversion
             * @param overflow_ptr optional pointer to overflow result, defaults to nullptr
             * @return numerator representing this fraction on the new base, or std::numeric_limits<int_type>::max() if an overflow occurred.
             */
            constexpr int_type to_num_of(const int_type& new_base_num, const uint_type& new_base_denom, bool * overflow_ptr=nullptr) const noexcept {
                if( 0 != denom && 0 != new_base_num ) {
                    int_type r;
                    if( mul_overflow(num, (int_type)new_base_denom, r) ) {
                        if( nullptr != overflow_ptr ) {
                            *overflow_ptr = true;
                        }
                        return std::numeric_limits<int_type>::max();
                    } else {
                        if( nullptr != overflow_ptr ) {
                            *overflow_ptr = false;
                        }
                        return r / (int_type)denom / new_base_num;
                    }
                } else {
                    // div-by-zero -> max value as we don't throw
                    return std::numeric_limits<int_type>::max();
                }
            }

            /**
             * Convenient shortcut to `to_num_of(1_ms)`
             * @return time in milliseconds
             * @see to_num_of()
             */
            constexpr int_type to_ms() const noexcept { return to_num_of(1l, 1'000lu); }

            /**
             * Convenient shortcut to `to_num_of(1_us)`
             * @return time in microseconds
             * @see to_num_of()
             */
            constexpr int_type to_us() const noexcept { return to_num_of(1l, 1'000'000lu); }

            /**
             * Convenient shortcut to `to_num_of(1_ns)`
             * @return time in nanoseconds
             * @see to_num_of()
             */
            constexpr int_type to_ns() const noexcept { return to_num_of(1l, 1'000'000'000lu); }

            /** Returns the converted fraction to lossy float */
            constexpr float to_float() const noexcept { return (float)num / (float)denom; }

            /** Returns the converted fraction to lossy double */
            constexpr double to_double() const noexcept { return (double)num / (double)denom; }

            /** Returns the converted fraction to lossy long double */
            constexpr long double to_ldouble() const noexcept { return (long double)num / (long double)denom; }

            /**
             * Constructs a fraction from the given std::chrono::duration and its Rep and Period
             * with smallest numerator and denominator using gcd()
             *
             * Note: sign is always stored in fraction's numerator, i.e. the denominator is always positive and hence unsigned.
             *
             * @tparam Rep Rep of given std::chrono::duration
             * @tparam Period Period of given std::chrono::duration
             * @param dur std::chrono::duration reference to convert into a fraction
             */
            template<typename Rep, typename Period>
            constexpr fraction(const std::chrono::duration<Rep, Period>& dur) noexcept
            : num(0), denom(1), overflow(false)
            {
                if( dur.count()*Period::num != 0 && Period::den != 0 ) {
                    // calculate smallest num and denom, both arguments 'n' and 'd' may be negative
                    const int_type n = dur.count()*Period::num;
                    const int_type d = Period::den;
                    const uint_type abs_d = jau::abs(d);
                    const uint_type _gcd = gcd<uint_type>( (uint_type)jau::abs(n), abs_d );
                    num = ( n * jau::sign(d) ) / (int_type)_gcd;
                    denom = abs_d / _gcd;
                }
            }

            /**
             * Convert this fraction into std::chrono::duration with given Rep and Period
             *
             * If overflow_ptr is not nullptr, true is stored if an overflow occurred otherwise false.
             *
             * @tparam Rep std::chrono::duration numerator type
             * @tparam Period std::chrono::duration denominator type, i.e. a std::ratio
             * @param dur_ref std::chrono::duration reference to please automated template type deduction and ease usage
             * @param overflow_ptr optional pointer to overflow result, defaults to nullptr
             * @return fraction converted into given std::chrono::duration Rep and Period, or using (Rep)std::numeric_limits<Rep>::max() if an overflow occurred
             */
            template<typename Rep, typename Period>
            std::chrono::duration<Rep, Period> to_duration(const std::chrono::duration<Rep, Period>& dur_ref, bool * overflow_ptr=nullptr) const noexcept {
                (void)dur_ref; // just to please template type deduction
                bool overflow_ = false;
                const int_type num_ = to_num_of( fraction<int_type>( (int_type)Period::num, (uint_type)Period::den ), &overflow_ );
                if( overflow_ ) {
                    if( nullptr != overflow_ptr ) {
                        *overflow_ptr = true;
                    }
                    return std::chrono::duration<Rep, Period>( (Rep)std::numeric_limits<Rep>::max() );
                } else {
                    if( nullptr != overflow_ptr ) {
                        *overflow_ptr = false;
                    }
                    return std::chrono::duration<Rep, Period>( (Rep)num_ );
                }
            }

            /**
             * Returns a string representation of this fraction.
             *
             * If the overflow flag is set, ` O! ` will be appended.
             *
             * @param show_double true to show the double value, otherwise false (default)
             * @return
             */
            std::string to_string(const bool show_double=false) const noexcept {
                std::string r = std::to_string(num) + "/" + std::to_string(denom);
                if( overflow ) {
                    r.append(" O! ");
                } else if( show_double ) {
                    const int precision = std::max<int>( 6, digits10(denom, false /* sign_is_digit */) );
                    std::string fmt("%.");
                    fmt.append(std::to_string(precision)).append("f");
                    char buf[96];
                    PRAGMA_DISABLE_WARNING_PUSH
                    PRAGMA_DISABLE_WARNING_FORMAT_NONLITERAL
                    int res = ::snprintf(buf, sizeof(buf), fmt.c_str(), to_double());
                    PRAGMA_DISABLE_WARNING_POP
                    if( 0 < res ) {
                        r.append(" ( " + std::string(buf) + " )");
                    }
                }
                return r;
            }

            /**
             * Returns true if numerator is zero.
             */
            constexpr bool is_zero() const noexcept {
                return 0 == num;
            }

            /**
             * Returns the value of the sign function applied to numerator.
             * <pre>
             * -1 for numerator < 0
             *  0 for numerator = 0
             *  1 for numerator > 0
             * </pre>
             * @return function result
             */
            constexpr snsize_t sign() const noexcept {
                return jau::sign(num);
            }

            /**
             * Unary minus
             *
             * @return new instance with negated value, reduced
             */
            constexpr fraction<int_type> operator-() const noexcept {
                fraction<int_type> r(*this);
                r.num *= (int_type)-1;
                return r;
            }

            /**
             * Multiplication of this fraction's numerator with scalar in place.
             *
             * Operation may set the overflow flag if occurring.
             *
             * @param rhs the scalar
             * @return reference to this instance, reduced
             */
            constexpr fraction<int_type>& operator*=(const int_type& rhs ) noexcept {
                if( mul_overflow(num, rhs, num) ) {
                    set_overflow();
                    return *this;
                } else {
                    return reduce();
                }
            }

            /**
             * Division of this fraction's numerator with scalar in place.
             *
             * @param rhs the scalar
             * @return reference to this instance, reduced
             */
            constexpr fraction<int_type>& operator/=(const int_type& rhs ) noexcept {
                return this->operator/=(fraction<int_type>(rhs, (int_type)1));
            }

            /**
             * Compound assignment (addition)
             *
             * Operation may set the overflow flag if occurring.
             *
             * @param rhs the other fraction
             * @return reference to this instance, reduced
             */
            constexpr fraction<int_type>& operator+=(const fraction<int_type>& rhs ) noexcept {
                if( denom == rhs.denom ) {
                    num += rhs.num;
                } else {
                    uint_type _lcm;
                    if( lcm_overflow<uint_type>(denom, rhs.denom, _lcm) ) {
                        set_overflow();
                        return *this;
                    } else {
                        const int_type num_new = ( num * (int_type)( _lcm / denom ) ) + ( rhs.num * (int_type)( _lcm / rhs.denom ) );
                        num = num_new;
                        denom = _lcm;
                    }
                }
                return reduce();
            }

            /**
             * Negative compound assignment (subtraction)
             *
             * Operation may set the overflow flag if occurring.
             *
             * @param rhs the other fraction
             * @return reference to this instance, reduced
             */
            constexpr fraction<int_type>& operator-=(const fraction<int_type>& rhs ) noexcept {
                if( denom == rhs.denom ) {
                    num -= rhs.num;
                } else if( 0 != denom && 0 != rhs.denom ) {
                    uint_type _lcm;
                    if( lcm_overflow<uint_type>(denom, rhs.denom, _lcm) ) {
                        set_overflow();
                        return *this;
                    } else {
                        const int_type num_new = ( num * (int_type)( _lcm / denom ) ) - ( rhs.num * (int_type)( _lcm / rhs.denom ) );
                        num = num_new;
                        denom = _lcm;
                    }
                } else {
                    // div-by-zero -> max value as we don't throw
                    if( 0 == denom && 0 == rhs.denom ) {
                        num = 0; // 0 = inf - inf
                    } else if( 0 == denom ) {
                        num = std::numeric_limits<int_type>::max(); // inf = inf - x
                    } else /* if( 0 == rhs.denom ) */ {
                        num = std::numeric_limits<int_type>::min(); // -inf = x - inf
                    }
                    denom = 1;
                }
                return reduce();
            }


            /**
             * Multiplication in place.
             *
             * Operation may set the overflow flag if occurring.
             *
             * @param rhs the other fraction
             * @return reference to this instance, reduced
             */
            constexpr fraction<int_type>& operator*=(const fraction<int_type>& rhs ) noexcept {
                const uint_type gcd1 = gcd<uint_type>( (uint_type)jau::abs(num),     rhs.denom );
                const uint_type gcd2 = gcd<uint_type>( (uint_type)jau::abs(rhs.num), denom );
                const int_type n1 = num / (int_type)gcd1;
                const int_type n2 = rhs.num / (int_type)gcd2;
                const uint_type d1 = denom / gcd2;
                const uint_type d2 = rhs.denom / gcd1;

                if( mul_overflow(n1, n2, num) || mul_overflow(d1, d2, denom) ) {
                    set_overflow();
                }
                return *this;
            }

            /**
             * Division in place.
             *
             * @param rhs the other fraction
             * @return reference to this instance, reduced
             */
            constexpr fraction<int_type>& operator/=(const fraction<int_type>& rhs ) noexcept {
                // flipped rhs num and denom as compared to multiply
                const uint_type abs_num2 = jau::abs(rhs.num);
                const uint_type gcd1 = gcd<uint_type>( (uint_type)jau::abs(num),  abs_num2 );
                const uint_type gcd2 = gcd<uint_type>(          rhs.denom ,  denom );

                num = ( num / (int_type)gcd1 ) * jau::sign(rhs.num) * ( rhs.denom / (int_type)gcd2 );
                denom = ( denom / gcd2 ) * ( abs_num2 / gcd1 );
                return *this;
            }
    };

    template<typename int_type>
    inline std::string to_string(const fraction<int_type>& v) noexcept { return v.to_string(); }

    template<typename int_type>
    constexpr bool operator!=(const fraction<int_type>& lhs, const fraction<int_type>& rhs ) noexcept {
        return lhs.denom != rhs.denom || lhs.num != rhs.num;
    }

    template<typename int_type>
    constexpr bool operator==(const fraction<int_type>& lhs, const fraction<int_type>& rhs ) noexcept {
        return !( lhs != rhs );
    }

    template<typename int_type>
    constexpr bool operator>(const fraction<int_type>& lhs, const fraction<int_type>& rhs ) noexcept {
        return lhs.num * (int_type)rhs.denom > (int_type)lhs.denom * rhs.num;
    }

    template<typename int_type>
    constexpr bool operator>=(const fraction<int_type>& lhs, const fraction<int_type>& rhs ) noexcept {
        return lhs.num * (int_type)rhs.denom >= (int_type)lhs.denom * rhs.num;
    }

    template<typename int_type>
    constexpr bool operator<(const fraction<int_type>& lhs, const fraction<int_type>& rhs ) noexcept {
        return lhs.num * (int_type)rhs.denom < (int_type)lhs.denom * rhs.num;
    }

    template<typename int_type>
    constexpr bool operator<=(const fraction<int_type>& lhs, const fraction<int_type>& rhs ) noexcept {
        return lhs.num * (int_type)rhs.denom <= (int_type)lhs.denom * rhs.num;
    }

    /** Return the maximum of the two given fractions */
    template<typename int_type>
    constexpr const fraction<int_type>& max(const fraction<int_type>& lhs, const fraction<int_type>& rhs ) noexcept {
        return lhs >= rhs ? lhs : rhs;
    }

    /** Return the minimum of the two given fractions */
    template<typename int_type>
    constexpr const fraction<int_type>& min(const fraction<int_type>& lhs, const fraction<int_type>& rhs ) noexcept {
        return lhs <= rhs ? lhs : rhs;
    }

    /**
     * Returns the value of the sign function applied to numerator.
     * <pre>
     * -1 for numerator < 0
     *  0 for numerator = 0
     *  1 for numerator > 0
     * </pre>
     * @return function result
     */
    template<typename int_type>
    constexpr snsize_t sign(const fraction<int_type>& rhs) noexcept {
        return jau::sign(rhs.num);
    }

    /**
     * Returns the absolute fraction
     */
    template<typename int_type>
    constexpr fraction<int_type> abs(const fraction<int_type>& rhs) noexcept {
        fraction<int_type> copy(rhs); // skip normalize
        copy.num = jau::abs(rhs.num);
        return copy;
    }

    /**
     * Returns multiplication of fraction with scalar.
     *
     * Operation may set the overflow flag in the returned instance, if occurring.
     *
     * @tparam int_type integral type
     * @param lhs the fraction
     * @param rhs the scalar
     * @return resulting new fraction, reduced
     */
    template<typename int_type>
    constexpr fraction<int_type> operator*(const fraction<int_type>& lhs, const int_type& rhs ) noexcept {
        return fraction<int_type>( lhs.num*rhs, lhs.denom );
    }

    /**
     * Returns multiplication of fraction with scalar.
     *
     * Operation may set the overflow flag in the returned instance, if occurring.
     *
     * @tparam int_type integral type
     * @param lhs the scalar
     * @param rhs the fraction
     * @return resulting new fraction, reduced
     */
    template<typename int_type>
    constexpr fraction<int_type> operator*(const int_type& lhs, const fraction<int_type>& rhs) noexcept {
        return fraction<int_type>( rhs.num*lhs, rhs.denom );
    }

    /**
     * Returns division of fraction with scalar.
     * @tparam int_type integral type
     * @param lhs the fraction
     * @param rhs the scalar
     * @return resulting new fraction, reduced
     */
    template<typename int_type>
    constexpr fraction<int_type> operator/(const fraction<int_type>& lhs, const int_type& rhs ) noexcept {
        fraction<int_type> r(lhs);
        return r /= rhs;
    }

    /**
     * Returns division of fraction with scalar.
     * @tparam int_type integral type
     * @param lhs the scalar
     * @param rhs the fraction
     * @return resulting new fraction, reduced
     */
    template<typename int_type>
    constexpr fraction<int_type> operator/(const int_type& lhs, const fraction<int_type>& rhs) noexcept {
        fraction<int_type> r( lhs, (int_type)1 );
        return r /= rhs;
    }

    /**
     * Returns sum of two fraction.
     *
     * Operation may set the overflow flag in the returned instance, if occurring.
     *
     * @tparam int_type integral type
     * @param lhs a fraction
     * @param rhs a fraction
     * @return resulting new fraction, reduced
     */
    template<typename int_type>
    constexpr fraction<int_type> operator+(const fraction<int_type>& lhs, const fraction<int_type>& rhs ) noexcept {
        fraction<int_type> r(lhs);
        r += rhs; // implicit reduce
        return r;
    }

    /**
     * Returns difference of two fraction.
     *
     * Operation may set the overflow flag in the returned instance, if occurring.
     *
     * @tparam int_type integral type
     * @param lhs a fraction
     * @param rhs a fraction
     * @return resulting new fraction, reduced
     */
    template<typename int_type>
    constexpr fraction<int_type> operator-(const fraction<int_type>& lhs, const fraction<int_type>& rhs ) noexcept {
        fraction<int_type> r(lhs);
        r -= rhs; // implicit reduce
        return r;
    }

    /**
     * Returns product of two fraction.
     *
     * Operation may set the overflow flag in the returned instance, if occurring.
     *
     * @tparam int_type integral type
     * @param lhs a fraction
     * @param rhs a fraction
     * @return resulting new fraction, reduced
     */
    template<typename int_type>
    constexpr fraction<int_type> operator*(const fraction<int_type>& lhs, const fraction<int_type>& rhs ) noexcept {
        fraction<int_type> r(lhs);
        r *= rhs; // implicit reduce
        return r;
    }

    /**
     * Returns division of two fraction.
     * @tparam int_type integral type
     * @param lhs a fraction
     * @param rhs a fraction
     * @return resulting new fraction, reduced
     */
    template<typename int_type>
    constexpr fraction<int_type> operator/(const fraction<int_type>& lhs, const fraction<int_type>& rhs ) noexcept {
        fraction<int_type> r(lhs);
        r /= rhs; // implicit reduce
        return r;
    }

    /** fraction using int64_t as integral type */
    typedef fraction<int64_t> fraction_i64;

    /**
     * Stores the fraction_i64 value of the given string value in format `<num>/<denom>`,
     * which may contain whitespace.
     *
     * It the given string value does not conform with the format
     * or exceeds the given value range, `false` is being returned.
     *
     * If the given string value has been accepted, it is stored in the result reference
     * and `true` is being returned.
     *
     * @param result storage for result is value is parsed successfully and within range.
     * @param value the string value
     * @param min_allowed the minimum allowed value
     * @param max_allowed the maximum allowed value
     * @return true if value has been accepted, otherwise false
     */
    bool to_fraction_i64(fraction_i64& result, const std::string & value, const fraction_i64& min_allowed, const fraction_i64& max_allowed) noexcept;


    /** fraction using uint64_t as integral type */
    typedef fraction<uint64_t> fraction_u64;

    /** fractions namespace to provide fraction constants using int64_t as underlying integral integer type. */
    namespace fractions_i64 { // Note: int64_t == intmax_t -> 10^18 or 19 digits (for intmax_t on 64bit platforms)
        /** tera is 10^12 */
        inline constexpr const jau::fraction_i64 tera ( 1'000'000'000'000l,                 1lu );
        /** giga is 10^9 */
        inline constexpr const jau::fraction_i64 giga (     1'000'000'000l,                 1lu );
        /** mega is 10^6 */
        inline constexpr const jau::fraction_i64 mega (         1'000'000l,                 1lu );
        /** years is 31'557'600/1 using 365.2425 days per year */
        inline constexpr const jau::fraction_i64 years (       31'557'600l,                 1lu );
        /** days is 86400/1 */
        inline constexpr const jau::fraction_i64 days (            86'400l,                 1lu );
        /** hours is 3660/1 */
        inline constexpr const jau::fraction_i64 hours (            3'600l,                 1lu );
        /** kilo is 10^3 */
        inline constexpr const jau::fraction_i64 kilo (             1'000l,                 1lu );
        /** minutes is 60/1 */
        inline constexpr const jau::fraction_i64 minutes (             60l,                 1lu );
        /** seconds is 1/1 */
        inline constexpr const jau::fraction_i64 seconds (              1l,                 1lu );
        /** one is 10^0 or 1/1 */
        inline constexpr const jau::fraction_i64 one  (                 1l,                 1lu );
        /** zero is 0/1 */
        inline constexpr const jau::fraction_i64 zero (                 0l,                 1lu );
        /** milli is 10^-3 */
        inline constexpr const jau::fraction_i64 milli(                 1l,             1'000lu );
        /** micro is 10^-6 */
        inline constexpr const jau::fraction_i64 micro(                 1l,         1'000'000lu );
        /** nano is 10^-9 */
        inline constexpr const jau::fraction_i64 nano (                 1l,     1'000'000'000lu );
        /** pico is 10^-12 */
        inline constexpr const jau::fraction_i64 pico (                 1l, 1'000'000'000'000lu );
    } // namespace fractions_i64

    namespace fractions_i64_literals {
        /** Literal for fractions_i64::tera */
        constexpr fraction_i64 operator ""_T(unsigned long long int __T)     { return (int64_t)__T    * fractions_i64::tera; }
        /** Literal for fractions_i64::giga */
        constexpr fraction_i64 operator ""_G(unsigned long long int __G)     { return (int64_t)__G    * fractions_i64::giga; }
        /** Literal for fractions_i64::mega */
        constexpr fraction_i64 operator ""_M(unsigned long long int __M)     { return (int64_t)__M    * fractions_i64::mega; }
        /** Literal for fractions_i64::kilo */
        constexpr fraction_i64 operator ""_k(unsigned long long int __k)     { return (int64_t)__k    * fractions_i64::kilo; }
        /** Literal for fractions_i64::one */
        constexpr fraction_i64 operator ""_one(unsigned long long int __one) { return (int64_t)__one  * fractions_i64::one; }
        /** Literal for fractions_i64::milli */
        constexpr fraction_i64 operator ""_m(unsigned long long int __m)     { return (int64_t)__m    * fractions_i64::milli; }
        /** Literal for fractions_i64::micro */
        constexpr fraction_i64 operator ""_u(unsigned long long int __u)     { return (int64_t)__u    * fractions_i64::micro; }
        /** Literal for fractions_i64::nano */
        constexpr fraction_i64 operator ""_n(unsigned long long int __n)     { return (int64_t)__n    * fractions_i64::nano; }
        /** Literal for fractions_i64::pico */
        constexpr fraction_i64 operator ""_p(unsigned long long int __p)     { return (int64_t)__p    * fractions_i64::pico; }

        /** Literal for fractions_i64::years */
        constexpr fraction_i64 operator ""_y(unsigned long long int __y)     { return (int64_t)__y    * fractions_i64::years; }
        /** Literal for fractions_i64::days */
        constexpr fraction_i64 operator ""_d(unsigned long long int __d)     { return (int64_t)__d    * fractions_i64::days; }
        /** Literal for fractions_i64::hours */
        constexpr fraction_i64 operator ""_h(unsigned long long int __h)     { return (int64_t)__h    * fractions_i64::hours; }
        /** Literal for fractions_i64::minutes */
        constexpr fraction_i64 operator ""_min(unsigned long long int __min) { return (int64_t)__min  * fractions_i64::minutes; }
        /** Literal for fractions_i64::seconds */
        constexpr fraction_i64 operator ""_s(unsigned long long int __s)     { return (int64_t)__s    * fractions_i64::seconds; }
        /** Literal for fractions_i64::milli */
        constexpr fraction_i64 operator ""_ms(unsigned long long int __ms)   { return (int64_t)__ms   * fractions_i64::milli; }
        /** Literal for fractions_i64::micro */
        constexpr fraction_i64 operator ""_us(unsigned long long int __us)   { return (int64_t)__us   * fractions_i64::micro; }
        /** Literal for fractions_i64::nano */
        constexpr fraction_i64 operator ""_ns(unsigned long long int __ns)   { return (int64_t)__ns   * fractions_i64::nano; }
    } // namespace fractions_i64_literals

    /**
     * Timespec structure using int64_t for its components
     * in analogy to `struct timespec_t` on 64-bit platforms.
     *
     * fraction_timespec covers an almost infinite range of time
     * while maintaining high precision like `struct timespec_t` on 64-bit platforms.
     *
     * If used as an absolute time-point, zero is time since Unix Epoch `00:00:00 UTC on 1970-01-01`.
     *
     * Note-1: Counting nanoseconds in int64_t only lasts until `2262-04-12`,
     * since INT64_MAX is 9'223'372'036'854'775'807 for 9'223'372'036 seconds or 292 years.
     *
     * Note-2: Limitations of `struct timespec` on 32-bit platforms
     * - to_timespec() conversion to `struct timespec`
     * - 32-bit signed integer only last for 68 years or until year 2038, starting from 1970 Unix Epoch
     * - [System call conversion for year 2038](https://lwn.net/Articles/643234/)
     * - test/test_fractions_01.cpp `struct timespec type validation Test 03.00`
     *
     * @see to_timespec()
     * @see to_fraction_i64()
     * @see sleep_until()
     * @see sleep_for()
     * @see wait_until()
     * @see wait_for()
     * @see getMonotonicTime()
     * @see getWallClockTime()
     */
    struct fraction_timespec {
        /**
         * Seconds component, with its absolute value in range [0..inf[ or [0..inf).
         */
        int64_t tv_sec;

        /**
         * Nanoseconds component with its absolute value in range [0..1'000'000'000[ or [0..1'000'000'000).
         */
        int64_t tv_nsec;

        /**
         * Constructs a zero fraction_timespec instance
         */
        constexpr fraction_timespec() noexcept
        : tv_sec(0), tv_nsec(0) { }

        /**
         * Constructs a fraction_timespec instance with given components, normalized.
         */
        constexpr fraction_timespec(const int64_t s, const int64_t ns) noexcept
        : tv_sec(s), tv_nsec(ns) { normalize(); }

        /**
         * Constructs a fraction_timespec instance with given floating point seconds, normalized.
         */
        explicit constexpr fraction_timespec(const double seconds) noexcept
        : tv_sec( static_cast<int64_t>(seconds) ),
          tv_nsec( static_cast<int64_t>( (seconds - static_cast<double>(tv_sec)) * 1e+9) ) { }

        /**
         * Conversion constructor of fraction_timespec for a fraction_i64 value.
         *
         * If overflow_ptr is not nullptr, true is stored if an overflow occurred, otherwise false.
         *
         * In case of an overflow, tv_sec and tv_nsec will also be set to INT64_MAX
         *
         * Example without overflow check and implicit fraction_i64 conversion to fraction_timespec:
         * <pre>
            fraction_i64 duration = 10_ms;
            const fraction_timespec timeout_time = getMonotonicTime() + duration;
         * </pre>
         *
         * Example with overflow check for potential durations > 292 years and explicit fraction_i64 conversion to fraction_timespec:
         * <pre>
            fraction_i64 duration = 10_ms;
            bool overflow = false;
            const fraction_timespec timeout_time = getMonotonicTime() + fraction_timespec(duration, &overflow);
            if( overflow ) {
                return; // bail out
            }
         * </pre>
         * @param r the conversion input
         * @param overflow_ptr optional pointer to overflow result, defaults to nullptr
         */
        constexpr fraction_timespec(const fraction_i64& r, bool * overflow_ptr=nullptr) noexcept
        : tv_sec(0), tv_nsec(0)
        {
            bool overflow = false;
            tv_sec = r.to_num_of(fractions_i64::seconds, &overflow);
            if( !overflow ) {
                const fraction_i64 ns = r - tv_sec * fractions_i64::seconds;
                tv_nsec = ns.to_num_of(fractions_i64::nano, &overflow);
            }
            if( overflow ) {
                if( nullptr != overflow_ptr ) {
                    *overflow_ptr = true;
                }
                tv_sec = INT64_MAX;
                tv_nsec = INT64_MAX;
            }
        }

        /**
         * Conversion constructor from an ISO8601 time string,
         * as produced via to_iso8601_string().
         *
         * Implementation allows having space instead of specified delimiters like `T` and `Z`.
         *
         * @param datestr the ISO8601 time string to parse
         * @param utcOffsetSec storage for UTC offset in seconds
         * @param consumedChars number of consumed chars successfully parsed
         * @param returns the fraction_timespec in UTC 
         */
        static fraction_timespec from(const std::string_view datestr, int64_t &utcOffsetSec, size_t &consumedChars) noexcept;

        /**
         * Conversion constructor from an ISO8601 time string,
         * as produced via to_iso8601_string().
         *
         * Implementation allows having space instead of specified delimiters like `T` and `Z`.
         *
         * @param datestr the ISO8601 time string to parse
         * @param addUTCOffset if true, add UTC offset to the resulting time, otherwise return UTC only
         */
        static fraction_timespec from(const std::string_view datestr, Bool addUTCOffset = Bool::False) noexcept;

        /**
         * Conversion constructor from broken down values assuming UTC
         * @param year year number, 0 as 0 A.D.
         * @param month month number [1-12]
         * @param day day of the month [1-31]
         * @param hour hours since midnight [0-23]
         * @param minute minutes after the hour [0-59]
         * @param seconds seconds after the minute including one leap second [0-60]
         * @param nano_seconds nanoseconds [0, 1'000'000'000)
        */
        static fraction_timespec from(int year, unsigned month, unsigned day,
                                      unsigned hour=0, unsigned minute=0,
                                      unsigned seconds=0, uint64_t nano_seconds=0) noexcept;

        /**
         * Returns the sum of both components.
         *
         * If applied to relative duration, i.e. difference of two time points,
         * its range is good for 292 years and exceeds that of an `int64_t nanoseconds` timepoint-difference greatly.
         *
         * <pre>
         *   fraction_timespec t0 = getMonotonicTime();
         *   // do something
         *
         *   // Exact duration
         *   fraction_timespec td_1 = getMonotonicTime() - t0;
         *
         *   // or for durations <= 292 years
         *   fraction_i64 td_2 = (getMonotonicTime() - t0).to_fraction_i64();
         * </pre>
         * @see getMonotonicTime()
         */
        constexpr fraction_i64 to_fraction_i64() const noexcept {
            return ( tv_sec * fractions_i64::seconds ) + ( tv_nsec * fractions_i64::nano );
        }

        /**
         * Normalize tv_nsec with its absolute range [0..1'000'000'000[ or [0..1'000'000'000)
         * and having same sign as tv_sec.
         *
         * Used after arithmetic operations.
         *
         * @returns reference to this instance
         */
        constexpr fraction_timespec& normalize() noexcept {
            if( 0 != tv_nsec ) {
                constexpr int64_t ns_per_sec = 1'000'000'000L;
                if( std::abs(tv_nsec) >= ns_per_sec ) {
                    const int64_t c = tv_nsec / ns_per_sec;
                    tv_nsec -= c * ns_per_sec;
                    tv_sec += c;
                }
                if( tv_nsec < 0 && tv_sec >= 1 ) {
                    tv_nsec += ns_per_sec;
                    tv_sec -= 1;
                } else if( tv_nsec > 0 && tv_sec <= -1 ) {
                    tv_nsec -= ns_per_sec;
                    tv_sec += 1;
                }
            }
            return *this;
        }

        constexpr bool isZero() noexcept { return 0 == tv_sec && 0 == tv_nsec; }
        constexpr void clear() noexcept { tv_sec=0; tv_nsec=0; }
        
        /** Two way comparison operator */
        constexpr bool operator==(const fraction_timespec& rhs ) const noexcept {
            return tv_sec == rhs.tv_sec && tv_nsec == rhs.tv_nsec;
        }

        /** Three way std::strong_ordering comparison operator */
        constexpr std::strong_ordering operator<=>(const fraction_timespec& rhs) const noexcept {
            if( tv_sec == rhs.tv_sec ) {
                return tv_nsec == rhs.tv_nsec ?
                    std::strong_ordering::equal :
                    ( tv_nsec < rhs.tv_nsec ? std::strong_ordering::less : std::strong_ordering::greater );
            }
            return tv_sec < rhs.tv_sec ? std::strong_ordering::less : std::strong_ordering::greater;
        }

        /**
         * Compound assignment (addition)
         *
         * @param rhs the other fraction_timespec
         * @return reference to this instance, normalized
         */
        constexpr fraction_timespec& operator+=(const fraction_timespec& rhs ) noexcept {
            tv_nsec += rhs.tv_nsec; // we allow the 'overflow' over 1'000'000'000, fitting into type and normalize() later
            tv_sec += rhs.tv_sec;
            return normalize();
        }

        /**
         * Negative compound assignment (subtraction)
         *
         * @param rhs the other fraction_timespec
         * @return reference to this instance, normalized
         */
        constexpr fraction_timespec& operator-=(const fraction_timespec& rhs ) noexcept {
            tv_nsec -= rhs.tv_nsec; // we allow the 'overflow' over 1'000'000'000, fitting into type and normalize() later
            tv_sec -= rhs.tv_sec;
            return normalize();
        }
        
        /**
         * Compound product (multiplication)
         *
         * @param rhs a scalar
         * @return reference to this instance, normalized
         */
        constexpr fraction_timespec& operator*=(const int64_t rhs ) noexcept {
            tv_nsec *= rhs; // we allow the 'overflow' over 1'000'000'000, fitting into type and normalize() later
            tv_sec *= rhs;
            return normalize();
        }

        /**
         * Compound fraction (division)
         *
         * @param rhs a scalar
         * @return reference to this instance, normalized
         */
        constexpr fraction_timespec& operator/=(const int64_t rhs ) noexcept {
            constexpr int64_t ns_per_sec  = 1'000'000'000L;
            constexpr int64_t SecLimit  = std::numeric_limits<int64_t>::max() / ns_per_sec;
            if( tv_sec < SecLimit-1 ) {
                const int64_t ns = ( tv_sec * ns_per_sec + tv_nsec) / rhs;
                tv_sec = ns / ns_per_sec;
                tv_nsec = ns - ( tv_sec * ns_per_sec );  // we allow the 'overflow' over 1'000'000'000, fitting into type and normalize() later
                return normalize();
            } else {
                const double sec = to_double() / double(rhs);
                tv_sec = static_cast<int64_t>(sec);
                tv_nsec = static_cast<int64_t>( (sec - static_cast<double>(tv_sec)) * 1e+9);
                return *this;
            }
        }

        /**
         * Return conversion to POSIX `struct timespec`,
         * potentially narrowing the components if underlying system is not using 64-bit signed integer.
         *
         * Note-2: Limitations of `struct timespec` on 32-bit platforms
         * - 32-bit signed integer only last for 68 years or until year 2038, starting from 1970 Unix Epoch
         * - [System call conversion for year 2038](https://lwn.net/Articles/643234/)
         * - test/test_fractions_01.cpp `struct timespec type validation Test 03.00`
         */
        constexpr struct timespec to_timespec() const noexcept {
            using ns_type = decltype(timespec::tv_nsec);
            return { static_cast<std::time_t>( tv_sec ), static_cast<ns_type>( tv_nsec ) };
        }

        /**
         * Returns time in milliseconds
         *
         * If either tv_sec or tv_nsec is negative, method return 0.
         *
         * In case of overflow tv_sec >= UINT64_MAX / 1'000, method returns UINT64_MAX.
         */
        constexpr uint64_t to_ms() const noexcept {
            constexpr uint64_t ns_per_ms = 1'000'000UL;
            constexpr uint64_t ms_per_sec  =   1'000UL;
            constexpr int64_t SecLimit  = std::numeric_limits<uint64_t>::max() / ms_per_sec;
            if( tv_sec < 0 || tv_nsec < 0 ) {
                return 0;
            }
            return tv_sec < SecLimit ? tv_sec * ms_per_sec + tv_nsec / ns_per_ms : std::numeric_limits<uint64_t>::max();
        }

        /**
         * Returns time in microseconds
         *
         * If either tv_sec or tv_nsec is negative, method return 0.
         *
         * In case of overflow tv_sec >= UINT64_MAX / 1'000'000, method returns UINT64_MAX.
         */
        constexpr uint64_t to_us() const noexcept {
            constexpr uint64_t ns_per_us =       1'000UL;
            constexpr uint64_t us_per_sec  = 1'000'000UL;
            constexpr int64_t SecLimit  = std::numeric_limits<uint64_t>::max() / us_per_sec;
            if( tv_sec < 0 || tv_nsec < 0 ) {
                return 0;
            }
            return tv_sec < SecLimit ? tv_sec * us_per_sec + tv_nsec / ns_per_us : std::numeric_limits<uint64_t>::max();
        }

        /** Returns time in fractions of seconds of type double */
        constexpr double to_double() const noexcept {
            constexpr double ns_per_sec = 1'000'000'000.0;
            return double(tv_sec) + double(tv_nsec) / ns_per_sec;
        }


        /**
         * Return simple string representation in seconds and nanoseconds.
         */
        std::string to_string() const noexcept;

        /**
         * Convenience string conversion interpreted since Unix Epoch in UTC
         * to ISO 8601 `YYYY-mm-ddTHH:MM:SS.ssZ`, e.g. '2022-06-23T14:34:10Z` or '2022-06-23T14:34:10.228978909Z`.
         *
         * Implementation uses `strftime()` with format `%Y-%m-%dT%H:%M:%S`
         * and adds 9 digits nanoseconds as fractions of seconds if not zero and the final `Z`.
         *
         * Time will be dropped if hours, minutes, seconds and fractions of a seconds are all zero.
         *
         * @param space_separator if true, use simple space separator instead of `T` and drop `Z`,
         *        otherwise use ISO 8601 as described above. Defaults to `false`.
         *
         * @param muteTime if true, always mute time
         */
        std::string to_iso8601_string(bool space_separator=false, bool muteTime=false) const noexcept;
    };

    inline std::string to_string(const fraction_timespec& v) noexcept { return v.to_string(); }

    /** Return the maximum of the two given fraction_timespec */
    constexpr const fraction_timespec& max(const fraction_timespec& lhs, const fraction_timespec& rhs ) noexcept {
        return lhs > rhs ? lhs : rhs;
    }

    /** Return the minimum of the two given fraction_timespec */
    constexpr const fraction_timespec& min(const fraction_timespec& lhs, const fraction_timespec& rhs ) noexcept {
        return lhs < rhs ? lhs : rhs;
    }

    /**
     * Returns the value of the sign function applied to tv_sec.
     * <pre>
     * -1 for numerator < 0
     *  0 for numerator = 0
     *  1 for numerator > 0
     * </pre>
     * @return function result
     */
    constexpr snsize_t sign(const fraction_timespec& rhs) noexcept {
        return 0 != rhs.tv_sec ? jau::sign(rhs.tv_sec) : jau::sign(rhs.tv_nsec);
    }

    /**
     * Returns the absolute fraction_timespec
     */
    constexpr fraction_timespec abs(const fraction_timespec& rhs) noexcept {
        fraction_timespec copy(rhs); // skip normalize
        copy.tv_sec= jau::abs(rhs.tv_sec);
        copy.tv_nsec= jau::abs(rhs.tv_nsec);
        return copy;
    }

    /**
     * Returns sum of two fraction_timespec.
     *
     * @param lhs a fraction_timespec
     * @param rhs a fraction_timespec
     * @return resulting new fraction_timespec, each component reduced and both fraction_timespec::normalize() 'ed
     */
    constexpr fraction_timespec operator+(const fraction_timespec& lhs, const fraction_timespec& rhs ) noexcept {
        fraction_timespec r(lhs);
        r += rhs; // implicit normalize
        return r;
    }

    /**
     * Returns difference of two fraction_timespec.
     *
     * See fraction_timespec::to_fraction_i64().
     *
     * @param lhs a fraction_timespec
     * @param rhs a fraction_timespec
     * @return resulting new fraction_timespec, each component reduced and both fraction_timespec::normalize() 'ed
     */
    constexpr fraction_timespec operator-(const fraction_timespec& lhs, const fraction_timespec& rhs ) noexcept {
        fraction_timespec r(lhs);
        r -= rhs; // implicit normalize
        return r;
    }
    
    /**
     * Returns product of fraction_timespec * scalar.
     *
     * @param lhs a fraction_timespec
     * @param rhs a scalar
     * @return resulting new fraction_timespec, each component reduced and both fraction_timespec::normalize() 'ed
     */
    constexpr fraction_timespec operator*(const fraction_timespec& lhs, const int64_t rhs ) noexcept {
        fraction_timespec r(lhs);
        r *= rhs; // implicit normalize
        return r;
    }
    /**
     * Returns product of scalar * fraction_timespec.
     *
     * @param lhs a scalar
     * @param rhs a fraction_timespec
     * @return resulting new fraction_timespec, each component reduced and both fraction_timespec::normalize() 'ed
     */
    constexpr fraction_timespec operator*(const int64_t lhs, const fraction_timespec& rhs ) noexcept {
        fraction_timespec r(rhs);
        r *= lhs; // implicit normalize
        return r;
    }
    /**
     * Returns fraction of fraction_timespec / scalar.
     *
     * @param lhs a fraction_timespec
     * @param rhs a scalar
     * @return resulting new fraction_timespec, each component reduced and both fraction_timespec::normalize() 'ed
     */
    constexpr fraction_timespec operator/(const fraction_timespec& lhs, const int64_t rhs ) noexcept {
        fraction_timespec r(lhs);
        r /= rhs; // implicit normalize
        return r;
    }
    /**
     * Returns fraction of scalar / fraction_timespec.
     *
     * @param lhs a scalar
     * @param rhs a fraction_timespec
     * @return resulting new fraction_timespec, each component reduced and both fraction_timespec::normalize() 'ed
     */
    constexpr fraction_timespec operator/(const int64_t lhs, const fraction_timespec& rhs ) noexcept {
        fraction_timespec r(rhs);
        r /= lhs; // implicit normalize
        return r;
    }

    namespace fraction_tv {

        /** jau::fraction_timespec zero is { 0, 0 } */
        inline constexpr const jau::fraction_timespec zero(0, 0);

    } // namespace fraction_tv

    /** SC atomic integral scalar jau::fraction_i64. Memory-Model (MM) guaranteed sequential consistency (SC) between acquire (read) and release (write). Requires libatomic with libstdc++10. */
    typedef ordered_atomic<jau::fraction_i64, std::memory_order_seq_cst> sc_atomic_fraction_i64;

    /** Relaxed non-SC atomic integral scalar jau::fraction_i64. Memory-Model (MM) only guarantees the atomic value, _no_ sequential consistency (SC) between acquire (read) and release (write). Requires libatomic with libstdc++10. */
    typedef ordered_atomic<jau::fraction_i64, std::memory_order_relaxed> relaxed_atomic_fraction_i64;

    /** SC atomic integral scalar jau::fraction_u64. Memory-Model (MM) guaranteed sequential consistency (SC) between acquire (read) and release (write). Requires libatomic with libstdc++10. */
    typedef ordered_atomic<jau::fraction_u64, std::memory_order_seq_cst> sc_atomic_fraction_u64;

    /** Relaxed non-SC atomic integral scalar jau::fraction_u64. Memory-Model (MM) only guarantees the atomic value, _no_ sequential consistency (SC) between acquire (read) and release (write). Requires libatomic with libstdc++10. */
    typedef ordered_atomic<jau::fraction_u64, std::memory_order_relaxed> relaxed_atomic_fraction_u64;

    /**@}*/

} // namespace jau

namespace std {

    /** \addtogroup Fractions
     */

    /**
     * Output stream operator for jau::fraction_timespec
     */
    inline std::ostream& operator<<(std::ostream& os, const jau::fraction_timespec& v) noexcept {
        os << v.to_string();
        return os;
    }

    /**
     * Output stream operator for jau::fraction
     */
    template<typename int_type>
    inline std::ostream& operator<<(std::ostream& os, const jau::fraction<int_type>& v) noexcept {
        os << v.to_string();
        return os;
    }

    /**@}*/

} // namespace std


#endif /* JAU_FRACTION_TYPE_HPP_ */
