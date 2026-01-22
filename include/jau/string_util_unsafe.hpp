/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021-2026 Gothel Software e.K.
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

#ifndef JAU_STRING_UTIL_UNSAFE_HPP_
#define JAU_STRING_UTIL_UNSAFE_HPP_

#include <cstdarg>
#include <cstring>
#include <string>

namespace jau::unsafe {
    /** \addtogroup StringUtils
     *
     *  @{
     */

    /**
     * Returns a (potentially truncated) string according to `snprintf()` formatting rules
     * and variable number of arguments following the `format` argument
     * while utilizing the unsafe `vsnprintf`.
     *
     * This variant doesn't validate `format` against given arguments, see jau::format_string_n.
     *
     * Resulting string is truncated to `min(maxStrLen, formatLen)`,
     * with `formatLen` being the given formatted string length of output w/o limitation.
     *
     * @param maxStrLen maximum resulting string length
     * @param format `printf()` compliant format string
     * @param args optional arguments matching the format string
     */
    std::string format_string_n(const std::size_t maxStrLen, const char* format, ...) noexcept;
    std::string vformat_string_n(const std::size_t maxStrLen, const char* format, va_list args) noexcept;

    /**
     * Returns a (non-truncated) string according to `snprintf()` formatting rules
     * and variable number of arguments following the `format` argument
     * while utilizing the unsafe `vsnprintf`.
     *
     * This variant doesn't validate `format` against given arguments, see jau::format_string_h.
     *
     * Resulting string size matches formated output w/o limitation.
     *
     * @param strLenHint initially used string length w/o EOS
     * @param format `printf()` compliant format string
     * @param args optional arguments matching the format string
     */
    std::string format_string_h(const std::size_t strLenHint, const char* format, ...) noexcept;
    std::string vformat_string_h(const std::size_t strLenHint, const char* format, va_list args) noexcept;

    /**
     * Returns a (non-truncated) string according to `snprintf()` formatting rules
     * and variable number of arguments following the `format` argument
     * while utilizing the unsafe `vsnprintf`.
     *
     * This variant doesn't validate `format` against given arguments, see jau::format_string.
     *
     * Resulting string size matches formated output w/o limitation.
     *
     * @param format `printf()` compliant format string
     * @param args optional arguments matching the format string
     */
    std::string format_string(const char* format, ...) noexcept;

    void errPrint(FILE *out, const char *msg, bool addErrno, bool addBacktrace, const char *func, const char *file, const int line,
                  const char* format, ...) noexcept;

    /**@}*/

}  // namespace jau::unsafe

#endif // JAU_STRING_UTIL_UNSAFE_HPP_
