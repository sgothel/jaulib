/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020 Gothel Software e.K.
 * Copyright (c) 2020 ZAFENA AB
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

#ifndef JAU_BYTE_UTIL_HPP_
#define JAU_BYTE_UTIL_HPP_

#include <cstring>
#include <string>
#include <memory>
#include <cstdint>
#include <vector>
#include <type_traits>

#include <jau/cpp_lang_util.hpp>
#include <jau/cpp_pragma.hpp>
#include <jau/packed_attribute.hpp>

#include <jau/int_types.hpp>

namespace jau {

    #if defined __has_builtin
        #if __has_builtin(__builtin_bswap16)
            #define __has_builtin_bswap16 1
        #endif
    #elif defined(__GNUC__) && __GNUC_PREREQ (4, 8)
        #define __has_builtin_bswap16 1
    #endif
    #if defined __has_builtin
        #if __has_builtin(__builtin_bswap32)
            #define __has_builtin_bswap32 1
        #endif
    #elif defined(__GNUC__) && __GNUC_PREREQ (4, 8)
        #define __has_builtin_bswap32 1
    #endif
    #if defined __has_builtin
        #if __has_builtin(__builtin_bswap64)
            #define __has_builtin_bswap64 1
        #endif
    #elif defined(__GNUC__) && __GNUC_PREREQ (4, 8)
        #define __has_builtin_bswap64 1
    #endif

    constexpr uint16_t bswap(uint16_t const source) noexcept {
        #if defined __has_builtin_bswap16
            return __builtin_bswap16(source);
        #else
            return (uint16_t) ( ( ( (source) >> 8 ) & 0xff ) |
                                ( ( (source) & 0xff) << 8 ) );
        #endif
    }

    constexpr uint32_t bswap(uint32_t const source) noexcept {
        #if defined __has_builtin_bswap32
            return __builtin_bswap32(source);
        #else
            return ( ( source & 0xff000000U ) >> 24 ) |
                   ( ( source & 0x00ff0000U ) >>  8 ) |
                   ( ( source & 0x0000ff00U ) <<  8 ) |
                   ( ( source & 0x000000ffU ) << 24 );
        #endif
    }

    constexpr uint64_t bswap(uint64_t const & source) noexcept {
        #if defined __has_builtin_bswap64
            return __builtin_bswap64(source);
        #else
            return ( ( source & 0xff00000000000000ULL ) >> 56 ) |
                   ( ( source & 0x00ff000000000000ULL ) >> 40 ) |
                   ( ( source & 0x0000ff0000000000ULL ) >> 24 ) |
                   ( ( source & 0x000000ff00000000ULL ) >>  8 ) |
                   ( ( source & 0x00000000ff000000ULL ) <<  8 ) |
                   ( ( source & 0x0000000000ff0000ULL ) << 24 ) |
                   ( ( source & 0x000000000000ff00ULL ) << 40 ) |
                   ( ( source & 0x00000000000000ffULL ) << 56 );
        #endif
    }

    constexpr uint128_t bswap(uint128_t const & source) noexcept {
        uint128_t dest;
        uint8_t const * const s = source.data;
        uint8_t * const d = dest.data;
        for(nsize_t i=0; i<16; i++) {
            d[i] = s[15-i];
        }
        return dest;
    }

    constexpr uint192_t bswap(uint192_t const & source) noexcept {
        uint192_t dest;
        uint8_t const * const s = source.data;
        uint8_t * const d = dest.data;
        for(nsize_t i=0; i<24; i++) {
            d[i] = s[23-i];
        }
        return dest;
    }

    constexpr uint256_t bswap(uint256_t const & source) noexcept {
        uint256_t dest;
        uint8_t const * const s = source.data;
        uint8_t * const d = dest.data;
        for(nsize_t i=0; i<32; i++) {
            d[i] = s[31-i];
        }
        return dest;
    }

    /**
    // *************************************************
    // *************************************************
    // *************************************************
     */

