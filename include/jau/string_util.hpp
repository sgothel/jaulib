/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021 Gothel Software e.K.
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

#ifndef JAU_STRING_UTIL_HPP_
#define JAU_STRING_UTIL_HPP_

#include <cstdint>
#include <cstring>
#include <string>
#include <cstdarg>
#include <memory>
#include <type_traits>
#include <vector>

#include <jau/cpp_lang_util.hpp>
#include <jau/packed_attribute.hpp>
#include <jau/type_traits_queries.hpp>

#include <jau/int_types.hpp>
#include <jau/int_math.hpp>

namespace jau {

    /** @defgroup StringUtils String Utilities
     *  String utilities for type conversion and manipulation.
     *
     *  @{
     */

    /**
     * Returns a C++ String taken from buffer with maximum length of min(max_len, max_len).
     * <p>
     * The maximum length only delimits the string length and does not contain the EOS null byte.
     * An EOS null byte will will be added.
     * </p>
     * <p>
     * The source string within buffer is not required to contain an EOS null byte;
     * </p>
     */
    std::string get_string(const uint8_t *buffer, nsize_t const buffer_len, nsize_t const max_len) noexcept;

    /** trim in place */
    void trimInPlace(std::string &s) noexcept;

    /** trim copy */
    std::string trim(const std::string &s) noexcept;

    /** Split given string `str` at `separator` into the resulting std::vector excluding the separator sequence . */
    std::vector<std::string> split_string(const std::string& str, const std::string& separator) noexcept;

    std::string& toLowerInPlace(std::string& s) noexcept;

    std::string toLower(const std::string& s) noexcept;

    /**
    // *************************************************
    // *************************************************
    // *************************************************
     */

    /**
     * Converts a given hexadecimal string representation into a byte vector.
     *
     * In case a non valid hexadecimal digit appears in the given string,
     * conversion ends and fills the byte vector up until the violation.
     *
     * @param out the byte vector sink
     * @param hexstr the hexadecimal string representation
     * @param lsbFirst low significant byte first
     * @param checkLeading0x if true, checks for a leading `0x` and removes it, otherwise not.
     * @return the length of the matching byte vector
     */
    size_t hexStringBytes(std::vector<uint8_t>& out, const std::string& hexstr, const bool lsbFirst, const bool checkLeading0x) noexcept;

    /** See hexStringBytes() */
    size_t hexStringBytes(std::vector<uint8_t>& out, const uint8_t hexstr[], const size_t hexstr_len, const bool lsbFirst, const bool checkLeading0x) noexcept;

    /**
     * Produce a hexadecimal string representation of the given byte values.
     * <p>
     * If lsbFirst is true, orders LSB left -> MSB right, usual for byte streams. Result will not have a leading `0x`.<br>
     * Otherwise orders MSB left -> LSB right, usual for readable integer values. Result will have a leading `0x`.
     * </p>
     * @param data pointer to the first byte to print, less offset
     * @param offset offset to bytes pointer to the first byte to print
     * @param length number of bytes to print
     * @param lsbFirst true having the least significant byte printed first (lowest addressed byte to highest),
     *                 otherwise have the most significant byte printed first (highest addressed byte to lowest).
     *                 A leading `0x` will be prepended if `lsbFirst == false`.
     * @param lowerCase true to use lower case hex-chars, otherwise capital letters are being used.
     * @return the hex-string representation of the data
     */
    std::string bytesHexString(const void* data, const nsize_t offset, const nsize_t length,
                               const bool lsbFirst, const bool lowerCase=true) noexcept;

    template< class uint8_container_type,
              std::enable_if_t<std::is_integral_v<typename uint8_container_type::value_type> &&
                               std::is_convertible_v<typename uint8_container_type::value_type, uint8_t>,
                               bool> = true>
    std::string bytesHexString(const uint8_container_type& bytes,
                               const bool lsbFirst, const bool lowerCase=true) noexcept {
            return bytesHexString((const uint8_t *)bytes.data(), 0, bytes.size(), lsbFirst, lowerCase);
    }

    /**
     * Produce a hexadecimal string representation of the given byte value.
     * @param dest the std::string reference destination to append
     * @param value the byte value to represent
     * @param lowerCase true to use lower case hex-chars, otherwise capital letters are being used.
     * @return the given std::string reference for chaining
     */
    std::string& byteHexString(std::string& dest, const uint8_t value, const bool lowerCase) noexcept;

