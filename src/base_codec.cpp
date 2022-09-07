/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2022 Gothel Software e.K.
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

#include <jau/base_codec.hpp>
#include <jau/basic_algos.hpp>
#include <jau/debug.hpp>

using namespace jau;
using namespace jau::codec::base;

std::string jau::codec::base::encode(int num, const alphabet& aspec, const unsigned int min_width) noexcept
{
    const int base = aspec.base();
    if( 0 > num || 1 >= base ) {
        return "";
    }
    std::string res;
    do {
        std::div_t quotient = std::div(num, base);
        res.insert( res.begin(), aspec[ quotient.rem ] ); // safe: base <= alphabet.length()
        num = quotient.quot;
    } while ( 0 != num );

    const char s0 = aspec[0];
    for(unsigned int i=res.length(); i<min_width; ++i) {
        res.insert(res.begin(), s0);
    }
    return res;
}

std::string jau::codec::base::encode(int64_t num, const alphabet& aspec, const unsigned int min_width) noexcept {
    const int base = aspec.base();
    if( 0 > num || 1 >= base ) {
        return "";
    }
    std::string res;
    do {
        std::lldiv_t quotient = std::lldiv(num, (int64_t)base);
        res.insert( res.begin(), aspec[ quotient.rem ] ); // safe: base <= alphabet.length()
        num = quotient.quot;
    } while ( 0 != num );

    const char s0 = aspec[0];
    for(unsigned int i=res.length(); i<min_width; ++i) {
        res.insert(res.begin(), s0);
    }
    return res;
}

int64_t jau::codec::base::decode(const std::string_view& str, const alphabet& aspec) noexcept
{
    const int base = aspec.base();
    if( 1 >= base ) {
        return -1;
    }
    std::string::size_type str_len = str.length();
    int64_t res = 0;
    for (std::string::size_type pos = 0; pos < str_len; ++pos) {
        const int cp = aspec.code_point( str[pos] );
        if( 0 > cp ) {
            return -1; // encoded value not found
        }
        res = res * base + static_cast<int64_t>(cp);
    }
    return res;
}

std::string jau::codec::base::encode64(const void* in_octets, size_t in_len, const alphabet& aspec) noexcept {
    if( 64 != aspec.base() ) {
        return "";
    }
    const char padding = aspec.padding64();
    const uint8_t* in_bytes = (const uint8_t*)in_octets;

    size_t out_len = ( in_len + 2 ) / 3 * 4; // estimate ..
    std::string res;
    res.reserve(out_len);

    while( 0 < in_len && 0 < out_len ) {
        // Note: Addition is basically a bitwise XOR, plus carry bit

        // 1st symbol
        res.push_back( aspec[ ( in_bytes[0] >> 2 ) & 0x3f ] ); // take in[0] 6 bits[7..2] -> symbol[5..0]
        if( 0 == --in_len ) {
            // len == 1 bytes
            // 2nd symbol
            res.push_back( aspec[   ( in_bytes[0] << 4 ) & 0x3f ] ); // take in[0] 2 bits[1..0] -> symbol[5..4]
            if( 0 != padding ) {
                res.push_back(padding);
                res.push_back(padding);
            }
            break;
        } else {
            // len >= 2 bytes
            // 2nd symbol
            res.push_back( aspec[ ( ( in_bytes[0] << 4 ) + ( in_bytes[1] >> 4) ) & 0x3f ] ); // take ( in[0] 2 bits[1..0] -> symbol[5..4] ) + ( int[1] 4 bits[7..4] -> symbol[3..0] )
        }
        if( 0 == --in_len ) {
            // len == 2 bytes
            // 3rd symbol
            res.push_back( aspec[   ( in_bytes[1] << 2 ) & 0x3f ] ); // take in[1] 4 bits[3..0] -> symbol[5..2]
            if( 0 != padding ) {
                res.push_back(padding);
            }
            break;
        } else {
            // len >= 3 bytes
            // 3rd symbol
            res.push_back( aspec[ ( ( in_bytes[1] << 2 ) + ( in_bytes[2] >> 6) ) & 0x3f ] ); // take ( in[1] 4 bits[3..0] -> symbol[5..2] ) + ( int[2] 2 bits[7..6] -> symbol[1..0] )
            // 4th symbol
            res.push_back( aspec[     in_bytes[2] & 0x3f ] ); // take in[2] 6 bits[5..0] -> symbol[5..0]
            --in_len;
            in_bytes+=3;
        }
    }
    return res;
}

std::vector<uint8_t> jau::codec::base::decode64(const std::string_view& in_code, const alphabet& aspec) noexcept {
    if( 64 != aspec.base() ) {
        return std::vector<uint8_t>(); // Error
    }
    size_t in_len = in_code.length();
    if( 0 == in_len ) {
        return std::vector<uint8_t>(); // OK
    }
    const char padding = aspec.padding64();
    const char* in_chars = in_code.data();

    const size_t out_len = 3 * ( in_len / 4 ) + 2; // estimate w/ potentially up to 2 additional bytes
    std::vector<uint8_t> res;
    res.reserve(out_len);

    while( in_len >= 2 ) {
        const int cp0 = aspec.code_point( in_chars[0] );
        const int cp1 = aspec.code_point( in_chars[1] );
        if( 0 > cp0 || 0 > cp1 ) {
            break;
        }
        res.push_back( cp0 << 2 | cp1 >> 4 );
        if( 2 == in_len ) {
            if( 0 == padding ) {
                in_len = 0; // accept w/o padding
            }
            break;
        }
        if( padding == in_chars[2] ) {
            if( 4 != in_len ) {
                break;
            }
            if( padding != in_chars[3] ) {
                break;
            }
        } else {
            const int cp2 = aspec.code_point( in_chars[2] );
            if( 0 > cp2 ) {
                break;
            }
            res.push_back( ( ( cp1 << 4 ) & 0xf0 ) | ( cp2 >> 2 ) );
            if( 3 == in_len ) {
                if( 0 == padding ) {
                    in_len = 0; // accept w/o padding
                }
                break;
            }
            if( padding == in_chars[3] ) {
                if( 4 != in_len ) {
                    break;
                }
            } else {
                const int cp3 = aspec.code_point( in_chars[3] );
                if( 0 > cp3 ) {
                    break;
                }
                res.push_back( ( ( cp2 << 6 ) & 0xc0 ) | cp3 );
            }
        }
        in_chars += 4;
        in_len -= 4;
    }

    if( 0 != in_len ) {
        DBG_PRINT("in_len %zu/%zu at '%s', out_len %zu/%zu\n", (in_code.length()-in_len), in_code.length(), std::string(in_code).c_str(), res.size(), out_len);
        return std::vector<uint8_t>(); // decoding error
    } else {
        return res;
    }
}

size_t jau::codec::base::insert_lf(std::string& str, const size_t period) noexcept {
    size_t count = 0;
    for(size_t i = period; i < str.length(); i += period + 1) {
        str.insert(i, "\n");
        ++count;
    }
    return count;
}

size_t jau::codec::base::remove_lf(std::string& str) noexcept {
    size_t count = 0;
    auto it = jau::remove_if( str.begin(), str.end(), [&count](char c){
        if( c == 0x0a ) {
            ++count;
            return true;
        } else {
            return false;
        }
    });
    str.erase(it, str.end()); // erase empty tail
    return count;
}
