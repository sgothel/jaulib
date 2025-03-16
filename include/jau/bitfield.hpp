/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2022 Gothel Software e.K.
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
#ifndef JAU_BITFIELD_HPP_
#define JAU_BITFIELD_HPP_

#include <array>

#include <cstdint>
#include <limits>
#include <unistd.h>
#include <jau/int_math_ct.hpp>

namespace jau {
    /**
     * Simple bitfield template for efficient bit storage access in O(1).
     *
     * Implementations utilizes an in-memory `std::array<uint32_t, (BitSize+31)/5>`.
     */
    template <size_t BitSize>
    class bitfield {
      public:
        static constexpr size_t unit_shift = 5;
        static constexpr size_t unit_bit_size = 32;
        static constexpr size_t bit_size = BitSize;

      private:
        static constexpr size_t storage_size = std::max<size_t>(1, ( bit_size + unit_bit_size - 1 ) >> unit_shift);
        typedef std::array<uint32_t, storage_size> storage_t;
        storage_t storage;

        static constexpr bool in_range(size_t bit_size_, size_t bitnum) { return bitnum < bit_size_; }

        /// Returns the 32 bit mask of n-bits, i.e. n low order 1’s
        static constexpr uint32_t bit_mask(size_t n) noexcept {
            if( unit_bit_size > n ) {
                return ( 1 << n ) - 1;
            } else if ( unit_bit_size == n ) {
                return std::numeric_limits<uint32_t>::max();
            } else {
                return 0;
            }
        }

      public:
        static constexpr bool in_range(size_t bitnum) { return bitnum < bit_size; }

        constexpr bitfield() noexcept { clear(); }

        /*** Clears whole bitfield, i.e. sets all bits to zero. */
        constexpr void clear() noexcept {
            for(size_t i=0; i<storage_size; ++i) {
                storage[i] = 0;
            }
        }

        /*** Reads the bit value at position `bitnum`. */
        constexpr bool operator[](size_t bitnum) const noexcept { return get(bitnum); }

        /*** Reads the bit value at position `bitnum`. */
        constexpr bool get(size_t bitnum) const noexcept {
            if( !in_range(bitnum) ) {
                return false;
            }
            const size_t u = bitnum >> unit_shift;
            const size_t b = bitnum - ( u << unit_shift );
            return 0 != ( storage[u] & ( 1U << b ) ) ;
        }
        /*** Writes the bit value `v` to position `bitnum` into this storage. */
        constexpr void put(size_t bitnum, bool v) noexcept {
            if( !in_range(bitnum) ) {
                return;
            }
            const size_t u = bitnum >> unit_shift;
            const size_t b = bitnum - ( u << unit_shift );
            const uint32_t m = 1 << b;
            if( v ) {
                storage[u] |=  m;
            } else {
                storage[u] &= ~m;
            }
        }
        /*** Sets the bit at position `bitnum` of this storage. */
        constexpr void set(size_t bitnum) noexcept { put(bitnum, true); }
        /*** Clear the bit at position `bitnum` of this storage. */
        constexpr void clr(size_t bitnum) noexcept { put(bitnum, false); }

        /*** Copies the bit at position `srcBitnum` to position `dstBitnum`, returning the copied bit-value. */
        bool copy(size_t srcBitnum, size_t dstBitnum) noexcept {
            const bool bit = get(srcBitnum);
            put(dstBitnum, bit);
            return bit;
        }

        /*** Reads `length` bits from this storage, starting with the lowest bit from the storage position `lowBitnum`.*/
        uint32_t getu32(size_t lowBitnum, size_t length) const noexcept {
            if( 0 == length || length > unit_bit_size || !in_range(bit_size-length+1, lowBitnum) ) {
                return 0;
            }
            const size_t u = lowBitnum >> unit_shift;
            const size_t left = unit_bit_size - ( lowBitnum - ( u << unit_shift ) ); // remaining bits of first chunk storage
            if( unit_bit_size == left ) {
                // fast path
                const uint32_t m = bit_mask(length);                  // mask of chunk
                return m & storage[u];
            } else {
                // slow path
                const size_t l1 = std::min(length, left);             // length of first chunk < 32
                const uint32_t m1 = ( 1 << l1 ) - 1;                  // mask of first chunk
                const uint32_t d1 = m1 & ( storage[u] >> lowBitnum ); // data of first chunk
                const size_t l2 = length - l1;                        // length of second chunk < 32
                if( l2 > 0 ) {
                    const uint32_t m2 = ( 1 << l2 ) - 1;              // mask of second chunk
                    return d1 | ( ( m2 & storage[u+1] ) << l1 );      // data combined chunk 1+2
                } else {
                    return d1;                                        // data of chunk 1 only
                }
            }
        }

        /*** Writes `length` bits of given `data` into this storage, starting with the lowest bit from the storage position `lowBitnum`.*/
        void putu32(size_t lowBitnum, size_t length, uint32_t data) noexcept {
            if( 0 == length || length > unit_bit_size || !in_range(bit_size-length+1, lowBitnum) ) {
                return;
            }
            const size_t u = lowBitnum >> unit_shift;
            const size_t left = unit_bit_size - ( lowBitnum - ( u << unit_shift ) ); // remaining bits of first chunk storage
            if( unit_bit_size == left ) {
                // fast path
                const uint32_t m = bit_mask(length);                  // mask of chunk
                storage[u] = ( ( ~m ) & storage[u] )                  // keep non-written storage bits
                             | (  m   & data );                       // overwrite storage w/ used data bits
            } else {
                // slow path
                const size_t l1 = std::min(length, left);             // length of first chunk < 32
                const uint32_t m1 = ( 1 << l1 ) - 1;                  // mask of first chunk
                storage[u] = ( (~( m1 << lowBitnum ) ) & storage[u])  // keep non-written storage bits
                             | ( ( m1 & data ) << lowBitnum);         // overwrite storage w/ used data bits
                const size_t l2 = length - l1;                        // length of second chunk < 32
                if( l2 > 0 ) {
                    const uint32_t m2 = ( 1 << l2 ) - 1;              // mask of second chunk
                    storage[u+1] = ( ( ~m2 ) & storage[u+1] )         // keep non-written storage bits
                                   | (  m2   & ( data >> l1 ) );      // overwrite storage w/ used data bits
                }
            }
        }

        /*** Copies `length` bits at position `srcLowBitnum` to position `dstLowBitnum`, returning the copied bits.*/
        uint32_t copyu32(size_t srcBitnum, size_t dstBitnum, size_t length) noexcept {
            const uint32_t data = getu32(srcBitnum, length);
            putu32(dstBitnum, length, data);
            return data;
        }

        /*** Returns the number of set bits within this bitfield. */
        size_t bitCount() const noexcept {
            size_t c = 0;
            for(size_t i=0; i<storage_size; ++i) {
                c += jau::ct_bit_count(storage[i]);
            }
            return c;
        }
    };
}

#endif /*  JAU_BITFIELD_HPP_ */

