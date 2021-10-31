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

typedef jau::snsize_t IntegralType;

class Integer {
    public:
        IntegralType value;

        Integer(IntegralType v) : value(v) {}

        Integer() noexcept : value(0) { }

        Integer(const Integer &o) noexcept = default;
        Integer(Integer &&o) noexcept = default;
        Integer& operator=(const Integer &o) noexcept = default;
        Integer& operator=(Integer &&o) noexcept = default;

        operator IntegralType() const {
            return value;
        }
        IntegralType intValue() const { return value; }
        static Integer valueOf(const IntegralType i) { return Integer(i); }
};

typedef Integer TrivialType;
static const TrivialType TrivialTypeNullElem(-1);
typedef ringbuffer<TrivialType, jau::nsize_t> TrivialTypeRingbuffer;

// Test examples.
class TestRingbuffer12 {
  private:

    TrivialTypeRingbuffer createEmpty(jau::nsize_t initialCapacity) {
        TrivialTypeRingbuffer rb(initialCapacity);
        REQUIRE_MSG("empty "+rb.toString(), rb.isEmpty());
        return rb;
    }
    TrivialTypeRingbuffer createFull(const std::vector<TrivialType> & source) {
        TrivialTypeRingbuffer rb(source);
        REQUIRE_MSG("full "+rb.toString(), rb.isFull());
        return rb;
    }

    std::vector<TrivialType> createIntArray(const jau::nsize_t capacity, const IntegralType startValue) {
        std::vector<TrivialType> array(capacity);
        for(jau::nsize_t i=0; i<capacity; i++) {
            array[i] = TrivialType(startValue+i);
        }
        return array;
    }

    void getThreadType01(const std::string msg, TrivialTypeRingbuffer* rb, jau::nsize_t len) {
        // std::thread::id this_id = std::this_thread::get_id();
        // pthread_t this_id = pthread_self();

        // INFO_STR, INFO: Not thread safe yet
        // INFO_STR(msg+": Created / " + rb->toString());
        for(jau::nsize_t i=0; i<len; i++) {
            TrivialType svI;
            REQUIRE_MSG("not empty at read #"+std::to_string(i+1)+": "+rb->toString(), rb->getBlocking(svI));
            // INFO_STR("Got "+std::to_string(svI->intValue())+" / " + rb->toString());
        }
        // INFO_STR(msg+": Dies / " + rb->toString());
        (void)msg;
    }

    void getRangeThreadType02(const std::string msg, TrivialTypeRingbuffer* rb, jau::nsize_t len) {
        // std::thread::id this_id = std::this_thread::get_id();
        // pthread_t this_id = pthread_self();

        // INFO_STR, INFO: Not thread safe yet
        // INFO_STR(msg+": Created / " + rb->toString());
        std::vector<TrivialType> array(len);
        REQUIRE_MSG("get-range of "+std::to_string(array.size())+" elem in "+rb->toString(), len==rb->getBlocking( &(*array.begin()), len, len) );

        for(jau::nsize_t i=0; i<len; i++) {
            TrivialType svI = array[i];
            REQUIRE_MSG("not empty at read #"+std::to_string(i+1)+": "+rb->toString(), svI!=TrivialTypeNullElem);
            // INFO_STR("Got "+std::to_string(svI.intValue())+" / " + rb->toString());
        }
        // INFO_STR(msg+": Dies / " + rb->toString());
        (void)msg;
    }

    void putThreadType01(const std::string msg, TrivialTypeRingbuffer* rb, jau::nsize_t len, IntegralType startValue) {
        // std::thread::id this_id = std::this_thread::get_id();
        // pthread_t this_id = pthread_self();

        // INFO_STR(msg+": Created / " + rb->toString());
        for(jau::nsize_t i=0; i<len; i++) {
            Integer vI(startValue+i);
            // INFO_STR("Putting "+std::to_string(vI->intValue())+" ... / " + rb->toString());
            rb->putBlocking( vI );
        }
        // INFO_STR(msg+": Dies / " + rb->toString());
        (void)msg;
    }

    void putRangeThreadType02(const std::string msg, TrivialTypeRingbuffer* rb, jau::nsize_t len, IntegralType startValue) {
        // std::thread::id this_id = std::this_thread::get_id();
        // pthread_t this_id = pthread_self();

        // INFO_STR(msg+": Created / " + rb->toString());
        std::vector<TrivialType> data = createIntArray(len, startValue);
        REQUIRE_MSG("put-range of "+std::to_string(data.size())+" elem in "+rb->toString(), rb->put( &(*data.begin()), &(*data.end()) ) );

        // INFO_STR(msg+": Dies / " + rb->toString());
        (void)msg;
    }

  public:

