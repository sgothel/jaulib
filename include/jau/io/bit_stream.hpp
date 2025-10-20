/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021-2025 Gothel Software e.K.
 *
 * SPDX-License-Identifier: MIT
 *
 * This Source Code Form is subject to the terms of the MIT License
 * If a copy of the MIT was not distributed with this file,
 * you can obtain one at https://opensource.org/license/mit/.
 */

#ifndef JAU_IO_BIT_STREAM_HPP_
#define JAU_IO_BIT_STREAM_HPP_

#include <unistd.h>
#include <climits>
#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>

#include <jau/basic_types.hpp>
#include <jau/byte_util.hpp>
#include <jau/cpp_lang_util.hpp>
#include <jau/debug.hpp>
#include <jau/int_types.hpp>
#include <jau/io/byte_stream.hpp>
#include <jau/io/io_util.hpp>
#include <jau/string_util.hpp>

namespace jau::io {

    using namespace jau::enums;

    /** I/O read or write access. */
    enum class ioaccess_t : bool {
        /** Read intent */
        read = false,
        /** Write intend */
        write = true
    };
    /**
     * Return std::string representation of the given ioaccess_t.
     * @param v the ioaccess_t value
     * @return the std::string representation
     */
    std::string to_string(const ioaccess_t v) noexcept;

    /**
     * Versatile Bitstream implementation supporting:
     * - Utilize I/O operations on I/O streams, buffers and arrays
     * - Uses least-significant-bit (lsb) first addressing and order for bit-operations
     * - Linear bit R/W operations
     * - Bulk 64-bit R/W bit-operations
     * - Bulk data-type operations w/ endian conversion
     * - Allow mark/reset and switching streams and input/output mode
     * - Optimized bulk-operations
     */
    class Bitstream {
      private:
        // private static final boolean DEBUG = Debug.debug("Bitstream");
        constexpr static jau::nsize_t ncount = std::numeric_limits<jau::nsize_t>::max();
        typedef uint64_t data_type; ///< uint64_t data type, bit buffer

      public:
        using size_type = jau::io::ByteStream::size_type; ///< uint64_t size data type, bit position and count

        /** Invalid position constant, denoting unset mark() or invalid position. Value: `std::numeric_limits<size_type>::max()` */
        constexpr static size_type npos = jau::io::ByteStream::npos;

        /** Maximum read bitCacheSizeRead() and fixed 64-bit write cache size. */
        constexpr static jau::nsize_t MaxBitCacheSize = sizeof(data_type)*CHAR_BIT;

      private:
        constexpr static size_type data_shift = log2_byteshift(sizeof(data_type));
        constexpr static size_type byte_shift = log2_byteshift(sizeof(uint8_t));

        constexpr static bool useFastPathStream = true;
        constexpr static bool useFastPathTypes = true;

        std::unique_ptr<ByteStream> m_bytes;

        /** 64-bit cache of byte stream */
        data_type m_bitCache;
        data_type m_bitsDataMark;

        jau::nsize_t m_bitCacheSizeRead;
        /** See bitCount, ranges [0..63]. */
        jau::nsize_t m_bitCount;
        jau::nsize_t m_bitsCountMark;

        ioaccess_t m_access;

      public:
        /**
         * @param stream input and/or output stream
         * @param access ioaccess_t::read for read-access and ioaccess_t::write for read-access
         * @throws IllegalArgumentError if requested `writeMode` doesn't match stream's ByteStream::canRead() and ByteStream::canWrite().
         */
        Bitstream(std::unique_ptr<ByteStream> && stream, ioaccess_t access)
        : m_bytes(std::move(stream)), m_access(access) {
            resetLocal();
            validateMode1(access);
        }

