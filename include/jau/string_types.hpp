/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2025 Gothel Software e.K.
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

#ifndef JAU_STRING_TYPES_HPP_
#define JAU_STRING_TYPES_HPP_

#include <string>
#include <string_view>
#include <type_traits>
#include <jau/cpp_lang_util.hpp>
#include <jau/type_info.hpp>

namespace jau {

    /** \addtogroup Floats
     *
     *  @{
     */

    #if __cplusplus > 201703L
        // C++20
        typedef char8_t                          uchar8_t;
        typedef std::u8string                    u8string;
        typedef std::u8string_view               u8string_view;
    #else
        typedef uint8_t                          uchar8_t;
        typedef std::basic_string<uchar8_t>      u8string;
        typedef std::basic_string_view<uchar8_t> u8string_view;
    #endif
    static_assert(1 == sizeof(uchar8_t));
    // static_assert(std::is_assignable_v<uint8_t, uchar8_t>);
    // static_assert(std::is_same_v<uint8_t, uchar8_t>);
    static_assert(std::is_same_v<unsigned char, uint8_t>);

    typedef std::basic_ostream<uchar8_t> u8ostream;

    namespace string_literals {
        constexpr uchar8_t operator ""_uc8(long double __v)   { return (uchar8_t)__v; }
        // constexpr u8string operator ""_utf8(const uchar8_t* __v, size_t __n) { return u8string(__v, __n); }
    } // float_literals

    class string_ctti {
      public:
        static const jau::type_info& uc8() { return jau::static_ctti<uchar8_t>(); }
    };

    /**@}*/

} // namespace jau

#endif /* JAU_STRING_TYPES_HPP_ */
