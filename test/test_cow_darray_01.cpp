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

#include <jau/basic_algos.hpp>
#include <jau/basic_types.hpp>
#include <jau/darray.hpp>
#include <jau/cow_darray.hpp>
#include <jau/cow_vector.hpp>
#include <jau/counting_allocator.hpp>
#include <jau/callocator.hpp>
#include <jau/counting_callocator.hpp>

/**
 * Test general use of jau::darray, jau::cow_darray and jau::cow_vector.
 */
using namespace jau;

static uint8_t start_addr_b[] = {0x20, 0x26, 0x2A, 0x01, 0x20, 0x10};
static Addr48Bit start_addr(start_addr_b);

typedef std::vector<DataType01, counting_allocator<DataType01>> std_vector_DataType01;
typedef jau::darray<DataType01, counting_callocator<DataType01>> jau_darray_DataType01;
typedef jau::cow_vector<DataType01, counting_allocator<DataType01>> jau_cow_vector_DataType01;
typedef jau::cow_darray<DataType01, counting_callocator<DataType01>> jau_cow_darray_DataType01;

JAU_TYPENAME_CUE_ALL(std_vector_DataType01)
JAU_TYPENAME_CUE_ALL(jau_darray_DataType01)
JAU_TYPENAME_CUE_ALL(jau_cow_vector_DataType01)
JAU_TYPENAME_CUE_ALL(jau_cow_darray_DataType01)

template<class T>
DataType01 * findDataSet01_idx(T& data, DataType01 const & elem) noexcept {
    const size_t size = data.size();
    for (size_t i = 0; i < size; i++) {
        DataType01 & e = data[i];
        if ( elem == e ) {
            return &e;
        }
    }
    return nullptr;
}

template<class T>
static void test_00_list_idx(T& data, const bool show) {
    Addr48Bit a0(start_addr);
    const std::size_t size = data.size();
    for (std::size_t i = 0; i < size && a0.next(); ++i) {
        const DataType01 & e = data[i];
        e.nop();
        if( show ) {
            printf("data[%zu]: %s\n", i, e.toString().c_str());
        }
        REQUIRE(e.address == a0);
    }
}
template<class T>
static int test_00_list_itr(T& data, const bool show) {
    Addr48Bit a0(start_addr);
    int some_number = 0, i=0; // add some validated work, avoiding any 'optimization away'
    jau::for_each_const(data, [&some_number, &a0, &i, show](const DataType01 & e) {
        some_number += e.nop();
        if( show ) {
            printf("data[%d]: %s\n", i, e.toString().c_str());
        }
        REQUIRE( a0.next() );
        REQUIRE(e.address == a0);
        ++i;
    } );
    REQUIRE(some_number > 0);
    return some_number;
}

template<class T>
static void test_00_seq_find_idx(T& data) {
    Addr48Bit a0(start_addr);
    const std::size_t size = data.size();
    std::size_t fi = 0, i=0;

    for(; i<size && a0.next(); i++) {
        DataType01 elem(a0, static_cast<uint8_t>(1));
        DataType01 *found = findDataSet01_idx(data, elem);
        if( nullptr != found ) {
            fi++;
            found->nop();
        }
    }
    REQUIRE(fi == i);
}
template<class T>
static void test_00_seq_find_itr(T& data) {
    Addr48Bit a0(start_addr);
    const std::size_t size = data.size();
    std::size_t fi = 0, i=0;

    for(; i<size && a0.next(); i++) {
        DataType01 elem(a0, static_cast<uint8_t>(1));
        const DataType01 *found = jau::find_const(data, elem);
        if( nullptr != found ) {
            fi++;
            found->nop();
        }
    }
    REQUIRE(fi == i);
}

template<class T>
static void test_00_seq_fill(T& data, const std::size_t size) {
    Addr48Bit a0(start_addr);
    std::size_t i=0;

    for(; i<size && a0.next(); i++) {
        data.emplace_back( a0, static_cast<uint8_t>(1) );
    }
    if( i != data.size() ) {
        test_00_list_itr(data, true);
        printf("a0 %s\n", a0.toString().c_str());
        printf("Size %zu, expected %zu, iter %zu\n", static_cast<std::size_t>(data.size()), size, i);
    }
    REQUIRE(i == data.size());
}

