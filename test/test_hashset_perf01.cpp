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

#include <jau/test/catch2_ext.hpp>

#include "test_datatype01.hpp"

#include <jau/basic_types.hpp>
#include <jau/basic_algos.hpp>
#include <jau/counting_allocator.hpp>
#include <jau/callocator.hpp>
#include <jau/counting_callocator.hpp>
#include <jau/darray.hpp>
#include <jau/cow_darray.hpp>
#include <jau/cow_vector.hpp>

using namespace jau;

static uint8_t start_addr_b[] = {0x20, 0x26, 0x2A, 0x01, 0x20, 0x10};
static Addr48Bit start_addr(start_addr_b);

// #define USE_STD_ITER_ALGO 1
#define USE_JAU_ITER_ALGO 1

template<class T>
const DataType01 * findDataSet01_hash(T& data, DataType01 const & elem) noexcept {
    auto search = data.find(elem);
    if( search != data.end() ) {
        return &(*search);
    }
    return nullptr;
}

template<class T>
static int test_00_list_itr(T& data) {
    int some_number = 0; // add some validated work, avoiding any 'optimization away'
    jau::for_each_const(data, [&some_number](const DataType01 & e) {
        some_number += e.nop();
    } );
    REQUIRE(some_number > 0);
    return some_number;
}

template<class T, typename Size_type>
static void test_00_seq_find_itr(T& data) {
    Addr48Bit a0(start_addr);
    const Size_type size = data.size();
    Size_type fi = 0, i=0;

    for(; i<size && a0.next(); ++i) {
        DataType01 elem(a0, static_cast<uint8_t>(1));
        const DataType01 *found = jau::find_const<T>(data, elem);
        if( nullptr != found ) {
            ++fi;
            found->nop();
        }
    }
    REQUIRE(fi == i);
}

template<class T>
static void test_00_seq_find_hash(T& data) {
    Addr48Bit a0(start_addr);
    const std::size_t size = data.size();
    std::size_t fi = 0, i=0;

    for(; i<size && a0.next(); ++i) {
        DataType01 elem(a0, static_cast<uint8_t>(1));
        const DataType01 *found = findDataSet01_hash(data, elem);
        if( nullptr != found ) {
            ++fi;
            found->nop();
        }
    }
    REQUIRE(fi == i);
}

template<class T, typename Size_type>
static void test_00_seq_fill(T& data, const Size_type size) {
    Addr48Bit a0(start_addr);
    Size_type i=0;

    for(; i<size && a0.next(); ++i) {
        data.emplace_back( a0, static_cast<uint8_t>(1) );
    }
    REQUIRE(i == data.size());
}

template<class T, typename Size_type>
static void test_00_seq_fill_unique_itr(T& data, const Size_type size) {
    Addr48Bit a0(start_addr);
    Size_type i=0, fi=0;

    for(; i<size && a0.next(); ++i) {
        DataType01 elem(a0, static_cast<uint8_t>(1));
        const DataType01* exist = jau::find_const<T>(data, elem);
        if( nullptr == exist ) {
            data.push_back( std::move( elem ) );
            ++fi;
        }
    }
    REQUIRE(i == data.size());
    REQUIRE(fi == size);
}

template<class T>
static void test_00_seq_fill_unique_hash(T& data, const std::size_t size) {
    Addr48Bit a0(start_addr);
    std::size_t i=0, fi=0;

    for(; i<size && a0.next(); ++i) {
        if( data.emplace(a0, static_cast<uint8_t>(1)).second ) {
            ++fi;
        }
    }
    REQUIRE(i == data.size());
    REQUIRE(fi == size);
}

template<class T>
static void print_mem(const std::string& pre, const T& data) {
    std::size_t bytes_element = sizeof(DataType01);
    std::size_t elements = data.size();
    std::size_t bytes_net = elements * bytes_element;
    std::size_t bytes_total = data.get_allocator().memory_usage;
    double overhead = 0 == bytes_total ? 0.0 : ( 0 == bytes_net ? 10.0 : (double)bytes_total / (double)bytes_net );
    printf("Mem: %s: Elements %s x %zu bytes; %s, %lf ratio\n",
            pre.c_str(), to_decstring(elements, ',', 5).c_str(),
            bytes_element, data.get_allocator().toString(10, 5).c_str(), overhead);
    // 5:     1,000
    // 7:   100,000
    // 9: 1,000,000
}


