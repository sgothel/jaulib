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

#include <algorithm>
#include <concepts>
#include <cstdarg>
#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "jau/basic_types.hpp"
#include "jau/type_info.hpp"

#include <jau/byte_util.hpp>
#include <jau/cpp_lang_util.hpp>
#include <jau/int_math.hpp>
#include <jau/int_types.hpp>
#include <jau/packed_attribute.hpp>
#include <jau/string_cfmt.hpp>
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
    std::vector<std::string> split_string(const std::string &str, const std::string &separator) noexcept;

    std::string &toLowerInPlace(std::string &s) noexcept;

    std::string toLower(const std::string &s) noexcept;

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
     * Converts a given hexadecimal string representation into a byte vector, lsb-first.
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
     * @param out the byte vector sink, lsb-first
     * @param hexstr the hexadecimal string representation
     * @param hexstr_len length of hextstr
     * @param byteOrder lb_endian_t::big for big-endian bytes in `hexstr` (default)
     * @param checkPrefix if True, checks for a leading `0x` and removes it, otherwise not.
     * @return pair [size_t consumed_chars, bool complete], i.e. consumed characters of string and completed=false if not fully consumed.
     */
    SizeBoolPair fromHexString(std::vector<uint8_t> &out, const uint8_t hexstr[], const size_t hexstr_len,
                               const lb_endian_t byteOrder = lb_endian_t::big, const Bool checkPrefix = Bool::True) noexcept;

    /** See hexStringBytes() */
    inline SizeBoolPair fromHexString(std::vector<uint8_t> &out, const std::string_view hexstr,
                                      const lb_endian_t byteOrder = lb_endian_t::big, const Bool checkPrefix = Bool::True) noexcept {
        return jau::fromHexString(out, cast_char_ptr_to_uint8(hexstr.data()), hexstr.length(), byteOrder, checkPrefix); // NOLINT(bugprone-suspicious-stringview-data-usage)
    }

    /**
     * Converts a given hexadecimal string representation into a uint64_t value according to hexStringBytes().
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

    inline constexpr const char *HexadecimalArray = "0123456789abcdef";

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
    std::string toHexString(const void *data, const nsize_t length,
                            const lb_endian_t byteOrder = lb_endian_t::big, const LoUpCase capitalization = LoUpCase::lower,
                            const PrefixOpt prefix = PrefixOpt::prefix) noexcept;

    /**
     * Produce a hexadecimal string representation of the given byte value and appends it to the given string
     * @param dest the std::string reference destination to append
     * @param value the byte value to represent
     * @param capitalization LoUpCase capitalization, default is LoUpCase::lower
     * @return the given std::string reference for chaining
     */
    std::string &appendToHexString(std::string &dest, const uint8_t value, const LoUpCase capitalization = LoUpCase::lower) noexcept;

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
     * Converts a given binary string representation into a byte vector, lsb-first.
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
     * @param out the byte vector sink, lsb-first
     * @param bitstr the binary string representation
     * @param bitstr_len length of bitstr
     * @param bitOrder bit_order_t::msb for most significant bits in `bitstr` first (default)
     * @param checkPrefix if True, checks for a leading `0b` and removes it, otherwise not.
     * @return pair [size_t consumed_chars, bool complete], i.e. consumed characters of string and completed=false if not fully consumed.
     */
    SizeBoolPair fromBitString(std::vector<uint8_t> &out, const uint8_t bitstr[], const size_t bitstr_len,
                               const bit_order_t bitOrder = bit_order_t::msb, const Bool checkPrefix = Bool::True) noexcept;

    /** See fromBitString() */
    inline SizeBoolPair fromBitString(std::vector<uint8_t> &out, const std::string_view bitstr,
                                      const bit_order_t bitOrder = bit_order_t::msb, const Bool checkPrefix = Bool::True) noexcept {
        return jau::fromBitString(out, cast_char_ptr_to_uint8(bitstr.data()), bitstr.length(), bitOrder, checkPrefix); // NOLINT(bugprone-suspicious-stringview-data-usage)
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
    std::string toBitString(const void *data, const nsize_t length,
                            const bit_order_t bitOrder = bit_order_t::msb, const PrefixOpt prefix = PrefixOpt::prefix,
                            size_t bit_len=0) noexcept;

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
        std::string res(total_chars, ' ');

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
        return res;
    }

    /**
    // *************************************************
    // *************************************************
    // *************************************************
     */

    /**
     * Produce a string representation of an unsigned integral integer value with given radix.
     * @tparam value_type an unsigned integral integer type
     * @param v the unsigned integral integer value
     * @param radix base of the number system, supported: 2 binary, 8 octal, 10 decimal, 16 hexadecimal
     * @param prefix pass PrefixOpt::prefix (default) to add leading prefix for radix. Prefixes: `0x` hex, `0` octal and `0b` binary.
     * @param min_width the minimum number of characters to be printed including prefix. Add padding with `padding` if result is shorter.
     * @param separator separator character for each decimal 3 or other radix 4. Defaults to 0 for no separator.
     * @param padding padding character, defaults to '0'. See 'min_width' above.
     * @return the string representation of the unsigned integral integer value with given radix
     */
    template<class value_type,
             std::enable_if_t<std::is_integral_v<value_type> &&
                              std::is_unsigned_v<value_type>,
                              bool> = true>
    std::string to_string(value_type v, const nsize_t radix, const PrefixOpt prefix = PrefixOpt::prefix,
                          size_t min_width = 0, const char separator = 0, const char padding = '0') noexcept
    {
        nsize_t shift;
        switch ( radix ) {
            case 16: shift = 4; break;
            case 10: shift = 0; break;
            case 8:  shift = 3; break;
            case 2:  shift = 1; break;
            default: return "";
        }
        const nsize_t mask = radix - 1;  // ignored for radix 10
        const size_t val_digits = jau::digits<value_type>(v, radix);
        const size_t prefix_len = (PrefixOpt::none == prefix || 10 == radix) ? 0 : (8 == radix ? 1 : 2);
        const size_t separator_gap = 10 == radix ? 3 : 4;
        size_t separator_count;
        if ( separator && '0' == padding ) {
            // separator inside padding
            if ( min_width > prefix_len ) {
                const size_t len0 = std::max<size_t>(min_width - prefix_len, val_digits);
                separator_count = (len0 - 1) / separator_gap;
                if ( val_digits + separator_count + prefix_len > min_width ) {
                    --separator_count;  // fix down
                }
            } else {
                separator_count = (val_digits - 1) / separator_gap;
            }
        } else if ( separator ) {
            // separator w/o padding
            separator_count = (val_digits - 1) / separator_gap;
        } else {
            separator_count = 0;
        }
        size_t len = std::max<size_t>(min_width, val_digits + separator_count + prefix_len);

        std::string str(len, ' ');
        size_t digit_idx = 0, separator_idx = 0;
        while ( len > prefix_len ) {
            if ( separator_idx < separator_count && 0 < digit_idx && 0 == digit_idx % separator_gap ) {
                str[--len] = separator;
                ++separator_idx;
            }
            if ( len > prefix_len ) {
                if ( 10 != radix ) {
                    str[--len] = digit_idx < val_digits ? HexadecimalArray[v & mask] : padding;
                    v >>= shift;
                } else {
                    str[--len] = digit_idx < val_digits ? '0' + (v % 10) : padding;
                    v /= 10;
                }
                ++digit_idx;
            }
        }
        if ( len > 0 ) {
            switch ( radix ) {  // NOLINT(bugprone-switch-missing-default-case)
                case 16: str[--len] = 'x'; break;
                case 8:  str[--len] = '0'; break;
                case 2:  str[--len] = 'b'; break;
            }
            if ( len > 0 ) {
                str[--len] = '0';
            }
        }
        return str;
    }

    /**
    // *************************************************
    // *************************************************
    // *************************************************
     */

    namespace impl {
        template<typename... Args>
        constexpr std::string format_string_n(const std::size_t maxStrLen, const std::string_view &format, const Args &...args) {
            std::string str;
            str.reserve(maxStrLen + 1);  // incl. EOS
            str.resize(maxStrLen);       // excl. EOS

            // -Wformat=2 -> -Wformat -Wformat-nonliteral -Wformat-security -Wformat-y2k
            // -Wformat=2 -Wformat-overflow=2 -Wformat-signedness
            PRAGMA_DISABLE_WARNING_PUSH
            PRAGMA_DISABLE_WARNING_FORMAT_NONLITERAL
            PRAGMA_DISABLE_WARNING_FORMAT_SECURITY
            const size_t nchars = std::snprintf(&str[0], maxStrLen + 1, format.data(), args...);  // NOLINT
            PRAGMA_DISABLE_WARNING_POP
            if ( nchars < maxStrLen + 1 ) {
                str.resize(nchars);
                str.shrink_to_fit();
            }  // else truncated w/ nchars > MaxStrLen
            return str;
        }

        template <typename... Args>
        constexpr std::string format_string_h(const std::size_t strLenHint, const std::string_view format, const Args &...args) {
            size_t nchars;
            std::string str;
            {
                const size_t bsz = strLenHint + 1;  // including EOS
                str.reserve(bsz);                   // incl. EOS
                str.resize(bsz - 1);                // excl. EOS

                // -Wformat=2 -> -Wformat -Wformat-nonliteral -Wformat-security -Wformat-y2k
                // -Wformat=2 -Wformat-overflow=2 -Wformat-signedness
                PRAGMA_DISABLE_WARNING_PUSH
                PRAGMA_DISABLE_WARNING_FORMAT_NONLITERAL
                PRAGMA_DISABLE_WARNING_FORMAT_SECURITY
                nchars = std::snprintf(&str[0], bsz, format.data(), args...);  // NOLINT
                PRAGMA_DISABLE_WARNING_POP
                if ( nchars < bsz ) {
                    str.resize(nchars);
                    str.shrink_to_fit();
                    return str;
                }
            }
            {
                const size_t bsz = std::min<size_t>(nchars + 1, str.max_size() + 1);  // limit incl. EOS
                str.reserve(bsz);                                                     // incl. EOS
                str.resize(bsz - 1);                                                  // excl. EOS

                // -Wformat=2 -> -Wformat -Wformat-nonliteral -Wformat-security -Wformat-y2k
                // -Wformat=2 -Wformat-overflow=2 -Wformat-signedness
                PRAGMA_DISABLE_WARNING_PUSH
                PRAGMA_DISABLE_WARNING_FORMAT_NONLITERAL
                PRAGMA_DISABLE_WARNING_FORMAT_SECURITY
                nchars = std::snprintf(&str[0], bsz, format.data(), args...);  // NOLINT
                PRAGMA_DISABLE_WARNING_POP

                str.resize(nchars);
                return str;
            }
        }
    }  // namespace impl

    /**
     * Safely returns a (potentially truncated) string according to `snprintf()` formatting rules
     * and variable number of arguments following the `format` argument.
     *
     * jau::cfmt2::checkR2() is utilize to validate `format` against given arguments at *runtime*
     * and throws jau::IllegalArgumentError on mismatch.
     *
     * Resulting string is truncated to `min(maxStrLen, formatLen)`,
     * with `formatLen` being the given formatted string length of output w/o limitation.
     *
     * @param maxStrLen maximum resulting string length
     * @param format `printf()` compliant format string
     * @param args optional arguments matching the format string
     */
    template<typename... Args>
    constexpr std::string format_string_n(const std::size_t maxStrLen, const std::string_view &format, const Args &...args) {
        const jau::cfmt::PResult pr = jau::cfmt::checkR2<Args...>(format);
        if ( pr.argCount() < 0 ) {
            throw jau::IllegalArgumentError("format/arg mismatch `" + std::string(format) + "`: " + pr.toString(), E_FILE_LINE);
        }
        return impl::format_string_n(maxStrLen, format, args...);
    }

    /**
     * Safely returns a (potentially truncated) string according to `snprintf()` formatting rules
     * and variable number of arguments following the `format` argument.
     *
     * jau::cfmt2::checkR2() is utilize to validate `format` against given arguments at *compile time*
     * and fails to compile on mismatch.
     *
     * Resulting string is truncated to `min(maxStrLen, formatLen)`,
     * with `formatLen` being the given formatted string length of output w/o limitation.
     *
     * @tparam format `printf()` compliant format string
     * @param maxStrLen maximum resulting string length
     * @param args optional arguments matching the format string
     */
    template <StringLiteral format, typename... Args>
    consteval_cxx20 std::string format_string_n(const std::size_t maxStrLen, const Args &...args) {
        static_assert(0 <= jau::cfmt::checkR2<Args...>(format.view()).argCount());
        return impl::format_string_n(maxStrLen, format.view(), args...);
    }

    /**
     * Safely returns a (non-truncated) string according to `snprintf()` formatting rules
     * and variable number of arguments following the `format` argument.
     *
     * jau::cfmt2::checkR2() is utilize to validate `format` against given arguments at *runtime*
     * and throws jau::IllegalArgumentError on mismatch.
     *
     * Resulting string size matches formated output w/o limitation.
     *
     * @param strLenHint initially used string length w/o EOS
     * @param format `printf()` compliant format string
     * @param args optional arguments matching the format string
     */
    template <typename... Args>
    constexpr std::string format_string_h(const std::size_t strLenHint, const std::string_view format, const Args &...args) {
        const jau::cfmt::PResult pr = jau::cfmt::checkR2<Args...>(format);
        if ( pr.argCount() < 0 ) {
            throw jau::IllegalArgumentError("format/arg mismatch `" + std::string(format) + "`: " + pr.toString(), E_FILE_LINE);
        }
        return impl::format_string_h(strLenHint, format, args...);
    }

    /**
     * Safely returns a (non-truncated) string according to `snprintf()` formatting rules
     * and variable number of arguments following the `format` argument.
     *
     * jau::cfmt2::checkR2() is utilize to validate `format` against given arguments at *runtime*
     * and throws jau::IllegalArgumentError on mismatch.
     *
     * Resulting string size matches formated output w/o limitation.
     *
     * @param format `printf()` compliant format string
     * @param args optional arguments matching the format string
     */
    template <typename... Args>
    constexpr std::string format_string(const std::string_view format, const Args &...args) {
        return format_string_h(1023, format, args...);
    }

    /**
     * Safely returns a (non-truncated) string according to `snprintf()` formatting rules
     * and variable number of arguments following the `format` argument.
     *
     * jau::cfmt2::checkR2() is utilize to validate `format` against given arguments at *compile time*
     * and fails to compile on mismatch.
     *
     * Resulting string size matches formated output w/o limitation.
     *
     * @tparam format `printf()` compliant format string
     * @param args optional arguments matching the format string
     */
    template <StringLiteral format, typename... Args>
    consteval_cxx20 std::string format_string(const Args &...args) {
        static_assert(0 <= jau::cfmt::checkR2<Args...>(format.view()).argCount());
        return impl::format_string_h(1023, format.view(), args...);
    }

    /**
    // *************************************************
    // *************************************************
    // *************************************************
     */

    template<class value_type,
             std::enable_if_t<(std::is_integral_v<value_type> && !std::is_same_v<bool, value_type>) ||
                              std::is_floating_point_v<value_type>,
                              bool> = true>
    inline std::string to_string(const value_type &ref) {
        return std::to_string(ref);
    }

    template<class value_type,
             std::enable_if_t<std::is_same_v<bool, value_type>,
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

    /**
     * Returns tuple [int64_t result, size_t consumed_chars, bool complete] of string to integer conversion via `std::strtoll`.
     *
     * Even if complete==false, result holds the partial value if consumed_chars>0.
     *
     * You may use C++17 structured bindings to handle the tuple.
     */
    Int64SizeBoolTuple to_integer(const char *str, size_t str_len, const nsize_t radix = 10, const char limiter = '\0', const char *limiter_pos = nullptr);


    /**
     * Returns tuple [int64_t result, size_t consumed_chars, bool complete] of string to integer conversion via `std::strtoll`.
     *
     * Even if complete==false, result holds the partial value if consumed_chars>0.
     *
     * You may use C++17 structured bindings to handle the tuple.
     */
     inline Int64SizeBoolTuple to_integer(const std::string_view str, const nsize_t radix = 10, const char limiter = '\0', const char *limiter_pos = nullptr) {
         return to_integer(str.data(), str.length(), radix, limiter, limiter_pos);
     }

    /**
     * C++20: Heterogeneous Lookup in (Un)ordered Containers
     *
     * @see https://www.cppstories.com/2021/heterogeneous-access-cpp20/
     */
    struct string_hash {
        using is_transparent = void;
        [[nodiscard]] size_t operator()(const char *txt) const {
            return std::hash<std::string_view>{}(txt);
        }
        [[nodiscard]] size_t operator()(std::string_view txt) const {
            return std::hash<std::string_view>{}(txt);
        }
        [[nodiscard]] size_t operator()(const std::string &txt) const {
            return std::hash<std::string>{}(txt);
        }
    };

    template<typename T>
    using StringHashMap = std::unordered_map<std::string, T, string_hash, std::equal_to<>>;

    using StringHashSet = std::unordered_set<std::string, string_hash, std::equal_to<>>;

    /**@}*/

}  // namespace jau

#define jau_format_string_static(...) \
    jau::format_string(__VA_ARGS__);  \
    static_assert(0 <= jau::cfmt::checkR(__VA_ARGS__).argCount());  // compile time validation!

/** \example test_intdecstring01.cpp
 * This C++ unit test validates the jau::to_decstring implementation
 */

#endif /* JAU_STRING_UTIL_HPP_ */
