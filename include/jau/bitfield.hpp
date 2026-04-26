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
#include <string_view>

#include <jau/basic_types.hpp>
#include <jau/bitview.hpp>
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
     * Simple statically sized bitfield template for efficient bit storage access via jau::bitview.
     *
     * Bit-position and bit-order are in least-significant-bits (lsb) first.
     *
     * Implementations utilizes an in-memory `std::array<StorageType, (BitSize+StorageTypeBits-1)/StorageTypeBits>`
     * with unsigned integral StorageType of sizeof(StorageType) <= sizeof(size_t).
     *
     * Similar to std::bitset, but providing custom methods.
     *
     * @see jau::bitview
     * @see jau::bitheap
     */
    template<jau::req::unsigned_integral StorageType, size_t BitSize>
        requires requires (StorageType) { sizeof(StorageType) <= sizeof(size_t); }
    class bitfield_t : public jau::bitview<StorageType> {
      public:
        typedef StorageType unit_type;                                                ///< Unit data type
        typedef jau::bitview<unit_type> bitview_t;                                    ///< Storage bitview
        using typename bitview_t::size_type;
        static constexpr size_type bit_size = BitSize;                                ///< Storage size in bits

        /// Storage size in units
        static constexpr size_type unit_size = std::max<size_type>(1, (bit_size + bitview_t::unit_bit_size - 1) >> bitview_t::unit_shift);

      private:
        typedef std::array<unit_type, unit_size> storage_t;
        storage_t m_storage;

        constexpr void clearInt() noexcept {
            std::memset(m_storage.data(), 0, bitview_t::unit_byte_size * bitview_t::unitSize());
        }
        using bitview_t::reset;

      public:
        /** Constructs an empty bitfield instance */
        constexpr bitfield_t() noexcept
        : bitview_t(m_storage, bit_size)
        { clearInt(); }

        /**
         * Constructs a bitfield instance, initialized with `bitstr` msb bit-pattern.
         * @param bitstr most-significat (msb) bit string pattern
         * @throws jau::IllegalArgumentError if bitstr put failed
         * @see put(std::string_view)
         */
        bitfield_t(std::string_view bitstr)
        : bitview_t(m_storage, bit_size)
        {
            clearInt();
            if( !bitview_t::put(0, bitstr) ) {
                throw jau::IllegalArgumentError("Invalid bit-patter "+std::string(bitstr), E_FILE_LINE);
            }
        }

        /// Returns the bitview representing this bit storage
        bitview_t& view() noexcept { return *this; }
        /// Returns the bitview representing this bit storage
        const bitview_t& view() const noexcept { return *this; }

        template<size_t BitLength>
        std::pair<bitfield_t<StorageType, BitLength>, bool> subbits(size_type bitpos) const noexcept {
            if ( 0 == BitLength ) {
                return { bitfield_t<StorageType, BitLength>(), true };
            } else if ( !in_range(bitpos, BitLength) ) {
                return { bitfield_t<StorageType, BitLength>(), false };
            }
            std::pair<bitfield_t<StorageType, BitLength>, bool> r{ bitfield_t<StorageType, BitLength>(), true };
            r.first.put(0, *this, bitpos, BitLength);
            return r;
        }

        std::string infoString() const noexcept {
            return bitview_t::infoString("bitfield");
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
