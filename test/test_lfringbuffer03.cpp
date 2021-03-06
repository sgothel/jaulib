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

std::shared_ptr<Integer> NullInteger = nullptr;

typedef std::shared_ptr<Integer> SharedType;
constexpr const std::nullptr_t SharedTypeNullElem = nullptr ;
typedef ringbuffer<SharedType, std::nullptr_t, jau::nsize_t> SharedTypeRingbuffer;

// Test examples.
class TestRingbuffer03 {
  private:

    std::shared_ptr<SharedTypeRingbuffer> createEmpty(jau::nsize_t initialCapacity) {
        std::shared_ptr<SharedTypeRingbuffer> rb = std::shared_ptr<SharedTypeRingbuffer>(new SharedTypeRingbuffer(nullptr, initialCapacity));
        REQUIRE_MSG("empty "+rb->toString(), rb->isEmpty());
        return rb;
    }
    std::shared_ptr<SharedTypeRingbuffer> createFull(const std::vector<std::shared_ptr<Integer>> & source) {
        std::shared_ptr<SharedTypeRingbuffer> rb = std::shared_ptr<SharedTypeRingbuffer>(new SharedTypeRingbuffer(nullptr, source));
        REQUIRE_MSG("full "+rb->toString(), rb->isFull());
        return rb;
    }

    std::vector<SharedType> createIntArray(const jau::nsize_t capacity, const IntegralType startValue) {
        std::vector<SharedType> array(capacity);
        for(jau::nsize_t i=0; i<capacity; i++) {
            array[i] = SharedType(new Integer(startValue+i));
        }
        return array;
    }

    void readTestImpl(SharedTypeRingbuffer &rb, bool clearRef, jau::nsize_t capacity, jau::nsize_t len, IntegralType startValue) {
        (void) clearRef;

        jau::nsize_t preSize = rb.getSize();
        REQUIRE_MSG("capacity "+rb.toString(), capacity == rb.capacity());
        REQUIRE_MSG("capacity at read "+std::to_string(len)+" elems: "+rb.toString(), capacity >= len);
        REQUIRE_MSG("size at read "+std::to_string(len)+" elems: "+rb.toString(), preSize >= len);
        REQUIRE_MSG("not empty "+rb.toString(), !rb.isEmpty());

        for(jau::nsize_t i=0; i<len; i++) {
            SharedType svI = rb.get();
            REQUIRE_MSG("not empty at read #"+std::to_string(i+1)+": "+rb.toString(), svI!=nullptr);
            REQUIRE_MSG("value at read #"+std::to_string(i+1)+": "+rb.toString(), IntegralType(startValue+i) == svI->intValue());
        }

        REQUIRE_MSG("size "+rb.toString(), preSize-len == rb.getSize());
        REQUIRE_MSG("free slots after reading "+std::to_string(len)+": "+rb.toString(), rb.getFreeSlots()>= len);
        REQUIRE_MSG("not full "+rb.toString(), !rb.isFull());
    }
    void readTestImpl2(SharedTypeRingbuffer &rb, bool clearRef, jau::nsize_t capacity, jau::nsize_t len, IntegralType startValue) {
        (void) clearRef;

        jau::nsize_t preSize = rb.getSize();
        REQUIRE_MSG("capacity "+rb.toString(), capacity == rb.capacity());
        REQUIRE_MSG("capacity at read "+std::to_string(len)+" elems: "+rb.toString(), capacity >= len);
        REQUIRE_MSG("size at read "+std::to_string(len)+" elems: "+rb.toString(), preSize >= len);
        REQUIRE_MSG("not empty "+rb.toString(), !rb.isEmpty());

        for(jau::nsize_t i=0; i<len; i++) {
            SharedType svI;
            REQUIRE_MSG("ringbuffer get", rb.get(svI));
            REQUIRE_MSG("not empty at read #"+std::to_string(i+1)+": "+rb.toString(), svI!=nullptr);
            REQUIRE_MSG("value at read #"+std::to_string(i+1)+": "+rb.toString(), IntegralType(startValue+i) == svI->intValue());
        }

        REQUIRE_MSG("size "+rb.toString(), preSize-len == rb.getSize());
        REQUIRE_MSG("free slots after reading "+std::to_string(len)+": "+rb.toString(), rb.getFreeSlots()>= len);
        REQUIRE_MSG("not full "+rb.toString(), !rb.isFull());
    }

