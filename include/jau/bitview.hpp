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
#ifndef JAU_BITVIEW_HPP_
#define JAU_BITVIEW_HPP_

#include <unistd.h>

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
     * Simple bitview for efficient bit access to a storage-proxy.
     *
     * Bit-position and bit-order are in least-significant-bits (lsb) first.
     *
     * Implementations utilizes a memory proxy `std::span<StorageType>`
     * with unsigned integral StorageType of sizeof(StorageType) <= sizeof(size_t).
     *
     * Similar to std::bitset, but using a storage proxy std::span and providing custom methods.
     *
     * @see jau::bitheap
     * @see jau::bitaccess_t
     */
    template<jau::req::unsigned_integral StorageType>
        requires requires (StorageType) { sizeof(StorageType) <= sizeof(size_t); }
    class bitview {
      public:
        typedef StorageType unit_type;                                                ///< Unit data type
        typedef size_t   size_type;                                                   ///< size_t data type, bit position and count
        typedef std::span<unit_type> storage_t;                                       ///< storage view
        static constexpr size_type unit_byte_size = sizeof(unit_type);                ///< One unit size in bytes
        static constexpr size_type unit_bit_size = unit_byte_size << 3;               ///< One unit size in bits
        static constexpr size_type unit_shift = jau::log2_byteshift(unit_byte_size);  ///< One unit shift amount
        static constexpr size_type unitSize(size_type bitSize) noexcept { return (bitSize + unit_bit_size - 1) >> unit_shift; }
        static constexpr size_type bitSize(storage_t s, size_type bitOffset, size_type max) noexcept { return std::min(max, (s.size_bytes() << 3) - bitOffset); }

      private:
        static constexpr unit_type one_u = 1;

        size_type m_bit_size;     ///< Storage size in bits
        size_type m_bit_offset;   ///< bitpos offset in storage in bits
        storage_t m_storage;

        template<typename Arg, typename Func0, typename Func1>
        void forall(size_t bitpos, size_t length, Arg arg, Func0 func0, Func1 func1) const noexcept {
            bitpos += m_bit_offset;
            size_type remaining = length;
            size_type u;
            if( bitpos ) {
                u = bitpos >> unit_shift;
                assert(u < unitSize());
                const size_type b_lo = bitpos & (unit_bit_size-1); // % unit_bit_size;
                if (b_lo) {
                    const size_type b_hi = std::min(b_lo+remaining, unit_bit_size);
                    assert(b_hi >= b_lo);
                    // const unit_type s = m_storage[u];
                    const unit_type m = bit_mask<unit_type>(b_hi) & ~bit_mask<unit_type>(b_lo);
                    func0(m_storage[u++], m, arg);
                    remaining -= b_hi - b_lo;
                    // jau_printf("XXX.1: u %zu, b_lo %zu, b_hi %zu, s %zb, m %zb, rem %zu\n", u-1, b_lo, b_hi, s, m, remaining);
                }
            } else {
                u = 0;
            }
            while (remaining >= unit_bit_size) {
                assert(u < unitSize());
                func1(m_storage[u++], arg);
                remaining -= unit_bit_size;
                // jau_printf("XXX.2: u %zu, rem %zu\n", u-1, remaining);
            }
            if (remaining) {
                assert(u < unitSize());
                // const unit_type s = m_storage[u];
                const unit_type m = bit_mask<unit_type>(remaining);
                func0(m_storage[u], m, arg);
                // jau_printf("XXX.3: u %zu, s %zb, m %zb\n", u, s, m);
            }
            assert(u <= unitSize());
        }

      protected:
        std::string infoString(const std::string &tag) const noexcept {
            return tag+"[unit[bits "+std::to_string(unit_bit_size)+", count "+std::to_string(unitSize())+
                    "], bits "+std::to_string(m_bit_size)+", offset "+std::to_string(m_bit_offset)+": "+toString()+"]";
        }

        /** Reset storage and maximum `maxBits` bits. Useful if underlying storage has been invalidated. */
        constexpr void reset(storage_t s, size_type maxBits=std::numeric_limits<size_type>::max()) noexcept {
            reset(0, s, maxBits);
        }
        /** Reset storage, its offset and maximum `maxBits` bits. Useful if underlying storage has been invalidated. */
        constexpr void reset(size_type bitOffset, storage_t s, size_type maxBits=std::numeric_limits<size_type>::max()) noexcept {
            m_bit_size = bitSize(s, bitOffset, maxBits);
            m_bit_offset = bitOffset;
            m_storage = s;
        }

      public:
        constexpr bool in_range(size_type bitpos) const noexcept { return bitpos < m_bit_size; }
        constexpr bool in_range(size_type bitpos, size_type length) const noexcept { return bitpos + length <= m_bit_size; }

        /** Constructs an empty bitview instance of zero bits */
        constexpr bitview() noexcept
        : m_bit_size(0), m_bit_offset(0), m_storage()
        { }

        /** Constructs a bitview instance of given storage `s` with maximum `maxBits` bits */
        constexpr bitview(storage_t s, size_type maxBits=std::numeric_limits<size_type>::max()) noexcept
        : m_bit_size(bitSize(s, 0, maxBits)), m_bit_offset(0), m_storage(s)
        { }

        /** Constructs a bitview instance of given storage `s` at bit-offset `bitOffset` with maximum `maxBits` bits */
        constexpr bitview(size_type bitOffset, storage_t s, size_type maxBits=std::numeric_limits<size_type>::max()) noexcept
        : m_bit_size(bitSize(s, bitOffset, maxBits)), m_bit_offset(bitOffset), m_storage(s)
        { }

        /**
         * Constructs a bitview instance, initialized with `bitstr` msb bit-pattern.
         * @param bitstr most-significat (msb) bit string pattern
         * @throws jau::IllegalArgumentError if bitstr put failed
         * @see put(std::string_view)
         */
        bitview(storage_t s, std::string_view bitstr)
        : m_bit_size(bitSize(s, 0, bitstr.size())), m_bit_offset(0),
          m_storage(s)
        {
            clear();
            if( !put(0, bitstr) ) {
                throw jau::IllegalArgumentError("Invalid bit-patter "+std::string(bitstr), E_FILE_LINE);
            }
        }

        /** Returns storage size in bits */
        constexpr size_type size() const noexcept { return m_bit_size; }

        /** Returns storage offset in bits */
        constexpr size_type offset() const noexcept { return m_bit_offset; }

        /** Returns storage size in storage-units */
        constexpr size_type unitSize() const noexcept { return m_storage.size(); }

        /// Returns true if whole storage is occupied by this bitview
        constexpr bool wholeStorage() const noexcept { return !m_bit_offset && 0 == (m_bit_size & (unit_bit_size-1)); } // % unit_bit_size

        /*** Returns the number of set bits. */
        size_type count() const noexcept {
            size_type counter = 0;
            forall<size_type&>(0, m_bit_size, counter,
                [](const unit_type unit, const unit_type mask, size_type& c) noexcept { c += jau::bit_count<unit_type>(unit & mask); },
                [](const unit_type unit, size_type& c) noexcept { c += jau::bit_count<unit_type>(unit); }
            );
            return counter;
        }

        /*** Clears all bits, i.e. sets all bits to zero. */
        constexpr bitview &clear() noexcept {
            if (wholeStorage()) {
                std::memset(m_storage.data(), 0, unit_byte_size * unitSize());
            } else {
                set(0, m_bit_size, false);
            }
            return *this;
        }

        /*** Reads the bit value at position `bitpos`. */
        constexpr bool operator[](size_type bitpos) const noexcept { return get(bitpos); }

        /*** Reads the bit value at position `bitpos`. */
        constexpr bool get(size_type bitpos) const noexcept {
            if ( !in_range(bitpos) ) {
                return false;
            }
            bitpos += m_bit_offset;
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
            bitpos += m_bit_offset;
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
            bitpos += m_bit_offset;
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
        constexpr bitview &flip() noexcept {
            forall<int>(0, m_bit_size, 0,
                [](unit_type &unit, const unit_type mask, int) noexcept { unit = (~unit & mask) | (unit & ~mask); },
                [](unit_type &unit, int) noexcept { unit = ~unit; }
            );
            return *this;
        }

        /*** Reverse all bits in this storage. */
        constexpr bitview &reverse() noexcept {
            if (m_bit_size) {
                if (wholeStorage()) { // fast-path, swap units
                    size_type l = 0, r = unitSize()-1;
                    while ( l < r ) {
                        const unit_type v_l = jau::rev_bits(m_storage[l]);
                        const unit_type v_r = jau::rev_bits(m_storage[r]);
                        m_storage[l++] = v_r;
                        m_storage[r--] = v_l;
                    }
                    if( l == r ) { // last odd middle
                        m_storage[l] = jau::rev_bits(m_storage[l]);
                    }
                } else { // slow-path, swap bits
                    // FIXME PERF
                    for(size_type l = 0, r = m_bit_size-1; l < r; ++l, --r) {
                        const bool s = get(l);
                        (void)put(l, get(r));
                        (void)put(r, s);
                    }
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
            bitpos += m_bit_offset;
            const size_type u = bitpos >> unit_shift;
            const size_type b = bitpos & (unit_bit_size-1); // % unit_bit_size;
            if ( 0 == b ) {
                // fast-path
                const unit_type m = bit_mask<unit_type>(length);  // mask of chunk
                return m & m_storage[u];
            } else {
                // slow-path
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
            bitpos += m_bit_offset;
            const size_type u = bitpos >> unit_shift;
            const size_type b = bitpos & (unit_bit_size-1); // % unit_bit_size;
            if ( 0 == b ) {
                // fast-path
                const unit_type m = bit_mask<unit_type>(length);  // mask of chunk
                m_storage[u] = ((~m) & m_storage[u])              // keep non-written storage bits
                             | (m & data);                        // overwrite storage w/ used data bits
            } else {
                // slow-path
                const size_type left = unit_bit_size - b;         // remaining bits of first chunk storage
                const size_type l1 = std::min(length, left);      // length of first chunk < unit_bit_size
                const unit_type m1 = (one_u << l1) - one_u;       // mask of first chunk
                m_storage[u] = ((~(m1 << b)) & m_storage[u])      // keep non-written storage bits
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
                    m_storage[u + 1] = ((~m2) & m_storage[u + 1]) // keep non-written storage bits
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
         * Set `length` bits starting at `bitpos` of this bitview to the given value `bit`.
         * @returns true on sucess, otherwise false
         */
        bool set(size_type bitpos, size_type length, bool bit) noexcept {
            if ( 0 == length ) {
                return true;
            } else if ( !in_range(bitpos, length) ) {
                return false;
            }
            const unit_type value = bit ? std::numeric_limits<unit_type>::max() : 0;
            forall<const unit_type>(bitpos, length, value,
                [](unit_type &unit, const unit_type mask, const unit_type v) noexcept { unit = (v & mask) | (unit & ~mask); },
                [](unit_type &unit, const unit_type v) noexcept { unit = v; }
            );
            return true;
        }
        /** Set all bits to the given value `bit`. */
        bitview &setAll(bool bit) noexcept {
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

        constexpr bool operator==(const bitview &rhs) const noexcept {
            if ( this == &rhs ) {
                return true;
            }
            if ( m_bit_size != rhs.m_bit_size ) {
                return false;
            }
            size_type remaining = m_bit_size;
            if (m_bit_offset == rhs.offset()) {
                size_type u;
                if (m_bit_offset) {
                    u = m_bit_offset >> unit_shift;
                    assert(u < unitSize());
                    const size_type b_lo = m_bit_offset & (unit_bit_size-1); // % unit_bit_size;
                    if (b_lo) {
                        const size_type b_hi = std::min(b_lo+remaining, unit_bit_size);
                        assert(b_hi >= b_lo);
                        const unit_type m = bit_mask<unit_type>(b_hi) & ~bit_mask<unit_type>(b_lo);
                        if ((m_storage[u] & m) != (rhs.m_storage[u] & m)) {
                            return false;
                        }
                        remaining -= b_hi - b_lo;
                        ++u;
                    }
                } else {
                    u = 0;
                }
                while (remaining >= unit_bit_size) {
                    assert(u < unitSize());
                    if (m_storage[u] != rhs.m_storage[u]) {
                        return false;
                    }
                    remaining -= unit_bit_size;
                    ++u;
                }
                if (remaining) {
                    assert(u < unitSize());
                    const unit_type m = bit_mask<unit_type>(remaining);
                    return (m_storage[u] & m) == (rhs.m_storage[u] & m);
                }
                return true;
            }
            // slow-path
            size_type i=0;
            while (remaining > unit_bit_size) {
                if (getUnit(i, unit_bit_size) != rhs.getUnit(i, unit_bit_size)) {
                    return false;
                }
                i += unit_bit_size;
                remaining -= unit_bit_size;
            }
            while ( remaining && get(i) == rhs.get(i) ) {
                ++i;
                --remaining;
            }
            return !remaining;
        }

        bool put(size_type dstbitpos, const bitview& src) {
            return put(dstbitpos, src, 0, src.size());
        }
        bool put(size_type dstbitpos, const bitview& src, size_type srcbitpos, size_type len) {
            if ( 0 == len ) {
                return true;
            } else if ( !in_range(dstbitpos, len) || !src.in_range(srcbitpos, len)) {
                return false;
            }
            // slow-path
            while (len > unit_bit_size) {
                if (!putUnit(dstbitpos, unit_bit_size, src.getUnit(srcbitpos, unit_bit_size))) {
                    return false;
                }
                dstbitpos += unit_bit_size;
                srcbitpos += unit_bit_size;
                len -= unit_bit_size;
            }
            while (len--) {
                if( !put(dstbitpos++, src.get(srcbitpos++)) ) {
                    return false;
                }
            }
            return true;
        }

        std::pair<bitview, bool> subview(size_type bitpos, size_type length) const noexcept {
            if ( 0 == length ) {
                return { bitview(), true };
            } else if ( !in_range(bitpos, length) ) {
                return { bitview(), false };
            }
            const size_type bitpos2 = bitpos+m_bit_offset;
            const size_type u = bitpos2 >> unit_shift;
            const size_type o = bitpos2 & (unit_bit_size-1); // % unit_bit_size;
            std::pair<bitview, bool> r{ bitview(o, storage_t(m_storage.begin()+u, unitSize(length+o)), length), true };
            return r;
        }

        std::string toString(size_type bitpos, size_type length, PrefixOpt prefix = PrefixOpt::none) const noexcept {
            if ( 0 == length ) {
                return "nil";
            }
            if ( !in_range(bitpos, length) ) {
                return "ERANGE";
            }
            std::string r;
            r.reserve(length + (prefix == PrefixOpt::none ? 0 : 2));
            if ( prefix == PrefixOpt::prefix ) {
                r.append("0b");
            }
            if (!m_bit_offset) {
                const size_type unit_pos = bitpos >> unit_shift;
                const size_type bit_pos = bitpos & (unit_bit_size-1); // % unit_bit_size;
                if (!bit_pos) {
                    // fast-path
                    const size_type unit_count = ( length + unit_bit_size - 1 ) >> unit_shift;
                    const size_type sz0 = (unitSize() - 1) * unit_bit_size;;
                    size_type l = length > sz0 ? length - sz0 : std::min(length, unit_bit_size);
                    for ( size_type i = unit_pos + unit_count; i-- > unit_pos && 0 < length; ) {
                        r.append( jau::toBitString(m_storage[i], bit_order_t::msb, PrefixOpt::none, l) );
                        length -= l;
                        l = std::min(length, unit_bit_size);
                    }
                    return r;
                }
            }
            size_type i = bitpos + length;
            while(length-- > 0) {
                r.push_back(get(--i) ? '1' : '0');
            }
            return r;
        }
        std::string toString(PrefixOpt prefix = PrefixOpt::none) const noexcept {
            return toString(0, m_bit_size, prefix);
        }

        std::string infoString() const noexcept {
            return infoString("bitview");
        }
    };

    template<jau::req::unsigned_integral StorageType>
        requires requires (StorageType) { sizeof(StorageType) <= sizeof(size_t); }
    inline std::ostream &operator<<(std::ostream &out, const bitview<StorageType> &v) {
        return out << v.toString();
    }

    /**@}*/

}  // namespace jau

#endif /*  JAU_BITVIEW_HPP_ */
