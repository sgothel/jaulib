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

#include <jau/ringbuffer.hpp>

using namespace jau;

class Integer {
    public:
        jau::nsize_t value;

        Integer(jau::nsize_t v) : value(v) {}

        Integer(const Integer &o) noexcept = default;
        Integer(Integer &&o) noexcept = default;
        Integer& operator=(const Integer &o) noexcept = default;
        Integer& operator=(Integer &&o) noexcept = default;

        operator jau::nsize_t() const {
            return value;
        }
        jau::nsize_t intValue() const { return value; }
        static Integer valueOf(const jau::nsize_t i) { return Integer(i); }
};

std::shared_ptr<Integer> NullInteger = nullptr;

typedef std::shared_ptr<Integer> SharedType;
typedef ringbuffer<SharedType, nullptr, jau::nsize_t> SharedTypeRingbuffer;

// Test examples.
class TestRingbuffer11 {
  private:

    std::shared_ptr<SharedTypeRingbuffer> createEmpty(jau::nsize_t initialCapacity) {
        return std::shared_ptr<SharedTypeRingbuffer>(new SharedTypeRingbuffer(initialCapacity));
    }
    std::shared_ptr<SharedTypeRingbuffer> createFull(const std::vector<std::shared_ptr<Integer>> & source) {
        return std::shared_ptr<SharedTypeRingbuffer>(new SharedTypeRingbuffer(source));
    }

    std::vector<SharedType> createIntArray(const jau::nsize_t capacity, const jau::nsize_t startValue) {
        std::vector<SharedType> array(capacity);
        for(jau::nsize_t i=0; i<capacity; i++) {
            array[i] = SharedType(new Integer(startValue+i));
        }
        return array;
    }

    void getThreadType01(const std::string msg, std::shared_ptr<SharedTypeRingbuffer> rb, jau::nsize_t len) {
        // std::thread::id this_id = std::this_thread::get_id();
        // pthread_t this_id = pthread_self();

        // INFO_STR, INFO: Not thread safe yet
        // INFO_STR(msg+": Created / " + rb->toString());
        for(jau::nsize_t i=0; i<len; i++) {
            SharedType svI = rb->getBlocking();
            REQUIRE_MSG("not empty at read #"+std::to_string(i+1)+": "+rb->toString(), svI!=nullptr);
            // INFO_STR("Got "+std::to_string(svI->intValue())+" / " + rb->toString());
        }
        // INFO_STR(msg+": Dies / " + rb->toString());
        (void)msg;
    }

    void putThreadType01(const std::string msg, std::shared_ptr<SharedTypeRingbuffer> rb, jau::nsize_t len, jau::nsize_t startValue) {
        // std::thread::id this_id = std::this_thread::get_id();
        // pthread_t this_id = pthread_self();

        // INFO_STR(msg+": Created / " + rb->toString());
        jau::nsize_t preSize = rb->getSize();
        (void)preSize;

        for(jau::nsize_t i=0; i<len; i++) {
            Integer * vI = new Integer(startValue+i);
            // INFO_STR("Putting "+std::to_string(vI->intValue())+" ... / " + rb->toString());
            rb->putBlocking( SharedType( vI ) );
        }
        // INFO_STR(msg+": Dies / " + rb->toString());
        (void)msg;
    }

  public:

    void test01_Read1Write1() {
        INFO_STR("\n\ntest01_Read1Write1\n");
        jau::nsize_t capacity = 100;
        std::shared_ptr<SharedTypeRingbuffer> rb = createEmpty(capacity);
        REQUIRE_MSG("empty size "+rb->toString(), 0 == rb->getSize());
        REQUIRE_MSG("empty "+rb->toString(), rb->isEmpty());

        std::thread getThread01(&TestRingbuffer11::getThreadType01, this, "test01.get01", rb, capacity); // @suppress("Invalid arguments")
        std::thread putThread01(&TestRingbuffer11::putThreadType01, this, "test01.put01", rb, capacity, 0); // @suppress("Invalid arguments")
        putThread01.join();
        getThread01.join();

        REQUIRE_MSG("empty "+rb->toString(), rb->isEmpty());
        REQUIRE_MSG("empty size "+rb->toString(), 0 == rb->getSize());
    }

