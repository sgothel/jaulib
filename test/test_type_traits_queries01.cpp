/*
 * Multiple Authors: firda (post @ stackoverflow),
 *                   Sven Gothel <sgothel@jausoft.com>
 *
 * Editor: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021-2024 Gothel Software e.K.
 * Copyright (c) 2021 The Authors (see above)
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
#include <iostream>

#include <jau/test/catch2_ext.hpp>

#include <jau/type_traits_queries.hpp>

using namespace jau;

struct One {
    typedef int type;
    static constexpr bool v = true;
    type x;
    One(type x_ = 0): x(x_) {}
    ~One() = default;
    type get() { return x; }
    type add(type x_, type y_) { return x_+y_; }
};
struct Two: One {};
struct Not {};

#if 0
TYPEDEF_CHECKER(has_type, type);
TYPEDEF_CHECKER_ANY(any_type, type);
TYPEDEF_CHECKER_STRICT(exact_type, type);
MVALUE_CHECKER(true_v, v, true);
MVALUE_CHECKER(true_z, z, true);
MVALUE_CHECKER(false_v, v, false);
MVALUE_CHECKER(one_v, v, 1);
MVALUE_CHECKER_STRICT(exact_v, v, 1);
#endif

MTYPE_CHECKER(has_x, x);
MTYPE_CHECKER_ANY(any_x, x);
MTYPE_CHECKER_STRICT(exact_x, x);

METHOD_CHECKER(has_get, get, long, ());
METHOD_CHECKER(has_add, add, long, (1,2))
METHOD_CHECKER_ANY(any_get, get, ());
METHOD_CHECKER_STRICT_RET(int_get, get, int, ())
METHOD_CHECKER_STRICT_RET(long_get, get, long, ())

template<template<typename, typename...> class TT, class U, typename... V>
static void check_2_sub(const std::string &tname, const std::string &desc) {
	std::cout << tname << " " << (TT<U, V...>() ? "has " : "does not have ") << desc << std::endl;
}

template<template<typename, typename...> class T, typename... V>
static void check_2(const std::string &desc) {
	std::cout << std::endl;
	check_2_sub<T, One, V...>("One", desc);
	check_2_sub<T, Two, V...>("Two", desc);
	check_2_sub<T, Not, V...>("Not", desc);
	check_2_sub<T, int, V...>("int", desc);
}

TEST_CASE( "01 Type Traits Queries") {
    std::string sep = std::string(60, '-');
#if 0
    std::cout << sep;
    check_2<any_type>("typedef type");
    check_2<has_type, long>("typedef type convertible to long");
    check_2<exact_type, int>("typedef type = int");
    check_2<exact_type, long>("typedef type = long");

    std::cout << sep;
    check_2<true_v>("var v with value equal to true");
    check_2<true_z>("var z with value equal to true");
    check_2<false_v>("var v with value equal to false");
    check_2<one_v>("var v with value equal to 1");
    check_2<exact_v>("var v with value equal to 1 of type int");
#endif

    std::cout << sep;
 	check_2<any_x>("var x");
    check_2<has_x, long>("var x of type convertible to long");
    check_2<exact_x, int>("var x of type int");
    check_2<exact_x, long>("var x of type long");

    std::cout << sep;
    check_2<has_get>("get()");
    check_2<has_get>("get() with return type covertible to long");
    check_2<has_add>("add() accepting two ints and returning ~ long");
    check_2<int_get>("int get()");
    check_2<long_get>("long get()");
}
