/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020 Gothel Software e.K.
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
#include <iostream>
#include <cassert>
#include <cinttypes>
#include <cstring>
#include <memory>
#include <thread>

#define CATCH_CONFIG_RUNNER
// #define CATCH_CONFIG_MAIN
#include <catch2/catch_amalgamated.hpp>
#include <jau/test/catch2_ext.hpp>

#include <jau/ringbuffer.hpp>

using namespace jau;

template<typename Value_type>
Value_type getDefault();

template<typename Integral_type, typename Value_type>
Value_type createValue(const Integral_type& v);

template<typename Integral_type, typename Value_type>
Integral_type getValue(const Value_type& e);

template <typename Integral_type, typename Value_type, typename Size_type,
        bool exp_memmove, bool exp_memcpy, bool exp_secmem,

        bool use_memmove = std::is_trivially_copyable_v<Value_type> || is_container_memmove_compliant_v<Value_type>,
        bool use_memcpy  = std::is_trivially_copyable_v<Value_type>,
        bool use_secmem  = is_enforcing_secmem_v<Value_type>
    >
class TestRingbuffer_A {
  public:
    typedef ringbuffer<Value_type, Size_type, use_memmove, use_memcpy, use_secmem> ringbuffer_t;

    typedef TestRingbuffer_A<Integral_type, Value_type, Size_type,
                             exp_memmove, exp_memcpy, exp_secmem,
                             use_memmove, use_memcpy, use_secmem> test_ringbuffer_t;

  private:

    ringbuffer_t createEmpty(jau::nsize_t initialCapacity) {
        ringbuffer_t rb(initialCapacity);
        REQUIRE_MSG("empty "+rb.toString(), rb.isEmpty());
        return rb;
    }
    ringbuffer_t createFull(const std::vector<Value_type> & source) {
        ringbuffer_t rb(source);
        REQUIRE_MSG("full "+rb.toString(), rb.isFull());
        return rb;
    }

    std::vector<Value_type> createIntArray(const jau::nsize_t capacity, const Integral_type startValue) {
        std::vector<Value_type> array(capacity);
        for(jau::nsize_t i=0; i<capacity; i++) {
            array[i] = createValue<Integral_type, Value_type>( (Integral_type)( startValue+i ));
        }
        return array;
    }

    void readTestImpl(ringbuffer_t &rb, jau::nsize_t capacity, jau::nsize_t dest_len, Integral_type startValue) {
        jau::nsize_t preSize = rb.size();
        REQUIRE_MSG("capacity "+rb.toString(), capacity == rb.capacity());
        REQUIRE_MSG("capacity at read "+std::to_string(dest_len)+" elems: "+rb.toString(), capacity >= dest_len);
        REQUIRE_MSG("size at read "+std::to_string(dest_len)+" elems: "+rb.toString(), preSize >= dest_len);
        REQUIRE_MSG("not empty "+rb.toString(), !rb.isEmpty());

        for(jau::nsize_t i=0; i<dest_len; i++) {
            Value_type svI = getDefault<Value_type>();
            REQUIRE_MSG("not empty at read #"+std::to_string(i)+": "+rb.toString(), rb.get(svI));
            REQUIRE_MSG("value at read #"+std::to_string(i)+": "+rb.toString(), startValue+(Integral_type)i == getValue<Integral_type, Value_type>(svI));
        }

        REQUIRE_MSG("size "+rb.toString(), preSize-dest_len == rb.size());
        REQUIRE_MSG("free slots after reading "+std::to_string(dest_len)+": "+rb.toString(), rb.freeSlots()>= dest_len);
        REQUIRE_MSG("not full "+rb.toString(), !rb.isFull());
    }

    void mtReadTestImpl(ringbuffer_t &rb, jau::nsize_t capacity, jau::nsize_t dest_len, Integral_type startValue) {
        REQUIRE_MSG("capacity "+rb.toString(), capacity == rb.capacity());
        REQUIRE_MSG("capacity at read "+std::to_string(dest_len)+" elems: "+rb.toString(), capacity >= dest_len);

        for(jau::nsize_t i=0; i<dest_len; i++) {
            Value_type svI = getDefault<Value_type>();
            REQUIRE_MSG("not empty at read #"+std::to_string(i)+" / "+std::to_string(dest_len), rb.getBlocking(svI));
            REQUIRE_MSG("value at read #"+std::to_string(i)+" / "+std::to_string(dest_len)+" @ "+std::to_string(startValue), startValue+(Integral_type)i == getValue<Integral_type, Value_type>(svI));
        }
        REQUIRE_MSG("free slots after reading "+std::to_string(dest_len)+": "+rb.toString(), rb.freeSlots()>= dest_len);
        REQUIRE_MSG("not full "+rb.toString(), !rb.isFull());
    }

    void readRangeTestImpl(ringbuffer_t &rb, jau::nsize_t capacity, jau::nsize_t dest_len, Integral_type startValue) {
        jau::nsize_t preSize = rb.size();
        REQUIRE_MSG("capacity "+rb.toString(), capacity == rb.capacity());
        REQUIRE_MSG("capacity at read "+std::to_string(dest_len)+" elems: "+rb.toString(), capacity >= dest_len);
        REQUIRE_MSG("size at read "+std::to_string(dest_len)+" elems: "+rb.toString(), preSize >= dest_len);
        REQUIRE_MSG("not empty "+rb.toString(), !rb.isEmpty());

        std::vector<Value_type> array(dest_len);
        REQUIRE_MSG("get-range of "+std::to_string(array.size())+" elem in "+rb.toString(), dest_len==rb.get( &(*array.begin()), dest_len, dest_len) );

        REQUIRE_MSG("size "+rb.toString(), preSize-dest_len == rb.size());
        REQUIRE_MSG("free slots after reading "+std::to_string(dest_len)+": "+rb.toString(), rb.freeSlots()>= dest_len);
        REQUIRE_MSG("not full "+rb.toString(), !rb.isFull());

        for(jau::nsize_t i=0; i<dest_len; i++) {
            Value_type svI = array[i];
            REQUIRE_MSG("value at read #"+std::to_string(i)+": "+rb.toString(), startValue+(Integral_type)i == getValue<Integral_type, Value_type>(svI));
        }
    }

