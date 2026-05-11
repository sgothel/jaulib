/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020-2026 Gothel Software e.K.
 *
 * SPDX-License-Identifier: MIT
 *
 * This Source Code Form is subject to the terms of the MIT License
 * If a copy of the MIT was not distributed with this file,
 * you can obtain one at https://opensource.org/license/mit/.
 */
#include <cassert>
#include <cinttypes>
#include <cstring>

#include <atomic>
#include <memory>

#include <thread>
#include <pthread.h>

#include <jau/test/catch2_ext.hpp>

#include <jau/ordered_atomic.hpp>

using namespace jau;

static int loops = 10;

/**
 * test_mm_sc_drf_02: Testing SC-DRF non-atomic global read and write within a critical C++20 atomic wait operation block.
 * <p>
 * Modified non-atomic memory within the atomic wait operation block,
 * must be visible for all threads according to memory model (MM) Sequentially Consistent (SC) being data-race-free (DRF).
 * <br>
 * See Herb Sutter's 2013-12-23 slides p19, first box "It must be impossible for the assertion to fail – wouldn’t be SC.".
 * </p>
 * See 'test_mm_sc_drf_00' implementing same test using an atomic acquire/release critical block with spin-lock.
 * See 'test_mm_sc_drf_01' implementing same test using mutex-lock and condition wait.
 */
class TestMemModelSCDRF02 {
  private:
    enum Defaults : int {
        array_size = 10
    };
    constexpr int number(const Defaults rhs) noexcept {
        return static_cast<int>(rhs);
    }

    int value1 = 0;
    int array[array_size] = { 0 };
    sc_atomic_int sync_value;

#if __cplusplus > 201703L // C++20

    void reset(int v1, int array_value) {
        int _sync_value = sync_value; // SC-DRF acquire atomic
        (void) _sync_value;
        value1 = v1;
        for(int & i : array) {
            i = array_value;
        }
        sync_value = v1; // SC-DRF release atomic
    }

    void putThreadType01(int _len, int startValue) {
        const int len = std::min(number(array_size), _len);
        {
            int _sync_value = sync_value; // SC-DRF acquire atomic
            (void) _sync_value;
            _sync_value = startValue;
            for(int i=0; i<len; i++) {
                array[i] = _sync_value+i;
            }
            value1 = startValue;
            sync_value = _sync_value; // SC-DRF release atomic
            sync_value.notify_all();
        }
    }

    void getThreadType01(const std::string& msg, int _len, int startValue) {
        const int len = std::min(number(array_size), _len);

        [[maybe_unused]] int old_value = sync_value.wait_for(startValue); // SC-DRF atomic wait for startValue, implies acquire atomic
        int _sync_value = startValue;
        REQUIRE_MSG(msg+": sync_value == startValue", _sync_value == startValue);
        // REQUIRE_MSG(msg+": old sync_value wasn't startValue", old_value != startValue);
        REQUIRE_MSG(msg+": value at read value1 (start)", startValue == value1);

        for(int i=0; i<len; i++) {
            int v = array[i];
            REQUIRE_MSG(msg+": start value at read array #"+std::to_string(i), (startValue+i) == v);
        }
        sync_value = _sync_value; // SC-DRF release atomic (unchanged)
    }

    void putThreadType11(int indexAndValue) {
        const int idx = std::min(number(array_size)-1, indexAndValue);
        {
            // idx is encoded on sync_value (v) as follows
            //   v > 0: get @ idx = v -1
            //   v < 0: put @ idx = abs(v) -1
            // fprintf(stderr, "putThreadType11.wait @ %d (exp %d)\n", idx, (idx+1)*-1);

            // SC-DRF atomic wait for encoded idx, implies acquire atomic
            [[maybe_unused]] int old_value = sync_value.wait_for((idx+1)*-1);

            // fprintf(stderr, "putThreadType11.done @ %d (old %d, has %d, exp %d)\n", idx, old_value, sync_value.load(), (idx+1)*-1);
            int _sync_value = idx;
            value1 = idx;
            array[idx] = idx; // last written checked first, SC-DRF should handle...
            sync_value = _sync_value; // SC-DRF release atomic
            sync_value.notify_all();
        }
    }
    void getThreadType11(const std::string& msg, int _idx) {
        const int idx = std::min(number(array_size)-1, _idx);

        // idx is encoded on sync_value (v) as follows
        //   v > 0: get @ idx = v -1
        //   v < 0: put @ idx = abs(v) -1
        // fprintf(stderr, "getThreadType11.wait for %d\n", idx);

        // SC-DRF atomic wait for idx, implies acquire atomic
        [[maybe_unused]] int old_value = sync_value.wait_for(idx);

        REQUIRE_MSG(msg+": value at read array (a), idx "+std::to_string(idx), idx == array[idx]); // check last-written first
        REQUIRE_MSG(msg+": value at read value1, idx "+std::to_string(idx), idx == value1);
        // next write encoded idx
        int _sync_value = (idx+1)%array_size;
        _sync_value = ( _sync_value + 1 ) * -1;
        // fprintf(stderr, "getThreadType11.done for %d, next %d (v %d, o %d)\n", idx, (idx+1)%array_size, _sync_value, old_value);
        value1 = _sync_value;
        sync_value = _sync_value; // SC-DRF release atomic
        sync_value.notify_all();
    }

#endif // C++20

