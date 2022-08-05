/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020 Gothel Software e.K.
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

#include <jau/float_math.hpp>
#include <jau/basic_types.hpp>

using namespace jau;

TEST_CASE( "Float Epsilon Test 01", "[datatype][float][epsilon]" ) {
    static float epsilon_f0 = std::numeric_limits<float>::epsilon();
    static double epsilon_d0 = std::numeric_limits<double>::epsilon();

    static float epsilon_f1 = jau::machineEpsilon<float>();
    static double epsilon_d1 = jau::machineEpsilon<double>();

    float epsilon_fd = epsilon_f1 - epsilon_f0;
    float epsilon_dd = epsilon_d1 - epsilon_d0;

    fprintf(stderr, "std::numeric_limits<float>::epsilon()  : %e\n", epsilon_f0);
    fprintf(stderr, "std::numeric_limits<double>::epsilon() : %le\n", epsilon_d0);
    fprintf(stderr, "jau::machineEpsilon<float>()           : %e\n", epsilon_f1);
    fprintf(stderr, "jau::machineEpsilon<double>()          : %le\n", epsilon_d1);

    fprintf(stderr, "float:  approximation - numeric_limits : %e\n", epsilon_fd);
    fprintf(stderr, "double: approximation - numeric_limits : %le\n", epsilon_dd);

    REQUIRE(jau::machine_equal(epsilon_f1, epsilon_f0, 5));
    REQUIRE(jau::machine_equal(epsilon_d1, epsilon_d0, 5));
}


