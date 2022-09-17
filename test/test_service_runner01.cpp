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

#include <jau/service_runner.hpp>
#include <jau/latch.hpp>

using namespace jau;
using namespace jau::fractions_i64_literals;
using namespace jau::int_literals;

class TestServiceRunner01 {
  private:
    // install it once ..
    bool sighandler_once = jau::service_runner::singleton_sighandler();;

    jau::sc_atomic_int ping_count = 0;

    jau::latch serviceInitDone;
    jau::latch serviceEndDone;
    jau::latch serviceWorkDone;

    void serviceCounterInit(jau::service_runner& sr0) noexcept {
        (void)sr0;
        serviceInitDone.count_down();
    }

    void serviceCounterEnd(jau::service_runner& sr) noexcept {
        (void)sr;
        serviceEndDone.count_down();
    }

    // immediately self stopping after start, testing service_runner::start()
    void service01FastStopWork(jau::service_runner& sr) noexcept {
        sr.set_shall_stop(); // trigger service_runner::start() issue: running == false and shall_stop == true before start queries while running != true

        ping_count++;
        serviceWorkDone.count_down();
    }

    // self stopping after work
    void service10CounterWork(jau::service_runner& sr) noexcept {
        ping_count++;
        serviceWorkDone.count_down();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        if( 0 == serviceWorkDone.value() ) {
            sr.set_shall_stop();
        }
    }

  public:

    // immediately self stopping after start, testing service_runner::start()
    void test01_service01_fast_stop() {
        fprintf(stderr, "\n\ntest01\n");

        ping_count = 0;
        serviceInitDone.set(1);
        serviceEndDone.set(1);
        serviceWorkDone.set(1);

        jau::service_runner service("service_01", 100_ms,
                jau::bind_member(this, &TestServiceRunner01::service01FastStopWork),
                jau::bind_member(this, &TestServiceRunner01::serviceCounterInit),
                jau::bind_member(this, &TestServiceRunner01::serviceCounterEnd));

        REQUIRE(  0 == ping_count.load() );
        REQUIRE(  1 == serviceInitDone.value());
        REQUIRE(  1 == serviceWorkDone.value());
        REQUIRE(  1 == serviceEndDone.value());

        fprintf(stderr, "test01: start: %s\n", service.toString().c_str());
        service.start();

        REQUIRE_MSG("service10_init_complete", true == serviceInitDone.wait_for(100_ms) );
        REQUIRE( true == serviceWorkDone.wait_for(2_s) );
        REQUIRE_MSG("service10_end_complete", true == serviceEndDone.wait_for(100_ms) );
        fprintf(stderr, "test01: latched: %zu\n", serviceWorkDone.value());
        fprintf(stderr, "test01: latched: %s\n", service.toString().c_str());
        REQUIRE(  1 == ping_count.load() );
        REQUIRE(  0 == serviceInitDone.value());
        REQUIRE(  0 == serviceWorkDone.value());
        REQUIRE(  0 == serviceEndDone.value());

        REQUIRE( true == service.stop() );
    }

    // 10'000x immediately self stopping after start - faster, testing service_runner::start()
    void test02_service01_fast_stop() {
        fprintf(stderr, "\n\ntest02\n");

        const int loops = 10000;
        for(int i=0; i<loops; ++i) {
            ping_count = 0;
            serviceWorkDone.set(1);

            jau::service_runner service("service_01", 100_ms,
                    jau::bind_member(this, &TestServiceRunner01::service01FastStopWork));

            REQUIRE(  0 == ping_count.load() );
            REQUIRE(  1 == serviceWorkDone.value());

            // fprintf(stderr, "test02: start %d: %s\n", i, service.toString().c_str());
            service.start();

            REQUIRE( true == serviceWorkDone.wait_for(2_s) );
            REQUIRE(  1 == ping_count.load() );
            REQUIRE(  0 == serviceWorkDone.value());

            REQUIRE( true == service.stop() );
        }
    }

    // self stopping after work
    void test10_service01_self_stop() {
        fprintf(stderr, "\n\ntest10\n");

        ping_count = 0;
        serviceInitDone.set(1);
        serviceEndDone.set(1);
        serviceWorkDone.set(10);

        jau::service_runner service("service_10", 100_ms,
                      jau::bind_member(this, &TestServiceRunner01::service10CounterWork),
                      jau::bind_member(this, &TestServiceRunner01::serviceCounterInit),
                      jau::bind_member(this, &TestServiceRunner01::serviceCounterEnd));

        REQUIRE(  0 == ping_count.load() );
        REQUIRE(  1 == serviceInitDone.value());
        REQUIRE( 10 == serviceWorkDone.value());
        REQUIRE(  1 == serviceEndDone.value());

        fprintf(stderr, "test10: start: %s\n", service.toString().c_str());
        service.start();

        REQUIRE_MSG("service10_init_complete", true == serviceInitDone.wait_for(100_ms) );
        REQUIRE_MSG("service10_work_complete", true == serviceWorkDone.wait_for(500_ms) );
        REQUIRE_MSG("service10_end_complete", true == serviceEndDone.wait_for(100_ms) );
        fprintf(stderr, "test10: latched: %zu\n", serviceWorkDone.value());
        fprintf(stderr, "test10: latched: %s\n", service.toString().c_str());
        REQUIRE( 10 == ping_count.load() );
        REQUIRE(  0 == serviceInitDone.value());
        REQUIRE(  0 == serviceWorkDone.value());
        REQUIRE(  0 == serviceEndDone.value());

        REQUIRE( true == service.stop() );
    }


};

METHOD_AS_TEST_CASE( TestServiceRunner01::test01_service01_fast_stop, "test01_service01_fast_stop");
METHOD_AS_TEST_CASE( TestServiceRunner01::test02_service01_fast_stop, "test02_service01_fast_stop");
METHOD_AS_TEST_CASE( TestServiceRunner01::test10_service01_self_stop, "test10_service01_self_stop");

