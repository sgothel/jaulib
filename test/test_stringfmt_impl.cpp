/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2024 Gothel Software e.K.
 *
 * ***
 *
 * SPDX-License-Identifier: MIT
 *
 * This Source Code Form is subject to the terms of the MIT License
 * If a copy of the MIT was not distributed with this
 * file, You can obtain one at https://opensource.org/license/mit/.
 *
 */
#include <sys/types.h>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>

#include <jau/basic_types.hpp>
#include <jau/cpp_lang_util.hpp>
#include <jau/cpp_pragma.hpp>
#include <jau/float_types.hpp>
#include <jau/int_types.hpp>
#include <jau/string_cfmt.hpp>
#include <jau/string_literal.hpp>
#include <jau/string_util.hpp>
#include <jau/test/catch2_ext.hpp>
#include <jau/type_concepts.hpp>
#include <jau/type_traits_queries.hpp>

#ifdef HAS_STD_FORMAT
    #include <format>
#endif

using namespace std::literals;

using namespace jau::float_literals;

using namespace jau::int_literals;

TEST_CASE("jau_cfmt_append_integral01", "[benchmark][jau][std::string][format_string]") {
    // const uint64_t i1 = std::numeric_limits<uint64_t>::max(); // Value = 18446744073709551615 (0xffffffffffffffff)
    // static constexpr const char *format_check_exp1 = "    0000000018'446'744'073'709'551'615";
    // static constexpr const char *format_check_exp0 = "    0000000000000018446744073709551615";

    {
        // Case 1: clang-analyzer-core.uninitialized.Assign: buf_ was written up-to val_digits! (Assigned value is garbage or undefined)
        // This analysis issue was only produced with preset `debug-clang`, not `release-clang`.
        static constexpr const char *format_exp = "-0";
        // radix 2
        // no flags_t::uppercase
        // no flags_t::thousands
        // no width
        // no precision
        // value = 0 ??? -> val_digits 1
        // negative = true
        // no inject_dot
        const uint64_t value = 0;
        const bool negative = true;
        const bool inject_dot = false;
        jau::cfmt::FormatOpts opts;
        opts.length_mod = jau::cfmt::plength_t::z;
        opts.setConversion('b');
        std::cout << "FormatOpts: " << opts << "\n";
        REQUIRE(2 == opts.radix);

        {
            std::string s;
            s.reserve(jau::cfmt::default_string_capacity + 1);

            jau::cfmt::impl::append_integral(s, s.max_size(), value, negative, opts, inject_dot);
            REQUIRE(format_exp == s);
        }
        {
            std::string s;
            s.reserve(jau::cfmt::default_string_capacity + 1);

            jau::cfmt::impl::append_integral_simple(s, s.max_size(), value, negative, opts);
            REQUIRE(format_exp == s);
        }
    }
}
