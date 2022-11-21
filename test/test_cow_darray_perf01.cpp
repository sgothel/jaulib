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
#include <cassert>
#include <cinttypes>
#include <cstring>
#include <random>
#include <vector>

#include <jau/test/catch2_ext.hpp>

#include "test_datatype01.hpp"

#include <jau/basic_types.hpp>
#include <jau/basic_algos.hpp>
#include <jau/darray.hpp>
#include <jau/cow_darray.hpp>
#include <jau/cow_vector.hpp>
#include <jau/counting_allocator.hpp>
#include <jau/callocator.hpp>
#include <jau/counting_callocator.hpp>

/**
 * Performance test of jau::darray, jau::cow_darray and jau::cow_vector.
 */
using namespace jau;

#define RUN_RESERVE_BENCHMARK 0
#define RUN_INDEXED_BENCHMARK 0

/****************************************************************************************
 ****************************************************************************************/

template< class Cont >
static void print_container_info(const std::string& type_id, const Cont &c,
        std::enable_if_t< jau::is_darray_type<Cont>::value, bool> = true )
{
    printf("\nContainer Type %s (a darray, a cow %d):\n  - Uses memmove %d (trivially_copyable %d); realloc %d; base_of jau::callocator %d; secmem %d; size %d bytes\n",
                type_id.c_str(), jau::is_cow_type<Cont>::value,
                Cont::uses_memmove,
                std::is_trivially_copyable<typename Cont::value_type>::value,
                Cont::uses_realloc,
                std::is_base_of<jau::callocator<typename Cont::value_type>, typename Cont::allocator_type>::value,
                Cont::uses_secmem,
                (int)sizeof(c));
}

template<class Cont>
static void print_container_info(const std::string& type_id, const Cont &c,
        std::enable_if_t< !jau::is_darray_type<Cont>::value, bool> = true )
{
    printf("\nContainer Type %s (!darray, a cow %d); size %d bytes\n",
                type_id.c_str(), jau::is_cow_type<Cont>::value, (int)sizeof(c));
}

/****************************************************************************************
 ****************************************************************************************/