    void readRangeTestImpl(SharedTypeRingbuffer &rb, bool clearRef, jau::nsize_t capacity, jau::nsize_t len, IntegralType startValue) {
        (void) clearRef;

        jau::nsize_t preSize = rb.getSize();
        REQUIRE_MSG("capacity "+rb.toString(), capacity == rb.capacity());
        REQUIRE_MSG("capacity at read "+std::to_string(len)+" elems: "+rb.toString(), capacity >= len);
        REQUIRE_MSG("size at read "+std::to_string(len)+" elems: "+rb.toString(), preSize >= len);
        REQUIRE_MSG("not empty "+rb.toString(), !rb.isEmpty());

        std::vector<SharedType> array(len);
        REQUIRE_MSG("get-range of "+std::to_string(array.size())+" elem in "+rb.toString(), len==rb.get( &(*array.begin()), len, len) );

        REQUIRE_MSG("size "+rb.toString(), preSize-len == rb.getSize());
        REQUIRE_MSG("free slots after reading "+std::to_string(len)+": "+rb.toString(), rb.getFreeSlots()>= len);
        REQUIRE_MSG("not full "+rb.toString(), !rb.isFull());

        for(jau::nsize_t i=0; i<len; i++) {
            SharedType svI = array[i];
            REQUIRE_MSG("not empty at read #"+std::to_string(i+1)+": "+rb.toString(), svI!=nullptr);
            REQUIRE_MSG("value at read #"+std::to_string(i+1)+": "+rb.toString(), IntegralType(startValue+i) == svI->intValue());
        }
    }

    void writeTestImpl(SharedTypeRingbuffer &rb, jau::nsize_t capacity, jau::nsize_t len, IntegralType startValue) {
        jau::nsize_t preSize = rb.getSize();

        REQUIRE_MSG("capacity "+rb.toString(), capacity == rb.capacity());
        REQUIRE_MSG("capacity at write "+std::to_string(len)+" elems: "+rb.toString(), capacity >= len);
        REQUIRE_MSG("size at write "+std::to_string(len)+" elems: "+rb.toString(), preSize+len <= capacity);
        REQUIRE_MSG("not full "+rb.toString(), !rb.isFull());

        for(jau::nsize_t i=0; i<len; i++) {
            std::string m = "buffer put #"+std::to_string(i)+": "+rb.toString();
            REQUIRE_MSG(m, rb.put( SharedType( new Integer(startValue+i) ) ) );
        }

        REQUIRE_MSG("size "+rb.toString(), preSize+len == rb.getSize());
        REQUIRE_MSG("not empty "+rb.toString(), !rb.isEmpty());
    }

    void writeRangeTestImpl(SharedTypeRingbuffer &rb, jau::nsize_t capacity, const std::vector<std::shared_ptr<Integer>> & data) {
        jau::nsize_t preSize = rb.getSize();
        jau::nsize_t postSize = preSize+data.size();

        REQUIRE_MSG("capacity "+rb.toString(), capacity == rb.capacity());
        REQUIRE_MSG("capacity at write "+std::to_string(data.size())+" elems: "+rb.toString(), capacity >= data.size());
        REQUIRE_MSG("size at write "+std::to_string(data.size())+" elems: "+rb.toString(), postSize<= capacity);
        REQUIRE_MSG("not full "+rb.toString(), !rb.isFull());
        REQUIRE_MSG("data fits in RB capacity "+rb.toString(), rb.capacity() >= data.size());
        REQUIRE_MSG("data fits in RB free-slots "+rb.toString(), rb.getFreeSlots() >= data.size());

        REQUIRE_MSG("put-range of "+std::to_string(data.size())+" elem in "+rb.toString(), rb.put( &(*data.begin()), &(*data.end()) ) );

        REQUIRE_MSG("size "+rb.toString(), postSize == rb.getSize());
        REQUIRE_MSG("not empty "+rb.toString(), !rb.isEmpty());
    }

    void moveGetPutImpl(SharedTypeRingbuffer &rb, jau::nsize_t pos) {
        REQUIRE_MSG("not empty "+rb.toString(), !rb.isEmpty());
        for(jau::nsize_t i=0; i<pos; i++) {
            REQUIRE_MSG("moveFull.get "+rb.toString(), IntegralType(i) == rb.get()->intValue());
            REQUIRE_MSG("moveFull.put "+rb.toString(), rb.put( SharedType( new Integer(i) ) ) );
        }
    }

