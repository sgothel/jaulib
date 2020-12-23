#include <iostream>
#include <cassert>
#include <cinttypes>
#include <cstring>

#define CATCH_CONFIG_MAIN
#include <catch2/catch_amalgamated.hpp>
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