    // The pragma to stop multichar warning == error `-Werror=multichar` doesn't seem to work with GCC <= 10.2
    // Hence we have to disable this specific warning via: `-Wno-multichar`
    // until all our compiler support `__builtin_bit_cast(T, a)`

    PRAGMA_DISABLE_WARNING_PUSH
    PRAGMA_DISABLE_WARNING_MULTICHAR

    namespace impl {
        constexpr uint32_t get_host_order() noexcept {
            if( jau::is_builtin_bit_cast_available() ) {
                constexpr uint8_t b[4] { 0x44, 0x43, 0x42, 0x41 }; // h->l: 41 42 43 44 = 'ABCD' hex ASCII code
                return jau::bit_cast<uint32_t, uint8_t[4]>( b );
            } else {
                return 'ABCD'; // h->l: 41 42 43 44 = 'ABCD' hex ASCII code
            }
        }
    }

    /**
     * Endian identifier, indicating endianess of all scaler types.
     * <p>
     * Inspired by C++20 std::endian
     * </p>
     * <p>
     * Corner case platforms currently not supported,
     * i.e. unified endianess and mixed endianess.
     * </p>
     * <p>
     * All endian API entries are of `constexpr` and hence evaluated at compile time.<br>
     * Therefore, if-branches and expressions are also of `constexpr` and optimized 'away' at compile time.<br>
     * This includes the `cpu_to_<endian>(..)` and `<endian>_to_cpu(..)` etc utility functions.
     * </p>
     */
    enum class endian : uint32_t
    {
        /** Identifier for little endian. */
        little      = 0x41424344U, // h->l: 41 42 43 44 = 'ABCD' hex ASCII code

        /** Identifier for big endian. */
        big         = 0x44434241U, // h->l: 44 43 42 41 = 'DCBA' hex ASCII code

        /** Identifier for DEC PDP-11, aka `ENDIAN_LITTLE_WORD`. */
        pdp         = 0x43444142U, // h->l: 43 44 41 42 = 'CDAB' hex ASCII code

        /** Identifier for Honeywell 316, aka `ENDIAN_BIG_WORD`. */
        honeywell   = 0x42414443U, // h->l: 42 41 44 43 = 'BADC' hex ASCII code

        /** Undetermined endian */
        undefined   = 0x00000000U,

        /** Identifier for native platform type, one of the above. */
        native      = impl::get_host_order()
    };

    PRAGMA_DISABLE_WARNING_POP

    /**
     * Return std::string representation of the given jau::endian.
     * @param v the jau::endian value
     * @return the std::string representation
     */
    constexpr_func_cxx20 std::string to_string(const endian& v) noexcept {
        switch(v) {
            case endian::little:  return "little";
            case endian::big:  return "big";
            case endian::pdp:  return "pdb";
            case endian::honeywell: return "honeywell";
            case endian::undefined: return "undefined";
        }
        return "unlisted";
    }

    /**
     * Evaluates `true` if the given endian is defined,
     * i.e. `little`, `big`, `pdp` or `honeywell`.
     */
    constexpr bool isDefinedEndian(const endian &v) noexcept {
        switch(v) {
            case endian::little:
                [[fallthrough]];
            case endian::big:
                [[fallthrough]];
            case endian::pdp:
                [[fallthrough]];
            case endian::honeywell:
                return true;
            default:
                return false;
        }
    }

    /**
     * Evaluates `true` if platform is running in little endian mode,
     * i.e. `jau::endian::little == jau::endian::native`.
     */
    constexpr bool isLittleEndian() noexcept {
        return endian::little == endian::native;
    }

    /**
     * Evaluates `true` if platform is running in big endian mode,
     * i.e. `jau::endian::big == jau::endian::native`.
     */
    constexpr bool isBigEndian() noexcept {
        return endian::big == endian::native;
    }

    /**
     * Evaluates `true` if platform is running in little or big endian mode,
     * i.e. `jau::endian::little == jau::endian::native || jau::endian::big == jau::endian::native`.
     */
    constexpr bool isLittleOrBigEndian() noexcept {
        return jau::endian::little == jau::endian::native || jau::endian::big == jau::endian::native;
    }

