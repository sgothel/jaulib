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
#include "catch2/catch_amalgamated.hpp"

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

class SomeClass {
  public:
    std::string toString() const { return "SomeClass toString"; }
};
enum class game_t : uint16_t {
    none,
    chess,
    pacman,
    mrdo
};
JAU_MAKE_ENUM_STRING(game_t, chess, pacman, mrdo); // NOLINT

enum class plainenum_t : uint16_t {
    none,
    lala,
    lili
};

enum freeenum_t : uint16_t {
    none,
    lala,
    lili
};

template<typename T>
class MyWrap {
  private:
    T store;

  public:
    using value_type = T;

    MyWrap(T v) noexcept : store(v) {}
    inline operator T() const noexcept { return store; }
    inline operator T&() noexcept { return store; }
};

TEST_CASE( "01 Type Concept Queries: Build-In") {
    static_assert(true  == std::is_integral_v<char> );
    static_assert(true  == std::is_integral_v<unsigned> );
    static_assert(true  == std::is_integral_v<unsigned short> );
    static_assert(true  == std::is_integral_v<int> );
    static_assert(true  == std::is_integral_v<short> );
    static_assert(true  == std::is_integral_v<bool> );
    static_assert(true  == std::is_integral_v<decltype(1_u32)> );
    static_assert(true  == std::is_integral_v<decltype(1_i32)> );
    static_assert(true  == std::is_integral_v<decltype(1)> );
    static_assert(false == std::is_integral_v<float> );
    static_assert(false == std::is_integral_v<game_t> );

    static_assert(true  == jau::req::integer<unsigned> );
    static_assert(true  == jau::req::integer<int> );
    static_assert(true  == jau::req::integer<short> );
    static_assert(true  == jau::req::integer<unsigned short> );
    static_assert(true  == jau::req::integer<char> );
    static_assert(true  == jau::req::integer<decltype(1_u32)> );
    static_assert(true  == jau::req::integer<decltype(1_i32)> );
    static_assert(true  == jau::req::integer<decltype(1)> );
    static_assert(false == jau::req::integer<bool> );
    static_assert(false == jau::req::integer<float> );
    static_assert(false == jau::req::integer<game_t> );

    static_assert(true  == jau::req::signed_integer<int> );
    static_assert(true  == jau::req::signed_integer<short> );
    static_assert(true  == jau::req::signed_integer<char> );
    static_assert(true  == jau::req::signed_integer<decltype(1_i32)> );
    static_assert(true  == jau::req::signed_integer<decltype(1)> );
    static_assert(false == jau::req::signed_integer<unsigned> );
    static_assert(false == jau::req::signed_integer<decltype(1_u32)> );
    static_assert(false == jau::req::signed_integer<bool> );
    static_assert(false == jau::req::signed_integer<unsigned short> );
    static_assert(false == jau::req::signed_integer<float> );
    static_assert(false == jau::req::signed_integer<game_t> );

    static_assert(true  == jau::req::unsigned_integer<unsigned> );
    static_assert(true  == jau::req::unsigned_integer<unsigned short> );
    static_assert(true  == jau::req::unsigned_integer<decltype(1_u32)> );
    static_assert(false == jau::req::unsigned_integer<int> );
    static_assert(false == jau::req::unsigned_integer<short> );
    static_assert(false == jau::req::unsigned_integer<char> );
    static_assert(false == jau::req::unsigned_integer<decltype(1_i32)> );
    static_assert(false == jau::req::unsigned_integer<decltype(1)> );
    static_assert(false == jau::req::unsigned_integer<bool> );
    static_assert(false == jau::req::unsigned_integer<float> );
    static_assert(false == jau::req::unsigned_integer<game_t> );

    static_assert(false == std::is_unsigned_v<char> );

    static_assert(false == jau::req::pointer<int> );
    static_assert(true  == jau::req::pointer<int*> );

    static_assert(true == std::is_integral_v<bool> );
    static_assert(true == std::is_same_v<bool, std::type_identity_t<bool>> );
    static_assert(true == jau::req::boolean<std::type_identity_t<bool>> );

    static_assert(true  == jau::req::unsigned_integral<unsigned> );
    static_assert(true  == jau::req::unsigned_integral<unsigned short> );
    static_assert(false == jau::req::unsigned_integral<int> );
    static_assert(true  == jau::req::unsigned_integral<bool> );
    static_assert(false == jau::req::unsigned_integral<float> );
    static_assert(true  == jau::req::unsigned_integral<decltype(1_u32)> );
    static_assert(false == jau::req::unsigned_integral<decltype(1_i32)> );
    static_assert(false == jau::req::unsigned_integral<decltype(1)> );

    static_assert(true  == jau::req::signed_integral<int> );
    static_assert(false == jau::req::signed_integral<unsigned> );
    static_assert(false == jau::req::signed_integral<unsigned short> );
    static_assert(false == jau::req::signed_integral<bool> );
    static_assert(false == jau::req::signed_integral<float> );
    static_assert(true  == jau::req::signed_integral<decltype(1_i32)> );
    static_assert(false == jau::req::signed_integral<decltype(1_u32)> );
    static_assert(true  == jau::req::signed_integral<decltype(1)> );

    static_assert(false == std::integral<jau::uint128dp_t> );
    static_assert(false == jau::req::unsigned_integral<jau::uint128dp_t> );
    static_assert(false == jau::req::signed_integral<jau::uint128dp_t> );

    static_assert(true  == std::is_integral_v<bool> );
    static_assert(true  == jau::req::boolean<bool> );
    static_assert(false == jau::req::boolean<int> );

    static_assert(true  == std::is_enum_v<freeenum_t> );
    static_assert(true  == std::is_enum_v<plainenum_t> );
    static_assert(true  == std::is_enum_v<game_t> );
    static_assert(false  == std::is_integral_v<freeenum_t> );
    static_assert(false  == std::is_integral_v<plainenum_t> );
    static_assert(false  == std::is_integral_v<game_t> );
    static_assert(false  == jau::req::has_free_to_string_any<freeenum_t>);
    static_assert(false  == jau::req::has_free_to_string_any<plainenum_t>);
    static_assert(true  == jau::req::has_free_to_string_any<game_t>);

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

    {
        using namespace jau::req;
        MyWrap<uint16_t> w_u16 = 42;
        uint16_t u16_0 = 42;
        static_assert(true == std::is_same_v<uint16_t, type_of<decltype(w_u16)>>);
        static_assert(true == std::is_same_v<uint16_t, type_of<decltype(u16_0)>>);

        // copy value
        const type_of<decltype(w_u16)> u16_1 = value_of(w_u16);
        // mutable reference value, copy
        type_of<decltype(w_u16)> &u16_2 = reference_of(w_u16);
        REQUIRE(42 == jau::req::value_of(w_u16));
        REQUIRE(42 == u16_1);
        REQUIRE(42 == u16_2);
        REQUIRE(42 == u16_0);

        u16_2 += 99;
        reference_of(u16_2) += 1;
        REQUIRE(142 == u16_2);
        REQUIRE(142 == w_u16);

        w_u16 = 24;
        REQUIRE(24 == w_u16);
        REQUIRE(24 == jau::req::value_of(w_u16));
        REQUIRE(42 == u16_1);
        REQUIRE(24 == u16_2);

        reference_of(w_u16) = 88;
        REQUIRE(88 == w_u16);
        REQUIRE(88 == jau::req::value_of(w_u16));
        REQUIRE(42 == u16_1);
        REQUIRE(88 == u16_2);
    }
    {
        using namespace jau::req;
        game_t g1_0 = game_t::pacman;
        MyWrap<game_t> w_g1 = game_t::chess;
        static_assert(true == std::is_same_v<game_t, type_of<decltype(w_g1)>>);
        static_assert(true == std::is_same_v<game_t, type_of<decltype(g1_0)>>);

        // copy value
        const type_of<decltype(w_g1)> g1_1 = value_of(w_g1);
        // mutable reference value, copy
        type_of<decltype(w_g1)> &g1_2 = reference_of(w_g1);

        REQUIRE(game_t::chess == jau::req::value_of(w_g1));
        REQUIRE(game_t::chess == g1_1);
        REQUIRE(game_t::chess == g1_2);
        REQUIRE(game_t::pacman == g1_0);

        g1_2 = game_t::none;
        reference_of(g1_2) = game_t::mrdo;
        REQUIRE(game_t::mrdo == w_g1);
        REQUIRE(game_t::mrdo == jau::req::value_of(w_g1));
        REQUIRE(game_t::chess == g1_1);
        REQUIRE(game_t::mrdo == g1_2);

        w_g1 = game_t::pacman;
        REQUIRE(game_t::pacman == w_g1);
        REQUIRE(game_t::pacman == jau::req::value_of(w_g1));
        REQUIRE(game_t::chess == g1_1);
        REQUIRE(game_t::pacman == g1_2);

        reference_of(w_g1) = game_t::mrdo;
        REQUIRE(game_t::mrdo == w_g1);
        REQUIRE(game_t::mrdo == jau::req::value_of(w_g1));
        REQUIRE(game_t::chess == g1_1);
        REQUIRE(game_t::mrdo == g1_2);
    }
    {
        using namespace jau::req;

        uint16_t native_u16 = 1;
        type_of<decltype(native_u16)> u16_0 = value_of(native_u16);
        REQUIRE(1 == u16_0);

        jau::relaxed_atomic_uint16 relaxed_u16 = 11U;
        static_assert(true == std::is_same_v<uint16_t, type_of<decltype(relaxed_u16)>>);
        static_assert(true  == jau::req::wrapped_unsigned_integral<decltype(relaxed_u16)> );
        type_of<decltype(relaxed_u16)> u16_1 = value_of(relaxed_u16);

        REQUIRE(11U == jau::req::value_of(relaxed_u16));
        REQUIRE(11U == u16_1);
    }
    REQUIRE(true == true);
}

