/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020 Gothel Software e.K.
 * Copyright (c) 2020 ZAFENA AB
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

#ifndef JAU_EUI48_HPP_
#define JAU_EUI48_HPP_

#include <cstring>
#include <string>
#include <memory>
#include <cstdint>
#include <vector>

#include <jau/basic_types.hpp>
#include <jau/packed_attribute.hpp>

namespace jau {

    /** @defgroup NetUtils Network Utilities
     *  Networking types and functionality.
     *
     *  @{
     */

    /**
     * A 48 bit EUI-48 sub-identifier, see EUI48.
     *
     * Stores value in endian::native byte order.
     */
    struct EUI48Sub {
      public:
        /** EUI48 MAC address matching any device, i.e. `0:0:0:0:0:0`. */
        static const EUI48Sub ANY_DEVICE;
        /** EUI48 MAC address matching all device, i.e. `ff:ff:ff:ff:ff:ff`. */
        static const EUI48Sub ALL_DEVICE;
        /** EUI48 MAC address matching local device, i.e. `0:0:0:ff:ff:ff`. */
        static const EUI48Sub LOCAL_DEVICE;

        /**
         * The <= 6 byte EUI48 sub-address.
         */
        uint8_t b[6]; // == sizeof(EUI48)

        /**
         * The actual length in bytes of the EUI48 sub-address, less or equal 6 bytes.
         */
        jau::nsize_t length;

        constexpr EUI48Sub() noexcept : b{0}, length{0} { }

        /**
         * Copy len_ address bytes from given source and byte_order,
         * while converting them to endian::native byte order.
         *
         * @param b_ sub address bytes in endian::native byte order
         * @param len_ length
         * @param byte_order endian::little or endian::big byte order of given sub_address, one may pass endian::native.
         */
        EUI48Sub(const uint8_t * b_, const jau::nsize_t len_, const endian byte_order) noexcept;

        /**
         * Fills given EUI48Sub instance via given string representation.
         * <p>
         * Implementation is consistent with EUI48Sub::toString().
         * </p>
         * @param str a string of less or equal of 17 characters representing less or equal of 6 bytes as hexadecimal numbers separated via colon,
         * e.g. `01:02:03:0A:0B:0C`, `01:02:03:0A`, `:`, (empty).
         * @param dest EUI48Sub to set its value
         * @param errmsg error parsing message if returning false
         * @return true if successful, otherwise false
         * @see EUI48Sub::EUI48Sub
         * @see EUI48Sub::toString()
         */
        static bool scanEUI48Sub(const std::string& str, EUI48Sub& dest, std::string& errmsg);

        /**
         * Construct a sub EUI48 via given string representation.
         * <p>
         * Implementation is consistent with EUI48Sub::toString().
         * </p>
         * @param str a string of less or equal of 17 characters representing less or equal of 6 bytes as hexadecimal numbers separated via colon,
         * e.g. `01:02:03:0A:0B:0C`, `01:02:03:0A`, `:`, (empty).
         * @see EUI48Sub::scanEUI48Sub()
         * @see EUI48Sub::toString()
         * @throws jau::IllegalArgumentException if given string doesn't comply with EUI48
         */
        EUI48Sub(const std::string& str);

        constexpr EUI48Sub(const EUI48Sub &o) noexcept = default;
        EUI48Sub(EUI48Sub &&o) noexcept = default;
        constexpr EUI48Sub& operator=(const EUI48Sub &o) noexcept = default;
        EUI48Sub& operator=(EUI48Sub &&o) noexcept = default;

        constexpr std::size_t hash_code() const noexcept {
            // 31 * x == (x << 5) - x
            std::size_t h = length;
            for(jau::nsize_t i=0; i<length; i++) {
                h = ( ( h << 5 ) - h ) + b[i];
            }
            return h;
        }

        /**
         * Method clears the underlying byte array {@link #b} and sets length to zero.
         */
        void clear() {
            b[0] = 0; b[1] = 0; b[2] = 0;
            b[3] = 0; b[4] = 0; b[5] = 0;
            length = 0;
        }

        /**
         * Find index of needle within haystack in the given byte order.
         *
         * The returned index will be adjusted for the desired byte order.
         * - endian::big will return index 0 for the leading byte like the toString() representation from left (MSB) to right (LSB).
         * - endian::little will return index 5 for the leading byte
         *
         * @param haystack_b haystack data
         * @param haystack_length haystack length
         * @param needle_b needle data
         * @param needle_length needle length
         * @param byte_order byte order will adjust the returned index, endian::big is equivalent with toString() representation from left (MSB) to right (LSB).
         * @return index of first element of needle within haystack or -1 if not found. If the needle length is zero, 0 (found) is returned.
         */
        static jau::snsize_t indexOf(const uint8_t haystack_b[], const jau::nsize_t haystack_length,
                                     const uint8_t needle_b[], const jau::nsize_t needle_length,
                                     const endian byte_order) noexcept;

