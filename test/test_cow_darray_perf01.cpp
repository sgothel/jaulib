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

#define CATCH_CONFIG_RUNNER
// #define CATCH_CONFIG_MAIN
#include <catch2/catch_amalgamated.hpp>
#include <jau/test/catch2_ext.hpp>

#include "test_datatype01.hpp"

#include <jau/basic_types.hpp>
#include <jau/basic_algos.hpp>
#include <jau/darray.hpp>
#include <jau/cow_darray.hpp>
#include <jau/cow_vector.hpp>
#include <jau/counting_allocator.hpp>

/**
 * Performance test of jau::darray, jau::cow_darray and jau::cow_vector.
 */
using namespace jau;

static uint8_t start_addr_b[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static Addr48Bit start_addr(start_addr_b);

// #define USE_STD_ITER_ALGO 1
#define USE_JAU_ITER_ALGO 1

/****************************************************************************************
 ****************************************************************************************/

template<class T, typename Size_type>
DataType01 * findDataSet01_idx(T& data, DataType01 const & elem) noexcept {
    const Size_type size = data.size();
    for (Size_type i = 0; i < size; i++) {
        DataType01 & e = data[i];
        if ( elem == e ) {
            return &e;
        }
    }
    return nullptr;
}

template<class T>
const DataType01 * findDataSet01_itr(T& data, DataType01 const & elem) noexcept {
#if defined(USE_STD_ITER_ALGO)
    // much slower, approx 3x over 1000 * 1000, why?
    typename T::const_iterator end = data.cend();
    auto it = std::find( data.cbegin(), end, elem);
    if( it != end ) {
        return &(*it);
    }
#elif defined (USE_JAU_ITER_ALGO)
    // same logic, much faster
    typename T::const_iterator end = data.cend();
    auto it = jau::find( data.cbegin(), end, elem);
    if( it != end ) {
        return &(*it);
    }
#else
    typename T::const_iterator iter = data.cbegin();
    typename T::const_iterator end = data.cend();
    for(; iter != end ; ++iter) {
        if( elem == *iter ) {
            return &(*iter);
        }
    }
#endif
    return nullptr;
}

template<class T, typename Size_type>
static void test_00_list_idx(T& data) {
    const Size_type size = data.size();
    for (Size_type i = 0; i < size; i++) {
        const DataType01 & e = data[i];
        e.nop();
    }
}

template<class T>
static void test_00_list_itr(T& data) {
#if defined(USE_STD_ITER_ALGO)
    // slower, why?
    std::for_each(data.cbegin(), data.cend(), [](const DataType01 & e) { e.nop(); });
#elif defined (USE_JAU_ITER_ALGO)
    // same logic, faster
    jau::for_each(data.cbegin(), data.cend(), [](const DataType01 & e) { e.nop(); });
#else
    typename T::const_iterator iter = data.cbegin();
    typename T::const_iterator end = data.cend();
    for(; iter != end ; ++iter) {
        const DataType01 & e = *iter;
        e.nop();
    }
#endif
}

template<class T, typename Size_type>
static void test_00_seq_find_idx(T& data) {
    Addr48Bit a0(start_addr);
    const Size_type size = data.size();
    Size_type fi = 0, i=0;

    for(; i<size && a0.next(); i++) {
        DataType01 elem(a0, static_cast<uint8_t>(1));
        DataType01 *found = findDataSet01_idx<T, Size_type>(data, elem);
        if( nullptr != found ) {
            fi++;
            found->nop();
        }
    }
    REQUIRE(fi == i);
}

template<class T, typename Size_type>
static void test_00_seq_find_itr(T& data) {
    Addr48Bit a0(start_addr);
    const Size_type size = data.size();
    Size_type fi = 0, i=0;

    for(; i<size && a0.next(); i++) {
        DataType01 elem(a0, static_cast<uint8_t>(1));
        const DataType01 *found = findDataSet01_itr<T>(data, elem);
        if( nullptr != found ) {
            fi++;
            found->nop();
        }
    }
    REQUIRE(fi == i);
}

template<class T, typename Size_type>
static void test_00_seq_fill(T& data, const Size_type size) {
    Addr48Bit a0(start_addr);
    Size_type i=0;

    for(; i<size && a0.next(); i++) {
        data.emplace_back( a0, static_cast<uint8_t>(1) );
    }
    REQUIRE(i == data.size());
}

template<class T, typename Size_type>
static void test_00_seq_fill_unique_idx(T& data, const Size_type size) {
    Addr48Bit a0(start_addr);
    Size_type i=0, fi=0;

    for(; i<size && a0.next(); i++) {
        DataType01 elem(a0, static_cast<uint8_t>(1));
        DataType01* exist = findDataSet01_idx<T, Size_type>(data, elem);
        if( nullptr == exist ) {
            data.push_back( std::move( elem ) );
            fi++;
        }
    }
    REQUIRE(i == data.size());
    REQUIRE(fi == size);
}
template<class T, typename Size_type>
static void test_00_seq_fill_unique_itr(T& data, const Size_type size) {
    Addr48Bit a0(start_addr);
    Size_type i=0, fi=0;

    for(; i<size && a0.next(); i++) {
        DataType01 elem(a0, static_cast<uint8_t>(1));
        const DataType01* exist = findDataSet01_itr<T>(data, elem);
        if( nullptr == exist ) {
            data.push_back( std::move( elem ) );
            fi++;
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
            pre.c_str(), int64DecString(elements, ',', 5).c_str(),
            bytes_element, data.get_allocator().toString(10, 5).c_str(), overhead);
    // 5:     1,000
    // 7:   100,000
    // 9: 1,000,000
}

/****************************************************************************************
 ****************************************************************************************/

template<class T, typename Size_type>
static bool test_01_seq_fill_list_idx(const std::string& type_id, const Size_type size0, const Size_type reserve0, const bool do_print_mem) {
    T data;
    REQUIRE(0 == data.get_allocator().memory_usage);
    REQUIRE(data.size() == 0);
    // if( do_print_mem ) { print_mem(type_id+" 01 (empty)", data); }

    if( 0 < reserve0 ) {
        data.reserve(size0);
        REQUIRE(data.size() == 0);
        REQUIRE(0 != data.get_allocator().memory_usage);
        REQUIRE(data.capacity() == size0);
    }

    test_00_seq_fill<T, Size_type>(data, size0);
    REQUIRE(0 != data.get_allocator().memory_usage);
    REQUIRE(data.size() == size0);

    test_00_list_idx<T, Size_type>(data);
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
static bool test_01_seq_fill_list_itr(const std::string& type_id, const Size_type size0, const Size_type reserve0, const bool do_print_mem) {
    T data;
    REQUIRE(0 == data.get_allocator().memory_usage);
    REQUIRE(data.size() == 0);
    // if( do_print_mem ) { print_mem(type_id+" 01 (empty)", data); }

    if( 0 < reserve0 ) {
        data.reserve(size0);
        REQUIRE(data.size() == 0);
        REQUIRE(0 != data.get_allocator().memory_usage);
        REQUIRE(data.capacity() == size0);
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

template<class T, typename Size_type>
static bool test_02_seq_fillunique_find_idx(const std::string& type_id, const Size_type size0, const Size_type reserve0, const bool do_print_mem) {
    T data;
    REQUIRE(0 == data.get_allocator().memory_usage);
    REQUIRE(data.size() == 0);
    // if( do_print_mem ) { print_mem(type_id+" 02 (empty)", data); }

    if( 0 < reserve0 ) {
        data.reserve(reserve0);
        REQUIRE(data.size() == 0);
        REQUIRE(0 != data.get_allocator().memory_usage);
        REQUIRE(data.capacity() == reserve0);
    }

    test_00_seq_fill_unique_idx<T, Size_type>(data, size0);
    REQUIRE(0 != data.get_allocator().memory_usage);
    REQUIRE(data.size() == size0);

    test_00_seq_find_idx<T, Size_type>(data);
    REQUIRE(0 != data.get_allocator().memory_usage);
    REQUIRE(data.size() == size0);
    if( do_print_mem ) { print_mem(type_id+" 02 (full_)", data); }

    data.clear();
    REQUIRE(data.size() == 0);
    // if( do_print_mem ) { print_mem(type_id+" 02 (clear)", data); }
    // REQUIRE(0 == data.get_allocator().memory_usage);
    return data.size() == 0;
}
template<class T, typename Size_type>
static bool test_02_seq_fillunique_find_itr(const std::string& type_id, const Size_type size0, const Size_type reserve0, const bool do_print_mem) {
    T data;
    REQUIRE(0 == data.get_allocator().memory_usage);
    REQUIRE(data.size() == 0);
    // if( do_print_mem ) { print_mem(type_id+" 02 (empty)", data); }

    if( 0 < reserve0 ) {
        data.reserve(reserve0);
        REQUIRE(data.size() == 0);
        REQUIRE(0 != data.get_allocator().memory_usage);
        REQUIRE(data.capacity() == reserve0);
    }

    test_00_seq_fill_unique_itr<T, Size_type>(data, size0);
    REQUIRE(0 != data.get_allocator().memory_usage);
    REQUIRE(data.size() == size0);

    test_00_seq_find_itr<T, Size_type>(data);
    REQUIRE(0 != data.get_allocator().memory_usage);
    REQUIRE(data.size() == size0);
    if( do_print_mem ) { print_mem(type_id+" 02 (full_)", data); }

    data.clear();
    REQUIRE(data.size() == 0);
    // if( do_print_mem ) { print_mem(type_id+" 02 (clear)", data); }
    // REQUIRE(0 == data.get_allocator().memory_usage);
    return data.size() == 0;
}

template<class T, typename Size_type>
static bool test_02b_seq_fillunique_find_itr_rserv(const std::string& type_id, const Size_type size0, const bool do_print_mem) {
    T data;
    REQUIRE(0 == data.get_allocator().memory_usage);
    REQUIRE(data.size() == 0);
    // if( do_print_mem ) { print_mem(type_id+" 02 (empty)", data); }

    data.reserve(size0);
    REQUIRE(data.size() == 0);
    REQUIRE(0 != data.get_allocator().memory_usage);
    REQUIRE(data.capacity() == size0);

    test_00_seq_fill_unique_itr<T, Size_type>(data, size0);
    REQUIRE(0 != data.get_allocator().memory_usage);
    REQUIRE(data.size() == size0);

    test_00_seq_find_itr<T, Size_type>(data);
    REQUIRE(0 != data.get_allocator().memory_usage);
    REQUIRE(data.size() == size0);
    if( do_print_mem ) { print_mem(type_id+" 02 (full_)", data); }

    data.clear();
    REQUIRE(data.size() == 0);
    // if( do_print_mem ) { print_mem(type_id+" 02 (clear)", data); }
    // REQUIRE(0 == data.get_allocator().memory_usage);
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

template<class T, typename Size_type>
static bool benchmark_fillseq_list_idx(const std::string& title_pre, const std::string& type_id,
                                         const bool do_rserv) {
    if( catch_perf_analysis ) {
        BENCHMARK(title_pre+" FillSeq_List 1000") {
            return test_01_seq_fill_list_idx<T, Size_type>(type_id, 1000, do_rserv? 1000 : 0, false);
        };
        return true;
    }
    if( catch_auto_run ) {
        test_01_seq_fill_list_idx<T, Size_type>(type_id, 50, do_rserv? 50 : 0, false);
        return true;
    }
    // BENCHMARK(title_pre+" FillSeq_List 25") {
    //     return test_01_seq_fill_list_idx<T, Size_type>(type_id, 25, do_rserv? 25 : 0, false);
    // };
    BENCHMARK(title_pre+" FillSeq_List 50") {
        return test_01_seq_fill_list_idx<T, Size_type>(type_id, 50, do_rserv? 50 : 0, false);
    };
    BENCHMARK(title_pre+" FillSeq_List 100") {
        return test_01_seq_fill_list_idx<T, Size_type>(type_id, 100, do_rserv? 100 : 0, false);
    };
    BENCHMARK(title_pre+" FillSeq_List 1000") {
        return test_01_seq_fill_list_idx<T, Size_type>(type_id, 1000, do_rserv? 1000 : 0, false);
    };
    return true;
}
template<class T, typename Size_type>
static bool benchmark_fillseq_list_itr(const std::string& title_pre, const std::string& type_id,
                                         const bool do_rserv) {
    if( catch_perf_analysis ) {
        BENCHMARK(title_pre+" FillSeq_List 1000") {
            return test_01_seq_fill_list_itr<T, Size_type>(type_id, 1000, do_rserv? 1000 : 0, false);
        };
        return true;
    }
    if( catch_auto_run ) {
        test_01_seq_fill_list_itr<T, Size_type>(type_id, 50, do_rserv? 50 : 0, false);
        return true;
    }
    // BENCHMARK(title_pre+" FillSeq_List 25") {
    //     return test_01_seq_fill_list_idx<T, Size_type>(type_id, 25, do_rserv? 25 : 0, false);
    // };
    BENCHMARK(title_pre+" FillSeq_List 50") {
        return test_01_seq_fill_list_itr<T, Size_type>(type_id, 50, do_rserv? 50 : 0, false);
    };
    BENCHMARK(title_pre+" FillSeq_List 100") {
        return test_01_seq_fill_list_itr<T, Size_type>(type_id, 100, do_rserv? 100 : 0, false);
    };
    BENCHMARK(title_pre+" FillSeq_List 1000") {
        return test_01_seq_fill_list_itr<T, Size_type>(type_id, 1000, do_rserv? 1000 : 0, false);
    };
    return true;
}

template<class T, typename Size_type>
static bool benchmark_fillunique_find_idx(const std::string& title_pre, const std::string& type_id,
                                                const bool do_rserv) {
    if( catch_perf_analysis ) {
        BENCHMARK(title_pre+" FillUni_List 1000") {
            return test_02_seq_fillunique_find_idx<T, Size_type>(type_id, 1000, do_rserv? 1000 : 0, false);
        };
        return true;
    }
    if( catch_auto_run ) {
        test_02_seq_fillunique_find_idx<T, Size_type>(type_id, 50, do_rserv? 50 : 0, false);
        return true;
    }
    // BENCHMARK(title_pre+" FillUni_List 25") {
    //    return test_02_seq_fillunique_find_idx<T, Size_type>(type_id, 25, do_rserv? 25 : 0, false);
    // };
    BENCHMARK(title_pre+" FillUni_List 50") {
        return test_02_seq_fillunique_find_idx<T, Size_type>(type_id, 50, do_rserv? 50 : 0, false);
    };
    BENCHMARK(title_pre+" FillUni_List 100") {
        return test_02_seq_fillunique_find_idx<T, Size_type>(type_id, 100, do_rserv? 100 : 0, false);
    };
    BENCHMARK(title_pre+" FillUni_List 1000") {
        return test_02_seq_fillunique_find_idx<T, Size_type>(type_id, 1000, do_rserv? 1000 : 0, false);
    };
    return true;
}
template<class T, typename Size_type>
static bool benchmark_fillunique_find_itr(const std::string& title_pre, const std::string& type_id,
                                                const bool do_rserv) {
    if( catch_perf_analysis ) {
        BENCHMARK(title_pre+" FillUni_List 1000") {
            return test_02_seq_fillunique_find_itr<T, Size_type>(type_id, 1000, do_rserv? 1000 : 0, false);
        };
        // test_02_seq_fillunique_find_itr<T, Size_type>(type_id, 100000, do_rserv? 100000 : 0, false);
        return true;
    }
    if( catch_auto_run ) {
        test_02_seq_fillunique_find_itr<T, Size_type>(type_id, 50, do_rserv? 50 : 0, false);
        return true;
    }
    // BENCHMARK(title_pre+" FillUni_List 25") {
    //    return test_02_seq_fillunique_find_itr<T, Size_type>(type_id, 25, do_rserv? 25 : 0, false);
    // };
    BENCHMARK(title_pre+" FillUni_List 50") {
        return test_02_seq_fillunique_find_itr<T, Size_type>(type_id, 50, do_rserv? 50 : 0, false);
    };
    BENCHMARK(title_pre+" FillUni_List 100") {
        return test_02_seq_fillunique_find_itr<T, Size_type>(type_id, 100, do_rserv? 100 : 0, false);
    };
    BENCHMARK(title_pre+" FillUni_List 1000") {
        return test_02_seq_fillunique_find_itr<T, Size_type>(type_id, 1000, do_rserv? 1000 : 0, false);
    };
    return true;
}
template<class T, typename Size_type>
static bool benchmark_fillunique_find_itr_rserv(const std::string& title_pre, const std::string& type_id) {
    if( catch_perf_analysis ) {
        BENCHMARK(title_pre+" FillUni_List 1000") {
            return test_02b_seq_fillunique_find_itr_rserv<T, Size_type>(type_id, 1000, false);
        };
        // test_02b_seq_fillunique_find_itr_rserv<T, Size_type>(type_id, 100000, false);
        return true;
    }
    if( catch_auto_run ) {
        test_02b_seq_fillunique_find_itr_rserv<T, Size_type>(type_id, 50, false);
        return true;
    }
    // BENCHMARK(title_pre+" FillUni_List 25") {
    //    return test_02b_seq_fillunique_find_itr_rserv<T, Size_type>(type_id, 25, false);
    // };
    BENCHMARK(title_pre+" FillUni_List 50") {
        return test_02b_seq_fillunique_find_itr_rserv<T, Size_type>(type_id, 50, false);
    };
    BENCHMARK(title_pre+" FillUni_List 100") {
        return test_02b_seq_fillunique_find_itr_rserv<T, Size_type>(type_id, 100, false);
    };
    BENCHMARK(title_pre+" FillUni_List 1000") {
        return test_02b_seq_fillunique_find_itr_rserv<T, Size_type>(type_id, 1000, false);
    };
    return true;
}

/****************************************************************************************
 ****************************************************************************************/

TEST_CASE( "Memory Footprint 01 - Fill Sequential and List", "[datatype][footprint]" ) {
    if( catch_perf_analysis ) {
#if 1
        footprint_fillseq_list_itr< jau::cow_vector<DataType01, counting_allocator<DataType01>>, std::size_t >("cowstdvec_empty_", false);
        footprint_fillseq_list_itr< jau::cow_darray<DataType01, counting_allocator<DataType01>>, jau::nsize_t >("cowdarray_empty_", false);
#endif
        return;
    }
    footprint_fillseq_list_itr< std::vector<DataType01, counting_allocator<DataType01>>, std::size_t >("stdvec_empty_", false);
    footprint_fillseq_list_itr< jau::darray<DataType01, counting_allocator<DataType01>>, jau::nsize_t >("darray_empty_", false);
    footprint_fillseq_list_itr< jau::cow_vector<DataType01, counting_allocator<DataType01>>, std::size_t >("cowstdvec_empty_", false);
    footprint_fillseq_list_itr< jau::cow_darray<DataType01, counting_allocator<DataType01>>, jau::nsize_t >("cowdarray_empty_", false);

#if 0
    footprint_fillseq_list_itr< std::vector<DataType01, counting_allocator<DataType01>>, std::size_t >("stdvec_rserv", true);
    footprint_fillseq_list_itr< jau::darray<DataType01, counting_allocator<DataType01>>, jau::nsize_t >("darray_rserv", true);
    footprint_fillseq_list_itr< jau::cow_vector<DataType01, counting_allocator<DataType01>>, std::size_t >("cowstdvec_rserv", true);
    footprint_fillseq_list_itr< jau::cow_darray<DataType01, counting_allocator<DataType01>>, jau::nsize_t >("cowdarray_rserv", true);
#endif
}

TEST_CASE( "Perf Test 01 - Fill Sequential and List, empty and reserve", "[datatype][sequential]" ) {
    if( catch_perf_analysis ) {
#if 1
        benchmark_fillseq_list_itr< std::vector<DataType01, counting_allocator<DataType01>>, std::size_t >("STD_Vector_empty_itr", "stdvec_empty_", false);
        benchmark_fillseq_list_itr< jau::darray<DataType01, counting_allocator<DataType01>, jau::nsize_t>, jau::nsize_t >("JAU_DArray_empty_itr", "darray_empty_", false);
#endif
        return;
    }
    benchmark_fillseq_list_idx< std::vector<DataType01, counting_allocator<DataType01>>, std::size_t >("STD_Vector_empty_idx", "stdvec_empty_", false);
    benchmark_fillseq_list_itr< std::vector<DataType01, counting_allocator<DataType01>>, std::size_t >("STD_Vector_empty_itr", "stdvec_empty_", false);

    benchmark_fillseq_list_idx< jau::darray<DataType01, counting_allocator<DataType01>>, jau::nsize_t >("JAU_DArray_empty_idx", "darray_empty_", false);
    benchmark_fillseq_list_itr< jau::darray<DataType01, counting_allocator<DataType01>, jau::nsize_t>, jau::nsize_t >("JAU_DArray_empty_itr", "darray_empty_", false);

    // benchmark_fillseq_list_idx< jau::cow_vector<DataType01, counting_allocator<DataType01>>, std::size_t >("COW_Vector_empty_idx", "cowstdvec_empty_", false);
    benchmark_fillseq_list_itr< jau::cow_vector<DataType01, counting_allocator<DataType01>>, std::size_t >("COW_Vector_empty_itr", "cowstdvec_empty_", false);

    // benchmark_fillseq_list_idx< jau::cow_darray<DataType01, counting_allocator<DataType01>>, jau::nsize_t >("COW_DArray_empty_idx", "cowdarray_empty_", false);
    benchmark_fillseq_list_itr< jau::cow_darray<DataType01, counting_allocator<DataType01>>, jau::nsize_t >("COW_DArray_empty_itr", "cowdarray_empty_", false);

    benchmark_fillseq_list_itr< std::vector<DataType01, counting_allocator<DataType01>>, std::size_t >("STD_Vector_rserv_itr", "stdvec_rserv", true);
    benchmark_fillseq_list_itr< jau::darray<DataType01, counting_allocator<DataType01>>, jau::nsize_t >("JAU_DArray_rserv_itr", "darray_rserv", true);
    benchmark_fillseq_list_itr< jau::cow_vector<DataType01, counting_allocator<DataType01>>, std::size_t >("COW_Vector_rserv_itr", "cowstdvec_rserv", true);
    benchmark_fillseq_list_itr< jau::cow_vector<DataType01, counting_allocator<DataType01>>, std::size_t >("COW_Vector_rserv_itr", "cowstdvec_rserv", true);
}

TEST_CASE( "Perf Test 02 - Fill Unique and List, empty and reserve", "[datatype][unique]" ) {
    if( catch_perf_analysis ) {
#if 1
        // benchmark_fillunique_find_itr_rserv< jau::cow_vector<DataType01, counting_allocator<DataType01>>, std::size_t >("COW_Vector_rserv_itr", "cowstdvec_rserv_");
        // benchmark_fillunique_find_itr_rserv< jau::cow_darray<DataType01, counting_allocator<DataType01>>, jau::nsize_t >("COW_DArray_rserv_itr", "cowdarray_rserv_");

        benchmark_fillunique_find_itr< jau::cow_vector<DataType01, counting_allocator<DataType01>>, std::size_t >("COW_Vector_empty_itr", "cowstdvec_empty_", false);
        benchmark_fillunique_find_itr< jau::cow_darray<DataType01, counting_allocator<DataType01>>, jau::nsize_t >("COW_DArray_empty_itr", "cowdarray_empty_", false);
#endif
        return;
    }
    benchmark_fillunique_find_idx< std::vector<DataType01, counting_allocator<DataType01>>, std::size_t >("STD_Vector_empty_idx", "stdvec_empty_", false);

    benchmark_fillunique_find_itr< std::vector<DataType01, counting_allocator<DataType01>>, std::size_t >("STD_Vector_empty_itr", "stdvec_empty_", false);

    benchmark_fillunique_find_idx< jau::darray<DataType01, counting_allocator<DataType01>>, jau::nsize_t >("JAU_DArray_empty_idx", "darray_empty_", false);
    benchmark_fillunique_find_itr< jau::darray<DataType01, counting_allocator<DataType01>>, jau::nsize_t >("JAU_DArray_empty_itr", "darray_empty_", false);

    // benchmark_fillunique_find_idx< jau::cow_vector<DataType01, counting_allocator<DataType01>>, std::size_t >("COW_Vector_empty_idx", "cowstdvec_empty_", false);
    benchmark_fillunique_find_itr< jau::cow_vector<DataType01, counting_allocator<DataType01>>, std::size_t >("COW_Vector_empty_itr", "cowstdvec_empty_", false);

    // benchmark_fillunique_find_idx< jau::cow_darray<DataType01, counting_allocator<DataType01>>, jau::nsize_t >("COW_DArray_empty_idx", "cowdarray_empty_", false);
    benchmark_fillunique_find_itr< jau::cow_darray<DataType01, counting_allocator<DataType01>>, jau::nsize_t >("COW_DArray_empty_itr", "cowdarray_empty_", false);

#if 1
    benchmark_fillunique_find_itr_rserv< std::vector<DataType01, counting_allocator<DataType01>>, std::size_t >("STD_Vector_rserv_itr", "stdvec_rserv");
    benchmark_fillunique_find_itr_rserv< jau::darray<DataType01, counting_allocator<DataType01>>, jau::nsize_t >("JAU_DArray_rserv_itr", "darray_rserv");
    benchmark_fillunique_find_itr_rserv< jau::cow_vector<DataType01, counting_allocator<DataType01>>, std::size_t >("COW_Vector_rserv_itr", "cowstdvec_rserv");
    benchmark_fillunique_find_itr_rserv< jau::cow_darray<DataType01, counting_allocator<DataType01>>, jau::nsize_t >("COW_DArray_rserv_itr", "cowdarray_rserv");
#endif
}

