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
        /// jau::float_64_t or just double
        static const jau::type_info& f64() { return jau::static_ctti<float64_t>(); }
    };

    /**@}*/

} // namespace jau

#endif /* JAU_FLOAT_TYPES_HPP_ */
