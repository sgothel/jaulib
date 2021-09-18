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

#include <cstring>
#include <string>
#include <memory>
#include <cstdint>
#include <cstdio>

#include <jau/eui48.hpp>

using namespace jau;

std::string EUI48Sub::toString() const noexcept {
    // str_len = 2 * len + ( len - 1 )
    // str_len = 3 * len - 1
    // len = ( str_len + 1 ) / 3
    std::string str;
    if( 0 < length ) {
        str.reserve(3 * length - 1);

        for(int i=length-1; 0 <= i; i--) {
            jau::byteHexString(str, b[i], false /* lowerCase */);
            if( 0 < i ) {
                str.push_back(':');
            }
        }
    } else {
        str.push_back(':');
    }
    return str;
}

bool EUI48Sub::scanEUI48Sub(const std::string& str, EUI48Sub& dest, std::string& errmsg) {
    const jau::nsize_t str_len = static_cast<jau::nsize_t>( str.length() );
    dest.clear();

    if( 17 < str_len ) { // not exceeding byte_size
        errmsg.append("EUI48 sub-string must be less or equal length 17 but "+std::to_string(str_len)+": "+str);
        return false;
    }
    const char * str_ptr = str.c_str();
    jau::nsize_t j=0;
    bool exp_colon = false;
    uint8_t b_[6]; // intermediate result high -> low
    while( j+1 < str_len /* && byte_count_ < byte_size */ ) { // min 2 chars left
        const bool is_colon = ':' == str[j];
        if( exp_colon && !is_colon ) {
            errmsg.append("EUI48Sub sub-string not in format '01:02:03:0A:0B:0C', but '"+str+"', colon missing, pos "+std::to_string(j)+", len "+std::to_string(str_len));
            return false;
        } else if( is_colon ) {
            ++j;
            exp_colon = false;
        } else {
            if ( sscanf(str_ptr+j, "%02hhx", &b_[dest.length]) != 1 ) // b_: high->low
            {
                errmsg.append("EUI48Sub sub-string not in format '01:02:03:0A:0B:0C' but '"+str+"', pos "+std::to_string(j)+", len "+std::to_string(str_len));
                return false;
            }
            j += 2;
            ++dest.length;
            exp_colon = true;
        }
    }
    for(j=0; j<dest.length; ++j) { // swap low->high
        dest.b[j] = b_[dest.length-1-j];
    }
    // sscanf provided host data type, in which we store the values,
    // hence no endian conversion
    return true;
}

EUI48Sub::EUI48Sub(const std::string& str) {
    std::string errmsg;
    if( !scanEUI48Sub(str, *this, errmsg) ) {
        throw jau::IllegalArgumentException(errmsg, E_FILE_LINE);
    }
}

EUI48Sub::EUI48Sub(const uint8_t * b_, const jau::nsize_t len_) noexcept {
    length = len_;
    const jau::nsize_t cpsz = std::max<jau::nsize_t>(sizeof(b), len_);
    const jau::nsize_t bzsz = sizeof(b) - cpsz;
    memcpy(b, b_, cpsz);
    if( bzsz > 0 ) {
        bzero(b+cpsz, bzsz);
    }
}

jau::snsize_t EUI48Sub::indexOf(const uint8_t haystack_b[], const jau::nsize_t haystack_length,
                                const uint8_t needle_b[], const jau::nsize_t needle_length) noexcept {
    if( 0 == needle_length ) {
        return 0;
    }
    if( haystack_length < needle_length ) {
        return -1;
    }
    const uint8_t first = needle_b[0];
    const jau::nsize_t outerEnd = haystack_length - needle_length + 1; // exclusive

    for (jau::nsize_t i = 0; i < outerEnd; i++) {
        // find first char of other
        while( haystack_b[i] != first ) {
            if( ++i == outerEnd ) {
                return -1;
            }
        }
        if( i < outerEnd ) { // otherLen chars left to match?
            // continue matching other chars
            const jau::nsize_t innerEnd = i + needle_length; // exclusive
            jau::nsize_t j = i, k=0;
            do {
                if( ++j == innerEnd ) {
                    return i; // gotcha
                }
            } while( haystack_b[j] == needle_b[++k] );
        }
    }
    return -1;
}

std::string EUI48::toString() const noexcept {
    // str_len = 2 * len + ( len - 1 )
    // str_len = 3 * len - 1
    // len = ( str_len + 1 ) / 3
    std::string str;
    str.reserve(17); // 6 * 2 + ( 6 - 1 )

    for(int i=6-1; 0 <= i; i--) {
        jau::byteHexString(str, b[i], false /* lowerCase */);
        if( 0 < i ) {
            str.push_back(':');
        }
    }
    return str;
}

bool EUI48::scanEUI48(const std::string& str, EUI48& dest, std::string& errmsg) {
    if( 17 != str.length() ) {
        errmsg.append("EUI48 string not of length 17 but ");
        errmsg.append(std::to_string(str.length()));
        errmsg.append(": "+str);
        return false;
    }
    if ( sscanf(str.c_str(), "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx",
                &dest.b[5], &dest.b[4], &dest.b[3], &dest.b[2], &dest.b[1], &dest.b[0]) != 6 )
    {
        errmsg.append("EUI48 string not in format '01:02:03:0A:0B:0C' but '"+str+"'");
        return false;
    }
    // sscanf provided host data type, in which we store the values,
    // hence no endian conversion
    return true;
}

EUI48::EUI48(const std::string& str) {
    std::string errmsg;
    if( !scanEUI48(str, *this, errmsg) ) {
        throw jau::IllegalArgumentException(errmsg, E_FILE_LINE);
    }
}

EUI48::EUI48(const uint8_t * b_) noexcept {
    memcpy(b, b_, sizeof(b));
}

static uint8_t _EUI48_ALL_DEVICE[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
static uint8_t _EUI48_LOCAL_DEVICE[] = {0x00, 0x00, 0x00, 0xff, 0xff, 0xff};

const EUI48Sub jau::EUI48Sub::ANY_DEVICE; // default ctor is zero bytes!
const EUI48Sub jau::EUI48Sub::ALL_DEVICE( _EUI48_ALL_DEVICE, 6 );
const EUI48Sub jau::EUI48Sub::LOCAL_DEVICE( _EUI48_LOCAL_DEVICE, 6 );

const EUI48 jau::EUI48::ANY_DEVICE; // default ctor is zero bytes!
const EUI48 jau::EUI48::ALL_DEVICE( _EUI48_ALL_DEVICE );
const EUI48 jau::EUI48::LOCAL_DEVICE( _EUI48_LOCAL_DEVICE );

