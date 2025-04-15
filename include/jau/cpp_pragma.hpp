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

#ifndef CPP_PRAGMA_HPP_
#define CPP_PRAGMA_HPP_

namespace jau {

    /** \addtogroup CppLang
     *
     *  @{
     */

#if defined(_MSC_VER)
    #define PRAGMA_DISABLE_WARNING_PUSH           __pragma(warning( push ))
    #define PRAGMA_DISABLE_WARNING_POP            __pragma(warning( pop ))
    #define PRAGMA_DISABLE_WARNING(warningNumber) __pragma(warning( disable : warningNumber ))

    #define PRAGMA_DISABLE_WARNING_UNREFERENCED_FORMAL_PARAMETER    PRAGMA_DISABLE_WARNING(4100)
    #define PRAGMA_DISABLE_WARNING_UNREFERENCED_FUNCTION            PRAGMA_DISABLE_WARNING(4505)
    #define PRAGMA_DISABLE_WARNING_CPP
    #define PRAGMA_DISABLE_WARNING_MULTICHAR
    #define PRAGMA_DISABLE_WARNING_NULL_DEREFERENCE
    #define PRAGMA_DISABLE_WARNING_FORMAT_OVERFLOW
    #define PRAGMA_DISABLE_WARNING_FORMAT_NONLITERAL
    #define PRAGMA_DISABLE_WARNING_FORMAT_SECURITY
    #define PRAGMA_DISABLE_WARNING_PMF_CONVERSIONS
    #define PRAGMA_DISABLE_WARNING_STRINGOP_OVERFLOW
    #define PRAGMA_DISABLE_WARNING_INT_OVERFLOW
    #define PRAGMA_DISABLE_WARNING_RESTRICT
    #define PRAGMA_DISABLE_WARNING_PEDANTIC
    #define PRAGMA_DISABLE_WARNING_ZERO_LENGTH_ARRAY

#elif defined(__GNUC__) || defined(__clang__)
    #define DO_PRAGMA(X) _Pragma(#X)
    #define PRAGMA_DISABLE_WARNING_PUSH           DO_PRAGMA(GCC diagnostic push)
    #define PRAGMA_DISABLE_WARNING_POP            DO_PRAGMA(GCC diagnostic pop)
    #define PRAGMA_DISABLE_WARNING(warningName)   DO_PRAGMA(GCC diagnostic ignored #warningName)
    #define PRAGMA_WARNING_ONLY(warningName)      DO_PRAGMA(GCC diagnostic warning #warningName)

    #define PRAGMA_DISABLE_WARNING_UNREFERENCED_FORMAL_PARAMETER    PRAGMA_DISABLE_WARNING(-Wunused-parameter)
    #define PRAGMA_DISABLE_WARNING_UNREFERENCED_FUNCTION            PRAGMA_DISABLE_WARNING(-Wunused-function)
    #define PRAGMA_DISABLE_WARNING_CPP                              PRAGMA_DISABLE_WARNING(-Wcpp)
    #define PRAGMA_DISABLE_WARNING_MULTICHAR                        PRAGMA_DISABLE_WARNING(-Wmultichar)
    #define PRAGMA_DISABLE_WARNING_NULL_DEREFERENCE                 PRAGMA_DISABLE_WARNING(-Wnull-dereference)
    #define PRAGMA_DISABLE_WARNING_FORMAT_OVERFLOW                  PRAGMA_DISABLE_WARNING(-Wformat-overflow)
    #define PRAGMA_DISABLE_WARNING_FORMAT_NONLITERAL                PRAGMA_DISABLE_WARNING(-Wformat-nonliteral)
    #define PRAGMA_DISABLE_WARNING_FORMAT_SECURITY                  PRAGMA_DISABLE_WARNING(-Wformat-security)
    #if defined(__GNUC__) && !defined(__clang__)
        #define PRAGMA_DISABLE_WARNING_PMF_CONVERSIONS              PRAGMA_DISABLE_WARNING(-Wpmf-conversions)
        #define PRAGMA_DISABLE_WARNING_STRINGOP_OVERFLOW            PRAGMA_DISABLE_WARNING(-Wstringop-overflow)
        #define PRAGMA_DISABLE_WARNING_INT_OVERFLOW
        #define PRAGMA_DISABLE_WARNING_RESTRICT                     PRAGMA_DISABLE_WARNING(-Wrestrict)
        #define PRAGMA_DISABLE_WARNING_PEDANTIC                     PRAGMA_DISABLE_WARNING(-Wpedantic)
        #define PRAGMA_DISABLE_WARNING_ZERO_LENGTH_ARRAY
    #else
        #define PRAGMA_DISABLE_WARNING_PMF_CONVERSIONS
        #define PRAGMA_DISABLE_WARNING_STRINGOP_OVERFLOW
        #define PRAGMA_DISABLE_WARNING_INT_OVERFLOW                 PRAGMA_DISABLE_WARNING(-Winteger-overflow)
        #define PRAGMA_DISABLE_WARNING_RESTRICT
        #define PRAGMA_DISABLE_WARNING_PEDANTIC                     PRAGMA_DISABLE_WARNING(-Wpedantic)
        #define PRAGMA_DISABLE_WARNING_ZERO_LENGTH_ARRAY            PRAGMA_DISABLE_WARNING(-Wzero-length-array)
    #endif

#else
    #define PRAGMA_DISABLE_WARNING_PUSH
    #define PRAGMA_DISABLE_WARNING_POP
    #define PRAGMA_DISABLE_WARNING_UNREFERENCED_FORMAL_PARAMETER
    #define PRAGMA_DISABLE_WARNING_UNREFERENCED_FUNCTION
    #define PRAGMA_DISABLE_WARNING_CPP
    #define PRAGMA_DISABLE_WARNING_MULTICHAR
    #define PRAGMA_DISABLE_WARNING_NULL_DEREFERENCE
    #define PRAGMA_DISABLE_WARNING_FORMAT_OVERFLOW
    #define PRAGMA_DISABLE_WARNING_FORMAT_NONLITERAL
    #define PRAGMA_DISABLE_WARNING_FORMAT_SECURITY
    #define PRAGMA_DISABLE_WARNING_PMF_CONVERSIONS
    #define PRAGMA_DISABLE_WARNING_STRINGOP_OVERFLOW
    #define PRAGMA_DISABLE_WARNING_INT_OVERFLOW
    #define PRAGMA_DISABLE_WARNING_RESTRICT
    #define PRAGMA_DISABLE_WARNING_PEDANTIC
    #define PRAGMA_DISABLE_WARNING_ZERO_LENGTH_ARRAY

#endif

    /**@}*/

} // namespace jau

#endif /* CPP_PRAGMA_HPP_ */
