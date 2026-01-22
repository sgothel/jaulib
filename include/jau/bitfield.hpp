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
#include <cassert>
#include <climits>
#include <cmath>
#include <cstring>
#include <limits>
#include <string_view>

#include <jau/basic_types.hpp>
#include <jau/byte_util.hpp>
#include <jau/cpp_lang_util.hpp>
#include <jau/int_math.hpp>
#include <jau/int_math_ct.hpp>
#include <jau/int_types.hpp>
#include <jau/string_util.hpp>
#include <jau/type_concepts.hpp>

namespace jau {

    /** \addtogroup ByteUtils
     *
     *  @{
     */

    /**
     * Simple statically sized bitfield template for efficient bit storage access.
     *
     * Bit-position and bit-order are in least-significant-bits (lsb) first.
     *
     * Implementations utilizes an in-memory `std::array<StorageType, (BitSize+StorageTypeBits-1)/StorageTypeBits>`
     * with unsigned integral StorageType of sizeof(StorageType) <= sizeof(size_t).
     *
     * Similar to std::bitset, but providing custom methods.
     *
     * @see jau::bitheap
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

      public:
        static constexpr bool in_range(size_type bitpos) { return bitpos < bit_size; }
        static constexpr bool in_range(size_type bitpos, size_type length) {
            return bitpos + length <= bit_size;
        }

        /** Constructs an empty bitfield instance */
        constexpr bitfield_t() noexcept { clear(); }

        /**
         * Constructs a bitfield instance, initialized with `bitstr` msb bit-pattern.
         * @param bitstr most-significat (msb) bit string pattern
         * @throws jau::IllegalArgumentError if bitstr put failed
         * @see put(std::string_view)
         */
        bitfield_t(std::string_view bitstr) {
            clear();
            if( !put(0, bitstr) ) {
                throw jau::IllegalArgumentError("Invalid bit-patter "+std::string(bitstr), E_FILE_LINE);
            }
        }

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

        /**
         * Writes the bit value `v` to position `bitpos` into this storage.
         * @returns true on success, otherwise false
         */
        constexpr bool put(size_type bitpos, bool v) noexcept {
            if ( !in_range(bitpos) ) {
                return false;
            }
            const size_type u = bitpos >> unit_shift;
            const size_type b = bitpos - (u << unit_shift);
            const unit_type m = one_u << b;
            if ( v ) {
                storage[u] |= m;
            } else {
                storage[u] &= ~m;
            }
            return true;
        }

        /**
         * Flips the bit value at position `bitpos` in this storage.
         * @returns true on success, otherwise false
         */
        constexpr bool flip(size_type bitpos) noexcept {
            if ( !in_range(bitpos) ) {
                return false;
            }
            const size_type u = bitpos >> unit_shift;
            const size_type b = bitpos - (u << unit_shift);
            const unit_type m = one_u << b;
            if ( storage[u] & m ) {
                storage[u] &= ~m;
            } else {
                storage[u] |= m;
            }
            return true;
        }

        /*** Flips all bits in this storage. */
        constexpr bitfield_t &flip() noexcept {
            size_type remaining = bit_size;
            for ( size_type i = 0; i < unit_size; ++i ) {
                storage[i] = ~storage[i] & bit_mask<unit_type>(remaining);
                remaining -= unit_bit_size;
            }
            return *this;
        }

        /*** Reverse all bits in this storage. */
        constexpr bitfield_t &reverse() noexcept {
            const size_type s0 = bit_size & (unit_bit_size-1); // % unit_bit_size;
            if( 0 == s0 ) { // fast-path, swap units
                size_type l = 0, r = unit_size-1;
                while ( l < r ) {
                    const unit_type v_l = jau::rev_bits(storage[l]);
                    const unit_type v_r = jau::rev_bits(storage[r]);
                    storage[l++] = v_r;
                    storage[r--] = v_l;
                }
                if( l == r ) { // last odd middle
                    storage[l] = jau::rev_bits(storage[l]);
                }
            } else { // slow-path, swap bits
                for(size_type l = 0, r = bit_size-1; l < r; ++l, --r) {
                    const bool s = get(l);
                    (void)put(l, get(r));
                    (void)put(r, s);
                }
            }
            return *this;
        }
        /**
         * Sets the bit at position `bitpos` of this storage.
         * @returns true on success, otherwise false
         */
        constexpr bool set(size_type bitpos) noexcept { return put(bitpos, true); }
        /**
         * Clear the bit at position `bitpos` of this storage.
         * @returns true on success, otherwise false
         */
        constexpr bool clr(size_type bitpos) noexcept { return put(bitpos, false); }
        /**
         * Clear the bit at position `bitpos` of this storage.
         * @returns true on success, otherwise false
         */
        constexpr bool reset(size_type bitpos) noexcept { return put(bitpos, false); }