    /**
     * A little-endian type trait for convenience ..
     * <p>
     * Since all endian definitions are of `constexpr`, code can simply use expressions of these
     * for compile-time evaluation and optimization. A template `SFINAE` is not required.
     * </p>
     * @tparam Dummy_type just to make template `SFINAE` happy
     */
    template <typename Dummy_type> struct has_endian_little : std::integral_constant<bool, endian::little == endian::native> {};

    /**
     * Value access of little-endian type trait for convenience ..
     * <p>
     * Since all endian definitions are of `constexpr`, code can simply use expressions of these
     * for compile-time evaluation and optimization. A template `SFINAE` is not required.
     * </p>
     * @tparam Dummy_type just to make template `SFINAE` happy
     */
    template <typename Dummy_type> inline constexpr bool has_endian_little_v = has_endian_little<Dummy_type>::value;

    /**
     * A big-endian type trait for convenience ..
     * <p>
     * Since all endian definitions are of `constexpr`, code can simply use expressions of these
     * for compile-time evaluation and optimization. A template `SFINAE` is not required.
     * </p>
     * @tparam Dummy_type just to make template `SFINAE` happy
     */
    struct has_endian_big : std::integral_constant<bool, endian::big == endian::native> {};

    /**
     * Value access of big-endian type trait for convenience ..
     * <p>
     * Since all endian definitions are of `constexpr`, code can simply use expressions of these
     * for compile-time evaluation and optimization. A template `SFINAE` is not required.
     * </p>
     * @tparam Dummy_type just to make template `SFINAE` happy
     */
    inline constexpr bool has_endian_big_v = has_endian_big::value;

    /**
    // *************************************************
    // *************************************************
    // *************************************************
     */

    /**
     * On the i386 the host byte order is Least Significant Byte first (LSB) or Little-Endian,
     * whereas the network byte order, as used on the Internet, is Most Significant Byte first (MSB) or Big-Endian.
     * See #include <arpa/inet.h>
     *
     * Bluetooth is LSB or Little-Endian!
     */

    constexpr uint16_t be_to_cpu(uint16_t const n) noexcept {
        static_assert(isLittleOrBigEndian()); // one static_assert is sufficient for whole compilation unit
        if( isLittleEndian() ) {
            return bswap(n);
        } else if( isBigEndian() ) {
            return bswap(n);
        } else {
            return 0; // unreachable -> static_assert(..) above
        }
    }
    constexpr uint16_t cpu_to_be(uint16_t const h) noexcept {
        if( isLittleEndian() ) {
            return bswap(h);
        } else if( isBigEndian() ) {
            return h;
        } else {
            return 0; // unreachable -> static_assert(..) above
        }
    }
    constexpr uint16_t le_to_cpu(uint16_t const l) noexcept {
        if( isLittleEndian() ) {
            return l;
        } else if( isBigEndian() ) {
            return bswap(l);
        } else {
            return 0; // unreachable -> static_assert(..) above
        }
    }
    constexpr uint16_t cpu_to_le(uint16_t const h) noexcept {
        if( isLittleEndian() ) {
            return h;
        } else if( isBigEndian() ) {
            return bswap(h);
        } else {
            return 0; // unreachable -> static_assert(..) above
        }
    }

    constexpr uint32_t be_to_cpu(uint32_t const n) noexcept {
        if( isLittleEndian() ) {
            return bswap(n);
        } else if( isBigEndian() ) {
            return n;
        } else {
            return 0; // unreachable -> static_assert(..) above
        }
    }
    constexpr uint32_t cpu_to_be(uint32_t const h) noexcept {
        if( isLittleEndian() ) {
            return bswap(h);
        } else if( isBigEndian() ) {
            return h;
        } else {
            return 0; // unreachable -> static_assert(..) above
        }
    }
    constexpr uint32_t le_to_cpu(uint32_t const l) noexcept {
        if( isLittleEndian() ) {
            return l;
        } else if( isBigEndian() ) {
            return bswap(l);
        } else {
            return 0; // unreachable -> static_assert(..) above
        }
    }
    constexpr uint32_t cpu_to_le(uint32_t const h) noexcept {
        if( isLittleEndian() ) {
            return h;
        } else if( isBigEndian() ) {
            return bswap(h);
        } else {
            return 0; // unreachable -> static_assert(..) above
        }
    }

