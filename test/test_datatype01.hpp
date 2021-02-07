/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020 Gothel Software e.K.
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
#ifndef TEST_DATATYPE01_HPP_
#define TEST_DATATYPE01_HPP_

#include <iostream>
#include <cassert>
#include <cinttypes>
#include <cstring>
#include <random>

#include <iostream>

#include <jau/cpp_lang_macros.hpp>
#include <jau/packed_attribute.hpp>
#include <jau/ordered_atomic.hpp>
#include <jau/basic_types.hpp>

using namespace jau;

__pack ( struct Addr48Bit {
    uint8_t b[6]; // == sizeof(Addr48Bit)

    constexpr Addr48Bit() noexcept : b{0}  { }
    constexpr Addr48Bit(const uint64_t encoded) noexcept
    : b{static_cast<uint8_t>(encoded & 0xff),
        static_cast<uint8_t>( ( encoded >>  8 ) & 0xff),
        static_cast<uint8_t>( ( encoded >> 16 ) & 0xff),
        static_cast<uint8_t>( ( encoded >> 24 ) & 0xff),
        static_cast<uint8_t>( ( encoded >> 32 ) & 0xff),
        static_cast<uint8_t>( ( encoded >> 40 ) & 0xff)}  { }

    Addr48Bit(const uint8_t * b_) noexcept {
        memcpy(b, b_, sizeof(b));
    }
    // Trivially-copyable
    constexpr Addr48Bit(const Addr48Bit &o) noexcept = default;
    Addr48Bit(Addr48Bit &&o) noexcept = default;
    constexpr Addr48Bit& operator=(const Addr48Bit &o) noexcept = default;
    Addr48Bit& operator=(Addr48Bit &&o) noexcept = default;

    bool next() noexcept {
        for(int i=0; i<6; ++i) {
            if(b[i] < 0xff ) {
                ++b[i];
                for(int j=i-1; j>=0; --j) {
                    b[j]=0;
                }
                return true;
            }
        }
        return false;
    }
    void random(std::default_random_engine& e) {
        std::uniform_int_distribution<uint8_t> d(0, 255);
        for(int i=0; i<6; ++i) {
            b[i] = static_cast<uint8_t>( d(e) );
        }
    }

    constexpr std::size_t hash_code() const noexcept {
        // 31 * x == (x << 5) - x
        std::size_t h = b[0];
        h = ( ( h << 5 ) - h ) + b[1];
        h = ( ( h << 5 ) - h ) + b[2];
        h = ( ( h << 5 ) - h ) + b[3];
        h = ( ( h << 5 ) - h ) + b[4];
        h = ( ( h << 5 ) - h ) + b[5];
        // printf("hash.Addr48Bit %zu\n", h);
        return h;
    }

    constexpr_func_cxx20 std::string toString() const noexcept {
        std::string str;
        str.reserve(17);

        for(int i=6-1; 0 <= i; --i) {
            jau::byteHexString(str, b[i], false /* lowerCase */);
            if( 0 < i ) {
                str.push_back(':');
            }
        }
        return str;
    }

#if 0
    constexpr_cxx20 operator std::string() const noexcept {
        return toString();
    }
#endif

} );

JAU_TYPENAME_CUE_ALL(Addr48Bit)

std::ostream & operator << (std::ostream &out, const Addr48Bit &a) {
    out << a.toString();
    return out;
}

inline bool operator==(const Addr48Bit& lhs, const Addr48Bit& rhs) noexcept {
    if( &lhs == &rhs ) {
        return true;
    }
    //return !memcmp(&lhs, &rhs, sizeof(Addr48Bit));
    const uint8_t * a = lhs.b;
    const uint8_t * b = rhs.b;
    return a[0] == b[0] &&
           a[1] == b[1] &&
           a[2] == b[2] &&
           a[3] == b[3] &&
           a[4] == b[4] &&
           a[5] == b[5];
}

inline bool operator!=(const Addr48Bit& lhs, const Addr48Bit& rhs) noexcept
{ return !(lhs == rhs); }


class DataType01 {
    public:
        Addr48Bit address;
        uint8_t type;

    private:
        jau::relaxed_atomic_size_t hash = 0; // default 0, cache

    public:
        DataType01(const Addr48Bit & address_, uint8_t type_)
        : address(address_), type(type_) {}

        DataType01(const uint64_t encoded) noexcept
        : address(encoded), type(0) { }

        constexpr DataType01() noexcept : address(), type{0} { }
        DataType01(const DataType01 &o) noexcept : address(o.address), type(o.type) { }
        DataType01(DataType01 &&o) noexcept {
            address = std::move(o.address);
            type = std::move(o.type);
        }
        constexpr DataType01& operator=(const DataType01 &o) noexcept {
            address = o.address;
            type = o.type;
            return *this;
        }
        DataType01& operator=(DataType01 &&o) noexcept {
            address = std::move(o.address);
            type = std::move(o.type);
            return *this;
        }

        int nop() const noexcept { return address.b[0]+1; }

        std::size_t hash_code() const noexcept {
            std::size_t h = hash;
            if( 0 == h ) {
                // 31 * x == (x << 5) - x
                h = 31 + address.hash_code();
                h = ((h << 5) - h) + type;
                const_cast<DataType01 *>(this)->hash = h;
                // printf("hash.dataSet01 new %zu\n", h);
            } else {
                // printf("hash.dataSet01 *cache* %zu\n", h);
            }
            return h;
        }

        void clearHash() { hash = 0; }

        constexpr_func_cxx20 std::string toString() const noexcept {
            return "["+address.toString()+", "+std::to_string(type)+"]";
        }
#if 0
        constexpr_cxx20 operator std::string() const noexcept {
            return toString();
        }
#endif
};
JAU_TYPENAME_CUE_ALL(DataType01)

std::ostream & operator << (std::ostream &out, const DataType01 &a) {
    out << a.toString();
    return out;
}

inline bool operator==(const DataType01& lhs, const DataType01& rhs) noexcept {
    if( &lhs == &rhs ) {
        return true;
    }
    return lhs.address == rhs.address &&
           lhs.type == rhs.type;
}
inline bool operator!=(const DataType01& lhs, const DataType01& rhs) noexcept
{ return !(lhs == rhs); }

// injecting specialization of std::hash to namespace std of our types above
namespace std
{
    template<> struct hash<Addr48Bit> {
        std::size_t operator()(Addr48Bit const& a) const noexcept {
            return a.hash_code();
        }
    };

    template<> struct hash<DataType01> {
        std::size_t operator()(DataType01 const& a) const noexcept {
            return a.hash_code();
        }
    };
}

#endif /* TEST_DATATYPE01_HPP_ */
