/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2022-2025 Gothel Software e.K.
 *
 * SPDX-License-Identifier: MIT
 *
 * This Source Code Form is subject to the terms of the MIT License
 * If a copy of the MIT was not distributed with this file,
 * you can obtain one at https://opensource.org/license/mit/.
 */
#ifndef JAU_BITFIELD_HPP_
#define JAU_BITFIELD_HPP_

#include <unistd.h>

#include <array>
#include <cmath>
#include <cstring>
#include <limits>
#include <type_traits>

#include <jau/basic_types.hpp>
#include <jau/cpp_lang_util.hpp>
#include <jau/int_math.hpp>
#include <jau/int_math_ct.hpp>
#include <jau/string_util.hpp>
#include <jau/type_concepts.hpp>

namespace jau {
    /**
     * Simple bitfield template for efficient bit storage access in O(1).
     *
     * Implementations utilizes an in-memory `std::array<StorageType, (BitSize+31)/32>`
     * with unsigned integral StorageType of sizeof(StorageType) <= sizeof(size_t).
     *
     * Similar to std::bitset, but providing custom methods.
     */
    template<jau::req::unsigned_integral StorageType, size_t BitSize>
    requires requires (StorageType) { sizeof(StorageType) <= sizeof(size_t); }
    class bitfield_t {
      public:
        typedef StorageType unit_type;                                                ///< Unit data type
        typedef size_t   size_type;                                                   ///< size_t data type, bit position and count
        static constexpr size_type unit_byte_size = sizeof(unit_type);                ///< One unit size in bytes
        static constexpr size_type unit_bit_size = unit_byte_size * CHAR_BIT;         ///< One unit size in bits
        static constexpr size_type unit_shift = jau::log2_byteshift(unit_byte_size);  ///< One unit shift amount
        static constexpr size_type bit_size = BitSize;                                ///< Storage size in bits

        /** Returns storage size in bits */
        constexpr size_type size() const noexcept { return bit_size; }

        /// Storage size in units
        static constexpr size_type unit_size = std::max<size_type>(1, (bit_size + unit_bit_size - 1) >> unit_shift);

      private:
        static constexpr unit_type one_u = 1;

        typedef std::array<unit_type, unit_size> storage_t;
        storage_t storage;

        /// Returns the unit_type bit mask of n-bits, i.e. n low order 1’s
        static constexpr unit_type bit_mask(size_type n) noexcept {
            if ( n >= unit_bit_size ) {
                return std::numeric_limits<unit_type>::max();
            } else {
                return (one_u << n) - one_u;
            }
        }

      public:
        static constexpr bool in_range(size_type bitpos) { return bitpos < bit_size; }
        static constexpr bool in_range(size_type bitpos, size_type length) {
            return bitpos + length <= bit_size;
        }

        constexpr bitfield_t() noexcept { clear(); }

        /*** Clears whole bitfield, i.e. sets all bits to zero. */
        constexpr void clear() noexcept {
            std::memset(storage.data(), 0, unit_byte_size * unit_size);
        }
        /*** Clears whole bitfield, i.e. sets all bits to zero. */
        constexpr bitfield_t &reset() noexcept {
            clear();
            return *this;
        }

        /*** Reads the bit value at position `bitpos`. */
        constexpr bool operator[](size_type bitpos) const noexcept { return get(bitpos); }

