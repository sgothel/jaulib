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
#include <memory>
#include <type_traits>

#include <jau/cpp_lang_util.hpp>
#include <jau/packed_attribute.hpp>
#include <jau/type_traits_queries.hpp>

#include <jau/int_types.hpp>
#include <jau/int_math.hpp>

namespace jau {

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
    std::string trimCopy(const std::string &s) noexcept;

    /**
    // *************************************************
    // *************************************************
    // *************************************************
     */

    /**
     * Produce a hexadecimal string representation of the given byte values.
     * <p>
     * If lsbFirst is true, orders LSB left -> MSB right, usual for byte streams. Result will not have a leading `0x`.<br>
     * Otherwise orders MSB left -> LSB right, usual for readable integer values. Result will have a leading `0x`.
     * </p>
     * @param bytes pointer to the first byte to print, less offset
     * @param offset offset to bytes pointer to the first byte to print
     * @param length number of bytes to print
     * @param lsbFirst true having the least significant byte printed first (lowest addressed byte to highest),
     *                 otherwise have the most significant byte printed first (highest addressed byte to lowest).
     *                 A leading `0x` will be prepended if `lsbFirst == false`.
     * @param lowerCase true to use lower case hex-chars, otherwise capital letters are being used.
     * @return the hex-string representation of the data
     */
    std::string bytesHexString(const uint8_t * bytes, const nsize_t offset, const nsize_t length,
                               const bool lsbFirst, const bool lowerCase=true) noexcept;

    /**
     * Produce a hexadecimal string representation of the given byte value.
     * @param dest the std::string reference destination to append
     * @param value the byte value to represent
     * @param lowerCase true to use lower case hex-chars, otherwise capital letters are being used.
     * @return the given std::string reference for chaining
     */
    std::string& byteHexString(std::string& dest, const uint8_t value, const bool lowerCase) noexcept;

    /**
     * Produce a lower-case hexadecimal string representation of the given uint8_t values.
     * @param v the value
     * @return the hex-string representation of the value
     */
    inline std::string uint8HexString(const uint8_t v) noexcept {
        return bytesHexString(pointer_cast<const uint8_t*>(&v), 0, sizeof(v), false /* lsbFirst */);
    }

    /**
     * Produce a lower-case hexadecimal string representation of the given uint16_t value.
     * @param v the value
     * @return the hex-string representation of the value
     */
    inline std::string uint16HexString(const uint16_t v) noexcept {
        return bytesHexString(pointer_cast<const uint8_t*>(&v), 0, sizeof(v), false /* lsbFirst */);
    }

    /**
     * Produce a lower-case hexadecimal string representation of the given uint32_t value.
     * @param v the value
     * @return the hex-string representation of the value
     */
    inline std::string uint32HexString(const uint32_t v) noexcept {
        return bytesHexString(pointer_cast<const uint8_t*>(&v), 0, sizeof(v), false /* lsbFirst */);
    }

    /**
     * Produce a lower-case hexadecimal string representation of the given uint64_t value.
     * @param v the value
     * @return the hex-string representation of the value
     */
    inline std::string uint64HexString(const uint64_t& v) noexcept {
        return bytesHexString(pointer_cast<const uint8_t*>(&v), 0, sizeof(v), false /* lsbFirst */);
    }

    /**
     * Produce a lower-case hexadecimal string representation of the given 'void *' value.
     * @param v the value
     * @return the hex-string representation of the value
     */
    inline std::string aptrHexString(const void * v) noexcept {
        return uint64HexString(reinterpret_cast<uint64_t>(v));
    }

    /**
     * Produce a lower-case hexadecimal string representation of the given uint128_t value.
     * @param v the value
     * @return the hex-string representation of the value
     */
    inline std::string uint128HexString(const uint128_t& v) noexcept {
        return bytesHexString(v.data, 0, sizeof(v.data), false /* lsbFirst */);
    }

    /**
     * Produce a lower-case hexadecimal string representation of the given uint256_t value.
     * @param v the value
     * @return the hex-string representation of the value
     */
    inline std::string uint256HexString(const uint256_t& v) noexcept {
        return bytesHexString(v.data, 0, sizeof(v.data), false /* lsbFirst */);
    }

