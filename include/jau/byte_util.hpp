/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020-2024 Gothel Software e.K.
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

#include <array>
#include <cstring>
#include <string>
#include <cstdint>
#include <type_traits>

#include <jau/cpp_lang_util.hpp>
#include <jau/cpp_pragma.hpp>
#include <jau/packed_attribute.hpp>

#include <jau/int_types.hpp>
#include <jau/type_concepts.hpp>

namespace jau {

    /** @defgroup ByteUtils Byte Utilities
     * Byte utility functions and types for endian- and bit conversions,
     * inclusive alignment handling and general get & put functionality.
     *
     * This category is also supporting \ref Integer.
     *
     * All \ref endian API entries are of `constexpr` and hence evaluated at compile time.<br>
     * Therefore, if-branches and expressions are also of `constexpr` and optimized 'away' at compile time.<br>
     * This includes the `cpu_to_<endian>(..)` and `<endian>_to_cpu(..)` etc utility functions.
     *
     * See \ref endian enum class regarding endian `constexpr` compile time determination.
     *
     * Aligned memory transfer from and to potentially unaligned memory
     * is performed via put_uint16(), get_uint16() with all its explicit stdint types,
     * as well as the generic template functions put_value() and get_value().
     * The implementation uses \ref packed_t to resolve a potential memory alignment issue *free of costs*,
     * see \ref packed_t_alignment_cast.
     *
     * @{
     */

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

    // Remember: constexpr specifier used in a function or static data member (since C++17) declaration implies inline.

    constexpr uint16_t bswap(uint16_t const source) noexcept {
        #if defined __has_builtin_bswap16
            return __builtin_bswap16(source);
        #else
            return (uint16_t) ( ( ( (source) >> 8 ) & 0xff ) |
                                ( ( (source) & 0xff) << 8 ) );
        #endif
    }

