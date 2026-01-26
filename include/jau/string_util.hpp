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

#ifndef JAU_STRING_UTIL_HPP_
#define JAU_STRING_UTIL_HPP_

#include <algorithm>
#include <concepts>
#include <cstdarg>
#include <cstdint>
#include <cstring>
#include <exception>
#include <limits>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include <jau/string_util_unsafe.hpp>
#include "jau/type_info.hpp"

#include <jau/byte_util.hpp>
#include <jau/cpp_lang_util.hpp>
#include <jau/exceptions.hpp>
#include <jau/packed_attribute.hpp>

#include <jau/int_math.hpp>
#include <jau/int_types.hpp>
#include <jau/string_literal.hpp>

#include <jau/type_traits_queries.hpp>
#include <jau/type_concepts.hpp>

namespace jau {

    /** @defgroup StringUtils String Utilities
     *  String utilities for type conversion and manipulation.
     *
     *  @{
     */

    inline bool is_ascii_code(int c) noexcept {
        return 0 != std::iscntrl(c) || 0 != std::isprint(c);
    }

    /**
     * Returns true if given char `c` is one of the following whitespace character:
     * - space (0x20, ' ')
     * - form feed (0x0c, '\f')
     * - line feed (0x0a, '\n')
     * - carriage return (0x0d, '\r')
     * - horizontal tab (0x09, '\t')
     * - vertical tab (0x0b, '\v')
     */
    constexpr bool is_space(const char c) noexcept {
        switch(c) {
            case ' ': return true;
            case '\f': return true;
            case '\n': return true;
            case '\r': return true;
            case '\t': return true;
            case '\v': return true;
            default: return false;
        }
    }

    /// Returns true if given char `c` matches the char symbol range with the radix 16, 10 (default), 8 or 2
    constexpr bool is_digit(const char c, const uint32_t radix=10, const char separator = 0) noexcept {
        if( separator && separator == c ) {
            return true;
        }
        switch (radix) {
            case 16:
                return ('0' <= c && c <= '9')
                    || ('A' <= c && c <= 'F')
                    || ('a' <= c && c <= 'f');
            case 10:
                return '0' <= c && c <= '9';
            case 8:
                return '0' <= c && c <= '7';
            case 2:
                return '0' <= c && c <= '1';
            default:
                break;
        }
        return false;
    }
    /// Returns digit value of given char `c` matching the radix 16, 10, 8 or 2, `-1` on no match
    constexpr int32_t digit(const uint8_t c, const uint32_t radix=10) noexcept {
        switch (radix) {
            case 16:
                if ('0' <= c && c <= '9') {
                    return c - '0';
                }
                if ('A' <= c && c <= 'F') {
                    return c - 'A' + 10;
                }
                if ('a' <= c && c <= 'f') {
                    return c - 'a' + 10;
                }
                break;
            case 10:
                if ('0' <= c && c <= '9') {
                    return c - '0';
                }
                break;
            case 8:
                if ('0' <= c && c <= '7') {
                    return c - '0';
                }
                break;
            case 2:
                if ('0' <= c && c <= '1') {
                    return c - '0';
                }
                break;
            default:
                break;
        }
        return -1;
    }
    constexpr int32_t hexDigit(const uint8_t c) noexcept {
        return digit(c, 16);
    }

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
    std::string get_string(const uint8_t *buffer, nsize_t const buffer_len, nsize_t const max_len);

    /** trim in place */
    void trimInPlace(std::string &s) noexcept;

    /** trim copy */
    std::string trim(const std::string &s);

    /** Split given string `str` at `separator` into the resulting std::vector excluding the separator sequence . */
    std::vector<std::string> split_string(const std::string &str, const std::string &separator);

    std::string &toLowerInPlace(std::string &s) noexcept;

    std::string toLower(const std::string &s);

    /**
    // *************************************************
    // *************************************************
    // *************************************************
     */

    enum class LoUpCase : bool {
        lower = false,
        upper = true
    };

    enum class PrefixOpt : bool {
        none = false,
        prefix = true
    };

    /**
     * Converts a given hexadecimal string representation, appending to a byte vector (lsb-first).
     *
     * In case a non valid hexadecimal digit appears in the given string,
     * conversion ends and fills the byte vector up until the violation.
     *
     * In case hexstr contains an odd number of hex-nibbles, it will be interpreted as follows
     * - 0xf[12] = 0x0f12 = { 0x12, 0x0f } - msb, 1st single low-nibble is most significant
     * -   [12]f = 0xf012 = { 0x12, 0xf0 } - lsb, last single high-nibble is most significant
     *
     * Even if complete==false, result holds the partial value if consumed_chars>0.
     *
     * You may use C++17 structured bindings to handle the pair.
     *
     * @param out the byte vector sink to append, lsb-first
     * @param hexstr the hexadecimal string representation
     * @param hexstr_len length of hextstr
     * @param byteOrder lb_endian_t::big for big-endian bytes in `hexstr` (default)
     * @param checkPrefix if True, checks for a leading `0x` and removes it, otherwise not.
     * @return pair [size_t consumed_chars, bool complete], i.e. consumed characters of string and completed=false if not fully consumed.
     */
    SizeBoolPair fromHexString(std::vector<uint8_t> &out, const uint8_t hexstr[], const size_t hexstr_len,
                               const lb_endian_t byteOrder = lb_endian_t::big, const Bool checkPrefix = Bool::True);

    /** See hexStringBytes() */
    inline SizeBoolPair fromHexString(std::vector<uint8_t> &out, const std::string_view hexstr,
                                      const lb_endian_t byteOrder = lb_endian_t::big, const Bool checkPrefix = Bool::True) {
        return jau::fromHexString(out, cast_char_ptr_to_uint8(hexstr.data()), hexstr.length(), byteOrder, checkPrefix); // NOLINT(bugprone-suspicious-stringview-data-usage)
    }

