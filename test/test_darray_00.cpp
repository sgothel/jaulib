/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020-2024 Gothel Software e.K.
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
#include <cstring>

#include <jau/test/catch2_ext.hpp>

#include <jau/basic_algos.hpp>
#include <jau/basic_types.hpp>
#include <jau/darray.hpp>
#include <jau/counting_allocator.hpp>
#include <jau/callocator.hpp>
#include <jau/counting_callocator.hpp>

/**
 * Test general use of jau::darray, jau::cow_darray and jau::cow_vector.
 */
using namespace jau;

typedef jau::darray<int, jau::nsize_t, jau::callocator<int>, std::is_trivially_copyable_v<int>, true> secure_ints;
typedef jau::darray<int, jau::nsize_t, jau::callocator<int>, std::is_trivially_copyable_v<int>, false> normal_ints;

template<class T>
static void int_test() {
    T data;
    printf("COPY-0.1: %s\n\n", data.get_info().c_str());
    CHECK( 0 == data.size() );
    CHECK( 0 == data.capacity() );
    data.reserve(2);
    CHECK( 0 == data.size() );
    CHECK( 2 == data.capacity() );
    data.resize(2);
    CHECK( 2 == data.size() );
    CHECK( 2 == data.capacity() );
    int j=0;
    for(int i : data) {
        CHECK(0 == i);
        ++j;
    }
    CHECK(2 == j);
    
    data.resize(4, 42);
    printf("COPY-0.2: %s\n\n", data.get_info().c_str());
    CHECK( 4 == data.size() );
    CHECK( 4 == data.capacity() );
    j = 0;
    for(int i : data) {
        if( j < 2 ) {
            CHECK(0 == i);
        } else {
            CHECK(42 == i);
        }
        ++j;
    }
    CHECK(4 == j);
            
    data.erase(data.cbegin(), data.cend());
    printf("COPY-0.3: %s\n\n", data.get_info().c_str());
    CHECK( 0 == data.size() );
    CHECK( 4 == data.capacity() );
    
    data.push_back(1);
    data.push_back(2);
    printf("COPY-0.4: %s\n\n", data.get_info().c_str());
    CHECK( 2 == data.size() );
    CHECK( 4 == data.capacity() );
    CHECK(1 == data[0]);
    CHECK(2 == data[1]);
    
    data.shrink_to_fit();
    printf("COPY-0.5: %s\n\n", data.get_info().c_str());
    CHECK( 2 == data.size() );
    CHECK( 2 == data.capacity() );
    CHECK(1 == data[0]);
    CHECK(2 == data[1]);
    
    data.erase(data.cbegin(), data.cend());
    printf("COPY-0.6: %s\n\n", data.get_info().c_str());
    CHECK( 0 == data.size() );
    CHECK( 2 == data.capacity() );
    data.shrink_to_fit();
    
    printf("COPY-0.7: %s\n\n", data.get_info().c_str());
    CHECK( 0 == data.size() );
    CHECK( 0 == data.capacity() );
    
    data.push_back(42);
    printf("COPY-0.8: %s\n\n", data.get_info().c_str());
    CHECK( 1 == data.size() );
    CHECK( 1 <= data.capacity() );
    CHECK(42 == data[0]);
    
}

TEST_CASE( "JAU DArray Test 00.00 - basics", "[datatype][jau][darray]" ) {
    int_test<normal_ints>();
    int_test<secure_ints>();
}

