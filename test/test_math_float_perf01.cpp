/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright Gothel Software e.K.
 *
 * SPDX-License-Identifier: MIT
 *
 * This Source Code Form is subject to the terms of the MIT License
 * If a copy of the MIT was not distributed with this file,
 * you can obtain one at https://opensource.org/license/mit/.
 */
#include <cassert>
#include <cstring>

#include <jau/math/geom/aabbox2f.hpp>
#include <jau/test/catch2_ext.hpp>

#include <jau/math/vec2f.hpp>

using namespace jau;
using namespace jau::int_literals;

using namespace jau::math;
using namespace jau::math::geom;

struct AABBox {
    Point2f lo, hi;

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
        const Point2f lo_ = max(lo, o.lo);
        const Point2f hi_ = min(hi, o.hi);
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
        const Point2f lo_ = max(lo, o.lo);
        const Point2f hi_ = min(hi, o.hi);
        return lo_.x <= hi_.x && lo_.y <= hi_.y;
    }
};

std::ostream& operator<<(std::ostream& out, const AABBox& v) noexcept {
    return out << "aabb[bl " << v.lo << ", tr " << v.hi << "]";
}

#include <random>

TEST_CASE( "Float Math Bench 04a", "[intersect][benchmark][arithmetic][math]" ) {
    std::mt19937 rng;
    int32_t seed_val=0;
    rng.seed(seed_val);
    std::uniform_int_distribution<int32_t> rint(0,50);

    const int loops = catch_auto_run ? 1000 : 1000000;
    size_t isect_count=0;
    std::vector<AABBox2f> va0, vb0;
    std::vector<AABBox> va, vb;
    for(int i=0; i<loops; ++i) {
        Point2f lo((float)rint(rng), (float)rint(rng));
        Point2f hi(lo.x+(float)rint(rng), lo.y+(float)rint(rng));
        AABBox a { lo, hi };
        AABBox2f a0 { lo, hi };
        lo = Point2f((float)rint(rng), (float)rint(rng));
        hi = Point2f(lo.x+(float)rint(rng), lo.y+(float)rint(rng));
        AABBox2f b0 { lo, hi };
        AABBox b { lo, hi };
        va0.push_back(a0);
        vb0.push_back(b0);
        va.push_back(a);
        vb.push_back(b);
        bool i0 = a0.intersects(b0);
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
        REQUIRE( i1a == i0 );
        // std::cout << std::endl;
        bool i2a = a.intersects2a(b);
        bool i2b = a.intersects2b(b);
        bool i2c = a.intersects2c(b);
        REQUIRE( i1a == i2a );
        REQUIRE( i2a == i2b );
        REQUIRE( i2a == i2c );
        REQUIRE( i2a == i0 );
    }
    std::cout << "isect_count " << isect_count << "/" << va.size() << ", " << 100.0f*( (float)isect_count / (float)va.size() ) << "%" << std::endl;

    BENCHMARK("Intersect0 Benchmark") {
        size_t r = 0;
        for(size_t i = 0; i < va0.size(); ++i) {
            AABBox2f a = va0[i];
            AABBox2f b = vb0[i];
            r += a.intersects(b) ? 10 : 1;
        }
        return r;
    };
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