    /**
     * Produce a lower-case hexadecimal string representation of the given pointer.
     * @tparam value_type a pointer type
     * @param v the pointer of given pointer type
     * @return the hex-string representation of the value
     * @see bytesHexString()
     */
    template< class value_type,
              std::enable_if_t<std::is_pointer_v<value_type>,
                               bool> = true>
    inline std::string to_hexstring(value_type const & v) noexcept
    {
        const uintptr_t v2 = reinterpret_cast<uintptr_t>(v);
        return bytesHexString(pointer_cast<const uint8_t*>(&v2), 0, sizeof(v), false /* lsbFirst */);
    }

    /**
     * Produce a lower-case hexadecimal string representation of the given value with standard layout.
     * @tparam value_type a standard layout value type
     * @param v the value of given standard layout type
     * @return the hex-string representation of the value
     * @see bytesHexString()
     */
    template< class value_type,
              std::enable_if_t<!std::is_pointer_v<value_type> &&
                               std::is_standard_layout_v<value_type>,
                               bool> = true>
    inline std::string to_hexstring(value_type const & v) noexcept
    {
        return bytesHexString(pointer_cast<const uint8_t*>(&v), 0, sizeof(v), false /* lsbFirst */);
    }

    /**
    // *************************************************
    // *************************************************
    // *************************************************
     */

    /**
     * Produce a decimal string representation of an integral integer value.
     * @tparam T an integral integer type
     * @param v the integral integer value
     * @param separator if not 0, use as separation character, otherwise no separation characters are being used
     * @param width the minimum number of characters to be printed. Add padding with blank space if result is shorter.
     * @return the string representation of the integral integer value
     */
    template< class value_type,
              std::enable_if_t< std::is_integral_v<value_type>,
                                bool> = true>
    std::string to_decstring(const value_type& v, const char separator=',', const nsize_t width=0) noexcept {
        const snsize_t v_sign = jau::sign<value_type>(v);
        const nsize_t digit10_count1 = jau::digits10<value_type>(v, v_sign, true /* sign_is_digit */);
        const nsize_t digit10_count2 = v_sign < 0 ? digit10_count1 - 1 : digit10_count1; // less sign

        const nsize_t comma_count = 0 == separator ? 0 : ( digit10_count1 - 1 ) / 3;
        const nsize_t net_chars = digit10_count1 + comma_count;
        const nsize_t total_chars = std::max<nsize_t>(width, net_chars);
        std::string res(total_chars, ' ');

        value_type n = v;
        nsize_t char_iter = 0;

        for(nsize_t digit10_iter = 0; digit10_iter < digit10_count2 /* && char_iter < total_chars */; digit10_iter++ ) {
            const int digit = v_sign < 0 ? invert_sign( n % 10 ) : n % 10;
            n /= 10;
            if( 0 < digit10_iter && 0 == digit10_iter % 3 ) {
                res[total_chars-1-(char_iter++)] = separator;
            }
            res[total_chars-1-(char_iter++)] = '0' + digit;
        }
        if( v_sign < 0 /* && char_iter < total_chars */ ) {
            res[total_chars-1-(char_iter++)] = '-';
        }
        return res;
    }

    /**
    // *************************************************
    // *************************************************
    // *************************************************
     */

    /**
     * Returns a string according to `vprintf()` formatting rules
     * using `va_list` instead of a variable number of arguments.
     * @param format `printf()` compliant format string
     * @param ap `va_list` arguments
     */
    std::string vformat_string(const char* format, va_list ap) noexcept;

    /**
     * Returns a string according to `printf()` formatting rules
     * and variable number of arguments following the `format` argument.
     * @param format `printf()` compliant format string
     */
    std::string format_string(const char* format, ...) noexcept;

    /**
    // *************************************************
    // *************************************************
    // *************************************************
     */

    template< class value_type,
              std::enable_if_t< std::is_integral_v<value_type> ||
                                std::is_floating_point_v<value_type>,
                                bool> = true>
    inline std::string to_string(const value_type & ref)
    {
        return std::to_string(ref);
    }