static uint8_t start_addr_b[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static Addr48Bit start_addr(start_addr_b);

/****************************************************************************************
 ****************************************************************************************/

template<class T, typename Size_type>
const DataType01 * findDataSet01_idx(T& data, DataType01 const & elem) noexcept {
    const Size_type size = data.size();
    for (Size_type i = 0; i < size; ++i) {
        DataType01 & e = data[i];
        if ( elem == e ) {
            return &e;
        }
    }
    return nullptr;
}

template<class T, typename Size_type>
static int test_00_list_idx(T& data) {
    int some_number = 0; // add some validated work, avoiding any 'optimization away'
    const Size_type size = data.size();
    for (Size_type i = 0; i < size; ++i) {
        const DataType01 & e = data[i];
        some_number += e.nop();
    }
    REQUIRE(some_number > 0);
    return some_number;
}

template<class T, typename Size_type>
const DataType01 * findDataSet01_itr(T& data, DataType01 const & elem,
        std::enable_if_t< is_cow_type<T>::value, bool> = true) noexcept
{
    typename T::const_iterator first = data.cbegin();
    for (; !first.is_end(); ++first) {
        if (*first == elem) {
            return &(*first);
        }
    }
    return nullptr;
}
template<class T, typename Size_type>
const DataType01 * findDataSet01_itr(T& data, DataType01 const & elem,
        std::enable_if_t< !is_cow_type<T>::value, bool> = true) noexcept
{
    typename T::const_iterator first = data.cbegin();
    typename T::const_iterator last = data.cend();
    for (; first != last; ++first) {
        if (*first == elem) {
            return &(*first);
        }
    }
    return nullptr;
}

template<class T>
static int test_00_list_itr(T& data,
        std::enable_if_t< is_cow_type<T>::value, bool> = true )
{
    int some_number = 0; // add some validated work, avoiding any 'optimization away'
    typename T::const_iterator first = data.cbegin();
    for (; !first.is_end(); ++first) {
        some_number += (*first).nop();
    }
    REQUIRE(some_number > 0);
    return some_number;
}

template<class T>
static int test_00_list_itr(T& data,
        std::enable_if_t< !is_cow_type<T>::value, bool> = true )
{
    int some_number = 0; // add some validated work, avoiding any 'optimization away'
    typename T::const_iterator first = data.cbegin();
    typename T::const_iterator last = data.cend();
    for (; first != last; ++first) {
        some_number += (*first).nop();
    }
    REQUIRE(some_number > 0);
    return some_number;
}


template<class T, typename Size_type>
static void test_00_seq_find_idx(T& data) {
    Addr48Bit a0(start_addr);
    const Size_type size = data.size();
    Size_type fi = 0, i=0;

    for(; i<size && a0.next(); ++i) {
        DataType01 elem(a0, static_cast<uint8_t>(1));
        const DataType01 *found = findDataSet01_idx<T, Size_type>(data, elem);
        if( nullptr != found ) {
            ++fi;
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

    for(; i<size && a0.next(); ++i) {
        DataType01 elem(a0, static_cast<uint8_t>(1));
        const DataType01 *found = findDataSet01_itr<T, Size_type>(data, elem);
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
static void test_00_seq_fill_unique_idx(T& data, const Size_type size) {
    Addr48Bit a0(start_addr);
    Size_type i=0, fi=0;

    for(; i<size && a0.next(); ++i) {
        DataType01 elem(a0, static_cast<uint8_t>(1));
        const DataType01* exist = findDataSet01_idx<T, Size_type>(data, elem);
        if( nullptr == exist ) {
            data.push_back( std::move( elem ) );
            ++fi;
        }
    }
    REQUIRE(i == data.size());
    REQUIRE(fi == size);
}

template<class value_type>
static bool equal_comparator(const value_type& a, const value_type& b) {
    return a == b;
}

template<class T, typename Size_type>
static void test_00_seq_fill_unique_itr(T& data, const Size_type size,
        std::enable_if_t< is_cow_type<T>::value, bool> = true) noexcept
{
    Addr48Bit a0(start_addr);
    Size_type i=0, fi=0;

#if 0
    typename T::iterator first = data.begin();

    for(; i<size && a0.next(); ++i, first.to_begin()) {
        DataType01 elem(a0, static_cast<uint8_t>(1));
        for (; !first.is_end(); ++first) {
            if (*first == elem) {
                break;
            }
        }
        if( first.is_end() ) {
            first.push_back( std::move( elem ) );
            ++fi;
        }
    }
    first.write_back();
#else
    for(; i<size && a0.next(); ++i) {
        if( data.push_back_unique( DataType01(a0, static_cast<uint8_t>(1)),
                                   equal_comparator<typename T::value_type> ) ) {
            ++fi;
        }
    }
#endif
    REQUIRE(i == data.size());
    REQUIRE(fi == size);
}

template<class T, typename Size_type>
static void test_00_seq_fill_unique_itr(T& data, const Size_type size,
        std::enable_if_t< !is_cow_type<T>::value, bool> = true) 
{
    Addr48Bit a0(start_addr);
    Size_type i=0, fi=0;

    for(; i<size && a0.next(); ++i) {
        DataType01 elem(a0, static_cast<uint8_t>(1));
        typename T::const_iterator first = data.cbegin();
        typename T::const_iterator last = data.cend();
        for (; first != last; ++first) {
            if (*first == elem) {
                break;
            }
        }
        if( first == last ) {
            data.push_back( std::move( elem ) );
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
static bool test_01_seq_fill_list_idx(const std::string& type_id, const Size_type size0, const Size_type reserve0) {
    (void)type_id;
    T data;
    REQUIRE(data.size() == 0);

    if( 0 < reserve0 ) {
        data.reserve(reserve0);
        REQUIRE(data.size() == 0);
        REQUIRE(data.capacity() == reserve0);
    }

    test_00_seq_fill<T, Size_type>(data, size0);
    REQUIRE(data.size() == size0);
    REQUIRE(data.capacity() >= size0);

    test_00_list_idx<T, Size_type>(data);
    REQUIRE(data.size() == size0);
    REQUIRE(data.capacity() >= size0);

    data.clear();
    REQUIRE(data.size() == 0);
    return data.size() == 0;
}

template<class T, typename Size_type>
static bool test_01_seq_fill_list_footprint(const std::string& type_id, const Size_type size0, const Size_type reserve0, const bool do_print_mem) {
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
    REQUIRE(data.capacity() >= size0);

    test_00_list_itr<T>(data);
    REQUIRE(0 != data.get_allocator().memory_usage);
    REQUIRE(data.size() == size0);
    REQUIRE(data.capacity() >= size0);
    if( do_print_mem ) { print_mem(type_id+" 01 (full_)", data); }

    data.clear();
    REQUIRE(data.size() == 0);
    // if( do_print_mem ) { print_mem(type_id+" 01 (clear)", data); }
    // REQUIRE(0 == data.get_allocator().memory_usage);
    return data.size() == 0;
}

template<class T, typename Size_type>
static bool test_01_seq_fill_list_itr(const std::string& type_id, const Size_type size0, const Size_type reserve0) {
    (void)type_id;
    T data;
    REQUIRE(data.size() == 0);

    if( 0 < reserve0 ) {
        data.reserve(reserve0);
        REQUIRE(data.size() == 0);
        REQUIRE(data.capacity() == reserve0);
    }

    test_00_seq_fill<T, Size_type>(data, size0);
    REQUIRE(data.size() == size0);
    REQUIRE(data.capacity() >= size0);

    test_00_list_itr<T>(data);
    REQUIRE(data.size() == size0);
    REQUIRE(data.capacity() >= size0);

    data.clear();
    REQUIRE(data.size() == 0);
    return data.size() == 0;
}

template<class T, typename Size_type>
static bool test_02_seq_fillunique_find_idx(const std::string& type_id, const Size_type size0, const Size_type reserve0) {
    (void)type_id;
    T data;
    REQUIRE(data.size() == 0);

    if( 0 < reserve0 ) {
        data.reserve(reserve0);
        REQUIRE(data.size() == 0);
        REQUIRE(data.capacity() == reserve0);
    }

    test_00_seq_fill_unique_idx<T, Size_type>(data, size0);
    REQUIRE(data.size() == size0);
    REQUIRE(data.capacity() >= size0);

    test_00_seq_find_idx<T, Size_type>(data);
    REQUIRE(data.size() == size0);
    REQUIRE(data.capacity() >= size0);

    data.clear();
    REQUIRE(data.size() == 0);
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
    REQUIRE(data.capacity() >= size0);

    test_00_seq_find_itr<T, Size_type>(data);
    REQUIRE(data.size() == size0);
    REQUIRE(data.capacity() >= size0);

    data.clear();
    REQUIRE(data.size() == 0);
    return data.size() == 0;
}

/****************************************************************************************
 ****************************************************************************************/

template<class T, typename Size_type>
static bool footprint_fillseq_list_itr(const std::string& type_id, const bool do_rserv) {
    {
        T data;
        print_container_info(type_id, data);
    }
    // test_01_seq_fill_list_footprint<T, Size_type>(type_id, 25, do_rserv? 25 : 0, true);
    test_01_seq_fill_list_footprint<T, Size_type>(type_id, 50, do_rserv? 50 : 0, true);
    if( !catch_auto_run ) {
        test_01_seq_fill_list_footprint<T, Size_type>(type_id, 100, do_rserv? 100 : 0, true);
        test_01_seq_fill_list_footprint<T, Size_type>(type_id, 1000, do_rserv? 1000 : 0, true);
    }
    return true;
}

template<class T, typename Size_type>
static bool benchmark_fillseq_list_idx(const std::string& title_pre, const std::string& type_id,
                                         const bool do_rserv) {
#if RUN_INDEXED_BENCHMARK
    {
        T data;
        print_container_info(title_pre, data);
    }
    if( catch_perf_analysis ) {
        BENCHMARK(title_pre+" FillSeq_List 1000") {
            return test_01_seq_fill_list_idx<T, Size_type>(type_id, 1000, do_rserv? 1000 : 0);
        };
        return true;
    }
    if( catch_auto_run ) {
        test_01_seq_fill_list_idx<T, Size_type>(type_id, 50, do_rserv? 50 : 0);
        return true;
    }
    // BENCHMARK(title_pre+" FillSeq_List 25") {
    //     return test_01_seq_fill_list_idx<T, Size_type>(type_id, 25, do_rserv? 25 : 0);
    // };
    BENCHMARK(title_pre+" FillSeq_List 50") {
        return test_01_seq_fill_list_idx<T, Size_type>(type_id, 50, do_rserv? 50 : 0);
    };
    BENCHMARK(title_pre+" FillSeq_List 100") {
        return test_01_seq_fill_list_idx<T, Size_type>(type_id, 100, do_rserv? 100 : 0);
    };
    BENCHMARK(title_pre+" FillSeq_List 1000") {
        return test_01_seq_fill_list_idx<T, Size_type>(type_id, 1000, do_rserv? 1000 : 0);
    };
#else
    (void) title_pre;
    (void) type_id;
    (void) do_rserv;
#endif
    return true;
}
template<class T, typename Size_type>
static bool benchmark_fillseq_list_itr(const std::string& title_pre, const std::string& type_id,
                                         const bool do_rserv) {
    {
        T data;
        print_container_info(title_pre, data);
    }
    if( catch_perf_analysis ) {
        BENCHMARK(title_pre+" FillSeq_List 1000") {
           return test_01_seq_fill_list_itr<T, Size_type>(type_id, 1000, do_rserv? 1000 : 0);
        };
        // test_01_seq_fill_list_itr<T, Size_type>(type_id, 100000, do_rserv? 100000 : 0);
        return true;
    }
    if( catch_auto_run ) {
        test_01_seq_fill_list_itr<T, Size_type>(type_id, 50, do_rserv? 50 : 0);
        return true;
    }
    // BENCHMARK(title_pre+" FillSeq_List 25") {
    //     return test_01_seq_fill_list_idx<T, Size_type>(type_id, 25, do_rserv? 25 : 0);
    // };
    BENCHMARK(title_pre+" FillSeq_List 50") {
        return test_01_seq_fill_list_itr<T, Size_type>(type_id, 50, do_rserv? 50 : 0);
    };
    BENCHMARK(title_pre+" FillSeq_List 100") {
        return test_01_seq_fill_list_itr<T, Size_type>(type_id, 100, do_rserv? 100 : 0);
    };
    BENCHMARK(title_pre+" FillSeq_List 1000") {
        return test_01_seq_fill_list_itr<T, Size_type>(type_id, 1000, do_rserv? 1000 : 0);
    };
    return true;
}

template<class T, typename Size_type>
static bool benchmark_fillunique_find_idx(const std::string& title_pre, const std::string& type_id,
                                                const bool do_rserv) {
#if RUN_INDEXED_BENCHMARK
    {
        T data;
        print_container_info(title_pre, data);
    }
    if( catch_perf_analysis ) {
        BENCHMARK(title_pre+" FillUni_List 1000") {
            return test_02_seq_fillunique_find_idx<T, Size_type>(type_id, 1000, do_rserv? 1000 : 0);
        };
        return true;
    }
    if( catch_auto_run ) {
        test_02_seq_fillunique_find_idx<T, Size_type>(type_id, 50, do_rserv? 50 : 0);
        return true;
    }
    // BENCHMARK(title_pre+" FillUni_List 25") {
    //    return test_02_seq_fillunique_find_idx<T, Size_type>(type_id, 25, do_rserv? 25 : 0);
    // };
    BENCHMARK(title_pre+" FillUni_List 50") {
        return test_02_seq_fillunique_find_idx<T, Size_type>(type_id, 50, do_rserv? 50 : 0);
    };
    BENCHMARK(title_pre+" FillUni_List 100") {
        return test_02_seq_fillunique_find_idx<T, Size_type>(type_id, 100, do_rserv? 100 : 0);
    };
    BENCHMARK(title_pre+" FillUni_List 1000") {
        return test_02_seq_fillunique_find_idx<T, Size_type>(type_id, 1000, do_rserv? 1000 : 0);
    };
#else
    (void) title_pre;
    (void) type_id;
    (void) do_rserv;
#endif
    return true;
}
template<class T, typename Size_type>
static bool benchmark_fillunique_find_itr(const std::string& title_pre, const std::string& type_id,
                                                const bool do_rserv) {
    {
        T data;
        print_container_info(title_pre, data);
    }
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

/****************************************************************************************
 ****************************************************************************************/

TEST_CASE( "Memory Footprint 01 - Fill Sequential and List", "[datatype][footprint]" ) {
    if( catch_perf_analysis ) {
        // footprint_fillseq_list_itr< jau::cow_vector<DataType01, counting_allocator<DataType01>>,                std::size_t>("cowstdvec_empty_", false);
        // footprint_fillseq_list_itr< jau::cow_darray<DataType01, counting_callocator<DataType01>, jau::nsize_t>, jau::nsize_t>("cowdarray_empty_", false);
        return;
    }
    footprint_fillseq_list_itr< std::vector<DataType01, counting_allocator<DataType01>>,                std::size_t>("stdvec_def_empty_", false);
    footprint_fillseq_list_itr< jau::darray<DataType01, counting_callocator<DataType01>, jau::nsize_t>, jau::nsize_t>("darray_def_empty_", false);
    footprint_fillseq_list_itr< jau::darray<DataType01, counting_callocator<DataType01>, jau::nsize_t, true /* memmove */>, jau::nsize_t>("darray_mmm_empty_", false);
    footprint_fillseq_list_itr< jau::cow_vector<DataType01, counting_allocator<DataType01>>,                std::size_t>("cowstdvec_def_empty_", false);
    footprint_fillseq_list_itr< jau::cow_darray<DataType01, counting_callocator<DataType01>, jau::nsize_t>, jau::nsize_t>("cowdarray_def_empty_", false);
    footprint_fillseq_list_itr< jau::cow_darray<DataType01, counting_callocator<DataType01>, jau::nsize_t, true /* memmove */>, jau::nsize_t>("cowdarray_mmm_empty_", false);

#if RUN_RESERVE_BENCHMARK
    footprint_fillseq_list_itr< std::vector<DataType01, counting_allocator<DataType01>>,                std::size_t>("stdvec_def_rserv", true);
    footprint_fillseq_list_itr< jau::darray<DataType01, counting_callocator<DataType01>, jau::nsize_t>, jau::nsize_t>("darray_def_rserv", true);
    footprint_fillseq_list_itr< jau::darray<DataType01, counting_callocator<DataType01>, jau::nsize_t, true /* memmove */>, jau::nsize_t>("darray_mmm_rserv", true);
    footprint_fillseq_list_itr< jau::cow_vector<DataType01, counting_allocator<DataType01>>,                std::size_t>("cowstdvec_def_rserv", true);
    footprint_fillseq_list_itr< jau::cow_darray<DataType01, counting_callocator<DataType01>, jau::nsize_t>, jau::nsize_t>("cowdarray_def_rserv", true);
    footprint_fillseq_list_itr< jau::cow_darray<DataType01, counting_callocator<DataType01>, jau::nsize_t, true /* memmove */>, jau::nsize_t>("cowdarray_mmm_rserv", true);
#endif
}

TEST_CASE( "Perf Test 01 - Fill Sequential and List, empty and reserve", "[datatype][sequential]" ) {
    if( catch_perf_analysis ) {
        // benchmark_fillseq_list_itr< jau::cow_vector<DataType01, std::allocator<DataType01>>,            std::size_t>("COW_Vector_def_empty_itr", "cowstdvec_empty_", false);
        // benchmark_fillseq_list_itr< jau::darray<DataType01, jau::callocator<DataType01>, jau::nsize_t>, jau::nsize_t>("JAU_DArray_def_empty_itr", "darray_empty_", false);
        benchmark_fillseq_list_itr< std::vector<DataType01, std::allocator<DataType01>>,                std::size_t>("STD_Vector_def_empty_itr", "stdvec_empty_", false);
        benchmark_fillseq_list_itr< jau::darray<DataType01, jau::callocator<DataType01>, jau::nsize_t>, jau::nsize_t>("JAU_DArray_def_empty_itr", "darray_empty_", false);
        benchmark_fillseq_list_itr< jau::darray<DataType01, jau::callocator<DataType01>, jau::nsize_t, true /* memmove */>, jau::nsize_t>("JAU_DArray_mmm_empty_itr", "darray_empty_", false);
#if RUN_RESERVE_BENCHMARK
        benchmark_fillseq_list_itr< std::vector<DataType01, std::allocator<DataType01>>,                std::size_t>("STD_Vector_def_rserv_itr", "stdvec_rserv", true);
        benchmark_fillseq_list_itr< jau::darray<DataType01, jau::callocator<DataType01>, jau::nsize_t>, jau::nsize_t>("JAU_DArray_def_rserv_itr", "darray_rserv", true);
        benchmark_fillseq_list_itr< jau::darray<DataType01, jau::callocator<DataType01>, jau::nsize_t, true /* memmove */>, jau::nsize_t>("JAU_DArray_mmm_rserv_itr", "darray_rserv", true);
#endif
        return;
    }
    benchmark_fillseq_list_idx< std::vector<DataType01, std::allocator<DataType01>>,                std::size_t>("STD_Vector_def_empty_idx", "stdvec_empty_", false);
    benchmark_fillseq_list_itr< std::vector<DataType01, std::allocator<DataType01>>,                std::size_t>("STD_Vector_def_empty_itr", "stdvec_empty_", false);

    benchmark_fillseq_list_idx< jau::darray<DataType01, jau::callocator<DataType01>, jau::nsize_t>, jau::nsize_t>("JAU_DArray_def_empty_idx", "darray_empty_", false);
    benchmark_fillseq_list_idx< jau::darray<DataType01, jau::callocator<DataType01>, jau::nsize_t, true /* memmove */>, jau::nsize_t>("JAU_DArray_mmm_empty_idx", "darray_empty_", false);
    benchmark_fillseq_list_itr< jau::darray<DataType01, jau::callocator<DataType01>, jau::nsize_t>, jau::nsize_t>("JAU_DArray_def_empty_itr", "darray_empty_", false);
    benchmark_fillseq_list_itr< jau::darray<DataType01, jau::callocator<DataType01>, jau::nsize_t, true /* memmove */>, jau::nsize_t>("JAU_DArray_mmm_empty_itr", "darray_empty_", false);

    benchmark_fillseq_list_itr< jau::cow_vector<DataType01, std::allocator<DataType01>>,                std::size_t>("COW_Vector_def_empty_itr", "cowstdvec_empty_", false);

    benchmark_fillseq_list_itr< jau::cow_darray<DataType01, jau::callocator<DataType01>, jau::nsize_t>, jau::nsize_t>("COW_DArray_def_empty_itr", "cowdarray_empty_", false);
    benchmark_fillseq_list_itr< jau::cow_darray<DataType01, jau::callocator<DataType01>, jau::nsize_t, true /* memmove */>, jau::nsize_t>("COW_DArray_mmm_empty_itr", "cowdarray_empty_", false);

#if RUN_RESERVE_BENCHMARK
    benchmark_fillseq_list_itr< std::vector<DataType01, std::allocator<DataType01>>,                std::size_t>("STD_Vector_def_rserv_itr", "stdvec_rserv", true);
    benchmark_fillseq_list_itr< jau::darray<DataType01, jau::callocator<DataType01>, jau::nsize_t>, jau::nsize_t>("JAU_DArray_def_rserv_itr", "darray_rserv", true);
    benchmark_fillseq_list_itr< jau::darray<DataType01, jau::callocator<DataType01>, jau::nsize_t, true /* memmove */>, jau::nsize_t>("JAU_DArray_mmm_rserv_itr", "darray_rserv", true);
    benchmark_fillseq_list_itr< jau::cow_vector<DataType01, std::allocator<DataType01>>,               std::size_t>("COW_Vector_def_rserv_itr", "cowstdvec_rserv", true);
    benchmark_fillseq_list_itr< jau::cow_darray<DataType01, jau::callocator<DataType01>, jau::nsize_t>, std::size_t>("COW_DArray_def_rserv_itr", "cowdarray_rserv", true);
    benchmark_fillseq_list_itr< jau::cow_darray<DataType01, jau::callocator<DataType01>, jau::nsize_t, true /* memmove */>, std::size_t>("COW_DArray_mmm_rserv_itr", "cowdarray_rserv", true);
#endif
}

TEST_CASE( "Perf Test 02 - Fill Unique and List, empty and reserve", "[datatype][unique]" ) {
    if( catch_perf_analysis ) {
        benchmark_fillunique_find_itr< jau::cow_vector<DataType01, std::allocator<DataType01>>,                std::size_t>("COW_Vector_def_empty_itr", "cowstdvec_empty_", false);
        benchmark_fillunique_find_itr< jau::cow_darray<DataType01, jau::callocator<DataType01>, jau::nsize_t>, jau::nsize_t>("COW_DArray_def_empty_itr", "cowdarray_empty_", false);
        benchmark_fillunique_find_itr< jau::cow_darray<DataType01, jau::callocator<DataType01>, jau::nsize_t, true /* memmove */>, jau::nsize_t>("COW_DArray_mmm_empty_itr", "cowdarray_empty_", false);
#if RUN_RESERVE_BENCHMARK
        benchmark_fillunique_find_itr< jau::cow_vector<DataType01, std::allocator<DataType01>>,                std::size_t>("COW_Vector_def_rserv_itr", "cowstdvec_rserv", true);
        benchmark_fillunique_find_itr< jau::cow_darray<DataType01, jau::callocator<DataType01>, jau::nsize_t>, jau::nsize_t>("COW_DArray_def_rserv_itr", "cowdarray_rserv", true);
        benchmark_fillunique_find_itr< jau::cow_darray<DataType01, jau::callocator<DataType01>, jau::nsize_t, true /* memmove */>, jau::nsize_t>("COW_DArray_mmm_rserv_itr", "cowdarray_rserv", true);
#endif
        return;
    }
    benchmark_fillunique_find_idx< std::vector<DataType01, std::allocator<DataType01>>,                std::size_t>("STD_Vector_def_empty_idx", "stdvec_empty_", false);
    benchmark_fillunique_find_itr< std::vector<DataType01, std::allocator<DataType01>>,                std::size_t>("STD_Vector_def_empty_itr", "stdvec_empty_", false);

    benchmark_fillunique_find_idx< jau::darray<DataType01, jau::callocator<DataType01>, jau::nsize_t>, jau::nsize_t>("JAU_DArray_def_empty_idx", "darray_empty_", false);
    benchmark_fillunique_find_idx< jau::darray<DataType01, jau::callocator<DataType01>, jau::nsize_t, true /* memmove */>, jau::nsize_t>("JAU_DArray_mmm_empty_idx", "darray_empty_", false);
    benchmark_fillunique_find_itr< jau::darray<DataType01, jau::callocator<DataType01>, jau::nsize_t>, jau::nsize_t>("JAU_DArray_def_empty_itr", "darray_empty_", false);
    benchmark_fillunique_find_itr< jau::darray<DataType01, jau::callocator<DataType01>, jau::nsize_t, true /* memmove */>, jau::nsize_t>("JAU_DArray_mmm_empty_itr", "darray_empty_", false);

    benchmark_fillunique_find_itr< jau::cow_vector<DataType01, std::allocator<DataType01>>,                std::size_t>("COW_Vector_def_empty_itr", "cowstdvec_empty_", false);

    benchmark_fillunique_find_itr< jau::cow_darray<DataType01, jau::callocator<DataType01>, jau::nsize_t>, jau::nsize_t>("COW_DArray_def_empty_itr", "cowdarray_empty_", false);
    benchmark_fillunique_find_itr< jau::cow_darray<DataType01, jau::callocator<DataType01>, jau::nsize_t, true /* memmove */>, jau::nsize_t>("COW_DArray_mmm_empty_itr", "cowdarray_empty_", false);

#if RUN_RESERVE_BENCHMARK
    benchmark_fillunique_find_itr< std::vector<DataType01, std::allocator<DataType01>>,                    std::size_t>("STD_Vector_def_rserv_itr", "stdvec_rserv", true);
    benchmark_fillunique_find_itr< jau::darray<DataType01, jau::callocator<DataType01>, jau::nsize_t>,     jau::nsize_t>("JAU_DArray_def_rserv_itr", "darray_rserv", true);
    benchmark_fillunique_find_itr< jau::darray<DataType01, jau::callocator<DataType01>, jau::nsize_t, true /* memmove */>,     jau::nsize_t>("JAU_DArray_mmm_rserv_itr", "darray_rserv", true);
    benchmark_fillunique_find_itr< jau::cow_vector<DataType01, std::allocator<DataType01>>,                std::size_t>("COW_Vector_def_rserv_itr", "cowstdvec_rserv", true);
    benchmark_fillunique_find_itr< jau::cow_darray<DataType01, jau::callocator<DataType01>, jau::nsize_t>, jau::nsize_t>("COW_DArray_def_rserv_itr", "cowdarray_rserv", true);
    benchmark_fillunique_find_itr< jau::cow_darray<DataType01, jau::callocator<DataType01>, jau::nsize_t, true /* memmove */>, jau::nsize_t>("COW_DArray_mmm_rserv_itr", "cowdarray_rserv", true);
#endif
}

