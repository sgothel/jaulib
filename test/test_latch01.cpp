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
#include <iostream>
#include <cassert>
#include <cinttypes>
#include <cstring>
#include <memory>
#include <thread>
#include <pthread.h>

#define CATCH_CONFIG_MAIN
#include <catch2/catch_amalgamated.hpp>
#include <jau/test/catch2_ext.hpp>

#include <jau/latch.hpp>

using namespace jau;

class TestLatch01 {
  private:
    jau::relaxed_atomic_int my_counter = 0;

    void something(jau::latch& l) {
        my_counter = my_counter + 1;
        l.count_down();
    }

  public:

    void test01() {
        INFO_STR("\n\ntest01\n");
        const size_t count = 8;
        std::thread tasks[count];
        jau::latch completion(count+1);

        REQUIRE_MSG("not-zero", count+1 == completion.value());

        for(size_t i=0; i<count; i++) {
            tasks[i] = std::thread(&TestLatch01::something, this, std::ref(completion));
        }
        completion.arrive_and_wait();

        REQUIRE_MSG("zero", 0 == completion.value());
        REQUIRE_MSG("8", count == my_counter);

        for(size_t i=0; i<count; i++) {
            if( tasks[i].joinable() ) {
                tasks[i].join();
            }
        }
    }
};

METHOD_AS_TEST_CASE( TestLatch01::test01,      "Test TestLatch01");

