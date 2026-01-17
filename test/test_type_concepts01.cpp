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

#include <concepts>
#include <type_traits>
#include <vector>
#include <list>
#include <forward_list>
#include <deque>

#include <jau/int_math.hpp>
#include <jau/int_types.hpp>
#include <jau/cow_darray.hpp>
#include <jau/darray.hpp>
#include <jau/float_math.hpp>
#include <jau/float_types.hpp>
#include <jau/test/catch2_ext.hpp>

#include <jau/type_concepts.hpp>

using namespace jau::int_literals;
using namespace jau::float_literals;

class AnyClass {};

TEST_CASE( "01 Type Concept Queries: Build-In") {
    static_assert(true == std::is_integral_v<char> );
    static_assert(false == std::is_unsigned_v<char> );

    static_assert(false == jau::req::pointer<int> );
    static_assert(true  == jau::req::pointer<int*> );

    static_assert(true == std::is_integral_v<bool> );
    static_assert(true == std::is_same_v<bool, std::type_identity_t<bool>> );
    static_assert(true == jau::req::boolean<std::type_identity_t<bool>> );

    static_assert(true  == jau::req::unsigned_integral<unsigned> );
    static_assert(false == jau::req::unsigned_integral<int> );
    static_assert(true  == jau::req::unsigned_integral<bool> );
    static_assert(true  == jau::req::unsigned_integral<decltype(1_u32)> );
    static_assert(false == jau::req::unsigned_integral<decltype(1_i32)> );
    static_assert(false == jau::req::unsigned_integral<decltype(1)> );

    static_assert(true  == jau::req::signed_integral<int> );
    static_assert(false == jau::req::signed_integral<unsigned> );
    static_assert(false == jau::req::signed_integral<bool> );
    static_assert(true  == jau::req::signed_integral<decltype(1_i32)> );
    static_assert(false == jau::req::signed_integral<decltype(1_u32)> );
    static_assert(true  == jau::req::signed_integral<decltype(1)> );

    static_assert(true  == jau::req::boolean<bool> );
    static_assert(false == jau::req::boolean<int> );

    static_assert(true  == jau::req::pointer<int*> );
    static_assert(true  == jau::req::pointer<char*> );
    static_assert(true  == jau::req::pointer<const char*> );
    static_assert(false == jau::req::pointer<int> );

    static_assert(false == jau::req::is_container<int>() );

    static_assert(true == jau::req::is_container<std::vector<int>>() );
    static_assert(true == jau::req::is_container<std::map<int, int>>() );
    static_assert(true == jau::req::is_container<std::array<int, 10>>() );
    static_assert(false == jau::req::is_container<std::forward_list<int>>() ); // misses: size
    static_assert(true == jau::req::is_container<std::list<int>>() );
    static_assert(true == jau::req::is_container<std::deque<int>>() );
    static_assert(true == jau::req::is_container<jau::darray<int>>() );
    static_assert(false == jau::req::is_container<jau::cow_darray<int>>() ); // has no direct `end` method

    static_assert(true == jau::req::is_contiguous_container<std::vector<int>>() );
    static_assert(true == jau::req::is_contiguous_container<std::array<int, 10>>() );
    static_assert(true == jau::req::is_contiguous_container<jau::darray<int>>() );
    static_assert(false == jau::req::is_contiguous_container<std::map<int, int>>() );
    static_assert(false == jau::req::is_contiguous_container<std::forward_list<int>>() );
    static_assert(false == jau::req::is_contiguous_container<std::list<int>>() );
    static_assert(false == jau::req::is_contiguous_container<std::deque<int>>() );

    REQUIRE(true == true);
}