        /*** Reads the bit value at position `bitpos`. */
        constexpr bool get(size_type bitpos) const noexcept {
            if ( !in_range(bitpos) ) {
                return false;
            }
            const size_type u = bitpos >> unit_shift;
            const size_type b = bitpos - (u << unit_shift);
            return storage[u] & (one_u << b);
        }
        /*** Writes the bit value `v` to position `bitpos` into this storage. */
        constexpr bitfield_t &put(size_type bitpos, bool v) noexcept {
            if ( !in_range(bitpos) ) {
                return *this;
            }
            const size_type u = bitpos >> unit_shift;
            const size_type b = bitpos - (u << unit_shift);
            const unit_type m = one_u << b;
            if ( v ) {
                storage[u] |= m;
            } else {
                storage[u] &= ~m;
            }
            return *this;
        }
        /*** Flips the bit value at position `bitpos` in this storage. */
        constexpr bitfield_t &flip(size_type bitpos) noexcept {
            if ( !in_range(bitpos) ) {
                return *this;
            }
            const size_type u = bitpos >> unit_shift;
            const size_type b = bitpos - (u << unit_shift);
            const unit_type m = one_u << b;
            if ( storage[u] & m ) {
                storage[u] &= ~m;
            } else {
                storage[u] |= m;
            }
            return *this;
        }
        /*** Flips all bits in this storage. */
        constexpr bitfield_t &flip() noexcept {
            size_type remaining = bit_size;
            for ( size_type i = 0; i < unit_size; ++i ) {
                const unit_type m = bit_mask(remaining);
                storage[i] = ~storage[i] & m;
                remaining -= unit_bit_size;
            }
            return *this;
        }
        /*** Sets the bit at position `bitpos` of this storage. */
        constexpr bitfield_t &set(size_type bitpos) noexcept { return put(bitpos, true); }
        /*** Clear the bit at position `bitpos` of this storage. */
        constexpr bitfield_t &clr(size_type bitpos) noexcept { return put(bitpos, false); }
        /*** Clear the bit at position `bitpos` of this storage. */
        constexpr bitfield_t &reset(size_type bitpos) noexcept { return put(bitpos, false); }

        /*** Copies the bit at position `srcBitpos` to position `dstBitpos`, returning the copied bit-value. */
        bool copy(size_type srcBitpos, size_type dstBitpos) noexcept {
            const bool bit = get(srcBitpos);
            put(dstBitpos, bit);
            return bit;
        }

        /*** Reads `length` bits from this storage, starting with the lowest bit from the storage position `lowBitpos`.*/
        unit_type getUnit(size_type bitpos, size_type length) const noexcept {
            if ( 0 == length || length > unit_bit_size || !in_range(bitpos, length) ) {
                return 0;
            }
            const size_type u = bitpos >> unit_shift;
            const size_type b = bitpos - (u << unit_shift);
            if ( 0 == b ) {
                // fast path
                const unit_type m = bit_mask(length);             // mask of chunk
                return m & storage[u];
            } else {
                // slow path
                const size_type left = unit_bit_size - b;         // remaining bits of first chunk storage
                const size_type l1 = std::min(length, left);      // length of first chunk < unit_bit_size
                const unit_type m1 = (one_u << l1) - one_u;       // mask of first chunk
                const unit_type d1 = m1 & (storage[u] >> b);      // data of first chunk
                const size_type l2 = length - l1;                 // length of second chunk < unit_bit_size
                if ( l2 > 0 ) {
                    const unit_type m2 = (one_u << l2) - one_u;   // mask of second chunk
                    return d1 | ((m2 & storage[u + 1]) << l1);    // data combined chunk 1+2
                } else {
                    return d1;  // data of chunk 1 only
                }
            }
        }

        /*** Writes `length` bits of given `data` into this storage, starting with the lowest bit from the storage position `lowBitpos`.*/
        bool putUnit(size_type bitpos, size_type length, unit_type data) noexcept {
            if ( 0 == length || length > unit_bit_size || !in_range(bitpos, length) ) {
                return false;
            }
            const size_type u = bitpos >> unit_shift;
            const size_type b = bitpos - (u << unit_shift);
            if ( 0 == b ) {
                // fast path
                const unit_type m = bit_mask(length);             // mask of chunk
                storage[u] = ((~m) & storage[u])                  // keep non-written storage bits
                             | (m & data);                        // overwrite storage w/ used data bits
            } else {
                // slow path
                const size_type left = unit_bit_size - b;         // remaining bits of first chunk storage
                const size_type l1 = std::min(length, left);      // length of first chunk < unit_bit_size
                const unit_type m1 = (one_u << l1) - one_u;       // mask of first chunk
                storage[u] = ((~(m1 << b)) & storage[u])          // keep non-written storage bits
                             | ((m1 & data) << b);                // overwrite storage w/ used data bits
                const size_type l2 = length - l1;                 // length of second chunk < unit_bit_size
                if ( l2 > 0 ) {
                    const unit_type m2 = (one_u << l2) - one_u;   // mask of second chunk
                    storage[u + 1] = ((~m2) & storage[u + 1])     // keep non-written storage bits
                                     | (m2 & (data >> l1));       // overwrite storage w/ used data bits
                }
            }
            return true;
        }

