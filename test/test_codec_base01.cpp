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
#include <cassert>
#include <cinttypes>
#include <cstring>

#include <jau/test/catch2_ext.hpp>

#include <jau/base_codec.hpp>

using namespace jau::int_literals;

static void testRadix_3digits_int32(const int base, const jau::codec::base::alphabet& aspec) {
    REQUIRE( 1 < base );
    REQUIRE( base <= aspec.max_base() );

    const char min_cp = aspec[0]; // minimum code-point
    const char max_cp = aspec[base-1]; // maximum code-point

    const int min = (int)jau::codec::base::decode(std::string()+min_cp, base, aspec);
    const int max = (int)jau::codec::base::decode(std::string()+max_cp+max_cp+max_cp, base, aspec);
    const int max_s = (int)jau::codec::base::decode(std::string()+max_cp, base, aspec);

    const double machine_epsilon = std::numeric_limits<double>::epsilon();
    REQUIRE(0 == min);
    REQUIRE(base-1 == max_s);
    REQUIRE( std::abs( std::pow(base, 3)-1 - max ) <=  machine_epsilon );

    const std::string r1_min = jau::codec::base::encode(0, base, aspec, 3);
    const std::string r1_min_s = jau::codec::base::encode(0, base, aspec);
    REQUIRE(std::string()+min_cp+min_cp+min_cp == r1_min);
    REQUIRE(std::string()+min_cp == r1_min_s);

    const std::string r1_max = jau::codec::base::encode(base-1, base, aspec, 3);
    const std::string r1_max_s = jau::codec::base::encode(base-1, base, aspec);
    REQUIRE(std::string()+min_cp+min_cp+max_cp == r1_max);
    REQUIRE(std::string()+max_cp == r1_max_s);

    const std::string r3_max = jau::codec::base::encode((int)std::pow(base, 3)-1, base, aspec, 3);
    REQUIRE(std::string()+max_cp+max_cp+max_cp == r3_max);

    fprintf(stderr, "Test32Bit base %d, %s: [%d .. %d] <-> ['%s' .. '%s'], %d years (max/365d) \n",
            base, aspec.to_string().c_str(), min, max, jau::codec::base::encode(min, base, aspec).c_str(), jau::codec::base::encode(max, base, aspec).c_str(), (max/365));

    REQUIRE(0 == jau::codec::base::decode(std::string()+min_cp+min_cp+min_cp, base, aspec));
    REQUIRE(std::string()+min_cp == jau::codec::base::encode(0, base, aspec));
    REQUIRE(std::string()+min_cp+min_cp+min_cp == jau::codec::base::encode(0, base, aspec, 3));

    REQUIRE(max == jau::codec::base::decode(std::string()+max_cp+max_cp+max_cp, base, aspec));
    REQUIRE(std::string()+max_cp+max_cp+max_cp == jau::codec::base::encode(max, base, aspec, 3));
    REQUIRE(max_s == jau::codec::base::decode(std::string()+max_cp, base, aspec));
    REQUIRE(std::string()+min_cp+min_cp+max_cp == jau::codec::base::encode(max_s, base, aspec, 3));

    {
        const int v0_d = jau::codec::base::decode(r1_max, base, aspec);
        const std::string v1_s = jau::codec::base::encode(base-1, base, aspec, 3);
        REQUIRE(r1_max == v1_s);
        REQUIRE(base-1 == v0_d);
    }
    {
        const int v0_d = jau::codec::base::decode(r3_max, base, aspec);
        const std::string v1_s = jau::codec::base::encode(max, base, aspec, 3);
        REQUIRE(r3_max == v1_s);
        REQUIRE(max == v0_d);
    }
    for(int iter=min; iter<=max; ++iter) {
        const std::string rad = jau::codec::base::encode(iter, base, aspec, 3);
        const int dec = jau::codec::base::decode(rad, base, aspec);
#if 0
        fprintf(stderr, "test base %d: iter %d, rad '%s' %03d %03d %03d, dec %d\n",
                base, iter, rad.c_str(), (int)(0xFF & rad[0]), (int)(0xFF & rad[1]), (int)(0xFF & rad[2]), dec);
#endif
        REQUIRE(iter == dec);
    }
    static jau::codec::base::natural86_alphabet natural86_alphabet;
    if( natural86_alphabet == aspec ) {
        // Test 0-9 ..
        fprintf(stderr, "Natural 0-9: ");
        for(int iter=0; iter<=9; ++iter) {
            const std::string rad = jau::codec::base::encode(iter, base, aspec);
            fprintf(stderr, "%s, ", rad.c_str());
            const char c = (char)('0'+iter);
            REQUIRE(std::string()+c == rad);
        }
        fprintf(stderr, "\n");
    }
}

