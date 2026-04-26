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
     * Simple dynamically heap-sized bitheap for efficient bit storage access via jau::bitview.
     *
     * Bit-position and bit-order are in least-significant-bits (lsb) first.
     *
     * Implementations utilizes a dynamic heap `std::vector<jau::nsize_t>` StorageType.
     *
     * Similar to std::bitset, but utilizing dynamic runtime heapsize and  providing custom methods.
     *
     * @see jau::bitview
     * @see jau::bitfield
     * @see jau::nsize_t
     */
    class bitheap : public jau::bitview<jau::nsize_t> {
      public:
        typedef jau::nsize_t unit_type;                                               ///< Unit data type
        typedef jau::bitview<unit_type> bitview_t;                                    ///< Storage bitview
        using typename bitview_t::size_type;

        static constexpr size_type unitSize(size_type bitSize) noexcept { return (bitSize + bitview_t::unit_bit_size - 1) >> bitview_t::unit_shift; }

      private:
        typedef std::vector<unit_type> storage_t;
        storage_t m_storage;

        constexpr void clearInt() noexcept {
            std::memset(m_storage.data(), 0, bitview_t::unit_byte_size * bitview_t::unitSize());
        }
        using bitview_t::reset;

      public:
        /** Constructs an empty bitheap instance of zero bits */
        constexpr bitheap() noexcept
        : bitview_t(), m_storage(0, 0)
        { }

        /** Constructs a bitheap instance of bitSize bits */
        constexpr bitheap(size_type bitSize) noexcept
        : bitview_t(), m_storage(unitSize(bitSize), 0)
        {
          bitview_t::reset(m_storage, bitSize);
          clearInt();
        }

        /**
         * Constructs a bitheap instance, initialized with `bitstr` msb bit-pattern.
         * @param bitstr most-significat (msb) bit string pattern
         * @throws jau::IllegalArgumentError if bitstr put failed
         * @see put(std::string_view)
         */
        bitheap(std::string_view bitstr)
        : bitview_t(), m_storage(unitSize(bitstr.size()), 0)
        {
            bitview_t::reset(m_storage, bitstr.size());
            clearInt();
            if( !bitview_t::put(0, bitstr) ) {
                throw jau::IllegalArgumentError("Invalid bit-patter "+std::string(bitstr), E_FILE_LINE);
            }
        }

        /// Returns the bitview representing this bit storage
        bitview_t& view() noexcept { return *this; }
        /// Returns the bitview representing this bit storage
        const bitview_t& view() const noexcept { return *this; }

        void resize(size_t new_bit_size) {
            if (size() != new_bit_size) {
                m_storage.resize(unitSize(new_bit_size), 0);
                bitview_t::reset(m_storage, new_bit_size);
            }
        }

        std::pair<bitheap, bool> subbits(size_type bitpos, size_type length) const noexcept {
            if ( 0 == length ) {
                return { bitheap(0), true };
            } else if ( !in_range(bitpos, length) ) {
                return { bitheap(0), false };
            }
            std::pair<bitheap, bool> r{ bitheap(length), true };
            r.first.put(0, *this, bitpos, length);
            return r;
        }

        std::string infoString() const noexcept {
            return bitview_t::infoString("bitheap");
        }
    };

    inline std::ostream &operator<<(std::ostream &out, const bitheap &v) {
        return out << v.toString();
    }

    /**@}*/

}  // namespace jau

#endif /*  JAU_BITHEAP_HPP_ */
