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

#include <jau/basic_collections.hpp>
#include <jau/test/catch2_ext.hpp>

TEST_CASE( "StringHashMapWrap Test 00", "[hashmap][string][StringHashMapWrap]" ) {
    {
        const std::string two = "two";
        const std::string_view two2 = "two";
        jau::StringHashMapWrap<int, int, -1>  map;
        REQUIRE( 0 == map.size() );
        REQUIRE( true == map.put("one", 1) );
        REQUIRE( true == map.put(two, 2) );
        REQUIRE( 2 == map.size() );

        auto pp = map.find(two2);
        REQUIRE( nullptr != pp );
        REQUIRE( two == pp->first );
        REQUIRE( two2 == pp->first );
        REQUIRE( two.data() != pp->first.data() ); // string key maintains its own storage

        REQUIRE( 1 == map.get("one") );
        REQUIRE( 2 == map.get(two2) );
        REQUIRE( false == map.put(two2, 3) );
        REQUIRE( 2 == map.size() );
        REQUIRE( 3 == map.get(two2) );

        REQUIRE( 3 == map.put3(two2, 4) );
        REQUIRE( 2 == map.size() );
        REQUIRE( 4 == map.get(two2) );

        REQUIRE( -1 == map.put3("new", 100) );
        REQUIRE(  3 == map.size() );
        REQUIRE(100 == map.get("new") );

        REQUIRE(100 == map.remove2("new") );
        REQUIRE(  2 == map.size() );
        REQUIRE( -1 == map.remove2("new") );
        REQUIRE(  2 == map.size() );
        REQUIRE(true == map.remove(two2) );
        REQUIRE(  1 == map.size() );
        REQUIRE(false == map.remove(two2) );
        REQUIRE(  1 == map.size() );

        REQUIRE( false == map.insert("one", 1000) );
        REQUIRE( 1 == map.size() );
        REQUIRE( 1 == map.get("one") );
        REQUIRE( true == map.insert(two, 2) );
        REQUIRE( 2 == map.size() );
        REQUIRE( 2 == map.get(two2) );

        REQUIRE( true == map.replace(two2, 3) );
        REQUIRE( 2 == map.size() );
        REQUIRE( 3 == map.get(two2) );
        REQUIRE( false == map.replace("new", 1) );
        REQUIRE( 2 == map.size() );

        {
            const int v = 42;
            int &i2_0 = map.put2("i2", v);
            int &i2_1 = map.get("i2");
            const int &i2_2 = map.get("i2");
            REQUIRE( v == i2_0 );
            REQUIRE( v == i2_1 );
            REQUIRE( v == i2_2 );
            REQUIRE( &i2_0 != &v );
            REQUIRE( &i2_0 == &i2_1 );
            REQUIRE( &i2_0 == &i2_2 );
            REQUIRE( 3 == map.size() );
        }

        map.clear();
        REQUIRE( 0 == map.size() );
    }
}