TEST_CASE( "02 Type Concept Queries: Strings") {
    static_assert(false == jau::req::char_pointer<decltype(std::string("Hello"))> );
    static_assert(false == jau::req::char_pointer<decltype(std::string_view("Hello"))> );
    static_assert(false == jau::req::char_pointer<decltype("Hello")> );
    static_assert(true  == jau::req::char_pointer<decltype((const char*)"Hello")> );
    static_assert(false == jau::req::char_pointer<int*> );
    static_assert(false == jau::req::char_pointer<decltype('c')> );
    static_assert(false == jau::req::char_pointer<decltype(123)> );
    static_assert(false == jau::req::char_pointer<decltype(123.0f)> );
    static_assert(false == jau::req::char_pointer<AnyClass> );

    static_assert(false == jau::req::string_literal<decltype(std::string("Hello"))> );
    static_assert(false == jau::req::string_literal<decltype(std::string_view("Hello"))> );
    static_assert(true  == jau::req::string_literal<decltype("Hello")> );
    static_assert(false == jau::req::string_literal<decltype((const char*)"Hello")> );
    static_assert(false == jau::req::string_literal<int*> );
    static_assert(false == jau::req::string_literal<decltype('c')> );
    static_assert(false == jau::req::string_literal<decltype(123)> );
    static_assert(false == jau::req::string_literal<decltype(123.0f)> );
    static_assert(false == jau::req::string_literal<AnyClass> );

    static_assert(true  == jau::req::string_type<decltype(std::string("Hello"))> );
    static_assert(false == jau::req::string_type<decltype(std::string_view("Hello"))> );
    static_assert(false == jau::req::string_type<decltype("Hello")> );
    static_assert(false == jau::req::string_type<decltype((const char*)"Hello")> );
    static_assert(false == jau::req::string_type<int*> );
    static_assert(false == jau::req::string_type<decltype('c')> );
    static_assert(false == jau::req::string_type<decltype(123)> );
    static_assert(false == jau::req::string_type<decltype(123.0f)> );
    static_assert(false == jau::req::string_type<AnyClass> );

    static_assert(true  == jau::req::string_class<decltype(std::string("Hello"))> );
    static_assert(true  == jau::req::string_class<decltype(std::string_view("Hello"))> );
    static_assert(false == jau::req::string_class<decltype("Hello")> );
    static_assert(false == jau::req::string_class<decltype((const char*)"Hello")> );
    static_assert(false == jau::req::string_class<int*> );
    static_assert(false == jau::req::string_class<decltype('c')> );
    static_assert(false == jau::req::string_class<decltype(123)> );
    static_assert(false == jau::req::string_class<decltype(123.0f)> );
    static_assert(false == jau::req::string_class<AnyClass> );

    static_assert(true  == jau::req::string_alike<decltype(std::string("Hello"))> );
    static_assert(true  == jau::req::string_alike<decltype(std::string_view("Hello"))> );
    static_assert(true  == jau::req::string_alike<decltype("Hello")> );
    static_assert(true  == jau::req::string_alike<decltype((const char*)"Hello")> );
    static_assert(false == jau::req::string_alike<int*> );
    static_assert(false == jau::req::string_alike<decltype('c')> );
    static_assert(false == jau::req::string_alike<decltype(123)> );
    static_assert(false == jau::req::string_alike<decltype(123.0f)> );
    static_assert(false == jau::req::string_alike<AnyClass> );

    static_assert(true  == jau::req::stringifyable_std<decltype(std::string("Hello"))> );
    static_assert(true  == jau::req::stringifyable_std<decltype(std::string_view("Hello"))> );
    static_assert(true  == jau::req::stringifyable_std<decltype("Hello")> );
    static_assert(true  == jau::req::stringifyable_std<decltype((const char*)"Hello")> );
    static_assert(false == jau::req::stringifyable_std<int*> );
    static_assert(true  == jau::req::stringifyable_std<decltype('c')> );
    static_assert(true  == jau::req::stringifyable_std<decltype(123)> );
    static_assert(true  == jau::req::stringifyable_std<decltype(123.0f)> );
    static_assert(false == jau::req::stringifyable_std<AnyClass> );

    static_assert(true  == jau::req::stringifyable_jau<decltype(std::string("Hello"))> );
    static_assert(true  == jau::req::stringifyable_jau<decltype(std::string_view("Hello"))> );
    static_assert(true  == jau::req::stringifyable_jau<decltype("Hello")> );
    static_assert(true  == jau::req::stringifyable_jau<decltype((const char*)"Hello")> );
    static_assert(true  == jau::req::stringifyable_jau<int*> );
    static_assert(true  == jau::req::stringifyable_jau<decltype('c')> );
    static_assert(true  == jau::req::stringifyable_jau<decltype(123)> );
    static_assert(true  == jau::req::stringifyable_jau<decltype(123.0f)> );
    static_assert(false == jau::req::stringifyable_jau<AnyClass> );
}

template<typename T>
    requires std::integral<T>
static constexpr bool is_zero2(const T& a) noexcept {
    return 0 == a;
}
template<std::integral T>
static constexpr bool is_zero3(const T& a) noexcept {
    return 0 == a;
}
template<std::floating_point T>
static constexpr bool is_zero3(const T& a) noexcept {
    return 0 == a;
}

TEST_CASE( "03 Type Concept Misc") {
    static_assert(true == is_zero2(0_i32));

    static_assert(true == is_zero3(0_i32));
    static_assert(true == is_zero3(0_f32));

    REQUIRE(true == true);
}