    void movePutGetImpl(SharedTypeRingbuffer &rb, jau::nsize_t pos) {
        REQUIRE_MSG("RB is full "+rb.toString(), !rb.isFull());
        for(jau::nsize_t i=0; i<pos; i++) {
            REQUIRE_MSG("moveEmpty.put "+rb.toString(), rb.put( SharedType( new Integer(600+i) ) ) );
            REQUIRE_MSG("moveEmpty.get "+rb.toString(), IntegralType(600+i) == rb.get()->intValue());
        }
    }

  public:

    void test00_PrintInfo() {
        std::shared_ptr<SharedTypeRingbuffer> rb = createEmpty(11);

        std::string msg("Ringbuffer: uses_memcpy "+std::to_string(SharedTypeRingbuffer::uses_memcpy)+
                 ", uses_memset "+std::to_string(SharedTypeRingbuffer::uses_memset)+
                 ", trivially_copyable "+std::to_string(std::is_trivially_copyable<typename SharedTypeRingbuffer::value_type>::value)+
                 ", size "+std::to_string(sizeof(rb))+" bytes");
        fprintf(stderr, "%s", msg.c_str());
        REQUIRE_MSG("Ringbuffer<shared_ptr<T>> not using memcpy", !SharedTypeRingbuffer::uses_memcpy);
        REQUIRE_MSG("Ringbuffer<shared_ptr<T>> not using memset", !SharedTypeRingbuffer::uses_memset);
    }

    void test01_FullRead() {
        jau::nsize_t capacity = 11;
        std::vector<SharedType> source = createIntArray(capacity, 0);
        std::shared_ptr<SharedTypeRingbuffer> rb = createFull(source);
        INFO_STR("test01_FullRead: Created / "+ rb->toString());
        REQUIRE_MSG("full size "+rb->toString(), capacity == rb->getSize());
        REQUIRE_MSG("full "+rb->toString(), rb->isFull());

        readTestImpl(*rb, true, capacity, capacity, 0);
        INFO_STR("test01_FullRead: PostRead / " + rb->toString());
        REQUIRE_MSG("empty "+rb->toString(), rb->isEmpty());
    }

    void test02_EmptyWrite() {
        jau::nsize_t capacity = 11;
        std::shared_ptr<SharedTypeRingbuffer> rb = createEmpty(capacity);
        INFO( std::string("test02_EmptyWrite: Created / ") + rb->toString().c_str());
        REQUIRE_MSG("zero size "+rb->toString(), 0 == rb->getSize());
        REQUIRE_MSG("empty "+rb->toString(), rb->isEmpty());

        writeTestImpl(*rb, capacity, capacity, 0);
        INFO( std::string("test02_EmptyWrite: PostWrite / ") + rb->toString().c_str());
        REQUIRE_MSG("full size "+rb->toString(), capacity == rb->getSize());
        REQUIRE_MSG("full "+rb->toString(), rb->isFull());

        readTestImpl(*rb, true, capacity, capacity, 0);
        INFO( std::string("test02_EmptyWrite: PostRead / ") + rb->toString().c_str());
        REQUIRE_MSG("empty "+rb->toString(), rb->isEmpty());
    }

