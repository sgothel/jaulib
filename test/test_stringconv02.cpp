/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2025-2026 Gothel Software e.K.
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
#include <limits>

#include <jau/basic_types.hpp>
#include <jau/cpp_lang_util.hpp>
#include <jau/string_cfmt.hpp>
#include <jau/string_util.hpp>
#include <jau/test/catch2_ext.hpp>
#include <jau/type_traits_queries.hpp>

using namespace jau::int_literals;

template<typename value_type>
static void testTo(int line, value_type v, std::string_view exp_s,
                   uint32_t radix = 10, jau::LoUpCase capitalization = jau::LoUpCase::lower, jau::PrefixOpt prefix = jau::PrefixOpt::prefix,
                   const uint32_t min_width = 0, const char separator = 0, const char padding = '0')
{
    std::string has_s = jau::to_string(v, radix, capitalization, prefix, min_width, separator, padding);
    std::cerr << "line " << line << ": v '" << v << ", radix " << radix
              << ", exp_s '" << exp_s << "' (l " << exp_s.length()
              << "), has_s '" << has_s << "' (l " << has_s.length() << ", c " << has_s.capacity()
              << "), match " << (exp_s == has_s) << "\n";
    CHECK( exp_s.length() == has_s.length() );
    REQUIRE( exp_s == has_s );
}
template<typename value_type>
static void testToFrom(int line, value_type exp_v, std::string_view exp_s, std::string_view in_s,
                       uint32_t radix = 10, jau::LoUpCase capitalization = jau::LoUpCase::lower, jau::PrefixOpt prefix = jau::PrefixOpt::prefix,
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
struct DataFromTo01 {
    std::string_view from;
    value_type       to;
};

TEST_CASE( "Test 01 - from_chars()", "[jau][string][toBitString]" ) {
    {
        int64_t v;
        REQUIRE(false == jau::fromIntString(v, "").b ); // empty
        REQUIRE(true == jau::fromIntString(v, " 123").b ); // leading space
        REQUIRE(true == jau::fromIntString(v, "123  ").b ); // space tail OK
        REQUIRE(123 == v);
        REQUIRE(false == jau::fromIntString(v, "XXDK123").b ); // leading garbage
        REQUIRE(true == jau::fromIntString(v, "123SJKXNC").b ); // garbage tail OK
        REQUIRE(123 == v);
        REQUIRE(false == jau::fromIntString(v, "-9223372036854775808888888").b ); // underflow
        REQUIRE(false == jau::fromIntString(v, "9223372036854775808888888").b );  // overflow
    }
    {
        testToFrom<int64_t>(__LINE__, -1, "-1", "-1");
        testToFrom<int64_t>(__LINE__, 9, "9", "09.10");

        // NOLINTBEGIN
        DataFromTo01<int64_t> data[] = {
            {"0", 0}, {"1", 1}, {"2", 2}, {"3", 3}, {"4", 4}, {"5", 5}, {"6", 6}, {"7", 7}, {"8", 8}, {"9", 9},
                     {"-1", -1}, {"-2", -2}, {"-3", -3}, {"-4", -4}, {"-5", -5}, {"-6", -6}, {"-7", -7}, {"-8", -8}, {"-9", -9},
            {"10", 10}, {"-10", -10}, {"123", 123}, {"-123", -123}, {"65432", 65432}, {"-65432", -65432},
            {"-9223372036854775808", std::numeric_limits<int64_t>::min() },
            {"9223372036854775807", std::numeric_limits<int64_t>::max() }
        };
        // NOLINTEND

        for(auto d : data) {
            testToFrom(__LINE__, d.to, d.from, d.from);
        }
    }
    {
        // NOLINTBEGIN
        DataFromTo01<uint64_t> data[] = {
            {"0", 0}, {"1", 1}, {"2", 2}, {"3", 3}, {"4", 4}, {"5", 5}, {"6", 6}, {"7", 7}, {"8", 8}, {"9", 9},
            {"10", 10}, {"123", 123}, {"65432", 65432},
            { "9223372036854775807", std::numeric_limits<int64_t>::max() },
            {"18446744073709551615", std::numeric_limits<uint64_t>::max() }
        };
        // NOLINTEND

        for(auto d : data) {
            testToFrom(__LINE__, d.to, d.from, d.from);
        }
    }
}