    /**
     * Converts a given hexadecimal string representation, storing into a byte array(lsb-first).
     *
     * In case a non valid hexadecimal digit appears in the given string,
     * conversion ends and fills the byte vector up until the violation.
     *
     * In case hexstr contains an odd number of hex-nibbles, it will be interpreted as follows
     * - 0xf[12] = 0x0f12 = { 0x12, 0x0f } - msb, 1st single low-nibble is most significant
     * -   [12]f = 0xf012 = { 0x12, 0xf0 } - lsb, last single high-nibble is most significant
     *
     * Even if complete==false, result holds the partial value if consumed_chars>0.
     *
     * You may use C++17 structured bindings to handle the triple.
     *
     * @param out byte array pointer to store result, lsb-first
     * @param out_len size of byte array
     * @param hexstr the hexadecimal string representation
     * @param hexstr_len length of hextstr
     * @param byteOrder lb_endian_t::big for big-endian bytes in `hexstr` (default)
     * @param checkPrefix if True, checks for a leading `0x` and removes it, otherwise not.
     * @return triple [uint8_t* out_end, size_t consumed_chars, bool complete],
     *         i.e. end pointer of out (last write + 1), consumed characters of string and completed=false if not fully consumed.
     */
    UInt8PtrSizeBoolPair fromHexString(uint8_t *out, size_t out_len, const uint8_t hexstr[], const size_t hexstr_len,
                                          const lb_endian_t byteOrder = lb_endian_t::big, const Bool checkPrefix = Bool::True) noexcept;

    /** See hexStringBytes() */
    inline UInt8PtrSizeBoolPair fromHexString(uint8_t *out, size_t out_len, const std::string_view hexstr,
                                      const lb_endian_t byteOrder = lb_endian_t::big, const Bool checkPrefix = Bool::True) noexcept {
        return jau::fromHexString(out, out_len, cast_char_ptr_to_uint8(hexstr.data()), hexstr.length(), byteOrder, checkPrefix); // NOLINT(bugprone-suspicious-stringview-data-usage)
    }

    /**
     * Converts a given hexadecimal string representation appending to a uint64_t value according to hexStringBytes().
     *
     * Even if complete==false, result holds the partial value if consumed_chars>0.
     *
     * You may use C++17 structured bindings to handle the tuple.
     *
     * @param hexstr the hexadecimal string representation
     * @param byteOrder lb_endian_t::big for big-endian bytes in `hexstr` (default)
     * @param checkPrefix if True, checks for a leading `0x` and removes it, otherwise not.
     * @return tuple [uint64_t result, size_t consumed_chars, bool complete], i.e. consumed characters of string and completed=false if not fully consumed.
     * @see hexStringBytes()
     * @see to_hexstring()
     */
    UInt64SizeBoolTuple fromHexString(std::string_view const hexstr, const lb_endian_t byteOrder = lb_endian_t::big,
                                      const Bool checkPrefix = Bool::True) noexcept;

    inline constexpr const char *HexadecimalArrayLow = "0123456789abcdef";
    inline constexpr const char *HexadecimalArrayBig = "0123456789ABCDEF";

    /**
     * Appends a hexadecimal string representation of the given lsb-first byte values.
     *
     * If byteOrder is lb_endian_t::little, orders lsb-byte left, usual for byte streams. Result will not have a leading `0x`.
     * Otherwise, lb_endian_t::big (default),  orders msb-byte left for integer values. Result will have a leading `0x` if !skipPrefix.
     *
     * @param dest the std::string to append to
     * @param data pointer to the first byte to print, lsb-first
     * @param length number of bytes to print
     * @param byteOrder lb_endian_t::big for big-endian bytes in resulting hex-string (default).
     *                  A leading `0x` will be prepended if `byteOrder == lb_endian_t::big` and `PrefixOpt::prefix` given.
     * @param capitalization LoUpCase capitalization, default is LoUpCase::lower
     * @param prefix pass PrefixOpt::prefix (default) to add leading `0x` if `byteOrder == lb_endian_t::big` (default)
     * @return the given string buffer for concatenation
     */
    std::string& appendHexString(std::string& dest, const void *data, const nsize_t length,
                                 const lb_endian_t byteOrder = lb_endian_t::big, const LoUpCase capitalization = LoUpCase::lower,
                                 const PrefixOpt prefix = PrefixOpt::prefix) noexcept;

    /**
     * Produce a hexadecimal string representation of the given lsb-first byte values.
     *
     * If byteOrder is lb_endian_t::little, orders lsb-byte left, usual for byte streams. Result will not have a leading `0x`.
     * Otherwise, lb_endian_t::big (default),  orders msb-byte left for integer values. Result will have a leading `0x` if !skipPrefix.
     *
     * @param data pointer to the first byte to print, lsb-first
     * @param length number of bytes to print
     * @param byteOrder lb_endian_t::big for big-endian bytes in resulting hex-string (default).
     *                  A leading `0x` will be prepended if `byteOrder == lb_endian_t::big` and `PrefixOpt::prefix` given.
     * @param capitalization LoUpCase capitalization, default is LoUpCase::lower
     * @param prefix pass PrefixOpt::prefix (default) to add leading `0x` if `byteOrder == lb_endian_t::big` (default)
     * @return the hex-string representation of the data
     */
    inline std::string toHexString(const void *data, const nsize_t length,
                                   const lb_endian_t byteOrder = lb_endian_t::big, const LoUpCase capitalization = LoUpCase::lower,
                                   const PrefixOpt prefix = PrefixOpt::prefix) noexcept {
        std::string s;
        appendHexString(s, data, length, byteOrder, capitalization, prefix);
        return s;
    }

