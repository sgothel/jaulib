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

#ifndef JAU_BASIC_TYPES_HPP_
#define JAU_BASIC_TYPES_HPP_

#include <cstring>
#include <string>
#include <memory>
#include <cstdint>
#include <vector>
#include <type_traits>

extern "C" {
    #include <endian.h>
    #include <byteswap.h>
}

#include <jau/int_math.hpp>
#include <jau/cpp_lang_macros.hpp>
#include <jau/packed_attribute.hpp>

namespace jau {

    /**
     * Returns current monotonic time in milliseconds.
     */
    uint64_t getCurrentMilliseconds() noexcept;

    /**
    // *************************************************
    // *************************************************
    // *************************************************
     */

    #define E_FILE_LINE __FILE__,__LINE__

    class RuntimeException : public std::exception {
      private:
        std::string msg;
        std::string backtrace;
        std::string what_;

      protected:
        RuntimeException(std::string const type, std::string const m, const char* file, int line) noexcept;

      public:
        RuntimeException(std::string const m, const char* file, int line) noexcept
        : RuntimeException("RuntimeException", m, file, line) {}

        virtual ~RuntimeException() noexcept { }

        RuntimeException(const RuntimeException &o) = default;
        RuntimeException(RuntimeException &&o) = default;
        RuntimeException& operator=(const RuntimeException &o) = default;
        RuntimeException& operator=(RuntimeException &&o) = default;

        std::string get_backtrace() const noexcept { return backtrace; }

        virtual const char* what() const noexcept override {
            return what_.c_str(); // return std::runtime_error::what();
        }
    };

    class InternalError : public RuntimeException {
      public:
        InternalError(std::string const m, const char* file, int line) noexcept
        : RuntimeException("InternalError", m, file, line) {}
    };

    class OutOfMemoryError : public RuntimeException {
      public:
        OutOfMemoryError(std::string const m, const char* file, int line) noexcept
        : RuntimeException("OutOfMemoryError", m, file, line) {}
    };

    class NullPointerException : public RuntimeException {
      public:
        NullPointerException(std::string const m, const char* file, int line) noexcept
        : RuntimeException("NullPointerException", m, file, line) {}
    };

    class IllegalArgumentException : public RuntimeException {
      public:
        IllegalArgumentException(std::string const m, const char* file, int line) noexcept
        : RuntimeException("IllegalArgumentException", m, file, line) {}
    };

    class IllegalStateException : public RuntimeException {
      public:
        IllegalStateException(std::string const m, const char* file, int line) noexcept
        : RuntimeException("IllegalStateException", m, file, line) {}
    };

    class UnsupportedOperationException : public RuntimeException {
      public:
        UnsupportedOperationException(std::string const m, const char* file, int line) noexcept
        : RuntimeException("UnsupportedOperationException", m, file, line) {}
    };

    class IndexOutOfBoundsException : public RuntimeException {
      public:
        IndexOutOfBoundsException(const std::size_t index, const std::size_t length, const char* file, int line) noexcept
        : RuntimeException("IndexOutOfBoundsException", "Index "+std::to_string(index)+", data length "+std::to_string(length), file, line) {}

        IndexOutOfBoundsException(const std::string index_s, const std::string length_s, const char* file, int line) noexcept
        : RuntimeException("IndexOutOfBoundsException", "Index "+index_s+", data length "+length_s, file, line) {}

        IndexOutOfBoundsException(const std::size_t index, const std::size_t count, const std::size_t length, const char* file, int line) noexcept
        : RuntimeException("IndexOutOfBoundsException", "Index "+std::to_string(index)+", count "+std::to_string(count)+", data length "+std::to_string(length), file, line) {}
    };

    /**
    // *************************************************
    // *************************************************
    // *************************************************
     */

    __pack( struct uint128_t {
        uint8_t data[16];

        constexpr uint128_t() noexcept : data{0} {}
        constexpr uint128_t(const uint128_t &o) noexcept = default;
        uint128_t(uint128_t &&o) noexcept = default;
        constexpr uint128_t& operator=(const uint128_t &o) noexcept = default;
        uint128_t& operator=(uint128_t &&o) noexcept = default;

        void clear() noexcept { bzero(data, sizeof(data)); }

        constexpr bool operator==(uint128_t const &o) const noexcept {
            if( this == &o ) {
                return true;
            }
            return !std::memcmp(data, o.data, sizeof(data));
        }
        constexpr bool operator!=(uint128_t const &o) const noexcept
        { return !(*this == o); }
    } ) ;