    void test01a_Read1Write1() {
        INFO_STR("\n\ntest01a_Read1Write1\n");
        jau::nsize_t capacity = 100;
        TrivialTypeRingbuffer rb = createEmpty(capacity);
        REQUIRE_MSG("empty size "+rb.toString(), 0 == rb.size());
        REQUIRE_MSG("empty "+rb.toString(), rb.isEmpty());

        std::thread getThread01(&TestRingbuffer12::getThreadType01, this, "test01a.get01", &rb, capacity); // @suppress("Invalid arguments")
        std::thread putThread01(&TestRingbuffer12::putThreadType01, this, "test01a.put01", &rb, capacity, 0); // @suppress("Invalid arguments")
        putThread01.join();
        getThread01.join();

        REQUIRE_MSG("empty "+rb.toString(), rb.isEmpty());
        REQUIRE_MSG("empty size "+rb.toString(), 0 == rb.size());
    }

    void test01b_Read1Write1_Range() {
        INFO_STR("\n\ntest01b_Read1Write1_Range\n");
        jau::nsize_t capacity = 100;
        TrivialTypeRingbuffer rb = createEmpty(capacity);
        REQUIRE_MSG("empty size "+rb.toString(), 0 == rb.size());
        REQUIRE_MSG("empty "+rb.toString(), rb.isEmpty());

        std::thread getThread01(&TestRingbuffer12::getRangeThreadType02, this, "test01b.getR01", &rb, capacity); // @suppress("Invalid arguments")
        std::thread putThread01(&TestRingbuffer12::putRangeThreadType02, this, "test01b.putR01", &rb, capacity, 0); // @suppress("Invalid arguments")
        putThread01.join();
        getThread01.join();

        REQUIRE_MSG("empty "+rb.toString(), rb.isEmpty());
        REQUIRE_MSG("empty size "+rb.toString(), 0 == rb.size());
    }

    void test02a_Read4Write1() {
        INFO_STR("\n\ntest02a_Read4Write1\n");
        jau::nsize_t capacity = 400;
        TrivialTypeRingbuffer rb = createEmpty(capacity);
        REQUIRE_MSG("empty size "+rb.toString(), 0 == rb.size());
        REQUIRE_MSG("empty "+rb.toString(), rb.isEmpty());

        std::thread getThread01(&TestRingbuffer12::getThreadType01, this, "test02a.get01", &rb, capacity/4); // @suppress("Invalid arguments")
        std::thread getThread02(&TestRingbuffer12::getThreadType01, this, "test02a.get02", &rb, capacity/4); // @suppress("Invalid arguments")
        std::thread putThread01(&TestRingbuffer12::putThreadType01, this, "test02a.put01", &rb, capacity, 0); // @suppress("Invalid arguments")
        std::thread getThread03(&TestRingbuffer12::getThreadType01, this, "test02a.get03", &rb, capacity/4); // @suppress("Invalid arguments")
        std::thread getThread04(&TestRingbuffer12::getThreadType01, this, "test02a.get04", &rb, capacity/4); // @suppress("Invalid arguments")
        putThread01.join();
        getThread01.join();
        getThread02.join();
        getThread03.join();
        getThread04.join();

        REQUIRE_MSG("empty "+rb.toString(), rb.isEmpty());
        REQUIRE_MSG("empty size "+rb.toString(), 0 == rb.size());
    }

    void test02b_Read4Write1_Range() {
        INFO_STR("\n\ntest02b_Read4Write1_Range\n");
        jau::nsize_t capacity = 400;
        TrivialTypeRingbuffer rb = createEmpty(capacity);
        REQUIRE_MSG("empty size "+rb.toString(), 0 == rb.size());
        REQUIRE_MSG("empty "+rb.toString(), rb.isEmpty());

        std::thread getThread01(&TestRingbuffer12::getRangeThreadType02, this, "test02b.getR01", &rb, capacity/4); // @suppress("Invalid arguments")
        std::thread getThread02(&TestRingbuffer12::getRangeThreadType02, this, "test02b.getR02", &rb, capacity/4); // @suppress("Invalid arguments")
        std::thread putThread01(&TestRingbuffer12::putRangeThreadType02, this, "test02b.putR01", &rb, capacity, 0); // @suppress("Invalid arguments")
        std::thread getThread03(&TestRingbuffer12::getRangeThreadType02, this, "test02b.getR03", &rb, capacity/4); // @suppress("Invalid arguments")
        std::thread getThread04(&TestRingbuffer12::getRangeThreadType02, this, "test02b.getR04", &rb, capacity/4); // @suppress("Invalid arguments")
        putThread01.join();
        getThread01.join();
        getThread02.join();
        getThread03.join();
        getThread04.join();

        REQUIRE_MSG("empty "+rb.toString(), rb.isEmpty());
        REQUIRE_MSG("empty size "+rb.toString(), 0 == rb.size());
    }

