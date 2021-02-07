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

    /**
     * <code>constexpr</code> enabled for C++20.
     * <p>
     * The alternative qualifier used is `inline`,
     * as it is implied for for `constexpr` used for functions.
     * </p>
     */
#if __cplusplus > 201703L
    #define constexpr_func_cxx20 constexpr
#else
    #define constexpr_func_cxx20 inline
#endif

    /**
     * Used when designed to declare a function <code>constexpr</code>,
     * but prohibited by its specific implementation.
     * <p>
     * The alternative qualifier used is `inline`,
     * as it is implied for for `constexpr` used for functions.
     * </p>
     * <p>
     * Here it but uses non-literal variables, such as std::lock_guard etc.
     * As these can't be evaluated at compile time, the standard does
     * not allow using <code>constexpr</code> here.
     * </p>
     * <p>
     * Empty until standard defines otherwise.
     * </p>
     */
    #define constexpr_fun_non_literal_var inline

    /**
     * Used when designed to declare a function <code>constexpr</code>,
     * but prohibited by its specific implementation.
     * <p>
     * The alternative qualifier used is `inline`,
     * as it is implied for for `constexpr` used for functions.
     * </p>
     * <p>
     * Here it uses thread-safety related measures like atomic storage
     * or mutex locks, which are non-literal variables and hence
     * prohibit the use of <code>constexpr</code>.
     * </p>
     * @see constexpr_non_literal_var
     */
    #define constexpr_func_atomic inline

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