    void readRangeTestImpl2(ringbuffer_t &rb, jau::nsize_t capacity, jau::nsize_t dest_len, jau::nsize_t min_count, Integral_type startValue) {
        jau::nsize_t preSize = rb.size();
        REQUIRE_MSG("capacity "+rb.toString(), capacity == rb.capacity());
        REQUIRE_MSG("capacity at read "+std::to_string(dest_len)+" elems: "+rb.toString(), capacity >= dest_len);
        REQUIRE_MSG("size at read "+std::to_string(dest_len)+" elems: "+rb.toString(), preSize >= dest_len);
        REQUIRE_MSG("not empty "+rb.toString(), !rb.isEmpty());

        std::vector<Value_type> array(dest_len);
        REQUIRE_MSG("get-range of "+std::to_string(array.size())+" elem in "+rb.toString(), dest_len==rb.get( &(*array.begin()), dest_len, min_count) );

        REQUIRE_MSG("size "+rb.toString(), preSize-dest_len == rb.size());
        REQUIRE_MSG("free slots after reading "+std::to_string(dest_len)+": "+rb.toString(), rb.freeSlots()>= dest_len);
        REQUIRE_MSG("not full "+rb.toString(), !rb.isFull());

        for(jau::nsize_t i=0; i<dest_len; i++) {
            Value_type svI = array[i];
            REQUIRE_MSG("value at read #"+std::to_string(i)+": "+rb.toString(), startValue+(Integral_type)i == getValue<Integral_type, Value_type>(svI));
        }
    }

    jau::nsize_t mtReadRangeTestImpl(ringbuffer_t &rb, jau::nsize_t capacity, jau::nsize_t dest_len, jau::nsize_t min_count, Integral_type startValue) {
        REQUIRE_MSG("capacity "+rb.toString(), capacity == rb.capacity());
        REQUIRE_MSG("capacity at read "+std::to_string(dest_len)+" elems: "+rb.toString(), capacity >= dest_len);

        std::vector<Value_type> array(dest_len);
        const jau::nsize_t count = rb.getBlocking( &(*array.begin()), dest_len, min_count);
        REQUIRE_MSG("get-range >= min_count / "+std::to_string(array.size())+" of "+rb.toString(), min_count <= count);

        for(jau::nsize_t i=0; i<count; i++) {
            Value_type svI = array[i];
            REQUIRE_MSG("value at read #"+std::to_string(i)+" / "+std::to_string(count)+" @ "+std::to_string(startValue), startValue+(Integral_type)i == getValue<Integral_type, Value_type>(svI));
        }
        return count;
    }

    void writeTestImpl(ringbuffer_t &rb, jau::nsize_t capacity, jau::nsize_t len, Integral_type startValue) {
        jau::nsize_t preSize = rb.size();

        REQUIRE_MSG("capacity "+rb.toString(), capacity == rb.capacity());
        REQUIRE_MSG("capacity at write "+std::to_string(len)+" elems: "+rb.toString(), capacity >= len);
        REQUIRE_MSG("size at write "+std::to_string(len)+" elems: "+rb.toString(), preSize+len <= capacity);
        REQUIRE_MSG("not full "+rb.toString(), !rb.isFull());

        for(jau::nsize_t i=0; i<len; i++) {
            std::string m = "buffer put #"+std::to_string(i)+": "+rb.toString();
            REQUIRE_MSG(m, rb.put( createValue<Integral_type, Value_type>( (Integral_type)( startValue+i ) ) ) );
        }

        REQUIRE_MSG("size "+rb.toString(), preSize+len == rb.size());
        REQUIRE_MSG("not empty "+rb.toString(), !rb.isEmpty());
    }

    void mtWriteTestImpl(ringbuffer_t &rb, jau::nsize_t capacity, jau::nsize_t len, Integral_type startValue, jau::nsize_t period) {
        REQUIRE_MSG("capacity "+rb.toString(), capacity == rb.capacity());
        REQUIRE_MSG("capacity at write "+std::to_string(len)+" elems: "+rb.toString(), capacity >= len);
        REQUIRE_MSG("not full "+rb.toString(), !rb.isFull());

        for(jau::nsize_t i=0; i<len; i++) {
            std::string m = "buffer put #"+std::to_string(i)+": "+rb.toString();
            REQUIRE_MSG(m, rb.put( createValue<Integral_type, Value_type>( (Integral_type)( startValue+i ) ) ) );
            std::this_thread::sleep_for(std::chrono::milliseconds(period));
        }
    }

    void writeRangeTestImpl(ringbuffer_t &rb, jau::nsize_t capacity, const std::vector<Value_type> & data) {
        jau::nsize_t preSize = rb.size();
        jau::nsize_t postSize = preSize+data.size();

        REQUIRE_MSG("capacity "+rb.toString(), capacity == rb.capacity());
        REQUIRE_MSG("capacity at write "+std::to_string(data.size())+" elems: "+rb.toString(), capacity >= data.size());
        REQUIRE_MSG("size at write "+std::to_string(data.size())+" elems: "+rb.toString(), postSize<= capacity);
        REQUIRE_MSG("not full "+rb.toString(), !rb.isFull());
        REQUIRE_MSG("data fits in RB capacity "+rb.toString(), rb.capacity() >= data.size());
        REQUIRE_MSG("data fits in RB free-slots "+rb.toString(), rb.freeSlots() >= data.size());

        REQUIRE_MSG("put-range of "+std::to_string(data.size())+" elem in "+rb.toString(), rb.put( &(*data.begin()), &(*data.end()) ) );

        REQUIRE_MSG("size "+rb.toString(), postSize == rb.size());
        REQUIRE_MSG("not empty "+rb.toString(), !rb.isEmpty());
    }

