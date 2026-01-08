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
#include <iostream>

#include <jau/enum_util.hpp>
#include <jau/io/file_util.hpp>
#include <jau/test/catch2_ext.hpp>

// Define the `enum class` yourself ...
enum class test_type1_t : uint8_t {
    none = 0, // <no value item denoting no value
    one = 1,
    two = 2,
    three = 3
};
// and add the `enum class` support functions
JAU_MAKE_ENUM_STRING(test_type1_t, one, two, three);
JAU_MAKE_ENUM_INFO(test_type1_t, none, one, two, three);

// Define the `enum class` yourself ...
enum class test_type2_t : uint8_t {
    none = 0, // <no value item denoting no value
    one,   // <first value
    two,   // <second value
    three  // <third value
};
// and add the `enum class` support functions
JAU_MAKE_ENUM_STRING(test_type2_t, one, two, three);
JAU_MAKE_ENUM_INFO(test_type2_t, none, one, two, three);

// Define the `enum class` yourself ...
enum class test_type3_t : uint8_t {
    none = 0, // <no value item denoting no value
    one = 1 << 0,
    two = 1 << 1,
    three = 1 << 2
};
// and add the `enum class` support functions
JAU_MAKE_BITFIELD_ENUM_STRING(test_type3_t, one, two, three);
JAU_MAKE_ENUM_INFO(test_type3_t, none, one, two, three);

namespace jau::io::fs {
    JAU_MAKE_ENUM_INFO(fmode_t, none, sock, blk, chr, fifo, dir, file, link, no_access, not_existing);
    JAU_MAKE_ENUM_INFO(mountflags_linux, none, rdonly, nosuid, nodev, noexec, synchronous, remount, mandlock, dirsync, noatime,
                       nodiratime, bind, move, rec, silent, posixacl, unbindable, private_, slave, shared, relatime,
                       kernmount, i_version, strictatime, lazytime, active, nouser);
}

template<typename enum_info_t>
static void test_enum_info(size_t size)
{
    using namespace jau::enums;

    typedef typename enum_info_t::iterator iterator;
    typedef typename enum_info_t::value_type enum_t;
    const enum_info_t& ei = enum_info_t::get();
    std::cout << ei << std::endl;
    std::cout << "Enum type: " << ei.name() << std::endl;
    size_t i=0;
    for(iterator iter = ei.begin(); iter != ei.end(); ++iter, ++i) {
        enum_t ev = *iter;
        std::cout << "#" << i << ": " << ev << ", value: " << std::to_string( *ev ) << std::endl;
    }
    REQUIRE( size == enum_info_t::size() );
}

TEST_CASE( "Enum Class Value Type Test 10", "[enum][type]" ) {
    {
        using namespace jau::enums;

        static_assert( true == is_enum<test_type1_t::one>() );
        static_assert( true == is_enum<test_type1_t::two>() );
        static_assert( true == is_enum<test_type1_t::three>() );
        static_assert( "test_type1_t::one" == long_name<test_type1_t::one>() );
        static_assert( "test_type1_t::two" == long_name<test_type1_t::two>() );
        static_assert( "test_type1_t::three" == long_name<test_type1_t::three>() );
        static_assert( "one" == name<test_type1_t::one>() );
        static_assert( "two" == name<test_type1_t::two>() );
        static_assert( "three" == name<test_type1_t::three>() );

        REQUIRE( "test_type1_t::one" == long_name<test_type1_t::one>() );
        REQUIRE( "one" == name<test_type1_t::one>() );
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
        static_assert( 4 == test_type2_t_info_t::size() );
        static_assert( "one" == name(test_type2_t::one) );
        static_assert( "test_type2_t::one" == long_name(test_type2_t::one) );

        REQUIRE( "one" == name(test_type2_t::one) );
        REQUIRE( "test_type2_t::one" == long_name(test_type2_t::one) );
        REQUIRE( "one" == to_string(test_type2_t::one) );
    }
    {
        static_assert( 4 == test_type3_t_info_t::size() );
        static_assert( "one" == name(test_type3_t::one) );
        // static_assert( "one" == to_string(test_type3_t::one) );
        static_assert( "test_type3_t::one" == long_name(test_type3_t::one) );
        REQUIRE( "one" == name(test_type3_t::one) );
        REQUIRE( "test_type3_t::one" == long_name(test_type3_t::one) );

        REQUIRE( "[one]" == to_string(test_type3_t::one) );

        {
            using namespace jau::enums;

            REQUIRE( "[one, two]" == to_string(test_type3_t::one | test_type3_t::two) );
            REQUIRE( "[one, two, three]" == to_string(test_type3_t::one | test_type3_t::two | test_type3_t::three) );
        }
    }
    {
        test_enum_info<test_type1_t_info_t>(4);
        test_enum_info<test_type2_t_info_t>(4);
        test_enum_info<test_type3_t_info_t>(4);
        test_enum_info<jau::io::fs::fmode_t_info_t>(10);
        test_enum_info<jau::io::fs::mountflags_linux_info_t>(27);

    }
}

namespace test::local {
    // Define the `enum class` yourself ...
    enum class test_type4_t : uint8_t {
        none = 0, // <no value item denoting no value
        one = 1 << 0,
        two = 1 << 1,
        three = 1 << 2
    };
    // and add the `enum class` support functions
    JAU_MAKE_BITFIELD_ENUM_STRING(test_type4_t, one, two, three);
    JAU_MAKE_ENUM_INFO(test_type4_t, one, two, three);

    // Define the `enum class` yourself ...
    enum class test_type5_t : uint8_t {
        none = 0, // <no value item denoting no value
        one = 10,
        two = 20,
        three = 30
    };
    // and add the `enum class` support functions
    JAU_MAKE_ENUM_STRING(test_type5_t, one, two, three);
    JAU_MAKE_ENUM_INFO(test_type5_t, one, two, three);
}

TEST_CASE( "Enum Class Value Type Test 11", "[enum][type]" ) {
    {
        using namespace test::local;
        static_assert( 3 == test_type4_t_info_t::size() );
        static_assert( "one" == name(test_type4_t::one) );
        // static_assert( "one" == to_string(test_type4_t::one) );
        static_assert( "test_type4_t::one" == long_name(test_type4_t::one) );
        REQUIRE( "one" == name(test_type4_t::one) );
        REQUIRE( "test_type4_t::one" == long_name(test_type4_t::one) );

        REQUIRE( "[one]" == to_string(test_type4_t::one) );

        {
            using namespace jau::enums;
            REQUIRE( "[one, two]" == to_string(test_type4_t::one | test_type4_t::two) );
            REQUIRE( "[one, two, three]" == to_string(test_type4_t::one | test_type4_t::two | test_type4_t::three) );
        }
    }
    {
        using namespace test::local;

        static_assert( 3 == test_type5_t_info_t::size() );
        static_assert( "one" == name(test_type5_t::one) );
        static_assert( "test_type5_t::one" == long_name(test_type5_t::one) );
        REQUIRE( "one" == name(test_type5_t::one) );
        REQUIRE( "test_type5_t::one" == long_name(test_type5_t::one) );

        REQUIRE( "one" == to_string(test_type5_t::one) );

        using namespace jau::enums;
        static_assert( 10 == number(test_type5_t::one) );
        static_assert( 20 == *test_type5_t::two );
        static_assert( 30 == *test_type5_t::three );
    }
}