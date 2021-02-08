/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021 Gothel Software e.K.
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

#ifndef JAU_INT_TYPES_HPP_
#define JAU_INT_TYPES_HPP_

#include <cstdint>
#include <cstring>

#include <jau/packed_attribute.hpp>

namespace jau {
    /**
     * Natural 'size_t' alternative using `uint_fast32_t` as its natural sized type.
     * <p>
     * The leading 'n' stands for natural.
     * </p>
     * <p>
     * This is a compromise to indicate intend,
     * but to avoid handling a multiple sized `size_t` footprint where not desired.
     * </p>
     */
    typedef uint_fast32_t nsize_t;

    /**
     * Natural 'ssize_t' alternative using `int_fast32_t` as its natural sized type.
     * <p>
     * The leading 'n' stands for natural.
     * </p>
     * <p>
     * This is a compromise to indicate intend,
     * but to avoid handling a multiple sized `ssize_t` footprint where not desired.
     * </p>
     */
    typedef int_fast32_t snsize_t;

    /**
    // *************************************************
    // *************************************************
    // *************************************************
     */

    __pack( struct uint128_t {
        uint8_t data[16];

        constexpr uint128_t() noexcept : data{0} {}
        constexpr uint128_t(const uint128_t &o) noexcept = default;
        uint128_t(uint128_t &&o) noexcept = default;
        constexpr uint128_t& operator=(const uint128_t &o) noexcept = default;
        uint128_t& operator=(uint128_t &&o) noexcept = default;

        void clear() noexcept { bzero(data, sizeof(data)); }

        constexpr bool operator==(uint128_t const &o) const noexcept {
            if( this == &o ) {
                return true;
            }
            return !std::memcmp(data, o.data, sizeof(data));
        }
        constexpr bool operator!=(uint128_t const &o) const noexcept
        { return !(*this == o); }
    } ) ;

    __pack( struct uint192_t {
        uint8_t data[24];

        constexpr uint192_t() noexcept : data{0} {}
        constexpr uint192_t(const uint192_t &o) noexcept = default;
        uint192_t(uint192_t &&o) noexcept = default;
        constexpr uint192_t& operator=(const uint192_t &o) noexcept = default;
        uint192_t& operator=(uint192_t &&o) noexcept = default;

        void clear() noexcept { bzero(data, sizeof(data)); }

        constexpr bool operator==(uint192_t const &o) const noexcept {
            if( this == &o ) {
                return true;
            }
            return !std::memcmp(data, o.data, sizeof(data));
        }
        constexpr bool operator!=(uint192_t const &o) const noexcept
        { return !(*this == o); }
    } );

    __pack( struct uint256_t {
        uint8_t data[32];

        constexpr uint256_t() noexcept : data{0} {}
        constexpr uint256_t(const uint256_t &o) noexcept = default;
        uint256_t(uint256_t &&o) noexcept = default;
        constexpr uint256_t& operator=(const uint256_t &o) noexcept = default;
        uint256_t& operator=(uint256_t &&o) noexcept = default;

        void clear() noexcept { bzero(data, sizeof(data)); }

        constexpr bool operator==(uint256_t const &o) const noexcept {
            if( this == &o ) {
                return true;
            }
            return !std::memcmp(data, o.data, sizeof(data));
        }
        constexpr bool operator!=(uint256_t const &o) const noexcept
        { return !(*this == o); }
    } );

} // namespace jau

/** \example test_basictypeconv.cpp
 * This C++ unit test validates the jau::bswap and get/set value implementation
 */

#endif /* JAU_INT_TYPES_HPP_ */
