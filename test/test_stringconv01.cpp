/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020-2026 Gothel Software e.K.
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
#include <cstring>

#include "jau/base_math.hpp"
#include "test_datatype01.hpp"

#include <jau/basic_types.hpp>
#include <jau/cpp_lang_util.hpp>
#include <jau/string_cfmt.hpp>
#include <jau/string_util.hpp>
#include <jau/test/catch2_ext.hpp>
#include <jau/type_traits_queries.hpp>

typedef std::vector<int> std_vec_int;

typedef std_vec_int::iterator std_vec_int_iter;

typedef std_vec_int::const_iterator std_vec_int_citer;

typedef std_vec_int_citer::pointer std_vec_int_citer_pointer;

typedef decltype( std::declval<std_vec_int_citer>().operator->() ) std_vec_int_citer_ptrop_retval;

using namespace jau::int_literals;

template<typename value_type>
static void testDecTo(int line, value_type v, std::string_view exp_s,
                      const uint32_t min_width = 0, const char separator = 0)
{
    std::string has1_s = jau::to_string(v, 10, jau::LoUpCase::lower, jau::PrefixOpt::none, min_width, separator, ' ');
    std::string has2_s = jau::to_decstring(v, separator, min_width);
    std::cerr << "line " << line << ": v '" << v
              << ", exp_s '" << exp_s << "' (l " << exp_s.length()
              << "), has1_s '" << has1_s << "' (l " << has1_s.length() << ", c " << has1_s.capacity() << ", m " << (exp_s == has1_s)
              << "), has2_s '" << has2_s << "' (l " << has2_s.length() << ", c " << has2_s.capacity() << ", m " << (exp_s == has2_s)
              << ")\n";
    CHECK( exp_s.length() == has1_s.length() );
    CHECK( exp_s.length() == has2_s.length() );
    REQUIRE( exp_s == has1_s );
    REQUIRE( exp_s == has2_s );
}

template<typename value_type>
static void testTo(int line, value_type v, std::string_view exp_s,
                   uint32_t radix, jau::LoUpCase capitalization = jau::LoUpCase::lower, jau::PrefixOpt prefix = jau::PrefixOpt::prefix,
                   const uint32_t min_width = 0, const char separator = 0, const char padding = '0')
{
    if( radix == 10 && padding == ' ' ) {
        testDecTo(line, v, exp_s, min_width, separator);
    }
    std::string has1_s = jau::to_string(v, radix, capitalization, prefix, min_width, separator, padding);
    jau::cfmt::FormatOpts opts;
    bool use_cfm=false;
    {
        if (prefix == jau::PrefixOpt::prefix) {
            opts.addFlag('#');
        }
        if (padding == '0') {
            opts.addFlag('0');
        }
        if (separator == '\'' || separator == ',') {
            opts.addFlag(separator);
        }
        if( min_width > 0 ) {
            opts.setWidth(min_width);
        }
        if( sizeof(v) >= sizeof(uint64_t)) {
            opts.length_mod = jau::cfmt::plength_t::l;
        }
        if ( jau::is_positive(v)) {
            switch (radix) {
                case 16: use_cfm=true; opts.setConversion(capitalization==jau::LoUpCase::lower ? 'x' : 'X'); break;
                case 10: use_cfm=true; opts.setConversion('u'); break;
                case  8: use_cfm=true; opts.setConversion('o'); break;
                case  2: use_cfm=true; opts.setConversion('b'); break;
                default: break;
            }
        } else {
            if( radix==10) {
                use_cfm=true;
                opts.setConversion('d');
            }
        }
    }
    std::string fmt2 = opts.toFormat();
    std::string has2_s = use_cfm ? jau::cfmt::format(fmt2, v) : "";
    std::cerr << "line " << line << ": v '" << v << ", radix " << radix
              << ", exp_s '" << exp_s << "' (l " << exp_s.length()
              << "), has1_s '" << has1_s << "' (l " << has1_s.length() << ", c " << has1_s.capacity() << ", m " << (exp_s == has1_s) << ")"
              ;
    if(use_cfm) {
        std::cerr << ", has2_s '" << has2_s << "' (l " << has2_s.length() << ", c " << has2_s.capacity() << ", m " << (exp_s == has1_s)
                  << ", fmt2 '" << fmt2 << ", " << opts.toString() << ")";
    }
    std::cerr << "\n";
    CHECK( exp_s.length() == has1_s.length() );
    REQUIRE( exp_s == has1_s );
    if(use_cfm) {
        CHECK( exp_s.length() == has2_s.length() );
        REQUIRE( exp_s == has2_s );
    }
}
template<typename value_type>
static void testToFrom(int line, value_type exp_v, std::string_view exp_s, std::string_view in_s,
                       uint32_t radix=10, jau::LoUpCase capitalization = jau::LoUpCase::lower, jau::PrefixOpt prefix = jau::PrefixOpt::prefix,
                       const uint32_t min_width = 0, const char separator = 0, const char padding = '0')
{
    testTo(line, exp_v, exp_s, radix, capitalization, prefix, min_width, separator, padding);
    value_type v;
    auto [consumed, ok] = jau::fromIntString(v, in_s, radix, separator);
    std::cerr << "line " << line << ": exp_v " << exp_v << ", in_s '" << in_s << "', radix " << radix
              << ": ok " << ok << ", consumed " << consumed << " (match " << (consumed == in_s.length()) << "), value " << v << " (match " << (exp_v == v) << ")\n";
    REQUIRE( true == ok );
    REQUIRE( exp_v == v );
    // REQUIRE( exp_s.length() == consumed );
}
template<typename value_type>
static void testToFrom(int line, value_type exp_v, const std::string& exp_s,
                       uint32_t radix=10, jau::LoUpCase capitalization = jau::LoUpCase::lower, jau::PrefixOpt prefix = jau::PrefixOpt::prefix,
                       const uint32_t min_width = 0, const char separator = 0, const char padding = '0')
{
    testToFrom(line, exp_v, exp_s, exp_s, radix, capitalization, prefix, min_width, separator, padding);
}