    void test03_EmptyWriteRange() {
        {
            jau::nsize_t capacity = 11;
            std::shared_ptr<SharedTypeRingbuffer> rb = createEmpty(capacity);
            INFO( std::string("test03_EmptyWriteRange: Created / ") + rb->toString().c_str());
            REQUIRE_MSG("zero size "+rb->toString(), 0 == rb->getSize());
            REQUIRE_MSG("empty "+rb->toString(), rb->isEmpty());

            /**
             * Move R == W == 0
             * Empty [RW][][ ][  ][ ][ ][ ][ ][ ][ ][ ] ; start
             */
            std::vector<SharedType> new_data = createIntArray(capacity, 0);
            writeRangeTestImpl(*rb, capacity, new_data);

            INFO( std::string("test03_EmptyWriteRange: PostWrite / ") + rb->toString().c_str());
            REQUIRE_MSG("full size "+rb->toString(), capacity == rb->getSize());
            REQUIRE_MSG("full "+rb->toString(), rb->isFull());

            readRangeTestImpl(*rb, true, capacity, capacity, 0);
            INFO( std::string("test03_EmptyWriteRange: PostRead / ") + rb->toString().c_str());
            REQUIRE_MSG("empty "+rb->toString(), rb->isEmpty());
        }
        {
            jau::nsize_t capacity = 11;
            std::shared_ptr<SharedTypeRingbuffer> rb = createEmpty(capacity);
            INFO( std::string("test03_EmptyWriteRange: Created / ") + rb->toString().c_str());
            REQUIRE_MSG("zero size "+rb->toString(), 0 == rb->getSize());
            REQUIRE_MSG("empty "+rb->toString(), rb->isEmpty());

            /**
             * Move R == W == 3
             * Empty [RW][][ ][  ][ ][ ][ ][ ][ ][ ][ ] ; start
             * Empty [ ][ ][ ][RW][ ][ ][ ][ ][ ][ ][ ]
             */
            SharedType dummy(new Integer(-1));
            rb->put(dummy);
            rb->put(dummy);
            rb->put(dummy);
            rb->drop(3);

            std::vector<SharedType> new_data = createIntArray(capacity, 0);
            writeRangeTestImpl(*rb, capacity, new_data);

            INFO( std::string("test03_EmptyWriteRange: PostWrite / ") + rb->toString().c_str());
            REQUIRE_MSG("full size "+rb->toString(), capacity == rb->getSize());
            REQUIRE_MSG("full "+rb->toString(), rb->isFull());

            readRangeTestImpl(*rb, true, capacity, capacity, 0);
            INFO( std::string("test03_EmptyWriteRange: PostRead / ") + rb->toString().c_str());
            REQUIRE_MSG("empty "+rb->toString(), rb->isEmpty());
        }
        {
            jau::nsize_t capacity = 11;
            std::shared_ptr<SharedTypeRingbuffer> rb = createEmpty(capacity);
            INFO( std::string("test03_EmptyWriteRange: Created / ") + rb->toString().c_str());
            REQUIRE_MSG("zero size "+rb->toString(), 0 == rb->getSize());
            REQUIRE_MSG("empty "+rb->toString(), rb->isEmpty());

            /**
             * Move R == 2, W == 4, size 2
             * Empty [RW][][ ][ ][ ][ ][ ][ ][ ][ ][ ] ; start
             * Avail [ ][ ][R][.][W][ ][ ][ ][ ][ ][ ] ; W > R
             */
            SharedType dummy(new Integer(-1));
            rb->put(dummy); // w idx 0 -> 1
            rb->put(dummy);
            rb->put(dummy);
            rb->put(dummy); // w idx 3 -> 4
            rb->drop(2);     // r idx 0 -> 2

            // left = 11 - 2
            REQUIRE_MSG("size 2 "+rb->toString(), 2 == rb->getSize());
            REQUIRE_MSG("available 11-2 "+rb->toString(), capacity-2 == rb->getFreeSlots());

            std::vector<SharedType> new_data = createIntArray(capacity-2, 0);
            writeRangeTestImpl(*rb, capacity, new_data);
            // writeTestImpl(*rb, capacity, capacity-2, 0);

            INFO( std::string("test03_EmptyWriteRange: PostWrite / ") + rb->toString().c_str());
            REQUIRE_MSG("full size "+rb->toString(), capacity == rb->getSize());
            REQUIRE_MSG("full "+rb->toString(), rb->isFull());

            // take off 2 remaining dummies
            rb->drop(2);
            REQUIRE_MSG("size capacity-2 "+rb->toString(), capacity-2 == rb->getSize());

            readRangeTestImpl(*rb, true, capacity, capacity-2, 0);
            // readTestImpl(*rb, true, capacity, capacity-2, 0);
            INFO( std::string("test03_EmptyWriteRange: PostRead / ") + rb->toString().c_str());
            REQUIRE_MSG("size 0 "+rb->toString(), 0 == rb->getSize());
            REQUIRE_MSG("empty "+rb->toString(), rb->isEmpty());
        }
        {
            jau::nsize_t capacity = 11;
            std::shared_ptr<SharedTypeRingbuffer> rb = createEmpty(capacity);
            INFO( std::string("test03_EmptyWriteRange: Created / ") + rb->toString().c_str());
            REQUIRE_MSG("zero size "+rb->toString(), 0 == rb->getSize());
            REQUIRE_MSG("empty "+rb->toString(), rb->isEmpty());

            /**
             * Move R == 9, W == 1, size 3
             * Empty [RW][][ ][ ][ ][ ][ ][ ][ ][ ][ ] ; start
             * Avail [.][W][ ][ ][ ][ ][ ][ ][ ][R][.] ; W < R - 1
             */
            SharedType dummy(new Integer(-1));
            for(int i=0; i<11; i++) { rb->put(dummy); } // fill all
            REQUIRE_MSG("full "+rb->toString(), rb->isFull());

            rb->drop(10); // pull
            REQUIRE_MSG("size 1"+rb->toString(), 1 == rb->getSize());

            for(int i=0; i<2; i++) { rb->put(dummy); } // fill 2 more
            REQUIRE_MSG("size 3"+rb->toString(), 3 == rb->getSize());

            // left = 11 - 3
            REQUIRE_MSG("available 11-3 "+rb->toString(), capacity-3 == rb->getFreeSlots());

            std::vector<SharedType> new_data = createIntArray(capacity-3, 0);
            writeRangeTestImpl(*rb, capacity, new_data);
            // writeTestImpl(*rb, capacity, capacity-3, 0);

            INFO( std::string("test03_EmptyWriteRange: PostWrite / ") + rb->toString().c_str());
            REQUIRE_MSG("full size "+rb->toString(), capacity == rb->getSize());
            REQUIRE_MSG("full "+rb->toString(), rb->isFull());

            // take off 3 remaining dummies
            rb->drop(3); // pull
            REQUIRE_MSG("size capacity-3 "+rb->toString(), capacity-3 == rb->getSize());

            readRangeTestImpl(*rb, true, capacity, capacity-3, 0);
            // readTestImpl(*rb, true, capacity, capacity-3, 0);
            INFO( std::string("test03_EmptyWriteRange: PostRead / ") + rb->toString().c_str());
            REQUIRE_MSG("size 0 "+rb->toString(), 0 == rb->getSize());
            REQUIRE_MSG("empty "+rb->toString(), rb->isEmpty());
        }
    }