    constexpr uint64_t be_to_cpu(uint64_t const & n) noexcept {
        if( isLittleEndian() ) {
            return bswap(n);
        } else if( isBigEndian() ) {
            return n;
        } else {
            return 0; // unreachable -> static_assert(..) above
        }
    }
    constexpr uint64_t cpu_to_be(uint64_t const & h) noexcept {
        if( isLittleEndian() ) {
            return bswap(h);
        } else if( isBigEndian() ) {
            return h;
        } else {
            return 0; // unreachable -> static_assert(..) above
        }
    }
    constexpr uint64_t le_to_cpu(uint64_t const & l) noexcept {
        if( isLittleEndian() ) {
            return l;
        } else if( isBigEndian() ) {
            return bswap(l);
        } else {
            return 0; // unreachable -> static_assert(..) above
        }
    }
    constexpr uint64_t cpu_to_le(uint64_t const & h) noexcept {
        if( isLittleEndian() ) {
            return h;
        } else if( isBigEndian() ) {
            return bswap(h);
        } else {
            return 0; // unreachable -> static_assert(..) above
        }
    }

    constexpr uint128_t be_to_cpu(uint128_t const & n) noexcept {
        if( isLittleEndian() ) {
            return bswap(n);
        } else if( isBigEndian() ) {
            return n;
        } else {
            return uint128_t(); // unreachable -> static_assert(..) above
        }
    }
    constexpr uint128_t cpu_to_be(uint128_t const & h) noexcept {
        if( isLittleEndian() ) {
            return bswap(h);
        } else if( isBigEndian() ) {
            return h;
        } else {
            return uint128_t(); // unreachable -> static_assert(..) above
        }
    }
    constexpr uint128_t le_to_cpu(uint128_t const & l) noexcept {
        if( isLittleEndian() ) {
            return l;
        } else if( isBigEndian() ) {
            return bswap(l);
        } else {
            return uint128_t(); // unreachable -> static_assert(..) above
        }
    }
    constexpr uint128_t cpu_to_le(uint128_t const & h) noexcept {
        if( isLittleEndian() ) {
            return h;
        } else if( isBigEndian() ) {
            return bswap(h);
        } else {
            return uint128_t(); // unreachable -> static_assert(..) above
        }
    }

    constexpr uint192_t be_to_cpu(uint192_t const & n) noexcept {
        if( isLittleEndian() ) {
            return bswap(n);
        } else if( isBigEndian() ) {
            return n;
        } else {
            return uint192_t(); // unreachable -> static_assert(..) above
        }
    }
    constexpr uint192_t cpu_to_be(uint192_t const & h) noexcept {
        if( isLittleEndian() ) {
            return bswap(h);
        } else if( isBigEndian() ) {
            return h;
        } else {
            return uint192_t(); // unreachable -> static_assert(..) above
        }
    }
    constexpr uint192_t le_to_cpu(uint192_t const & l) noexcept {
        if( isLittleEndian() ) {
            return l;
        } else if( isBigEndian() ) {
            return bswap(l);
        } else {
            return uint192_t(); // unreachable -> static_assert(..) above
        }
    }
    constexpr uint192_t cpu_to_le(uint192_t const & h) noexcept {
        if( isLittleEndian() ) {
            return h;
        } else if( isBigEndian() ) {
            return bswap(h);
        } else {
            return uint192_t(); // unreachable -> static_assert(..) above
        }
    }

