#include <cassert>
#include <cstring>

#include <jau/test/catch2_ext.hpp>

#include <jau/basic_types.hpp>
#include <jau/io/eui48.hpp>
#include <jau/darray.hpp>

using namespace jau;
using namespace jau::io::net;

static void sum_vec_nothrow(int &sum, const std::vector<int>& v) {
    jau::for_each_const(v, [&sum](const int& i) noexcept {
            sum+=i;
        });

}
static void sum_vec_throw(int &sum, const std::vector<int>& v) {
    jau::for_each_const(v, [&sum](const int& i) {
            sum+=i;
        });

}
static void sum_cow_nothrow(int &sum, const jau::darray<int>& v) {
    jau::for_each_const(v, [&sum](const int& i) noexcept {
            sum+=i;
        });

}
static void sum_cow_throw(int &sum, const jau::darray<int>& v) {
    jau::for_each_const(v, [&sum](const int& i) {
            sum+=i;
        });

}

static int global_sum = 0;
static void add_vec_nothrow(const int &i) noexcept {
    global_sum += i;
}
static void add_vec_throw(const int &i) {
    global_sum += i;
}

TEST_CASE( "for_each_const Test 01", "[algos][for_each_const][nothrow_function][throw_function][function]" ) {
    {
        // nothrow
        std::vector<int> v = { 1, 2, 3 };
        int sum=0;
        jau::for_each_const(v, [&sum](int i) noexcept {
                sum+=i;
            });
        REQUIRE(sum == 6);

        global_sum=0;
        jau::for_each_const(v, add_vec_nothrow);
        REQUIRE(global_sum == 6);

        sum=0;
        sum_vec_nothrow(sum, v);
        REQUIRE(sum == 6);

        // throw
        sum=0;
        jau::for_each_const(v, [&sum](int i) {
                sum+=i;
            });
        REQUIRE(sum == 6);

        global_sum=0;
        jau::for_each_const(v, add_vec_throw);
        REQUIRE(global_sum == 6);

        sum=0;
        sum_vec_throw(sum, v);
        REQUIRE(sum == 6);
    }
    {
        jau::darray<int> v = { 1, 2, 3 };
        int sum=0;
        jau::for_each_const(v, [&sum](int i) noexcept {
                sum+=i;
            });
        REQUIRE(sum == 6);
    }
    {
        jau::darray<int> v = { 1, 2, 3 };
        int sum=0;
        jau::for_each_const(v, [&sum](int i) {
                sum+=i;
            });
        REQUIRE(sum == 6);
    }
    {
        jau::darray<int> v = { 1, 2, 3 };
        int sum=0;
        sum_cow_nothrow(sum, v);
        REQUIRE(sum == 6);
        sum=0;
        sum_cow_throw(sum, v);
        REQUIRE(sum == 6);
    }
}
