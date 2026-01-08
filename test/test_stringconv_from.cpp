/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2025 Gothel Software e.K.
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

static void testToFrom(uint64_t exp_v, std::string_view exp_s) {
    REQUIRE(exp_s == jau::to_string(exp_v, 10, jau::LoUpCase::lower, jau::PrefixOpt::none));
    uint64_t v;
    REQUIRE( true == jau::from_chars(v, exp_s) );
    REQUIRE( exp_v == v );
}

template<typename value_type>
static constexpr value_type testFrom(std::string_view exp_s) {
    value_type v;
    if( jau::from_chars(v, exp_s) ) {
        return v;
    }
    std::cerr << "from_chars " << exp_s << " -> " << v << "\n";
    return 0;
}

template<typename value_type>
struct DataFromTo01 {
    std::string_view from;
    value_type       to;
};

TEST_CASE( "Test 01 - from_chars()", "[jau][string][toBitString]" ) {
    {
        int64_t v;
        REQUIRE(false == jau::from_chars(v, "-9223372036854775808888888") );
        REQUIRE(false == jau::from_chars(v, "9223372036854775808888888") );
    }
    {
        REQUIRE( -1 == testFrom<int64_t>("-1"));
        REQUIRE(  9 == testFrom<int64_t>("09.10"));

        // NOLINTBEGIN
        DataFromTo01<int64_t> data[] = {
            {"0", 0}, {"1", 1}, {"2", 2}, {"3", 3}, {"4", 4}, {"5", 5}, {"6", 6}, {"7", 7}, {"8", 8}, {"9", 9},
                     {"-1", -1}, {"-2", -2}, {"-3", -3}, {"-4", -4}, {"-5", -5}, {"-6", -6}, {"-7", -7}, {"-8", -8}, {"-9", -9},
            {"10", 10}, {"-10", -10}, {"123", 123}, {"-123", -123}, {"65432", 65432}, {"-65432", -65432},
            {" -9223372036854775808 ", std::numeric_limits<int64_t>::min() },
            {"  9223372036854775807 ", std::numeric_limits<int64_t>::max() }
        };
        // NOLINTEND

        for(auto d : data) {
            REQUIRE( d.to == testFrom<int64_t>(d.from));
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
            REQUIRE( d.to == testFrom<uint64_t>(d.from));
        }
        for(auto d : data) {
            testToFrom(d.to, d.from);
        }
    }
}


