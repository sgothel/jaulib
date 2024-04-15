/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021-2024 Gothel Software e.K.
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
    /** @defgroup Integer Integer types and arithmetic
     * Integral integer types and arithmetic.
     *
     * Further support is coming from \ref ByteUtils and meta-group \ref Math
     *
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

    // Remember: constexpr specifier used in a function or static data member (since C++17) declaration implies inline.

    template <int bytesize> struct uint_bytes;
    template <> struct uint_bytes<4>{ using type = uint32_t; };
    template <> struct uint_bytes<8>{ using type = uint64_t; };
    #if defined(__SIZEOF_INT128__)
        template <> struct uint_bytes<16>{ using type = uint128_t; };
    #endif

    template <int bytesize> struct sint_bytes;
    template <> struct sint_bytes<4>{ using type = int32_t; };
    template <> struct sint_bytes<8>{ using type = int64_t; };
    #if defined(__SIZEOF_INT128__)
        template <> struct sint_bytes<16>{ using type = int128_t; };
    #endif

    template <int bytesize> struct float_bytes;
    template <> struct float_bytes<sizeof(float)>{ using type = float; };
    template <> struct float_bytes<sizeof(double)>{ using type = double; };
    template <> struct float_bytes<sizeof(long double)>{ using type = long double; };

    /**
    // *************************************************
    // *************************************************
    // *************************************************
     */

    __pack( /** A 128-bit packed uint8_t data array */ struct uint128dp_t {
        uint8_t data[16];

        constexpr uint128dp_t() noexcept : data{0} {}
        constexpr uint128dp_t(const uint8_t v[16]) noexcept : data{0} { *this = pointer_cast<packed_t<uint128dp_t>*>( v )->store; }
        constexpr uint128dp_t(const uint128dp_t &o) noexcept = default;
        uint128dp_t(uint128dp_t &&o) noexcept = default;
        constexpr uint128dp_t& operator=(const uint128dp_t &o) noexcept = default;
        uint128dp_t& operator=(uint128dp_t &&o) noexcept = default;

        void clear() noexcept { ::bzero(data, sizeof(data)); }

        constexpr bool operator==(uint128dp_t const &o) const noexcept {
            if( this == &o ) {
                return true;
            }
            return !std::memcmp(data, o.data, sizeof(data));
        }
        constexpr bool operator!=(uint128dp_t const &o) const noexcept
        { return !(*this == o); }
    } ) ;

    __pack( /** A 196-bit packed uint8_t data array */ struct uint192dp_t {
        uint8_t data[24];

        constexpr uint192dp_t() noexcept : data{0} {}
        constexpr uint192dp_t(const uint8_t v[24]) noexcept : data{0} { *this = pointer_cast<packed_t<uint192dp_t>*>( v )->store; }
        constexpr uint192dp_t(const uint192dp_t &o) noexcept = default;
        uint192dp_t(uint192dp_t &&o) noexcept = default;
        constexpr uint192dp_t& operator=(const uint192dp_t &o) noexcept = default;
        uint192dp_t& operator=(uint192dp_t &&o) noexcept = default;

        void clear() noexcept { ::bzero(data, sizeof(data)); }

        constexpr bool operator==(uint192dp_t const &o) const noexcept {
            if( this == &o ) {
                return true;
            }
            return !std::memcmp(data, o.data, sizeof(data));
        }
        constexpr bool operator!=(uint192dp_t const &o) const noexcept
        { return !(*this == o); }
    } );

    __pack( /** A 256-bit packed uint8_t data array */ struct uint256dp_t {
        uint8_t data[32];

        constexpr uint256dp_t() noexcept : data{0} {}
        constexpr uint256dp_t(const uint8_t v[32]) noexcept : data{0} { *this = pointer_cast<packed_t<uint256dp_t>*>( v )->store; }
        constexpr uint256dp_t(const uint256dp_t &o) noexcept = default;
        uint256dp_t(uint256dp_t &&o) noexcept = default;
        constexpr uint256dp_t& operator=(const uint256dp_t &o) noexcept = default;
        uint256dp_t& operator=(uint256dp_t &&o) noexcept = default;

        void clear() noexcept { ::bzero(data, sizeof(data)); }

        constexpr bool operator==(uint256dp_t const &o) const noexcept {
            if( this == &o ) {
                return true;
            }
            return !std::memcmp(data, o.data, sizeof(data));
        }
        constexpr bool operator!=(uint256dp_t const &o) const noexcept
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