      private:
        void resetLocal() {
            m_bitCache = 0;
            m_bitCacheSizeRead = 0;
            m_bitCount = 0;
            m_bitsDataMark = 0;
            m_bitsCountMark = ncount;
        }
        bool canRead0() const noexcept { return m_bytes->canRead(); }
        bool canWrite0() const noexcept { return m_bytes->canWrite(); }
        void validateMode1(ioaccess_t access) const {
            if( !canRead0() && !canWrite0() ) {
                throw jau::IllegalArgumentError("stream can neither input nor output: "+toStringImpl(), E_FILE_LINE);
            }
            if( ioaccess_t::write == access && !canWrite0() ) {
                throw jau::IllegalArgumentError("stream cannot output as requested: "+toStringImpl(), E_FILE_LINE);
            }
            if( ioaccess_t::read == access && !canRead0() ) {
                throw jau::IllegalArgumentError("stream cannot input as requested: "+toStringImpl(), E_FILE_LINE);
            }
        }
        bool validateMode2(ioaccess_t access) const {
            if( !canRead0() && !canWrite0() ) {
                return false;
            }
            if( ioaccess_t::write == access && !canWrite0() ) {
                return false;
            }
            if( ioaccess_t::read == access && !canRead0() ) {
                return false;
            }
            return true;
        }
        [[nodiscard]] bool writeCache() noexcept {
            const size_t s = (m_bitCount + 7) >> byte_shift;
            size_t w;
            if( s > 0 ) {
                m_bitCache &= jau::bit_mask<data_type>(m_bitCount);
                w = m_bytes->write(&m_bitCache, s); // LSB
            } else {
                w = 0;
            }
            if( s == w ) {
                m_bitCount = 0;
                m_bitCache = 0;
                m_bitCacheSizeRead = 0;
                return true;
            } else {
                return false;
            }
        }
        void fillCache() noexcept {
            m_bitCache = 0;
            m_bitCacheSizeRead = m_bytes->read(&m_bitCache, sizeof(data_type)) << byte_shift;
        }

      public:
        /** Returns the used underlying ByteStream. */
        ByteStream& byteStream() { return *m_bytes; }

        constexpr_cxx23 iomode_t mode() const noexcept { return m_bytes->mode(); }

        /**
         * Changes the access-mode to write or read and resets position and cache to zero.
         *
         * If the previous stream was in ioaccess_t::write mode, flush() is being called.
         *
         * Certain ByteStream implementations may not allow random rewinding of the stream,
         *
         * @param writeMode new access-mode
         * @returns false if requested `access` doesn't match stream's ByteStream::canRead() and
         *          ByteStream::canWrite() - or flush() failed, otherwise true
         */
        [[nodiscard]] bool setAccess(ioaccess_t access) noexcept {
            if( !validateMode2(access) ) {
                return false;
            }
            if( canWrite() && npos == flush() ) {
                return false;
            }
            m_access = access;
            if( 0 != m_bytes->seek(0) ) {
                return false;
            }
            resetLocal();
            return true;
         }

        /**
         * Changes the write-mode to read, sets the underlying ByteStream to read-only and resets position and cache to zero.
         *
         * If the previous stream was in ioaccess_t::write mode, flush() is being called.
         *
         * Certain ByteStream implementations may not allow random rewinding of the stream,
         *
         * @returns false if requested ioaccess_t::read doesn't match stream's ByteStream::canRead() - or flush() failed, otherwise true
         */
        [[nodiscard]] bool setImmutable() noexcept {
            if( canWrite() ) {
                if( !validateMode2(ioaccess_t::read) ) {
                    return false;
                }
                if( npos == flush() ) {
                    return false;
                }
                m_access = ioaccess_t::read;
            }
            m_bytes->setImmutable();
            if( 0 != m_bytes->seek(0) ) {
                return false;
            }
            resetLocal();
            return true;
        }

        /**
         * Returns endian byte-order of stream storage.
         *
         * Only affects multi-byte r/w operations, e.g. readU16(), writeU16(), etc.
         */
        constexpr_cxx23 lb_endian_t byteOrder() const noexcept { return m_bytes->byteOrder(); }

