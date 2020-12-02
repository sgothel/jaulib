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

#include <cstdint>
#include <cinttypes>

#include <algorithm>

#include <jau/debug.hpp>
#include <jau/basic_types.hpp>

using namespace jau;

static const int64_t NanoPerMilli = 1000000L;
static const int64_t MilliPerOne = 1000L;

/**
 * See <http://man7.org/linux/man-pages/man2/clock_gettime.2.html>
 * <p>
 * Regarding avoiding kernel via VDSO,
 * see <http://man7.org/linux/man-pages/man7/vdso.7.html>,
 * clock_gettime seems to be well supported at least on kernel >= 4.4.
 * Only bfin and sh are missing, while ia64 seems to be complicated.
 */
uint64_t jau::getCurrentMilliseconds() noexcept {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return static_cast<uint64_t>( t.tv_sec * MilliPerOne + t.tv_nsec / NanoPerMilli );
}

jau::RuntimeException::RuntimeException(std::string const type, std::string const m, const char* file, int line) noexcept
: msg(std::string(type).append(" @ ").append(file).append(":").append(std::to_string(line)).append(": ").append(m)),
  backtrace(jau::get_backtrace(true, 1))
{
}

const char* jau::RuntimeException::what() const noexcept {
    // return std::runtime_error::what();
    std::string out(msg);
    out.append("\nNative backtrace:\n");
    out.append(backtrace);
    return out.c_str();
}

std::string jau::get_string(const uint8_t *buffer, nsize_t const buffer_len, nsize_t const max_len) noexcept {
    const nsize_t cstr_len = std::min(buffer_len, max_len);
    char cstr[max_len+1]; // EOS
    memcpy(cstr, buffer, cstr_len);
    cstr[cstr_len] = 0; // EOS
    return std::string(cstr);
}

uint128_t jau::merge_uint128(uint16_t const uuid16, uint128_t const & base_uuid, nsize_t const uuid16_le_octet_index)
{
    if( uuid16_le_octet_index > 14 ) {
        std::string msg("uuid16_le_octet_index ");
        msg.append(std::to_string(uuid16_le_octet_index));
        msg.append(", not within [0..14]");
        throw IllegalArgumentException(msg, E_FILE_LINE);
    }
    uint128_t dest = base_uuid;

    // base_uuid: 00000000-0000-1000-8000-00805F9B34FB
    //    uuid16: DCBA
    // uuid16_le_octet_index: 12
    //    result: 0000DCBA-0000-1000-8000-00805F9B34FB
    //
    // LE: low-mem - FB349B5F8000-0080-0010-0000-ABCD0000 - high-mem
    //                                           ^ index 12
    // LE: uuid16 -> value.data[12+13]
    //
    // BE: low-mem - 0000DCBA-0000-1000-8000-00805F9B34FB - high-mem
    //                   ^ index 2
    // BE: uuid16 -> value.data[2+3]
    //
#if __BYTE_ORDER == __BIG_ENDIAN
    nsize_t offset = 15 - 1 - uuid16_le_octet_index;
#elif __BYTE_ORDER == __LITTLE_ENDIAN
    nsize_t offset = uuid16_le_octet_index;
#else
#error "Unexpected __BYTE_ORDER"
#endif
    // uint16_t * destu16 = (uint16_t*)(dest.data + offset);
    // *destu16 += uuid16;
    reinterpret_cast<packed_t<uint16_t>*>( dest.data + offset )->store += uuid16;
    return dest;
}

uint128_t jau::merge_uint128(uint32_t const uuid32, uint128_t const & base_uuid, nsize_t const uuid32_le_octet_index)
{
    if( uuid32_le_octet_index > 12 ) {
        std::string msg("uuid32_le_octet_index ");
        msg.append(std::to_string(uuid32_le_octet_index));
        msg.append(", not within [0..12]");
        throw IllegalArgumentException(msg, E_FILE_LINE);
    }
    uint128_t dest = base_uuid;

    // base_uuid: 00000000-0000-1000-8000-00805F9B34FB
    //    uuid32: 87654321
    // uuid32_le_octet_index: 12
    //    result: 87654321-0000-1000-8000-00805F9B34FB
    //
    // LE: low-mem - FB349B5F8000-0080-0010-0000-12345678 - high-mem
    //                                           ^ index 12
    // LE: uuid32 -> value.data[12..15]
    //
    // BE: low-mem - 87654321-0000-1000-8000-00805F9B34FB - high-mem
    //               ^ index 0
    // BE: uuid32 -> value.data[0..3]
    //
#if __BYTE_ORDER == __BIG_ENDIAN
    nsize_t offset = 15 - 3 - uuid32_le_octet_index;
#elif __BYTE_ORDER == __LITTLE_ENDIAN
    nsize_t offset = uuid32_le_octet_index;
#else
#error "Unexpected __BYTE_ORDER"
#endif
    // uint32_t * destu32 = (uint32_t*)(dest.data + offset);
    // *destu32 += uuid32;
    reinterpret_cast<packed_t<uint32_t>*>( dest.data + offset )->store += uuid32;
    return dest;
}

static const char* HEX_ARRAY_LOW = "0123456789abcdef";
static const char* HEX_ARRAY_BIG = "0123456789ABCDEF";

std::string jau::bytesHexString(const uint8_t * bytes, const nsize_t offset, const nsize_t length, const bool lsbFirst,
                                const bool leading0X, const bool lowerCase) noexcept
{
    const char* hex_array = lowerCase ? HEX_ARRAY_LOW : HEX_ARRAY_BIG;
    std::string str;

    if( nullptr == bytes ) {
        return "null";
    }
    if( 0 == length ) {
        return "nil";
    }
    if( leading0X ) {
        str.reserve(2 + length * 2 +1);
        str.push_back('0');
        str.push_back('x');
    } else {
        str.reserve(length * 2 +1);
    }
    if( lsbFirst ) {
        // LSB left -> MSB right
        for (nsize_t j = 0; j < length; j++) {
            const int v = bytes[offset+j] & 0xFF;
            str.push_back(hex_array[v >> 4]);
            str.push_back(hex_array[v & 0x0F]);
        }
    } else {
        // MSB left -> LSB right
        nsize_t j = length;
        do {
            j--;
            const int v = bytes[offset+j] & 0xFF;
            str.push_back(hex_array[v >> 4]);
            str.push_back(hex_array[v & 0x0F]);
        } while( j != 0);
    }
    return str;
}

void jau::trimInPlace(std::string &s) noexcept {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch);
    }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

std::string jau::trimCopy(const std::string &_s) noexcept {
    std::string s(_s);
    trimInPlace(s);
    return s;
}