    constexpr uint256_t be_to_cpu(uint256_t const & n) noexcept {
        if( isLittleEndian() ) {
            return bswap(n);
        } else if( isBigEndian() ) {
            return n;
        } else {
            return uint256_t(); // unreachable -> static_assert(..) above
        }
    }
    constexpr uint256_t cpu_to_be(uint256_t const & h) noexcept {
        if( isLittleEndian() ) {
            return bswap(h);
        } else if( isBigEndian() ) {
            return h;
        } else {
            return uint256_t(); // unreachable -> static_assert(..) above
        }
    }
    constexpr uint256_t le_to_cpu(uint256_t const & l) noexcept {
        if( isLittleEndian() ) {
            return l;
        } else if( isBigEndian() ) {
            return bswap(l);
        } else {
            return uint256_t(); // unreachable -> static_assert(..) above
        }
    }
    constexpr uint256_t cpu_to_le(uint256_t const & h) noexcept {
        if( isLittleEndian() ) {
            return h;
        } else if( isBigEndian() ) {
            return bswap(h);
        } else {
            return uint256_t(); // unreachable -> static_assert(..) above
        }

    }

    /**
    // *************************************************
    // *************************************************
    // *************************************************
     */

    constexpr void put_uint8(uint8_t * buffer, nsize_t const byte_offset, const uint8_t v) noexcept
    {
        *pointer_cast<uint8_t *>( buffer + byte_offset ) = v;
    }
    constexpr uint8_t get_uint8(uint8_t const * buffer, nsize_t const byte_offset) noexcept
    {
        return *pointer_cast<uint8_t const *>( buffer + byte_offset );
    }
    constexpr int8_t get_int8(uint8_t const * buffer, nsize_t const byte_offset) noexcept
    {
        return *pointer_cast<int8_t const *>( buffer + byte_offset );
    }

    /**
     * Safe access to a pointer cast from unaligned memory via __packed__ attribute,
     * i.e. utilizing compiler generated safe load and store operations.
     * <p>
     * This template shall cause no costs, the cast data pointer is identical to 'T & p = &store'.
     * </p>
     */
    template<typename T> __pack ( struct packed_t {
        T store;
        constexpr T get(const bool littleEndian) const noexcept { return littleEndian ? le_to_cpu(store) : be_to_cpu(store); }
    } ) ;

    constexpr void put_uint16(uint8_t * buffer, nsize_t const byte_offset, const uint16_t v) noexcept
    {
        /**
         * Handle potentially misaligned address of buffer + byte_offset, can't just
         *   uint16_t * p = (uint16_t *) ( buffer + byte_offset );
         *   *p = v;
         * Universal alternative using memcpy is costly:
         *   memcpy(buffer + byte_offset, &v, sizeof(v));
         * Use compiler magic 'struct __attribute__((__packed__))' access:
         */
        pointer_cast<packed_t<uint16_t>*>( buffer + byte_offset )->store = v;
    }
    constexpr void put_uint16(uint8_t * buffer, nsize_t const byte_offset, const uint16_t v, const bool littleEndian) noexcept
    {
        /**
         * Handle potentially misaligned address of buffer + byte_offset, can't just
         *   uint16_t * p = (uint16_t *) ( buffer + byte_offset );
         *   *p = littleEndian ? cpu_to_le(v) : cpu_to_be(v);
         * Universal alternative using memcpy is costly:
         *   const uint16_t v2 = littleEndian ? cpu_to_le(v) : cpu_to_be(v);
         *   memcpy(buffer + byte_offset, &v2, sizeof(v2));
         * Use compiler magic 'struct __attribute__((__packed__))' access:
         */
        pointer_cast<packed_t<uint16_t>*>( buffer + byte_offset )->store = littleEndian ? cpu_to_le(v) : cpu_to_be(v);
    }
    constexpr uint16_t get_uint16(uint8_t const * buffer, nsize_t const byte_offset) noexcept
    {
        /**
         * Handle potentially misaligned address of buffer + byte_offset, can't just
         *   uint16_t const * p = (uint16_t const *) ( buffer + byte_offset );
         *   return *p;
         * Universal alternative using memcpy is costly:
         *   uint16_t v;
         *   memcpy(&v, buffer + byte_offset, sizeof(v));
         *   return v;
         * Use compiler magic 'struct __attribute__((__packed__))' access:
         */
        return pointer_cast<const packed_t<uint16_t>*>( buffer + byte_offset )->store;
    }
    constexpr uint16_t get_uint16(uint8_t const * buffer, nsize_t const byte_offset, const bool littleEndian) noexcept
    {
        /**
         * Handle potentially misaligned address of buffer + byte_offset, can't just
         *   uint16_t const * p = (uint16_t const *) ( buffer + byte_offset );
         *   return littleEndian ? le_to_cpu(*p) : be_to_cpu(*p);
         * Universal alternative using memcpy is costly:
         *   uint16_t v;
         *   memcpy(&v, buffer + byte_offset, sizeof(v));
         *   return littleEndian ? le_to_cpu(v) : be_to_cpu(v);
         * Use compiler magic 'struct __attribute__((__packed__))' access:
         */
        return pointer_cast<const packed_t<uint16_t>*>( buffer + byte_offset )->get(littleEndian);
    }

