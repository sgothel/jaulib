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
#include <cassert>
#include <cinttypes>
#include <cstring>

#include <jau/test/catch2_ext.hpp>

#include <jau/basic_types.hpp>

using namespace jau;

#define SHOW_DECIMAL_STRING_STATS 0

#if SHOW_DECIMAL_STRING_STATS
    template<class T>
    void show_decimal_string_stats(const std::string msg, const T value, const bool use_separator, const int min_width) {
        const nsize_t max_digits10 = std::numeric_limits<T>::is_signed ?
                jau::digits10<T>(std::numeric_limits<T>::min()) :
                jau::digits10<T>(std::numeric_limits<T>::max());

        const nsize_t max_digits10_0 = std::numeric_limits<T>::digits10;
        const T max_value = std::numeric_limits<T>::max();
        const T min_value = std::numeric_limits<T>::min();
        const nsize_t max_digits10_1 = jau::digits10<T>(min_value);
        const nsize_t max_digits10_2 = jau::digits10<T>(max_value);

        const nsize_t max_commas = use_separator ? ( max_digits10 - 1 ) / 3 : 0;
        const nsize_t max_chars = max_digits10 + max_commas;

        const nsize_t digit10_count = jau::digits10<T>(value);

        const nsize_t comma_count = use_separator ? ( digit10_count - 1 ) / 3 : 0;
        const nsize_t net_chars = digit10_count + comma_count;
        const nsize_t total_chars = std::min<nsize_t>(max_chars, std::max<nsize_t>(min_width, net_chars));

        INFO(msg+": value "+std::to_string(value)+", use_separator "+std::to_string(use_separator)+", min_width "+std::to_string(min_width));
        INFO(msg+": min "+std::to_string(min_value)+", max "+std::to_string(max_value));
        INFO(msg+": max_digits10      "+std::to_string(max_digits10)+" [ orig "+std::to_string(max_digits10_0)+", min "+std::to_string(max_digits10_1)+", max "+std::to_string(max_digits10_2)+"]");
        INFO(msg+": max_commas        "+std::to_string(max_commas));
        INFO(msg+": max_chars         "+std::to_string(max_chars));
        INFO(msg+": value digits10    "+std::to_string(digit10_count));
        INFO(msg+": value commas      "+std::to_string(comma_count));
        INFO(msg+": value net_chars   "+std::to_string(net_chars));
        INFO(msg+": value total_chars "+std::to_string(total_chars));
        std::string s = to_decstring<T>(value, use_separator ? ',' : 0, min_width);
        INFO(msg+": result           '"+s+"', len "+std::to_string(s.length()));
    }
#endif

static void test_int32_t(const std::string msg, const int32_t v, const size_t expStrLen, const std::string expStr) {
#if SHOW_DECIMAL_STRING_STATS
    show_decimal_string_stats<int32_t>(msg, v, true /* use_separator */, 0 /* min_width */);
#endif
    const std::string str = to_decstring(v);
    INFO(msg+": has '"+str+"', len "+std::to_string(str.length()));
    INFO(msg+": exp '"+expStr+"', len "+std::to_string(expStr.length())+", equal: "+std::to_string(str==expStr));
    REQUIRE(str.length() == expStrLen);
    REQUIRE_THAT(str, Catch::Matchers::Equals(expStr, Catch::CaseSensitive::Yes));
}

static void test_uint32_t(const std::string msg, const uint32_t v, const size_t expStrLen, const std::string expStr) {
#if SHOW_DECIMAL_STRING_STATS
    show_decimal_string_stats<uint32_t>(msg, v, true /* use_separator */, 0 /* min_width */);
#endif

    const std::string str = to_decstring(v);
    INFO(msg+": has '"+str+"', len "+std::to_string(str.length()));
    INFO(msg+": exp '"+expStr+"', len "+std::to_string(expStr.length())+", equal: "+std::to_string(str==expStr));
    REQUIRE(str.length() == expStrLen);
    REQUIRE_THAT(str, Catch::Matchers::Equals(expStr, Catch::CaseSensitive::Yes));
}

static void test_uint64_t(const std::string msg, const uint64_t v, const size_t expStrLen, const std::string expStr) {
#if SHOW_DECIMAL_STRING_STATS
    show_decimal_string_stats<uint64_t>(msg, v, true /* use_separator */, 0 /* min_width */);
#endif

    const std::string str = to_decstring(v);
    INFO(msg+": has '"+str+"', len "+std::to_string(str.length()));
    INFO(msg+": exp '"+expStr+"', len "+std::to_string(expStr.length())+", equal: "+std::to_string(str==expStr));
    REQUIRE(str.length() == expStrLen);
    REQUIRE_THAT(str, Catch::Matchers::Equals(expStr, Catch::CaseSensitive::Yes));
}

TEST_CASE( "Integer Decimal String Test 01", "[datatype][int_dec_string]" ) {
    test_int32_t("INT32_MIN", INT32_MIN, 14, "-2,147,483,648");
    test_int32_t("int32_t -thousand", -1000, 6, "-1,000");
    test_int32_t("int32_t one", 1, 1, "1");
    test_int32_t("int32_t thousand", 1000, 5, "1,000");
    test_int32_t("INT32_MAX", INT32_MAX, 13, "2,147,483,647");

    test_uint32_t("UINT32_MIN", 0, 1, "0");
    test_uint32_t("uint32_t one", 1, 1, "1");
    test_uint32_t("uint32_t thousand", 1000, 5, "1,000");
    test_uint32_t("UINT32_MAX", UINT32_MAX, 13, "4,294,967,295");

    test_uint64_t("UINT64_MIN", 0, 1, "0");
    test_uint64_t("uint64_t one", 1, 1, "1");
    test_uint64_t("uint64_t thousand", 1000, 5, "1,000");
    test_uint64_t("UINT64_MAX", UINT64_MAX, 26, "18,446,744,073,709,551,615");
}