    /**
     * Produce a hexadecimal string representation of the given byte value and appends it to the given string
     * @param dest the std::string reference destination to append
     * @param value the byte value to represent
     * @param capitalization LoUpCase capitalization, default is LoUpCase::lower
     * @return the given std::string reference for chaining
     */
    std::string &appendHexString(std::string &dest, const uint8_t value, const LoUpCase capitalization = LoUpCase::lower) noexcept;

    /**
     * Produce a lower-case hexadecimal string representation with leading `0x` in MSB of the given pointer.
     * @tparam value_type a pointer type
     * @param v the pointer of given pointer type
     * @param byteOrder lb_endian_t::big for big-endian bytes in resulting hex-string (default).
     *                  A leading `0x` will be prepended if `byteOrder == lb_endian_t::big` and `PrefixOpt::prefix` given.
     * @param capitalization LoUpCase capitalization, default is LoUpCase::lower
     * @param prefix pass PrefixOpt::prefix (default) to add leading `0x` if `byteOrder == lb_endian_t::big` (default)
     * @return the hex-string representation of the value
     * @see bytesHexString()
     * @see from_hexstring()
     */
    template<class value_type>
        requires jau::req::pointer<value_type> &&
                 (!jau::req::container<value_type>)
    inline std::string toHexString(value_type const &v, const lb_endian_t byteOrder = lb_endian_t::big,
                                   const LoUpCase capitalization = LoUpCase::lower,
                                   const PrefixOpt prefix = PrefixOpt::prefix) noexcept
    {
#if defined(__EMSCRIPTEN__)                 // jau::os::is_generic_wasm()
        static_assert(is_little_endian());  // Bug in emscripten, unable to deduce uint16_t, uint32_t or uint64_t override of cpu_to_le() or bswap()
        const uintptr_t v_le = reinterpret_cast<uintptr_t>(v);
        return toHexString(pointer_cast<const uint8_t *>(&v_le), sizeof(v),  // NOLINT(bugprone-sizeof-expression): Intended
                           byteOrder, capitalization, prefix);
#else
        const uintptr_t v_le = jau::cpu_to_le(reinterpret_cast<uintptr_t>(v));
        return toHexString(pointer_cast<const uint8_t *>(&v_le), sizeof(v),  // NOLINT(bugprone-sizeof-expression): Intended
                           byteOrder, capitalization, prefix);
#endif
    }

    /**
     * Produce a lower-case hexadecimal string representation with leading `0x` in MSB of the given uint8_t continuous container values.
     * @tparam uint8_container_type a uint8_t continuous container type
     * @param bytes the value of given uint8_t continuous container type
     * @param byteOrder lb_endian_t::big for big-endian bytes in resulting hex-string (default).
     *                  A leading `0x` will be prepended if `byteOrder == lb_endian_t::big` and `PrefixOpt::prefix` given.
     * @param capitalization LoUpCase capitalization, default is LoUpCase::lower
     * @param prefix pass PrefixOpt::prefix (default) to add leading `0x` if `byteOrder == lb_endian_t::big` (default)
     * @return the hex-string representation of the value
     * @see bytesHexString()
     * @see from_hexstring()
     */
    template<class uint8_container_type>
        requires jau::req::contiguous_container<uint8_container_type> &&
                 std::convertible_to<typename uint8_container_type::value_type, uint8_t>
    inline std::string toHexString(const uint8_container_type &bytes,
                                   const lb_endian_t byteOrder = lb_endian_t::big, const LoUpCase capitalization = LoUpCase::lower,
                                   const PrefixOpt skipPrefix = PrefixOpt::prefix) noexcept
    {
        return toHexString((const uint8_t *)bytes.data(), bytes.size(), byteOrder, capitalization, skipPrefix);
    }

    /**
     * Produce a lower-case hexadecimal string representation with leading `0x` in MSB of the given value with standard layout.
     * @tparam value_type a standard layout value type
     * @param v the value of given standard layout type
     * @param byteOrder lb_endian_t::big for big-endian bytes in resulting hex-string (default).
     *                  A leading `0x` will be prepended if `byteOrder == lb_endian_t::big` and `PrefixOpt::prefix` given.
     * @param capitalization LoUpCase capitalization, default is LoUpCase::lower
     * @param prefix pass PrefixOpt::prefix (default) to add leading `0x` if `byteOrder == lb_endian_t::big` (default)
     * @return the hex-string representation of the value
     * @see bytesHexString()
     * @see from_hexstring()
     */
    template<class value_type>
      requires jau::req::standard_layout<value_type> &&
               jau::req::trivially_copyable<value_type> &&
               (!jau::req::container<value_type>) &&
               (!jau::req::pointer<value_type>)
    inline std::string toHexString(value_type const &v, const lb_endian_t byteOrder = lb_endian_t::big,
                                   const LoUpCase capitalization = LoUpCase::lower,
                                   const PrefixOpt prefix = PrefixOpt::prefix) noexcept {
        if constexpr ( is_little_endian() ) {
            return toHexString(pointer_cast<const uint8_t *>(&v), sizeof(v),
                               byteOrder, capitalization, prefix);
        } else {
            const value_type v_le = jau::bswap(v);
            return toHexString(pointer_cast<const uint8_t *>(&v_le), sizeof(v),
                               byteOrder, capitalization, prefix);
        }
    }

    /**
    // *************************************************
    // *************************************************
    // *************************************************
     */