static void testRadix_int64(const int base, const jau::codec::base::alphabet& aspec, const int64_t test_min, const int64_t test_max) {
    const int int64_max_enc_width = 11; // 9223372036854775807 ==  '7__________' (base 64, natural)

    REQUIRE( 1 < base );
    REQUIRE( base <= aspec.max_base() );

    const char min_cp = aspec[0]; // minimum code-point
    const char max_cp = aspec[base-1]; // maximum code-point

    const std::string max_radix = jau::codec::base::encode(std::numeric_limits<int64_t>::max(), base, aspec, int64_max_enc_width);

    const int64_t min = jau::codec::base::decode(std::string()+min_cp, base, aspec);
    const int64_t max = jau::codec::base::decode(max_radix, base, aspec);
    const int64_t max_s = jau::codec::base::decode(std::string()+max_cp, base, aspec);

    REQUIRE(0 == min);
    REQUIRE(base-1 == max_s);
    REQUIRE(std::numeric_limits<int64_t>::max() == max);

    const std::string r1_min = jau::codec::base::encode(0, base, aspec, int64_max_enc_width);
    const std::string r1_min_s = jau::codec::base::encode(0, base, aspec);
    REQUIRE(std::string()+min_cp+min_cp+min_cp+min_cp+min_cp+min_cp+min_cp+min_cp+min_cp+min_cp+min_cp == r1_min);
    REQUIRE(std::string()+min_cp == r1_min_s);

    const std::string r1_max = jau::codec::base::encode(base-1, base, aspec, int64_max_enc_width);
    const std::string r1_max_s = jau::codec::base::encode(base-1, base, aspec);
    REQUIRE(std::string()+min_cp+min_cp+min_cp+min_cp+min_cp+min_cp+min_cp+min_cp+min_cp+min_cp+max_cp == r1_max);
    REQUIRE(std::string()+max_cp == r1_max_s);

    fprintf(stderr, "int (sz %zu) == int32_t (sz %zu): %d, int (sz %zu) == int64_t (sz %zu): %d\n",
            sizeof(int), sizeof(int32_t), std::is_same_v<int, std::int32_t>,
            sizeof(int), sizeof(int64_t), std::is_same_v<int, std::int64_t>);

    fprintf(stderr, "Test64bit base %d, %s: [%" PRIi64 " .. %" PRIi64 "] <-> ['%s' .. '%s'], %" PRIi64 " years (max/365d) \n",
            base, aspec.to_string().c_str(),
            min, max, jau::codec::base::encode(min, base, aspec).c_str(), jau::codec::base::encode(max, base, aspec).c_str(), (max/365));

    fprintf(stderr, "- range: [%" PRIi64 " .. %" PRIi64 "] <-> ['%s' .. '%s']\n",
            test_min, test_max, jau::codec::base::encode(test_min, base, aspec).c_str(), jau::codec::base::encode(test_max, base, aspec).c_str());

    REQUIRE(0 == jau::codec::base::decode(std::string()+min_cp+min_cp+min_cp, base, aspec));
    REQUIRE(std::string()+min_cp == jau::codec::base::encode(0, base, aspec));

    {
        const int64_t v0_d = jau::codec::base::decode(r1_max, base, aspec);
        const std::string v1_s = jau::codec::base::encode(base-1, base, aspec, int64_max_enc_width);
        REQUIRE(r1_max == v1_s);
        REQUIRE(base-1 == v0_d);
    }
    for(int64_t iter=std::max(0_i64, test_min-1); iter<test_max; ) {
        ++iter;
        const std::string rad = jau::codec::base::encode(iter, base, aspec, int64_max_enc_width);
        const int64_t dec = jau::codec::base::decode(rad, base, aspec);
#if 0
        fprintf(stderr, "test base %d: iter %" PRIi64 ", rad '%s', dec %" PRIi64 "\n", base, iter, rad.c_str(), dec);
#endif
        REQUIRE(iter == dec);
    }
}