    void moveGetPutImpl(ringbuffer_t &rb, jau::nsize_t pos) {
        REQUIRE_MSG("not empty "+rb.toString(), !rb.isEmpty());
        for(jau::nsize_t i=0; i<pos; i++) {
            Value_type svI = getDefault<Value_type>();
            REQUIRE_MSG("moveFull.get "+rb.toString(), rb.get(svI));
            REQUIRE_MSG("moveFull.get "+rb.toString(), (Integral_type)i == getValue<Integral_type, Value_type>(svI));
            REQUIRE_MSG("moveFull.put "+rb.toString(), rb.put( createValue<Integral_type, Value_type>( (Integral_type)i ) ) );
        }
    }

    void movePutGetImpl(ringbuffer_t &rb, jau::nsize_t pos) {
        REQUIRE_MSG("RB is full "+rb.toString(), !rb.isFull());
        for(jau::nsize_t i=0; i<pos; i++) {
            REQUIRE_MSG("moveEmpty.put "+rb.toString(), rb.put( createValue<Integral_type, Value_type>( (Integral_type)( 600+i ) ) ) );
            Value_type svI = getDefault<Value_type>();
            REQUIRE_MSG("moveEmpty.get "+rb.toString(), rb.get(svI));
            REQUIRE_MSG("moveEmpty.get "+rb.toString(), 600+(Integral_type)i == getValue<Integral_type, Value_type>(svI));
        }
    }

  public:

    void testS00_PrintInfo() {
        ringbuffer_t rb = createEmpty(11);

        std::string msg("Ringbuffer: uses_memcpy "+std::to_string(ringbuffer_t::uses_memcpy)+
                 ", trivially_copyable "+std::to_string(std::is_trivially_copyable<typename ringbuffer_t::value_type>::value)+
                 ", size "+std::to_string(sizeof(rb))+" bytes");
        fprintf(stderr, "%s\n", msg.c_str());
        fprintf(stderr, "%s\n", rb.get_info().c_str());
        REQUIRE_MSG("Ringbuffer<T> memmove", ringbuffer_t::uses_memmove == exp_memmove);
        REQUIRE_MSG("Ringbuffer<T> memcpy", ringbuffer_t::uses_memcpy == exp_memcpy);
        REQUIRE_MSG("Ringbuffer<T> secmem", ringbuffer_t::uses_secmem == exp_secmem);
    }

    void testS01_FullRead() {
        jau::nsize_t capacity = 11;
        std::vector<Value_type> source = createIntArray(capacity, 0);
        ringbuffer_t rb = createFull(source);
        INFO_STR("testS01_FullRead: Created / "+ rb.toString());
        REQUIRE_MSG("full size "+rb.toString(), capacity == rb.size());
        REQUIRE_MSG("full "+rb.toString(), rb.isFull());

        readTestImpl(rb, capacity, capacity, 0);
        INFO_STR("testS01_FullRead: PostRead / " + rb.toString());
        REQUIRE_MSG("empty "+rb.toString(), rb.isEmpty());
    }

    void testS02_SingleRW01() {
        jau::nsize_t capacity = 11;
        ringbuffer_t rb = createEmpty(capacity);
        INFO( std::string("testS02_SingleRW01: Created / ") + rb.toString().c_str());
        REQUIRE_MSG("zero size "+rb.toString(), 0 == rb.size());
        REQUIRE_MSG("empty "+rb.toString(), rb.isEmpty());

        writeTestImpl(rb, capacity, capacity, 0);
        INFO( std::string("testS02_SingleRW01: PostWrite / ") + rb.toString().c_str());
        REQUIRE_MSG("full size "+rb.toString(), capacity == rb.size());
        REQUIRE_MSG("full "+rb.toString(), rb.isFull());

        readTestImpl(rb, capacity, capacity, 0);
        INFO( std::string("testS02_SingleRW01: PostRead / ") + rb.toString().c_str());
        REQUIRE_MSG("empty "+rb.toString(), rb.isEmpty());
    }