        /**
         * Finds the index of given EUI48Sub needle within this instance haystack in the given byte order.
         *
         * The returned index will be adjusted for the desired byte order.
         * - endian::big will return index 0 for the leading byte like the toString() representation from left (MSB) to right (LSB).
         * - endian::little will return index 5 for the leading byte
         *
         * @param needle
         * @param byte_order byte order will adjust the returned index, endian::big is equivalent with toString() representation from left (MSB) to right (LSB).
         * @return index of first element of needle within this instance haystack or -1 if not found. If the needle length is zero, 0 (found) is returned.
         * @see indexOf()
         */
        jau::snsize_t indexOf(const EUI48Sub& needle, const endian byte_order) const noexcept {
            return indexOf(b, length, needle.b, needle.length, byte_order);
        }

        /**
         * Returns true, if given EUI48Sub needle is contained in this instance haystack.
         * <p>
         * If the sub is zero, true is returned.
         * </p>
         */
        bool contains(const EUI48Sub& needle) const noexcept {
            return 0 <= indexOf(needle, endian::native);
        }

        /**
         * Returns the EUI48 sub-string representation with MSB first (endian::big),
         * less or equal 17 characters representing less or equal 6 bytes as upper case hexadecimal numbers separated via colon,
         * e.g. `01:02:03:0A:0B:0C`, `01:02:03:0A`, `:`, (empty).
         */
        std::string toString() const noexcept;
    };
    inline std::string to_string(const EUI48Sub& a) noexcept { return a.toString(); }

    inline bool operator==(const EUI48Sub& lhs, const EUI48Sub& rhs) noexcept {
        if( &lhs == &rhs ) {
            return true;
        }
        if( lhs.length != rhs.length ) {
            return false;
        }
        return !memcmp(&lhs.b, &rhs.b, lhs.length);
    }

    inline bool operator!=(const EUI48Sub& lhs, const EUI48Sub& rhs) noexcept
    { return !(lhs == rhs); }


    /**
     * A packed 48 bit EUI-48 identifier, formerly known as MAC-48
     * or simply network device MAC address (Media Access Control address).
     *
     * Stores value in endian::native byte order.
     */
    __pack ( struct EUI48 {
        /** EUI48 MAC address matching any device, i.e. `0:0:0:0:0:0`. */
        static const EUI48 ANY_DEVICE;
        /** EUI48 MAC address matching all device, i.e. `ff:ff:ff:ff:ff:ff`. */
        static const EUI48 ALL_DEVICE;
        /** EUI48 MAC address matching local device, i.e. `0:0:0:ff:ff:ff`. */
        static const EUI48 LOCAL_DEVICE;

        /**
         * The 6 byte EUI48 address.
         */
        uint8_t b[6]; // == sizeof(EUI48)

        constexpr EUI48() noexcept : b{0}  { }

        /**
         * Copy address bytes from given source and byte_order,
         * while converting them to endian::native byte order.
         *
         * @param source address bytes
         * @param byte_order endian::little or endian::big byte order of given source, one may pass endian::native.
         */
        EUI48(const uint8_t * source, const endian byte_order) noexcept;

        /**
         * Fills given EUI48 instance via given string representation.
         * <p>
         * Implementation is consistent with EUI48::toString().
         * </p>
         * @param str a string of exactly 17 characters representing 6 bytes as hexadecimal numbers separated via colon `01:02:03:0A:0B:0C`.
         * @param dest EUI48 to set its value
         * @param errmsg error parsing message if returning false
         * @return true if successful, otherwise false
         * @see EUI48::EUI48
         * @see EUI48::toString()
         */
        static bool scanEUI48(const std::string& str, EUI48& dest, std::string& errmsg);

        /**
         * Construct instance via given string representation.
         * <p>
         * Implementation is consistent with EUI48::toString().
         * </p>
         * @param str a string of exactly 17 characters representing 6 bytes as hexadecimal numbers separated via colon `01:02:03:0A:0B:0C`.
         * @see EUI48::scanEUI48()
         * @see EUI48::toString()
         * @throws jau::IllegalArgumentException if given string doesn't comply with EUI48
         */
        EUI48(const std::string& str);

        constexpr EUI48(const EUI48 &o) noexcept = default;
        EUI48(EUI48 &&o) noexcept = default;
        constexpr EUI48& operator=(const EUI48 &o) noexcept = default;
        EUI48& operator=(EUI48 &&o) noexcept = default;

        constexpr std::size_t hash_code() const noexcept {
            // 31 * x == (x << 5) - x
            std::size_t h = b[0];
            h = ( ( h << 5 ) - h ) + b[1];
            h = ( ( h << 5 ) - h ) + b[2];
            h = ( ( h << 5 ) - h ) + b[3];
            h = ( ( h << 5 ) - h ) + b[4];
            h = ( ( h << 5 ) - h ) + b[5];
            return h;
        }

        /**
         * Method clears the underlying byte array {@link #b}.
         */
        void clear() {
            b[0] = 0; b[1] = 0; b[2] = 0;
            b[3] = 0; b[4] = 0; b[5] = 0;
        }

        /**
         * Finds the index of given EUI48Sub needle within this instance haystack.
         *
         * The returned index will be adjusted for the desired byte order.
         * - endian::big will return index 0 for the leading byte like the string representation from left (MSB) to right (LSB).
         * - endian::little will return index 5 for the leading byte
         *
         * @param needle
         * @param byte_order byte order will adjust the returned index, endian::big is equivalent to the string representation from left (MSB) to right (LSB).
         * @return index of first element of needle within this instance haystack or -1 if not found. If the needle length is zero, 0 (found) is returned.
         * @see indexOf()
         */
        jau::snsize_t indexOf(const EUI48Sub& needle, const endian byte_order) const noexcept {
            return EUI48Sub::indexOf(b, sizeof(b), needle.b, needle.length, byte_order);
        }

        /**
         * Returns true, if given EUI48Sub needle is contained in this instance haystack.
         * <p>
         * If the sub is zero, true is returned.
         * </p>
         */
        bool contains(const EUI48Sub& needle) const noexcept {
            return 0 <= indexOf(needle, endian::native);
        }

        /**
         * Returns the EUI48 string representation with MSB first (endian::big),
         * exactly 17 characters representing 6 bytes as upper case hexadecimal numbers separated via colon `01:02:03:0A:0B:0C`.
         * @see EUI48::EUI48()
         */
        std::string toString() const noexcept;

        /**
         * Method transfers all bytes representing this instance into the given
         * destination array at the given position and in the given byte order.
         * <p>
         * Implementation is consistent with {@link #EUI48(byte[], int, ByteOrder)}.
         * </p>
         * @param sink the destination array
         * @param sink_pos starting position in the destination array
         * @param byte_order destination buffer byte order
         * @see #EUI48(byte[], int, ByteOrder)
         */
        jau::nsize_t put(uint8_t * const sink, jau::nsize_t const sink_pos, const endian byte_order) const noexcept;
    } );
    inline std::string to_string(const EUI48& a) noexcept { return a.toString(); }