  public:

    TestMemModelSCDRF02()
    : value1(0), sync_value(0) {}

#if __cplusplus > 201703L // C++20
    void test01_Read1Write1() {
        INFO_STR("\n\ntest01_Read1Write1.a\n");
        reset(0, 1010);

        std::thread getThread01(&TestMemModelSCDRF02::getThreadType01, this, "test01.get01", array_size, 3); // @suppress("Invalid arguments")
        std::thread putThread01(&TestMemModelSCDRF02::putThreadType01, this, array_size, 3); // @suppress("Invalid arguments")
        putThread01.join();
        getThread01.join();
    }

    void test02_Read2Write1() {
        INFO_STR("\n\ntest01_Read2Write1.a\n");
        reset(0, 1021);
        {
            std::thread getThread00(&TestMemModelSCDRF02::getThreadType01, this, "test01.get00", array_size, 4); // @suppress("Invalid arguments")
            std::thread getThread01(&TestMemModelSCDRF02::getThreadType01, this, "test01.get01", array_size, 4); // @suppress("Invalid arguments")
            std::thread putThread01(&TestMemModelSCDRF02::putThreadType01, this, array_size, 4); // @suppress("Invalid arguments")
            putThread01.join();
            getThread00.join();
            getThread01.join();
        }

        INFO_STR("\n\ntest01_Read2Write1.b\n");
        reset(0, 1022);
        {
            std::thread putThread01(&TestMemModelSCDRF02::putThreadType01, this, array_size, 5); // @suppress("Invalid arguments")
            std::thread getThread00(&TestMemModelSCDRF02::getThreadType01, this, "test01.get00", array_size, 5); // @suppress("Invalid arguments")
            std::thread getThread01(&TestMemModelSCDRF02::getThreadType01, this, "test01.get01", array_size, 5); // @suppress("Invalid arguments")
            putThread01.join();
            getThread00.join();
            getThread01.join();
        }
    }

    void test03_Read4Write1() {
        INFO_STR("\n\ntest02_Read4Write1\n");
        reset(0, 1030);

        std::thread getThread01(&TestMemModelSCDRF02::getThreadType01, this, "test02.get01", array_size, 6); // @suppress("Invalid arguments")
        std::thread getThread02(&TestMemModelSCDRF02::getThreadType01, this, "test02.get02", array_size, 6); // @suppress("Invalid arguments")
        std::thread putThread01(&TestMemModelSCDRF02::putThreadType01, this, array_size, 6); // @suppress("Invalid arguments")
        std::thread getThread03(&TestMemModelSCDRF02::getThreadType01, this, "test02.get03", array_size, 6); // @suppress("Invalid arguments")
        std::thread getThread04(&TestMemModelSCDRF02::getThreadType01, this, "test02.get04", array_size, 6); // @suppress("Invalid arguments")
        putThread01.join();
        getThread01.join();
        getThread02.join();
        getThread03.join();
        getThread04.join();
    }

    void test11_Read10Write10() {
        INFO_STR("\n\ntest11_Read10Write10\n");
        reset(-1, 1110); // start put idx 0

        std::thread reader[array_size];
        std::thread writer[array_size];
        for(int i=0; i<number(array_size); i++) {
            reader[i] = std::thread(&TestMemModelSCDRF02::getThreadType11, this, "test11.get11", i); // @suppress("Invalid arguments") // @suppress("Symbol is not resolved")
        }
        for(int i=0; i<number(array_size); i++) {
            writer[i] = std::thread(&TestMemModelSCDRF02::putThreadType11, this, i); // @suppress("Invalid arguments") // @suppress("Symbol is not resolved")
        }
        for(int i=0; i<number(array_size); i++) {
            writer[i].join();
        }
        for(int i=0; i<number(array_size); i++) {
            reader[i].join();
        }
    }

    void test12_Read10Write10() {
        INFO_STR("\n\ntest12_Read10Write10\n");
        reset(-1, 1120); // start put idx 0

        std::thread reader[array_size];
        std::thread writer[array_size];
        for(int i=0; i<number(array_size); i++) {
            writer[i] = std::thread(&TestMemModelSCDRF02::putThreadType11, this, i); // @suppress("Invalid arguments") // @suppress("Symbol is not resolved")
        }
        for(int i=0; i<number(array_size); i++) {
            reader[i] = std::thread(&TestMemModelSCDRF02::getThreadType11, this, "test12.get11", i); // @suppress("Invalid arguments") // @suppress("Symbol is not resolved")
        }
        for(int i=0; i<number(array_size); i++) {
            writer[i].join();
        }
        for(int i=0; i<number(array_size); i++) {
            reader[i].join();
        }
    }

    void test_list() {
        for(int i=loops; i>0; i--) { test01_Read1Write1(); }
        for(int i=loops; i>0; i--) { test02_Read2Write1(); }
        for(int i=loops; i>0; i--) { test03_Read4Write1(); }
        for(int i=loops; i>0; i--) { test11_Read10Write10(); }
        for(int i=loops; i>0; i--) { test12_Read10Write10(); }
    }

#else // C++20

    void test_list() {
        SUCCEED("Skipped C++ >= 20 only test");
    }

#endif // C++20

};

METHOD_AS_TEST_CASE( TestMemModelSCDRF02::test_list, "Test TestMemModelSCDRF 02- test_list");