    /**
     * Converts a given binary string representation, appending to a byte vector (lsb-first).
     *
     * In case a non valid binary digit appears in the given string,
     * conversion ends and fills the byte vector up until the violation.
     *
     * In case bitstr contains an incomplete number of bit-nibbles, it will be interpreted as follows
     * - 0b11[00000001] = 0x0301 = { 0x01, 0x03 } - msb, 1st single low-nibble is most significant
     * - 0b[01000000]11 = 0xC040 = { 0x40, 0xC0 } - lsb, last single high-nibble is most significant
     *   - 11 -> 11000000 -> C0
     *
     * Even if complete==false, result holds the partial value if consumed_chars>0.
     *
     * You may use C++17 structured bindings to handle the pair.
     *
     * @param out the byte vector sink to append, lsb-first
     * @param bitstr the binary string representation
     * @param bitstr_len length of bitstr
     * @param bitOrder bit_order_t::msb for most significant bits in `bitstr` first (default)
     * @param checkPrefix if True, checks for a leading `0b` and removes it, otherwise not.
     * @return pair [size_t consumed_chars, bool complete], i.e. consumed characters of string and completed=false if not fully consumed.
     */
    SizeBoolPair fromBitString(std::vector<uint8_t> &out, const uint8_t bitstr[], const size_t bitstr_len,
                               const bit_order_t bitOrder = bit_order_t::msb, const Bool checkPrefix = Bool::True);

    /** See fromBitString() */
    inline SizeBoolPair fromBitString(std::vector<uint8_t> &out, const std::string_view bitstr,
                                      const bit_order_t bitOrder = bit_order_t::msb, const Bool checkPrefix = Bool::True) {
        return jau::fromBitString(out, cast_char_ptr_to_uint8(bitstr.data()), bitstr.length(), bitOrder, checkPrefix); // NOLINT(bugprone-suspicious-stringview-data-usage)
    }

    /**
     * Converts a given binary string representation, storing into a byte array(lsb-first).
     *
     * In case a non valid binary digit appears in the given string,
     * conversion ends and fills the byte vector up until the violation.
     *
     * In case bitstr contains an incomplete number of bit-nibbles, it will be interpreted as follows
     * - 0b11[00000001] = 0x0301 = { 0x01, 0x03 } - msb, 1st single low-nibble is most significant
     * - 0b[01000000]11 = 0xC040 = { 0x40, 0xC0 } - lsb, last single high-nibble is most significant
     *   - 11 -> 11000000 -> C0
     *
     * Even if complete==false, result holds the partial value if consumed_chars>0.
     *
     * You may use C++17 structured bindings to handle the triple.
     *
     * @param out byte array pointer to store result, lsb-first
     * @param out_len size of byte array
     * @param bitstr the binary string representation
     * @param bitstr_len length of bitstr
     * @param bitOrder bit_order_t::msb for most significant bits in `bitstr` first (default)
     * @param checkPrefix if True, checks for a leading `0b` and removes it, otherwise not.
     * @return triple [uint8_t* out_end, size_t consumed_chars, bool complete],
     *         i.e. end pointer of out (last write + 1), consumed characters of string and completed=false if not fully consumed.
     */
    UInt8PtrSizeBoolPair fromBitString(uint8_t *out, size_t out_len, const uint8_t bitstr[], const size_t bitstr_len,
                                       const bit_order_t bitOrder = bit_order_t::msb, const Bool checkPrefix = Bool::True) noexcept;

    /** See fromBitString() */
    inline UInt8PtrSizeBoolPair fromBitString(uint8_t *out, size_t out_len, const std::string_view bitstr,
                                      const bit_order_t bitOrder = bit_order_t::msb, const Bool checkPrefix = Bool::True) noexcept {
        return jau::fromBitString(out, out_len, cast_char_ptr_to_uint8(bitstr.data()), bitstr.length(), bitOrder, checkPrefix); // NOLINT(bugprone-suspicious-stringview-data-usage)
    }

    /**
     * Converts a given binary string representation into a uint64_t value according to bitStringBytes().
     *
     * Even if complete==false, result holds the partial value if consumed_chars>0.
     *
     * You may use C++17 structured bindings to handle the tuple.
     *
     * @param bitstr the binary string representation
     * @param checkPrefix if true, checks for a leading `0b` and removes it, otherwise not.
     * @param bitOrder bit_order_t::msb for most significant bits in `bitstr` first (default)
     * @return tuple [uint64_t result, size_t consumed_chars, bool complete], i.e. consumed characters of string and completed=false if not fully consumed.
     * @see bitStringBytes()
     * @see to_bitstring()
     */
    UInt64SizeBoolTuple fromBitString(std::string_view const bitstr, const bit_order_t bitOrder = bit_order_t::msb, const Bool checkPrefix = Bool::True) noexcept;

    /**
     * Appends a binary string representation of the given lsb-first byte values.
     *
     * If byteOrder is lb_endian_t::little, orders lsb-byte left, usual for byte streams. Result will not have a leading `0b`.
     * Otherwise, lb_endian_t::big (default),  orders msb-byte left for integer values. Result will have a leading `0b` if !skipPrefix.
     *
     * @param dest the std::string to append to
     * @param data pointer to the first byte to print, lsb-first
     * @param length number of bytes to print
     * @param bitOrder bit_order_t::msb for most-significant-bit first in resulting bit-string, bit_order_t::msb is default
     *                  A leading `0b` will be prepended if `bitOrder == bit_order_t::msb` and `PrefixOpt::prefix` given.
     * @param prefix pass PrefixOpt::prefix (default) to add leading `0b` if `bitOrder == bit_order_t::msb` (default)
     * @param bit_len optional fixed number of bits to be printed counting from lsb excluding prefix. Pass zero for dropping zero leading bytes (default).
     * @return the given string buffer for concatenation
     */
    std::string& appendBitString(std::string& dest, const void *data, const nsize_t length,
                                 const bit_order_t bitOrder = bit_order_t::msb, const PrefixOpt prefix = PrefixOpt::prefix,
                                 size_t bit_len=0) noexcept;

