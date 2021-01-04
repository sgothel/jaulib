/*
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
 *
 */

#ifndef CPP_LANG_MACROS_HPP_
#define CPP_LANG_MACROS_HPP_

#if __cplusplus > 201703L
    #define __constexpr_cxx20_ constexpr
    #warning CXX 20 detected but not evaluated yet
#else
    #define __constexpr_cxx20_
#endif

    /**
     * Used when attempting to define a function 'constexpr',
     * but uses non-literal variables, such as std::lock_guard etc.
     * As these can't be evaluated at compile time, the standard does
     * not allow using 'constexpr' here.
     * <p>
     * Empty until standard defines otherwise.
     * </p>
     */
    #define __constexpr_non_literal_var__

    /**
     * See __constexpr_non_literal_var__
     */
    #define __constexpr_non_literal_atomic__

    /**
     * See __constexpr_non_literal_var__
     */
    #define __constexpr_non_literal_mutex__

    /**
     * Set define if RTTI is enabled during compilation,
     * implying its runtime availability.
     * <pre>
     * - clang ('__clang__') may have '__has_feature(cxx_rtti)'
     * - g++   ('__GNUC__')  may have '__GXX_RTTI'
     * - msvc  (_MSC_VER)    may have: '_CPPRTTI'
     * </pre>
     */
    #if defined(__clang__)
        #if __has_feature(cxx_rtti)
            #define __cxx_rtti_available__ 1
        #endif
    #else
        #if defined(__GXX_RTTI) || defined(_CPPRTTI)
            #define __cxx_rtti_available__ 1
        #endif
    #endif

#endif /* CPP_LANG_MACROS_HPP_ */