    void test04_FullReadReset() {
        jau::nsize_t capacity = 11;
        std::vector<SharedType> source = createIntArray(capacity, 0);
        std::shared_ptr<SharedTypeRingbuffer> rb = createFull(source);
        INFO_STR("test04_FullReadReset: Created / " + rb->toString());
        REQUIRE_MSG("full "+rb->toString(), rb->isFull());

        rb->reset(source);
        INFO_STR("test04_FullReadReset: Post Reset w/ source / " + rb->toString());
        REQUIRE_MSG("full "+rb->toString(), rb->isFull());

        readTestImpl(*rb, false, capacity, capacity, 0);
        INFO_STR("test04_FullReadReset: Post Read / " + rb->toString());
        REQUIRE_MSG("empty "+rb->toString(), rb->isEmpty());

        rb->reset(source);
        INFO_STR("test04_FullReadReset: Post Reset w/ source / " + rb->toString());
        REQUIRE_MSG("full "+rb->toString(), rb->isFull());

        readTestImpl2(*rb, false, capacity, capacity, 0);
        INFO_STR("test04_FullReadReset: Post Read / " + rb->toString());
        REQUIRE_MSG("empty "+rb->toString(), rb->isEmpty());
    }

    void test05_EmptyWriteClear() {
        jau::nsize_t capacity = 11;
        std::shared_ptr<SharedTypeRingbuffer> rb = createEmpty(capacity);
        REQUIRE_MSG("empty "+rb->toString(), rb->isEmpty());

        rb->clear();
        REQUIRE_MSG("empty "+rb->toString(), rb->isEmpty());

        writeTestImpl(*rb, capacity, capacity, 0);
        REQUIRE_MSG("full "+rb->toString(), rb->isFull());

        readTestImpl(*rb, false, capacity, capacity, 0);
        REQUIRE_MSG("empty "+rb->toString(), rb->isEmpty());

        rb->clear();
        REQUIRE_MSG("empty "+rb->toString(), rb->isEmpty());

        writeTestImpl(*rb, capacity, capacity, 0);
        REQUIRE_MSG("full "+rb->toString(), rb->isFull());

        readTestImpl2(*rb, false, capacity, capacity, 0);
        REQUIRE_MSG("empty "+rb->toString(), rb->isEmpty());
    }