    void test03a_Read8Write2() {
        INFO_STR("\n\ntest03a_Read8Write2\n");
        jau::nsize_t capacity = 800;
        TrivialTypeRingbuffer rb = createEmpty(capacity);
        REQUIRE_MSG("empty size "+rb.toString(), 0 == rb.size());
        REQUIRE_MSG("empty "+rb.toString(), rb.isEmpty());

        std::thread getThread01(&TestRingbuffer12::getThreadType01, this, "test03a.get01", &rb, capacity/8); // @suppress("Invalid arguments")
        std::thread getThread02(&TestRingbuffer12::getThreadType01, this, "test03a.get02", &rb, capacity/8); // @suppress("Invalid arguments")
        std::thread putThread01(&TestRingbuffer12::putThreadType01, this, "test03a.put01", &rb, capacity/2,  0); // @suppress("Invalid arguments")
        std::thread getThread03(&TestRingbuffer12::getThreadType01, this, "test03a.get03", &rb, capacity/8); // @suppress("Invalid arguments")
        std::thread getThread04(&TestRingbuffer12::getThreadType01, this, "test03a.get04", &rb, capacity/8); // @suppress("Invalid arguments")

        std::thread getThread05(&TestRingbuffer12::getThreadType01, this, "test03a.get05", &rb, capacity/8); // @suppress("Invalid arguments")
        std::thread getThread06(&TestRingbuffer12::getThreadType01, this, "test03a.get06", &rb, capacity/8); // @suppress("Invalid arguments")
        std::thread putThread02(&TestRingbuffer12::putThreadType01, this, "test03a.put02", &rb, capacity/2,  400); // @suppress("Invalid arguments")
        std::thread getThread07(&TestRingbuffer12::getThreadType01, this, "test03a.get07", &rb, capacity/8); // @suppress("Invalid arguments")
        std::thread getThread08(&TestRingbuffer12::getThreadType01, this, "test03a.get08", &rb, capacity/8); // @suppress("Invalid arguments")

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

        REQUIRE_MSG("empty "+rb.toString(), rb.isEmpty());
        REQUIRE_MSG("empty size "+rb.toString(), 0 == rb.size());
    }

    void test03b_Read8Write2_Range() {
        INFO_STR("\n\ntest03b_Read8Write2_Range\n");
        jau::nsize_t capacity = 800;
        TrivialTypeRingbuffer rb = createEmpty(capacity);
        REQUIRE_MSG("empty size "+rb.toString(), 0 == rb.size());
        REQUIRE_MSG("empty "+rb.toString(), rb.isEmpty());

        std::thread getThread01(&TestRingbuffer12::getRangeThreadType02, this, "test03b.getR01", &rb, capacity/8); // @suppress("Invalid arguments")
        std::thread getThread02(&TestRingbuffer12::getRangeThreadType02, this, "test03b.getR02", &rb, capacity/8); // @suppress("Invalid arguments")
        std::thread putThread01(&TestRingbuffer12::putRangeThreadType02, this, "test03b.putR01", &rb, capacity/2,  0); // @suppress("Invalid arguments")
        std::thread getThread03(&TestRingbuffer12::getRangeThreadType02, this, "test03b.getR03", &rb, capacity/8); // @suppress("Invalid arguments")
        std::thread getThread04(&TestRingbuffer12::getRangeThreadType02, this, "test03b.getR04", &rb, capacity/8); // @suppress("Invalid arguments")

        std::thread getThread05(&TestRingbuffer12::getRangeThreadType02, this, "test03b.getR05", &rb, capacity/8); // @suppress("Invalid arguments")
        std::thread getThread06(&TestRingbuffer12::getRangeThreadType02, this, "test03b.getR06", &rb, capacity/8); // @suppress("Invalid arguments")
        std::thread putThread02(&TestRingbuffer12::putRangeThreadType02, this, "test03b.putR02", &rb, capacity/2,  400); // @suppress("Invalid arguments")
        std::thread getThread07(&TestRingbuffer12::getRangeThreadType02, this, "test03b.getR07", &rb, capacity/8); // @suppress("Invalid arguments")
        std::thread getThread08(&TestRingbuffer12::getRangeThreadType02, this, "test03b.getR08", &rb, capacity/8); // @suppress("Invalid arguments")

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

        REQUIRE_MSG("empty "+rb.toString(), rb.isEmpty());
        REQUIRE_MSG("empty size "+rb.toString(), 0 == rb.size());
    }

    void test_sequential() {
        test01a_Read1Write1();
        test02a_Read4Write1();
        test03a_Read8Write2();

        test01a_Read1Write1();
        test02a_Read4Write1();
        test03a_Read8Write2();

        test03a_Read8Write2();
        test03a_Read8Write2();
        test03a_Read8Write2();
    }

    void test_range() {
        test01b_Read1Write1_Range();
        test02b_Read4Write1_Range();
        test03b_Read8Write2_Range();

        test01b_Read1Write1_Range();
        test02b_Read4Write1_Range();
        test03b_Read8Write2_Range();

        test01b_Read1Write1_Range();
        test02b_Read4Write1_Range();
        test03b_Read8Write2_Range();
    }
};

METHOD_AS_TEST_CASE( TestRingbuffer12::test_sequential, "Test TestRingbuffer 12- test_sequential");
METHOD_AS_TEST_CASE( TestRingbuffer12::test_range,      "Test TestRingbuffer 12- test_range");