    constexpr void put_uint32(uint8_t * buffer, nsize_t const byte_offset, const uint32_t v) noexcept
    {
        pointer_cast<packed_t<uint32_t>*>( buffer + byte_offset )->store = v;
    }
    constexpr void put_uint32(uint8_t * buffer, nsize_t const byte_offset, const uint32_t v, const bool littleEndian) noexcept
    {
        pointer_cast<packed_t<uint32_t>*>( buffer + byte_offset )->store = littleEndian ? cpu_to_le(v) : cpu_to_be(v);
    }
    constexpr uint32_t get_uint32(uint8_t const * buffer, nsize_t const byte_offset) noexcept
    {
        return pointer_cast<const packed_t<uint32_t>*>( buffer + byte_offset )->store;
    }
    constexpr uint32_t get_uint32(uint8_t const * buffer, nsize_t const byte_offset, const bool littleEndian) noexcept
    {
        return pointer_cast<const packed_t<uint32_t>*>( buffer + byte_offset )->get(littleEndian);
    }

    constexpr void put_uint64(uint8_t * buffer, nsize_t const byte_offset, const uint64_t & v) noexcept
    {
        pointer_cast<packed_t<uint64_t>*>( buffer + byte_offset )->store = v;
    }
    constexpr void put_uint64(uint8_t * buffer, nsize_t const byte_offset, const uint64_t & v, const bool littleEndian) noexcept
    {
        pointer_cast<packed_t<uint64_t>*>( buffer + byte_offset )->store = littleEndian ? cpu_to_le(v) : cpu_to_be(v);
    }
    constexpr uint64_t get_uint64(uint8_t const * buffer, nsize_t const byte_offset) noexcept
    {
        return pointer_cast<const packed_t<uint64_t>*>( buffer + byte_offset )->store;
    }
    constexpr uint64_t get_uint64(uint8_t const * buffer, nsize_t const byte_offset, const bool littleEndian) noexcept
    {
        return pointer_cast<const packed_t<uint64_t>*>( buffer + byte_offset )->get(littleEndian);
    }

    constexpr void put_uint128(uint8_t * buffer, nsize_t const byte_offset, const uint128_t & v) noexcept
    {
        pointer_cast<packed_t<uint128_t>*>( buffer + byte_offset )->store = v;
    }
    constexpr void put_uint128(uint8_t * buffer, nsize_t const byte_offset, const uint128_t & v, const bool littleEndian) noexcept
    {
        pointer_cast<packed_t<uint128_t>*>( buffer + byte_offset )->store = littleEndian ? cpu_to_le(v) : cpu_to_be(v);
    }
    constexpr uint128_t get_uint128(uint8_t const * buffer, nsize_t const byte_offset) noexcept
    {
        return pointer_cast<const packed_t<uint128_t>*>( buffer + byte_offset )->store;
    }
    constexpr uint128_t get_uint128(uint8_t const * buffer, nsize_t const byte_offset, const bool littleEndian) noexcept
    {
        return pointer_cast<const packed_t<uint128_t>*>( buffer + byte_offset )->get(littleEndian);
    }