template<typename value_type>
static void testFrom(int line, value_type exp_v, std::string_view in_s,uint32_t radix=10, const char separator = 0)
{
    value_type v;
    auto [consumed, ok] = jau::fromIntString(v, in_s, radix, separator);
    std::cerr << "line " << line << ": exp_v " << exp_v << ", in_s '" << in_s << "', radix " << radix
              << ": ok " << ok << ", consumed " << consumed << " (match " << (consumed == in_s.length()) << "), value " << v << " (match " << (exp_v == v) << ")\n";
    REQUIRE( true == ok );
    REQUIRE( exp_v == v );
    // REQUIRE( in_s.length() == consumed );
}

TEST_CASE( "Test 00 - to_string/appendIntString, fromIntString", "[jau][string][to_string][from_string]" ) {
    int i1 = 1;
    uint64_t u64_1 = 1116791496961ull;
    void * p_v_1 = (void *)0xAFFE;
    float float_1 = 1.65f;

    Addr48Bit addr48bit_1(u64_1);

    CHECK("1" == jau::to_string<int>(i1));
    CHECK("1116791496961" == jau::to_string(u64_1));
    CHECK("0xaffe" == jau::to_string(p_v_1));
    CHECK("0xaffe" == jau::toHexString(0xaffe_u32));
    {
        // radix, default: no-width, prefix, no-separator, no padding
        testToFrom(__LINE__, 0xdeadbeef_u32, "0xdeadbeef", 16);                               // hex
        testToFrom(__LINE__, 0xdeadbeef_u32, "0xdead'beef", "  0x'dead'beef la", 16, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 0, '\'');    // hex

        testToFrom(__LINE__, 876543210_u64, "876543210", 10);                      // dec
        testToFrom(__LINE__, 876543210_u64, "876'543'210", "  '876'543'210 la", 10, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 0, '\''); // dec

        testToFrom(__LINE__, 077652_u32, "077652", 8);                               // oct
        testToFrom(__LINE__, 077652_u32, "07'7652", "  07'7652 la", 8, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 0, '\'');              // oct

        testToFrom(__LINE__, 0b11010101101_u32, "0b110'1010'1101", "  0b'110'1010'1101 la", 2, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 0, '\''); // bin

        // no-prefix, radix, default: no-width, no-separator, no padding
        testToFrom(__LINE__, 0xaffe_u32, "affe", 16, jau::LoUpCase::lower, jau::PrefixOpt::none);                  // hex
        testToFrom(__LINE__, 0x1affe_u32, "1affe", 16, jau::LoUpCase::lower, jau::PrefixOpt::none);                // hex
        testToFrom(__LINE__, 876543210_u64, "876543210",   10, jau::LoUpCase::lower, jau::PrefixOpt::none);        // dec
        testToFrom(__LINE__, 1876543210_u64, "1876543210",   10, jau::LoUpCase::lower, jau::PrefixOpt::none);      // dec
        testToFrom(__LINE__, 043217652_u32, "43217652",   8, jau::LoUpCase::lower, jau::PrefixOpt::none);          // oct
        testToFrom(__LINE__, 0143217652_u32, "143217652",   8, jau::LoUpCase::lower, jau::PrefixOpt::none);        // oct
        testToFrom(__LINE__, 0b11010101101_u32, "11010101101", 2, jau::LoUpCase::lower, jau::PrefixOpt::none);     // bin
        testToFrom(__LINE__, 0b111010101101_u32, "111010101101", 2, jau::LoUpCase::lower, jau::PrefixOpt::none);   // bin

        // radix, width-expansion, default: prefix, no-separator, '0' padding
        testToFrom(__LINE__, 0xaffe_u32, "0x00affe", 16, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 8);                      // hex
        testToFrom(__LINE__, 0x1affe_u32, "0x01affe", 16, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 8);                     // hex
        testToFrom(__LINE__, 876543210_u64, "000876543210",   10, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 12);            // dec
        testToFrom(__LINE__, 1876543210_u64, "001876543210",   10, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 12);           // dec
        testToFrom(__LINE__, 043217652_u32, "0043217652",   8, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 10);               // oct
        testToFrom(__LINE__, 0143217652_u32, "0143217652",   8, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 10);              // oct
        testToFrom(__LINE__, 0b11010101101_u32, "0b00011010101101", 2, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 16);       // bin
        testToFrom(__LINE__, 0b111010101101_u32, "0b00111010101101", 2, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 16);      // bin

        // no-prefix, radix, width-expansion, default: no-separator, '0' padding
        testToFrom(__LINE__, 0xaffe_u32, "0000affe", 16, jau::LoUpCase::lower, jau::PrefixOpt::none, 8);                      // hex
        testToFrom(__LINE__, 0x1affe_u32, "0001affe", 16, jau::LoUpCase::lower, jau::PrefixOpt::none, 8);                     // hex
        testToFrom(__LINE__, 876543210_u64, "000876543210",   10, jau::LoUpCase::lower, jau::PrefixOpt::none, 12);            // dec
        testToFrom(__LINE__, 1876543210_u64, "001876543210",   10, jau::LoUpCase::lower, jau::PrefixOpt::none, 12);           // dec
        testToFrom(__LINE__, 043217652_u32, "0043217652",   8, jau::LoUpCase::lower, jau::PrefixOpt::none, 10);               // oct
        testToFrom(__LINE__, 0143217652_u32, "0143217652",   8, jau::LoUpCase::lower, jau::PrefixOpt::none, 10);              // oct
        testToFrom(__LINE__, 0b11010101101_u32, "0000011010101101", 2, jau::LoUpCase::lower, jau::PrefixOpt::none, 16);       // bin
        testToFrom(__LINE__, 0b111010101101_u32, "0000111010101101", 2, jau::LoUpCase::lower, jau::PrefixOpt::none, 16);      // bin

        // radix, separator, default: no-width, prefix, '0' padding
        testToFrom(__LINE__, 0xaffe_u32, "0xaffe", 16, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 0, '\'');                // hex
        testToFrom(__LINE__, 0x1affe_u32, "0x1'affe", 16, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 0, '\'');             // hex
        testToFrom(__LINE__, 876543210_u64, "876'543'210",   10, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 0, '\'');      // dec
        testToFrom(__LINE__, 1876543210_u64, "1'876'543'210",   10, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 0, '\'');   // dec
        testToFrom(__LINE__, 043217652_u32, "04321'7652",   8, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 0, '\'');        // oct
        testToFrom(__LINE__, 0143217652_u32, "01'4321'7652",   8, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 0, '\'');     // oct
        testToFrom(__LINE__, 0b10101101_u32, "0b1010'1101", 2, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 0, '\'');        // bin
        testToFrom(__LINE__, 0b110101101_u32, "0b1'1010'1101", 2, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 0, '\'');     // bin

        // no-prefix, radix, separator, default: no-width, '0' padding
        testToFrom(__LINE__, 0xaffe_u32, "affe", 16, jau::LoUpCase::lower, jau::PrefixOpt::none, 0, '\'');                  // hex
        testToFrom(__LINE__, 0x1affe_u32, "1'affe", 16, jau::LoUpCase::lower, jau::PrefixOpt::none, 0, '\'');               // hex
        testToFrom(__LINE__, 876543210_u64, "876'543'210",   10, jau::LoUpCase::lower, jau::PrefixOpt::none, 0, '\'');      // dec
        testToFrom(__LINE__, 1876543210_u64, "1'876'543'210",   10, jau::LoUpCase::lower, jau::PrefixOpt::none, 0, '\'');   // dec
        testToFrom(__LINE__, 043217652_u32, "4321'7652",   8, jau::LoUpCase::lower, jau::PrefixOpt::none, 0, '\'');         // oct
        testToFrom(__LINE__, 0143217652_u32, "1'4321'7652",   8, jau::LoUpCase::lower, jau::PrefixOpt::none, 0, '\'');      // oct
        testToFrom(__LINE__, 0b10101101_u32, "1010'1101", 2, jau::LoUpCase::lower, jau::PrefixOpt::none, 0, '\'');          // bin
        testToFrom(__LINE__, 0b110101101_u32, "1'1010'1101", 2, jau::LoUpCase::lower, jau::PrefixOpt::none, 0, '\'');       // bin

        // radix, width-expansion, separator, default: prefix, '0' padding
        testToFrom(__LINE__, 0xaffe_u32, "0xaffe", 16, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 6, '\'');   // hex
        testToFrom(__LINE__, 0xaffe_u32, "0x'affe", 16, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 7, '\'');  // hex
        testToFrom(__LINE__, 0xaffe_u32, "0x0'affe", 16, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 8, '\''); // hex

        testToFrom(__LINE__, 876543210_u64, "876'543'210",   10, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 11, '\''); // dec
        testToFrom(__LINE__, 876543210_u64, "'876'543'210",  10, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 12, '\''); // dec
        testToFrom(__LINE__, 876543210_u64, "0'876'543'210", 10, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 13, '\''); // dec

        testToFrom(__LINE__, 07652_u32, "07652",   8, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 5, '\'');   // oct
        testToFrom(__LINE__, 07652_u32, "0'7652",   8, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 6, '\'');  // oct
        testToFrom(__LINE__, 07652_u32, "00'7652",   8, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 7, '\''); // oct

        testToFrom(__LINE__, 0b111010101101_u32, "0b1110'1010'1101", 2, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 16, '\'');   // bin
        testToFrom(__LINE__, 0b111010101101_u32, "0b'1110'1010'1101", 2, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 17, '\'');  // bin
        testToFrom(__LINE__, 0b111010101101_u32, "0b0'1110'1010'1101", 2, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 18, '\''); // bin

        // no-prefix, radix, width-expansion, separator, default: '0' padding
        testToFrom(__LINE__, 0xaffe_u32, "affe", 16, jau::LoUpCase::lower, jau::PrefixOpt::none, 4, '\'');   // hex
        testToFrom(__LINE__, 0xaffe_u32, "'affe", 16, jau::LoUpCase::lower, jau::PrefixOpt::none, 5, '\'');  // hex
        testToFrom(__LINE__, 0xaffe_u32, "0'affe", 16, jau::LoUpCase::lower, jau::PrefixOpt::none, 6, '\''); // hex

        testToFrom(__LINE__, 876543210_u64, "876'543'210",   10, jau::LoUpCase::lower, jau::PrefixOpt::none, 11, '\''); // dec
        testToFrom(__LINE__, 876543210_u64, "'876'543'210",  10, jau::LoUpCase::lower, jau::PrefixOpt::none, 12, '\''); // dec
        testToFrom(__LINE__, 876543210_u64, "0'876'543'210", 10, jau::LoUpCase::lower, jau::PrefixOpt::none, 13, '\''); // dec

        testToFrom(__LINE__, 07652_u32, "7652",   8, jau::LoUpCase::lower, jau::PrefixOpt::none, 4, '\'');   // oct
        testToFrom(__LINE__, 07652_u32, "'7652",   8, jau::LoUpCase::lower, jau::PrefixOpt::none, 5, '\'');  // oct
        testToFrom(__LINE__, 07652_u32, "0'7652",   8, jau::LoUpCase::lower, jau::PrefixOpt::none, 6, '\''); // oct

        testToFrom(__LINE__, 0b111010101101_u32, "1110'1010'1101", 2, jau::LoUpCase::lower, jau::PrefixOpt::none, 14, '\'');   // bin
        testToFrom(__LINE__, 0b111010101101_u32, "'1110'1010'1101", 2, jau::LoUpCase::lower, jau::PrefixOpt::none, 15, '\'');  // bin
        testToFrom(__LINE__, 0b111010101101_u32, "0'1110'1010'1101", 2, jau::LoUpCase::lower, jau::PrefixOpt::none, 16, '\''); // bin

        // Also testing to_decstring() due to radix==10 and padding==' '

        //
        // a.b.c radix, no-width, space padding ' ', [prefix], [separator], [signed]
        //     |
        //     0 - unsigned
        //     1 - signed
        //   |
        //   0 = no-separator
        //   1 = separator
        // |
        // 0 - no-prefix,
        // 1 - prefix,

        // 0.0.0 unsigned, no-prefix, radix, space padding ' ', default: no-width, no-separator
        testToFrom(__LINE__, 0xaffe_u32, "affe", 16, jau::LoUpCase::lower, jau::PrefixOpt::none, 0, 0, ' ');   // hex
        testToFrom(__LINE__, 876543210_u64,  "876543210",   10, jau::LoUpCase::lower, jau::PrefixOpt::none, 0, 0, ' '); // dec
        testToFrom(__LINE__, 077652_u32, "77652",   8, jau::LoUpCase::lower, jau::PrefixOpt::none, 0, 0, ' ');   // oct
        testToFrom(__LINE__, 0b11010101101_u32, "11010101101", 2, jau::LoUpCase::lower, jau::PrefixOpt::none, 0, 0, ' ');   // bin

        // 0.0.1 signed, no-prefix, radix, space padding ' ', default: no-width, no-separator
        testToFrom(__LINE__, -0xaffe_i32, "-affe", 16, jau::LoUpCase::lower, jau::PrefixOpt::none, 0, 0, ' ');   // hex
        testToFrom(__LINE__, -876543210_i64, "-876543210",   10, jau::LoUpCase::lower, jau::PrefixOpt::none, 0, 0, ' '); // dec
        testToFrom(__LINE__, -077652_i32, "-77652",   8, jau::LoUpCase::lower, jau::PrefixOpt::none, 0, 0, ' ');   // oct
        testToFrom(__LINE__, -0b11010101101_i32, "-11010101101", 2, jau::LoUpCase::lower, jau::PrefixOpt::none, 0, 0, ' ');   // bin

        // 0.1.0 unsigned, no-prefix, radix, separator, space padding ' ', default: no-width
        testToFrom(__LINE__, 0xaffe_u32, "affe", 16, jau::LoUpCase::lower, jau::PrefixOpt::none, 0, '\'', ' ');   // hex
        testToFrom(__LINE__, 876543210_u64, "876'543'210",   10, jau::LoUpCase::lower, jau::PrefixOpt::none, 0, '\'', ' '); // dec
        testToFrom(__LINE__, 077652_u32, "7'7652",   8, jau::LoUpCase::lower, jau::PrefixOpt::none, 0, '\'', ' ');   // oct
        testToFrom(__LINE__, 0b11010101101_u32, "110'1010'1101", 2, jau::LoUpCase::lower, jau::PrefixOpt::none, 0, '\'', ' ');   // bin

        // 0.1.1 signed, no-prefix, radix, separator, space padding ' ', default: no-width
        testToFrom(__LINE__, -0xaffe_i32, "-affe", 16, jau::LoUpCase::lower, jau::PrefixOpt::none, 0, '\'', ' ');   // hex
        testToFrom(__LINE__, -876543210_i64, "-876'543'210",   10, jau::LoUpCase::lower, jau::PrefixOpt::none, 0, '\'', ' '); // dec
        testToFrom(__LINE__, -077652_i32, "-7'7652",   8, jau::LoUpCase::lower, jau::PrefixOpt::none, 0, '\'', ' ');   // oct
        testToFrom(__LINE__, -0b11010101101_i32, "-110'1010'1101", 2, jau::LoUpCase::lower, jau::PrefixOpt::none, 0, '\'', ' ');   // bin

        // 1.0.0 unsigned, radix, space padding ' ', default: prefix,  no-width, no-separator
        testToFrom(__LINE__, 0xaffe_u32, "0xaffe", 16, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 0, 0, ' ');   // hex
        testToFrom(__LINE__, 876543210_u64,  "876543210",   10, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 0, 0, ' '); // dec
        testToFrom(__LINE__, 077652_u32, "077652",   8, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 0, 0, ' ');   // oct
        testToFrom(__LINE__, 0b11010101101_u32, "0b11010101101", 2, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 0, 0, ' ');   // bin

        // 1.0.1 signed, radix, space padding ' ', default: prefix,  no-width, no-separator
        testToFrom(__LINE__, -0xaffe_i32, "-0xaffe", 16, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 0, 0, ' ');   // hex
        testToFrom(__LINE__, -876543210_i64, "-876543210",   10, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 0, 0, ' '); // dec
        testToFrom(__LINE__, -077652_i32, "-077652",   8, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 0, 0, ' ');   // oct
        testToFrom(__LINE__, -0b11010101101_i32, "-0b11010101101", 2, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 0, 0, ' ');   // bin

        // 1.1.0 unsigned, radix, separator, space padding ' ', default: prefix,  no-width
        testToFrom(__LINE__, 0xaffe_u32, "0xaffe", 16, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 0, '\'', ' ');   // hex
        testToFrom(__LINE__, 876543210_u64, "876'543'210",   10, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 0, '\'', ' '); // dec
        testToFrom(__LINE__, 077652_u32, "07'7652",   8, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 0, '\'', ' ');   // oct
        testToFrom(__LINE__, 0b11010101101_u32, "0b110'1010'1101", 2, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 0, '\'', ' ');   // bin

        // 1.1.1 signed, radix, separator, space padding ' ', default: prefix,  no-width
        testToFrom(__LINE__, -0xaffe_i32, "-0xaffe", 16, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 0, '\'', ' ');   // hex
        testToFrom(__LINE__, -876543210_i64, "-876'543'210",   10, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 0, '\'', ' '); // dec
        testToFrom(__LINE__, -077652_i32, "-07'7652",   8, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 0, '\'', ' ');   // oct
        testToFrom(__LINE__, -0b11010101101_i32, "-0b110'1010'1101", 2, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 0, '\'', ' ');   // bin

        //
        // a.b.c radix, width-expansion, space padding ' ', [prefix], [separator], [signed]
        //     |
        //     0 - unsigned
        //     1 - signed
        //   |
        //   0 = no-separator
        //   1 = separator
        // |
        // 0 - no-prefix,
        // 1 - prefix,

        // 0.0.0 unsigned, no-prefix, radix, width-expansion, space padding ' ', default: no-separator
        testToFrom(__LINE__, 0xaffe_u32, "    affe", 16, jau::LoUpCase::lower, jau::PrefixOpt::none, 8, 0, ' ');   // hex
        testToFrom(__LINE__, 876543210_u64,  "      876543210",   10, jau::LoUpCase::lower, jau::PrefixOpt::none, 15, 0, ' '); // dec
        testToFrom(__LINE__, 077652_u32, "     77652",   8, jau::LoUpCase::lower, jau::PrefixOpt::none, 10, 0, ' ');   // oct
        testToFrom(__LINE__, 0b11010101101_u32, "      11010101101", 2, jau::LoUpCase::lower, jau::PrefixOpt::none, 17, 0, ' ');   // bin

        // 0.0.1 signed, no-prefix, radix, width-expansion, space padding ' ', default: no-separator
        testToFrom(__LINE__, -0xaffe_i32, "   -affe", 16, jau::LoUpCase::lower, jau::PrefixOpt::none, 8, 0, ' ');   // hex
        testToFrom(__LINE__, -876543210_i64, "     -876543210",   10, jau::LoUpCase::lower, jau::PrefixOpt::none, 15, 0, ' '); // dec
        testToFrom(__LINE__, -077652_i32, "    -77652",   8, jau::LoUpCase::lower, jau::PrefixOpt::none, 10, 0, ' ');   // oct
        testToFrom(__LINE__, -0b11010101101_i32, "     -11010101101", 2, jau::LoUpCase::lower, jau::PrefixOpt::none, 17, 0, ' ');   // bin

        // 0.1.0 unsigned, no-prefix, radix, width-expansion, separator, space padding ' '
        testToFrom(__LINE__, 0xaffe_u32, "    affe", 16, jau::LoUpCase::lower, jau::PrefixOpt::none, 8, '\'', ' ');   // hex
        testToFrom(__LINE__, 876543210_u64, "    876'543'210",   10, jau::LoUpCase::lower, jau::PrefixOpt::none, 15, '\'', ' '); // dec
        testToFrom(__LINE__, 077652_u32, "    7'7652",   8, jau::LoUpCase::lower, jau::PrefixOpt::none, 10, '\'', ' ');   // oct
        testToFrom(__LINE__, 0b11010101101_u32, "    110'1010'1101", 2, jau::LoUpCase::lower, jau::PrefixOpt::none, 17, '\'', ' ');   // bin

        // 0.1.1 signed, no-prefix, radix, width-expansion, separator, space padding ' '
        testToFrom(__LINE__, -0xaffe_i32, "   -affe", 16, jau::LoUpCase::lower, jau::PrefixOpt::none, 8, '\'', ' ');   // hex
        testToFrom(__LINE__, -876543210_i64, "   -876'543'210",   10, jau::LoUpCase::lower, jau::PrefixOpt::none, 15, '\'', ' '); // dec
        testToFrom(__LINE__, -077652_i32, "   -7'7652",   8, jau::LoUpCase::lower, jau::PrefixOpt::none, 10, '\'', ' ');   // oct
        testToFrom(__LINE__, -0b11010101101_i32, "   -110'1010'1101", 2, jau::LoUpCase::lower, jau::PrefixOpt::none, 17, '\'', ' ');   // bin

        // 1.0.0 unsigned, radix, width-expansion, space padding ' ', default: prefix,  no-separator
        testToFrom(__LINE__, 0xaffe_u32, "  0xaffe", 16, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 8, 0, ' ');   // hex
        testToFrom(__LINE__, 876543210_u64,  "      876543210",   10, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 15, 0, ' '); // dec
        testToFrom(__LINE__, 077652_u32, "    077652",   8, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 10, 0, ' ');   // oct
        testToFrom(__LINE__, 0b11010101101_u32, "    0b11010101101", 2, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 17, 0, ' ');   // bin

        // 1.0.1 signed, radix, width-expansion, space padding ' ', default: prefix,  no-separator
        testToFrom(__LINE__, -0xaffe_i32, " -0xaffe", 16, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 8, 0, ' ');   // hex
        testToFrom(__LINE__, -876543210_i64, "     -876543210",   10, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 15, 0, ' '); // dec
        testToFrom(__LINE__, -077652_i32, "   -077652",   8, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 10, 0, ' ');   // oct
        testToFrom(__LINE__, -0b11010101101_i32, "   -0b11010101101", 2, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 17, 0, ' ');   // bin

        // 1.1.0 unsigned, radix, width-expansion, separator, space padding ' '
        testToFrom(__LINE__, 0xaffe_u32, "  0xaffe", 16, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 8, '\'', ' ');   // hex
        testToFrom(__LINE__, 876543210_u64, "    876'543'210",   10, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 15, '\'', ' '); // dec
        testToFrom(__LINE__, 077652_u32, "   07'7652",   8, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 10, '\'', ' ');   // oct
        testToFrom(__LINE__, 0b11010101101_u32, "  0b110'1010'1101", 2, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 17, '\'', ' ');   // bin

        // 1.1.1 signed, radix, width-expansion, separator, space padding ' '
        testToFrom(__LINE__, -0xaffe_i32, " -0xaffe", 16, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 8, '\'', ' ');   // hex
        testToFrom(__LINE__, -876543210_i64, "   -876'543'210",   10, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 15, '\'', ' '); // dec
        testToFrom(__LINE__, -077652_i32, "  -07'7652",   8, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 10, '\'', ' ');   // oct
        testToFrom(__LINE__, -0b11010101101_i32, " -0b110'1010'1101", 2, jau::LoUpCase::lower, jau::PrefixOpt::prefix, 17, '\'', ' ');   // bin

    }
    CHECK("1.650000" == jau::to_string(float_1));

    CHECK("01:04:05:F5:E1:01" == jau::to_string(addr48bit_1));

    //
    // Validating 'pointer std::vector::const_iterator.operator->()'
    // and the to_string type trait logic of it.
    //

    // jau::type_cue<std_vec_int_citer>::print("std_vec_int_citer", jau::TypeTraitGroup::ALL);
    // jau::type_cue<std_vec_int_citer_pointer>::print("std_vec_int_citer_pointer", jau::TypeTraitGroup::ALL);

    // jau::type_cue<std_vec_int_citer_ptrop_retval>::print("std_vec_int_citer_ptrop_retval", jau::TypeTraitGroup::ALL);
    printf("jau::has_member_of_pointer<std_vec_int_citer>) %d\n", jau::has_member_of_pointer<std_vec_int_citer>::value);

    std_vec_int vec_int_1;
    vec_int_1.push_back(1); vec_int_1.push_back(2); vec_int_1.push_back(3);
    std_vec_int_citer vec_int_citer_1B = vec_int_1.cbegin();
    uint8_t* vec_int_citer_1B_ptr = (uint8_t*)(vec_int_citer_1B.operator->());
    std::string vec_int_citer_1B_str = jau::toHexString(vec_int_citer_1B_ptr);

    std_vec_int_citer vec_int_citer_1E = vec_int_1.cend();
    uint8_t* vec_int_citer_1E_ptr = (uint8_t*)(vec_int_citer_1E.operator->());
    std::string vec_int_citer_1E_str = jau::toHexString(vec_int_citer_1E_ptr);

    std::ptrdiff_t vec_int_citer_1E_1B_ptrdiff = vec_int_citer_1E_ptr - vec_int_citer_1B_ptr;
    size_t vec_int_citer_1E_1B_ptr_count = vec_int_citer_1E_1B_ptrdiff / sizeof(int);
    size_t vec_int_citer_1E_1B_itr_count = vec_int_citer_1E - vec_int_citer_1B;

    printf("vec_int_citer_1E - vec_int_citer_1B = itr_count %zu, ptr_count %zu\n",
           vec_int_citer_1E_1B_itr_count, vec_int_citer_1E_1B_ptr_count);
    printf("vec_int_citer_1E - vec_int_citer_1B = %zu\n", vec_int_citer_1E_1B_itr_count);
    printf("vec_int_citer_1B_ptr %s, vec_int_citer_1E1_ptr = %s\n", vec_int_citer_1B_str.c_str(), vec_int_citer_1E_str.c_str());

    CHECK(vec_int_citer_1E_1B_itr_count == 3);
    CHECK(vec_int_citer_1E_1B_itr_count == vec_int_citer_1E_1B_ptr_count);

    CHECK(vec_int_citer_1E_str == jau::to_string(vec_int_citer_1E));
}

