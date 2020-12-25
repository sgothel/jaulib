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
#include <random>
#include <vector>
#include <unordered_set>

#define CATCH_CONFIG_RUNNER
// #define CATCH_CONFIG_MAIN
#include <catch2/catch_amalgamated.hpp>
#include <jau/test/catch2_ext.hpp>

#include "test_datatype01.hpp"

#include <jau/basic_types.hpp>
#include <jau/counting_allocator.hpp>

using namespace jau;

static uint8_t start_addr_b[] = {0x20, 0x26, 0x2A, 0x01, 0x20, 0x10};
static Addr48Bit start_addr(start_addr_b);

typedef std::vector<DataType01, counting_allocator<DataType01>> DataType01Vector;
typedef std::unordered_set<DataType01, std::hash<DataType01>, std::equal_to<DataType01>, counting_allocator<DataType01>> DataType01Set;

DataType01 * findDataSet01(DataType01Vector& data, DataType01 const & elem) noexcept {
    const size_t size = data.size();
    for (size_t i = 0; i < size; i++) {
        DataType01 & e = data[i];
        if ( elem == e ) {
            return &e;
        }
    }
    return nullptr;
}
const DataType01 * findDataSet01(DataType01Set& data, DataType01 const & elem) noexcept {
    auto search = data.find(elem);
    if( search != data.end() ) {
        return &(*search);
    }
    return nullptr;
}

static void test_stdvec_00_list(DataType01Vector& data, const bool show) {
    const std::size_t size = data.size();
    for (std::size_t i = 0; i < size; i++) {
        DataType01 & e = data[i];
        e.nop();
        if( show ) {
            printf("data[%zu]: %s\n", i, e.toString().c_str());
        }
    }
}
static void test_stdset_00_list(DataType01Set& data, const bool show) {
    std::size_t i=0;
    for(auto it = data.begin(); it != data.end(); it++, i++) {
        const DataType01 & e = *it;
        e.nop();
        if( show ) {
            printf("data[%zu]: %s\n", i, e.toString().c_str());
        }
    }
}

static void test_stdvec_00_seq_find_each(DataType01Vector& data, const bool show) {
    Addr48Bit a0(start_addr);
    const std::size_t size = data.size();
    std::size_t fi = 0, i=0;

    for(; i<size && a0.next(); i++) {
        DataType01 elem(a0, static_cast<uint8_t>(1));
        DataType01 *found = findDataSet01(data, elem);
        if( nullptr != found ) {
            fi++;
            found->nop();
            if( show ) {
                printf("data[%zu, %zu]: %s\n", i, fi, found->toString().c_str());
            }
        }
    }
    REQUIRE(fi == i);
}
static void test_stdset_00_seq_find_each(DataType01Set& data, const bool show) {
    Addr48Bit a0(start_addr);
    const std::size_t size = data.size();
    std::size_t fi = 0, i=0;

    for(; i<size && a0.next(); i++) {
        DataType01 elem(a0, static_cast<uint8_t>(1));
        const DataType01 *found = findDataSet01(data, elem);
        if( nullptr != found ) {
            fi++;
            found->nop();
            if( show ) {
                printf("data[%zu, %zu]: %s\n", i, fi, found->toString().c_str());
            }
        }
    }
    REQUIRE(fi == i);
}

static void test_stdvec_00_seq_fill(DataType01Vector& data, const std::size_t size) {
    // data.reserve(size);
    Addr48Bit a0(start_addr);
    std::size_t i=0;

    for(; i<size && a0.next(); i++) {
        DataType01 elem(a0, static_cast<uint8_t>(1));
        data.push_back( elem );
    }
    if( i != data.size() ) {
        test_stdvec_00_list(data, true);
        printf("a0 %s\n", a0.toString().c_str());
        printf("Size %zu, expected %zu, iter %zu\n", data.size(), size, i);
    }
    REQUIRE(i == data.size());
}