    inline bool operator==(const EUI48& lhs, const EUI48& rhs) noexcept {
        if( &lhs == &rhs ) {
            return true;
        }
        //return !memcmp(&lhs, &rhs, sizeof(EUI48));
        const uint8_t * a = lhs.b;
        const uint8_t * b = rhs.b;
        return a[0] == b[0] &&
               a[1] == b[1] &&
               a[2] == b[2] &&
               a[3] == b[3] &&
               a[4] == b[4] &&
               a[5] == b[5];
    }

    inline bool operator!=(const EUI48& lhs, const EUI48& rhs) noexcept
    { return !(lhs == rhs); }

    constexpr static void bswap_6bytes(uint8_t* sink, const uint8_t* source) noexcept {
        sink[0] = source[5];
        sink[1] = source[4];
        sink[2] = source[3];
        sink[3] = source[2];
        sink[4] = source[1];
        sink[5] = source[0];
    }

    constexpr EUI48 bswap(EUI48 const & source) noexcept {
        EUI48 dest;
        bswap_6bytes(dest.b, source.b);
        return dest;
    }

    constexpr EUI48 be_to_cpu(EUI48 const & n) noexcept {
        static_assert(isLittleOrBigEndian()); // one static_assert is sufficient for whole compilation unit
        if( isLittleEndian() ) {
            return bswap(n);
        } else {
            return n;
        }
    }
    constexpr EUI48 cpu_to_be(EUI48 const & h) noexcept {
        if( isLittleEndian() ) {
            return bswap(h);
        } else {
            return h;
        }
    }
    constexpr EUI48 le_to_cpu(EUI48 const & l) noexcept {
        if( isLittleEndian() ) {
            return l;
        } else {
            return bswap(l);
        }
    }
    constexpr EUI48 cpu_to_le(EUI48 const & h) noexcept {
        if( isLittleEndian() ) {
            return h;
        } else {
            return bswap(h);
        }
    }

    /**@}*/

} /* namespace jau */

// injecting specialization of std::hash to namespace std of our types above
namespace std
{
    /** \addtogroup NetUtils
     *
     */

    template<> struct hash<jau::EUI48Sub> {
        std::size_t operator()(jau::EUI48Sub const& a) const noexcept {
            return a.hash_code();
        }
    };

    template<> struct hash<jau::EUI48> {
        std::size_t operator()(jau::EUI48 const& a) const noexcept {
            return a.hash_code();
        }
    };

    /**@}*/
}

#endif /* JAU_EUI48_HPP_ */