#if 0
TEST_CASE( "Test 01 - to_string(radix)", "[jau][string][to_string(radix)]" ) {
    REQUIRE( true == true );
}

TEST_CASE( "Test 02 - toHexString()", "[jau][string][toHexString]" ) {
    REQUIRE( true == true );
}
#endif

static void testToBitString(std::string_view prefix, std::string_view exp_be_s, const uint64_t &exp_be_v, size_t max_bits, bool check_value=true) {
    std::cout << prefix << ": max_bits " << max_bits << "\n";
    std::string has_be_s1 = jau::toBitString(exp_be_v, jau::bit_order_t::msb, jau::PrefixOpt::none, max_bits);
    std::cout << "  exp_be_s : " << exp_be_s << "\n";
    std::cout << "  has_be_s1: " << has_be_s1 << "\n";
    REQUIRE( exp_be_s == has_be_s1 );

    if( check_value ) {
        const auto [has_be_v, len_be, ok_be] = jau::fromBitString(exp_be_s);
        REQUIRE(true == ok_be);
        REQUIRE(exp_be_s.size() == len_be);
        std::string has_be_s2 = jau::toBitString(has_be_v, jau::bit_order_t::msb, jau::PrefixOpt::none, max_bits);
        std::cout << "  has_be_s2: " << has_be_s2 << "\n";
        REQUIRE(exp_be_v == has_be_v);
    }
}
static void testToBitString(std::string_view prefix, std::string_view s_be1, const uint32_t &v_be1) {
    testToBitString(prefix, s_be1, v_be1, s_be1.size(), true);
}