    void test06_ReadResetMid01() {
        jau::nsize_t capacity = 11;
        std::vector<SharedType> source = createIntArray(capacity, 0);
        std::shared_ptr<SharedTypeRingbuffer> rb = createFull(source);
        REQUIRE_MSG("full "+rb->toString(), rb->isFull());

        rb->reset(source);
        REQUIRE_MSG("full "+rb->toString(), rb->isFull());

        readTestImpl(*rb, false, capacity, 5, 0);
        REQUIRE_MSG("not empty "+rb->toString(), !rb->isEmpty());
        REQUIRE_MSG("not Full "+rb->toString(), !rb->isFull());

        rb->reset(source);
        REQUIRE_MSG("full "+rb->toString(), rb->isFull());

        readTestImpl(*rb, false, capacity, capacity, 0);
        REQUIRE_MSG("empty "+rb->toString(), rb->isEmpty());
    }

    void test07_ReadResetMid02() {
        jau::nsize_t capacity = 11;
        std::vector<SharedType> source = createIntArray(capacity, 0);
        std::shared_ptr<SharedTypeRingbuffer> rb = createFull(source);
        REQUIRE_MSG("full "+rb->toString(), rb->isFull());

        rb->reset(source);
        REQUIRE_MSG("full "+rb->toString(), rb->isFull());

        moveGetPutImpl(*rb, 5);
        readTestImpl(*rb, false, capacity, 5, 5);
        REQUIRE_MSG("not empty "+rb->toString(), !rb->isEmpty());
        REQUIRE_MSG("not Full "+rb->toString(), !rb->isFull());

        rb->reset(source);
        REQUIRE_MSG("full "+rb->toString(), rb->isFull());

        readTestImpl(*rb, false, capacity, capacity, 0);
        REQUIRE_MSG("empty "+rb->toString(), rb->isEmpty());
    }

  private:

    void test_GrowFullImpl(jau::nsize_t initialCapacity, jau::nsize_t pos) {
        jau::nsize_t growAmount = 5;
        jau::nsize_t grownCapacity = initialCapacity+growAmount;
        std::vector<SharedType> source = createIntArray(initialCapacity, 0);
        std::shared_ptr<SharedTypeRingbuffer> rb = createFull(source);

        for(jau::nsize_t i=0; i<initialCapacity; i++) {
            SharedType svI = rb->get();
            REQUIRE_MSG("not empty at read #"+std::to_string(i+1)+": "+rb->toString(), svI!=nullptr);
            REQUIRE_MSG("value at read #"+std::to_string(i+1)+": "+rb->toString(), IntegralType((0+i)%initialCapacity) == svI->intValue());
        }
        REQUIRE_MSG("zero size "+rb->toString(), 0 == rb->getSize());

        rb->reset(source);
        REQUIRE_MSG("orig size "+rb->toString(), initialCapacity == rb->getSize());

        moveGetPutImpl(*rb, pos);
        // PRINTM("X02 "+rb->toString());
        // rb->dump(stderr, "X02");

        rb->recapacity(grownCapacity);
        REQUIRE_MSG("capacity "+rb->toString(), grownCapacity == rb->capacity());
        REQUIRE_MSG("orig size "+rb->toString(), initialCapacity == rb->getSize());
        REQUIRE_MSG("not full "+rb->toString(), !rb->isFull());
        REQUIRE_MSG("not empty "+rb->toString(), !rb->isEmpty());
        // PRINTM("X03 "+rb->toString());
        // rb->dump(stderr, "X03");

        for(jau::nsize_t i=0; i<growAmount; i++) {
            REQUIRE_MSG("buffer not full at put #"+std::to_string(i)+": "+rb->toString(), rb->put( SharedType( new Integer(100+i) ) ) );
        }
        REQUIRE_MSG("new size "+rb->toString(), grownCapacity == rb->getSize());
        REQUIRE_MSG("full "+rb->toString(), rb->isFull());

        for(jau::nsize_t i=0; i<initialCapacity; i++) {
            SharedType svI = rb->get();
            // PRINTM("X05["+std::to_string(i)+"]: "+rb->toString()+", svI-null: "+std::to_string(svI==nullptr));
            REQUIRE_MSG("not empty at read #"+std::to_string(i+1)+": "+rb->toString(), svI!=nullptr);
            REQUIRE_MSG("value at read #"+std::to_string(i+1)+": "+rb->toString(), IntegralType((pos+i)%initialCapacity) == svI->intValue());
        }

        for(jau::nsize_t i=0; i<growAmount; i++) {
            SharedType svI = rb->get();
            // PRINTM("X07["+std::to_string(i)+"]: "+rb->toString()+", svI-null: "+std::to_string(svI==nullptr));
            REQUIRE_MSG("not empty at read #"+std::to_string(i+1)+": "+rb->toString(), svI!=nullptr);
            REQUIRE_MSG("value at read #"+std::to_string(i+1)+": "+rb->toString(), IntegralType(100+i) == svI->intValue());
        }

        REQUIRE_MSG("zero size "+rb->toString(), 0 == rb->getSize());
        REQUIRE_MSG("empty "+rb->toString(), rb->isEmpty());

        REQUIRE_MSG("not full "+rb->toString(), !rb->isFull());
    }