/****************************************************************************************
 ****************************************************************************************/

template<class T, typename Size_type>
static bool test_01_seq_fill_list_itr(const std::string& type_id, const Size_type size0, const Size_type reserve0, const bool do_print_mem) {
    T data;
    REQUIRE(0 == data.get_allocator().memory_usage);
    REQUIRE(data.size() == 0);
    // if( do_print_mem ) { print_mem(type_id+" 01 (empty)", data); }

    if( 0 < reserve0 ) {
        data.reserve(reserve0);
        REQUIRE(data.size() == 0);
        REQUIRE(0 != data.get_allocator().memory_usage);
        REQUIRE(data.capacity() == reserve0);
    }

    test_00_seq_fill<T, Size_type>(data, size0);
    REQUIRE(0 != data.get_allocator().memory_usage);
    REQUIRE(data.size() == size0);

    test_00_list_itr<T>(data);
    REQUIRE(0 != data.get_allocator().memory_usage);
    REQUIRE(data.size() == size0);
    if( do_print_mem ) { print_mem(type_id+" 01 (full_)", data); }

    data.clear();
    REQUIRE(data.size() == 0);
    // if( do_print_mem ) { print_mem(type_id+" 01 (clear)", data); }
    // REQUIRE(0 == data.get_allocator().memory_usage);
    return data.size() == 0;
}

template<class T>
static std::size_t get_capacity(const T& data) {
    const std::size_t bucket_count = data.bucket_count();
    std::size_t capacity = 0;
    for(std::size_t i=0; i<bucket_count; i++) {
        capacity = data.bucket_size(i);
    }
    return capacity;
}

static bool test_01_seq_fill_list_hash(const std::string& type_id, const std::size_t size0, const std::size_t reserve0, const bool do_print_mem) {
    typedef std::unordered_set<DataType01, std::hash<DataType01>, std::equal_to<DataType01>, counting_allocator<DataType01>> DataType01Set;
    DataType01Set data;
    REQUIRE(0 == data.get_allocator().memory_usage);
    REQUIRE(data.size() == 0);
    // if( do_print_mem ) { print_mem(type_id+" 01 (empty)", data); }

    if( 0 < reserve0 ) {
        data.reserve(reserve0);
        REQUIRE(data.size() == 0);
        REQUIRE(0 != data.get_allocator().memory_usage);
        REQUIRE(get_capacity<DataType01Set>(data) >= reserve0);
    }

    test_00_seq_fill_unique_hash(data, size0);
    REQUIRE(0 != data.get_allocator().memory_usage);
    REQUIRE(data.size() == size0);

    test_00_list_itr<DataType01Set>(data);
    REQUIRE(0 != data.get_allocator().memory_usage);
    REQUIRE(data.size() == size0);
    if( do_print_mem ) { print_mem(type_id+" 01 (full_)", data); }

    data.clear();
    REQUIRE(data.size() == 0);
    // if( do_print_mem ) { print_mem(type_id+" 01 (clear)", data); }
    // REQUIRE(0 == data.get_allocator().memory_usage);
    return data.size() == 0;
}

template<class T, typename Size_type>
static bool test_02_seq_fillunique_find_itr(const std::string& type_id, const Size_type size0, const Size_type reserve0) {
    (void)type_id;
    T data;
    REQUIRE(data.size() == 0);

    if( 0 < reserve0 ) {
        data.reserve(reserve0);
        REQUIRE(data.size() == 0);
        REQUIRE(data.capacity() == reserve0);
    }

    test_00_seq_fill_unique_itr<T, Size_type>(data, size0);
    REQUIRE(data.size() == size0);

    test_00_seq_find_itr<T, Size_type>(data);
    REQUIRE(data.size() == size0);

    data.clear();
    REQUIRE(data.size() == 0);
    return data.size() == 0;
}