static void test_stdvec_00_seq_fill_unique(DataType01Vector& data, const std::size_t size) {
    // data.reserve(size);
    Addr48Bit a0(start_addr);
    std::size_t i=0, fi=0;

    for(; i<size && a0.next(); i++) {
        DataType01 elem(a0, static_cast<uint8_t>(1));
        DataType01* exist = findDataSet01(data, elem);
        if( nullptr == exist ) {
            data.push_back(elem);
            fi++;
        } else {
            printf("Not unique #%zu: %s == %s (%d)\n", i, elem.toString().c_str(), exist->toString().c_str(), (elem == *exist));
        }
    }
    if( fi != size ) {
        test_stdvec_00_list(data, true);
        printf("a0 %s\n", a0.toString().c_str());
        printf("Size %zu, expected %zu, iter %zu\n", data.size(), size, i);
    }
    REQUIRE(i == data.size());
    REQUIRE(fi == size);
}
static void test_stdset_00_seq_fill_unique(DataType01Set& data, const std::size_t size) {
    Addr48Bit a0(start_addr);
    std::size_t i=0, fi=0;

    for(; i<size && a0.next(); i++) {
        DataType01 elem(a0, static_cast<uint8_t>(1));
        if( data.insert(elem).second ) {
            fi++;
        }
    }
    if( fi != size ) {
        test_stdset_00_list(data, true);
        printf("a0 %s\n", a0.toString().c_str());
        printf("Size %zu, expected %zu, iter %zu\n", data.size(), size, i);
    }
    REQUIRE(i == data.size());
    REQUIRE(fi == size);
}

static void print_mem(const std::string& pre, const DataType01Vector &data) {
    std::size_t bytes_element = sizeof(DataType01);
    std::size_t elements = data.size();
    std::size_t bytes_net = elements * bytes_element;
    std::size_t bytes_total = data.get_allocator().memory_usage;
    double overhead = 0 == bytes_total ? 0.0 : ( 0 == bytes_net ? 10.0 : (double)bytes_total / (double)bytes_net );
    printf("Mem: %s: Elements %s x %zu bytes; %s, %lf ratio\n",
            pre.c_str(), int64DecString(elements, ',', 5).c_str(),
            bytes_element, data.get_allocator().toString(9, 5).c_str(), overhead);
    // 5:     1,000
    // 7:   100,000
    // 9: 1,000,000
}
static void print_mem(const std::string& pre, const DataType01Set &data) {
    std::size_t bytes_element = sizeof(DataType01);
    std::size_t elements = data.size();
    std::size_t bytes_net = elements * bytes_element;
    std::size_t bytes_total = data.get_allocator().memory_usage;
    double overhead = 0 == bytes_total ? 0.0 : ( 0 == bytes_net ? 10.0 : (double)bytes_total / (double)bytes_net );
    printf("Mem: %s: Elements %s x %zu bytes; %s, %lf ratio\n",
            pre.c_str(), int64DecString(elements, ',', 5).c_str(),
            bytes_element, data.get_allocator().toString(9, 5).c_str(), overhead);
}

static bool test_stdvec_01_seq_fill_list_clear(const std::size_t size0, const bool do_print_mem) {
    std::vector<DataType01, counting_allocator<DataType01>> data;
    REQUIRE(0 == data.get_allocator().memory_usage);
    REQUIRE(data.size() == 0);
    // if( do_print_mem ) { print_mem("stdvec_01 (empty)", data); }

    test_stdvec_00_seq_fill(data, size0);
    REQUIRE(data.size() == size0);

    test_stdvec_00_list(data, false);
    REQUIRE(data.size() == size0);
    if( do_print_mem ) { print_mem("stdvec_01 (full_)", data); }

    data.clear();
    REQUIRE(data.size() == 0);
    // if( do_print_mem ) { print_mem("stdvec_01 (clear)", data); }
    // REQUIRE(0 == data.get_allocator().memory_usage);
    return data.size() == 0;
}

static bool test_stdvec_02_seq_fillunique_findeach_clear(const std::size_t size0, const bool do_print_mem) {
    std::vector<DataType01, counting_allocator<DataType01>> data;
    REQUIRE(0 == data.get_allocator().memory_usage);
    REQUIRE(data.size() == 0);
    // if( do_print_mem ) { print_mem("stdvec_02 (empty)", data); }

    test_stdvec_00_seq_fill_unique(data, size0);
    REQUIRE(data.size() == size0);

    test_stdvec_00_seq_find_each(data, false);
    REQUIRE(data.size() == size0);
    if( do_print_mem ) { print_mem("stdvec_02 (full_)", data); }

    data.clear();
    REQUIRE(data.size() == 0);
    // if( do_print_mem ) { print_mem("stdvec_02 (clear)", data); }
    // REQUIRE(0 == data.get_allocator().memory_usage);
    return data.size() == 0;
}