  public:

    void test20_GrowFull01_Begin() {
        test_GrowFullImpl(11, 0);
    }
    void test21_GrowFull02_Begin1() {
        test_GrowFullImpl(11, 0+1);
    }
    void test22_GrowFull03_Begin2() {
        test_GrowFullImpl(11, 0+2);
    }
    void test23_GrowFull04_Begin3() {
        test_GrowFullImpl(11, 0+3);
    }
    void test24_GrowFull05_End() {
        test_GrowFullImpl(11, 11-1);
    }
    void test25_GrowFull11_End1() {
        test_GrowFullImpl(11, 11-1-1);
    }
    void test26_GrowFull12_End2() {
        test_GrowFullImpl(11, 11-1-2);
    }
    void test27_GrowFull13_End3() {
        test_GrowFullImpl(11, 11-1-3);
    }

};

METHOD_AS_TEST_CASE( TestRingbuffer03::test00_PrintInfo,         "Test TestRingbuffer 03- 00");
METHOD_AS_TEST_CASE( TestRingbuffer03::test01_FullRead,          "Test TestRingbuffer 03- 01");
METHOD_AS_TEST_CASE( TestRingbuffer03::test02_EmptyWrite,        "Test TestRingbuffer 03- 02");
METHOD_AS_TEST_CASE( TestRingbuffer03::test03_EmptyWriteRange,   "Test TestRingbuffer 03- 03");
METHOD_AS_TEST_CASE( TestRingbuffer03::test04_FullReadReset,     "Test TestRingbuffer 03- 04");
METHOD_AS_TEST_CASE( TestRingbuffer03::test05_EmptyWriteClear,   "Test TestRingbuffer 03- 05");
METHOD_AS_TEST_CASE( TestRingbuffer03::test06_ReadResetMid01,    "Test TestRingbuffer 03- 06");
METHOD_AS_TEST_CASE( TestRingbuffer03::test07_ReadResetMid02,    "Test TestRingbuffer 03- 07");
METHOD_AS_TEST_CASE( TestRingbuffer03::test20_GrowFull01_Begin,  "Test TestRingbuffer 03- 20");
METHOD_AS_TEST_CASE( TestRingbuffer03::test21_GrowFull02_Begin1, "Test TestRingbuffer 03- 21");
METHOD_AS_TEST_CASE( TestRingbuffer03::test22_GrowFull03_Begin2, "Test TestRingbuffer 03- 22");
METHOD_AS_TEST_CASE( TestRingbuffer03::test23_GrowFull04_Begin3, "Test TestRingbuffer 03- 23");
METHOD_AS_TEST_CASE( TestRingbuffer03::test24_GrowFull05_End,    "Test TestRingbuffer 03- 24");
METHOD_AS_TEST_CASE( TestRingbuffer03::test25_GrowFull11_End1,   "Test TestRingbuffer 03- 25");
METHOD_AS_TEST_CASE( TestRingbuffer03::test26_GrowFull12_End2,   "Test TestRingbuffer 03- 26");
METHOD_AS_TEST_CASE( TestRingbuffer03::test27_GrowFull13_End3,   "Test TestRingbuffer 03- 27");