template<class T>
static void test_00_seq_fill_unique_idx(T& data, const std::size_t size) {
    Addr48Bit a0(start_addr);
    std::size_t i=0, fi=0;

    for(; i<size && a0.next(); i++) {
        DataType01 elem(a0, static_cast<uint8_t>(1));
        DataType01* exist = findDataSet01_idx(data, elem);
        if( nullptr == exist ) {
            data.push_back( std::move(elem) );
            fi++;
        } else {
            printf("Not unique #%zu: %s == %s (%d)\n", i, elem.toString().c_str(), exist->toString().c_str(), (elem == *exist));
        }
    }
    if( fi != size ) {
        test_00_list_idx(data, true);
        printf("a0 %s\n", a0.toString().c_str());
        printf("Size %zu, expected %zu, iter %zu\n", static_cast<std::size_t>(data.size()), size, i);
    }
    REQUIRE(i == data.size());
    REQUIRE(fi == size);
}
template<class T>
static void test_00_seq_fill_unique_itr(T& data, const std::size_t size) {
    Addr48Bit a0(start_addr);
    std::size_t i=0, fi=0;

    for(; i<size && a0.next(); i++) {
        DataType01 elem(a0, static_cast<uint8_t>(1));
        const DataType01* exist = jau::find_const(data, elem);
        if( nullptr == exist ) {
            data.push_back( std::move(elem) );
            fi++;
        } else {
            printf("Not unique #%zu: %s == %s (%d)\n", i, elem.toString().c_str(), exist->toString().c_str(), (elem == *exist));
        }
    }
    if( fi != size ) {
        test_00_list_itr(data, true);
        printf("a0 %s\n", a0.toString().c_str());
        printf("Size %zu, expected %zu, iter %zu\n", static_cast<std::size_t>(data.size()), size, i);
    }
    REQUIRE(i == data.size());
    REQUIRE(fi == size);
}

/****************************************************************************************
 ****************************************************************************************/

template<class T>
static bool test_01_seq_fill_list_idx(const std::string& type_id, const std::size_t size0, const std::size_t reserve0) {
    (void)type_id;

    T data;
    REQUIRE(0 == data.get_allocator().memory_usage);
    REQUIRE(data.size() == 0);
    REQUIRE(data.capacity() == 0);

    if( 0 < reserve0 ) {
        data.reserve(reserve0);
        REQUIRE(data.size() == 0);
        REQUIRE(0 != data.get_allocator().memory_usage);
        REQUIRE(data.capacity() == reserve0);
    }

    test_00_seq_fill(data, size0);
    REQUIRE(0 != data.get_allocator().memory_usage);
    REQUIRE(data.size() == size0);

    test_00_list_idx(data, false);
    REQUIRE(0 != data.get_allocator().memory_usage);
    REQUIRE(data.size() == size0);

    data.clear();
    REQUIRE(data.size() == 0);
    // REQUIRE(0 == data.get_allocator().memory_usage);
    return data.size() == 0;
}
template<class T>
static bool test_01_seq_fill_list_itr(const std::string& type_id, const std::size_t size0, const std::size_t reserve0) {
    (void)type_id;

    T data;
    REQUIRE(0 == data.get_allocator().memory_usage);
    REQUIRE(data.size() == 0);
    REQUIRE(data.capacity() == 0);

    if( 0 < reserve0 ) {
        data.reserve(reserve0);
        REQUIRE(data.size() == 0);
        REQUIRE(0 != data.get_allocator().memory_usage);
        REQUIRE(data.capacity() == reserve0);
    }

    test_00_seq_fill(data, size0);
    REQUIRE(0 != data.get_allocator().memory_usage);
    REQUIRE(data.size() == size0);

    test_00_list_itr(data, false);
    REQUIRE(0 != data.get_allocator().memory_usage);
    REQUIRE(data.size() == size0);

    data.clear();
    REQUIRE(data.size() == 0);
    // REQUIRE(0 == data.get_allocator().memory_usage);
    return data.size() == 0;
}

