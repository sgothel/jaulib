/*
 * Multiple Authors: firda (post @ stackoverflow),
 *                   Sven Gothel <sgothel@jausoft.com>
 *
 * Editor: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021 Gothel Software e.K.
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
    ~One() {}
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

TEST_CASE( "01 Type Traits Queries") {
    #define CHECK_2(name, desc, ...) std::cout << std::endl; \
    std::cout << "One " << (name<One, ##__VA_ARGS__>() ? "has " : "does not have ") << desc << std::endl; \
    std::cout << "Two " << (name<Two, ##__VA_ARGS__>() ? "has " : "does not have ") << desc << std::endl; \
    std::cout << "Not " << (name<Not, ##__VA_ARGS__>() ? "has " : "does not have ") << desc << std::endl; \
    std::cout << "int " << (name<int, ##__VA_ARGS__>() ? "has " : "does not have ") << desc << std::endl

    std::string sep = std::string(60, '-');
#if 0
    std::cout << sep;
    CHECK_2(any_type, "typedef type");
    CHECK_2(has_type, "typedef type convertible to long", long);
    CHECK_2(exact_type, "typedef type = int", int);
    CHECK_2(exact_type, "typedef type = long", long);

    std::cout << sep;
    CHECK_2(true_v, "var v with value equal to true");
    CHECK_2(true_z, "var z with value equal to true");
    CHECK_2(false_v, "var v with value equal to false");
    CHECK_2(one_v, "var v with value equal to 1");
    CHECK_2(exact_v, "var v with value equal to 1 of type int");
#endif

    std::cout << sep;
    CHECK_2(any_x, "var x");
    CHECK_2(has_x, "var x of type convertible to long", long);
    CHECK_2(exact_x, "var x of type int", int);
    CHECK_2(exact_x, "var x of type long", long);

    std::cout << sep;
    CHECK_2(has_get, "get()");
    CHECK_2(has_get, "get() with return type covertible to long");
    CHECK_2(has_add, "add() accepting two ints and returning ~ long");
    CHECK_2(int_get, "int get()");
    CHECK_2(long_get, "long get()");
}
