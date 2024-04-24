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

#ifndef JAU_FLOAT_SI_TYPES_HPP_
#define JAU_FLOAT_SI_TYPES_HPP_

namespace jau {

    /** \addtogroup Floats
     *
     *  @{
     */

    /** Time in fractions of seconds using float. */
    typedef float si_timef_t;
    /** Length in fractions of meter using float. */
    typedef float si_lengthf_t;
    /** Mass in fractions of kilograms using float. */
    typedef float si_massf_t;
    /** Speed in fractions of meter/seconds using float. */
    typedef float si_speedf_t;

    namespace float_literals {
        constexpr si_timef_t operator ""_h(unsigned long long int __v)   { return (si_timef_t)__v*3600.0; }
        constexpr si_timef_t operator ""_min(unsigned long long int __v)   { return (si_timef_t)__v*60.0; }
        constexpr si_timef_t operator ""_s(unsigned long long int __v)   { return (si_timef_t)__v; }
        constexpr si_timef_t operator ""_ms(unsigned long long int __v)   { return (si_timef_t)__v/1000.0; }

        constexpr si_lengthf_t operator ""_km(unsigned long long int __v)   { return (si_lengthf_t)__v*1000.0; }
        constexpr si_lengthf_t operator ""_m(unsigned long long int __v)   { return (si_lengthf_t)__v; }
        constexpr si_lengthf_t operator ""_cm(unsigned long long int __v)   { return (si_lengthf_t)__v/100.0; }
        constexpr si_lengthf_t operator ""_mm(unsigned long long int __v)   { return (si_lengthf_t)__v/1000.0; }

        constexpr si_massf_t operator ""_kg(unsigned long long int __v)   { return (si_massf_t)__v; }
        constexpr si_massf_t operator ""_g(unsigned long long int __v)   { return (si_massf_t)__v/1000.0; }

        constexpr si_massf_t operator ""_m_s(unsigned long long int __v)   { return (si_massf_t)__v; }
        constexpr si_massf_t operator ""_km_h(unsigned long long int __v)   { return (si_massf_t)__v / 3.6; }
    } // float_literals

    /**@}*/

} // namespace jau

#endif /* JAU_FLOAT_SI_TYPES_HPP_ */