template<class T>
static bool test_02_seq_fillunique_find_idx(const std::string& type_id, const std::size_t size0, const std::size_t reserve0) {
    (void)type_id;

    T data;
    REQUIRE(0 == data.get_allocator().memory_usage);
    REQUIRE(data.size() == 0);
    REQUIRE(data.capacity() == 0);

    if( 0 < reserve0 ) {
        data.reserve(reserve0);
        REQUIRE(data.size() == 0);
        REQUIRE(0 != data.get_allocator().memory_usage);
        REQUIRE(data.capacity() == reserve0);
    }

    test_00_seq_fill_unique_idx(data, size0);
    REQUIRE(0 != data.get_allocator().memory_usage);
    REQUIRE(data.size() == size0);

    test_00_seq_find_idx(data);
    REQUIRE(0 != data.get_allocator().memory_usage);
    REQUIRE(data.size() == size0);

    data.clear();
    REQUIRE(data.size() == 0);
    // REQUIRE(0 == data.get_allocator().memory_usage);
    return data.size() == 0;
}
template<class T>
static bool test_02_seq_fillunique_find_itr(const std::string& type_id, const std::size_t size0, const std::size_t reserve0) {
    (void)type_id;

    T data;
    REQUIRE(0 == data.get_allocator().memory_usage);
    REQUIRE(data.size() == 0);
    REQUIRE(data.capacity() == 0);

    if( 0 < reserve0 ) {
        data.reserve(reserve0);
        REQUIRE(data.size() == 0);
        REQUIRE(0 != data.get_allocator().memory_usage);
        REQUIRE(data.capacity() == reserve0);
    }

    test_00_seq_fill_unique_itr(data, size0);
    REQUIRE(0 != data.get_allocator().memory_usage);
    REQUIRE(data.size() == size0);

    test_00_seq_find_itr(data);
    REQUIRE(0 != data.get_allocator().memory_usage);
    REQUIRE(data.size() == size0);

    data.clear();
    REQUIRE(data.size() == 0);
    // REQUIRE(0 == data.get_allocator().memory_usage);
    return data.size() == 0;
}

/****************************************************************************************
 ****************************************************************************************/

TEST_CASE( "STD Vector Test 01 - Fill Sequential and List", "[datatype][std][vector]" ) {
    test_01_seq_fill_list_idx< std::vector<DataType01, counting_allocator<DataType01>> >("stdvec_fillseq_empty__", 100, 0);
    test_01_seq_fill_list_idx< std::vector<DataType01, counting_allocator<DataType01>> >("stdvec_fillseq_reserve", 100, 100);

    test_01_seq_fill_list_itr< std::vector<DataType01, counting_allocator<DataType01>> >("stdvec_fillseq_empty__", 100, 0);
    test_01_seq_fill_list_itr< std::vector<DataType01, counting_allocator<DataType01>> >("stdvec_fillseq_reserve", 100, 100);
}

TEST_CASE( "STD Vector Test 02 - Fill Unique and Find-Each", "[datatype][std][vector]" ) {
    test_02_seq_fillunique_find_idx< std::vector<DataType01, counting_allocator<DataType01>> >("stdvec_filluni_empty__", 100, 0);
    test_02_seq_fillunique_find_idx< std::vector<DataType01, counting_allocator<DataType01>> >("stdvec_filluni_reserve", 100, 100);

    test_02_seq_fillunique_find_itr< std::vector<DataType01, counting_allocator<DataType01>> >("stdvec_filluni_empty__", 100, 0);
    test_02_seq_fillunique_find_itr< std::vector<DataType01, counting_allocator<DataType01>> >("stdvec_filluni_reserve", 100, 100);
}

TEST_CASE( "JAU COW_Vector Test 11 - Fill Sequential and List", "[datatype][jau][cow_vector]" ) {
    // test_01_seq_fill_list_idx< jau::cow_vector<DataType01, counting_allocator<DataType01>> >("cowstdvec_fillseq_empty__", 100, 0);
    // test_01_seq_fill_list_idx< jau::cow_vector<DataType01, counting_allocator<DataType01>> >("cowstdvec_fillseq_reserve", 100, 100);

    test_01_seq_fill_list_itr< jau::cow_vector<DataType01, counting_allocator<DataType01>> >("cowstdvec_fillseq_empty__", 100, 0);
    test_01_seq_fill_list_itr< jau::cow_vector<DataType01, counting_allocator<DataType01>> >("cowstdvec_fillseq_reserve", 100, 100);
}