        /** Set `length` bits starting at `bitpos` of this bitfield to the given value `bit`. */
        bitfield_t &set(size_type bitpos, size_type length, bool bit) noexcept {
            if ( 0 == length || length > bit_size || !in_range(bitpos, length) ) {
                return *this;
            }
            const StorageType v = bit ? std::numeric_limits<StorageType>::max() : 0;
            size_type remaining = length;
            size_type u = bitpos >> unit_shift;
            {
                const size_type b = bitpos - (u << unit_shift);
                if ( b > 0 ) {
                    const size_type l = std::min(unit_bit_size-b, remaining);
                    putUnit(bitpos, l, v); // first incomplete unit
                    remaining -= l;
                    bitpos += l;
                    u = bitpos >> unit_shift;
                }
            }
            while ( remaining > unit_bit_size ) {
                storage[u++] = v;
                bitpos += unit_bit_size;
                remaining -= unit_bit_size;
            }
            if ( remaining > 0 ) {
                const size_type l = std::min(unit_bit_size, remaining);
                putUnit(bitpos, l, v); // last incomplete unit
                remaining -= l;
                bitpos += l;
            }
            assert(0 == remaining);
            return *this;
        }
        /** Set all bits of this bitfield to the given value `bit`. */
        bitfield_t &setAll(bool bit) noexcept {
            return set(0, bit_size, bit);
        }

        /*** Copies `length` bits at position `srcLowBitpos` to position `dstLowBitpos`, returning the copied bits.*/
        unit_type copyUnit(size_type srcBitpos, size_type dstBitpos, size_type length) noexcept {
            const unit_type data = getUnit(srcBitpos, length);
            putUnit(dstBitpos, length, data);
            return data;
        }

        /*** Returns the number of set bits within this bitfield. */
        size_type count() const noexcept {
            size_type c = 0;
            for ( size_type i = 0; i < unit_size; ++i ) {
                c += jau::bit_count(storage[i]);
            }
            return c;
        }
        constexpr bool operator==(const bitfield_t &rhs) const noexcept {
            if ( this == &rhs ) {
                return true;
            }
            if ( unit_size != rhs.unit_size ) {
                return false;
            }
            size_type i = 0;
            while ( i < unit_size && storage[i] == rhs.storage[i] ) {
                ++i;
            }
            return i == unit_size;
        }

        std::string toString(bool skipPrefix = true) const noexcept {
            std::string r;
            r.reserve(bit_size + (skipPrefix ? 0 : 2));
            if ( !skipPrefix ) {
                r.append("0b");
            }
            const size_type bsz_high = bit_size - (unit_size - 1) * unit_bit_size;
            bool is_high = true;
            for ( size_type i = unit_size; i-- > 0; ) {
                r.append( jau::to_string(storage[i], 2, Bool::True, is_high ? bsz_high : unit_bit_size) );
                is_high = false;
            }
            return r;
        }
        std::string infoString() const noexcept {
            return "bitfield[unit[bits "+std::to_string(unit_bit_size)+", count "+std::to_string(unit_size)+
                    "], bits"+std::to_string(bit_size)+": "+toString()+"]";
        }
    };

    template<typename StorageType, StorageType BitSize,
             std::enable_if_t<std::is_integral_v<StorageType>, bool> = true>
    std::ostream &operator<<(std::ostream &out, const bitfield_t<StorageType, BitSize> &v) {
        out << v.toString();
        return out;
    }

    /**
     * Simple bitfield template for efficient bit storage access in O(1).
     *
     * Implementations utilizes an in-memory `std::array<unsigned long, (BitSize+31)/32>`
     * with `unsigned long` StorageType.
     *
     * Alias for bitfield_t, using `unsigned long` for `StorageType`, i.e. at least 32bit or 64bit on LP64.
     */
    template<size_t BitSize>
    using bitfield = bitfield_t<unsigned long, BitSize>;

}  // namespace jau

#endif /*  JAU_BITFIELD_HPP_ */
