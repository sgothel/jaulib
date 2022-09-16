/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2022 Gothel Software e.K.
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
#include <memory>
#include <thread>
#include <pthread.h>

#include <jau/test/catch2_ext.hpp>

#include <jau/simple_timer.hpp>

using namespace jau;
using namespace jau::fractions_i64_literals;
using namespace jau::int_literals;

class TestSimpleTimer01 {
  private:
    // install it once ..
    bool sighandler_once = jau::service_runner::singleton_sighandler();;

    int dog_count = 0;
    const jau::fraction_i64 dog_period = 10_ms;
    const jau::fraction_i64 test_period = 100_ms;

    jau::sc_atomic_int ping_count = 0;
    jau::simple_timer periodic_dog = jau::simple_timer("dog-"+std::to_string(dog_count), 100_ms /* shutdown timeout */);

    jau::fraction_i64 dog_watch_func(jau::simple_timer& timer) {
        static jau::fraction_timespec t0;

        if( timer.shall_stop() ) {
            return 0_s;
        }
        jau::fraction_timespec now = jau::getMonotonicTime();
        jau::fraction_i64 td = ( now - t0 ).to_fraction_i64();
        t0 = now;
        fprintf(stderr, "%3.3d dog is watching: Since last ping %" PRIi64 " us\n", ping_count++, td.to_num_of(1_us));

        return timer.shall_stop() ? 0_s : dog_period;
    }

  public:

    void test01_dog1() {
        INFO_STR("\n\ntest01\n");

        fprintf(stderr, "test01_dog1: start\n");
        REQUIRE( 0 == ping_count );
        REQUIRE( false == periodic_dog.is_running() );
        REQUIRE( true == periodic_dog.shall_stop() );
        const bool r = periodic_dog.start(dog_period, jau::bind_member(this, &TestSimpleTimer01::dog_watch_func));
        REQUIRE( true == r );
        REQUIRE( true  == periodic_dog.is_running() );
        REQUIRE( false == periodic_dog.shall_stop() );

        {
            jau::fraction_timespec t0 = jau::getMonotonicTime();
            jau::sleep_for( test_period );
            jau::fraction_i64 td = ( jau::getMonotonicTime() - t0 ).to_fraction_i64();
            REQUIRE( td <= test_period + 50_ms ); // allow some fuzzy
        }
        REQUIRE( true  == periodic_dog.is_running() );
        REQUIRE( false == periodic_dog.shall_stop() );

        {
            jau::fraction_timespec t0 = jau::getMonotonicTime();
            REQUIRE( true == periodic_dog.stop() );
            jau::fraction_i64 td = ( jau::getMonotonicTime() - t0 ).to_fraction_i64();
            REQUIRE( td <= 100_ms );
        }
        fprintf(stderr, "test01_dog1: stopped\n");
        REQUIRE( false  == periodic_dog.is_running() );
        REQUIRE( true == periodic_dog.shall_stop() );
        REQUIRE( 0 < ping_count );
        ping_count = 0;
    }

    void test01_dog2() {
        INFO_STR("\n\ntest01\n");

        fprintf(stderr, "test01_dog2: start\n");
        REQUIRE( 0 == ping_count );
        REQUIRE( false == periodic_dog.is_running() );
        REQUIRE( true == periodic_dog.shall_stop() );
        const bool r = periodic_dog.start(dog_period, jau::bind_member(this, &TestSimpleTimer01::dog_watch_func));
        REQUIRE( true == r );
        REQUIRE( true  == periodic_dog.is_running() );
        REQUIRE( false == periodic_dog.shall_stop() );

        {
            jau::fraction_timespec t0 = jau::getMonotonicTime();
            jau::sleep_for( test_period );
            jau::fraction_i64 td = ( jau::getMonotonicTime() - t0 ).to_fraction_i64();
            REQUIRE( td <= test_period + 50_ms ); // allow some fuzzy
        }
        REQUIRE( true  == periodic_dog.is_running() );
        REQUIRE( false == periodic_dog.shall_stop() );

        {
            jau::fraction_timespec t0 = jau::getMonotonicTime();
            REQUIRE( true == periodic_dog.stop() );
            jau::fraction_i64 td = ( jau::getMonotonicTime() - t0 ).to_fraction_i64();
            REQUIRE( td <= 100_ms );
        }
        fprintf(stderr, "test01_dog2: stopped\n");
        REQUIRE( false  == periodic_dog.is_running() );
        REQUIRE( true == periodic_dog.shall_stop() );
        REQUIRE( 0 < ping_count );
    }

};

METHOD_AS_TEST_CASE( TestSimpleTimer01::test01_dog1, "Test TestSimpleTimer01 - test01_dog1");
METHOD_AS_TEST_CASE( TestSimpleTimer01::test01_dog2, "Test TestSimpleTimer01 - test01_dog2");