        /** Returns true in case stream is in write mode, false if in read mode. */
        constexpr bool canWrite() const noexcept { return ioaccess_t::write == m_access; }
        /** Returns ioaccess_t stream mode. */
        constexpr ioaccess_t ioaccess() const noexcept { return m_access; }

        /**
         * Closing the underlying stream, implies {@link #flush()}.
         * <p>
         * Implementation will <code>null</code> the stream references,
         * hence {@link #setStream(Object)} must be called before re-using instance.
         * </p>
         * <p>
         * If the closed stream was in {@link #writeMode() output mode},
         * {@link #flush()} is being called.
         * </p>
         *
         * @throws IOException
         */
        void close() noexcept {
            flush();
            m_bytes->close();
            resetLocal();
        }

        /**
         * Synchronizes underlying ByteStream output stream operations in writeMode(), or does nothing.
         *
         * Method also flushes incomplete bytes to the underlying ByteStream
         * and hence skips to the next byte position.
         *
         * @return ::npos caused by writing failure, otherwise one if pending bit-buffer was written or zero for none.
         */
        size_type flush() noexcept {
            if( !canWrite() ) {
                return 0;
            }
            size_type c = 0;
            if( 0 != m_bitCount ) {
                if( !writeCache() ) {
                    return npos;
                }
                c = 1;
            }
            m_bytes->flush();
            return c;
        }

        /**
         * Set `markpos` to current bit-position, allowing the stream to be seekMark().
         *
         * seek() will clear `markpos` if > newPos.
         *
         * For implementations where seek() doesn't allow random rewinding of the stream,
         * setMark() will allow rewinding back to `markpos` if not exceeding `readLimit`.
         *
         * @param readlimit maximum number of bytes able to read before invalidating the `markpos`.
         * @return true if marks is set successfully, otherwise false
         */
        [[nodiscard]] bool setMark(size_type readLimit) noexcept {
            if(!m_bytes->setMark(readLimit) ) {
                return false;
            }
            m_bitsDataMark = m_bitCache;
            m_bitsCountMark = m_bitCount;
            return true;
        }

        /** Returns the `markpos` set via setMark() or ByteStream::npos if unset. */
        size_type mark() const noexcept { return m_bytes->mark(); }

        /** Returns the `readLimit` set via setMark(). If unset either 0 or implicit limit. */
        uint64_t markReadLimit() const noexcept { return m_bytes->markReadLimit(); }

        /**
         * Seeks stream bit-position to `markpos` as set via setMark().
         *
         * `markpos` is kept, hence seekMark() can be called multiple times.
         *
         * @return true if successful (incl eofbit),
         *         otherwise false with unchanged position due to I/O failure (iostate::fail set) or setMark() not set.
         */
        [[nodiscard]] bool seekMark() noexcept {
            if( ncount == m_bitsCountMark || !m_bytes->seekMark() ) {
                return false;
            }
            m_bitCache = m_bitsDataMark;
            m_bitCount = m_bitsCountMark;
            return true;
        }

        /**
         * Returns filled read bit-cache-size, i.e. up to 64-bit MaxBitCacheSize reading.
         * @see MaxBitCacheSize
         * @see cachedBitCount()
         * @see position()
         */
        constexpr jau::nsize_t bitCacheSizeRead() const noexcept { return m_bitCacheSizeRead; }

        /**
         * Returns number of cached bits.
         *
         * - Read operation
         *   - Number of bits cached before next up-to 64-bit-read when zero is reached, flipping over to bitCacheSizeRead()
         *   - Counting down
         *   - Range (bitCacheSizeRead()..0]
         *   - bitCacheSizeRead() is set when zero is reached and cache is filled
         *
         * - Write operation
         *   - Number of bits cached before next 64-bit MaxBitCacheSize write when 64-bits limit is reached, flipping over to 0
         *   - Counting up
         *   - Range [0..MaxBitCacheSize)
         *   - bitCacheSizeRead() is set to cachedBitCount() after writing
         *
         * @see MaxBitCacheSize
         * @see bitCacheSizeRead()
         * @see bitCache()
         * @see position()
         */
        constexpr jau::nsize_t cachedBitCount() const noexcept { return m_bitCount; }