    void testM02_SingleRW01(const jau::nsize_t element_count=100, const jau::nsize_t sleep_period=5) {
        /**
         * Test Details
         * - One producer thread + current consumer thread
         * - Producer period of sleep_period, producing element_count elements.
         * - Consumer w/o delay (as-fast-as-possible), will grab one element each single call.
         */
        {
            const jau::nsize_t capacity = 4096;
            ringbuffer_t rb = createEmpty(capacity);
            INFO( std::string("testM02_SingleRW01: Created / ") + rb.toString().c_str());
            REQUIRE_MSG("zero size "+rb.toString(), 0 == rb.size());
            REQUIRE_MSG("empty "+rb.toString(), rb.isEmpty());

            /**
             * Move R == W == 0
             * Empty [RW][][ ][  ][ ][ ][ ][ ][ ][ ][ ] ; start
             */
            std::thread producer01(&test_ringbuffer_t::mtWriteTestImpl, this, std::ref(rb), capacity, element_count, 0 /* start value */, sleep_period); // @suppress("Invalid arguments")

            mtReadTestImpl(rb, capacity, element_count, 0);
            if( producer01.joinable() ) {
                producer01.join();
            }
            INFO( std::string("testM02_SingleRW01: PostRead / ") + rb.toString().c_str());
            REQUIRE_MSG("empty "+rb.toString(), rb.isEmpty());
        }
        /**
         * Test Details
         * - Two producer threads for two ringbuffer + current consumer thread on both ringbuffer.
         * - Producer period of sleep_period, producing element_count elements.
         * - Consumer w/ 5 * sleep_period delay after each 4 elements, will grab each single element.
         */
        {
            const jau::nsize_t capacity = 4096;

            ringbuffer_t rb1 = createEmpty(capacity);
            INFO( std::string("testM03a_RangeRW01: Created.1 / ") + rb1.toString().c_str());
            REQUIRE_MSG("zero size.1 "+rb1.toString(), 0 == rb1.size());
            REQUIRE_MSG("empty.1 "+rb1.toString(), rb1.isEmpty());

            ringbuffer_t rb2 = createEmpty(capacity);
            INFO( std::string("testM03a_RangeRW01: Created.2 / ") + rb2.toString().c_str());
            REQUIRE_MSG("zero size.2 "+rb2.toString(), 0 == rb2.size());
            REQUIRE_MSG("empty.2 "+rb2.toString(), rb2.isEmpty());

            /**
             * Move R == W == 0
             * Empty [RW][][ ][  ][ ][ ][ ][ ][ ][ ][ ] ; start
             */
            std::thread producer01(&test_ringbuffer_t::mtWriteTestImpl, this, std::ref(rb1), capacity, element_count, 0 /* start value */, sleep_period); // @suppress("Invalid arguments")
            std::thread producer02(&test_ringbuffer_t::mtWriteTestImpl, this, std::ref(rb2), capacity, element_count, 0 /* start value */, sleep_period); // @suppress("Invalid arguments")

            jau::nsize_t count1 = 0;
            jau::nsize_t count2 = 0;
            jau::nsize_t loop = 0;

            while(count1 < element_count || count2 < element_count) {
                Value_type svI = getDefault<Value_type>();
                if( count1 < element_count ) {
                    REQUIRE_MSG("not empty at read.1 #"+std::to_string(count1)+" / "+std::to_string(element_count), rb1.getBlocking(svI));
                    REQUIRE_MSG("value at read.1 #"+std::to_string(count1)+" / "+std::to_string(element_count), (Integral_type)count1 == getValue<Integral_type, Value_type>(svI));
                    ++count1;
                }
                if( count2 < element_count ) {
                    REQUIRE_MSG("not empty at read.2 #"+std::to_string(count2)+" / "+std::to_string(element_count), rb2.getBlocking(svI));
                    REQUIRE_MSG("value at read.2 #"+std::to_string(count2)+" / "+std::to_string(element_count), (Integral_type)count2 == getValue<Integral_type, Value_type>(svI));
                    ++count2;
                }
                if( 0 == ( ++loop % 4 ) ) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(5*sleep_period));
                }
            }
            if( producer01.joinable() ) {
                producer01.join();
            }
            if( producer02.joinable() ) {
                producer02.join();
            }
            INFO( std::string("testM03a_RangeRW01: PostRead.1 / ") + rb1.toString().c_str());
            INFO( std::string("testM03a_RangeRW01: PostRead.2 / ") + rb2.toString().c_str());
            REQUIRE_MSG("got all elements count.1 == element_count "+rb1.toString(), count1 == element_count);
            REQUIRE_MSG("got all elements count.2 == element_count "+rb1.toString(), count2 == element_count);
            REQUIRE_MSG("empty.1 "+rb1.toString(), rb1.isEmpty());
            REQUIRE_MSG("empty.2 "+rb2.toString(), rb2.isEmpty());
        }
    }

    void testS03a_RangeRW01() {
        {
            jau::nsize_t capacity = 2*11;
            ringbuffer_t rb = createEmpty(capacity);
            INFO( std::string("testS03a_RangeRW01: Created / ") + rb.toString().c_str());
            REQUIRE_MSG("zero size "+rb.toString(), 0 == rb.size());
            REQUIRE_MSG("empty "+rb.toString(), rb.isEmpty());

            /**
             * Move R == W == 0
             * Empty [RW][][ ][  ][ ][ ][ ][ ][ ][ ][ ] ; start
             */
            // std::vector<Value_type> new_data = createIntArray(capacity, 0);
            // writeRangeTestImpl(rb, capacity, new_data);
            writeTestImpl(rb, capacity, capacity, 0);

            INFO( std::string("testS03a_RangeRW01: PostWrite / ") + rb.toString().c_str());
            REQUIRE_MSG("full size "+rb.toString(), capacity == rb.size());
            REQUIRE_MSG("full "+rb.toString(), rb.isFull());

            readRangeTestImpl(rb, capacity, capacity/2, 0);
            INFO( std::string("testS03a_RangeRW01: PostRead-1 / ") + rb.toString().c_str());
            REQUIRE_MSG("not empty "+rb.toString(), !rb.isEmpty());

            readRangeTestImpl(rb, capacity, capacity/2, capacity/2);
            INFO( std::string("testS03a_RangeRW01: PostRead-2 / ") + rb.toString().c_str());
            REQUIRE_MSG("empty "+rb.toString(), rb.isEmpty());
        }
        {
            jau::nsize_t capacity = 2*11;
            ringbuffer_t rb = createEmpty(capacity);
            INFO( std::string("testS03a_RangeRW01: Created / ") + rb.toString().c_str());
            REQUIRE_MSG("zero size "+rb.toString(), 0 == rb.size());
            REQUIRE_MSG("empty "+rb.toString(), rb.isEmpty());

            /**
             * Move R == W == 0
             * Empty [RW][][ ][  ][ ][ ][ ][ ][ ][ ][ ] ; start
             */
            // std::vector<Value_type> new_data = createIntArray(capacity, 0);
            // writeRangeTestImpl(rb, capacity, new_data);
            writeTestImpl(rb, capacity, capacity, 0);

            INFO( std::string("testS03a_RangeRW01: PostWrite / ") + rb.toString().c_str());
            REQUIRE_MSG("full size "+rb.toString(), capacity == rb.size());
            REQUIRE_MSG("full "+rb.toString(), rb.isFull());

            readRangeTestImpl2(rb, capacity, capacity/2, 1, 0);
            INFO( std::string("testS03a_RangeRW01: PostRead-1 / ") + rb.toString().c_str());
            REQUIRE_MSG("not empty "+rb.toString(), !rb.isEmpty());

            readRangeTestImpl2(rb, capacity, capacity/2, 1, capacity/2);
            INFO( std::string("testS03a_RangeRW01: PostRead-2 / ") + rb.toString().c_str());
            REQUIRE_MSG("empty "+rb.toString(), rb.isEmpty());
        }
    }

    void testM03a_RangeRW01(const jau::nsize_t element_count=100, const jau::nsize_t sleep_period=5) {
        /**
         * Test Details
         * - One producer thread + current consumer thread
         * - Producer period of sleep_period, producing element_count elements.
         * - Consumer w/o delay (as-fast-as-possible), will grab one element each range call likely.
         */
        {
            const jau::nsize_t capacity = 4096;
            ringbuffer_t rb = createEmpty(capacity);
            INFO( std::string("testM03a_RangeRW01: Created / ") + rb.toString().c_str());
            REQUIRE_MSG("zero size "+rb.toString(), 0 == rb.size());
            REQUIRE_MSG("empty "+rb.toString(), rb.isEmpty());

            /**
             * Move R == W == 0
             * Empty [RW][][ ][  ][ ][ ][ ][ ][ ][ ][ ] ; start
             */
            std::thread producer01(&test_ringbuffer_t::mtWriteTestImpl, this, std::ref(rb), capacity, element_count, 0 /* start value */, sleep_period); // @suppress("Invalid arguments")

            const jau::nsize_t min_count = 1;
            jau::nsize_t count = 0;

            while(count < element_count) {
                const jau::nsize_t c = mtReadRangeTestImpl(rb, capacity, element_count, min_count, (Integral_type)count);
                REQUIRE_MSG("got elements >= min_count "+rb.toString(), c >= min_count);
                count += c;
            }
            if( producer01.joinable() ) {
                producer01.join();
            }
            INFO( std::string("testM03a_RangeRW01: PostRead / ") + rb.toString().c_str());
            REQUIRE_MSG("got all elements count == element_count "+rb.toString(), count == element_count);
            REQUIRE_MSG("empty "+rb.toString(), rb.isEmpty());
        }
        /**
         * Test Details
         * - Two producer threads for two ringbuffer + current consumer thread on both ringbuffer.
         * - Producer period of sleep_period, producing element_count elements.
         * - Consumer w/ 5 * sleep_period delay, will grab five elements each range call likely.
         */
        {
            const jau::nsize_t capacity = 4096;

            ringbuffer_t rb1 = createEmpty(capacity);
            INFO( std::string("testM03a_RangeRW01: Created.1 / ") + rb1.toString().c_str());
            REQUIRE_MSG("zero size.1 "+rb1.toString(), 0 == rb1.size());
            REQUIRE_MSG("empty.1 "+rb1.toString(), rb1.isEmpty());

            ringbuffer_t rb2 = createEmpty(capacity);
            INFO( std::string("testM03a_RangeRW01: Created.2 / ") + rb2.toString().c_str());
            REQUIRE_MSG("zero size.2 "+rb2.toString(), 0 == rb2.size());
            REQUIRE_MSG("empty.2 "+rb2.toString(), rb2.isEmpty());

            /**
             * Move R == W == 0
             * Empty [RW][][ ][  ][ ][ ][ ][ ][ ][ ][ ] ; start
             */
            std::thread producer01(&test_ringbuffer_t::mtWriteTestImpl, this, std::ref(rb1), capacity, element_count, 0 /* start value */, sleep_period); // @suppress("Invalid arguments")
            std::thread producer02(&test_ringbuffer_t::mtWriteTestImpl, this, std::ref(rb2), capacity, element_count, 0 /* start value */, sleep_period); // @suppress("Invalid arguments")

            const jau::nsize_t min_count = 1;
            jau::nsize_t count1 = 0;
            jau::nsize_t count2 = 0;

            while(count1 < element_count || count2 < element_count) {
                if( count1 < element_count ) {
                    const jau::nsize_t c = mtReadRangeTestImpl(rb1, capacity, element_count, min_count, (Integral_type)count1);
                    REQUIRE_MSG("got elements.1 >= min_count "+rb1.toString(), c >= min_count);
                    count1 += c;
                }
                if( count2 < element_count ) {
                    const jau::nsize_t c = mtReadRangeTestImpl(rb2, capacity, element_count, min_count, (Integral_type)count2);
                    REQUIRE_MSG("got elements.2 >= min_count "+rb2.toString(), c >= min_count);
                    count2 += c;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(5*sleep_period));
            }
            if( producer01.joinable() ) {
                producer01.join();
            }
            if( producer02.joinable() ) {
                producer02.join();
            }
            INFO( std::string("testM03a_RangeRW01: PostRead.1 / ") + rb1.toString().c_str());
            INFO( std::string("testM03a_RangeRW01: PostRead.2 / ") + rb2.toString().c_str());
            REQUIRE_MSG("got all elements count.1 == element_count "+rb1.toString(), count1 == element_count);
            REQUIRE_MSG("got all elements count.2 == element_count "+rb1.toString(), count2 == element_count);
            REQUIRE_MSG("empty.1 "+rb1.toString(), rb1.isEmpty());
            REQUIRE_MSG("empty.2 "+rb2.toString(), rb2.isEmpty());
        }
    }

    void testS03b_RangeRW02() {
        {
            jau::nsize_t capacity = 2*11;
            ringbuffer_t rb = createEmpty(capacity);
            INFO( std::string("testS03b_RangeRW02: Created / ") + rb.toString().c_str());
            REQUIRE_MSG("zero size "+rb.toString(), 0 == rb.size());
            REQUIRE_MSG("empty "+rb.toString(), rb.isEmpty());

            /**
             * Move R == W == 0
             * Empty [RW][][ ][  ][ ][ ][ ][ ][ ][ ][ ] ; start
             */
            std::vector<Value_type> new_data = createIntArray(capacity, 0);
            writeRangeTestImpl(rb, capacity, new_data);

            INFO( std::string("testS03b_RangeRW02: PostWrite / ") + rb.toString().c_str());
            REQUIRE_MSG("full size "+rb.toString(), capacity == rb.size());
            REQUIRE_MSG("full "+rb.toString(), rb.isFull());

            readRangeTestImpl(rb, capacity, capacity/2, 0);
            INFO( std::string("testS03b_RangeRW02: PostRead-1 / ") + rb.toString().c_str());
            REQUIRE_MSG("not empty "+rb.toString(), !rb.isEmpty());

            readRangeTestImpl(rb, capacity, capacity/2, capacity/2);
            INFO( std::string("testS03b_RangeRW02: PostRead-2 / ") + rb.toString().c_str());
            REQUIRE_MSG("empty "+rb.toString(), rb.isEmpty());
        }
        {
            jau::nsize_t capacity = 2*11;

            ringbuffer_t rb = createEmpty(capacity);
            INFO( std::string("testS03b_RangeRW02: Created / ") + rb.toString().c_str());
            REQUIRE_MSG("zero size "+rb.toString(), 0 == rb.size());
            REQUIRE_MSG("empty "+rb.toString(), rb.isEmpty());

            /**
             * Move R == W == 3
             * Empty [RW][][ ][  ][ ][ ][ ][ ][ ][ ][ ] ; start
             * Empty [ ][ ][ ][RW][ ][ ][ ][ ][ ][ ][ ]
             */
            Value_type dummy = getDefault<Value_type>();
            rb.put(dummy);
            rb.put(dummy);
            rb.put(dummy);
            rb.drop(3);

            std::vector<Value_type> new_data = createIntArray(capacity, 0);
            writeRangeTestImpl(rb, capacity, new_data);
            // writeTestImpl(rb, capacity, capacity, 0);

            INFO( std::string("testS03b_RangeRW02: PostWrite / ") + rb.toString().c_str());
            REQUIRE_MSG("full size "+rb.toString(), capacity == rb.size());
            REQUIRE_MSG("full "+rb.toString(), rb.isFull());

            readRangeTestImpl(rb, capacity, capacity/2, 0);
            INFO( std::string("testS03b_RangeRW02: PostRead-1 / ") + rb.toString().c_str());
            REQUIRE_MSG("not empty "+rb.toString(), !rb.isEmpty());

            readRangeTestImpl(rb, capacity, capacity/2, capacity/2);
            INFO( std::string("testS03b_RangeRW02: PostRead-2 / ") + rb.toString().c_str());
            REQUIRE_MSG("size 0 "+rb.toString(), 0 == rb.size());
            REQUIRE_MSG("empty "+rb.toString(), rb.isEmpty());
        }
        {
            jau::nsize_t capacity = 2*11;
            ringbuffer_t rb = createEmpty(capacity);
            INFO( std::string("testS03b_RangeRW02: Created / ") + rb.toString().c_str());
            REQUIRE_MSG("zero size "+rb.toString(), 0 == rb.size());
            REQUIRE_MSG("empty "+rb.toString(), rb.isEmpty());

            /**
             * Move R == 2, W == 4, size 2
             * Empty [RW][][ ][ ][ ][ ][ ][ ][ ][ ][ ] ; start
             * Avail [ ][ ][R][.][W][ ][ ][ ][ ][ ][ ] ; W > R
             */
            Value_type dummy = getDefault<Value_type>();
            rb.put(dummy); // r idx 0 -> 1
            rb.put(dummy);
            rb.put(dummy);
            rb.put(dummy); // r idx 3 -> 4
            rb.drop(2);    // r idx 0 -> 2

            // left = 22 - 2
            REQUIRE_MSG("size 2 "+rb.toString(), 2 == rb.size());
            REQUIRE_MSG("available 11-2 "+rb.toString(), capacity-2 == rb.freeSlots());

            std::vector<Value_type> new_data = createIntArray(capacity-2, 0);
            writeRangeTestImpl(rb, capacity, new_data);
            // writeTestImpl(rb, capacity, capacity-2, 0);

            INFO( std::string("testS03b_RangeRW02: PostWrite / ") + rb.toString().c_str());
            REQUIRE_MSG("full size "+rb.toString(), capacity == rb.size());
            REQUIRE_MSG("full "+rb.toString(), rb.isFull());

            // take off 2 remaining dummies
            rb.drop(2);
            REQUIRE_MSG("size capacity-2 "+rb.toString(), capacity-2 == rb.size());

            readRangeTestImpl(rb, capacity, capacity/2-2, 0);
            INFO( std::string("testS03b_RangeRW02: PostRead-1 / ") + rb.toString().c_str());
            REQUIRE_MSG("not empty "+rb.toString(), !rb.isEmpty());

            readRangeTestImpl(rb, capacity, capacity/2, capacity/2-2);
            INFO( std::string("testS03b_RangeRW02: PostRead-2 / ") + rb.toString().c_str());
            REQUIRE_MSG("size 0 "+rb.toString(), 0 == rb.size());
            REQUIRE_MSG("empty "+rb.toString(), rb.isEmpty());
        }
        {
            jau::nsize_t capacity = 2*11;
            ringbuffer_t rb = createEmpty(capacity);
            INFO( std::string("testS03b_RangeRW02: Created / ") + rb.toString().c_str());
            REQUIRE_MSG("zero size "+rb.toString(), 0 == rb.size());
            REQUIRE_MSG("empty "+rb.toString(), rb.isEmpty());

            /**
             * Move R == 9, W == 1, size 3
             * Empty [RW][][ ][ ][ ][ ][ ][ ][ ][ ][ ] ; start
             * Avail [.][W][ ][ ][ ][ ][ ][ ][ ][R][.] ; W < R - 1
             */
            Value_type dummy = getDefault<Value_type>();
            for(jau::nsize_t i=0; i<capacity; i++) { rb.put(dummy); } // fill all
            REQUIRE_MSG("full "+rb.toString(), rb.isFull());

            // for(int i=0; i<10; i++) { rb.get(); } // pull
            rb.drop(capacity-1); // pull
            REQUIRE_MSG("size 1"+rb.toString(), 1 == rb.size());

            for(int i=0; i<2; i++) { rb.put(dummy); } // fill 2 more
            REQUIRE_MSG("size 3"+rb.toString(), 3 == rb.size());

            // left = 22 - 3
            REQUIRE_MSG("available 22-3 "+rb.toString(), capacity-3 == rb.freeSlots());

            std::vector<Value_type> new_data = createIntArray(capacity-3, 0);
            writeRangeTestImpl(rb, capacity, new_data);
            // writeTestImpl(rb, capacity, capacity-3, 0);

            INFO( std::string("testS03b_RangeRW02: PostWrite / ") + rb.toString().c_str());
            REQUIRE_MSG("full size "+rb.toString(), capacity == rb.size());
            REQUIRE_MSG("full "+rb.toString(), rb.isFull());

            // take off 3 remaining dummies
            rb.drop(3); // pull
            // for(int i=0; i<3; i++) { rb.get(); } // pull
            REQUIRE_MSG("size capacity-3 "+rb.toString(), capacity-3 == rb.size());

            readRangeTestImpl(rb, capacity, capacity/2-3, 0);
            INFO( std::string("testS03b_RangeRW02: PostRead-1 / ") + rb.toString().c_str());
            REQUIRE_MSG("not empty "+rb.toString(), !rb.isEmpty());

            readRangeTestImpl(rb, capacity, capacity/2, capacity/2-3);
            INFO( std::string("testS03b_RangeRW02: PostRead-2 / ") + rb.toString().c_str());
            REQUIRE_MSG("size 0 "+rb.toString(), 0 == rb.size());
            REQUIRE_MSG("empty "+rb.toString(), rb.isEmpty());
        }
    }

    void testS04_FullReadReset() {
        jau::nsize_t capacity = 11;
        std::vector<Value_type> source = createIntArray(capacity, 0);
        ringbuffer_t rb = createFull(source);
        INFO_STR("testS04_FullReadReset: Created / " + rb.toString());
        REQUIRE_MSG("full "+rb.toString(), rb.isFull());

        rb.reset(source);
        INFO_STR("testS04_FullReadReset: Post Reset w/ source / " + rb.toString());
        REQUIRE_MSG("full "+rb.toString(), rb.isFull());

        readTestImpl(rb, capacity, capacity, 0);
        INFO_STR("testS04_FullReadReset: Post Read / " + rb.toString());
        REQUIRE_MSG("empty "+rb.toString(), rb.isEmpty());

        rb.reset(source);
        INFO_STR("testS04_FullReadReset: Post Reset w/ source / " + rb.toString());
        REQUIRE_MSG("full "+rb.toString(), rb.isFull());

        readTestImpl(rb, capacity, capacity, 0);
        INFO_STR("testS04_FullReadReset: Post Read / " + rb.toString());
        REQUIRE_MSG("empty "+rb.toString(), rb.isEmpty());
    }

    void testS05_EmptyWriteClear() {
        jau::nsize_t capacity = 11;
        ringbuffer_t rb = createEmpty(capacity);
        REQUIRE_MSG("empty "+rb.toString(), rb.isEmpty());

        rb.clear();
        REQUIRE_MSG("empty "+rb.toString(), rb.isEmpty());

        writeTestImpl(rb, capacity, capacity, 0);
        REQUIRE_MSG("full "+rb.toString(), rb.isFull());

        readTestImpl(rb, capacity, capacity, 0);
        REQUIRE_MSG("empty "+rb.toString(), rb.isEmpty());

        rb.clear();
        REQUIRE_MSG("empty "+rb.toString(), rb.isEmpty());

        writeTestImpl(rb, capacity, capacity, 0);
        REQUIRE_MSG("full "+rb.toString(), rb.isFull());

        readTestImpl(rb, capacity, capacity, 0);
        REQUIRE_MSG("empty "+rb.toString(), rb.isEmpty());
    }

    void testS06_ReadResetMid01() {
        jau::nsize_t capacity = 11;
        std::vector<Value_type> source = createIntArray(capacity, 0);
        ringbuffer_t rb = createFull(source);
        REQUIRE_MSG("full "+rb.toString(), rb.isFull());

        rb.reset(source);
        REQUIRE_MSG("full "+rb.toString(), rb.isFull());

        readTestImpl(rb, capacity, 5, 0);
        REQUIRE_MSG("not empty "+rb.toString(), !rb.isEmpty());
        REQUIRE_MSG("not Full "+rb.toString(), !rb.isFull());

        rb.reset(source);
        REQUIRE_MSG("full "+rb.toString(), rb.isFull());

        readTestImpl(rb, capacity, capacity, 0);
        REQUIRE_MSG("empty "+rb.toString(), rb.isEmpty());
    }

    void testS07_ReadResetMid02() {
        jau::nsize_t capacity = 11;
        std::vector<Value_type> source = createIntArray(capacity, 0);
        ringbuffer_t rb = createFull(source);
        REQUIRE_MSG("full "+rb.toString(), rb.isFull());

        rb.reset(source);
        REQUIRE_MSG("full "+rb.toString(), rb.isFull());

        moveGetPutImpl(rb, 5);
        readTestImpl(rb, capacity, 5, 5);
        REQUIRE_MSG("not empty "+rb.toString(), !rb.isEmpty());
        REQUIRE_MSG("not Full "+rb.toString(), !rb.isFull());

        rb.reset(source);
        REQUIRE_MSG("full "+rb.toString(), rb.isFull());

        readTestImpl(rb, capacity, capacity, 0);
        REQUIRE_MSG("empty "+rb.toString(), rb.isEmpty());
    }

  private:

    void test_GrowFullImpl(jau::nsize_t initialCapacity, jau::nsize_t pos) {
        jau::nsize_t growAmount = 5;
        jau::nsize_t grownCapacity = initialCapacity+growAmount;
        std::vector<Value_type> source = createIntArray(initialCapacity, 0);
        ringbuffer_t rb = createFull(source);

        for(jau::nsize_t i=0; i<initialCapacity; i++) {
            Value_type svI = getDefault<Value_type>();
            REQUIRE_MSG("not empty at read #"+std::to_string(i)+": "+rb.toString(), rb.get(svI));
            REQUIRE_MSG("value at read #"+std::to_string(i)+": "+rb.toString(), Integral_type((0+i)%initialCapacity) == getValue<Integral_type, Value_type>(svI));
        }
        REQUIRE_MSG("zero size "+rb.toString(), 0 == rb.size());

        rb.reset(source);
        REQUIRE_MSG("orig size "+rb.toString(), initialCapacity == rb.size());

        moveGetPutImpl(rb, pos);
        // PRINTM("X02 "+rb.toString());
        // rb.dump(stderr, "X02");

        rb.recapacity(grownCapacity);
        REQUIRE_MSG("capacity "+rb.toString(), grownCapacity == rb.capacity());
        REQUIRE_MSG("orig size "+rb.toString(), initialCapacity == rb.size());
        REQUIRE_MSG("not full "+rb.toString(), !rb.isFull());
        REQUIRE_MSG("not empty "+rb.toString(), !rb.isEmpty());
        // PRINTM("X03 "+rb.toString());
        // rb.dump(stderr, "X03");

        for(jau::nsize_t i=0; i<growAmount; i++) {
            REQUIRE_MSG("buffer not full at put #"+std::to_string(i)+": "+rb.toString(), rb.put( createValue<Integral_type, Value_type>( (Integral_type)(100+i) ) ) );
        }
        REQUIRE_MSG("new size "+rb.toString(), grownCapacity == rb.size());
        REQUIRE_MSG("full "+rb.toString(), rb.isFull());

        for(jau::nsize_t i=0; i<initialCapacity; i++) {
            Value_type svI = getDefault<Value_type>();
            REQUIRE_MSG("not empty at read #"+std::to_string(i)+": "+rb.toString(), rb.get(svI));
            REQUIRE_MSG("value at read #"+std::to_string(i)+": "+rb.toString(), Integral_type((pos+i)%initialCapacity) == getValue<Integral_type, Value_type>(svI));
        }

        for(jau::nsize_t i=0; i<growAmount; i++) {
            Value_type svI = getDefault<Value_type>();
            REQUIRE_MSG("not empty at read #"+std::to_string(i)+": "+rb.toString(), rb.get(svI));
            REQUIRE_MSG("value at read #"+std::to_string(i)+": "+rb.toString(), Integral_type(100+i) == getValue<Integral_type, Value_type>(svI));
        }

        REQUIRE_MSG("zero size "+rb.toString(), 0 == rb.size());
        REQUIRE_MSG("empty "+rb.toString(), rb.isEmpty());

        REQUIRE_MSG("not full "+rb.toString(), !rb.isFull());
    }

  public:

    void testS20_GrowFull01_Begin() {
        test_GrowFullImpl(11, 0);
    }
    void testS21_GrowFull02_Begin1() {
        test_GrowFullImpl(11, 0+1);
    }
    void testS22_GrowFull03_Begin2() {
        test_GrowFullImpl(11, 0+2);
    }
    void testS23_GrowFull04_Begin3() {
        test_GrowFullImpl(11, 0+3);
    }
    void testS24_GrowFull05_End() {
        test_GrowFullImpl(11, 11-1);
    }
    void testS25_GrowFull11_End1() {
        test_GrowFullImpl(11, 11-1-1);
    }
    void testS26_GrowFull12_End2() {
        test_GrowFullImpl(11, 11-1-2);
    }
    void testS27_GrowFull13_End3() {
        test_GrowFullImpl(11, 11-1-3);
    }

};