static void testIntegerBase64(const jau::codec::base::alphabet& aspec) {
    testRadix_3digits_int32(64, aspec);
    testRadix_int64(64, aspec, 0x7fffff00_i64, 0x80000100_i64);
    testRadix_int64(64, aspec, 0xFFFFFFF0_i64, 0x100000010_i64);
    testRadix_int64(64, aspec, 0x7FFFFFFFFFFFFFF0_i64, 0x7FFFFFFFFFFFFFFF_i64);
    // testRadix_int64(64, aspec, 0x0_i64, 0x7FFFFFFFFFFFFFFF_i64);
}

static void testIntegerBase86(const jau::codec::base::alphabet& aspec) {
    testRadix_3digits_int32(86, aspec);
    testRadix_int64(86, aspec, 0x7fffff00_i64, 0x80000100_i64);
    testRadix_int64(86, aspec, 0xFFFFFFF0_i64, 0x100000010_i64);
    testRadix_int64(86, aspec, 0x7FFFFFFFFFFFFFF0_i64, 0x7FFFFFFFFFFFFFFF_i64);
    // testRadix_int64(86, aspec, 0x0_i64, 0x7FFFFFFFFFFFFFFF_i64);
}

TEST_CASE( "Integer Base 64 Encoding Test 01", "[integer][type]" ) {
    testIntegerBase64(jau::codec::base::base64_alphabet());
    testIntegerBase64(jau::codec::base::base64url_alphabet());
    testIntegerBase64(jau::codec::base::natural86_alphabet());
    testIntegerBase64(jau::codec::base::ascii64_alphabet());
    testIntegerBase64(jau::codec::base::ascii86_alphabet());
}

TEST_CASE( "Integer Base 86 Encoding Test 02", "[integer][type]" ) {
    testIntegerBase86(jau::codec::base::natural86_alphabet());
    testIntegerBase86(jau::codec::base::ascii86_alphabet());
}

class base64_alphabet_nopadding : public jau::codec::base::alphabet {
    private:
        static inline constexpr const std::string_view data  = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        static int s_code_point(const char c) noexcept {
            if ('A' <= c && c <= 'Z') {
                return c - 'A';
            } else if ('a' <= c && c <= 'z') {
                return c - 'a' + 26;
            } else if ('0' <= c && c <= '9') {
                return c - '0' + 52;
            } else if ('+' == c) {
                return 62;
            } else if ('/' == c) {
                return 63;
            } else {
                return -1;
            }
        }

    public:
        base64_alphabet_nopadding() noexcept
        : alphabet("base64", 64, data, 0, s_code_point) {}
};

