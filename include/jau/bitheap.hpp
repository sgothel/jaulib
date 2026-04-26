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
#ifndef JAU_BITHEAP_HPP_
#define JAU_BITHEAP_HPP_

#include <unistd.h>

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
     * Simple dynamically heap-sized bitheap for efficient bit storage access.
     *
     * Bit-position and bit-order are in least-significant-bits (lsb) first.
     *
     * Implementations utilizes a dynamic heap `std::vector<jau::nsize_t>` StorageType.
     *
     * Similar to std::bitset, but utilizing dynamic runtime heapsize and  providing custom methods.
     *
     * @see jau::bitfield
     * @see jau::nsize_t
     */
    class bitheap {
      public:
        typedef jau::nsize_t unit_type;                                               ///< Unit data type
        typedef size_t   size_type;                                                   ///< size_t data type, bit position and count
        static constexpr size_type unit_byte_size = sizeof(unit_type);                ///< One unit size in bytes
        static constexpr size_type unit_bit_size = unit_byte_size << 3;               ///< One unit size in bits
        static constexpr size_type unit_shift = jau::log2_byteshift(unit_byte_size);  ///< One unit shift amount
        static constexpr size_type unitSize(size_type bitSize) noexcept { return (bitSize + unit_bit_size - 1) >> unit_shift; }

      private:
        static constexpr unit_type one_u = 1;

        typedef std::vector<unit_type> storage_t;
        size_type m_bit_size;     ///< Storage size in bits
        size_type m_unit_size;    ///< Storage size in units
        storage_t m_storage;

      public:
        constexpr bool in_range(size_type bitpos) const noexcept { return bitpos < m_bit_size; }
        constexpr bool in_range(size_type bitpos, size_type length) const noexcept { return bitpos + length <= m_bit_size; }

        /** Constructs an empty bitheap instance of zero bits */
        constexpr bitheap() noexcept
        : m_bit_size(0), m_unit_size(0), m_storage(0, 0)
        { clear(); }

        /** Constructs a bitheap instance of bitSize bits */
        constexpr bitheap(size_type bitSize) noexcept
        : m_bit_size(bitSize),
          m_unit_size(unitSize(m_bit_size)),
          m_storage(m_unit_size, 0)
        { clear(); }

        /**
         * Constructs a bitheap instance, initialized with `bitstr` msb bit-pattern.
         * @param bitstr most-significat (msb) bit string pattern
         * @throws jau::IllegalArgumentError if bitstr put failed
         * @see put(std::string_view)
         */
        bitheap(std::string_view bitstr)
        : m_bit_size(bitstr.size()),
          m_unit_size(unitSize(m_bit_size)),
          m_storage(m_unit_size, 0)
        {
            clear();
            if( !put(0, bitstr) ) {
                throw jau::IllegalArgumentError("Invalid bit-patter "+std::string(bitstr), E_FILE_LINE);
            }
        }

        /** Returns storage size in bits */
        constexpr size_type size() const noexcept { return m_bit_size; }

        /** Returns storage size in storage-units */
        constexpr size_type unitSize() const noexcept { return m_unit_size; }

        /*** Returns the number of set bits within this bitfield. */
        size_type count() const noexcept {
            size_type c = 0;
            for ( size_type i = 0; i < m_unit_size; ++i ) {
                c += jau::bit_count(m_storage[i]);
            }
            return c;
        }

        void resize(size_t new_bit_size) {
            if (m_bit_size != new_bit_size) {
                m_bit_size = new_bit_size;
                m_unit_size = unitSize(m_bit_size);
                m_storage.resize(m_unit_size, 0);
            }
        }

        /*** Clears whole bitfield, i.e. sets all bits to zero. */
        constexpr void clear() noexcept {
            std::memset(m_storage.data(), 0, unit_byte_size * m_unit_size);
        }
        /*** Clears whole bitfield, i.e. sets all bits to zero. */
        constexpr bitheap &reset() noexcept {
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
            const size_type b = bitpos & (unit_bit_size-1); // % unit_bit_size;
            return m_storage[u] & (one_u << b);
        }
        /**
         * Writes the bit value `v` to position `bitpos` into this storage.
         * @returns true on sucess, otherwise false
         */
        constexpr bool put(size_type bitpos, bool v) noexcept {
            if ( !in_range(bitpos) ) {
                return false;
            }
            const size_type u = bitpos >> unit_shift;
            const size_type b = bitpos & (unit_bit_size-1); // % unit_bit_size;
            const unit_type m = one_u << b;
            if ( v ) {
                m_storage[u] |= m;
            } else {
                m_storage[u] &= ~m;
            }
            return true;
        }
        /**
         * Flips the bit value at position `bitpos` in this storage.
         * @returns true on sucess, otherwise false
         */
        constexpr bool flip(size_type bitpos) noexcept {
            if ( !in_range(bitpos) ) {
                return false;
            }
            const size_type u = bitpos >> unit_shift;
            const size_type b = bitpos & (unit_bit_size-1); // % unit_bit_size;
            const unit_type m = one_u << b;
            if ( m_storage[u] & m ) {
                m_storage[u] &= ~m;
            } else {
                m_storage[u] |= m;
            }
            return true;
        }
        /*** Flips all bits in this storage. */
        constexpr bitheap &flip() noexcept {
            size_type remaining = m_bit_size;
            for ( size_type i = 0; i < m_unit_size; ++i ) {
                m_storage[i] = ~m_storage[i] & bit_mask<unit_type>(remaining);
                remaining -= unit_bit_size;
            }
            return *this;
        }

        /*** Reverse all bits in this storage. */
        constexpr bitheap &reverse() noexcept {
            const size_type s0 = m_bit_size & (unit_bit_size-1); // % unit_bit_size;
            if (0 == s0 && m_unit_size > 0) { // fast-path, swap units
                size_type l = 0, r = m_unit_size-1;
                while ( l < r ) {
                    const unit_type v_l = jau::rev_bits(m_storage[l]);
                    const unit_type v_r = jau::rev_bits(m_storage[r]);
                    m_storage[l++] = v_r;
                    m_storage[r--] = v_l;
                }
                if( l == r ) { // last odd middle
                    m_storage[l] = jau::rev_bits(m_storage[l]);
                }
            } else if (m_bit_size>0) { // slow-path, swap bits
                for(size_type l = 0, r = m_bit_size-1; l < r; ++l, --r) {
                    const bool s = get(l);
                    (void)put(l, get(r));
                    (void)put(r, s);
                }
            }
            return *this;
        }
        /**
         * Sets the bit at position `bitpos` of this storage.
         * @returns true on sucess, otherwise false
         */
        constexpr bool set(size_type bitpos) noexcept { return put(bitpos, true); }
        /**
         * Clear the bit at position `bitpos` of this storage.
         * @returns true on sucess, otherwise false
         */
        constexpr bool clr(size_type bitpos) noexcept { return put(bitpos, false); }
        /**
         * Clear the bit at position `bitpos` of this storage.
         * @returns true on sucess, otherwise false
         */
        constexpr bool reset(size_type bitpos) noexcept { return put(bitpos, false); }

        /**
         * Copies the bit at position `srcBitpos` to position `dstBitpos`, returning the copied bit-value.
         * @returns true on sucess, otherwise false
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
            const size_type b = bitpos & (unit_bit_size-1); // % unit_bit_size;
            if ( 0 == b ) {
                // fast path
                const unit_type m = bit_mask<unit_type>(length);  // mask of chunk
                return m & m_storage[u];
            } else {
                // slow path
                const size_type left = unit_bit_size - b;         // remaining bits of first chunk storage
                const size_type l1 = std::min(length, left);      // length of first chunk < unit_bit_size
                const unit_type m1 = (one_u << l1) - one_u;       // mask of first chunk
                const unit_type d1 = m1 & (m_storage[u] >> b);    // data of first chunk
                const size_type l2 = length - l1;                 // length of second chunk < unit_bit_size
                if ( l2 > 0 ) {
                    const unit_type m2 = (one_u << l2) - one_u;   // mask of second chunk
                    return d1 | ((m2 & m_storage[u + 1]) << l1);  // data combined chunk 1+2
                } else {
                    return d1;  // data of chunk 1 only
                }
            }
        }

        /**
         * Writes `length` bits of given `data` into this storage, starting with the lowest bit from the storage position `bitpos`.
         * @returns true on sucess, otherwise false
         */
        bool putUnit(size_type bitpos, size_type length, unit_type data) noexcept {
            if ( 0 == length ) {
                return true;
            } else if ( length > unit_bit_size || !in_range(bitpos, length) ) {
                return false;
            }
            const size_type u = bitpos >> unit_shift;
            const size_type b = bitpos & (unit_bit_size-1); // % unit_bit_size;
            if ( 0 == b ) {
                // fast path
                const unit_type m = bit_mask<unit_type>(length);  // mask of chunk
                m_storage[u] = ((~m) & m_storage[u])              // keep non-written storage bits
                             | (m & data);                        // overwrite storage w/ used data bits
            } else {
                // slow path
                const size_type left = unit_bit_size - b;         // remaining bits of first chunk storage
                const size_type l1 = std::min(length, left);      // length of first chunk < unit_bit_size
                const unit_type m1 = (one_u << l1) - one_u;       // mask of first chunk
                m_storage[u] = ((~(m1 << b)) & m_storage[u])      // keep non-written storage bits
                             | ((m1 & data) << b);                // overwrite storage w/ used data bits
                const size_type l2 = length - l1;                 // length of second chunk < unit_bit_size
                if ( l2 > 0 ) {
                    const unit_type m2 = (one_u << l2) - one_u;   // mask of second chunk
                    m_storage[u + 1] = ((~m2) & m_storage[u + 1]) // keep non-written storage bits
                                     | (m2 & (data >> l1));       // overwrite storage w/ used data bits
                }
            }
            return true;
        }

        /**
         * Writes `bitstr` msb bit-pattern into this storage, starting with the lowest bit from the storage position `bitpos`.
         * @param bitpos bit position to insert
         * @param bitstr most-significat (msb) bit string pattern
         * @returns true on sucess, otherwise false
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
         * @returns true on sucess, otherwise false
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
                const size_type b = bitpos & (unit_bit_size-1); // % unit_bit_size;
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
                m_storage[u++] = v;
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
        bitheap &setAll(bool bit) noexcept {
            (void)set(0, m_bit_size, bit);
            return *this;
        }

        /**
         * Copies `length` bits at position `srcBitpos` to position `dstBitpos`, returning the copied bits.
         * @returns true on sucess, otherwise false
         */
        bool copyUnit(size_type srcBitpos, size_type dstBitpos, size_type length) noexcept {
            return putUnit(dstBitpos, length, getUnit(srcBitpos, length));
        }

        constexpr bool operator==(const bitheap &rhs) const noexcept {
            if ( this == &rhs ) {
                return true;
            }
            if ( m_unit_size != rhs.m_unit_size ) {
                return false;
            }
            size_type i = 0;
            while ( i < m_unit_size && m_storage[i] == rhs.m_storage[i] ) {
                ++i;
            }
            return i == m_unit_size;
        }

        bool put(size_t bitpos, const bitheap& o) {
            size_t length = o.m_bit_size;
            if ( 0 == length ) {
                return true;
            } else if ( !in_range(bitpos, length) ) {
                return false;
            }
            const size_type unit_count = ( length + unit_bit_size - 1 ) >> unit_shift;
            const size_type unit_bit_pos = bitpos & (unit_bit_size-1); // % unit_bit_size;
            if ( 0 == unit_bit_pos ) {
                size_type l = std::min(length, unit_bit_size);
                size_type i = bitpos;
                for ( size_type u = 0; u < unit_count && 0 < length; ++u ) {
                    if( !putUnit( i, l, o.m_storage[u] ) ) {
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

        std::pair<bitheap, bool> subbits(size_type bitpos, size_type length) const noexcept {
            if ( 0 == length ) {
                return { bitheap(0), true };
            } else if ( !in_range(bitpos, length) ) {
                return { bitheap(0), false };
            }
            std::pair<bitheap, bool> r{ bitheap(length), true };
            const size_type unit_count = ( length + unit_bit_size - 1 ) >> unit_shift;
            const size_type unit_pos = bitpos >> unit_shift;
            const size_type unit_bit_pos = bitpos & (unit_bit_size-1); // % unit_bit_size;
            if ( 0 == unit_bit_pos ) {
                size_type l = std::min(length, unit_bit_size);
                size_type i = 0;
                for ( size_type u = unit_pos; u < unit_count && 0 < length; ++u ) {
                    if( !r.first.putUnit( i, l, m_storage[u] ) ) {
                        return { bitheap(0), false };
                    }
                    length -= l;
                    i += l;
                    l = std::min(length, unit_bit_size);
                }
            } else {
                for(size_type i = 0; i < length; ++i) {
                    if( !r.first.put(i, get(bitpos+i)) ) {
                        return { bitheap(0), false };
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
            const size_type bit_pos = bitpos & (unit_bit_size-1); // % unit_bit_size;
            if ( 0 == bit_pos ) {
                // fast path
                const size_type sz0 = (m_unit_size - 1) * unit_bit_size;;
                size_type l = length > sz0 ? length - sz0 : std::min(length, unit_bit_size);
                for ( size_type i = unit_pos + unit_count; i-- > unit_pos && 0 < length; ) {
                    r.append( jau::toBitString(m_storage[i], bit_order_t::msb, PrefixOpt::none, l) );
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
            return toString(0, m_bit_size, prefix);
        }

        std::string infoString() const noexcept {
            return "bitfield[unit[bits "+std::to_string(unit_bit_size)+", count "+std::to_string(m_unit_size)+
                    "], bits"+std::to_string(m_bit_size)+": "+toString()+"]";
        }
    };

    inline std::ostream &operator<<(std::ostream &out, const bitheap &v) {
        return out << v.toString();
    }

    /**@}*/

}  // namespace jau

#endif /*  JAU_BITHEAP_HPP_ */