    constexpr uint128_t bswap(uint128_t const & source) noexcept {
        uint128_t dest;
        uint8_t const * const s = source.data;
        uint8_t * const d = dest.data;
        for(nsize_t i=0; i<16; i++) {
            d[i] = s[15-i];
        }
        return dest;
    }

    __pack( struct uint192_t {
        uint8_t data[24];

        constexpr uint192_t() noexcept : data{0} {}
        constexpr uint192_t(const uint192_t &o) noexcept = default;
        uint192_t(uint192_t &&o) noexcept = default;
        constexpr uint192_t& operator=(const uint192_t &o) noexcept = default;
        uint192_t& operator=(uint192_t &&o) noexcept = default;

        void clear() noexcept { bzero(data, sizeof(data)); }

        constexpr bool operator==(uint192_t const &o) const noexcept {
            if( this == &o ) {
                return true;
            }
            return !std::memcmp(data, o.data, sizeof(data));
        }
        constexpr bool operator!=(uint192_t const &o) const noexcept
        { return !(*this == o); }
    } );

    constexpr uint192_t bswap(uint192_t const & source) noexcept {
        uint192_t dest;
        uint8_t const * const s = source.data;
        uint8_t * const d = dest.data;
        for(nsize_t i=0; i<24; i++) {
            d[i] = s[23-i];
        }
        return dest;
    }

    __pack( struct uint256_t {
        uint8_t data[32];

        constexpr uint256_t() noexcept : data{0} {}
        constexpr uint256_t(const uint256_t &o) noexcept = default;
        uint256_t(uint256_t &&o) noexcept = default;
        constexpr uint256_t& operator=(const uint256_t &o) noexcept = default;
        uint256_t& operator=(uint256_t &&o) noexcept = default;

        void clear() noexcept { bzero(data, sizeof(data)); }

        constexpr bool operator==(uint256_t const &o) const noexcept {
            if( this == &o ) {
                return true;
            }
            return !std::memcmp(data, o.data, sizeof(data));
        }
        constexpr bool operator!=(uint256_t const &o) const noexcept
        { return !(*this == o); }
    } );

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

    /**
     * On the i386 the host byte order is Least Significant Byte first (LSB) or Little-Endian,
     * whereas the network byte order, as used on the Internet, is Most Significant Byte first (MSB) or Big-Endian.
     * See #include <arpa/inet.h>
     *
     * Bluetooth is LSB or Little-Endian!
     */

#if __BYTE_ORDER == __BIG_ENDIAN
    inline uint16_t be_to_cpu(uint16_t const n) noexcept {
        return n;
    }
    inline uint16_t cpu_to_be(uint16_t const h) noexcept {
        return h;
    }
    inline uint16_t le_to_cpu(uint16_t const l) noexcept {
        return bswap_16(l);
    }
    inline uint16_t cpu_to_le(uint16_t const h) noexcept {
        return bswap_16(h);
    }

    inline uint32_t be_to_cpu(uint32_t const n) noexcept {
        return n;
    }
    inline uint32_t cpu_to_be(uint32_t const h) noexcept {
        return h;
    }
    inline uint32_t le_to_cpu(uint32_t const l) noexcept {
        return bswap_32(l);
    }
    inline uint32_t cpu_to_le(uint32_t const h) noexcept {
        return bswap_32(h);
    }

    inline uint64_t be_to_cpu(uint64_t const & n) noexcept {
        return n;
    }
    inline uint64_t cpu_to_be(uint64_t const & h) noexcept {
        return h;
    }
    inline uint64_t le_to_cpu(uint64_t const & l) noexcept {
        return bswap_64(l);
    }
    inline uint64_t cpu_to_le(uint64_t const & h) noexcept {
        return bswap_64(h);
    }

    constexpr uint128_t be_to_cpu(uint128_t const & n) noexcept {
        return n;
    }
    constexpr uint128_t cpu_to_be(uint128_t const & h) noexcept {
        return n;
    }
    constexpr uint128_t le_to_cpu(uint128_t const & l) noexcept {
        return bswap(l);
    }
    constexpr uint128_t cpu_to_le(uint128_t const & h) noexcept {
        return bswap(h);
    }