template <typename Integral_type, typename Value_type, typename Size_type,
        bool exp_memmove, bool exp_memcpy, bool exp_secmem,
        bool use_memmove = std::is_trivially_copyable_v<Value_type> || is_container_memmove_compliant_v<Value_type>,
        bool use_memcpy  = std::is_trivially_copyable_v<Value_type>,
        bool use_secmem  = is_enforcing_secmem_v<Value_type>
    >
void PerformRingbufferTests() {
    typedef TestRingbuffer_A<Integral_type, Value_type, Size_type,
            exp_memmove, exp_memcpy, exp_secmem,
            use_memmove, use_memcpy, use_secmem> TestRingbuffer;

    TestRingbuffer trb;

    SECTION("testS00_PrintInfo", "[ringbuffer]") {
        trb.testS00_PrintInfo();
    }
    SECTION("testS01_FullRead", "[ringbuffer]") {
        trb.testS01_FullRead();
    }
    SECTION("testS02", "[ringbuffer]") {
        trb.testS02_SingleRW01();
    }
    SECTION("testM02", "[ringbuffer]") {
        if( catch_auto_run ) {
            trb.testM02_SingleRW01(100 /* element_count */,  1 /* sleep_period */);
        } else {
            trb.testM02_SingleRW01(100 /* element_count */,  5 /* sleep_period */);
        }
    }
    SECTION("testS03a", "[ringbuffer]") {
        trb.testS03a_RangeRW01();
    }
    SECTION("testM03a", "[ringbuffer]") {
        if( catch_auto_run ) {
            trb.testM03a_RangeRW01(100 /* element_count */,  1 /* sleep_period */);
        } else {
            trb.testM03a_RangeRW01(100 /* element_count */,  5 /* sleep_period */);
        }
    }
    SECTION("testS03b", "[ringbuffer]") {
        trb.testS03b_RangeRW02();
    }
    SECTION("testS04", "[ringbuffer]") {
        trb.testS04_FullReadReset();
    }
    SECTION("testS05", "[ringbuffer]") {
        trb.testS05_EmptyWriteClear();
    }
    SECTION("testS06", "[ringbuffer]") {
        trb.testS06_ReadResetMid01();
    }
    SECTION("testS07", "[ringbuffer]") {
        trb.testS07_ReadResetMid02();
    }
    SECTION("testS20", "[ringbuffer]") {
        trb.testS20_GrowFull01_Begin();
    }
    SECTION("testS21", "[ringbuffer]") {
        trb.testS21_GrowFull02_Begin1();
    }
    SECTION("testS22", "[ringbuffer]") {
        trb.testS22_GrowFull03_Begin2();
    }
    SECTION("testS23", "[ringbuffer]") {
        trb.testS23_GrowFull04_Begin3();
    }
    SECTION("testS24", "[ringbuffer]") {
        trb.testS24_GrowFull05_End();
    }
    SECTION("testS25", "[ringbuffer]") {
        trb.testS25_GrowFull11_End1();
    }
    SECTION("testS26", "[ringbuffer]") {
        trb.testS26_GrowFull12_End2();
    }
    SECTION("testS27", "[ringbuffer]") {
        trb.testS27_GrowFull13_End3();
    }
}