        /**
         * Copies the bit at position `srcBitpos` to position `dstBitpos`, returning the copied bit-value.
         * @returns true on success, otherwise false
         */
        bool copy(size_type srcBitpos, size_type dstBitpos) noexcept {
            return put(dstBitpos, get(srcBitpos));
        }

        /*** Reads `length` bits from this storage, starting with the lowest bit from the storage position `bitpos`.*/
        unit_type getUnit(size_type bitpos, size_type length) const noexcept {
            if ( 0 == length || length > unit_bit_size || !in_range(bitpos, length) ) {
                return 0;
            }
            const size_type u = bitpos >> unit_shift;
            const size_type b = bitpos - (u << unit_shift);
            if ( 0 == b ) {
                // fast path
                const unit_type m = bit_mask<unit_type>(length);  // mask of chunk
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

        /**
         * Writes `length` bits of given `data` into this storage, starting with the lowest bit from the storage position `bitpos`.
         * @returns true on success, otherwise false
         */
        bool putUnit(size_type bitpos, size_type length, unit_type data) noexcept {
            if ( 0 == length ) {
                return true;
            } else if ( length > unit_bit_size || !in_range(bitpos, length) ) {
                return false;
            }
            const size_type u = bitpos >> unit_shift;
            const size_type b = bitpos - (u << unit_shift);
            if ( 0 == b ) {
                // fast path
                const unit_type m = bit_mask<unit_type>(length);  // mask of chunk
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
                    /**
                     * g++ 4:14.2.0-1 Debian 13
                     * g++ bug: False positive of '-Warray-bounds'
                     * See <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=56456>
                     */
                    PRAGMA_DISABLE_WARNING_PUSH
                    PRAGMA_DISABLE_WARNING_ARRAY_BOUNDS
                    storage[u + 1] = ((~m2) & storage[u + 1])     // keep non-written storage bits
                                     | (m2 & (data >> l1));       // overwrite storage w/ used data bits
                    PRAGMA_DISABLE_WARNING_POP
                }
            }
            return true;
        }

        /**
         * Writes `bitstr` msb bit-pattern into this storage, starting with the lowest bit from the storage position `bitpos`.
         * @param bitpos bit position to insert
         * @param bitstr most-significat (msb) bit string pattern
         * @returns true on success, otherwise false
         */
        bool put(size_type bitpos, std::string_view bitstr) noexcept {
            if ( 0 == bitstr.size() ) {
                return true;
            } else if ( !in_range(bitpos, bitstr.size()) ) {
                return false;
            }
            size_type left = bitstr.size();
            size_type strPos = left - std::min(unit_bit_size, left);
            while(left > 0) {
                size_type len = std::min(unit_bit_size, left);
                std::string_view segment = bitstr.substr(strPos, len);
                const auto [val, sz, ok] = jau::fromBitString(segment, bit_order_t::msb);
                if( !ok || sz != len || !putUnit(bitpos, len, unit_type(val)) ) {
                    return false;
                }
                bitpos += len;
                strPos -= len;
                left -= len;
            }
            return true;
        }

        /**
         * Set `length` bits starting at `bitpos` of this bitfield to the given value `bit`.
         * @returns true on success, otherwise false
         */
        bool set(size_type bitpos, size_type length, bool bit) noexcept {
            if ( 0 == length ) {
                return true;
            } else if ( !in_range(bitpos, length) ) {
                return false;
            }
            const unit_type v = bit ? std::numeric_limits<unit_type>::max() : 0;
            size_type remaining = length;
            size_type u = bitpos >> unit_shift;
            {
                const size_type b = bitpos - (u << unit_shift);
                if ( b > 0 ) {
                    const size_type l = std::min(unit_bit_size-b, remaining);
                    if (!putUnit(bitpos, l, v)) { // first incomplete unit
                        return false;
                    }
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
                if (!putUnit(bitpos, l, v)) { // last incomplete unit
                    return false;
                }
                remaining -= l;
            }
            assert(0 == remaining);
            (void)remaining;
            return true;
        }
        /** Set all bits of this bitfield to the given value `bit`. */
        bitfield_t &setAll(bool bit) noexcept {
            (void)set(0, bit_size, bit);
            return *this;
        }

        /**
         * Copies `length` bits at position `srcBitpos` to position `dstBitpos`, returning the copied bits.
         * @returns true on success, otherwise false
         */
        bool copyUnit(size_type srcBitpos, size_type dstBitpos, size_type length) noexcept {
            return putUnit(dstBitpos, length, getUnit(srcBitpos, length));
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

        template<size_t OBitSize>
        bool put(size_t bitpos, const bitfield_t<StorageType, OBitSize>& o) {
            size_t length = o.bit_size;
            if ( 0 == length ) {
                return true;
            } else if ( !in_range(bitpos, length) ) {
                return false;
            }
            const size_type unit_count = ( length + unit_bit_size - 1 ) >> unit_shift;
            const size_type unit_pos = bitpos >> unit_shift;
            const size_type unit_bit_pos = bitpos - (unit_pos << unit_shift);
            if ( 0 == unit_bit_pos ) {
                size_type l = std::min(length, unit_bit_size);
                size_type i = bitpos;
                for ( size_type u = 0; u < unit_count && 0 < length; ++u ) {
                    if( !putUnit( i, l, o.storage[u] ) ) {
                        return false;
                    }
                    length -= l;
                    i += l;
                    l = std::min(length, unit_bit_size);
                }
            } else {
                for ( size_type i = 0; i < length; ++i ) {
                    if( !put(bitpos + i, o.get(i)) ) {
                        return false;
                    }
                }
            }
            return true;
        }

        template<size_t BitLength>
        std::pair<bitfield_t<StorageType, BitLength>, bool> subbits(size_type bitpos) const noexcept {
            if ( 0 == BitLength ) {
                return { bitfield_t<StorageType, BitLength>(), true };
            } else if ( !in_range(bitpos, BitLength) ) {
                return { bitfield_t<StorageType, BitLength>(), false };
            }
            std::pair<bitfield_t<StorageType, BitLength>, bool> r{ bitfield_t<StorageType, BitLength>(), true };
            size_type length = BitLength;
            const size_type unit_count = ( BitLength + unit_bit_size - 1 ) >> unit_shift;
            const size_type unit_pos = bitpos >> unit_shift;
            const size_type unit_bit_pos = bitpos - (unit_pos << unit_shift);
            if ( 0 == unit_bit_pos ) {
                size_type l = std::min(length, unit_bit_size);
                size_type i = 0;
                for ( size_type u = unit_pos; u < unit_count && 0 < length; ++u ) {
                    if( !r.first.putUnit( i, l, storage[u] ) ) {
                        return { bitfield_t<StorageType, BitLength>(), false };
                    }
                    length -= l;
                    i += l;
                    l = std::min(length, unit_bit_size);
                }
            } else {
                for(size_type i = 0; i < length; ++i) {
                    if( !r.first.put(i, get(bitpos+i)) ) {
                        return { bitfield_t<StorageType, BitLength>(), false };
                    }
                }
            }
            return r;
        }

        std::string toString(size_type bitpos, size_type length, PrefixOpt prefix = PrefixOpt::none) const noexcept {
            if ( 0 == length || !in_range(bitpos, length) ) {
                return "";
            }
            std::string r;
            r.reserve(length + (prefix == PrefixOpt::none ? 0 : 2));
            if ( prefix == PrefixOpt::prefix ) {
                r.append("0b");
            }
            const size_type unit_count = ( length + unit_bit_size - 1 ) >> unit_shift;
            const size_type unit_pos = bitpos >> unit_shift;
            const size_type bit_pos = bitpos - (unit_pos << unit_shift);
            if ( 0 == bit_pos ) {
                // fast path
                const size_type sz0 = (unit_size - 1) * unit_bit_size;;
                size_type l = length > sz0 ? length - sz0 : std::min(length, unit_bit_size);
                for ( size_type i = unit_pos + unit_count; i-- > unit_pos && 0 < length; ) {
                    r.append( jau::toBitString(storage[i], bit_order_t::msb, PrefixOpt::none, l) );
                    length -= l;
                    l = std::min(length, unit_bit_size);
                }
            } else {
                size_type i = bitpos + length;
                while(length-- > 0) {
                    r.push_back(get(--i) ? '1' : '0');
                }
            }
            return r;
        }
        std::string toString(PrefixOpt prefix = PrefixOpt::none) const noexcept {
            return toString(0, bit_size, prefix);
        }

        std::string infoString() const noexcept {
            return "bitfield[unit[bits "+std::to_string(unit_bit_size)+", count "+std::to_string(unit_size)+
                    "], bits"+std::to_string(bit_size)+": "+toString()+"]";
        }
    };

    template<jau::req::unsigned_integral StorageType, size_t BitSize>
        requires requires (StorageType) { sizeof(StorageType) <= sizeof(size_t); }
    inline std::ostream &operator<<(std::ostream &out, const bitfield_t<StorageType, BitSize> &v) {
        return out << v.toString();
    }

    /**
     * Simple bitfield template for efficient bit storage access.
     *
     * Implementations utilizes an in-memory `std::array<StorageType = jau::nsize_t, (BitSize+StorageTypeBits-1)/StorageTypeBits>`.
     *
     * @see jau::bitfield_t
     * @see jau::bitheap
     * @see jau::nsize_t
     */
    template<size_t BitSize>
    using bitfield = bitfield_t<jau::nsize_t, BitSize>;

    /**@}*/

}  // namespace jau

#endif /*  JAU_BITFIELD_HPP_ */
