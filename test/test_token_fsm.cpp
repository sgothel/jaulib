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

#include <jau/token_fsm.hpp>

using namespace jau::int_literals;

void test00_hello() {
    typedef jau::lang::token_fsm<uint16_t> token_fsm_u17;

    std::vector<token_fsm_u17::token_value_t> tkey_words = {
        { 1, "on" },
        { 2, "one" },
        { 3, "oneworld" },
        { 4, "onward" },
        { 5, "hello" }
    };

    token_fsm_u17 token( jau::lang::ascii26_alphabet(), tkey_words );
    fprintf(stderr, "token: %s\n", token.fsm_to_string(26).c_str());
    fprintf(stderr, "token: %s\n", token.to_string().c_str());
    REQUIRE( false == token.empty() );
    REQUIRE( 5 == token.count() );

    REQUIRE( true == token.add( { 6, "heaven" } ) );
    REQUIRE( 6 == token.count() );
    REQUIRE( false == token.empty() );

    for(size_t count=0; count < tkey_words.size(); ++count) {
        token_fsm_u17::uint_t res = token.get(tkey_words[count].value);
        fprintf(stderr, "%2zu: %s -> %zu (token)\n", count, std::string(tkey_words[count].value).c_str(), (size_t)res);
        REQUIRE( tkey_words[count].name == res );
    }
    REQUIRE( 6 == token.get("heaven") );
    {
        std::string haystack = "012345 hello aa"; // [7..12[
        token_fsm_u17::result_t res = token.find(haystack); // [7..12[
        fprintf(stderr, "find '%s' -> %s\n", haystack.c_str(), res.to_string().c_str());
        REQUIRE( tkey_words[4].name == res.token_name );
        REQUIRE(  7 == res.source_begin );
        REQUIRE( 12 == res.source_last );
    }
    {
        std::string haystack = "012345 hello"; // [7..12[
        token_fsm_u17::result_t res = token.find(haystack); // [7..12[
        fprintf(stderr, "find '%s' -> %s\n", haystack.c_str(), res.to_string().c_str());
        REQUIRE( tkey_words[4].name == res.token_name );
        REQUIRE(  7 == res.source_begin );
        REQUIRE( 12 == res.source_last );
    }
}

void test10_cpp_token() {
    std::vector<std::string_view> skey_words = {
        "alignas",  // C++11
        "alignof",  // C++11
        "and", 
        "and_eq", 
        "asm", 
        "atomic_cancel", // TM TS
        "atomic_commit", // TM TS
        "atomic_noexcept", // TM TS
        "auto", 
        "bitand", 
        "bitor", 
        "bool", 
        "break", 
        "case", 
        "catch", 
        "char", 
        "char8_t", // C++20
        "char16_t",  // C++11
        "char32_t",  // C++11
        "class", 
        "compl", 
        "concept", // C++20
        "const", 
        "consteval", // C++20
        "constexpr",  // C++11
        "constinit", // C++20
        "const_cast", 
        "continue", 
        "co_await", // C++20
        "co_return", // C++20
        "co_yield", // C++20
        "decltype",  // C++11
        "default", 
        "delete", 
        "do", 
        "double", 
        "dynamic_cast", 
        "else", 
        "enum", 
        "explicit", 
        "export", 
        "extern", 
        "false", 
        "float", 
        "for", 
        "friend", 
        "goto", 
        "if", 
        "inline", 
        "int", 
        "long", 
        "mutable", 
        "namespace", 
        "new", 
        "noexcept",  // C++11
        "not", 
        "not_eq", 
        "nullptr",  // C++11
        "operator", 
        "or", 
        "or_eq", 
        "private", 
        "protected", 
        "public", 
        "reflexpr", // reflection TS
        "register",
        "reinterpret_cast", 
        "requires", // C++20
        "return", 
        "short", 
        "signed", 
        "sizeof",
        "static", 
        "static_assert",  // C++11
        "static_cast", 
        "struct", 
        "switch", 
        "synchronized", // TM TS
        "template", 
        "this", 
        "thread_local",  // C++11
        "throw", 
        "true", 
        "try", 
        "typedef", 
        "typeid", 
        "typename", 
        "union", 
        "unsigned", 
        "using", 
        "virtual", 
        "void", 
        "volatile", 
        "wchar_t", 
        "while", 
        "xor", 
        "xor_eq" };

    typedef jau::lang::token_fsm<uint16_t> token_fsm_u17;

    std::vector<token_fsm_u17::token_value_t> tkey_words;
    for(size_t i=0; i<skey_words.size(); ++i ) {
        tkey_words.push_back( { static_cast<token_fsm_u17::uint_t>( i+1 ) /* token */, skey_words[i] /* value */ } );
    }

    token_fsm_u17 token( jau::lang::ascii69_alphabet(), tkey_words );
    // fprintf(stderr, "cpp_token: %s\n", token.fsm_to_string(26).c_str());
    fprintf(stderr, "cpp_token: %s\n", token.to_string().c_str());
    REQUIRE( false == token.empty() );

    for(size_t count=0; count < tkey_words.size(); ++count) {
        token_fsm_u17::uint_t res = token.get(tkey_words[count].value);
        fprintf(stderr, "%2zu: %s -> %zu (token)\n", count, std::string(tkey_words[count].value).c_str(), (size_t)res);
        REQUIRE( tkey_words[count].name == res );
    }
}

METHOD_AS_TEST_CASE( test00_hello, "00_hello");
METHOD_AS_TEST_CASE( test10_cpp_token, "10_cpp_token");