static bool test_02_seq_fillunique_find_hash(const std::string& type_id, const std::size_t size0, const std::size_t reserve0) {
    (void)type_id;
    typedef std::unordered_set<DataType01, std::hash<DataType01>, std::equal_to<DataType01>, std::allocator<DataType01>> DataType01Set;
    DataType01Set data;
    REQUIRE(data.size() == 0);

    if( 0 < reserve0 ) {
        data.reserve(reserve0);
        REQUIRE(data.size() == 0);
        // REQUIRE(0 != data.get_allocator().memory_usage);
        // REQUIRE(get_capacity<DataType01Set>(data) >= reserve0);
    }

    test_00_seq_fill_unique_hash(data, size0);
    REQUIRE(data.size() == size0);

    test_00_seq_find_hash(data);
    REQUIRE(data.size() == size0);

    data.clear();
    REQUIRE(data.size() == 0);
    return data.size() == 0;
}

/****************************************************************************************
 ****************************************************************************************/

template<class T, typename Size_type>
static bool footprint_fillseq_list_itr(const std::string& type_id, const bool do_rserv) {
    // test_01_seq_fill_list_itr<T, Size_type>(type_id, 25, do_rserv? 25 : 0, true);
    test_01_seq_fill_list_itr<T, Size_type>(type_id, 50, do_rserv? 50 : 0, true);
    if( !catch_auto_run ) {
        test_01_seq_fill_list_itr<T, Size_type>(type_id, 100, do_rserv? 100 : 0, true);
        test_01_seq_fill_list_itr<T, Size_type>(type_id, 1000, do_rserv? 1000 : 0, true);
    }
    return true;
}

static bool footprint_fillseq_list_hash(const std::string& type_id, const bool do_rserv) {
    // test_01_seq_fill_list_hash(type_id, 25, do_rserv? 25 : 0, true);
    test_01_seq_fill_list_hash(type_id, 50, do_rserv? 50 : 0, true);
    if( !catch_auto_run ) {
        test_01_seq_fill_list_hash(type_id, 100, do_rserv? 100 : 0, true);
        test_01_seq_fill_list_hash(type_id, 1000, do_rserv? 1000 : 0, true);
    }
    return true;
}

template<class T, typename Size_type>
static bool benchmark_fillunique_find_itr(const std::string& title_pre, const std::string& type_id,
                                          const bool do_rserv) {
    if( catch_perf_analysis ) {
        BENCHMARK(title_pre+" FillUni_List 1000") {
            return test_02_seq_fillunique_find_itr<T, Size_type>(type_id, 1000, do_rserv? 1000 : 0);
        };
        // test_02_seq_fillunique_find_itr<T, Size_type>(type_id, 100000, do_rserv? 100000 : 0);
        return true;
    }
    if( catch_auto_run ) {
        test_02_seq_fillunique_find_itr<T, Size_type>(type_id, 50, do_rserv? 50 : 0);
        return true;
    }
    // BENCHMARK(title_pre+" FillUni_List 25") {
    //    return test_02_seq_fillunique_find_itr<T, Size_type>(type_id, 25, do_rserv? 25 : 0);
    // };
    BENCHMARK(title_pre+" FillUni_List 50") {
        return test_02_seq_fillunique_find_itr<T, Size_type>(type_id, 50, do_rserv? 50 : 0);
    };
    BENCHMARK(title_pre+" FillUni_List 100") {
        return test_02_seq_fillunique_find_itr<T, Size_type>(type_id, 100, do_rserv? 100 : 0);
    };
    BENCHMARK(title_pre+" FillUni_List 1000") {
        return test_02_seq_fillunique_find_itr<T, Size_type>(type_id, 1000, do_rserv? 1000 : 0);
    };
    return true;
}

static bool benchmark_fillunique_find_hash(const std::string& title_pre, const std::string& type_id,
                                          const bool do_rserv) {
    if( catch_perf_analysis ) {
        BENCHMARK(title_pre+" FillUni_List 1000") {
            return test_02_seq_fillunique_find_hash(type_id, 1000, do_rserv? 1000 : 0);
        };
        // test_02_seq_fillunique_find_hash(type_id, 100000, do_rserv? 100000 : 0, false);
        return true;
    }
    if( catch_auto_run ) {
        test_02_seq_fillunique_find_hash(type_id, 50, do_rserv? 50 : 0);
        return true;
    }
    // BENCHMARK(title_pre+" FillUni_List 25") {
    //    return test_02_seq_fillunique_find_hash(type_id, 25, do_rserv? 25 : 0);
    // };
    BENCHMARK(title_pre+" FillUni_List 50") {
        return test_02_seq_fillunique_find_hash(type_id, 50, do_rserv? 50 : 0);
    };
    BENCHMARK(title_pre+" FillUni_List 100") {
        return test_02_seq_fillunique_find_hash(type_id, 100, do_rserv? 100 : 0);
    };
    BENCHMARK(title_pre+" FillUni_List 1000") {
        return test_02_seq_fillunique_find_hash(type_id, 1000, do_rserv? 1000 : 0);
    };
    return true;
}