TEST_CASE( "StringViewHashMapWrap Test 00", "[hashmap][string_view][StringViewHashMapWrap]" ) {
    {
        // string_view key must be persistent!
        const std::string_view one = "one";
        const std::string_view two = "two";
        jau::StringViewHashMapWrap<int, int, -1>  map;
        REQUIRE( 0 == map.size() );
        REQUIRE( true == map.put(one, 1) );
        REQUIRE( true == map.put(two, 2) );
        REQUIRE( 2 == map.size() );

        auto pp = map.find(two);
        REQUIRE( nullptr != pp );
        REQUIRE( two == pp->first );
        REQUIRE( two.data() == pp->first.data() ); // string_view key uses external storage two

        REQUIRE( 1 == map.get(one) );
        REQUIRE( 2 == map.get(two) );
        REQUIRE( false == map.put(two, 3) );
        REQUIRE( 2 == map.size() );
        REQUIRE( 3 == map.get(two) );

        REQUIRE( 3 == map.put3(two, 4) );
        REQUIRE( 2 == map.size() );
        REQUIRE( 4 == map.get(two) );

        REQUIRE( -1 == map.put3("new", 100) );
        REQUIRE(  3 == map.size() );
        REQUIRE(100 == map.get("new") );

        REQUIRE(100 == map.remove2("new") );
        REQUIRE(  2 == map.size() );
        REQUIRE( -1 == map.remove2("new") );
        REQUIRE(  2 == map.size() );
        REQUIRE(true == map.remove(two) );
        REQUIRE(  1 == map.size() );
        REQUIRE(false == map.remove(two) );
        REQUIRE(  1 == map.size() );

        REQUIRE( false == map.insert("one", 1000) );
        REQUIRE( 1 == map.size() );
        REQUIRE( 1 == map.get("one") );
        REQUIRE( true == map.insert(two, 2) );
        REQUIRE( 2 == map.size() );
        REQUIRE( 2 == map.get(two) );

        REQUIRE( true == map.replace(two, 3) );
        REQUIRE( 2 == map.size() );
        REQUIRE( 3 == map.get(two) );
        REQUIRE( false == map.replace("new", 1) );
        REQUIRE( 2 == map.size() );

        {
            const int v = 42;
            const std::string_view i2_k = "i2";
            int &i2_0 = map.put2(i2_k, v);
            int &i2_1 = map.get(i2_k);
            const int &i2_2 = map.get(i2_k);
            REQUIRE( v == i2_0 );
            REQUIRE( v == i2_1 );
            REQUIRE( v == i2_2 );
            REQUIRE( &i2_0 != &v );
            REQUIRE( &i2_0 == &i2_1 );
            REQUIRE( &i2_0 == &i2_2 );
            REQUIRE( 3 == map.size() );
        }

        map.clear();
        REQUIRE( 0 == map.size() );
    }
    {
        // string_view key must be persistent!
        const std::string one1_storage = "one";
        const std::string one2_storage = "one";
        const std::string two1_storage = "two";
        const std::string two2_storage = "two";
        const std::string_view one1(one1_storage);
        const std::string_view one2(one2_storage);
        const std::string_view two1(two1_storage);
        const std::string_view two2(two2_storage);
        REQUIRE(one1.data() != one2.data());
        REQUIRE(two1.data() != two2.data());

        jau::StringViewHashMapWrap<int, int, -1>  map;
        REQUIRE( 0 == map.size() );
        REQUIRE( true == map.put(one1, 1) );
        REQUIRE( true == map.put(two1, 2) );
        REQUIRE( 2 == map.size() );

        auto pp = map.find(two2);
        REQUIRE( nullptr != pp );
        REQUIRE( two1 == pp->first );
        REQUIRE( two2 == pp->first );
        REQUIRE( two1.data() == pp->first.data() ); // string_view key uses external storage two1
        REQUIRE( two2.data() != pp->first.data() ); // string_view key uses external storage two1

        REQUIRE( 1 == map.get(one2) );
        REQUIRE( 2 == map.get(two2) );
        REQUIRE( false == map.put(two2, 3) );
        REQUIRE( 2 == map.size() );
        REQUIRE( 3 == map.get(two2) );

        REQUIRE( 3 == map.put3(two2, 4) );
        REQUIRE( 2 == map.size() );
        REQUIRE( 4 == map.get(two2) );

        REQUIRE( -1 == map.put3("new", 100) );
        REQUIRE(  3 == map.size() );
        REQUIRE(100 == map.get("new") );

        REQUIRE(100 == map.remove2("new") );
        REQUIRE(  2 == map.size() );
        REQUIRE( -1 == map.remove2("new") );
        REQUIRE(  2 == map.size() );
        REQUIRE(true == map.remove(two2) );
        REQUIRE(  1 == map.size() );
        REQUIRE(false == map.remove(two2) );
        REQUIRE(  1 == map.size() );

        REQUIRE( false == map.insert("one", 1000) );
        REQUIRE( 1 == map.size() );
        REQUIRE( 1 == map.get("one") );
        REQUIRE( true == map.insert(two1, 2) );
        REQUIRE( 2 == map.size() );
        REQUIRE( 2 == map.get(two2) );

        REQUIRE( true == map.replace(two2, 3) );
        REQUIRE( 2 == map.size() );
        REQUIRE( 3 == map.get(two2) );
        REQUIRE( false == map.replace("new", 1) );
        REQUIRE( 2 == map.size() );

        map.clear();
        REQUIRE( 0 == map.size() );
    }
}