    void test02_Read4Write1() {
        INFO_STR("\n\ntest02_Read4Write1\n");
        jau::nsize_t capacity = 400;
        std::shared_ptr<SharedTypeRingbuffer> rb = createEmpty(capacity);
        REQUIRE_MSG("empty size "+rb->toString(), 0 == rb->getSize());
        REQUIRE_MSG("empty "+rb->toString(), rb->isEmpty());

        std::thread getThread01(&TestRingbuffer11::getThreadType01, this, "test02.get01", rb, capacity/4); // @suppress("Invalid arguments")
        std::thread getThread02(&TestRingbuffer11::getThreadType01, this, "test02.get02", rb, capacity/4); // @suppress("Invalid arguments")
        std::thread putThread01(&TestRingbuffer11::putThreadType01, this, "test02.put01", rb, capacity, 0); // @suppress("Invalid arguments")
        std::thread getThread03(&TestRingbuffer11::getThreadType01, this, "test02.get03", rb, capacity/4); // @suppress("Invalid arguments")
        std::thread getThread04(&TestRingbuffer11::getThreadType01, this, "test02.get04", rb, capacity/4); // @suppress("Invalid arguments")
        putThread01.join();
        getThread01.join();
        getThread02.join();
        getThread03.join();
        getThread04.join();

        REQUIRE_MSG("empty "+rb->toString(), rb->isEmpty());
        REQUIRE_MSG("empty size "+rb->toString(), 0 == rb->getSize());
    }

    void test03_Read8Write2() {
        INFO_STR("\n\ntest03_Read8Write2\n");
        jau::nsize_t capacity = 800;
        std::shared_ptr<SharedTypeRingbuffer> rb = createEmpty(capacity);
        REQUIRE_MSG("empty size "+rb->toString(), 0 == rb->getSize());
        REQUIRE_MSG("empty "+rb->toString(), rb->isEmpty());

        std::thread getThread01(&TestRingbuffer11::getThreadType01, this, "test03.get01", rb, capacity/8); // @suppress("Invalid arguments")
        std::thread getThread02(&TestRingbuffer11::getThreadType01, this, "test03.get02", rb, capacity/8); // @suppress("Invalid arguments")
        std::thread putThread01(&TestRingbuffer11::putThreadType01, this, "test03.put01", rb, capacity/2,  0); // @suppress("Invalid arguments")
        std::thread getThread03(&TestRingbuffer11::getThreadType01, this, "test03.get03", rb, capacity/8); // @suppress("Invalid arguments")
        std::thread getThread04(&TestRingbuffer11::getThreadType01, this, "test03.get04", rb, capacity/8); // @suppress("Invalid arguments")

        std::thread getThread05(&TestRingbuffer11::getThreadType01, this, "test03.get05", rb, capacity/8); // @suppress("Invalid arguments")
        std::thread getThread06(&TestRingbuffer11::getThreadType01, this, "test03.get06", rb, capacity/8); // @suppress("Invalid arguments")
        std::thread putThread02(&TestRingbuffer11::putThreadType01, this, "test03.put02", rb, capacity/2,  400); // @suppress("Invalid arguments")
        std::thread getThread07(&TestRingbuffer11::getThreadType01, this, "test03.get07", rb, capacity/8); // @suppress("Invalid arguments")
        std::thread getThread08(&TestRingbuffer11::getThreadType01, this, "test03.get08", rb, capacity/8); // @suppress("Invalid arguments")

        putThread01.join();
        putThread02.join();
        getThread01.join();
        getThread02.join();
        getThread03.join();
        getThread04.join();
        getThread05.join();
        getThread06.join();
        getThread07.join();
        getThread08.join();

        REQUIRE_MSG("empty "+rb->toString(), rb->isEmpty());
        REQUIRE_MSG("empty size "+rb->toString(), 0 == rb->getSize());
    }

    void test_list() {
        test01_Read1Write1();
        test02_Read4Write1();
        test03_Read8Write2();

        test01_Read1Write1();
        test02_Read4Write1();
        test03_Read8Write2();

        test03_Read8Write2();
        test03_Read8Write2();
        test03_Read8Write2();
    }
};

METHOD_AS_TEST_CASE( TestRingbuffer11::test_list, "Test TestRingbuffer 11- test_list");