/****************************************************************************************
 ****************************************************************************************/
TEST_CASE( "Memory Footprint 01 - Fill Sequential and List", "[datatype][footprint]" ) {
    if( catch_perf_analysis ) {
        footprint_fillseq_list_hash("hash__set_empty_", false);
        footprint_fillseq_list_itr< jau::cow_vector<DataType01, counting_allocator<DataType01>>,                std::size_t>("cowstdvec_empty_", false);
        footprint_fillseq_list_itr< jau::cow_darray<DataType01, counting_callocator<DataType01>, jau::nsize_t>, jau::nsize_t>("cowdarray_empty_", false);
        return;
    }
    footprint_fillseq_list_hash("hash__set_empty_", false);
    footprint_fillseq_list_itr< std::vector<DataType01, counting_allocator<DataType01>>,                std::size_t>("stdvec_empty_", false);
    footprint_fillseq_list_itr< jau::darray<DataType01, counting_callocator<DataType01>, jau::nsize_t>, jau::nsize_t>("darray_empty_", false);
    footprint_fillseq_list_itr< jau::cow_vector<DataType01, counting_allocator<DataType01>>,                std::size_t>("cowstdvec_empty_", false);
    footprint_fillseq_list_itr< jau::cow_darray<DataType01, counting_callocator<DataType01>, jau::nsize_t>, jau::nsize_t>("cowdarray_empty_", false);
}

TEST_CASE( "Perf Test 02 - Fill Unique and List, empty and reserve", "[datatype][unique]" ) {
    if( catch_perf_analysis ) {
        benchmark_fillunique_find_hash("HashSet_NoOrdr_empty", "hash__set_empty_", false);
        benchmark_fillunique_find_itr< jau::cow_vector<DataType01, std::allocator<DataType01>>,                std::size_t>("COW_Vector_empty_itr", "cowstdvec_empty_", false);
        benchmark_fillunique_find_itr< jau::cow_darray<DataType01, jau::callocator<DataType01>, jau::nsize_t>, jau::nsize_t>("COW_DArray_empty_itr", "cowdarray_empty_", false);

        return;
    }
    benchmark_fillunique_find_hash("HashSet_NoOrdr_empty", "hash__set_empty_", false);
    benchmark_fillunique_find_itr< std::vector<DataType01, std::allocator<DataType01>>,                std::size_t>("STD_Vector_empty_itr", "stdvec_empty_", false);
    benchmark_fillunique_find_itr< jau::darray<DataType01, jau::callocator<DataType01>, jau::nsize_t>, jau::nsize_t>("JAU_DArray_empty_itr", "darray_empty_", false);
    benchmark_fillunique_find_itr< jau::cow_vector<DataType01, std::allocator<DataType01>>,                std::size_t>("COW_Vector_empty_itr", "cowstdvec_empty_", false);
    benchmark_fillunique_find_itr< jau::cow_darray<DataType01, jau::callocator<DataType01>, jau::nsize_t>, jau::nsize_t>("COW_DArray_empty_itr", "cowdarray_empty_", false);

    benchmark_fillunique_find_hash("HashSet_NoOrdr_rserv", "hash__set_empty_", true);
    benchmark_fillunique_find_itr< std::vector<DataType01, std::allocator<DataType01>>,                    std::size_t>("STD_Vector_rserv_itr", "stdvec_rserv", true);
    benchmark_fillunique_find_itr< jau::darray<DataType01, jau::callocator<DataType01>, jau::nsize_t>,     jau::nsize_t>("JAU_DArray_rserv_itr", "darray_rserv", true);
    benchmark_fillunique_find_itr< jau::cow_vector<DataType01, std::allocator<DataType01>>,                std::size_t>("COW_Vector_rserv_itr", "cowstdvec_rserv", true);
    benchmark_fillunique_find_itr< jau::cow_darray<DataType01, jau::callocator<DataType01>, jau::nsize_t>, jau::nsize_t>("COW_DArray_rserv_itr", "cowdarray_rserv", true);

}