TEST_CASE( "JAU COW_Vector Test 12 - Fill Unique and Find-Each", "[datatype][jau][cow_vector]" ) {
    // test_02_seq_fillunique_find_idx< jau::cow_vector<DataType01, counting_allocator<DataType01>> >("cowstdvec_filluni_empty__", 100, 0);
    // test_02_seq_fillunique_find_idx< jau::cow_vector<DataType01, counting_allocator<DataType01>> >("cowstdvec_filluni_reserve", 100, 100);

    test_02_seq_fillunique_find_itr< jau::cow_vector<DataType01, counting_allocator<DataType01>> >("cowstdvec_filluni_empty__", 100, 0);
    test_02_seq_fillunique_find_itr< jau::cow_vector<DataType01, counting_allocator<DataType01>> >("cowstdvec_filluni_reserve", 100, 100);
}

TEST_CASE( "JAU DArray Test 21 - Fill Sequential and List", "[datatype][jau][darray]" ) {
    test_01_seq_fill_list_idx< jau::darray<DataType01, counting_callocator<DataType01>> >("darray_fillseq_empty__", 100, 0);
    test_01_seq_fill_list_idx< jau::darray<DataType01, counting_callocator<DataType01>> >("darray_fillseq_reserve", 100, 100);

    test_01_seq_fill_list_itr< jau::darray<DataType01, counting_callocator<DataType01>> >("darray_fillseq_empty__", 100, 0);
    test_01_seq_fill_list_itr< jau::darray<DataType01, counting_callocator<DataType01>> >("darray_fillseq_reserve", 100, 100);
}

TEST_CASE( "JAU DArray Test 22 - Fill Unique and Find-Each", "[datatype][jau][darray]" ) {
    test_02_seq_fillunique_find_idx< jau::darray<DataType01, counting_callocator<DataType01>> >("darray_filluni_empty__", 100, 0);
    test_02_seq_fillunique_find_idx< jau::darray<DataType01, counting_callocator<DataType01>> >("darray_filluni_reserve", 100, 100);

    test_02_seq_fillunique_find_itr< jau::darray<DataType01, counting_callocator<DataType01>> >("darray_filluni_empty__", 100, 0);
    test_02_seq_fillunique_find_itr< jau::darray<DataType01, counting_callocator<DataType01>> >("darray_filluni_reserve", 100, 100);
}

TEST_CASE( "JAU COW_DArray Test 31 - Fill Sequential and List", "[datatype][jau][cow_darray]" ) {
    // test_01_seq_fill_list_idx< jau::cow_darray<DataType01, counting_callocator<DataType01>> >("cowdarray_fillseq_empty__", 100, 0);
    // test_01_seq_fill_list_idx< jau::cow_darray<DataType01, counting_callocator<DataType01>> >("cowdarray_fillseq_reserve", 100, 100);

    test_01_seq_fill_list_itr< jau::cow_darray<DataType01, counting_callocator<DataType01>> >("cowdarray_fillseq_empty__", 100, 0);
    test_01_seq_fill_list_itr< jau::cow_darray<DataType01, counting_callocator<DataType01>> >("cowdarray_fillseq_reserve", 100, 100);
}

TEST_CASE( "JAU COW_DArray Test 32 - Fill Unique and Find-Each", "[datatype][jau][cow_darray]" ) {
    // test_02_seq_fillunique_find_idx< jau::cow_darray<DataType01, counting_callocator<DataType01>> >("cowdarray_filluni_empty__", 100, 0);
    // test_02_seq_fillunique_find_idx< jau::cow_darray<DataType01, counting_callocator<DataType01>> >("cowdarray_filluni_reserve", 100, 100);

    test_02_seq_fillunique_find_itr< jau::cow_darray<DataType01, counting_callocator<DataType01>> >("cowdarray_filluni_empty__", 100, 0);
    test_02_seq_fillunique_find_itr< jau::cow_darray<DataType01, counting_callocator<DataType01>> >("cowdarray_filluni_reserve", 100, 100);
}