        /**
         * Return the next cached bit position.
         * @see MaxBitCacheSize
         * @see bitCacheSizeRead()
         * @see cachedBitCount()
         */
        constexpr jau::nsize_t cachedBitPos() const noexcept {
            if ( canWrite() ) {
                return m_bitCount;
            } else {
                return m_bitCacheSizeRead - m_bitCount;
            }
        }

        /**
         * Returns the 64-bit MaxBitCacheSize cache buffer value.
         * @see MaxBitCacheSize
         * @see bitCacheSizeRead()
         * @see cachedBitCount()
         * @see position()
         */
        data_type bitCache() { return m_bitCache; }

        /**
         * Returns the bit position in the stream.
         * @see MaxBitCacheSize
         * @see bitCacheSizeRead()
         * @see cachedBitCount()
         */
        size_type position() const noexcept {
            if ( !m_bytes->isOpen() ) {
                return npos;
            }
            const size_type streamBitPos = m_bytes->position() << byte_shift;
            if ( canWrite()  ) {
                return streamBitPos + m_bitCount;
            } else {
                return streamBitPos - m_bitCount;
            }
        }

        /**
         * Sets this stream's bit position.
         *
         * A set mark is cleared.
         *
         * Known supporting implementation is {@link ByteBufferStream} and {@link ByteArrayStream}.
         *
         * @param newPos desired absolute bit-position
         * @return resulting bit-position if successful (incl EOF) or npos otherwise having an unchanged position().
         */
        [[nodiscard]] size_type seek(size_type newPos) noexcept {
            const size_type pos0 = position();
            if( newPos == pos0 ) {
                return newPos;
            } else if( newPos > pos0 ) {
                return pos0 + skip(newPos - pos0);
            } else {
                // backwards
                if( canWrite() ) {
                    if( 0 < m_bitCount && !writeCache() ) {
                        return 0;
                    }
                }
                resetLocal();
                if( 0 != m_bytes->seek(0) ) {
                    return position();
                }
                return skip(newPos);
            }
        }

        /**
         * Skip given number of bits.
         *
         * @param n number of bits to skip
         * @return actual skipped bits
         */
        [[nodiscard]] size_type skip(size_type n) noexcept {
            DBG_PRINT("Bitstream.skip.0: %" PRIu64 " - %s", n, toStringImpl().c_str());
            // forward skip
            if( !canWrite() && n <= m_bitCount ) {
                m_bitCount -= n;
                DBG_PRINT("Bitstream.skip.F_N1: %" PRIu64 " - %s", n, toStringImpl().c_str());
                return n;
            } else if( canWrite() && n <= MaxBitCacheSize-m_bitCount ) {
                m_bitCount += n;
                DBG_PRINT("Bitstream.skip.F_N2: %" PRIu64 " - %s", n, toStringImpl().c_str());
                return n;
            } else {
                // r: n > bitCount
                // w: n > MaxBitCacheSize-m_bitCount
                if( canWrite() ) {
                    if( 0 < m_bitCount && !writeCache() ) {
                        return 0;
                    }
                }
                const size_type n1 = n - m_bitCount;                        // bits to skip, subtracting cached bits, bitCount is zero at this point
                const size_type n2 = n1 & ~(MaxBitCacheSize-1);             // 64-bit aligned bits to skip
                const size_type n3 = n2 >> byte_shift;                      // bytes to skip (64-bit aligned)
                const size_type n4 = m_bytes->seek(m_bytes->position()+n3); // actual skipped bytes (64-bit aligned)
                const size_type n5 = n1 - ( n3 << byte_shift );             // remaining skip bits == nX % 64 (64-bit aligned)
                const size_type nX = (n4 << byte_shift) + n5 + m_bitCount;  // actual skipped bits
                m_bitCount = 0;
                // DBG_PRINT("Bitstream.skip.1: n %" PRIu64 ", n1 %" PRIu64 ", n2 %" PRIu64 ", n3 %" PRIu64 ", n4 %" PRIu64 ", n5 %" PRIu64 ", nX %" PRIu64 ",  - %s",
                //    n, n1, n2, n3, n4, n5, nX, toStringImpl().c_str());
                if( nX < n ) {
                    // incomplete skipping
                    m_bitCache = 0;
                    DBG_PRINT("Bitstream.skip.F_EOS: %" PRIu64 " - %s", n, toStringImpl().c_str());
                    return nX;
                }
                jau::nsize_t notReadBits = 0;
                if( 0 < n5 ) {
                    assert(!canWrite()); // flushed above, m_bitCount:=0
                    fillCache();
                    if( m_bitCacheSizeRead >= n5 ) {
                        m_bitCount = m_bitCacheSizeRead - n5;
                    } else {
                        notReadBits = n5 - m_bitCacheSizeRead; // EOF
                    }
                } else {
                    m_bitCount = 0;
                }
                DBG_PRINT("Bitstream.skip.F_X2: %" PRIu64 ", notReadBits %zu - %s", n, (size_t)notReadBits, toStringImpl().c_str());
                return nX - notReadBits;
            }
        }

