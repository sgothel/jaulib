/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2024 Gothel Software e.K.
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
#include <cstring>

#include <jau/test/catch2_ext.hpp>

#include <jau/int_math.hpp>
#include <jau/math/vec2i.hpp>

using namespace jau;
using namespace jau::int_literals;

TEST_CASE( "Int Math Bench 01a", "[abs][benchmark][arithmetic][math]" ) {
    BENCHMARK("jau::abs Benchmark") {
        REQUIRE( 1 == jau::abs( 1) );
        REQUIRE( 1 == jau::abs(-1) );
        REQUIRE( 1_i64 == jau::abs( 1_i64) );
        REQUIRE( 1_i64 == jau::abs(-1_i64) );
        REQUIRE( std::numeric_limits<int64_t>::max()  == jau::abs( std::numeric_limits<int64_t>::max() ) );
        REQUIRE( INT32_MAX  == jau::abs( INT32_MAX ) );
    };
}
TEST_CASE( "Int Math Bench 01b", "[ct_abs][benchmark][arithmetic][math]" ) {
    BENCHMARK("jau::ct_abs Benchmark") {
        REQUIRE( 1 == jau::ct_abs( 1) );
        REQUIRE( 1 == jau::ct_abs(-1) );
        REQUIRE( 1_i64 == jau::ct_abs( 1_i64) );
        REQUIRE( 1_i64 == jau::ct_abs(-1_i64) );
        REQUIRE( std::numeric_limits<int64_t>::max()  == jau::ct_abs( std::numeric_limits<int64_t>::max() ) );
        // REQUIRE( std::numeric_limits<int64_t>::max()  == jau::ct_abs( std::numeric_limits<int64_t>::min() ) ); // UB
        REQUIRE( INT32_MAX  == jau::ct_abs( INT32_MAX ) );
        // REQUIRE( INT32_MAX  == jau::ct_abs( INT32_MIN ) ); // UB
    };
}
TEST_CASE( "Int Math Bench 01c", "[abs][benchmark][arithmetic][math]" ) {
    BENCHMARK("std::abs Benchmark") {
        REQUIRE( 1 == std::abs( 1) );
        REQUIRE( 1 == std::abs(-1) );
        REQUIRE( 1_i64 == std::abs( 1_i64) );
        REQUIRE( 1_i64 == std::abs(-1_i64) );
        REQUIRE( std::numeric_limits<int64_t>::max()  == std::abs( std::numeric_limits<int64_t>::max() ) );
        REQUIRE( INT32_MAX  == std::abs( INT32_MAX ) );
    };
}

TEST_CASE( "Int Math Bench 02a", "[min][max][benchmark][arithmetic][math]" ) {
    BENCHMARK("MinMax Benchmark") {
        REQUIRE(         0  == jau::min( 0, INT32_MAX ) );
        REQUIRE( INT32_MAX  == jau::max( 0, INT32_MAX ) );
        REQUIRE( INT32_MAX-1== jau::min( INT32_MAX-1, INT32_MAX ) );
        REQUIRE( INT32_MAX  == jau::max( INT32_MAX-1, INT32_MAX ) );
        REQUIRE( INT32_MIN  == jau::min( 0, INT32_MIN ) );
        REQUIRE(         0  == jau::max( 0, INT32_MIN ) );
        REQUIRE( INT32_MIN  == jau::min( INT32_MIN+1, INT32_MIN ) );
        REQUIRE( INT32_MIN+1== jau::max( INT32_MIN+1, INT32_MIN ) );
    };
}
TEST_CASE( "Int Math Bench 03a", "[ct_min][ct_max][benchmark][arithmetic][math]" ) {
    BENCHMARK("Min2Max2 Benchmark") {
        REQUIRE(         0  == jau::ct_min( 0, INT32_MAX ) );
        REQUIRE( INT32_MAX  == jau::ct_max( 0, INT32_MAX ) );
        REQUIRE( INT32_MAX-1== jau::ct_min( INT32_MAX-1, INT32_MAX ) );
        REQUIRE( INT32_MAX  == jau::ct_max( INT32_MAX-1, INT32_MAX ) );
        REQUIRE( INT32_MIN+1  == jau::ct_min( 0, INT32_MIN+1 ) ); // limitation: `MIN <= x - y <= MAX`
        REQUIRE(         0  == jau::ct_max( 0, INT32_MIN+1 ) );   // limitation: `MIN <= x - y <= MAX`
        REQUIRE( INT32_MIN  == jau::ct_min( INT32_MIN+1, INT32_MIN ) );
        REQUIRE( INT32_MIN+1== jau::ct_max( INT32_MIN+1, INT32_MIN ) );
    };
}

using namespace jau::math;

struct AABBox {
    Point2i lo, hi;