    /**
     * Produce a binary string representation of the given lsb-first byte values.
     *
     * If byteOrder is lb_endian_t::little, orders lsb-byte left, usual for byte streams. Result will not have a leading `0b`.
     * Otherwise, lb_endian_t::big (default),  orders msb-byte left for integer values. Result will have a leading `0b` if !skipPrefix.
     *
     * @param data pointer to the first byte to print, lsb-first
     * @param length number of bytes to print
     * @param bitOrder bit_order_t::msb for most-significant-bit first in resulting bit-string, bit_order_t::msb is default
     *                  A leading `0b` will be prepended if `bitOrder == bit_order_t::msb` and `PrefixOpt::prefix` given.
     * @param prefix pass PrefixOpt::prefix (default) to add leading `0b` if `bitOrder == bit_order_t::msb` (default)
     * @param bit_len optional fixed number of bits to be printed counting from lsb excluding prefix. Pass zero for dropping zero leading bytes (default).
     * @return the bit-string representation of the data
     */
    inline std::string toBitString(const void *data, const nsize_t length,
                                   const bit_order_t bitOrder = bit_order_t::msb, const PrefixOpt prefix = PrefixOpt::prefix,
                                   size_t bit_len=0) noexcept {
        std::string s;
        appendBitString(s, data, length, bitOrder, prefix, bit_len);
        return s;
    }

    /**
     * Produce a binary string representation with leading `0b` in MSB of the given uint8_t continuous container values.
     * @tparam uint8_container_type a uint8_t continuous container type
     * @param bytes the value of given uint8_t continuous container type
     * @param bitOrder bit_order_t::msb for most-significant-bit first in resulting bit-string, bit_order_t::msb is default
     *                  A leading `0b` will be prepended if `bitOrder == bit_order_t::msb` and `PrefixOpt::prefix` given.
     * @param prefix pass PrefixOpt::prefix (default) to add leading `0b` if `bitOrder == bit_order_t::msb` (default)
     * @param bit_len optional fixed number of bits to be printed counting from lsb excluding prefix. Pass zero for dropping zero leading bytes (default).
     * @return the bit-string representation of the value
     * @see bytesBitString()
     * @see from_bitstring()
     */
    template<class uint8_container_type>
        requires jau::req::contiguous_container<uint8_container_type> &&
                 std::convertible_to<typename uint8_container_type::value_type, uint8_t>
    inline std::string toBitString(const uint8_container_type &bytes,
                            const bit_order_t bitOrder = bit_order_t::msb, const PrefixOpt prefix = PrefixOpt::prefix, size_t bit_len=0) noexcept {
        return toBitString((const uint8_t *)bytes.data(), bytes.size(), bitOrder, prefix, bit_len);
    }

    /**
     * Produce a binary string representation with leading `0b` in MSB of the given value with standard layout.
     * @tparam value_type a standard layout value type
     * @param v the value of given standard layout type
     * @param bitOrder bit_order_t::msb for most-significant-bit first in resulting bit-string, bit_order_t::msb is default
     *                  A leading `0b` will be prepended if `bitOrder == bit_order_t::msb` and `PrefixOpt::prefix` given.
     * @param prefix pass PrefixOpt::prefix (default) to add leading `0b` if `bitOrder == bit_order_t::msb` (default)
     * @param bit_len optional fixed number of bits to be printed counting from lsb excluding prefix. Pass zero for dropping zero leading bytes (default).
     * @return the bit-string representation of the value
     * @see bytesBitString()
     * @see from_bitstring()
     */
    template<class value_type>
        requires jau::req::standard_layout<value_type> &&
                 jau::req::trivially_copyable<value_type> &&
                 (!jau::req::container<value_type>) &&
                 (!jau::req::pointer<value_type>)
    inline std::string toBitString(value_type const &v, const bit_order_t bitOrder = bit_order_t::msb,
                                   const PrefixOpt prefix = PrefixOpt::prefix, size_t bit_len=0) noexcept
    {
        if constexpr ( is_little_endian() ) {
            return toBitString(pointer_cast<const uint8_t *>(&v), sizeof(v),
                               bitOrder, prefix, bit_len);
        } else {
            const value_type v_le = jau::bswap(v);
            return toBitString(pointer_cast<const uint8_t *>(&v_le), sizeof(v),
                               bitOrder, prefix, bit_len);
        }
    }

    /**
    // *************************************************
    // *************************************************
    // *************************************************
     */

    /**
     * Produce a decimal string representation of an integral integer value.
     * @tparam value_type an integral integer type
     * @param v the integral integer value
     * @param separator if not 0, use as separation character, otherwise no separation characters are being used
     * @param width the minimum number of characters to be printed. Add padding with blank space if result is shorter.
     * @return the string representation of the integral integer value
     */
    template<class value_type,
             std::enable_if_t<std::is_integral_v<value_type>,
                              bool> = true>
    std::string to_decstring(const value_type &v, const char separator = ',', const nsize_t width = 0) noexcept {
        const snsize_t v_sign = jau::sign<value_type>(v);
        const size_t digit10_count1 = jau::digits10<value_type>(v, v_sign, true /* sign_is_digit */);
        const size_t digit10_count2 = v_sign < 0 ? digit10_count1 - 1 : digit10_count1;  // less sign

        const size_t separator_count = separator ? (digit10_count1 - 1) / 3 : 0;
        const size_t net_chars = digit10_count1 + separator_count;
        const size_t total_chars = std::max<size_t>(width, net_chars);
        std::string res;
        std::exception_ptr eptr;
        try {
            res.resize(total_chars, ' ');

            value_type n = v;
            size_t char_iter = 0;

            for ( size_t digit10_iter = 0; digit10_iter < digit10_count2 /* && char_iter < total_chars */; digit10_iter++ ) {
                const int digit = v_sign < 0 ? invert_sign(n % 10) : n % 10;
                n /= 10;
                if ( separator && 0 < digit10_iter && 0 == digit10_iter % 3 ) {
                    res[total_chars - 1 - (char_iter++)] = separator;
                }
                res[total_chars - 1 - (char_iter++)] = '0' + digit;
            }
            if ( v_sign < 0 /* && char_iter < total_chars */ ) {
                res[total_chars - 1 - (char_iter++)] = '-';
            }
        } catch (...) {
            eptr = std::current_exception();
        }
        jau::handle_exception(eptr, E_FILE_LINE);
        return res;
    }