    constexpr void put_uint192(uint8_t * buffer, nsize_t const byte_offset, const uint192_t & v) noexcept
    {
        pointer_cast<packed_t<uint192_t>*>( buffer + byte_offset )->store = v;
    }
    constexpr void put_uint192(uint8_t * buffer, nsize_t const byte_offset, const uint192_t & v, const bool littleEndian) noexcept
    {
        pointer_cast<packed_t<uint192_t>*>( buffer + byte_offset )->store = littleEndian ? cpu_to_le(v) : cpu_to_be(v);
    }
    constexpr uint192_t get_uint192(uint8_t const * buffer, nsize_t const byte_offset) noexcept
    {
        return pointer_cast<const packed_t<uint192_t>*>( buffer + byte_offset )->store;
    }
    constexpr uint192_t get_uint192(uint8_t const * buffer, nsize_t const byte_offset, const bool littleEndian) noexcept
    {
        return pointer_cast<const packed_t<uint192_t>*>( buffer + byte_offset )->get(littleEndian);
    }

    constexpr void put_uint256(uint8_t * buffer, nsize_t const byte_offset, const uint256_t & v) noexcept
    {
        pointer_cast<packed_t<uint256_t>*>( buffer + byte_offset )->store = v;
    }
    constexpr void put_uint256(uint8_t * buffer, nsize_t const byte_offset, const uint256_t & v, const bool littleEndian) noexcept
    {
        pointer_cast<packed_t<uint256_t>*>( buffer + byte_offset )->store = littleEndian ? cpu_to_le(v) : cpu_to_be(v);
    }
    constexpr uint256_t get_uint256(uint8_t const * buffer, nsize_t const byte_offset) noexcept
    {
        return pointer_cast<const packed_t<uint256_t>*>( buffer + byte_offset )->store;
    }
    constexpr uint256_t get_uint256(uint8_t const * buffer, nsize_t const byte_offset, const bool littleEndian) noexcept
    {
        return pointer_cast<const packed_t<uint256_t>*>( buffer + byte_offset )->get(littleEndian);
    }

    /**
    // *************************************************
    // *************************************************
    // *************************************************
     */

    template<typename T>
    constexpr
    typename std::enable_if_t<
        std::is_standard_layout_v<T>,
        void>
    put_value(uint8_t * buffer, nsize_t const byte_offset, const T& v) noexcept
    {
        // reinterpret_cast<packed_t<T>*>( buffer + byte_offset )->store = v;
        pointer_cast<packed_t<T>*>( buffer + byte_offset )->store = v;
    }

    template<typename T>
    constexpr
    typename std::enable_if_t<
        std::is_standard_layout_v<T>,
        void>
    put_value(uint8_t * buffer, nsize_t const byte_offset, const T& v, const bool littleEndian) noexcept
    {
        pointer_cast<packed_t<T>*>( buffer + byte_offset )->store = littleEndian ? cpu_to_le(v) : cpu_to_be(v);
    }

    template<typename T>
    constexpr
    typename std::enable_if_t<
        std::is_standard_layout_v<T>,
        T>
    get_value(uint8_t const * buffer, nsize_t const byte_offset) noexcept
    {
        return pointer_cast<const packed_t<T>*>( buffer + byte_offset )->store;
    }

    template<typename T>
    constexpr
    typename std::enable_if_t<
        std::is_standard_layout_v<T>,
        T>
    get_value(uint8_t const * buffer, nsize_t const byte_offset, const bool littleEndian) noexcept
    {
        return pointer_cast<const packed_t<T>*>( buffer + byte_offset )->get(littleEndian);
    }

} // namespace jau

/** \example test_basictypeconv.cpp
 * This C++ unit test validates the jau::bswap and get/set value implementation
 */

#endif /* JAU_BYTE_UTIL_HPP_ */
