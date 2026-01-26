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

#include <jau/basic_types.hpp>
#include <jau/cpp_lang_util.hpp>
#include <jau/string_literal.hpp>
#include <jau/test/catch2_ext.hpp>
#include <jau/type_traits_queries.hpp>

TEST_CASE("jau::BasicStringLiteral_00", "[jau][std::string][BasicStringLiteral]") {
    static_assert(true == jau::req::string_literal<decltype("lala")>);
    static_assert(true == jau::req::string_alike<decltype("lala")>);
    static_assert(true == jau::req::string_alike<decltype(std::string_view("lala"))>);
    static_assert(true == jau::req::string_alike<decltype(std::string("lala"))>);
    static_assert(true == jau::req::string_alike<decltype((const char*)"lala")>);

    {
        constexpr const char s[] = "Hello";
        constexpr const jau::StringLiteral sl1 = s;
        constexpr const jau::StringLiteral sl2 = "Hello";
        static_assert( sl1 == sl2 );

        constexpr const jau::StringLiteral sl3 = sl2;
        static_assert( sl1 == sl3 );
        REQUIRE( sl1 == sl3 );

        jau::StringLiteral sl4 = sl3;
        REQUIRE( sl1 == sl4 );

        constexpr const jau::StringLiteral sl5 = "Hello";
        static_assert( sl1 == sl5 );

        constexpr const jau::StringLiteral sl6 = "Hello World";
        static_assert( sl1 != sl6 );
        REQUIRE( sl1 != sl6 );

        constexpr const jau::StringLiteral sl7 = " ";
        constexpr const jau::StringLiteral sl8 = "World";
        constexpr const jau::StringLiteral sl9 = sl1 + sl7 + sl8;
        static_assert( sl1 != sl9 );
        REQUIRE( sl1 != sl9 );
        static_assert( sl6 == sl9 );
        REQUIRE( sl6 == sl9 );

        constexpr const char sl7_b[] = " ";
        constexpr const jau::StringLiteral sl10 = sl1 + sl7_b + sl8;
        static_assert( sl1 != sl10 );
        REQUIRE( sl1 != sl10 );
        static_assert( sl6 == sl10 );
        REQUIRE( sl6 == sl10 );

        const char sl7_c[] = " ";
        const jau::StringLiteral sl11 = sl1 + sl7_c + sl8;
        REQUIRE( sl1 != sl11 );
        REQUIRE( sl6 == sl11 );

        constexpr const jau::StringLiteral sl12 = sl1 + " " + sl8;
        static_assert( sl1 != sl12 );
        REQUIRE( sl1 != sl12 );
        static_assert( sl6 == sl12 );
        REQUIRE( sl6 == sl12 );
    }
    {
        constexpr const char s1_c[] = "Hello";
        constexpr jau::StringLiteral sl2 = "Hello";
        constexpr jau::StringLiteral sl1 = s1_c;
        static_assert( s1_c == sl2 );
        REQUIRE( s1_c == sl2 );
        static_assert( sl1 == sl2 );
        REQUIRE( sl1 == sl2 );
        static_assert( sl2 == sl1 );
        REQUIRE( sl2 == sl1 );
    }
}