    bool __attribute__ ((noinline))
    intersects1a(const AABBox& o) const
    {
        return hi.x >= o.lo.x &&
               hi.y >= o.lo.y &&
               lo.x <= o.hi.x &&
               lo.y <= o.hi.y;
    }
    bool __attribute__ ((noinline))
    intersects1b(const AABBox& o) const
    {
        return !( hi.x < o.lo.x ||
                  hi.y < o.lo.y ||
                  lo.x > o.hi.x ||
                  lo.y > o.hi.y );
    }
    bool __attribute__ ((noinline))
    intersects1c(const AABBox& o) const
    {
        const Point2i lo_ = max(lo, o.lo);
        const Point2i hi_ = min(hi, o.hi);
        return lo_.x <= hi_.x && lo_.y <= hi_.y;
    }

    bool intersects2a(const AABBox& o) const
    {
        return hi.x >= o.lo.x &&
               hi.y >= o.lo.y &&
               lo.x <= o.hi.x &&
               lo.y <= o.hi.y;
    }
    bool intersects2b(const AABBox& o) const
    {
        return !( hi.x < o.lo.x ||
                  hi.y < o.lo.y ||
                  lo.x > o.hi.x ||
                  lo.y > o.hi.y );
    }
    bool intersects2c(const AABBox& o) const
    {
        const Point2i lo_ = max(lo, o.lo);
        const Point2i hi_ = min(hi, o.hi);
        return lo_.x <= hi_.x && lo_.y <= hi_.y;
    }
};

#include <random>

TEST_CASE( "Int Math Bench 04a", "[intersect][benchmark][arithmetic][math]" ) {
    std::mt19937 rng;
    int32_t seed_val=0;
    rng.seed(seed_val);
    std::uniform_int_distribution<int32_t> rint(0,50);

    const int loops = catch_auto_run ? 1000 : 1000000;
    size_t isect_count=0;
    std::vector<AABBox> va, vb;
    for(int i=0; i<loops; ++i) {
        Point2i lo(rint(rng), rint(rng));
        Point2i hi(lo.x+rint(rng), lo.y+rint(rng));
        AABBox a { .lo=lo, .hi=hi };
        lo = Point2i(rint(rng), rint(rng));
        hi = Point2i(lo.x+rint(rng), lo.y+rint(rng));
        AABBox b { .lo=lo, .hi=hi };
        va.push_back(a);
        vb.push_back(b);
        bool i1a = a.intersects1a(b);
        bool i1b = a.intersects1b(b);
        bool i1c = a.intersects1c(b);
        if( i1a ) {
            ++isect_count;
        }
        // std::cout << "# " << i << std::endl;
        // std::cout << "A: " << a << std::endl;
        // std::cout << "B: " << b << ", i " << i1a << std::endl;
        REQUIRE( i1a == i1b );
        REQUIRE( i1a == i1c );
        // std::cout << std::endl;
        bool i2a = a.intersects2a(b);
        bool i2b = a.intersects2b(b);
        bool i2c = a.intersects2c(b);
        REQUIRE( i1a == i2a );
        REQUIRE( i2a == i2b );
        REQUIRE( i2a == i2c );
    }
    std::cout << "isect_count " << isect_count << "/" << va.size() << ", " << 100.0f*( (float)isect_count / (float)va.size() ) << "%" << std::endl;

    BENCHMARK("Intersect1a Benchmark") {
        size_t r = 0;
        for(size_t i = 0; i < va.size(); ++i) {
            AABBox a = va[i];
            AABBox b = vb[i];
            r += a.intersects1a(b) ? 10 : 1;
        }
        return r;
    };
    BENCHMARK("Intersect1b Benchmark") {
        size_t r = 0;
        for(size_t i = 0; i < va.size(); ++i) {
            AABBox a = va[i];
            AABBox b = vb[i];
            r += a.intersects1b(b) ? 10 : 1;
        }
        return r;
    };
    BENCHMARK("Intersect1c Benchmark") {
        size_t r = 0;
        for(size_t i = 0; i < va.size(); ++i) {
            AABBox a = va[i];
            AABBox b = vb[i];
            r += a.intersects1c(b) ? 10 : 1;
        }
        return r;
    };
    BENCHMARK("Intersect2a Benchmark") {
        size_t r = 0;
        for(size_t i = 0; i < va.size(); ++i) {
            AABBox a = va[i];
            AABBox b = vb[i];
            r += a.intersects2a(b) ? 10 : 1;
        }
        return r;
    };
    BENCHMARK("Intersect2b Benchmark") {
        size_t r = 0;
        for(size_t i = 0; i < va.size(); ++i) {
            AABBox a = va[i];
            AABBox b = vb[i];
            r += a.intersects2b(b) ? 10 : 1;
        }
        return r;
    };
    BENCHMARK("Intersect2c Benchmark") {
        size_t r = 0;
        for(size_t i = 0; i < va.size(); ++i) {
            AABBox a = va[i];
            AABBox b = vb[i];
            r += a.intersects2c(b) ? 10 : 1;
        }
        return r;
    };
}