    template<typename T>
    inline
    typename std::enable_if_t<
        std::is_standard_layout_v<T>,
        std::string>
    to_hex_string(T const & v) noexcept
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
    template<class T>
    std::string to_decimal_string(const T& v, const char separator=',', const nsize_t width=0) noexcept {
        const snsize_t v_sign = jau::sign<T>(v);
        const nsize_t digit10_count1 = jau::digits10<T>(v, v_sign, true /* sign_is_digit */);
        const nsize_t digit10_count2 = v_sign < 0 ? digit10_count1 - 1 : digit10_count1; // less sign

        const nsize_t comma_count = 0 == separator ? 0 : ( digit10_count1 - 1 ) / 3;
        const nsize_t net_chars = digit10_count1 + comma_count;
        const nsize_t total_chars = std::max<nsize_t>(width, net_chars);
        std::string res(total_chars, ' ');

        T n = v;
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
     * Produce a decimal string representation of an int32_t value.
     * @param v the value
     * @param separator if not 0, use as separation character, otherwise no separation characters are being used
     * @param width the minimum number of characters to be printed. Add padding with blank space if result is shorter.
     * @return the string representation of the value
     */
    inline std::string int32DecString(const int32_t v, const char separator=',', const nsize_t width=0) noexcept {
        return to_decimal_string<int32_t>(v, separator, width);
    }

    /**
     * Produce a decimal string representation of a uint32_t value.
     * @param v the value
     * @param separator if not 0, use as separation character, otherwise no separation characters are being used
     * @param width the minimum number of characters to be printed. Add padding with blank space if result is shorter.
     * @return the string representation of the value
     */
    inline std::string uint32DecString(const uint32_t v, const char separator=',', const nsize_t width=0) noexcept {
        return to_decimal_string<uint32_t>(v, separator, width);
    }

    /**
     * Produce a decimal string representation of an int64_t value.
     * @param v the value
     * @param separator if not 0, use as separation character, otherwise no separation characters are being used
     * @param width the minimum number of characters to be printed. Add padding with blank space if result is shorter.
     * @return the string representation of the value
     */
    inline std::string int64DecString(const int64_t& v, const char separator=',', const nsize_t width=0) noexcept {
        return to_decimal_string<int64_t>(v, separator, width);
    }

    /**
     * Produce a decimal string representation of a uint64_t value.
     * @param v the value
     * @param separator if not 0, use as separation character, otherwise no separation characters are being used
     * @param width the minimum number of characters to be printed. Add padding with blank space if result is shorter.
     * @return the string representation of the value
     */
    inline std::string uint64DecString(const uint64_t& v, const char separator=',', const nsize_t width=0) noexcept {
        return to_decimal_string<uint64_t>(v, separator, width);
    }

    /**
    // *************************************************
    // *************************************************
    // *************************************************
     */

    template< class value_type,
              std::enable_if_t< std::is_integral_v<value_type> ||
                                std::is_floating_point_v<value_type>,
                                bool> = true>
    std::string to_string(const value_type & ref)
    {
        return std::to_string(ref);
    }
    template< class value_type,
              std::enable_if_t<!std::is_integral_v<value_type> &&
                               !std::is_floating_point_v<value_type> &&
                                std::is_pointer_v<value_type>,
                               bool> = true>
    std::string to_string(const value_type & ref)
    {
        return aptrHexString((void*)ref);
    }

    template< class value_type,
              std::enable_if_t<!std::is_integral_v<value_type> &&
                               !std::is_floating_point_v<value_type> &&
                               !std::is_pointer_v<value_type> &&
                                jau::has_toString_v<value_type>,
                               bool> = true>
    std::string to_string(const value_type & ref) {
        return ref.toString();
    }

    template< class value_type,
              std::enable_if_t<!std::is_integral_v<value_type> &&
                               !std::is_floating_point_v<value_type> &&
                               !std::is_pointer_v<value_type> &&
                               !jau::has_toString_v<value_type> &&
                                jau::has_to_string_v<value_type>,
                               bool> = true>
    std::string to_string(const value_type & ref) {
        return ref.to_string();
    }

    template< class value_type,
              std::enable_if_t<!std::is_integral_v<value_type> &&
                               !std::is_floating_point_v<value_type> &&
                               !std::is_pointer_v<value_type> &&
                               !jau::has_toString_v<value_type> &&
                               !jau::has_to_string_v<value_type> &&
                                jau::has_member_of_pointer_v<value_type>,
                               bool> = true>
    std::string to_string(const value_type & ref) {
        return aptrHexString((void*)ref.operator->());
    }

    template< class value_type,
              std::enable_if_t<!std::is_integral_v<value_type> &&
                               !std::is_floating_point_v<value_type> &&
                               !std::is_pointer_v<value_type> &&
                               !jau::has_toString_v<value_type> &&
                               !jau::has_to_string_v<value_type> &&
                               !jau::has_member_of_pointer_v<value_type>,
                               bool> = true>
    std::string to_string(const value_type & ref) {
        (void)ref;
        return "jau::to_string<T> not available for "+type_cue<value_type>::print("unknown", TypeTraitGroup::ALL);
    }

} // namespace jau

/** \example test_intdecstring01.cpp
 * This C++ unit test validates the jau::to_decimal_string implementation
 */

#endif /* JAU_STRING_UTIL_HPP_ */
