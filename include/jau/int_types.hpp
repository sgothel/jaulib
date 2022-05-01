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

#include <jau/cpp_lang_util.hpp>
#include <jau/packed_attribute.hpp>

namespace jau {
    /** @defgroup Integrals Integral type support
     * Integral integer data types and arithmetic
     *  @{
     */

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
        constexpr uint128_t(const uint8_t v[16]) noexcept : data{0} { *this = pointer_cast<packed_t<uint128_t>*>( v )->store; }
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
        constexpr uint192_t(const uint8_t v[24]) noexcept : data{0} { *this = pointer_cast<packed_t<uint192_t>*>( v )->store; }
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
        constexpr uint256_t(const uint8_t v[32]) noexcept : data{0} { *this = pointer_cast<packed_t<uint256_t>*>( v )->store; }
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

    namespace int_literals {
        /** Literal for signed int8_t */
        constexpr int8_t operator ""_i8(unsigned long long int __v)   { return (int8_t)__v; }

        /** Literal for unsigned uint8_t */
        constexpr uint8_t operator ""_u8(unsigned long long int __v)  { return (uint8_t)__v; }

        /** Literal for signed int16_t */
        constexpr int16_t operator ""_i16(unsigned long long int __v)   { return (int16_t)__v; }

        /** Literal for unsigned uint16_t */
        constexpr uint16_t operator ""_u16(unsigned long long int __v)  { return (uint16_t)__v; }

        /** Literal for signed int32_t */
        constexpr int32_t operator ""_i32(unsigned long long int __v)   { return (int32_t)__v; }

        /** Literal for unsigned uint32_t */
        constexpr uint32_t operator ""_u32(unsigned long long int __v)  { return (uint32_t)__v; }

        /** Literal for signed int64_t */
        constexpr int64_t operator ""_i64(unsigned long long int __v)   { return (int64_t)__v; }

        /** Literal for unsigned uint64_t */
        constexpr uint64_t operator ""_u64(unsigned long long int __v)  { return (uint64_t)__v; }

        /** Literal for signed ssize_t */
        constexpr ssize_t operator ""_iz(unsigned long long int __v)  { return (ssize_t)__v; }

        /** Literal for unsigned size_t */
        constexpr size_t operator ""_uz(unsigned long long int __v)  { return (size_t)__v; }

        /** Literal for signed jau::snsize_t */
        constexpr jau::snsize_t operator ""_inz(unsigned long long int __v)  { return (jau::snsize_t)__v; }

        /** Literal for unsigned jau::nsize_t */
        constexpr jau::nsize_t operator ""_unz(unsigned long long int __v)  { return (jau::nsize_t)__v; }
    }

    /**@}*/

} // namespace jau

/** \example test_basictypeconv.cpp
 * This C++ unit test validates the jau::bswap and get/set value implementation
 */

#endif /* JAU_INT_TYPES_HPP_ */