    /**
    // *************************************************
    // *************************************************
    // *************************************************
     */

    /**
     * Converts a given integer string representation to the given result reference,
     * compatible with `::strtoll()`
     *
     * - Signed value_type: `[space][+-][prefix][digits+sep]`
     * - Unsigned value_type: `[space][+][prefix][digits+sep]`
     *
     * Even if complete==false due to empty string, under- or overflow,
     * the result holds the partial value if consumed_chars>0.
     *
     * Whitespace and non-matching chars
     * - Leading and tailing whitespace chars are consumed, see jau::is_space()
     * - Tail non-matching chars are ignored: complete is true but consumed_chars is < str.length()
     *
     * You may use C++17 structured bindings to handle the pair.
     * - `[size_t consumed_chars, bool complete]`
     *
     * @param result the integral reference for the result
     * @param str the decimal string representation
     * @param radix base of the number system, supported: 2 binary, 8 octal, 10 decimal, 16 hexadecimal
     * @param separator separator character (default 0, none), allowing to ignore like thousand separator characters
     * @return pair [size_t consumed_chars, bool complete], i.e. consumed characters of string and completed=false if not fully consumed.
     */
    template<std::integral value_type>
    constexpr SizeBoolPair fromIntString(value_type &result, std::string_view str, uint32_t radix=10, const char separator = 0) noexcept {
        using namespace jau::int_literals;
        result = 0;

        std::string_view::const_iterator str_begin = str.cbegin();
        std::string_view::const_iterator str_end = str.cend();
        std::string_view::const_iterator begin = str_begin; // begin of digits
        value_type sign = 1;

        // consume leading whitespace
        while( begin < str_end && jau::is_space(*begin)) { ++begin; }
        // sign
        if (begin < str_end ) {
            if (*begin == '-') {
                if constexpr (std::is_unsigned_v<value_type>) {
                    return { .s = size_t(begin-str_begin), .b = false }; // invalid
                }
                sign = -1;
                ++begin;
            } else if (*begin == '+') {
                ++begin;
            }
        }
        // prefix
        if( 16 == radix && begin < str_end-1 ) {
            if ( *begin == '0' && (*(begin+1) == 'x' || *(begin+1) == 'X') ) {
                begin+=2;
            }
        } else if( 8 == radix && begin < str_end ) {
            if ( *begin == '0' ) {
                begin+=1;
            }
        } else if( 2 == radix && begin < str_end-1 ) {
            if ( *begin == '0' && *(begin+1) == 'b' ) {
                begin+=2;
            }
        }
        if (begin == str_end || !jau::is_digit(*begin, radix, separator)) {
            return { .s = size_t(begin-str_begin), .b = false }; // no number (empty or no digit)
        }

        std::string_view::const_iterator end = begin;
        while( end < str_end && jau::is_digit(*end, radix, separator)) { ++end; } // position to last digit, ignore tail
        const size_t len = end-str_begin; // consumed length w/o tail
        std::string_view::const_iterator iter = end;

        value_type multiplier = 1;
        while( iter > begin ) {
            const int32_t d = digit(*(--iter), radix);
            if ( 0 > d ) {
                continue; // skip seperator
            }
            const value_type sum = d * multiplier * sign;
            if( sign > 0 && result > std::numeric_limits<value_type>::max() - sum ) {
                // overflow
                return { .s = len, .b = false };
            }
            if constexpr (std::is_signed_v<value_type>) {
                if( sign < 0 && result < std::numeric_limits<value_type>::min() - sum ) {
                    // underflow
                    return { .s = len, .b = false };
                }
            }
            result += sum;
            multiplier *= radix;
        }
        // consume tailing whitespace
        while( end < str_end && jau::is_space(*end)) { ++end; }
        return { .s = size_t(end-str_begin), .b = true };
    }
    /// See fromIntString
    template<std::integral value_type>
    constexpr SizeBoolPair fromIntString(value_type &result, const char *str, size_t str_len, uint32_t radix=10, const char separator = 0) noexcept {
        return fromIntString(result, std::string_view(str, str_len), radix, separator);
    }

