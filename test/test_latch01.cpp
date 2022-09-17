/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021 Gothel Software e.K.
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

#include <jau/latch.hpp>

using namespace jau;
using namespace jau::fractions_i64_literals;
using namespace jau::int_literals;

class TestLatch01 {
  private:
    jau::relaxed_atomic_int my_counter = 0;

    void something(jau::latch& l, const fraction_i64 duration) {
        my_counter++;
        jau::sleep_for( duration );
        l.count_down();
    }

    void somethingUp2x(jau::latch& l, const fraction_i64 duration) {
        l.count_up();
        l.count_up();
        my_counter++;
        jau::sleep_for( duration );
        l.count_down();
        my_counter++;
        jau::sleep_for( duration );
        l.count_down();
    }

  public:

    /**
     * Testing jau::latch with set initial count value, count_down() and arrive_and_wait().
     */
    void test01_down_wait() {
        INFO_STR("\n\ntest01\n");
        const size_t count = 10;
        std::thread tasks[count];
        jau::latch completion(count+1);

        REQUIRE_MSG("not-zero", count+1 == completion.value());

        for(size_t i=0; i<count; i++) {
            tasks[i] = std::thread(&TestLatch01::something, this, std::ref(completion), (int64_t)i*1_ms);
        }
        completion.arrive_and_wait();

        REQUIRE_MSG("zero", 0 == completion.value());
        REQUIRE_MSG("10", count == my_counter);

        for(size_t i=0; i<count; i++) {
            if( tasks[i].joinable() ) {
                tasks[i].join();
            }
        }
    }

    /**
     * Testing jau::latch with set initial count value, count_down() and arrive_and_wait_for().
     */
    void test02_down_wait_for() {
        INFO_STR("\n\ntest02\n");
        const size_t count = 10;
        std::thread tasks[count];
        jau::latch completion(count+1);

        REQUIRE_MSG("not-zero", count+1 == completion.value());

        for(size_t i=0; i<count; i++) {
            tasks[i] = std::thread(&TestLatch01::something, this, std::ref(completion), (int64_t)i*1_ms);
        }
        REQUIRE_MSG("complete", true == completion.arrive_and_wait_for(10_s) );

        REQUIRE_MSG("zero", 0 == completion.value());
        REQUIRE_MSG("10", count == my_counter);

        for(size_t i=0; i<count; i++) {
            if( tasks[i].joinable() ) {
                tasks[i].join();
            }
        }
    }

    /**
     * Testing jau::latch default ctor with zero value, then set initial count value, count_down() and arrive_and_wait().
     */
    void test03_down_wait_for() {
        INFO_STR("\n\ntest03\n");
        const size_t count = 10;
        std::thread tasks[count];
        jau::latch completion;

        REQUIRE_MSG("zero", 0 == completion.value());

        completion.set(count+1);
        REQUIRE_MSG("not-zero", count+1 == completion.value());

        for(size_t i=0; i<count; i++) {
            tasks[i] = std::thread(&TestLatch01::something, this, std::ref(completion), (int64_t)i*1_ms);
        }
        REQUIRE_MSG("complete", true == completion.arrive_and_wait_for(10_s) );

        REQUIRE_MSG("zero", 0 == completion.value());
        REQUIRE_MSG("10", count == my_counter);

        for(size_t i=0; i<count; i++) {
            if( tasks[i].joinable() ) {
                tasks[i].join();
            }
        }
    }


    /**
     * Testing jau::latch with zero initial count value, count_up(), count_down() and wait().
     */
    void test04_up_wait_for() {
        INFO_STR("\n\ntest04\n");
        const size_t count = 10;
        std::thread tasks[count];
        jau::latch completion;

        REQUIRE_MSG("zero", 0 == completion.value());

        for(size_t i=0; i<count/2; i++) {
            tasks[i] = std::thread(&TestLatch01::somethingUp2x, this, std::ref(completion), (int64_t)i*1_ms);
        }
        REQUIRE_MSG("not-zero", 0 < completion.value()); // at least one count_up() occurred

        REQUIRE_MSG("complete", true == completion.arrive_and_wait_for(10_s) );

        REQUIRE_MSG("zero", 0 == completion.value());
        REQUIRE_MSG("10", count == my_counter);

        for(size_t i=0; i<count; i++) {
            if( tasks[i].joinable() ) {
                tasks[i].join();
            }
        }
    }
};

METHOD_AS_TEST_CASE( TestLatch01::test01_down_wait,      "test01_down_wait");
METHOD_AS_TEST_CASE( TestLatch01::test02_down_wait_for,  "test02_down_wait_for");
METHOD_AS_TEST_CASE( TestLatch01::test03_down_wait_for,  "test03_down_wait_for");
METHOD_AS_TEST_CASE( TestLatch01::test04_up_wait_for,    "test04_up_wait_for");
