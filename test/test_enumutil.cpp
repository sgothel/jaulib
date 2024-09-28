/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright Gothel Software e.K.
 *
 * SPDX-License-Identifier: MIT
 *
 * This Source Code Form is subject to the terms of the MIT License
 * If a copy of the MIT was not distributed with this
 * file, You can obtain one at https://opensource.org/license/mit/.
 */
#include <cassert>
#include <cstring>
#include <string_view>

#include <jau/enum_util.hpp>
#include <jau/test/catch2_ext.hpp>

// Define the `enum class` yourself ...
enum class test_type1_t : uint8_t {
    one = 1,
    two = 2,
    three = 3
};
// and add the `enum class` support functions
JAU_MAKE_ENUM_IMPL(test_type1_t, one, two, three);

// Define the `enum class` yourself ...
enum class test_type2_t : uint8_t {
    one,   // <first value
    two,   // <second value
    three  // <third value
};
// and add the `enum class` support functions
JAU_MAKE_ENUM_IMPL(test_type2_t, one, two, three);

// Define the `enum class` yourself ...
enum class test_type3_t : uint8_t {
    one = 1 << 0,
    two = 1 << 1,
    three = 1 << 2
};
// and add the `enum class` support functions
JAU_MAKE_BITFIELD_ENUM_IMPL(test_type3_t, one, two, three);

TEST_CASE( "Enum Class Value Type Test 10", "[enum][type]" ) {
    {
        using namespace jau::enums;

        static_assert( true == is_enum<test_type1_t::one>() );
        static_assert( true == is_enum<test_type1_t::two>() );
        static_assert( true == is_enum<test_type1_t::three>() );
        static_assert( "test_type1_t::one" == enum_longname<test_type1_t::one>() );
        static_assert( "test_type1_t::two" == enum_longname<test_type1_t::two>() );
        static_assert( "test_type1_t::three" == enum_longname<test_type1_t::three>() );
        static_assert( "one" == enum_name<test_type1_t::one>() );
        static_assert( "two" == enum_name<test_type1_t::two>() );
        static_assert( "three" == enum_name<test_type1_t::three>() );

        REQUIRE( "test_type1_t::one" == enum_longname<test_type1_t::one>() );
        REQUIRE( "one" == enum_name<test_type1_t::one>() );
        REQUIRE( true == is_enum<test_type1_t::one>() );
        {
            // std::string_view *res = fill_names<test_type1_t::one, test_type1_t::two, test_type1_t::three>();
            constexpr auto nt = get_names<test_type1_t::one, test_type1_t::two, test_type1_t::three>();
            for(std::string_view sv : nt.names) {
                std::cout << "NameTable: val -> string: " << sv << std::endl;
                REQUIRE( false == sv.empty() );
            }
            constexpr auto vt = get_values(test_type1_t::one, test_type1_t::two, test_type1_t::three);
            for(test_type1_t v : vt.values) {
                std::cout << "ValueTable: val: " << static_cast<int>(v) << std::endl;
            }
        }
    }

    {
        static_assert( 3 == test_type2_t_count() );
        static_assert( "one" == enum_name(test_type2_t::one) );
        static_assert( "test_type2_t::one" == enum_longname(test_type2_t::one) );

        REQUIRE( "one" == enum_name(test_type2_t::one) );
        REQUIRE( "test_type2_t::one" == enum_longname(test_type2_t::one) );
        REQUIRE( "one" == to_string(test_type2_t::one) );
    }
    {
        static_assert( 3 == test_type3_t_count() );
        static_assert( "one" == enum_name(test_type3_t::one) );
        // static_assert( "one" == to_string(test_type3_t::one) );
        static_assert( "test_type3_t::one" == enum_longname(test_type3_t::one) );
        REQUIRE( "one" == enum_name(test_type3_t::one) );
        REQUIRE( "test_type3_t::one" == enum_longname(test_type3_t::one) );

        REQUIRE( "[one]" == to_string(test_type3_t::one) );

        {
            using namespace jau::enums;

            REQUIRE( "[one, two]" == to_string(test_type3_t::one | test_type3_t::two) );
            REQUIRE( "[one, two, three]" == to_string(test_type3_t::one | test_type3_t::two | test_type3_t::three) );
        }
    }
}

namespace test::local {
    // Define the `enum class` yourself ...
    enum class test_type4_t : uint8_t {
        one = 1 << 0,
        two = 1 << 1,
        three = 1 << 2
    };
    // and add the `enum class` support functions
    JAU_MAKE_BITFIELD_ENUM_IMPL(test_type4_t, one, two, three);

    // Define the `enum class` yourself ...
    enum class test_type5_t : uint8_t {
        one = 10,
        two = 20,
        three = 30
    };
    // and add the `enum class` support functions
    JAU_MAKE_ENUM_IMPL(test_type5_t, one, two, three);
}

TEST_CASE( "Enum Class Value Type Test 11", "[enum][type]" ) {
    {
        using namespace test::local;
        static_assert( 3 == test_type4_t_count() );
        static_assert( "one" == enum_name(test_type4_t::one) );
        // static_assert( "one" == to_string(test_type4_t::one) );
        static_assert( "test_type4_t::one" == enum_longname(test_type4_t::one) );
        REQUIRE( "one" == enum_name(test_type4_t::one) );
        REQUIRE( "test_type4_t::one" == enum_longname(test_type4_t::one) );

        REQUIRE( "[one]" == to_string(test_type4_t::one) );

        {
            using namespace jau::enums;
            REQUIRE( "[one, two]" == to_string(test_type4_t::one | test_type4_t::two) );
            REQUIRE( "[one, two, three]" == to_string(test_type4_t::one | test_type4_t::two | test_type4_t::three) );
        }
    }
    {
        using namespace test::local;

        static_assert( 3 == test_type5_t_count() );
        static_assert( "one" == enum_name(test_type5_t::one) );
        static_assert( "test_type5_t::one" == enum_longname(test_type5_t::one) );
        REQUIRE( "one" == enum_name(test_type5_t::one) );
        REQUIRE( "test_type5_t::one" == enum_longname(test_type5_t::one) );

        REQUIRE( "one" == to_string(test_type5_t::one) );

        using namespace jau::enums;
        static_assert( 10 == number(test_type5_t::one) );
        static_assert( 20 == number(test_type5_t::two) );
        static_assert( 30 == number(test_type5_t::three) );
    }
}