    /**
     * Appends an integer string representation of an integral integer value with given radix.
     *
     * `[space][-][prefix][zeros_padding+sep][digits+sep]`
     *
     * @tparam value_type an integral integer type
     * @param dest the std::string to append to
     * @param val the unsigned integral integer value
     * @param radix base of the number system, supported: 2 binary, 8 octal, 10 decimal, 16 hexadecimal
     * @param capitalization LoUpCase capitalization, default is LoUpCase::lower
     * @param prefix pass PrefixOpt::prefix (default) to add leading prefix for radix. Prefixes: `0x` hex, `0` octal and `0b` binary.
     * @param min_width the minimum number of characters to be printed including sign and prefix. Add padding if sign+prefix+val-digits are shorter.
     * @param separator separator character for each decimal 3 or other radix 4. Defaults to 0 for no separator.
     * @param padding padding character, defaults to '0'. See 'min_width' above.
     * @return the given string buffer for concatenation
     */
    template<std::integral value_type>
    std::string& appendIntString(std::string &dest, value_type val, const uint32_t radix,
                                   const LoUpCase capitalization = LoUpCase::lower,
                                   const PrefixOpt prefix = PrefixOpt::prefix,
                                   const uint32_t min_width = 0, const char separator = 0, const char padding = '0') noexcept
    {
        const size_t dest_start_len = dest.size();
        uint32_t shift;
        switch ( radix ) {
            case 16: shift = 4; break;
            case 10: shift = 0; break;
            case 8:  shift = 3; break;
            case 2:  shift = 1; break;
            default: return dest;
        }
        using unsigned_value_type = std::make_unsigned_t<value_type>;
        unsigned_value_type v = jau::unsigned_value( val );
        const uint32_t val_digits = jau::digits<unsigned_value_type>(v, radix);
        const uint32_t sign_len = jau::is_positive(val) ? 0 : 1;
        const uint32_t prefix_len = (PrefixOpt::none == prefix || 10 == radix) ? 0 : (8 == radix ? 1 : 2);
        const uint32_t sep_gap = 10 == radix ? 3 : 4;
        uint32_t sep_count = 0, space_left=0;
        if (separator) {
            sep_count = (val_digits - 1) / sep_gap;
            const uint32_t len0 = sign_len + prefix_len + val_digits + sep_count;
            if (min_width > len0) {
                if (val_digits > 0 && '0' == padding) {
                    // separator inside padding
                    const uint32_t len1 = min_width - sign_len - prefix_len;
                    sep_count = (len1 - 1) / sep_gap;
                    if ( sign_len + prefix_len + (val_digits + sep_count) > min_width ) {
                        --sep_count;  // fix down
                    }
                } else {
                    space_left = min_width - len0;
                }
            }
        }
        {
            std::exception_ptr eptr;
            try {
                const size_t added_len = std::max<size_t>(min_width, space_left + sign_len + prefix_len + (val_digits + sep_count));
                dest.reserve(dest_start_len + added_len + 1); // w/ EOS
                dest.resize(dest_start_len + added_len, ' '); // w/o EOS
            } catch (...) {
                eptr = std::current_exception();
            }
            if (handle_exception(eptr, E_FILE_LINE)) {
                return dest;
            }
        }
        const char * const d_start = dest.data() + dest_start_len;
        const char * const d_start_num = d_start + space_left + sign_len + prefix_len;
        char *d = dest.data()+dest.size();
        *d = 0; // EOS (is reserved)
        {
            const char *hex_array = LoUpCase::lower == capitalization ? HexadecimalArrayLow : HexadecimalArrayBig;
            const unsigned_value_type mask = unsigned_value_type(radix - 1);  // ignored for radix 10
            uint32_t digit_cnt = 0, separator_idx = 0;
            while (d > d_start_num) {
                if (separator_idx < sep_count && 0 < digit_cnt && 0 == digit_cnt % sep_gap) {
                    *(--d) = separator;
                    ++separator_idx;
                }
                if (d > d_start_num) {
                    if (digit_cnt >= val_digits) {
                        *(--d) = padding;
                    } else if (10 == radix) {
                        *(--d) = '0' + (v % 10);
                        v /= 10;
                    } else {
                        *(--d) = hex_array[v & mask];
                        v >>= shift;
                    }
                    ++digit_cnt;
                }
            }
        }
        if ( prefix_len && d > d_start ) {
            switch ( radix ) {  // NOLINT(bugprone-switch-missing-default-case)
                case 16: *(--d) = 'x'; break;
                case 8:  *(--d) = '0'; break;
                case 2:  *(--d) = 'b'; break;
            }
            if ( d > d_start ) {
                *(--d) = '0';
            }
        }
        if ( sign_len && d > d_start ) {
            *(--d) = '-';
        }
        return dest;
    }

    /**
     * Produces an integer string representation of an integral integer value with given radix.
     *
     * `[space][-][prefix][zeros_padding+sep][digits+sep]`
     *
     * @tparam value_type an unsigned integral integer type
     * @param v the integral integer value
     * @param radix base of the number system, supported: 2 binary, 8 octal, 10 decimal, 16 hexadecimal
     * @param capitalization LoUpCase capitalization, default is LoUpCase::lower
     * @param prefix pass PrefixOpt::prefix (default) to add leading prefix for radix. Prefixes: `0x` hex, `0` octal and `0b` binary.
     * @param min_width the minimum number of characters to be printed including prefix. Add padding with `padding` if result is shorter.
     * @param separator separator character for each decimal 3 or other radix 4. Defaults to 0 for no separator.
     * @param padding padding character, defaults to '0'. See 'min_width' above.
     * @return the string representation of the unsigned integral integer value with given radix
     */
    template<std::integral value_type>
    std::string to_string(value_type v, const nsize_t radix,
                          const LoUpCase capitalization = LoUpCase::lower,
                          const PrefixOpt prefix = PrefixOpt::prefix,
                          const nsize_t min_width = 0, const char separator = 0, const char padding = '0') noexcept
    {
        std::string str;
        appendIntString(str, v, radix, capitalization, prefix, min_width, separator, padding);
        return str;
    }

    /**
    // *************************************************
    // *************************************************
    // *************************************************
     */

    template<typename CharT, std::size_t N>
    constexpr std::string to_string(const CharT (&ref)[N]) {
        return std::string(ref);
    }

    template<class value_type>
    requires std::is_same_v<jau::StringLiteral<value_type::size>, value_type> // jau::req::string_alike<value_type>
    constexpr std::string to_string(const value_type &ref) {
        return std::string(ref);
    }