    template< class value_type,
              std::enable_if_t<!std::is_integral_v<value_type> &&
                               !std::is_floating_point_v<value_type> &&
                                std::is_base_of_v<std::string, value_type>,
                               bool> = true>
    inline std::string to_string(const value_type & ref) {
        return ref;
    }

    template< class value_type,
              std::enable_if_t<!std::is_integral_v<value_type> &&
                               !std::is_floating_point_v<value_type> &&
                               !std::is_base_of_v<std::string, value_type> &&
                                std::is_base_of_v<std::string_view, value_type>,
                               bool> = true>
    inline std::string to_string(const value_type & ref) {
        return std::string(ref);
    }

    template< class value_type,
              std::enable_if_t<!std::is_integral_v<value_type> &&
                               !std::is_floating_point_v<value_type> &&
                               !std::is_base_of_v<std::string, value_type> &&
                               !std::is_base_of_v<std::string_view, value_type> &&
                                std::is_pointer_v<value_type>,
                               bool> = true>
    inline std::string to_string(const value_type & ref)
    {
        return to_hexstring((void*)ref);
    }

    template< class value_type,
              std::enable_if_t<!std::is_integral_v<value_type> &&
                               !std::is_floating_point_v<value_type> &&
                               !std::is_base_of_v<std::string, value_type> &&
                               !std::is_base_of_v<std::string_view, value_type> &&
                               !std::is_pointer_v<value_type> &&
                                jau::has_toString_v<value_type>,
                               bool> = true>
    inline std::string to_string(const value_type & ref) {
        return ref.toString();
    }

    template< class value_type,
              std::enable_if_t<!std::is_integral_v<value_type> &&
                               !std::is_floating_point_v<value_type> &&
                               !std::is_base_of_v<std::string, value_type> &&
                               !std::is_base_of_v<std::string_view, value_type> &&
                               !std::is_pointer_v<value_type> &&
                               !jau::has_toString_v<value_type> &&
                                jau::has_to_string_v<value_type>,
                               bool> = true>
    inline std::string to_string(const value_type & ref) {
        return ref.to_string();
    }

    template< class value_type,
              std::enable_if_t<!std::is_integral_v<value_type> &&
                               !std::is_floating_point_v<value_type> &&
                               !std::is_base_of_v<std::string, value_type> &&
                               !std::is_base_of_v<std::string_view, value_type> &&
                               !std::is_pointer_v<value_type> &&
                               !jau::has_toString_v<value_type> &&
                               !jau::has_to_string_v<value_type> &&
                                jau::has_member_of_pointer_v<value_type>,
                               bool> = true>
    inline std::string to_string(const value_type & ref) {
        return to_hexstring((void*)ref.operator->());
    }

    template< class value_type,
              std::enable_if_t<!std::is_integral_v<value_type> &&
                               !std::is_floating_point_v<value_type> &&
                               !std::is_base_of_v<std::string, value_type> &&
                               !std::is_base_of_v<std::string_view, value_type> &&
                               !std::is_pointer_v<value_type> &&
                               !jau::has_toString_v<value_type> &&
                               !jau::has_to_string_v<value_type> &&
                               !jau::has_member_of_pointer_v<value_type>,
                               bool> = true>
    inline std::string to_string(const value_type & ref) {
        (void)ref;
        return "jau::to_string<T> not available for "+type_cue<value_type>::print("unknown", TypeTraitGroup::ALL);
    }

    template<typename T>
    std::string to_string(std::vector<T> const &list, const std::string& delim)
    {
        if ( list.empty() ) {
            return std::string();
        }
        bool need_delim = false;
        std::string res;
        for(const T& e : list) {
            if( need_delim ) {
                res.append( delim );
            }
            res.append( to_string( e ) );
            need_delim = true;
        }
        return res;
    }
    template<typename T>
    std::string to_string(std::vector<T> const &list) { return to_string<T>(list, ", "); }

    bool to_integer(long long & result, const std::string& str, const char limiter='\0', const char *limiter_pos=nullptr);
    bool to_integer(long long & result, const char * str, size_t str_len, const char limiter='\0', const char *limiter_pos=nullptr);

    /**@}*/

} // namespace jau

/** \example test_intdecstring01.cpp
 * This C++ unit test validates the jau::to_decstring implementation
 */

#endif /* JAU_STRING_UTIL_HPP_ */
