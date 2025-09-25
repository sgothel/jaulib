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

template<typename T>
requires jau::req::pointer<T>
static bool is_pointer() {
    return true;
}

template<typename T>
requires (!jau::req::pointer<T>)
static bool is_pointer() {
    return false;
}


TEST_CASE( "01 Type Concept Queries") {
    REQUIRE(false == is_pointer<int>() );
    REQUIRE(true == is_pointer<int*>() );

    REQUIRE(false == jau::req::is_container<int>() );

    REQUIRE(true == jau::req::is_container<std::vector<int>>() );
    REQUIRE(true == jau::req::is_container<std::map<int, int>>() );
    REQUIRE(true == jau::req::is_container<std::array<int, 10>>() );
    REQUIRE(false == jau::req::is_container<std::forward_list<int>>() ); // misses: size
    REQUIRE(true == jau::req::is_container<std::list<int>>() );
    REQUIRE(true == jau::req::is_container<std::deque<int>>() );
    REQUIRE(true == jau::req::is_container<jau::darray<int>>() );
    REQUIRE(false == jau::req::is_container<jau::cow_darray<int>>() ); // has no direct `end` method

    REQUIRE(true == jau::req::is_contiguous_container<std::vector<int>>() );
    REQUIRE(true == jau::req::is_contiguous_container<std::array<int, 10>>() );
    REQUIRE(true == jau::req::is_contiguous_container<jau::darray<int>>() );
    REQUIRE(false == jau::req::is_contiguous_container<std::map<int, int>>() );
    REQUIRE(false == jau::req::is_contiguous_container<std::forward_list<int>>() );
    REQUIRE(false == jau::req::is_contiguous_container<std::list<int>>() );
    REQUIRE(false == jau::req::is_contiguous_container<std::deque<int>>() );
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

TEST_CASE( "02 Type Concept Misc") {
    REQUIRE(true == is_zero2(0_i32));

    REQUIRE(true == is_zero3(0_i32));
    REQUIRE(true == is_zero3(0_f32));
}