        /**
         * Read incoming bit in least-significant-bit (LSB) first order.
         * @return the read bit or `-1` if end-of-stream is reached.
         * @see msbFirst()
         */
        [[nodiscard]] int readBit() noexcept {
            if( canWrite() ) {
                return -1;
            }
            if( 0 < m_bitCount ) {
                --m_bitCount;
                return int((m_bitCache >> (m_bitCacheSizeRead - 1 - m_bitCount)) & data_type(0x01)); // LSB
            } else {
                fillCache();
                if( 0 < m_bitCacheSizeRead ) {
                    m_bitCount = m_bitCacheSizeRead - 1;
                    return int(m_bitCache & data_type(0x01)); // LSB
                } else {
                    return -1;
                }
            }
        }

        /**
         * Write given bit in least-significant-bit (LSB) first order.
         * @param bit
         * @return true if successful, otherwise false
         * @see msbFirst()
         */
        [[nodiscard]] bool writeBit(uint8_t bit) noexcept {
            if( !canWrite() ) {
                return false;
            }
            m_bitCache |= data_type(0x01 & bit) << m_bitCount; // LSB
            if( MaxBitCacheSize == ++m_bitCount ) {
                if( !writeCache() ) {
                    return false;
                }
            }
            m_bitCacheSizeRead = m_bitCount;
            return true;
        }

        /**
         * Read incoming bits in least-significant-bit (LSB) first order.
         * @param n number of bits, maximum 64 bits
         * @return the number of bits read. Zero for none, including errors.
         */
        [[nodiscard]] jau::nsize_t readBits64(jau::nsize_t n, data_type &r) noexcept {
            r = 0;
            if( MaxBitCacheSize < n || canWrite() || 0 == n ) {
                return 0;
            }
            if constexpr ( !useFastPathStream ) {
                // Slow path
                for(jau::nsize_t i = 0; i < n; ++i) {
                    const int b = readBit();
                    if( 0 > b ) {
                        return i;
                    }
                    r |= data_type(b) << i;
                }
            } else {
                // fast path
                jau::nsize_t c = n;
                const jau::nsize_t n1 = std::min(n, m_bitCount);  // remaining portion
                if( 0 < n1 ) {
                    const data_type m1 = (data_type(1) << n1) - data_type(1);
                    const jau::nsize_t s1 = m_bitCacheSizeRead - m_bitCount;  // LSB: right-shift to new bits
                    m_bitCount -= n1;
                    c -= n1;
                    r = m1 & (m_bitCache >> s1);  // LSB
                    if( 0 == c ) {
                        return n1;
                    }
                }
                assert(0 == m_bitCount);
                fillCache();
                if( 0 == m_bitCacheSizeRead ) {
                    return n1;
                }
                const nsize_t n2 = std::min(c, m_bitCacheSizeRead);  // full portion
                const data_type m2 = (data_type(1) << n2) - data_type(1);
                m_bitCount = m_bitCacheSizeRead - n2;
                c -= n2;
                r |= (m2 & m_bitCache) << n1;  // LSB
                assert(0 == c);
                (void)c;
            }
            return n;
        }