    constexpr uint192_t be_to_cpu(uint192_t const & n) noexcept {
        return n;
    }
    constexpr uint192_t cpu_to_be(uint192_t const & h) noexcept {
        return n;
    }
    constexpr uint192_t le_to_cpu(uint192_t const & l) noexcept {
        return bswap(l);
    }
    constexpr uint192_t cpu_to_le(uint192_t const & h) noexcept {
        return bswap(h);
    }

    constexpr uint256_t be_to_cpu(uint256_t const & n) noexcept {
        return n;
    }
    constexpr uint256_t cpu_to_be(uint256_t const & h) noexcept {
        return n;
    }
    constexpr uint256_t le_to_cpu(uint256_t const & l) noexcept {
        return bswap(l);
    }
    constexpr uint256_t cpu_to_le(uint256_t const & h) noexcept {
        return bswap(h);
    }
#elif __BYTE_ORDER == __LITTLE_ENDIAN
    inline uint16_t be_to_cpu(uint16_t const n) noexcept {
        return bswap_16(n);
    }
    inline uint16_t cpu_to_be(uint16_t const h) noexcept {
        return bswap_16(h);
    }
    inline uint16_t le_to_cpu(uint16_t const l) noexcept {
        return l;
    }
    inline uint16_t cpu_to_le(uint16_t const h) noexcept {
        return h;
    }

    inline uint32_t be_to_cpu(uint32_t const n) noexcept {
        return bswap_32(n);
    }
    inline uint32_t cpu_to_be(uint32_t const h) noexcept {
        return bswap_32(h);
    }
    inline uint32_t le_to_cpu(uint32_t const l) noexcept {
        return l;
    }
    inline uint32_t cpu_to_le(uint32_t const h) noexcept {
        return h;
    }

    inline uint64_t be_to_cpu(uint64_t const & n) noexcept {
        return bswap_64(n);
    }
    inline uint64_t cpu_to_be(uint64_t const & h) noexcept {
        return bswap_64(h);
    }
    inline uint64_t le_to_cpu(uint64_t const & l) noexcept {
        return l;
    }
    inline uint64_t cpu_to_le(uint64_t const & h) noexcept {
        return h;
    }

    constexpr uint128_t be_to_cpu(uint128_t const & n) noexcept {
        return bswap(n);
    }
    constexpr uint128_t cpu_to_be(uint128_t const & h) noexcept {
        return bswap(h);
    }
    constexpr uint128_t le_to_cpu(uint128_t const & l) noexcept {
        return l;
    }
    constexpr uint128_t cpu_to_le(uint128_t const & h) noexcept {
        return h;
    }

    constexpr uint192_t be_to_cpu(uint192_t const & n) noexcept {
        return bswap(n);
    }
    constexpr uint192_t cpu_to_be(uint192_t const & h) noexcept {
        return bswap(h);
    }
    constexpr uint192_t le_to_cpu(uint192_t const & l) noexcept {
        return l;
    }
    constexpr uint192_t cpu_to_le(uint192_t const & h) noexcept {
        return h;
    }

    constexpr uint256_t be_to_cpu(uint256_t const & n) noexcept {
        return bswap(n);
    }
    constexpr uint256_t cpu_to_be(uint256_t const & h) noexcept {
        return bswap(h);
    }
    constexpr uint256_t le_to_cpu(uint256_t const & l) noexcept {
        return l;
    }
    constexpr uint256_t cpu_to_le(uint256_t const & h) noexcept {
        return h;
    }
#else
    #error "Unexpected __BYTE_ORDER"
#endif

