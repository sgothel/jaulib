/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2026 Gothel Software e.K.
 *
 * SPDX-License-Identifier: MIT
 *
 * This Source Code Form is subject to the terms of the MIT License
 * If a copy of the MIT was not distributed with this file,
 * you can obtain one at https://opensource.org/license/mit/.
 */

#ifndef JAU_HASH_FNV1A_HPP_
#define JAU_HASH_FNV1A_HPP_

#include <cstddef>
#include <cstdint>
#include <string_view>

#include <jau/cpp_lang_util.hpp>

namespace jau {

    /** \addtogroup ByteUtils
     *
     *  @{
     */

    /**
     * Crude implementation of 64-bit FNV-1a hash algorithm,
     * see http://www.isthe.com/chongo/tech/comp/fnv/,
     * and http://www.isthe.com/chongo/tech/comp/fnv/#FNV-1a
     * and https://www.rfc-editor.org/info/rfc9923/ for details.
     */
    namespace fnv1a_64 {
        constexpr uint64_t FNV_OFFSET = 14695981039346656037UL;
        constexpr uint64_t FNV_PRIME  = 1099511628211UL;

        /**
         * Crude implementation of 64-bit FNV-1a hash algorithm.
         * @tparam T an integral type of 1 byte size (octet)
         * @param d octet data pointer
         * @param len length of data
         * @param h hash from previous round, use FNV_OFFSET for initial round
         */
        template<typename T>
        requires std::is_integral_v<T> && (sizeof(T) == 1)
        constexpr uint64_t hash(const T *d, size_t len, uint64_t h) noexcept {
            const T * const e = d + len;

            while (d < e) {
                h ^= (uint64_t)*d++;
                // h *= FNV_PRIME_64;
                h += (h<<1) + (h<<4) + (h<<5) + (h<<7) + (h<<8) + (h<<40);
            }
            return h;
        }

        /**
         * Crude implementation of 64-bit FNV-1a hash algorithm.
         * @param d string_view data
         * @param h hash from previous round, use FNV_OFFSET for initial round
         */
        CXX_ALWAYS_INLINE
        constexpr uint64_t hash(std::string_view d, uint64_t h) noexcept {
            return hash(d.data(), d.size(), h);
        }

        /**
         * Initial round of crude 64-bit FNV-1a hash algo.
         * @tparam T an integral type of 1 byte size (octet)
         * @param d octet data pointer
         * @param len length of data
         */
        template<typename T>
        requires std::is_integral_v<T> && (sizeof(T) == 1)
        CXX_ALWAYS_INLINE
        constexpr uint64_t hash(const T *d, size_t len) noexcept {
            return hash(d, len, FNV_OFFSET);
        }

        /**
         * Initial round of crude 64-bit FNV-1a hash algo
         * @param d string_view data
         */
        CXX_ALWAYS_INLINE
        constexpr uint64_t hash(std::string_view d) noexcept {
            return hash(d.data(), d.size());
        }

        /**
         * Clipping 64-bit hash64_fnv1a value to max_bit_count.
         * @tparam T an integral type of 1 byte size (octet)
         * @param d octet data pointer
         * @param len length of data
         * @param max_bit_count maximum bit-count to clip
         */
        template<typename T>
        requires std::is_integral_v<T> && (sizeof(T) == 1)
        constexpr uint64_t hash_clipped(const T *d, size_t len, size_t max_bit_count) noexcept {
            if(0==max_bit_count || 0==len) {
                return 0;
            }
            if(64 <= max_bit_count) {
                return hash(d, len, FNV_OFFSET);
            }
            const uint64_t h = hash(d, len, FNV_OFFSET);
            return ((h>>max_bit_count) ^ h) & (((uint64_t)1<<max_bit_count)-1);
        }
    } // namespace fnv1a_64

    /**
     * Crude implementation of 32-bit FNV-1a hash algorithm,
     * see http://www.isthe.com/chongo/tech/comp/fnv/,
     * and http://www.isthe.com/chongo/tech/comp/fnv/#FNV-1a
     * and https://www.rfc-editor.org/info/rfc9923/ for details.
     */
    namespace fnv1a_32 {
        constexpr uint32_t FNV_OFFSET = 2166136261U;
        constexpr uint32_t FNV_PRIME  = 16777619U;

        /**
         * Crude implementation of 32-bit FNV-1a hash algorithm.
         * @tparam T an integral type of 1 byte size (octet)
         * @param d octet octet data pointer
         * @param len length of data
         * @param h hash from previous round, use FNV_OFFSET for initial round
         */
        template<typename T>
        requires std::is_integral_v<T> && (sizeof(T) == 1)
        constexpr uint32_t hash(const T *d, size_t len, uint32_t h) noexcept {
            const T * const e = d + len;

            while (d < e) {
                h ^= (uint32_t)*d++;
                // h *= FNV_PRIME_32;
                h += (h<<1) + (h<<4) + (h<<7) + (h<<8);
            }
            return h;
        }

        /**
         * Crude implementation of 32-bit FNV-1a hash algorithm.
         * @param d string_view data
         * @param h hash from previous round, use FNV_OFFSET for initial round
         */
        CXX_ALWAYS_INLINE
        constexpr uint32_t hash(std::string_view d, uint32_t h) noexcept {
            return hash(d.data(), d.size(), h);
        }