static void testBinaryBase64() {
    const jau::codec::base::base64_alphabet aspec;
    const jau::codec::base::base64url_alphabet aspec_url;
    base64_alphabet_nopadding aspec_nopadding;

    // Test Vectors taken from `base64` [RFC 4648](https://www.rfc-editor.org/rfc/rfc4648.html)
    {
        std::vector<uint8_t> octets = { };
        std::string encstr = jau::codec::base::encode64(octets.data(), octets.size(), aspec);
        REQUIRE( "" == encstr);
        std::vector<uint8_t> dec_octets = jau::codec::base::decode64(encstr, aspec);
        REQUIRE( octets == dec_octets );
    }
    {
        std::vector<uint8_t> octets = { 'f' };
        std::string encstr = jau::codec::base::encode64(octets.data(), octets.size(), aspec);
        REQUIRE( "Zg==" == encstr);
        std::vector<uint8_t> dec_octets = jau::codec::base::decode64(encstr, aspec);
        REQUIRE( octets == dec_octets );
    }
    {
        std::vector<uint8_t> octets = { 'f', 'o' };
        std::string encstr = jau::codec::base::encode64(octets.data(), octets.size(), aspec);
        REQUIRE( "Zm8=" == encstr);
        std::vector<uint8_t> dec_octets = jau::codec::base::decode64(encstr, aspec);
        REQUIRE( octets == dec_octets );
    }
    {
        std::vector<uint8_t> octets = { 'f', 'o', 'o' };
        std::string encstr = jau::codec::base::encode64(octets.data(), octets.size(), aspec);
        REQUIRE( "Zm9v" == encstr);
        std::vector<uint8_t> dec_octets = jau::codec::base::decode64(encstr, aspec);
        REQUIRE( octets == dec_octets );
    }
    {
        std::vector<uint8_t> octets = { 'f', 'o', 'o', 'b' };
        std::string encstr = jau::codec::base::encode64(octets.data(), octets.size(), aspec);
        REQUIRE( "Zm9vYg==" == encstr);
        std::vector<uint8_t> dec_octets = jau::codec::base::decode64(encstr, aspec);
        REQUIRE( octets == dec_octets );
    }
    {
        std::vector<uint8_t> octets = { 'f', 'o', 'o', 'b', 'a' };
        std::string encstr = jau::codec::base::encode64(octets.data(), octets.size(), aspec);
        REQUIRE( "Zm9vYmE=" == encstr);
        std::vector<uint8_t> dec_octets = jau::codec::base::decode64(encstr, aspec);
        REQUIRE( octets == dec_octets );
    }

    // Further encoding tests
    {
        std::vector<uint8_t> octets = { 'a' };
        std::string encstr = jau::codec::base::encode64(octets.data(), octets.size(), aspec);
        REQUIRE( "YQ==" == encstr);
        std::vector<uint8_t> dec_octets = jau::codec::base::decode64(encstr, aspec);
        REQUIRE( octets == dec_octets );
    }
    {
        std::vector<uint8_t> octets = { 'a', 'b' };
        std::string encstr = jau::codec::base::encode64(octets.data(), octets.size(), aspec);
        REQUIRE( "YWI=" == encstr);
        std::vector<uint8_t> dec_octets = jau::codec::base::decode64(encstr, aspec);
        REQUIRE( octets == dec_octets );
    }
    {
        std::vector<uint8_t> octets = { 'a', 'b', 'c' };
        std::string encstr = jau::codec::base::encode64(octets.data(), octets.size(), aspec);
        REQUIRE( "YWJj" == encstr);
        std::vector<uint8_t> dec_octets = jau::codec::base::decode64(encstr, aspec);
        REQUIRE( octets == dec_octets );
    }
    {
        std::vector<uint8_t> octets = { 'a', 'b', 'c', 'd' };
        std::string encstr = jau::codec::base::encode64(octets.data(), octets.size(), aspec);
        REQUIRE( "YWJjZA==" == encstr);
        std::vector<uint8_t> dec_octets = jau::codec::base::decode64(encstr, aspec);
        REQUIRE( octets == dec_octets );
    }
    {
        std::vector<uint8_t> octets = { 'a', 'b', 'c', 'd', 'e' };
        std::string encstr = jau::codec::base::encode64(octets.data(), octets.size(), aspec);
        REQUIRE( "YWJjZGU=" == encstr);
        std::vector<uint8_t> dec_octets = jau::codec::base::decode64(encstr, aspec);
        REQUIRE( octets == dec_octets );
    }
    {
        std::vector<uint8_t> octets = { 'a', 'b', 'c', 'd', 'e', 'f', 'g' };
        std::string encstr = jau::codec::base::encode64(octets.data(), octets.size(), aspec);
        REQUIRE( "YWJjZGVmZw==" == encstr);
        std::vector<uint8_t> dec_octets = jau::codec::base::decode64(encstr, aspec);
        REQUIRE( octets == dec_octets );
    }
    {
        // Test no-padding accept and error, double padding dropped '=='
        std::vector<uint8_t> octets = { 'a', 'b', 'c', 'd', 'e', 'f', 'g' };
        std::string encstr = jau::codec::base::encode64(octets.data(), octets.size(), aspec_nopadding);
        REQUIRE( "YWJjZGVmZw" == encstr);
        {
            // accept no padding
            std::vector<uint8_t> dec_octets = jau::codec::base::decode64(encstr, aspec_nopadding);
            REQUIRE( octets == dec_octets );
        }
        {
            // not accepting lack of padding
            std::vector<uint8_t> dec_octets = jau::codec::base::decode64(encstr, aspec);
            REQUIRE( dec_octets.empty() );
        }
    }
    {
        // Test no-padding accept and error, double padding dropped '=='
        std::vector<uint8_t> octets = { 'a' };
        std::string encstr = jau::codec::base::encode64(octets.data(), octets.size(), aspec_nopadding);
        REQUIRE( "YQ" == encstr);
        {
            // accept no padding
            std::vector<uint8_t> dec_octets = jau::codec::base::decode64(encstr, aspec_nopadding);
            REQUIRE( octets == dec_octets );
        }
        {
            // not accepting lack of padding
            std::vector<uint8_t> dec_octets = jau::codec::base::decode64(encstr, aspec);
            REQUIRE( dec_octets.empty() );
        }
    }
    {
        // Test no-padding accept and error, single padding dropped '='
        std::vector<uint8_t> octets = { 'a', 'b', 'c', 'd', 'e' };
        std::string encstr = jau::codec::base::encode64(octets.data(), octets.size(), aspec_nopadding);
        REQUIRE( "YWJjZGU" == encstr);
        {
            // accept no padding
            std::vector<uint8_t> dec_octets = jau::codec::base::decode64(encstr, aspec_nopadding);
            REQUIRE( octets == dec_octets );
        }
        {
            // not accepting lack of padding
            std::vector<uint8_t> dec_octets = jau::codec::base::decode64(encstr, aspec);
            REQUIRE( dec_octets.empty() );
        }
    }
    {
        // Test no-padding accept and error, single padding dropped '='
        std::vector<uint8_t> octets = { 'a', 'b' };
        std::string encstr = jau::codec::base::encode64(octets.data(), octets.size(), aspec_nopadding);
        REQUIRE( "YWI" == encstr);
        {
            // accept no padding
            std::vector<uint8_t> dec_octets = jau::codec::base::decode64(encstr, aspec_nopadding);
            REQUIRE( octets == dec_octets );
        }
        {
            // not accepting lack of padding
            std::vector<uint8_t> dec_octets = jau::codec::base::decode64(encstr, aspec);
            REQUIRE( dec_octets.empty() );
        }
    }
    {
        // Test no-padding accept and error, zero padding dropped
        std::vector<uint8_t> octets = { 'a', 'b', 'c' };
        std::string encstr = jau::codec::base::encode64(octets.data(), octets.size(), aspec_nopadding);
        REQUIRE( "YWJj" == encstr);
        {
            // accept no padding
            std::vector<uint8_t> dec_octets = jau::codec::base::decode64(encstr, aspec_nopadding);
            REQUIRE( octets == dec_octets );
        }
        {
            // accept no padding
            std::vector<uint8_t> dec_octets = jau::codec::base::decode64(encstr, aspec);
            REQUIRE( octets == dec_octets );
        }
    }
    {
        std::string in_str = "aaaaaaaaaaaaaaaaa"; // a17
        std::vector<uint8_t> octets(in_str.begin(), in_str.end());
        std::string encstr = jau::codec::base::encode64(octets.data(), octets.size(), aspec);
        REQUIRE( "YWFhYWFhYWFhYWFhYWFhYWE=" == encstr);
        std::vector<uint8_t> dec_octets = jau::codec::base::decode64(encstr, aspec);
        REQUIRE( octets == dec_octets );
    }

    {
        // Test code-points 63 and 64 of base64
        std::vector<uint8_t> octets = { 0x03, 0xef, 0xff, 0xf9 };
        std::string encstr = jau::codec::base::encode64(octets.data(), octets.size(), aspec);
        REQUIRE( "A+//+Q==" == encstr);
        std::vector<uint8_t> dec_octets = jau::codec::base::decode64(encstr, aspec);
        REQUIRE( octets == dec_octets );
    }
    {
        // Test code-points 63 and 64 of base64url
        std::vector<uint8_t> octets = { 0x03, 0xef, 0xff, 0xf9 };
        std::string encstr = jau::codec::base::encode64(octets.data(), octets.size(), aspec_url);
        REQUIRE( "A-__-Q==" == encstr);
        std::vector<uint8_t> dec_octets = jau::codec::base::decode64(encstr, aspec_url);
        REQUIRE( octets == dec_octets );
    }

    {
        std::string in_str = "one two three four five six seven eight nine ten eleven twelve thirteen fourteen fivteen sixteen seventeen eighteen nineteen twenty twenty-one";
        std::string exp_encstr = "b25lIHR3byB0aHJlZSBmb3VyIGZpdmUgc2l4IHNldmVuIGVpZ2h0IG5pbmUgdGVuIGVsZXZlbiB0"
                                 "d2VsdmUgdGhpcnRlZW4gZm91cnRlZW4gZml2dGVlbiBzaXh0ZWVuIHNldmVudGVlbiBlaWdodGVl"
                                 "biBuaW5ldGVlbiB0d2VudHkgdHdlbnR5LW9uZQ==";
        std::vector<uint8_t> octets(in_str.begin(), in_str.end());
        std::string encstr = jau::codec::base::encode64(octets.data(), octets.size(), aspec);
        REQUIRE( exp_encstr == encstr);
        std::vector<uint8_t> dec_octets = jau::codec::base::decode64(encstr, aspec);
        REQUIRE( octets == dec_octets );
    }
    {
        std::string in_str = "one two three four five six seven eight nine ten eleven twelve thirteen fourteen fivteen sixteen seventeen eighteen nineteen twenty twenty-one";
        std::string exp_encstr = "b25lIHR3byB0aHJlZSBmb3VyIGZpdmUgc2l4IHNldmVuIGVpZ2h0IG5pbmUgdGVuIGVsZXZlbiB0\n"
                                 "d2VsdmUgdGhpcnRlZW4gZm91cnRlZW4gZml2dGVlbiBzaXh0ZWVuIHNldmVudGVlbiBlaWdodGVl\n"
                                 "biBuaW5ldGVlbiB0d2VudHkgdHdlbnR5LW9uZQ==";
        std::vector<uint8_t> octets(in_str.begin(), in_str.end());
        std::string encstr = jau::codec::base::encode64_mime(octets.data(), octets.size(), aspec);
        REQUIRE( exp_encstr == encstr);
        std::vector<uint8_t> dec_octets = jau::codec::base::decode64_lf(encstr, aspec);
        REQUIRE( octets == dec_octets );
    }
    {
        std::string in_str = "one two three four five six seven eight nine ten eleven twelve thirteen fourteen fivteen sixteen seventeen eighteen nineteen twenty twenty-one";
        std::string exp_encstr = "b25lIHR3byB0aHJlZSBmb3VyIGZpdmUgc2l4IHNldmVuIGVpZ2h0IG5pbmUgdGVu\n"
                                 "IGVsZXZlbiB0d2VsdmUgdGhpcnRlZW4gZm91cnRlZW4gZml2dGVlbiBzaXh0ZWVu\n"
                                 "IHNldmVudGVlbiBlaWdodGVlbiBuaW5ldGVlbiB0d2VudHkgdHdlbnR5LW9uZQ==";
        std::vector<uint8_t> octets(in_str.begin(), in_str.end());
        std::string encstr = jau::codec::base::encode64_pem(octets.data(), octets.size(), aspec);
        REQUIRE( exp_encstr == encstr);
        std::vector<uint8_t> dec_octets = jau::codec::base::decode64_lf(encstr, aspec);
        REQUIRE( octets == dec_octets );
    }

    // Erroneous coded string in decoding
    {
        std::string encstr = "!@#$%^&*()"; // non-alphebet error
        std::vector<uint8_t> dec_octets = jau::codec::base::decode64(encstr, aspec);
        REQUIRE( dec_octets.empty() );
    }

}

TEST_CASE( "Binary Base 64 Encoding Test 11", "[binary][type]" ) {
    testBinaryBase64();
}