    inline void put_uint8(uint8_t * buffer, nsize_t const byte_offset, const uint8_t v) noexcept
    {
        *reinterpret_cast<uint8_t *>( buffer + byte_offset ) = v;
    }
    inline uint8_t get_uint8(uint8_t const * buffer, nsize_t const byte_offset) noexcept
    {
        return *reinterpret_cast<uint8_t const *>( buffer + byte_offset );
    }
    inline int8_t get_int8(uint8_t const * buffer, nsize_t const byte_offset) noexcept
    {
        return *reinterpret_cast<int8_t const *>( buffer + byte_offset );
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

    inline void put_uint16(uint8_t * buffer, nsize_t const byte_offset, const uint16_t v) noexcept
    {
        /**
         * Handle potentially misaligned address of buffer + byte_offset, can't just
         *   uint16_t * p = (uint16_t *) ( buffer + byte_offset );
         *   *p = v;
         * Universal alternative using memcpy is costly:
         *   memcpy(buffer + byte_offset, &v, sizeof(v));
         * Use compiler magic 'struct __attribute__((__packed__))' access:
         */
        reinterpret_cast<packed_t<uint16_t>*>( buffer + byte_offset )->store = v;
    }
    inline void put_uint16(uint8_t * buffer, nsize_t const byte_offset, const uint16_t v, const bool littleEndian) noexcept
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
        reinterpret_cast<packed_t<uint16_t>*>( buffer + byte_offset )->store = littleEndian ? cpu_to_le(v) : cpu_to_be(v);
    }
    inline uint16_t get_uint16(uint8_t const * buffer, nsize_t const byte_offset) noexcept
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
        return reinterpret_cast<const packed_t<uint16_t>*>( buffer + byte_offset )->store;
    }
    inline uint16_t get_uint16(uint8_t const * buffer, nsize_t const byte_offset, const bool littleEndian) noexcept
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
        return reinterpret_cast<const packed_t<uint16_t>*>( buffer + byte_offset )->get(littleEndian);
    }

    inline void put_uint32(uint8_t * buffer, nsize_t const byte_offset, const uint32_t v) noexcept
    {
        reinterpret_cast<packed_t<uint32_t>*>( buffer + byte_offset )->store = v;
    }
    inline void put_uint32(uint8_t * buffer, nsize_t const byte_offset, const uint32_t v, const bool littleEndian) noexcept
    {
        reinterpret_cast<packed_t<uint32_t>*>( buffer + byte_offset )->store = littleEndian ? cpu_to_le(v) : cpu_to_be(v);
    }
    inline uint32_t get_uint32(uint8_t const * buffer, nsize_t const byte_offset) noexcept
    {
        return reinterpret_cast<const packed_t<uint32_t>*>( buffer + byte_offset )->store;
    }
    inline uint32_t get_uint32(uint8_t const * buffer, nsize_t const byte_offset, const bool littleEndian) noexcept
    {
        return reinterpret_cast<const packed_t<uint32_t>*>( buffer + byte_offset )->get(littleEndian);
    }

    inline void put_uint64(uint8_t * buffer, nsize_t const byte_offset, const uint64_t & v) noexcept
    {
        reinterpret_cast<packed_t<uint64_t>*>( buffer + byte_offset )->store = v;
    }
    inline void put_uint64(uint8_t * buffer, nsize_t const byte_offset, const uint64_t & v, const bool littleEndian) noexcept
    {
        reinterpret_cast<packed_t<uint64_t>*>( buffer + byte_offset )->store = littleEndian ? cpu_to_le(v) : cpu_to_be(v);
    }
    inline uint64_t get_uint64(uint8_t const * buffer, nsize_t const byte_offset) noexcept
    {
        return reinterpret_cast<const packed_t<uint64_t>*>( buffer + byte_offset )->store;
    }
    inline uint64_t get_uint64(uint8_t const * buffer, nsize_t const byte_offset, const bool littleEndian) noexcept
    {
        return reinterpret_cast<const packed_t<uint64_t>*>( buffer + byte_offset )->get(littleEndian);
    }

    inline void put_uint128(uint8_t * buffer, nsize_t const byte_offset, const uint128_t & v) noexcept
    {
        reinterpret_cast<packed_t<uint128_t>*>( buffer + byte_offset )->store = v;
    }
    inline void put_uint128(uint8_t * buffer, nsize_t const byte_offset, const uint128_t & v, const bool littleEndian) noexcept
    {
        reinterpret_cast<packed_t<uint128_t>*>( buffer + byte_offset )->store = littleEndian ? cpu_to_le(v) : cpu_to_be(v);
    }
    inline uint128_t get_uint128(uint8_t const * buffer, nsize_t const byte_offset) noexcept
    {
        return reinterpret_cast<const packed_t<uint128_t>*>( buffer + byte_offset )->store;
    }
    inline uint128_t get_uint128(uint8_t const * buffer, nsize_t const byte_offset, const bool littleEndian) noexcept
    {
        return reinterpret_cast<const packed_t<uint128_t>*>( buffer + byte_offset )->get(littleEndian);
    }

    inline void put_uint192(uint8_t * buffer, nsize_t const byte_offset, const uint192_t & v) noexcept
    {
        reinterpret_cast<packed_t<uint192_t>*>( buffer + byte_offset )->store = v;
    }
    inline void put_uint192(uint8_t * buffer, nsize_t const byte_offset, const uint192_t & v, const bool littleEndian) noexcept
    {
        reinterpret_cast<packed_t<uint192_t>*>( buffer + byte_offset )->store = littleEndian ? cpu_to_le(v) : cpu_to_be(v);
    }
    inline uint192_t get_uint192(uint8_t const * buffer, nsize_t const byte_offset) noexcept
    {
        return reinterpret_cast<const packed_t<uint192_t>*>( buffer + byte_offset )->store;
    }
    inline uint192_t get_uint192(uint8_t const * buffer, nsize_t const byte_offset, const bool littleEndian) noexcept
    {
        return reinterpret_cast<const packed_t<uint192_t>*>( buffer + byte_offset )->get(littleEndian);
    }

    inline void put_uint256(uint8_t * buffer, nsize_t const byte_offset, const uint256_t & v) noexcept
    {
        reinterpret_cast<packed_t<uint256_t>*>( buffer + byte_offset )->store = v;
    }
    inline void put_uint256(uint8_t * buffer, nsize_t const byte_offset, const uint256_t & v, const bool littleEndian) noexcept
    {
        reinterpret_cast<packed_t<uint256_t>*>( buffer + byte_offset )->store = littleEndian ? cpu_to_le(v) : cpu_to_be(v);
    }
    inline uint256_t get_uint256(uint8_t const * buffer, nsize_t const byte_offset) noexcept
    {
        return reinterpret_cast<const packed_t<uint256_t>*>( buffer + byte_offset )->store;
    }
    inline uint256_t get_uint256(uint8_t const * buffer, nsize_t const byte_offset, const bool littleEndian) noexcept
    {
        return reinterpret_cast<const packed_t<uint256_t>*>( buffer + byte_offset )->get(littleEndian);
    }

    inline void set_bit_uint32(const uint8_t nr, uint32_t &mask)
    {
        if( nr > 31 ) { throw IndexOutOfBoundsException(nr, 32, E_FILE_LINE); }
        mask |= 1 << (nr & 31);
    }

    inline void clear_bit_uint32(const uint8_t nr, uint32_t &mask)
    {
        if( nr > 31 ) { throw IndexOutOfBoundsException(nr, 32, E_FILE_LINE); }
        mask |= ~(1 << (nr & 31));
    }

    inline uint32_t test_bit_uint32(const uint8_t nr, const uint32_t mask)
    {
        if( nr > 31 ) { throw IndexOutOfBoundsException(nr, 32, E_FILE_LINE); }
        return mask & (1 << (nr & 31));
    }

    inline void set_bit_uint64(const uint8_t nr, uint64_t &mask)
    {
        if( nr > 63 ) { throw IndexOutOfBoundsException(nr, 64, E_FILE_LINE); }
        mask |= 1 << (nr & 63);
    }

    inline void clear_bit_uint64(const uint8_t nr, uint64_t &mask)
    {
        if( nr > 63 ) { throw IndexOutOfBoundsException(nr, 64, E_FILE_LINE); }
        mask |= ~(1 << (nr & 63));
    }

    inline uint64_t test_bit_uint64(const uint8_t nr, const uint64_t mask)
    {
        if( nr > 63 ) { throw IndexOutOfBoundsException(nr, 64, E_FILE_LINE); }
        return mask & (1 << (nr & 63));
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

    /**
     * Merge the given 'uuid16' into a 'base_uuid' copy at the given little endian 'uuid16_le_octet_index' position.
     * <p>
     * The given 'uuid16' value will be added with the 'base_uuid' copy at the given position.
     * </p>
     * <pre>
     * base_uuid: 00000000-0000-1000-8000-00805F9B34FB
     *    uuid16: DCBA
     * uuid16_le_octet_index: 12
     *    result: 0000DCBA-0000-1000-8000-00805F9B34FB
     *
     * LE: low-mem - FB349B5F8000-0080-0010-0000-ABCD0000 - high-mem
     *                                           ^ index 12
     * LE: uuid16 -> value.data[12+13]
     *
     * BE: low-mem - 0000DCBA-0000-1000-8000-00805F9B34FB - high-mem
     *                   ^ index 2
     * BE: uuid16 -> value.data[2+3]
     * </pre>
     */
    uint128_t merge_uint128(uint16_t const uuid16, uint128_t const & base_uuid, nsize_t const uuid16_le_octet_index);

    /**
     * Merge the given 'uuid32' into a 'base_uuid' copy at the given little endian 'uuid32_le_octet_index' position.
     * <p>
     * The given 'uuid32' value will be added with the 'base_uuid' copy at the given position.
     * </p>
     * <pre>
     * base_uuid: 00000000-0000-1000-8000-00805F9B34FB
     *    uuid32: 87654321
     * uuid32_le_octet_index: 12
     *    result: 87654321-0000-1000-8000-00805F9B34FB
     *
     * LE: low-mem - FB349B5F8000-0080-0010-0000-12345678 - high-mem
     *                                           ^ index 12
     * LE: uuid32 -> value.data[12..15]
     *
     * BE: low-mem - 87654321-0000-1000-8000-00805F9B34FB - high-mem
     *               ^ index 0
     * BE: uuid32 -> value.data[0..3]
     * </pre>
     */
    uint128_t merge_uint128(uint32_t const uuid32, uint128_t const & base_uuid, nsize_t const uuid32_le_octet_index);

    /**
     * Produce a hexadecimal string representation of the given byte values.
     * <p>
     * If lsbFirst is true, orders LSB left -> MSB right, usual for byte streams.<br>
     * Otherwise orders MSB left -> LSB right, usual for readable integer values.
     * </p>
     * @param bytes pointer to the first byte to print, less offset
     * @param offset offset to bytes pointer to the first byte to print
     * @param length number of bytes to print
     * @param lsbFirst true having the least significant byte printed first (lowest addressed byte to highest),
     *                 otherwise have the most significant byte printed first (highest addressed byte to lowest).
     * @param leading0X true to have a leading '0x' being printed, otherwise no prefix is produced.
     * @param lowerCase true to use lower case hex-chars, otherwise capital letters are being used.
     * @return the hex-string representation of the data
     */
    std::string bytesHexString(const uint8_t * bytes, const nsize_t offset, const nsize_t length,
                               const bool lsbFirst, const bool leading0X=true, const bool lowerCase=true) noexcept;


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
     * @param leading0X true to have a leading '0x' being printed, otherwise no prefix is produced.
     * @return the hex-string representation of the value
     */
    inline std::string uint8HexString(const uint8_t v, const bool leading0X=true) noexcept {
        return bytesHexString(reinterpret_cast<const uint8_t*>(&v), 0, sizeof(v), false /* lsbFirst */, leading0X, true /* lowerCase */);
    }

    /**
     * Produce a lower-case hexadecimal string representation of the given uint16_t value.
     * @param v the value
     * @param leading0X true to have a leading '0x' being printed, otherwise no prefix is produced.
     * @return the hex-string representation of the value
     */
    inline std::string uint16HexString(const uint16_t v, const bool leading0X=true) noexcept {
        return bytesHexString(reinterpret_cast<const uint8_t*>(&v), 0, sizeof(v), false /* lsbFirst */, leading0X, true /* lowerCase */);
    }

    /**
     * Produce a lower-case hexadecimal string representation of the given uint32_t value.
     * @param v the value
     * @param leading0X true to have a leading '0x' being printed, otherwise no prefix is produced.
     * @return the hex-string representation of the value
     */
    inline std::string uint32HexString(const uint32_t v, const bool leading0X=true) noexcept {
        return bytesHexString(reinterpret_cast<const uint8_t*>(&v), 0, sizeof(v), false /* lsbFirst */, leading0X, true /* lowerCase */);
    }

    /**
     * Produce a lower-case hexadecimal string representation of the given uint64_t value.
     * @param v the value
     * @param leading0X true to have a leading '0x' being printed, otherwise no prefix is produced.
     * @return the hex-string representation of the value
     */
    inline std::string uint64HexString(const uint64_t v, const bool leading0X=true) noexcept {
        return bytesHexString(reinterpret_cast<const uint8_t*>(&v), 0, sizeof(v), false /* lsbFirst */, leading0X, true /* lowerCase */);
    }

    /**
     * Produce a lower-case hexadecimal string representation of the given 'void *' value.
     * @param v the value
     * @param leading0X true to have a leading '0x' being printed, otherwise no prefix is produced.
     * @return the hex-string representation of the value
     */
    inline std::string aptrHexString(const void * v, const bool leading0X=true) noexcept {
        return uint64HexString(reinterpret_cast<uint64_t>(v), leading0X);
    }

    /**
     * Produce a lower-case hexadecimal string representation of the given uint128_t value.
     * @param v the value
     * @param leading0X true to have a leading '0x' being printed, otherwise no prefix is produced.
     * @return the hex-string representation of the value
     */
    inline std::string uint128HexString(const uint128_t v, const bool leading0X=true) noexcept {
        return bytesHexString(v.data, 0, sizeof(v.data), false /* lsbFirst */, leading0X, true /* lowerCase */);
    }

    /**
     * Produce a lower-case hexadecimal string representation of the given uint256_t value.
     * @param v the value
     * @param leading0X true to have a leading '0x' being printed, otherwise no prefix is produced.
     * @return the hex-string representation of the value
     */
    inline std::string uint256HexString(const uint256_t v, const bool leading0X=true) noexcept {
        return bytesHexString(v.data, 0, sizeof(v.data), false /* lsbFirst */, leading0X, true /* lowerCase */);
    }

    /**
     * Produce a decimal string representation of an integral integer value.
     * @tparam T an integral integer type
     * @param v the integral integer value
     * @param separator if not 0, use as separation character, otherwise no separation characters are being used
     * @param width the minimum number of characters to be printed. Add padding with blank space if result is shorter.
     * @return the string representation of the integral integer value
     */
    template<class T>
    std::string to_decimal_string(const T v, const char separator=',', const nsize_t width=0) noexcept {
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
    inline std::string int64DecString(const int64_t v, const char separator=',', const nsize_t width=0) noexcept {
        return to_decimal_string<int64_t>(v, separator, width);
    }

    /**
     * Produce a decimal string representation of a uint64_t value.
     * @param v the value
     * @param separator if not 0, use as separation character, otherwise no separation characters are being used
     * @param width the minimum number of characters to be printed. Add padding with blank space if result is shorter.
     * @return the string representation of the value
     */
    inline std::string uint64DecString(const uint64_t v, const char separator=',', const nsize_t width=0) noexcept {
        return to_decimal_string<uint64_t>(v, separator, width);
    }

    /** trim in place */
    void trimInPlace(std::string &s) noexcept;

    /** trim copy */
    std::string trimCopy(const std::string &s) noexcept;

    /**
     * Helper, allowing simple access and provision of a typename string representation
     * at compile time, see jau::type_cue for usage.
     * <p>
     * You may use the macro <code>JAU_TYPENAME_CUE(TYPENAME)</code> to set a single type
     * or maybe <code>JAU_TYPENAME_CUE_ALL(TYPENAME)</code> to set the single type and
     * all pointer and reference variations (mutable and const).
     * </p>
     * <p>
     * Without user override, implementation will use <code>typeid(T).name()</code>.
     * </p>
     * @tparam T the typename to name
     * @see jau::type_cue
     */
    template <typename T>
    struct type_name_cue
    {
        /**
         * Return the string representation of this type.
         * <p>
         * This might be a compile time user override, see jau::type_name_cue.
         * </p>
         * <p>
         * If no user override has been provides, the default implementation
         * either returns <code>typeid(T).name()</code> if RTTI is enabled
         * or <code>"unnamed_type"</code> if RTTI is disabled.<br>
         * For the latter, we currently only test the G++ preprocessor macro <code>__GXX_RTTI</code>,
         * if RTTI is enabled.
         * </p>
         */
        static const char * name() {
#if defined(__cxx_rtti_available__)
            return typeid(T).name();
#else
            return "unnamed_type";
#endif
        }
    };
    /**
     * Helper, allowing simple access to compile time typename and <i>Type traits</i> information,
     * see jau::type_name_cue to setup typename's string representation.
     *
     * @tparam T the typename to introspect
     * @see jau::type_name_cue
     */
    template <typename T>
    struct type_cue : public type_name_cue<T>
    {
        /**
         * Print information of this type to stdout, potentially with all <i>Type traits</i> known.
         * @param typedefname the code typedefname (or typename) as a string, should match T
         * @param verbose if true, prints all <i>Type traits</i> known for this type. Be aware of the long output. Defaults to false.
         */
        static void print(const std::string& typedefname, bool verbose=false) {
            printf("Type: %s -> %s, %zu bytes\n", typedefname.c_str(), type_name_cue<T>::name(), sizeof(T));
            if( verbose ) {
                printf("  Primary Type Categories\n");
                printf("    void            %d\n", std::is_void<T>::value);
                printf("    null ptr        %d\n", std::is_null_pointer<T>::value);
                printf("    integral        %d\n", std::is_integral<T>::value);
                printf("    floating point  %d\n", std::is_floating_point<T>::value);
                printf("    array           %d\n", std::is_array<T>::value);
                printf("    enum            %d\n", std::is_enum<T>::value);
                printf("    union           %d\n", std::is_union<T>::value);
                printf("    class           %d\n", std::is_class<T>::value);
                printf("    function        %d\n", std::is_function<T>::value);
                printf("    pointer         %d\n", std::is_pointer<T>::value);
                printf("    lvalue ref      %d\n", std::is_lvalue_reference<T>::value);
                printf("    rvalue ref      %d\n", std::is_rvalue_reference<T>::value);
                printf("    member obj ptr  %d\n", std::is_member_object_pointer<T>::value);
                printf("    member func ptr %d\n", std::is_member_function_pointer<T>::value);
                printf("\n");
                printf("  Type Properties\n");
                printf("    const           %d\n", std::is_const<T>::value);
                printf("    volatile        %d\n", std::is_volatile<T>::value);
                printf("    trivial         %d\n", std::is_trivial<T>::value);
                printf("    trivially_copy. %d\n", std::is_trivially_copyable<T>::value);
                printf("    standard_layout %d\n", std::is_standard_layout<T>::value);
                printf("    pod             %d\n", std::is_pod<T>::value);
                printf("    unique_obj_rep  %d\n", std::has_unique_object_representations<T>::value);
                printf("    empty           %d\n", std::is_empty<T>::value);
                printf("    polymorphic     %d\n", std::is_polymorphic<T>::value);
                printf("    abstract        %d\n", std::is_abstract<T>::value);
                printf("    final           %d\n", std::is_final<T>::value);
                printf("    aggregate       %d\n", std::is_aggregate<T>::value);
                printf("    signed          %d\n", std::is_signed<T>::value);
                printf("    unsigned        %d\n", std::is_unsigned<T>::value);
    #if __cplusplus > 201703L
                printf("    bounded_array   %d\n", std::is_bounded_array<T>::value);
                printf("    unbounded_array %d\n", std::is_unbounded_array<T>::value);
                printf("    scoped enum     %d\n", std::is_scoped_enum<T>::value);
    #endif
                printf("\n");
                printf("  Composite Type Categories\n");
                printf("    fundamental     %d\n", std::is_fundamental<T>::value);
                printf("    arithmetic      %d\n", std::is_arithmetic<T>::value);
                printf("    scalar          %d\n", std::is_scalar<T>::value);
                printf("    object          %d\n", std::is_object<T>::value);
                printf("    compound        %d\n", std::is_compound<T>::value);
                printf("    reference       %d\n", std::is_reference<T>::value);
                printf("    member ptr      %d\n", std::is_member_pointer<T>::value);
                printf("\n");
            }
        }
    };
    #define JAU_TYPENAME_CUE(A) template<> struct jau::type_name_cue<A> { static const char * name() { return #A; } };
    #define JAU_TYPENAME_CUE_ALL(A) JAU_TYPENAME_CUE(A) JAU_TYPENAME_CUE(A*) JAU_TYPENAME_CUE(const A*) JAU_TYPENAME_CUE(A&) JAU_TYPENAME_CUE(const A&)


} // namespace jau

/** \example test_intdecstring01.cpp
 * This C++ unit test validates the jau::to_decimal_string implementation
 */

#endif /* JAU_BASIC_TYPES_HPP_ */