        /**
         * Initial round of crude 32-bit FNV-1a hash algo.
         * @tparam T an integral type of 1 byte size (octet)
         * @param octet d data pointer
         * @param len length of data
         */
        template<typename T>
        requires std::is_integral_v<T> && (sizeof(T) == 1)
        CXX_ALWAYS_INLINE
        constexpr uint32_t hash(const T *d, size_t len) noexcept {
            return hash(d, len, FNV_OFFSET);
        }

        /**
         * Initial round of crude 32-bit FNV-1a hash algo
         * @param d string_view data
         */
        CXX_ALWAYS_INLINE
        constexpr uint32_t hash(std::string_view d) noexcept {
            return hash(d.data(), d.size());
        }

        /**
         * Clipping 32-bit hash64_fnv1a value to max_bit_count.
         * @tparam T an integral type of 1 byte size (octet)
         * @param octet d data pointer
         * @param len length of data
         * @param max_bit_count maximum bit-count to clip
         */
        template<typename T>
        requires std::is_integral_v<T> && (sizeof(T) == 1)
        constexpr uint32_t hash_clipped(const T *d, size_t len, size_t max_bit_count) noexcept {
            if(0==max_bit_count || 0==len) {
                return 0;
            }
            if(32 <= max_bit_count) {
                return hash(d, len, FNV_OFFSET);
            }
            const uint32_t h = hash(d, len, FNV_OFFSET);
            return ((h>>max_bit_count) ^ h) & (((uint32_t)1<<max_bit_count)-1);
        }
    } // namespace fnv1a_32

    /**
     * Crude implementation of sizeof(size_t) FNV-1a hash algorithm,
     * see http://www.isthe.com/chongo/tech/comp/fnv/,
     * and http://www.isthe.com/chongo/tech/comp/fnv/#FNV-1a
     * and https://www.rfc-editor.org/info/rfc9923/ for details.
     */
    namespace fnv1a_size_t {
        /**
         * Crude implementation of size_t-bits FNV-1a hash algorithm.
         * @tparam T an integral type of 1 byte size (octet)
         * @param d octet data pointer
         * @param len length of data
         * @param h hash from previous round, use FNV_OFFSET for initial round
         */
        template<typename T>
        requires std::is_integral_v<T> && (sizeof(T) == 1)
        CXX_ALWAYS_INLINE
        constexpr size_t hash(const T *d, size_t len, size_t h) noexcept
        {
            if constexpr (sizeof(size_t) <= sizeof(uint32_t)) {
                return fnv1a_32::hash(d, len, static_cast<uint32_t>(h));
            } else if constexpr (sizeof(size_t) <= sizeof(uint64_t)) {
                return fnv1a_64::hash(d, len, h);
            } else {
                return static_cast<size_t>( fnv1a_64::hash(d, len, static_cast<uint64_t>(h)) );
            }
        }

        /**
         * Crude implementation of size_t-bits FNV-1a hash algorithm.
         * @param d string_view data
         * @param h hash from previous round, use FNV_OFFSET for initial round
         */
        CXX_ALWAYS_INLINE
        constexpr size_t hash(std::string_view d, size_t h) noexcept {
            return hash(d.data(), d.size(), h);
        }

        /**
         * Initial round of crude size_t-bits FNV-1a hash algo.
         * @tparam T an integral type of 1 byte size (octet)
         * @param d data pointer
         * @param len length of data
         */
        template<typename T>
        requires std::is_integral_v<T> && (sizeof(T) == 1)
        CXX_ALWAYS_INLINE
        constexpr size_t hash(const T *d, size_t len) noexcept {
            if constexpr (sizeof(size_t) <= sizeof(uint32_t)) {
                return fnv1a_32::hash(d, len, fnv1a_32::FNV_OFFSET);
            } else {
                return fnv1a_64::hash(d, len, fnv1a_64::FNV_OFFSET);
            }
        }

        /**
         * Initial round of crude size_t-bits FNV-1a hash algo
         * @param d string_view data
         */
        CXX_ALWAYS_INLINE
        constexpr size_t hash(std::string_view d) noexcept {
            return hash(d.data(), d.size());
        }

        /**
         * Clipping size_t-bits hash64_fnv1a value to max_bit_count.
         * @tparam T an integral type of 1 byte size (octet)
         * @param d data pointer
         * @param len length of data
         * @param max_bit_count maximum bit-count to clip
         */
        template<typename T>
        requires std::is_integral_v<T> && (sizeof(T) == 1)
        CXX_ALWAYS_INLINE
        constexpr size_t hash_clipped(const T *d, size_t len, size_t max_bit_count) noexcept {
            if constexpr (sizeof(size_t) <= sizeof(uint32_t)) {
                return fnv1a_32::hash_clipped(d, len, max_bit_count);
            } else {
                return fnv1a_64::hash_clipped(d, len, max_bit_count);
            }
        }
    }

    /**@}*/

}  // namespace jau

#endif /* JAU_HASH_FNV1A_HPP_ */