template <typename T>
requires jau::req::string_alike0<T>
static constexpr void checkOne() noexcept {}
template <typename T>
requires jau::req::boolean<T>
static constexpr void checkOne() noexcept {}

template <typename... Targs>
consteval_cxx20 void check2(std::string_view) noexcept {
    if constexpr( 0 < sizeof...(Targs) ) {
        ((checkOne<Targs>()), ...);
    }
}

TEST_CASE( "02 Type Concept Queries: Strings") {
    static_assert(false == jau::req::char_pointer<decltype(std::string("Hello"))> );
    static_assert(false == jau::req::char_pointer<decltype(std::string_view("Hello"))> );
    static_assert(false == jau::req::char_pointer<decltype("Hello")> );
    static_assert(true  == jau::req::char_pointer<decltype((const char*)"Hello")> );
    static_assert(false == jau::req::char_pointer<int*> );
    static_assert(false == jau::req::char_pointer<void*> );
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
    {
        bool b_;
        std::string name_;
        static_assert(true  == jau::req::string_alike<decltype(name_)> );
        static_assert(true  == jau::req::string_alike0<decltype(name_)> );
        static_assert(true  == jau::req::boolean<decltype(b_)> );
        check2<decltype(name_)>("");
        check2<decltype(b_)>("");
    }

    static_assert(true  == jau::req::stringifyable_std<decltype(std::string("Hello"))> );
    static_assert(true  == jau::req::stringifyable_std<decltype(std::string_view("Hello"))> );
    static_assert(true  == jau::req::stringifyable_std<decltype("Hello")> );
    static_assert(true  == jau::req::stringifyable_std<decltype((const char*)"Hello")> );
    static_assert(false == jau::req::stringifyable_std<int*> );
    static_assert(true  == jau::req::stringifyable_std<decltype('c')> );
    static_assert(true  == jau::req::stringifyable_std<decltype(123)> );
    static_assert(true  == jau::req::stringifyable_std<decltype(123.0f)> );
    static_assert(false == jau::req::stringifyable_std<AnyClass> );
    static_assert(false == jau::req::stringifyable_std<SomeClass> );
    static_assert(false == jau::req::stringifyable_std<game_t> );
    static_assert(false == jau::req::stringifyable_std<plainenum_t> );

    static_assert(true  == jau::req::stringifyable0_jau<decltype(std::string("Hello"))> );
    static_assert(true  == jau::req::stringifyable0_jau<decltype(std::string_view("Hello"))> );
    static_assert(true  == jau::req::stringifyable0_jau<decltype("Hello")> );
    static_assert(true  == jau::req::stringifyable0_jau<decltype((const char*)"Hello")> );
    static_assert(false == jau::req::stringifyable0_jau<int*> );
    static_assert(false == jau::req::stringifyable0_jau<void*> );
    static_assert(false == jau::req::stringifyable0_jau<decltype('c')> );
    static_assert(false == jau::req::stringifyable0_jau<decltype(123)> );
    static_assert(false == jau::req::stringifyable0_jau<decltype(123.0f)> );
    static_assert(false == jau::req::stringifyable0_jau<AnyClass> );
    static_assert(true  == jau::req::stringifyable0_jau<SomeClass> );
    static_assert(true  == jau::req::stringifyable0_jau<game_t> );
    static_assert(false == jau::req::stringifyable0_jau<plainenum_t> );

    static_assert(true  == jau::req::stringifyable1_jau<decltype(std::string("Hello"))> );
    static_assert(true  == jau::req::stringifyable1_jau<decltype(std::string_view("Hello"))> );
    static_assert(true  == jau::req::stringifyable1_jau<decltype("Hello")> );
    static_assert(true  == jau::req::stringifyable1_jau<decltype((const char*)"Hello")> );
    static_assert(true  == jau::req::stringifyable1_jau<int*> );
    static_assert(true  == jau::req::stringifyable1_jau<void*> );
    static_assert(true  == jau::req::stringifyable1_jau<decltype('c')> );
    static_assert(true  == jau::req::stringifyable1_jau<decltype(123)> );
    static_assert(true  == jau::req::stringifyable1_jau<decltype(123.0f)> );
    static_assert(false == jau::req::stringifyable1_jau<AnyClass> );
    static_assert(true  == jau::req::stringifyable1_jau<SomeClass> );
    static_assert(true  == jau::req::stringifyable1_jau<game_t> );
    static_assert(false == jau::req::stringifyable1_jau<plainenum_t> );

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