    template<class value_type,
             std::enable_if_t<(std::is_integral_v<value_type> && !std::is_same_v<bool, std::remove_cv_t<value_type>>) ||
                              std::is_floating_point_v<value_type>,
                              bool> = true>
    inline std::string to_string(const value_type &ref) {
        return std::to_string(ref);
    }

    template<class value_type,
             std::enable_if_t<std::is_same_v<bool, std::remove_cv_t<value_type>>,
                              bool> = true>
    inline std::string to_string(const value_type &ref) {
        return ref ? "T" : "F";
    }

    template<class value_type,
             std::enable_if_t<!std::is_integral_v<value_type> &&
                              !std::is_floating_point_v<value_type> &&
                              std::is_base_of_v<std::string, value_type>,
                              bool> = true>
    inline std::string to_string(const value_type &ref) {
        return ref;
    }

    template<class value_type,
             std::enable_if_t<!std::is_integral_v<value_type> &&
                              !std::is_floating_point_v<value_type> &&
                              !std::is_base_of_v<std::string, value_type> &&
                              std::is_base_of_v<std::string_view, value_type>,
                              bool> = true>
    inline std::string to_string(const value_type &ref) {
        return std::string(ref);
    }

    template<class value_type,
             std::enable_if_t<!std::is_integral_v<value_type> &&
                              !std::is_floating_point_v<value_type> &&
                              !std::is_base_of_v<std::string, value_type> &&
                              !std::is_base_of_v<std::string_view, value_type> &&
                              std::is_same_v<char*, jau::req::base_pointer<value_type>>,
                              bool> = true>
    inline std::string to_string(const value_type &ref) {
        return std::string(ref);
    }

    template<class value_type,
             std::enable_if_t<!std::is_integral_v<value_type> &&
                              !std::is_floating_point_v<value_type> &&
                              !std::is_base_of_v<std::string, value_type> &&
                              !std::is_base_of_v<std::string_view, value_type> &&
                              !std::is_same_v<char*, jau::req::base_pointer<value_type>> &&
                              std::is_pointer_v<value_type>,
                              bool> = true>
    inline std::string to_string(const value_type &ref) {
        return toHexString((void *)ref);  // NOLINT(bugprone-multi-level-implicit-pointer-conversion)
    }

    template<class value_type,
             std::enable_if_t<!std::is_integral_v<value_type> &&
                              !std::is_floating_point_v<value_type> &&
                              !std::is_base_of_v<std::string, value_type> &&
                              !std::is_base_of_v<std::string_view, value_type> &&
                              !std::is_pointer_v<value_type> &&
                              jau::has_toString_v<value_type>,
                              bool> = true>
    inline std::string to_string(const value_type &ref) {
        return ref.toString();
    }

    template<class value_type,
             std::enable_if_t<!std::is_integral_v<value_type> &&
                              !std::is_floating_point_v<value_type> &&
                              !std::is_base_of_v<std::string, value_type> &&
                              !std::is_base_of_v<std::string_view, value_type> &&
                              !std::is_pointer_v<value_type> &&
                              !jau::has_toString_v<value_type> &&
                              jau::has_to_string_v<value_type>,
                              bool> = true>
    inline std::string to_string(const value_type &ref) {
        return ref.to_string();
    }

    template<class value_type,
             std::enable_if_t<!std::is_integral_v<value_type> &&
                              !std::is_floating_point_v<value_type> &&
                              !std::is_base_of_v<std::string, value_type> &&
                              !std::is_base_of_v<std::string_view, value_type> &&
                              !std::is_pointer_v<value_type> &&
                              !jau::has_toString_v<value_type> &&
                              !jau::has_to_string_v<value_type> &&
                              jau::has_member_of_pointer_v<value_type>,
                              bool> = true>
    inline std::string to_string(const value_type &ref) {
        return toHexString((void *)ref.operator->());
    }

    template<class value_type,
             std::enable_if_t<!std::is_integral_v<value_type> &&
                              !std::is_floating_point_v<value_type> &&
                              !std::is_base_of_v<std::string, value_type> &&
                              !std::is_base_of_v<std::string_view, value_type> &&
                              !std::is_pointer_v<value_type> &&
                              !jau::has_toString_v<value_type> &&
                              !jau::has_to_string_v<value_type> &&
                              !jau::has_member_of_pointer_v<value_type>,
                              bool> = true>
    inline std::string to_string(const value_type &ref) {
        (void)ref;
        return "jau::to_string<T> n/a for type " + jau::static_ctti<value_type>().toString();
    }

    template<typename T>
    std::string to_string(std::vector<T> const &list, const std::string &delim) {
        if ( list.empty() ) {
            return std::string();
        }
        bool need_delim = false;
        std::string res;
        for ( const T &e : list ) {
            if ( need_delim ) {
                res.append(delim);
            }
            res.append(to_string(e));
            need_delim = true;
        }
        return res;
    }
    template<typename T>
    std::string to_string(std::vector<T> const &list) { return to_string<T>(list, ", "); }

    template<typename T>
    std::string to_string(std::vector<T> const &list, const std::string &delim, const nsize_t radix) {
        if ( list.empty() ) {
            return std::string();
        }
        bool need_delim = false;
        std::string res;
        for ( const T &e : list ) {
            if ( need_delim ) {
                res.append(delim);
            }
            res.append(to_string(e, radix));
            need_delim = true;
        }
        return res;
    }
    template<typename T>
    std::string to_string(std::vector<T> const &list, const nsize_t radix) { return to_string<T>(list, ", ", radix); }

    /**@}*/

}  // namespace jau

/** \example test_intdecstring01.cpp
 * This C++ unit test validates the jau::to_decstring implementation
 */

#endif /* JAU_STRING_UTIL_HPP_ */
