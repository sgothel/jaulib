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

extern "C" {
    #include <endian.h>
    #include <byteswap.h>
}

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

    /**
     * Natural 'size_t' alternative using 'unsigned int' as its natural sized type.
     * <p>
     * The leading 'n' stands for natural.
     * </p>
     * <p>
     * This is a compromise to indicate intend,
     * but to avoid handling a multiple sized 'size_t' footprint where not desired.
     * </p>
     */
    typedef unsigned int nsize_t;

    /**
     * Natural 'ssize_t' alternative using 'signed int' as its natural sized type.
     * <p>
     * The leading 'n' stands for natural.
     * </p>
     * <p>
     * This is a compromise to indicate intend,
     * but to avoid handling a multiple sized 'ssize_t' footprint where not desired.
     * </p>
     */
    typedef signed int snsize_t;

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

        virtual const char* what() const noexcept override;
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
        IndexOutOfBoundsException(const nsize_t index, const nsize_t length, const char* file, int line) noexcept
        : RuntimeException("IndexOutOfBoundsException", "Index "+std::to_string(index)+", data length "+std::to_string(length), file, line) {}

        IndexOutOfBoundsException(const nsize_t index, const nsize_t count, const nsize_t length, const char* file, int line) noexcept
        : RuntimeException("IndexOutOfBoundsException", "Index "+std::to_string(index)+", count "+std::to_string(count)+", data length "+std::to_string(length), file, line) {}
    };

    /**
    // *************************************************
    // *************************************************
    // *************************************************
     */

    struct __attribute__((packed)) uint128_t {
        uint8_t data[16];

        constexpr uint128_t() noexcept : data{0} {}
        constexpr uint128_t(const uint128_t &o) noexcept = default;
        uint128_t(uint128_t &&o) noexcept = default;
        constexpr uint128_t& operator=(const uint128_t &o) noexcept = default;
        uint128_t& operator=(uint128_t &&o) noexcept = default;

        constexpr bool operator==(uint128_t const &o) const noexcept {
            if( this == &o ) {
                return true;
            }
            return !std::memcmp(data, o.data, sizeof(data));
        }
        constexpr bool operator!=(uint128_t const &o) const noexcept
        { return !(*this == o); }
    };

    constexpr uint128_t bswap(uint128_t const & source) noexcept {
        uint128_t dest;
        uint8_t const * const s = source.data;
        uint8_t * const d = dest.data;
        for(nsize_t i=0; i<16; i++) {
            d[i] = s[15-i];
        }
        return dest;
    }

    struct __attribute__((packed)) uint192_t {
        uint8_t data[24];

        constexpr uint192_t() noexcept : data{0} {}
        constexpr uint192_t(const uint192_t &o) noexcept = default;
        uint192_t(uint192_t &&o) noexcept = default;
        constexpr uint192_t& operator=(const uint192_t &o) noexcept = default;
        uint192_t& operator=(uint192_t &&o) noexcept = default;

        constexpr bool operator==(uint192_t const &o) const noexcept {
            if( this == &o ) {
                return true;
            }
            return !std::memcmp(data, o.data, sizeof(data));
        }
        constexpr bool operator!=(uint192_t const &o) const noexcept
        { return !(*this == o); }
    };

    constexpr uint192_t bswap(uint192_t const & source) noexcept {
        uint192_t dest;
        uint8_t const * const s = source.data;
        uint8_t * const d = dest.data;
        for(nsize_t i=0; i<24; i++) {
            d[i] = s[23-i];
        }
        return dest;
    }

    struct __attribute__((packed)) uint256_t {
        uint8_t data[32];

        constexpr uint256_t() noexcept : data{0} {}
        constexpr uint256_t(const uint256_t &o) noexcept = default;
        uint256_t(uint256_t &&o) noexcept = default;
        constexpr uint256_t& operator=(const uint256_t &o) noexcept = default;
        uint256_t& operator=(uint256_t &&o) noexcept = default;

        constexpr bool operator==(uint256_t const &o) const noexcept {
            if( this == &o ) {
                return true;
            }
            return !std::memcmp(data, o.data, sizeof(data));
        }
        constexpr bool operator!=(uint256_t const &o) const noexcept
        { return !(*this == o); }
    };

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
        uint8_t * p = (uint8_t *) ( buffer + byte_offset );
        *p = v;
    }
    inline uint8_t get_uint8(uint8_t const * buffer, nsize_t const byte_offset) noexcept
    {
        uint8_t const * p = (uint8_t const *) ( buffer + byte_offset );
        return *p;
    }
    inline int8_t get_int8(uint8_t const * buffer, nsize_t const byte_offset) noexcept
    {
        int8_t const * p = (int8_t const *) ( buffer + byte_offset );
        return *p;
    }

    /**
     * Safe access to a pointer cast from unaligned memory via __packed__ attribute,
     * i.e. utilizing compiler generated safe load and store operations.
     * <p>
     * This template shall cause no costs, the cast data pointer is identical to 'T & p = &store'.
     * </p>
     */
    template<typename T> struct __attribute__((__packed__)) packed_t {
        T store;
        constexpr T get(const bool littleEndian) const noexcept { return littleEndian ? le_to_cpu(store) : be_to_cpu(store); }
    };

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

    std::string uint8HexString(const uint8_t v, const bool leading0X=true) noexcept;
    std::string uint16HexString(const uint16_t v, const bool leading0X=true) noexcept;
    std::string uint32HexString(const uint32_t v, const bool leading0X=true) noexcept;
    std::string uint64HexString(const uint64_t v, const bool leading0X=true) noexcept;
    std::string aptrHexString(const void * v, const bool leading0X=true) noexcept;
    std::string uint128HexString(const uint128_t v, const bool leading0X=true) noexcept;
    std::string uint256HexString(const uint256_t v, const bool leading0X=true) noexcept;

    /**
     * If lsbFirst is true, orders LSB left -> MSB right, usual for byte streams.
     * <p>
     * Otherwise orders MSB left -> LSB right, usual for readable integer values.
     * </p>
     */
    std::string bytesHexString(const uint8_t * bytes, const nsize_t offset, const nsize_t length, const bool lsbFirst, const bool leading0X=true) noexcept;

    std::string int32SeparatedString(const int32_t v, const char separator=',') noexcept;
    std::string uint32SeparatedString(const uint32_t v, const char separator=',') noexcept;
    std::string uint64SeparatedString(const uint64_t v, const char separator=',') noexcept;

    /** trim in place */
    void trimInPlace(std::string &s) noexcept;

    /** trim copy */
    std::string trimCopy(const std::string &s) noexcept;

} // namespace jau

#endif /* JAU_BASIC_TYPES_HPP_ */
