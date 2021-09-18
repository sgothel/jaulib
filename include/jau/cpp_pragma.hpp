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

#else
    #define PRAGMA_DISABLE_WARNING_PUSH
    #define PRAGMA_DISABLE_WARNING_POP
    #define PRAGMA_DISABLE_WARNING_UNREFERENCED_FORMAL_PARAMETER
    #define PRAGMA_DISABLE_WARNING_UNREFERENCED_FUNCTION
    #define PRAGMA_DISABLE_WARNING_CPP
    #define PRAGMA_DISABLE_WARNING_MULTICHAR
    #define PRAGMA_DISABLE_WARNING_NULL_DEREFERENCE
    #define PRAGMA_DISABLE_WARNING_FORMAT_OVERFLOW
#endif

} // namespace jau

#endif /* CPP_PRAGMA_HPP_ */