static bool test_stdset_12_seq_fillunique_findeach_clear(const std::size_t size0, const bool do_print_mem) {
    std::unordered_set<DataType01, std::hash<DataType01>, std::equal_to<DataType01>, counting_allocator<DataType01>> data;
    REQUIRE(0 == data.get_allocator().memory_usage);
    REQUIRE(data.size() == 0);
    // if( do_print_mem ) { print_mem("stdset_12 (empty)", data); }

    test_stdset_00_seq_fill_unique(data, size0);
    REQUIRE(data.size() == size0);

    test_stdset_00_seq_find_each(data, false);
    REQUIRE(data.size() == size0);
    if( do_print_mem ) { print_mem("stdset_12 (full_)", data); }

    data.clear();
    REQUIRE(data.size() == 0);
    // if( do_print_mem ) { print_mem("stdset_12 (clear)", data); }
    // REQUIRE(0 == data.get_allocator().memory_usage);
    return data.size() == 0;
}

TEST_CASE( "STD Vector Perf Test 01 - Fill Sequential and List", "[datatype][std][vector]" ) {
    test_stdvec_01_seq_fill_list_clear(25, true);
    test_stdvec_01_seq_fill_list_clear(50, true);
    if( !catch_auto_run ) {
        test_stdvec_01_seq_fill_list_clear(100, true);
        test_stdvec_01_seq_fill_list_clear(200, true);
        test_stdvec_01_seq_fill_list_clear(1000, true);
    }

    BENCHMARK("Seq_List 25") {
        return test_stdvec_01_seq_fill_list_clear(25, false);
    };
    BENCHMARK("Seq_List 50") {
        return test_stdvec_01_seq_fill_list_clear(50, false);
    };
    if( !catch_auto_run ) {
        BENCHMARK("Seq_List 100") {
            return test_stdvec_01_seq_fill_list_clear(100, false);
        };
        BENCHMARK("Seq_List 200") {
            return test_stdvec_01_seq_fill_list_clear(200, false);
        };
        BENCHMARK("Seq_List 1000") {
            return test_stdvec_01_seq_fill_list_clear(1000, false);
        };
    }
}

TEST_CASE( "STD Vector Perf Test 02 - Fill Unique and Find-Each", "[datatype][std][vector]" ) {
    test_stdvec_02_seq_fillunique_findeach_clear(25, true);
    test_stdvec_02_seq_fillunique_findeach_clear(50, true);
    if( !catch_auto_run ) {
        test_stdvec_02_seq_fillunique_findeach_clear(100, true);
        test_stdvec_02_seq_fillunique_findeach_clear(200, true);
        test_stdvec_02_seq_fillunique_findeach_clear(1000, true);
    }

    BENCHMARK("Unique Find 25") {
        return test_stdvec_02_seq_fillunique_findeach_clear(25, false);
    };
    BENCHMARK("Unique Find 50") {
        return test_stdvec_02_seq_fillunique_findeach_clear(50, false);
    };
    if( !catch_auto_run ) {
        BENCHMARK("Unique Find 100") {
            return test_stdvec_02_seq_fillunique_findeach_clear(100, false);
        };
        BENCHMARK("Unique Find 200") {
            return test_stdvec_02_seq_fillunique_findeach_clear(200, false);
        };
        BENCHMARK("Unique Find 1000") {
            return test_stdvec_02_seq_fillunique_findeach_clear(1000, false);
        };
    }
}
TEST_CASE( "STD Unordered-Set Perf Test 12 - Fill Unique and Find-Each", "[datatype][std][unordered_set]" ) {
    test_stdset_12_seq_fillunique_findeach_clear(25, true);
    test_stdset_12_seq_fillunique_findeach_clear(50, true);
    if( !catch_auto_run ) {
        test_stdset_12_seq_fillunique_findeach_clear(100, true);
        test_stdset_12_seq_fillunique_findeach_clear(200, true);
        test_stdset_12_seq_fillunique_findeach_clear(1000, true);
    }

    BENCHMARK("Unique Find 25") {
        return test_stdset_12_seq_fillunique_findeach_clear(25, false);
    };
    BENCHMARK("Unique Find 50") {
        return test_stdset_12_seq_fillunique_findeach_clear(50, false);
    };
    if( !catch_auto_run ) {
        BENCHMARK("Unique Find 100") {
            return test_stdset_12_seq_fillunique_findeach_clear(100, false);
        };
        BENCHMARK("Unique Find 200") {
            return test_stdset_12_seq_fillunique_findeach_clear(200, false);
        };
        BENCHMARK("Unique Find 1000") {
            return test_stdset_12_seq_fillunique_findeach_clear(1000, false);
        };
    }
}