        /**
         * Write given bits in least-significant-bit (LSB) first order.
         * @param n number of bits, maximum 64 bits
         * @param bits the bits to write
         * @return the number of bits written. Zero for none, including errors.
         */
        [[nodiscard]] jau::nsize_t writeBits64(jau::nsize_t n, data_type bits) noexcept {
            if( MaxBitCacheSize < n || !canWrite() || 0 == n ) {
                return 0;
            }
            if constexpr ( !useFastPathStream ) {
                // Slow path
                for(jau::nsize_t i = 0; i < n; ++i) {
                    if( !writeBit( uint8_t( (bits >> i) & data_type(0x1) ) ) ) {
                        return i;
                    }
                }
            } else {
                // fast path
                jau::nsize_t c = n;
                const jau::nsize_t n1 = std::min(n, MaxBitCacheSize-m_bitCount);  // remaining free cache portion
                if( 0 < n1 ) {
                    const data_type m1 = (data_type(1) << n1) - data_type(1);
                    const jau::nsize_t s1 = m_bitCount;  // LSB: left-shift to free bit-pos
                    m_bitCount += n1;
                    c -= n1;
                    m_bitCache |= (m1 & bits) << s1;  // LSB
                    if( MaxBitCacheSize == m_bitCount ) {
                        if( !writeCache() ) {
                            return 0;
                        }
                    }
                    if( 0 == c ) {
                        return n1;
                    }
                }
                assert(0 == m_bitCount);
                {
                    const jau::nsize_t n2 = std::min(c, MaxBitCacheSize);  // full portion
                    const data_type m2 = (data_type(1) << n2) - data_type(1);
                    m_bitCount = n2;
                    c -= n2;
                    m_bitCache = (m2 & (bits >> n1));  // LSB
                    if( MaxBitCacheSize == m_bitCount ) {
                        if( !writeCache() ) {
                            return n1;
                        }
                    }
                }
                assert(0 == c);
                (void)c;
                m_bitCacheSizeRead = m_bitCount;
            }
            return n;
        }

        /**
         * Read incoming `uint8_t` via readBits32().
         *
         * In case of a `int8_t` 2-complement signed value, simply cast the result.
         *
         * @param bits reference to result
         * @return true if successful, otherwise false
         */
        [[nodiscard]] bool readUInt8(uint8_t &bits) noexcept {
            if( 0 == m_bitCount && useFastPathTypes ) {
                return m_bytes->read(bits);
            }
            uint64_t tmp;
            if( 8 != readBits64(8, tmp) ) {
                return false;
            }
            bits = static_cast<uint8_t>(tmp);
            return true;
        }

        /**
         * Write the given 8 bits via {@link #writeBits31(int, int)}.
         * @return true if successful, otherwise false
         */
        [[nodiscard]] bool writeUInt8(uint8_t bits) noexcept {
            if( 0 == m_bitCount && useFastPathTypes ) {
                return m_bytes->write(bits);
            } else {
                return 8 == writeBits64(8, bits);
            }
        }

        /**
         * Read `uint16_t`.
         *
         * If stream byteOrder() != lb_endian_t::native, result is bytes swapped via jau::bswap().
         * @param bits reference to result
         * @return true if successful, otherwise false
         */
        [[nodiscard]] bool readUInt16(uint16_t& bits) noexcept {
            if( 0 == m_bitCount && useFastPathTypes ) {
                return m_bytes->readU16(bits);
            }
            uint64_t tmp;
            if( 16 != readBits64(16, tmp) ) {
                return false;
            }
            bits = static_cast<uint16_t>(tmp);
            if( byteOrder() != lb_endian_t::native ) {
                bits = jau::bswap(bits);
            }
            return true;
        }
        /**
         * Read `int16_t`.
         *
         * If stream byteOrder() != lb_endian_t::native, result is bytes swapped via jau::bswap().
         * @param bits reference to result
         * @return true if successful, otherwise false
         */
        [[nodiscard]] bool readSInt16(int16_t& bits) noexcept { return readUInt16( *reinterpret_cast<uint16_t*>(&bits) ); }