    constexpr int16_t bswap(int16_t const source) noexcept {
        #if defined __has_builtin_bswap64
            return jau::bit_cast<int16_t>( __builtin_bswap16(jau::bit_cast<uint16_t>(source)) );
        #else
            return (int16_t) ( ( ( (source) >> 8 ) & 0xff ) |
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

    constexpr int32_t bswap(int32_t const source) noexcept {
        #if defined __has_builtin_bswap64
            return jau::bit_cast<int32_t>( __builtin_bswap32(jau::bit_cast<uint32_t>(source)) );
        #else
            return jau::bit_cast<int32_t>( ( ( source & 0xff000000U ) >> 24 ) |
                                           ( ( source & 0x00ff0000U ) >>  8 ) |
                                           ( ( source & 0x0000ff00U ) <<  8 ) |
                                           ( ( source & 0x000000ffU ) << 24 ) );
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

    constexpr int64_t bswap(int64_t const & source) noexcept {
        #if defined __has_builtin_bswap64
            return jau::bit_cast<int64_t>( __builtin_bswap64(jau::bit_cast<uint64_t>(source)) );
        #else
            return jau::bit_cast<int64_t>( ( ( source & 0xff00000000000000ULL ) >> 56 ) |
                                           ( ( source & 0x00ff000000000000ULL ) >> 40 ) |
                                           ( ( source & 0x0000ff0000000000ULL ) >> 24 ) |
                                           ( ( source & 0x000000ff00000000ULL ) >>  8 ) |
                                           ( ( source & 0x00000000ff000000ULL ) <<  8 ) |
                                           ( ( source & 0x0000000000ff0000ULL ) << 24 ) |
                                           ( ( source & 0x000000000000ff00ULL ) << 40 ) |
                                           ( ( source & 0x00000000000000ffULL ) << 56 ) );
        #endif
    }

    constexpr void bswap(uint8_t * sink, uint8_t const * source, nsize_t len) {
        source += len - 1;
        for (; len > 0; len--) {
            *sink++ = *source--;
        }
    }

    constexpr uint128dp_t bswap(uint128dp_t const & source) noexcept {
        uint128dp_t dest;
        bswap(dest.data, source.data, 16);
        return dest;
    }

    constexpr uint192dp_t bswap(uint192dp_t const & source) noexcept {
        uint192dp_t dest;
        bswap(dest.data, source.data, 24);
        return dest;
    }

    constexpr uint256dp_t bswap(uint256dp_t const & source) noexcept {
        uint256dp_t dest;
        bswap(dest.data, source.data, 32);
        return dest;
    }

    inline char* cast_uint8_ptr_to_char(uint8_t* b) noexcept {
        return reinterpret_cast<char*>(b);
    }
    inline const char* cast_uint8_ptr_to_char(const uint8_t* b) noexcept {
        return reinterpret_cast<const char*>(b);
    }

    inline const uint8_t* cast_char_ptr_to_uint8(const char* s) noexcept {
       return reinterpret_cast<const uint8_t*>(s);
    }

    /**
    // *************************************************
    // *************************************************
    // *************************************************
     */

    PRAGMA_DISABLE_WARNING_PUSH
    PRAGMA_DISABLE_WARNING_MULTICHAR

    namespace impl {
        constexpr uint32_t get_host_order() noexcept {
            if constexpr ( jau::has_builtin_bit_cast() ) {
                constexpr uint8_t b[4] { 0x44, 0x43, 0x42, 0x41 }; // h->l: 41 42 43 44 = 'ABCD' hex ASCII code
                return jau::bit_cast<uint32_t, uint8_t[4]>( b );
            } else {
                // The pragma to stop multichar warning == error `-Werror=multichar` doesn't seem to work with GCC <= 10.2
                // Hence we have to disable this specific warning via: `-Wno-multichar`
                // until all our compiler support `__builtin_bit_cast(T, a)`
                return 'ABCD'; // h->l: 41 42 43 44 = 'ABCD' hex ASCII code
            }
        }
    }

    /**
     * Endian identifier, indicating endianess of all scalar types.
     *
     * Inspired by C++20 std::endian
     *
     * Corner case platforms currently not supported,
     * i.e. unified endianess and mixed endianess.
     *
     * All endian API entries are of `constexpr` and hence evaluated at compile time.<br>
     * Therefore, if-branches and expressions are also of `constexpr` and optimized 'away' at compile time.<br>
     * This includes the `cpu_to_<endian>(..)` and `<endian>_to_cpu(..)` etc utility functions.
     *
     * On i386 platforms the host byte order is Least Significant Byte first (LSB) or Little-Endian,
     * whereas the network byte order, as used on the Internet, is Most Significant Byte first (MSB) or Big-Endian.
     * See #include <arpa/inet.h>
     *
     * Bluetooth is LSB or Little-Endian!
     *
     * See \ref lb_endian
     */
    enum class endian_t : uint32_t
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

    /** Simplified reduced \ref endian type only covering little- and big-endian. See \ref endian for details. */
    enum class lb_endian_t : uint32_t
    {
        /** Identifier for little endian, equivalent to endian::little. */
        little      = static_cast<uint32_t>( endian_t::little ),

        /** Identifier for big endian, equivalent to endian::big. */
        big         = static_cast<uint32_t>( endian_t::big ),

        /** Identifier for native platform type, one of the above. */
        native = static_cast<uint32_t>( endian_t::native )
    };

    /**
     * Return std::string representation of the given \ref endian.
     * @param v the \ref endian value
     * @return the std::string representation
     */
    std::string to_string(const endian_t v) noexcept;

    /**
     * Return std::string representation of the given \ref lb_endian.
     * @param v the \ref lb_endian value
     * @return the std::string representation
     */
    std::string to_string(const lb_endian_t v) noexcept;

    constexpr lb_endian_t to_lb_endian(const endian_t v) noexcept {
        switch(v) {
            case endian_t::little: return lb_endian_t::little;
            case endian_t::big: return lb_endian_t::big;
            default: {
                abort(); // never reached
            }
        }
    }
    constexpr endian_t to_endian(const lb_endian_t v) noexcept {
        switch(v) {
            case lb_endian_t::little: return endian_t::little;
            case lb_endian_t::big: return endian_t::big;
            default: {
                abort(); // never reached
            }
        }
    }

    /**
     * Evaluates `true` if the given \ref endian is defined,
     * i.e. `little`, `big`, `pdp` or `honeywell`.
     */
    constexpr bool is_defined_endian(const endian_t &v) noexcept {
        switch(v) {
            case endian_t::little:
                [[fallthrough]];
            case endian_t::big:
                [[fallthrough]];
            case endian_t::pdp:
                [[fallthrough]];
            case endian_t::honeywell:
                return true;
            default:
                return false;
        }
    }

    /**
     * Returns `true` if given `byte_order` equals endian::little, otherwise false.
     */
    constexpr bool is_little_endian(const endian_t byte_order) noexcept {
        return endian_t::little == byte_order;
    }

    /**
     * Returns `true` if given `byte_order` equals lb_endian::little, otherwise false.
     */
    constexpr bool is_little_endian(const lb_endian_t byte_order) noexcept {
        return lb_endian_t::little == byte_order;
    }

    /**
     * Evaluates `true` if platform is running in little \ref endian mode,
     * i.e. `jau::endian::little == jau::endian::native`.
     */
    constexpr bool is_little_endian() noexcept {
        return endian_t::little == endian_t::native;
    }

    /**
     * Evaluates `true` if platform is running in big \ref endian mode,
     * i.e. `jau::endian::big == jau::endian::native`.
     */
    constexpr bool is_big_endian() noexcept {
        return endian_t::big == endian_t::native;
    }

    /**
     * Evaluates `true` if platform is running in little or big \ref endian mode,
     * i.e. `jau::endian::little == jau::endian::native || jau::endian::big == jau::endian::native`.
     */
    constexpr bool is_little_or_big_endian() noexcept {
        return jau::endian_t::little == jau::endian_t::native || jau::endian_t::big == jau::endian_t::native;
    }

    /**
     * A little-endian type trait for convenience ..
     * <p>
     * Since all \ref endian definitions are of `constexpr`, code can simply use expressions of these
     * for compile-time evaluation and optimization. A template `SFINAE` is not required.
     * </p>
     * @tparam Dummy_type just to make template `SFINAE` happy
     */
    template <typename Dummy_type> struct has_endian_little : std::integral_constant<bool, endian_t::little == endian_t::native> {};

    /**
     * Value access of little-endian type trait for convenience ..
     * <p>
     * Since all \ref endian definitions are of `constexpr`, code can simply use expressions of these
     * for compile-time evaluation and optimization. A template `SFINAE` is not required.
     * </p>
     * @tparam Dummy_type just to make template `SFINAE` happy
     */
    template <typename Dummy_type> constexpr bool has_endian_little_v = has_endian_little<Dummy_type>::value;

    /**
     * A big-endian type trait for convenience ..
     * <p>
     * Since all \ref endian definitions are of `constexpr`, code can simply use expressions of these
     * for compile-time evaluation and optimization. A template `SFINAE` is not required.
     * </p>
     * @tparam Dummy_type just to make template `SFINAE` happy
     */
    template <typename Dummy_type> struct has_endian_big : std::integral_constant<bool, endian_t::big == endian_t::native> {};

    /**
     * Value access of big-endian type trait for convenience ..
     * <p>
     * Since all \ref endian definitions are of `constexpr`, code can simply use expressions of these
     * for compile-time evaluation and optimization. A template `SFINAE` is not required.
     * </p>
     * @tparam Dummy_type just to make template `SFINAE` happy
     */
    template <typename Dummy_type> constexpr bool has_endian_big_v = has_endian_big<Dummy_type>::value;

    /**
    // *************************************************
    // *************************************************
    // *************************************************
     */

    // one static_assert is sufficient for whole compilation unit
    static_assert( is_defined_endian(endian_t::native) );
    static_assert( is_little_or_big_endian() );

    constexpr uint16_t be_to_cpu(uint16_t const n) noexcept {
        if constexpr ( is_little_endian() ) {
            return bswap(n);
        } else if constexpr ( is_big_endian() ) {
            return n;
        } else {
            return 0; // unreachable -> static_assert(..) above
        }
    }
    constexpr uint16_t cpu_to_be(uint16_t const h) noexcept {
        if constexpr ( is_little_endian() ) {
            return bswap(h);
        } else if constexpr ( is_big_endian() ) {
            return h;
        } else {
            return 0; // unreachable -> static_assert(..) above
        }
    }
    constexpr uint16_t le_to_cpu(uint16_t const l) noexcept {
        if constexpr ( is_little_endian() ) {
            return l;
        } else if constexpr ( is_big_endian() ) {
            return bswap(l);
        } else {
            return 0; // unreachable -> static_assert(..) above
        }
    }
    constexpr uint16_t cpu_to_le(uint16_t const h) noexcept {
        if constexpr ( is_little_endian() ) {
            return h;
        } else if constexpr ( is_big_endian() ) {
            return bswap(h);
        } else {
            return 0; // unreachable -> static_assert(..) above
        }
    }

    constexpr int16_t be_to_cpu(int16_t const n) noexcept {
        if constexpr ( is_little_endian() ) {
            return bswap(n);
        } else if constexpr ( is_big_endian() ) {
            return n;
        } else {
            return 0; // unreachable -> static_assert(..) above
        }
    }
    constexpr int16_t cpu_to_be(int16_t const h) noexcept {
        if constexpr ( is_little_endian() ) {
            return bswap(h);
        } else if constexpr ( is_big_endian() ) {
            return h;
        } else {
            return 0; // unreachable -> static_assert(..) above
        }
    }
    constexpr int16_t le_to_cpu(int16_t const l) noexcept {
        if constexpr ( is_little_endian() ) {
            return l;
        } else if constexpr ( is_big_endian() ) {
            return bswap(l);
        } else {
            return 0; // unreachable -> static_assert(..) above
        }
    }
    constexpr int16_t cpu_to_le(int16_t const h) noexcept {
        if constexpr ( is_little_endian() ) {
            return h;
        } else if constexpr ( is_big_endian() ) {
            return bswap(h);
        } else {
            return 0; // unreachable -> static_assert(..) above
        }
    }

    constexpr uint32_t be_to_cpu(uint32_t const n) noexcept {
        if constexpr ( is_little_endian() ) {
            return bswap(n);
        } else if constexpr ( is_big_endian() ) {
            return n;
        } else {
            return 0; // unreachable -> static_assert(..) above
        }
    }
    constexpr uint32_t cpu_to_be(uint32_t const h) noexcept {
        if constexpr ( is_little_endian() ) {
            return bswap(h);
        } else if constexpr ( is_big_endian() ) {
            return h;
        } else {
            return 0; // unreachable -> static_assert(..) above
        }
    }
    constexpr uint32_t le_to_cpu(uint32_t const l) noexcept {
        if constexpr ( is_little_endian() ) {
            return l;
        } else if constexpr ( is_big_endian() ) {
            return bswap(l);
        } else {
            return 0; // unreachable -> static_assert(..) above
        }
    }
    constexpr uint32_t cpu_to_le(uint32_t const h) noexcept {
        if constexpr ( is_little_endian() ) {
            return h;
        } else if constexpr ( is_big_endian() ) {
            return bswap(h);
        } else {
            return 0; // unreachable -> static_assert(..) above
        }
    }

    constexpr int32_t be_to_cpu(int32_t const n) noexcept {
        if constexpr ( is_little_endian() ) {
            return bswap(n);
        } else if constexpr ( is_big_endian() ) {
            return n;
        } else {
            return 0; // unreachable -> static_assert(..) above
        }
    }
    constexpr int32_t cpu_to_be(int32_t const h) noexcept {
        if constexpr ( is_little_endian() ) {
            return bswap(h);
        } else if constexpr ( is_big_endian() ) {
            return h;
        } else {
            return 0; // unreachable -> static_assert(..) above
        }
    }
    constexpr int32_t le_to_cpu(int32_t const l) noexcept {
        if constexpr ( is_little_endian() ) {
            return l;
        } else if constexpr ( is_big_endian() ) {
            return bswap(l);
        } else {
            return 0; // unreachable -> static_assert(..) above
        }
    }
    constexpr int32_t cpu_to_le(int32_t const h) noexcept {
        if constexpr ( is_little_endian() ) {
            return h;
        } else if constexpr ( is_big_endian() ) {
            return bswap(h);
        } else {
            return 0; // unreachable -> static_assert(..) above
        }
    }

    constexpr uint64_t be_to_cpu(uint64_t const & n) noexcept {
        if constexpr ( is_little_endian() ) {
            return bswap(n);
        } else if constexpr ( is_big_endian() ) {
            return n;
        } else {
            return 0; // unreachable -> static_assert(..) above
        }
    }
    constexpr uint64_t cpu_to_be(uint64_t const & h) noexcept {
        if constexpr ( is_little_endian() ) {
            return bswap(h);
        } else if constexpr ( is_big_endian() ) {
            return h;
        } else {
            return 0; // unreachable -> static_assert(..) above
        }
    }
    constexpr uint64_t le_to_cpu(uint64_t const & l) noexcept {
        if constexpr ( is_little_endian() ) {
            return l;
        } else if constexpr ( is_big_endian() ) {
            return bswap(l);
        } else {
            return 0; // unreachable -> static_assert(..) above
        }
    }
    constexpr uint64_t cpu_to_le(uint64_t const & h) noexcept {
        if constexpr ( is_little_endian() ) {
            return h;
        } else if constexpr ( is_big_endian() ) {
            return bswap(h);
        } else {
            return 0; // unreachable -> static_assert(..) above
        }
    }

    constexpr int64_t be_to_cpu(int64_t const & n) noexcept {
        if constexpr ( is_little_endian() ) {
            return bswap(n);
        } else if constexpr ( is_big_endian() ) {
            return n;
        } else {
            return 0; // unreachable -> static_assert(..) above
        }
    }
    constexpr int64_t cpu_to_be(int64_t const & h) noexcept {
        if constexpr ( is_little_endian() ) {
            return bswap(h);
        } else if constexpr ( is_big_endian() ) {
            return h;
        } else {
            return 0; // unreachable -> static_assert(..) above
        }
    }
    constexpr int64_t le_to_cpu(int64_t const & l) noexcept {
        if constexpr ( is_little_endian() ) {
            return l;
        } else if constexpr ( is_big_endian() ) {
            return bswap(l);
        } else {
            return 0; // unreachable -> static_assert(..) above
        }
    }
    constexpr int64_t cpu_to_le(int64_t const & h) noexcept {
        if constexpr ( is_little_endian() ) {
            return h;
        } else if constexpr ( is_big_endian() ) {
            return bswap(h);
        } else {
            return 0; // unreachable -> static_assert(..) above
        }
    }

    constexpr uint128dp_t be_to_cpu(uint128dp_t const & n) noexcept {
        if constexpr ( is_little_endian() ) {
            return bswap(n);
        } else if constexpr ( is_big_endian() ) {
            return n;
        } else {
            return {}; // unreachable -> static_assert(..) above
        }
    }
    constexpr uint128dp_t cpu_to_be(uint128dp_t const & h) noexcept {
        if constexpr ( is_little_endian() ) {
            return bswap(h);
        } else if constexpr ( is_big_endian() ) {
            return h;
        } else {
            return {}; // unreachable -> static_assert(..) above
        }
    }
    constexpr uint128dp_t le_to_cpu(uint128dp_t const & l) noexcept {
        if constexpr ( is_little_endian() ) {
            return l;
        } else if constexpr ( is_big_endian() ) {
            return bswap(l);
        } else {
            return {}; // unreachable -> static_assert(..) above
        }
    }
    constexpr uint128dp_t cpu_to_le(uint128dp_t const & h) noexcept {
        if constexpr ( is_little_endian() ) {
            return h;
        } else if constexpr ( is_big_endian() ) {
            return bswap(h);
        } else {
            return {}; // unreachable -> static_assert(..) above
        }
    }

    constexpr uint192dp_t be_to_cpu(uint192dp_t const & n) noexcept {
        if constexpr ( is_little_endian() ) {
            return bswap(n);
        } else if constexpr ( is_big_endian() ) {
            return n;
        } else {
            return {}; // unreachable -> static_assert(..) above
        }
    }
    constexpr uint192dp_t cpu_to_be(uint192dp_t const & h) noexcept {
        if constexpr ( is_little_endian() ) {
            return bswap(h);
        } else if constexpr ( is_big_endian() ) {
            return h;
        } else {
            return {}; // unreachable -> static_assert(..) above
        }
    }
    constexpr uint192dp_t le_to_cpu(uint192dp_t const & l) noexcept {
        if constexpr ( is_little_endian() ) {
            return l;
        } else if constexpr ( is_big_endian() ) {
            return bswap(l);
        } else {
            return {}; // unreachable -> static_assert(..) above
        }
    }
    constexpr uint192dp_t cpu_to_le(uint192dp_t const & h) noexcept {
        if constexpr ( is_little_endian() ) {
            return h;
        } else if constexpr ( is_big_endian() ) {
            return bswap(h);
        } else {
            return {}; // unreachable -> static_assert(..) above
        }
    }

    constexpr uint256dp_t be_to_cpu(uint256dp_t const & n) noexcept {
        if constexpr ( is_little_endian() ) {
            return bswap(n);
        } else if constexpr ( is_big_endian() ) {
            return n;
        } else {
            return {}; // unreachable -> static_assert(..) above
        }
    }
    constexpr uint256dp_t cpu_to_be(uint256dp_t const & h) noexcept {
        if constexpr ( is_little_endian() ) {
            return bswap(h);
        } else if constexpr ( is_big_endian() ) {
            return h;
        } else {
            return {}; // unreachable -> static_assert(..) above
        }
    }
    constexpr uint256dp_t le_to_cpu(uint256dp_t const & l) noexcept {
        if constexpr ( is_little_endian() ) {
            return l;
        } else if constexpr ( is_big_endian() ) {
            return bswap(l);
        } else {
            return {}; // unreachable -> static_assert(..) above
        }
    }
    constexpr uint256dp_t cpu_to_le(uint256dp_t const & h) noexcept {
        if constexpr ( is_little_endian() ) {
            return h;
        } else if constexpr ( is_big_endian() ) {
            return bswap(h);
        } else {
            return {}; // unreachable -> static_assert(..) above
        }

    }

    /**
    // *************************************************
    // *************************************************
    // *************************************************
     */

    namespace impl {
        /** @see https://graphics.stanford.edu/~seander/bithacks.html#BitReverseTable */
        static constexpr std::array<uint8_t, 256> BitRevTable256 =
            []() constexpr -> std::array<uint8_t, 256> {
                using namespace jau::int_literals;
                std::array<uint8_t, 256> result{};
                for (size_t i = 0; i < 256; ++i) {
                    // result[i] = (i * 0x0202020202_u64 & 0x010884422010_u64) % 1023;
                    result[i] = ( (i * 0x80200802_u64) & 0x0884422110_u64 ) * 0x0101010101_u64 >> 32;
                }
                return result;
            }();
    }

    /**
     * Reverse bits of one byte
     * @see https://graphics.stanford.edu/~seander/bithacks.html#BitReverseTable
     */
    constexpr uint8_t rev_bits(uint8_t v) noexcept { return impl::BitRevTable256[v]; };
    /**
     * Reverse bits of two bytes
     * @see https://graphics.stanford.edu/~seander/bithacks.html#BitReverseTable
     */
    constexpr uint16_t rev_bits(uint16_t v) noexcept {
        return ( uint16_t( impl::BitRevTable256[ v        & 0xff] ) << 8) |
               ( uint16_t( impl::BitRevTable256[(v >>  8) & 0xff] )     );
    };
    /**
     * Reverse bits of four bytes
     * @see https://graphics.stanford.edu/~seander/bithacks.html#BitReverseTable
     */
    constexpr uint32_t rev_bits(uint32_t v) noexcept {
        return ( uint32_t( impl::BitRevTable256[ v        & 0xff] ) << 24) |
               ( uint32_t( impl::BitRevTable256[(v >>  8) & 0xff] ) << 16) |
               ( uint32_t( impl::BitRevTable256[(v >> 16) & 0xff] ) <<  8) |
               ( uint32_t( impl::BitRevTable256[(v >> 24) & 0xff] )      );
    };
    /**
     * Reverse bits of eight bytes
     * @see https://graphics.stanford.edu/~seander/bithacks.html#BitReverseTable
     */
    constexpr uint64_t rev_bits(uint64_t v) noexcept {
        return ( uint64_t( impl::BitRevTable256[ v        & 0xff] ) << 56) |
               ( uint64_t( impl::BitRevTable256[(v >>  8) & 0xff] ) << 48) |
               ( uint64_t( impl::BitRevTable256[(v >> 16) & 0xff] ) << 40) |
               ( uint64_t( impl::BitRevTable256[(v >> 24) & 0xff] ) << 32) |
               ( uint64_t( impl::BitRevTable256[(v >> 32) & 0xff] ) << 24) |
               ( uint64_t( impl::BitRevTable256[(v >> 40) & 0xff] ) << 16) |
               ( uint64_t( impl::BitRevTable256[(v >> 48) & 0xff] ) <<  8) |
               ( uint64_t( impl::BitRevTable256[(v >> 56) & 0xff] )      );
    };

    /** Returns the unit_type bit mask of n-bits, i.e. n low order 1’s */
    template<jau::req::unsigned_integral T>
    static constexpr T bit_mask(size_t n) noexcept {
        if ( n >= sizeof(T) * CHAR_BIT ) {
            return std::numeric_limits<T>::max();
        } else {
            return (T(1) << n) - T(1);
        }
    }

    /**
     * Reversed `n` bits of value `v`, this is an O(n) operation.
     *
     * The reversed bits will stick in their `n` bits position,
     * i.e. not shifted to the left of `n` bits as `rev_bits(v)` would.
     * @see https://graphics.stanford.edu/~seander/bithacks.html#BitReverseTable
     */
    template<jau::req::unsigned_integral T>
    constexpr T rev_bits(jau::nsize_t n, T v) {
        if ( n >= sizeof(T) * CHAR_BIT ) {
            return rev_bits(v); // reverse all bits
        }
        v &= (T(1) << n) - T(1); // mask-out undesired bits
        T r = v & 1; // r will be reversed bits of v; first get LSB of v
        jau::nsize_t s = std::min(n-1, sizeof(T) * CHAR_BIT - 1); // extra shift needed at end
        for (v >>= 1; v; v >>= 1) {
            r <<= 1;
            r |= v & 1;
            --s;
        }
        return r << s; // shift when v's highest bits are zero
    }

    /**
    // *************************************************
    // *************************************************
    // *************************************************
     */

    constexpr void put_uint8(uint8_t * buffer, const uint8_t v) noexcept
    {
        *pointer_cast<uint8_t *>( buffer ) = v;
    }
    constexpr uint8_t get_uint8(uint8_t const * buffer) noexcept
    {
        return *pointer_cast<uint8_t const *>( buffer );
    }
    constexpr int8_t get_int8(uint8_t const * buffer) noexcept
    {
        return *pointer_cast<int8_t const *>( buffer );
    }

    /**
     * Return packed_t::store after converting it to from either lb_endian::little or lb_endian::big depending on given `byte_order`
     * to lb_endian::native.
     * @tparam T
     * @param source
     * @param byte_order
     */
    template<typename T>
    constexpr T get_packed_value(const packed_t<T>* source, const lb_endian_t byte_order) noexcept {
        return is_little_endian(byte_order) ? le_to_cpu(source->store) : be_to_cpu(source->store);
    }

    /**
     * Put the given uint16_t value into the given byte address
     * using \ref packed_t to resolve a potential memory alignment issue *free of costs*.
     *
     * @see \ref packed_t_alignment_cast
     */
    constexpr void put_uint16(uint8_t * buffer, const uint16_t v) noexcept
    {
        pointer_cast<packed_t<uint16_t>*>( buffer )->store = v;
    }
    /**
     * Put the given uint16_t value into the given byte address
     * using \ref packed_t to resolve a potential memory alignment issue *free of costs*.
     *
     * The value is converted from lb_endian::native to either lb_endian::little or lb_endian::big depending on given `byte_order`
     * before it is stored in memory.
     *
     * @see \ref packed_t_alignment_cast
     */
    constexpr void put_uint16(uint8_t * buffer, const uint16_t v, const lb_endian_t byte_order) noexcept
    {
        pointer_cast<packed_t<uint16_t>*>( buffer )->store = is_little_endian(byte_order) ? cpu_to_le(v) : cpu_to_be(v);
    }
    /**
     * Returns a uint16_t value from the given byte address
     * using \ref packed_t to resolve a potential memory alignment issue *free of costs*.
     *
     * @see \ref packed_t_alignment_cast
     */
    constexpr uint16_t get_uint16(uint8_t const * buffer) noexcept
    {
        return pointer_cast<const packed_t<uint16_t>*>( buffer )->store;
    }
    /**
     * Returns a uint16_t value from the given byte address
     * using \ref packed_t to resolve a potential memory alignment issue *free of costs*.
     *
     * The value is converted from either lb_endian::little or lb_endian::big depending on given `byte_order`
     * to lb_endian::native before it is returned to the caller.
     *
     * @see \ref packed_t_alignment_cast
     */
    constexpr uint16_t get_uint16(uint8_t const * buffer, const lb_endian_t byte_order) noexcept
    {
        return get_packed_value(pointer_cast<const packed_t<uint16_t>*>( buffer ), byte_order);
    }

    /**
     * Put the given int16_t value into the given byte address
     * using \ref packed_t to resolve a potential memory alignment issue *free of costs*.
     *
     * @see \ref packed_t_alignment_cast
     */
    constexpr void put_int16(uint8_t * buffer, const int16_t v) noexcept
    {
        pointer_cast<packed_t<int16_t>*>( buffer )->store = v;
    }
    /**
     * Put the given uint16_t value into the given byte address
     * using \ref packed_t to resolve a potential memory alignment issue *free of costs*.
     *
     * The value is converted from lb_endian::native to either lb_endian::little or lb_endian::big depending on given `byte_order`
     * before it is stored in memory.
     *
     * @see \ref packed_t_alignment_cast
     */
    constexpr void put_int16(uint8_t * buffer, const int16_t v, const lb_endian_t byte_order) noexcept
    {
        pointer_cast<packed_t<int16_t>*>( buffer )->store = is_little_endian(byte_order) ? cpu_to_le(v) : cpu_to_be(v);
    }
    /**
     * Returns a int16_t value from the given byte address
     * using \ref packed_t to resolve a potential memory alignment issue *free of costs*.
     *
     * @see \ref packed_t_alignment_cast
     */
    constexpr int16_t get_int16(uint8_t const * buffer) noexcept
    {
        return pointer_cast<const packed_t<int16_t>*>( buffer )->store;
    }
    /**
     * Returns a int16_t value from the given byte address
     * using \ref packed_t to resolve a potential memory alignment issue *free of costs*.
     *
     * The value is converted from either lb_endian::little or lb_endian::big depending on given `byte_order`
     * to lb_endian::native before it is returned to the caller.
     *
     * @see \ref packed_t_alignment_cast
     */
    constexpr int16_t get_int16(uint8_t const * buffer, const lb_endian_t byte_order) noexcept
    {
        return get_packed_value(pointer_cast<const packed_t<int16_t>*>( buffer ), byte_order);
    }

    /**
     * See put_uint16() for reference.
     * @see \ref packed_t_alignment_cast
     */
    constexpr void put_uint32(uint8_t * buffer, const uint32_t v) noexcept
    {
        pointer_cast<packed_t<uint32_t>*>( buffer )->store = v;
    }
    /**
     * See put_uint16() for reference.
     * @see \ref packed_t_alignment_cast
     */
    constexpr void put_uint32(uint8_t * buffer, const uint32_t v, const lb_endian_t byte_order) noexcept
    {
        pointer_cast<packed_t<uint32_t>*>( buffer )->store = is_little_endian(byte_order) ? cpu_to_le(v) : cpu_to_be(v);
    }
    /**
     * See get_uint16() for reference.
     * @see \ref packed_t_alignment_cast
     */
    constexpr uint32_t get_uint32(uint8_t const * buffer) noexcept
    {
        return pointer_cast<const packed_t<uint32_t>*>( buffer )->store;
    }
    /**
     * See get_uint16() for reference.
     * @see \ref packed_t_alignment_cast
     */
    constexpr uint32_t get_uint32(uint8_t const * buffer, const lb_endian_t byte_order) noexcept
    {
        return get_packed_value(pointer_cast<const packed_t<uint32_t>*>( buffer ), byte_order);
    }

    /**
     * See put_int16() for reference.
     * @see \ref packed_t_alignment_cast
     */
    constexpr void put_int32(uint8_t * buffer, const int32_t v) noexcept
    {
        pointer_cast<packed_t<int32_t>*>( buffer )->store = v;
    }
    /**
     * See put_int16() for reference.
     * @see \ref packed_t_alignment_cast
     */
    constexpr void put_int32(uint8_t * buffer, const int32_t v, const lb_endian_t byte_order) noexcept
    {
        pointer_cast<packed_t<int32_t>*>( buffer )->store = is_little_endian(byte_order) ? cpu_to_le(v) : cpu_to_be(v);
    }
    /**
     * See get_int16() for reference.
     * @see \ref packed_t_alignment_cast
     */
    constexpr int32_t get_int32(uint8_t const * buffer) noexcept
    {
        return pointer_cast<const packed_t<int32_t>*>( buffer )->store;
    }
    /**
     * See get_int16() for reference.
     * @see \ref packed_t_alignment_cast
     */
    constexpr int32_t get_int32(uint8_t const * buffer, const lb_endian_t byte_order) noexcept
    {
        return get_packed_value(pointer_cast<const packed_t<int32_t>*>( buffer ), byte_order);
    }

    /**
     * See put_uint16() for reference.
     * @see \ref packed_t_alignment_cast
     */
    constexpr void put_uint64(uint8_t * buffer, const uint64_t & v) noexcept
    {
        pointer_cast<packed_t<uint64_t>*>( buffer )->store = v;
    }
    /**
     * See put_uint16() for reference.
     * @see \ref packed_t_alignment_cast
     */
    constexpr void put_uint64(uint8_t * buffer, const uint64_t & v, const lb_endian_t byte_order) noexcept
    {
        pointer_cast<packed_t<uint64_t>*>( buffer )->store = is_little_endian(byte_order) ? cpu_to_le(v) : cpu_to_be(v);
    }
    /**
     * See get_uint16() for reference.
     * @see \ref packed_t_alignment_cast
     */
    constexpr uint64_t get_uint64(uint8_t const * buffer) noexcept
    {
        return pointer_cast<const packed_t<uint64_t>*>( buffer )->store;
    }
    /**
     * See get_uint16() for reference.
     * @see \ref packed_t_alignment_cast
     */
    constexpr uint64_t get_uint64(uint8_t const * buffer, const lb_endian_t byte_order) noexcept
    {
        return get_packed_value(pointer_cast<const packed_t<uint64_t>*>( buffer ), byte_order);
    }

    /**
     * See put_int16() for reference.
     * @see \ref packed_t_alignment_cast
     */
    constexpr void put_int64(uint8_t * buffer, const int64_t & v) noexcept
    {
        pointer_cast<packed_t<int64_t>*>( buffer )->store = v;
    }
    /**
     * See put_int16() for reference.
     * @see \ref packed_t_alignment_cast
     */
    constexpr void put_int64(uint8_t * buffer, const int64_t & v, const lb_endian_t byte_order) noexcept
    {
        pointer_cast<packed_t<int64_t>*>( buffer )->store = is_little_endian(byte_order) ? cpu_to_le(v) : cpu_to_be(v);
    }
    /**
     * See get_int16() for reference.
     * @see \ref packed_t_alignment_cast
     */
    constexpr int64_t get_int64(uint8_t const * buffer) noexcept
    {
        return pointer_cast<const packed_t<int64_t>*>( buffer )->store;
    }
    /**
     * See get_int16() for reference.
     * @see \ref packed_t_alignment_cast
     */
    constexpr int64_t get_int64(uint8_t const * buffer, const lb_endian_t byte_order) noexcept
    {
        return get_packed_value(pointer_cast<const packed_t<int64_t>*>( buffer ), byte_order);
    }

    /**
     * See put_uint16() for reference.
     * @see \ref packed_t_alignment_cast
     */
    constexpr void put_uint128(uint8_t * buffer, const uint128dp_t & v) noexcept
    {
        pointer_cast<packed_t<uint128dp_t>*>( buffer )->store = v;
    }
    /**
     * See put_uint16() for reference.
     * @see \ref packed_t_alignment_cast
     */
    constexpr void put_uint128(uint8_t * buffer, const uint128dp_t & v, const lb_endian_t byte_order) noexcept
    {
        pointer_cast<packed_t<uint128dp_t>*>( buffer )->store = is_little_endian(byte_order) ? cpu_to_le(v) : cpu_to_be(v);
    }
    /**
     * See get_uint16() for reference.
     * @see \ref packed_t_alignment_cast
     */
    constexpr uint128dp_t get_uint128(uint8_t const * buffer) noexcept
    {
        return pointer_cast<const packed_t<uint128dp_t>*>( buffer )->store;
    }
    /**
     * See get_uint16() for reference.
     * @see \ref packed_t_alignment_cast
     */
    constexpr uint128dp_t get_uint128(uint8_t const * buffer, const lb_endian_t byte_order) noexcept
    {
        return get_packed_value(pointer_cast<const packed_t<uint128dp_t>*>( buffer ), byte_order);
    }

    /**
     * See put_uint16() for reference.
     * @see \ref packed_t_alignment_cast
     */
    constexpr void put_uint192(uint8_t * buffer, const uint192dp_t & v) noexcept
    {
        pointer_cast<packed_t<uint192dp_t>*>( buffer )->store = v;
    }
    /**
     * See put_uint16() for reference.
     * @see \ref packed_t_alignment_cast
     */
    constexpr void put_uint192(uint8_t * buffer, const uint192dp_t & v, const lb_endian_t byte_order) noexcept
    {
        pointer_cast<packed_t<uint192dp_t>*>( buffer )->store = is_little_endian(byte_order) ? cpu_to_le(v) : cpu_to_be(v);
    }
    /**
     * See get_uint16() for reference.
     * @see \ref packed_t_alignment_cast
     */
    constexpr uint192dp_t get_uint192(uint8_t const * buffer) noexcept
    {
        return pointer_cast<const packed_t<uint192dp_t>*>( buffer )->store;
    }
    /**
     * See get_uint16() for reference.
     * @see \ref packed_t_alignment_cast
     */
    constexpr uint192dp_t get_uint192(uint8_t const * buffer, const lb_endian_t byte_order) noexcept
    {
        return get_packed_value(pointer_cast<const packed_t<uint192dp_t>*>( buffer ), byte_order);
    }

    /**
     * See put_uint16() for reference.
     * @see \ref packed_t_alignment_cast
     */
    constexpr void put_uint256(uint8_t * buffer, const uint256dp_t & v) noexcept
    {
        pointer_cast<packed_t<uint256dp_t>*>( buffer )->store = v;
    }
    /**
     * See put_uint16() for reference.
     * @see \ref packed_t_alignment_cast
     */
    constexpr void put_uint256(uint8_t * buffer, const uint256dp_t & v, const lb_endian_t byte_order) noexcept
    {
        pointer_cast<packed_t<uint256dp_t>*>( buffer )->store = is_little_endian(byte_order) ? cpu_to_le(v) : cpu_to_be(v);
    }
    /**
     * See get_uint16() for reference.
     * @see \ref packed_t_alignment_cast
     */
    constexpr uint256dp_t get_uint256(uint8_t const * buffer) noexcept
    {
        return pointer_cast<const packed_t<uint256dp_t>*>( buffer )->store;
    }
    /**
     * See get_uint16() for reference.
     * @see \ref packed_t_alignment_cast
     */
    constexpr uint256dp_t get_uint256(uint8_t const * buffer, const lb_endian_t byte_order) noexcept
    {
        return get_packed_value(pointer_cast<const packed_t<uint256dp_t>*>( buffer ), byte_order);
    }

    /**
    // *************************************************
    // *************************************************
    // *************************************************
     */

    /**
     * Put the given T value into the given byte address
     * using \ref packed_t to resolve a potential memory alignment issue *free of costs*.
     *
     * @tparam T
     * @param buffer
     * @param v
     * @see \ref packed_t_alignment_cast
     */
    template<typename T>
    constexpr
    typename std::enable_if_t<
        std::is_standard_layout_v<T>,
        void>
    put_value(uint8_t * buffer, const T& v) noexcept
    {
        // reinterpret_cast<packed_t<T>*>( buffer )->store = v;
        pointer_cast<packed_t<T>*>( buffer )->store = v;
    }

    /**
     * Put the given T value into the given byte address
     * using \ref packed_t to resolve a potential memory alignment issue *free of costs*.
     *
     * The value is converted from lb_endian::native to either lb_endian::little or lb_endian::big depending on given `byte_order`
     * before it is stored in memory.
     *
     * @tparam T
     * @param buffer
     * @param v
     * @param byte_order
     * @see \ref packed_t_alignment_cast
     */
    template<typename T>
    constexpr
    typename std::enable_if_t<
        std::is_standard_layout_v<T>,
        void>
    put_value(uint8_t * buffer, const T& v, const lb_endian_t byte_order) noexcept
    {
        pointer_cast<packed_t<T>*>( buffer )->store = is_little_endian(byte_order) ? cpu_to_le(v) : cpu_to_be(v);
    }

    /**
     * Returns a T value from the given byte address
     * using \ref packed_t to resolve a potential memory alignment issue *free of costs*.
     *
     * @tparam T
     * @param buffer
     * @return
     * @see \ref packed_t_alignment_cast
     */
    template<typename T>
    constexpr
    typename std::enable_if_t<
        std::is_standard_layout_v<T>,
        T>
    get_value(uint8_t const * buffer) noexcept
    {
        return pointer_cast<const packed_t<T>*>( buffer )->store;
    }

    /**
     * Returns a T value from the given byte address
     * using \ref packed_t to resolve a potential memory alignment issue *free of costs*.
     *
     * The value is converted from either lb_endian::little or lb_endian::big depending on given `byte_order`
     * to lb_endian::native before it is returned to the caller.
     *
     * @tparam T
     * @param buffer
     * @param byte_order
     * @return
     * @see \ref packed_t_alignment_cast
     */
    template<typename T>
    constexpr
    typename std::enable_if_t<
        std::is_standard_layout_v<T>,
        T>
    get_value(uint8_t const * buffer, const lb_endian_t byte_order) noexcept
    {
        return get_packed_value(pointer_cast<const packed_t<T>*>( buffer ), byte_order);
    }

    /**@}*/

} // namespace jau

/** \example test_basictypeconv.cpp
 * This C++ unit test validates the jau::bswap and get/set value implementation
 */

#endif /* JAU_BYTE_UTIL_HPP_ */