TEST_CASE( "Test 03 - toBitString()", "[jau][string][toBitString]" ) {
    {
        testToBitString("Test 03.01.01", "000101100101110111011001",         0b101100101110111011001_u64, 0);
        testToBitString("Test 03.01.02", "000101100101110111011001",         0b101100101110111011001_u64);
        testToBitString("Test 03.01.03", "101110111011001",                  0b101100101110111011001_u64, 15, false);
        testToBitString("Test 03.01.04", "00000000000101100101110111011001",  0b101100101110111011001_u64);
        testToBitString("Test 03.01.05", "000000000000101100101110111011001", 0b101100101110111011001_u64, 33);

        testToBitString("Test 03.02.01", "11011001011101110110011110001101", 0b11011001011101110110011110001101_u64, 0);
        testToBitString("Test 03.02.02", "11011001011101110110011110001101", 0b11011001011101110110011110001101_u64, 32);
        testToBitString("Test 03.02.03", "01011001011101110110011110001101", 0b01011001011101110110011110001101_u64, 0);
        testToBitString("Test 03.02.04", "01011001011101110110011110001101", 0b01011001011101110110011110001101_u64, 32);
        testToBitString("Test 03.02.05", "0101110111011001",                 0b0101100101110111011001_u64, 16, false);

        testToBitString("Test 03.03.01", "1101100101110111011001111000110111011001011101110110011110001101",
                                        0b1101100101110111011001111000110111011001011101110110011110001101_u64, 0);
        testToBitString("Test 03.03.02", "1101100101110111011001111000110111011001011101110110011110001101",
                                        0b1101100101110111011001111000110111011001011101110110011110001101_u64, 64);

        testToBitString("Test 03.03.03", "0101100101110111011001111000110111011001011101110110011110001101",
                                        0b0101100101110111011001111000110111011001011101110110011110001101_u64, 0);
        testToBitString("Test 03.03.04", "0101100101110111011001111000110111011001011101110110011110001101",
                                        0b0101100101110111011001111000110111011001011101110110011110001101_u64, 64);

        testToBitString("Test 03.03.05", "0001100101110111011001111000110111011001011101110110011110001101",
                                        0b0001100101110111011001111000110111011001011101110110011110001101_u64, 0);
        testToBitString("Test 03.03.06", "0001100101110111011001111000110111011001011101110110011110001101",
                                        0b0001100101110111011001111000110111011001011101110110011110001101_u64, 64);

        testToBitString("Test 03.03.07", "1111111111101010111101101011111000000000000000000000000000000000",
                                         0b1111111111101010111101101011111000000000000000000000000000000000_u64, 0);
        testToBitString("Test 03.03.08", "1111111111101010111101101011111000000000000000000000000000000000",
                                         0b1111111111101010111101101011111000000000000000000000000000000000_u64, 64);

        testToBitString("Test 03.03.09", "11111110101001111110101011110110",
                                         0b0000000000000000000000000000000011111110101001111110101011110110_u64, 0);
        testToBitString("Test 03.03.10", "0000000000000000000000000000000011111110101001111110101011110110",
                                         0b0000000000000000000000000000000011111110101001111110101011110110_u64, 64);
        testToBitString("Test 03.03.11", "011111110101001111110101011110110",
                                         0b0000000000000000000000000000000011111110101001111110101011110110_u64, 33);

        testToBitString("Test 03.03.12", "00000000",
                                         0b0000000000000000000000000000000000000000000000000000000000000000_u64, 0);
        testToBitString("Test 03.03.13", "0000000000000000000000000000000000000000000000000000000000000000",
                                         0b0000000000000000000000000000000000000000000000000000000000000000_u64, 64);
    }
}