        /**
         * Write `uint16_t`
         *
         * If stream byteOrder() != lb_endian_t::native, result is bytes swapped via jau::bswap().
         * @param bits data to write
         * @return true if successful, otherwise false
         */
        [[nodiscard]] bool writeUInt16(uint16_t bits) noexcept {
            if( !canWrite() ) {
                return false;
            }
            if( byteOrder() != lb_endian_t::native ) {
                bits = jau::bswap(bits);
            }
            if( 0 == m_bitCount && useFastPathTypes ) {
                // fast path
                return 2 == m_bytes->write(&bits, 2);
            } else {
                return 16 == writeBits64(16, bits);
            }
        }

        /**
         * Read `uint32_t`.
         *
         * If stream byteOrder() != lb_endian_t::native, result is bytes swapped via jau::bswap().
         * @param bits reference to result
         * @return true if successful, otherwise false
         */
        [[nodiscard]] bool readU32(uint32_t& bits) noexcept {
            if( 0 == m_bitCount && useFastPathTypes ) {
                // fast path
                if( canWrite() || 4 != m_bytes->read(&bits, 4) ) {
                    return false;
                }
            } else {
                uint64_t tmp;
                if( 32 != readBits64(32, tmp) ) {
                    return false;
                }
                bits = static_cast<uint32_t>(tmp);
            }
            if ( byteOrder() != lb_endian_t::native ) {
                bits = jau::bswap(bits);
            }
            return true;
        }

        /**
         * Write `uint32_t`.
         *
         * If stream byteOrder() != lb_endian_t::native, result is bytes swapped via jau::bswap().
         * @param bits data to write
         * @return true if successful, otherwise false
         */
        [[nodiscard]] bool writeU32(uint32_t bits) noexcept {
            if( !canWrite() ) {
                return false;
            }
            if ( byteOrder() != lb_endian_t::native ) {
                bits = jau::bswap(bits);
            }
            if( 0 == m_bitCount && useFastPathTypes ) {
                // fast path
                return m_bytes->write(&bits, 4);
            } else {
                return 32 == writeBits64(32, bits);
            }
        }

        std::string toString() const {
            std::string s("Bitstream[");
            return s.append(toStringImpl()).append("]");
        }
        std::string toStringImpl() const {
            std::string s;
            s.append(canWrite() ? "W" : "R");
            if( !m_bytes->isOpen() ) {
                s.append(" [closed]");
            }
            s.append(", order[byte ").append(jau::to_string(m_bytes->byteOrder()))
             .append("], pos ")
             .append(std::to_string(position())).append(" (").append(std::to_string(m_bytes->position()))
             .append(" bytes), cache[size ").append(std::to_string(cachedBitCount())).append("/").append(std::to_string(m_bitCacheSizeRead))
             .append(", pos ").append(std::to_string(cachedBitPos()))
             .append("), data ").append(toHexBinaryString(m_bitCache)).append("]");
            return s;
        }

        static std::string toHexBinaryString(uint64_t v, unsigned bitCount = MaxBitCacheSize) {
            unsigned nibbles = 0 == bitCount ? 2 : (bitCount + 3) / 4;
            std::string fmt("[%u: 0x%0");
            fmt.append(std::to_string(nibbles)).append(PRIx64);
            return jau::format_string(fmt, bitCount, v).append(", ").append(jau::toBitString(v)).append("]");
        }
    };

    inline std::ostream& operator<<(std::ostream& os, const Bitstream& v) { return os << v.toString(); }


    /**@}*/

}  // namespace jau::io

#endif /* JAU_IO_BIT_STREAM_HPP_ */
