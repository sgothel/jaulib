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
#include <type_traits>

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
 * Test jau:cow_[ro|rw]_iterator special properties from jau::cow_darray and jau::cow_vector in detail.
 * <p>
 * Normal random-access iterator operations are also tested from std::vector, jau::darray, jau::cow_darray and jau::cow_vector in detail,
 * which either use the std::iterator (1s two container) or jau:cow_[ro|rw]_iterator (latter two cow-container).
 * </p>
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

/****************************************************************************************
 ****************************************************************************************/

template< class Iter >
static void print_iterator_info(const std::string& typedefname,
        typename std::enable_if<
                std::is_class<Iter>::value
            >::type* = 0
) {
    jau::type_cue<Iter>::print(typedefname);
    jau::type_cue<typename Iter::iterator_category>::print(typedefname+"::iterator_category");
    jau::type_cue<typename Iter::iterator_type>::print(typedefname+"::iterator_type");
    jau::type_cue<typename Iter::value_type>::print(typedefname+"::value_type");
    jau::type_cue<typename Iter::reference>::print(typedefname+"::reference");
    jau::type_cue<typename Iter::pointer>::print(typedefname+"::pointer");
}

template<class Iter>
static void print_iterator_info(const std::string& typedefname,
        typename std::enable_if<
                !std::is_class<Iter>::value
            >::type* = 0
) {
    jau::type_cue<Iter>::print(typedefname);
}

template<class T>
static bool test_00_inspect_iterator_types(const std::string& type_id) {
    typedef typename T::size_type T_size_t;
    typedef typename T::difference_type T_difference_t;

    printf("**** Type Info: %s\n", type_id.c_str());
    jau::type_cue<T>::print("T");
    jau::type_cue<typename T::value_type>::print("T::value_type");
    jau::type_cue<T_size_t>::print("T::size_type");
    jau::type_cue<T_difference_t>::print("T::difference_type");
    jau::type_cue<typename T::reference>::print("T::reference");
    jau::type_cue<typename T::pointer>::print("T::pointer");
    print_iterator_info<typename T::iterator>("T::iterator");
    print_iterator_info<typename T::const_iterator>("T::const_iterator");
    printf("\n\n");

    return true;
}

/****************************************************************************************
 ****************************************************************************************/

template<class T, typename iterator_type>
static void test_iterator_equal(iterator_type& citer1, iterator_type& citer2)
{
    // Adding redundant switched operands comparison
    // to test all relational combination of the overloading. (Leave it as is)

    REQUIRE(     citer1 ==  citer2   );  // iter op==(iter1, iter2)
    REQUIRE(     citer2 ==  citer1   );  // iter op==(iter1, iter2)
    REQUIRE( !(  citer1 !=  citer2 ) );  // iter op!=(iter1, iter2)
    REQUIRE( !(  citer2 !=  citer1 ) );  // iter op!=(iter1, iter2)
    REQUIRE(    *citer1 == *citer2   );  // iter op*() and value_type ==
    REQUIRE(    *citer2 == *citer1   );  // iter op*() and value_type ==
    REQUIRE( !( *citer1 != *citer2 ) );  // iter op*() and value_type !=
    REQUIRE( !( *citer2 != *citer1 ) );  // iter op*() and value_type !=
}
template<class T, typename iterator_type>
static void test_iterator_notequal(iterator_type& citer1, iterator_type& citer2)
{
    // Adding redundant switched operands comparison
    // to test all relational combination of the overloading. (Leave it as is)

    REQUIRE(     citer1 !=  citer2   );  // iter op==(iter1, iter2)
    REQUIRE(     citer2 !=  citer1   );  // iter op==(iter1, iter2)
    REQUIRE( !(  citer1 ==  citer2 ) );  // iter op!=(iter1, iter2)
    REQUIRE( !(  citer2 ==  citer1 ) );  // iter op!=(iter1, iter2)
    REQUIRE(    *citer1 != *citer2   );  // iter op*() and value_type ==
    REQUIRE(    *citer2 != *citer1   );  // iter op*() and value_type ==
    REQUIRE( !( *citer1 == *citer2 ) );  // iter op*() and value_type !=
    REQUIRE( !( *citer2 == *citer1 ) );  // iter op*() and value_type !=
}

template<class T, typename iterator_type>
static void test_iterator_compare(const typename T::size_type size,
                                  iterator_type& begin,
                                  iterator_type& end,
                                  iterator_type& citer1, iterator_type& citer2,
                                  const typename T::difference_type citer1_idx,
                                  const typename T::difference_type citer2_idx)
{
    typename T::difference_type d_size = static_cast<typename T::difference_type>(size);
    typename T::difference_type distance = citer2_idx - citer1_idx;
    // iter op-(iter1, iter2)
    REQUIRE( ( end    -   begin ) == d_size);
    REQUIRE( ( citer2 -   begin ) == citer2_idx);
    REQUIRE( ( citer1 -   begin ) == citer1_idx);
    REQUIRE( ( end    -  citer1 ) == d_size - citer1_idx);
    REQUIRE( ( end    -  citer2 ) == d_size - citer2_idx);
    REQUIRE( ( citer2 -  citer1 ) == distance);

    // iter op-(iter, difference_type) and iter op==(iter1, iter2)
    REQUIRE( ( citer1 -   citer1_idx ) == begin);
    REQUIRE( ( citer2 -   citer2_idx ) == begin);
    REQUIRE( ( citer2 -   distance )   == citer1);

    // iter op+(iter, difference_type) and iter op==(iter1, iter2)
    {
        typename T::difference_type d_citer1_end = end - citer1;
        typename T::difference_type d_citer2_end = end - citer2;
        REQUIRE( ( citer1_idx + d_citer1_end ) == d_size); // validate op-(iter1, iter2)
        REQUIRE( ( citer2_idx + d_citer2_end ) == d_size); // validate op-(iter1, iter2)

        REQUIRE( ( citer1 + d_citer1_end ) == end);
        REQUIRE( ( citer2 + d_citer2_end ) == end);
    }

    // Adding redundant switched operands comparison
    // to test all relational combination of the overloading. (Leave it as is)

    if( 0 == distance ) {
        test_iterator_equal<T, iterator_type>(citer1, citer2);
        REQUIRE( !( citer2 >   citer1 ) );      // iter op>(iter1, iter2)
        REQUIRE(    citer2 >=  citer1   );      // iter op>=(iter1, iter2)
        REQUIRE( !( citer2 <   citer1 ) );      // iter op<(iter1, iter2)
        REQUIRE(    citer2 <=  citer1   );      // iter op<=(iter1, iter2)
        REQUIRE(    citer1 <=  citer2   );      // iter op>=(iter1, iter2)
        REQUIRE(    citer1 >=  citer2   );      // iter op>=(iter1, iter2)
    } else if( distance > 0 ) { // citer2 > citer1
        test_iterator_notequal<T, iterator_type>(citer1, citer2);
        REQUIRE(    citer2 >   citer1   );      // iter op>(iter1, iter2)
        REQUIRE(    citer2 >=  citer1   );      // iter op>=(iter1, iter2)
        REQUIRE( !( citer2 <   citer1 ) );      // iter op<(iter1, iter2)
        REQUIRE( !( citer2 <=  citer1 ) );      // iter op<=(iter1, iter2)
        REQUIRE(    citer1 <=  citer2   );      // iter op>(iter1, iter2)
        REQUIRE(    citer1 <   citer2   );      // iter op>=(iter1, iter2)
    } else { // distance < 0: citer2 < citer1
        test_iterator_notequal<T, iterator_type>(citer1, citer2);
        REQUIRE( !( citer2 >   citer1 ) );      // iter op>(iter1, iter2)
        REQUIRE( !( citer2 >=  citer1 ) );      // iter op>=(iter1, iter2)
        REQUIRE(    citer2 <   citer1   );      // iter op<(iter1, iter2)
        REQUIRE(    citer2 <=  citer1   );      // iter op<=(iter1, iter2)
        REQUIRE(    citer1 >   citer2   );      // iter op<(iter1, iter2)
        REQUIRE(    citer1 >=  citer2   );      // iter op<=(iter1, iter2)
    }
}

template<class T, typename iterator_type>
static void test_iterator_dereference(const typename T::size_type size,
                                      iterator_type& begin, iterator_type& end)
{
    printf("**** test_iterator_dereference:\n");
    print_iterator_info<iterator_type>("iterator_type");

    // dereferencing, pointer, equality
    iterator_type citer1 = begin;
    iterator_type citer2 = begin;

    REQUIRE(    citer1 ==  begin         );  // iter op==()
    REQUIRE(    citer2 ==  begin         );  // iter op==()
    REQUIRE(    citer1 ==  citer1        );  // iter op==()
    REQUIRE(    citer2 ==  citer1        );  // iter op==()

    REQUIRE(   *citer1 == *begin         );  // iter op*(), and value_type ==
    REQUIRE(   *citer2 == *begin         );  // iter op*(), and value_type ==
    REQUIRE(   *citer1 == *citer1        );  // iter op*(), and value_type ==
    REQUIRE(   *citer2 == *citer1        );  // iter op*(), and value_type ==

    REQUIRE(    citer1[1] == *(begin+1)  );  // iter op[](diff), op+(iter, diff), iter op*(), and value_type ==
    REQUIRE(    citer2[1] == *(begin+1)  );  // iter op[](diff), op+(iter, diff), iter op*(), and value_type ==
    REQUIRE(    citer1[1] == *(citer2+1) );  // iter op[](diff), op+(iter, diff), iter op*(), and value_type ==

    REQUIRE(    citer1    !=  end-1      );  // iter op!=()
    REQUIRE(    citer2    !=  end-1      );  // iter op!=()
    REQUIRE(   *citer1    != *(end-1)    );  // iter op*(), and value_type ==
    REQUIRE(   *citer2    != *(end-1)    );  // iter op*(), and value_type ==
    REQUIRE(    citer1[1] != *(end-2)    );  // iter op[](diff), op+(iter, diff), iter op*(), and value_type ==
    REQUIRE(    citer2[1] != *(end-2)    );  // iter op[](diff), op+(iter, diff), iter op*(), and value_type ==

    REQUIRE(   citer2+size-1  ==   end -1  );
    REQUIRE( *(citer2+size-1) == *(end -1) );
    REQUIRE(   citer2[size-1] ==   end[-1] );

    REQUIRE( (citer2+0)->toString() == begin[0].toString() );
    REQUIRE( (citer2+1)->toString() == begin[1].toString() );
    REQUIRE( (citer2+2)->toString() == begin[2].toString() );
    REQUIRE( (citer2+3)->toString() == begin[3].toString() );
    REQUIRE( (citer2+size-1)->toString() == (end-1)->toString() );

#if 0
    printf("[0]: %s == %s\n", (citer2+0)->toString().c_str(), begin[3].toString().c_str());
    printf("[1]: %s == %s\n", (citer2+1)->toString().c_str(), begin[3].toString().c_str());
    printf("[2]: %s == %s\n", (citer2+2)->toString().c_str(), begin[3].toString().c_str());
    printf("[3]: %s == %s\n", (citer2+3)->toString().c_str(), begin[3].toString().c_str());
    printf("[E]: %s == %s\n", (citer2+size-1)->toString().c_str(), (end-1)->toString().c_str());
#endif

    test_iterator_compare<T, iterator_type>(size, begin, end, citer1, citer2, 0, 0);
}

template<class T, typename iterator_type>
static void test_iterator_arithmetic(const typename T::size_type size,
                                     iterator_type& begin, iterator_type& end)
{
    printf("**** test_iterator_arithmetic:\n");
    print_iterator_info<iterator_type>("iterator_type");

    // const_iterator operations
    // op++(), op--(), op++(int), op--(int),
    // op+=(difference_type), op+(iter a, difference_type) ..
    {
        iterator_type citer1 = begin;
        iterator_type citer2 = begin;
        test_iterator_compare<T, iterator_type>(size, begin, end, citer1, citer2, 0, 0);

        // iter op++(int)
        citer2++;
        test_iterator_compare<T, iterator_type>(size, begin, end, citer1, citer2, 0, 1);

        // iter op++(int)
        citer1++;
        test_iterator_compare<T, iterator_type>(size, begin, end, citer1, citer2, 1, 1);

        // iter op--(int)
        citer2--;
        test_iterator_compare<T, iterator_type>(size, begin, end, citer1, citer2, 1, 0);

        // iter op--(int)
        citer1--;
        test_iterator_compare<T, iterator_type>(size, begin, end, citer1, citer2, 0, 0);
        REQUIRE( citer2->toString() == begin[0].toString() );
        // printf("[0]: %s == %s\n", citer2->toString().c_str(), begin[0].toString().c_str());

        // iter op++(int)
        citer2++;
        test_iterator_compare<T, iterator_type>(size, begin, end, citer1, citer2, 0, 1);
        REQUIRE(   *citer2 == *(begin+1)    );  // iter op*(), op+(iter, difference_type) and value_type ==
        REQUIRE(   *citer2 == begin[1]      );  // iter op*(), op[](difference_type) and value_type ==
        REQUIRE( citer2->toString() == begin[1].toString() );

        // iter op++(int)
        citer2++;
        test_iterator_compare<T, iterator_type>(size, begin, end, citer1, citer2, 0, 2);
        REQUIRE(   *citer2 == *(begin+2)    );  // iter op*(), op+(iter, difference_type) and value_type ==
        REQUIRE(   *citer2 == begin[2]      );  // iter op*(), op[](difference_type) and value_type ==
        REQUIRE( citer2->toString() == begin[2].toString() );

        // iter op++(int)
        citer2++;
        test_iterator_compare<T, iterator_type>(size, begin, end, citer1, citer2, 0, 3);
        REQUIRE(   *citer2 == *(begin+3)    );  // iter op*(), op+(iter, difference_type) and value_type ==
        REQUIRE(   *citer2 == begin[3]      );  // iter op*(), op[](difference_type) and value_type ==
        REQUIRE( citer2->toString() == begin[3].toString() );
        // printf("[3]: %s == %s\n", citer2->toString().c_str(), begin[3].toString().c_str());

        // iter op++()
        --citer2;
        --citer2;
        --citer2;
        test_iterator_compare<T, iterator_type>(size, begin, end, citer1, citer2, 0, 0);
        REQUIRE(   *citer2 == *(begin+0)    );  // iter op*(), op+(iter, difference_type) and value_type ==
        REQUIRE(   *citer2 == begin[0]      );    // iter op*(), op[](difference_type) and value_type ==
        REQUIRE( citer2->toString() == begin[0].toString() );
        // printf("[3]: %s == %s\n", citer2->toString().c_str(), begin[3].toString().c_str());

        // iter +=(diff)
        citer2 += 3;
        test_iterator_compare<T, iterator_type>(size, begin, end, citer1, citer2, 0, 3);

        // iter +=(diff)
        citer2 += 7;
        test_iterator_compare<T, iterator_type>(size, begin, end, citer1, citer2, 0, 10);

        // iter -=(diff)
        citer2 -= 10;
        test_iterator_compare<T, iterator_type>(size, begin, end, citer1, citer2, 0, 0);
    }
    {
        // Adding redundant switched operands comparison
        // to test all relational combination of the overloading. (Leave it as is)

        iterator_type citer1 = begin;
        iterator_type citer2 = begin;

        REQUIRE(    citer1 == citer1        );  // iter op==()
        REQUIRE(    citer2 == citer1        );  // iter op==()

        ++citer2;
        REQUIRE(    citer2 != citer1        );  // iter op==()
        REQUIRE(    citer1 != citer2        );  // iter op==()
        REQUIRE(    citer2 >  citer1        );  // iter op==()
        REQUIRE(    citer2 >= citer1        );  // iter op==()
        REQUIRE(    citer1 <  citer2        );  // iter op==()
        REQUIRE(    citer1 <= citer2        );  // iter op==()
        REQUIRE( (  citer2 -  citer1 ) ==  1);  // iter op==()
        REQUIRE( (  citer1 -  citer2 ) == -1);  // iter op==()
    }

}

template<class T>
static bool test_const_iterator_ops(const std::string& type_id, T& data,
                std::enable_if_t< is_cow_type<T>::value, bool> = true )
{
    printf("**** test_const_iterator_ops(CoW): %s\n", type_id.c_str());
    {
        typename T::const_iterator begin = data.cbegin(); // immutable new_store non-const iterator, gets held until destruction
        typename T::const_iterator end = begin.end();    // no new store iterator, on same store as begin, obtained from begin
        typename T::difference_type data_size = static_cast<typename T::difference_type>(data.size());
        typename T::difference_type begin_size = static_cast<typename T::difference_type>(begin.size());
        typename T::difference_type end_size = static_cast<typename T::difference_type>(end.size());
        REQUIRE( begin_size                 == data_size );
        REQUIRE( end_size                   == data_size );
        REQUIRE( end  - begin               == data_size );
        REQUIRE( end - end_size             == begin     );
        REQUIRE( begin + begin_size         == end       );
        REQUIRE( *( end - end_size )        == *begin    );
        REQUIRE( *( begin + begin_size )    == *end      );
        test_iterator_dereference<T, typename T::const_iterator>(begin.size(), begin, end);
    }

    {
        typename T::const_iterator begin = data.cbegin(); // no new store const_iterator
        typename T::const_iterator end = begin.end();     // no new store const_iterator, on same store as begin, obtained from begin
        test_iterator_arithmetic<T, typename T::const_iterator>(data.size(), begin, end);
    }
#if 0
    {
        // INTENIONAL FAILURES, checking behavior of error values etc
        typename T::const_iterator begin = data.cbegin(); // no new store const_iterator
        typename T::const_iterator iter = begin + 1;
        CHECK( *begin == *iter );
        CHECK( begin == iter );
    }
#endif
    return true;
}
template<class T>
static bool test_const_iterator_ops(const std::string& type_id, T& data,
        std::enable_if_t< !is_cow_type<T>::value, bool> = true )
{
    printf("**** test_const_iterator_ops: %s\n", type_id.c_str());
    {
        typename T::const_iterator begin = data.cbegin(); // mutable new_store non-const iterator, gets held until destruction
        typename T::const_iterator end = data.cend();    // no new store iterator, on same store as begin and from begin
        typename T::difference_type data_size = static_cast<typename T::difference_type>(data.size());
        REQUIRE( end  - begin               == data_size );
        REQUIRE( end - data_size            == begin     );
        REQUIRE( begin + data_size          == end       );
        REQUIRE( *( end - data_size )       == *begin    );
        REQUIRE( *( begin + data_size - 1 ) == *( end - 1 ) );
        REQUIRE( end[-data_size]            == begin[0]    );
        REQUIRE( begin[data_size - 1]       == end[-1]     );
        test_iterator_dereference<T, typename T::const_iterator>(data.size(), begin, end);
    }

    {
        typename T::const_iterator begin = data.cbegin(); // no new store const_iterator
        typename T::const_iterator end = data.cend();     // no new store const_iterator, on same store as begin
        test_iterator_arithmetic<T, typename T::const_iterator>(data.size(), begin, end);
    }
#if 0
    {
        // INTENIONAL FAILURES, checking behavior of error values etc
        typename T::const_iterator begin = data.cbegin(); // no new store const_iterator
        typename T::const_iterator iter = begin + 1;
        CHECK( *begin == *iter );
        CHECK( begin == iter );
    }
#endif
    return true;
}

template<class T>
static bool test_mutable_iterator_ops(const std::string& type_id, T& data,
        std::enable_if_t< is_cow_type<T>::value, bool> = true )
{
    printf("**** test_mutable_iterator_ops(CoW): %s\n", type_id.c_str());
    {
        typename T::iterator begin = data.begin(); // mutable new_store non-const iterator, gets held until destruction
        typename T::iterator end = begin.end();    // no new store iterator, on same store as begin and from begin
        typename T::difference_type data_size = static_cast<typename T::difference_type>(data.size());
        typename T::difference_type begin_size = static_cast<typename T::difference_type>(begin.size());
        typename T::difference_type end_size = static_cast<typename T::difference_type>(end.size());
        REQUIRE( begin_size                 == data_size );
        REQUIRE( end_size                   == data_size );
        REQUIRE( end  - begin               == data_size );
        REQUIRE( end - end_size             == begin     );
        REQUIRE( begin + begin_size         == end       );
        REQUIRE( *( end - end_size )        == *begin    );
        REQUIRE( *( begin + begin_size - 1 ) == *( end - 1 ) );
        REQUIRE( end[-end_size]              == begin[0]    );
        REQUIRE( begin[begin_size - 1]       == end[-1]     );
        test_iterator_dereference<T, typename T::iterator>(begin.size(), begin, end);
    }

    {
        typename T::iterator begin = data.begin();      // mutable new_store non-const iterator, gets held until destruction
        typename T::iterator end = begin.end();
        test_iterator_arithmetic<T, typename T::iterator>(data.size(), begin, end);
    }

    // iterator-op: darray/vector-op
    // -------------------------------------------
    // 1 pop_back()
    // 1 erase ():      erase (const_iterator pos)
    // 3 erase (count): erase (iterator first, const_iterator last)
    // 1 insert(const value_type& x): iterator insert(const_iterator pos, const value_type& x)
    // 0 insert(value_type&& x): iterator insert(const_iterator pos, value_type&& x)
    // 1 emplace(Args&&... args): emplace(const_iterator pos, Args&&... args)
    // 2 insert(InputIt first, InputIt last ): insert( const_iterator pos, InputIt first, InputIt last )
    // 1 void push_back(value_type& x)
    // 1 void push_back(value_type&& x)
    // 1 reference emplace_back(Args&&... args)
    // 0 [void push_back( InputIt first, InputIt last )]
    // 1 rewind()
    {
        typename T::iterator iter             = data.begin(); // mutable new_store non-const iterator, gets held until destruction
        typename T::iterator::size_type size_pre = iter.size();
        typename T::iterator::value_type elem  = iter.end()[-2];

        // pop_back()
        int i;
        (void)i;
        iter.pop_back();
        REQUIRE( iter.size() == size_pre-1 );
        REQUIRE( iter == iter.end() );
        REQUIRE( iter == iter.begin()+size_pre-1 );
        REQUIRE( iter[-1] == elem );

        // insert( const_iterator pos, InputIt first, InputIt last )
        REQUIRE( iter == iter.end() );
        size_pre = iter.size();
        jau_darray_DataType01 data2;
        test_00_seq_fill(data2, 10);
        // iter.push_back(data2.cbegin(), data2.cend()); // FIXME: Only in jau::darray not stl::vector
        iter.insert(data2.cbegin(), data2.cend()); // same as push_pack(..) since pointing to end() - but iter points here to first new elem
        REQUIRE( iter.size() == size_pre+10 );
        REQUIRE( iter == iter.end()-10 );

        // erase (count): erase (iterator first, const_iterator last)
        REQUIRE( iter == iter.end()-10 );
        size_pre = iter.size();
        // std::cout << "iter.5" << iter << (iter - iter.begin()) << "/" << iter.size() << ", size2 " << size2 << std::endl;
        iter.erase(10);
        // std::cout << "iter.6" << iter << (iter - iter.begin()) << "/" << iter.size() << std::endl;
        REQUIRE( iter.size() == size_pre-10 );
        REQUIRE( iter == iter.end() );

        // erase ()
        size_pre = iter.size();
        iter.rewind();
        REQUIRE( iter == iter.begin() );
        elem  = iter.begin()[1];
        iter.erase();
        REQUIRE( iter.size() == size_pre-1 );
        REQUIRE( iter == iter.begin() );
        REQUIRE( *iter == elem );

        // void push_back(value_type& x)
        size_pre = iter.size();
        REQUIRE( iter == iter.begin() );
        elem  = iter.end()[-1];
        iter.push_back(data2[0]);
        iter.push_back(data2[1]);
        iter.push_back(data2[2]);
        REQUIRE( iter.size() == size_pre+3 );
        REQUIRE( iter == iter.end() );
        REQUIRE( iter[-3] == data2[0] );
        REQUIRE( iter[-2] == data2[1] );
        REQUIRE( iter[-1] == data2[2] );

        // erase (count): erase (iterator first, const_iterator last)
        size_pre = iter.size();
        REQUIRE( iter == iter.end() );
        iter -= 3;
        iter.erase(3);
        REQUIRE( iter.size() == size_pre-3 );
        REQUIRE( iter == iter.end() );

        // void push_back(value_type&& x)
        size_pre = iter.size();
        REQUIRE( iter == iter.end() );
        {
            typename T::iterator::value_type elem0 = iter.begin()[0];
            iter.push_back( std::move(elem0));
        }
        {
            typename T::iterator::value_type elem0 = iter.begin()[1];
            iter.push_back( std::move(elem0));
        }
        {
            typename T::iterator::value_type elem0 = iter.begin()[2];
            iter.push_back( std::move(elem0));
        }
        REQUIRE( iter.size() == size_pre+3 );
        REQUIRE( iter == iter.end() );
        REQUIRE( iter[-3] == iter.begin()[0] );
        REQUIRE( iter[-2] == iter.begin()[1] );
        REQUIRE( iter[-1] == iter.begin()[2] );

        // iterator insert(const_iterator pos, const value_type& x)
        iter.rewind();
        iter += 20;
        REQUIRE( iter == iter.begin()+20 );
        size_pre = iter.size();
        iter.insert(data2[0]);
        iter.insert(data2[1]);
        iter.insert(data2[2]);
        // i=0; jau::for_each(iter.begin(), iter.end(), [&](const typename T::iterator::value_type & e) { printf("data[%d]: %s\n", i++, e.toString().c_str()); } );
        REQUIRE( iter.size() == size_pre+3 );
        REQUIRE( iter == iter.begin()+20 );
        iter.rewind();
        REQUIRE( iter[20] == data2[2] );
        REQUIRE( iter[21] == data2[1] );
        REQUIRE( iter[22] == data2[0] );

        // insert( const_iterator pos, InputIt first, InputIt last )
        iter += 20;
        REQUIRE( iter == iter.begin()+20 );
        size_pre = iter.size();
        iter.insert(data2.cbegin(), data2.cbegin()+11);
        REQUIRE( iter.size() == size_pre+11 );
        REQUIRE( iter == iter.begin()+20 );

        // erase (count): erase (iterator first, const_iterator last)
        REQUIRE( iter == iter.begin()+20 );
        size_pre = iter.size();
        iter -= 10;
        REQUIRE( iter == iter.begin()+10 );
        iter.erase(11);
        REQUIRE( iter.size() == size_pre-11 );
        REQUIRE( iter == iter.begin()+10 );

        // 1 emplace(Args&&... args): emplace(const_iterator pos, Args&&... args)
        Addr48Bit a0(start_addr);
        size_pre = iter.size();
        REQUIRE( iter == iter.begin()+10 );
        iter.emplace( a0, static_cast<uint8_t>(2) );
        a0.next();
        iter.emplace( a0, static_cast<uint8_t>(3) );
        a0.next();
        iter.emplace( a0, static_cast<uint8_t>(4) );
        a0.next();
        REQUIRE( iter == iter.begin()+10 );
        REQUIRE( iter[0].type == 4 );
        REQUIRE( iter[1].type == 3 );
        REQUIRE( iter[2].type == 2 );

        // 1 reference emplace_back(Args&&... args)
        size_pre = iter.size();
        REQUIRE( iter == iter.begin()+10 );
        iter.emplace_back( a0, static_cast<uint8_t>(2) );
        a0.next();
        iter.emplace_back( a0, static_cast<uint8_t>(3) );
        a0.next();
        iter.emplace_back( a0, static_cast<uint8_t>(4) );
        a0.next();
        REQUIRE( iter == iter.end() );
        REQUIRE( iter[-1].type == 4 );
        REQUIRE( iter[-2].type == 3 );
        REQUIRE( iter[-3].type == 2 );

        // multiple erase()
        size_pre = iter.size();
        REQUIRE( iter == iter.end() );
        iter -= 15;
        REQUIRE( iter == iter.end()-15 );
        {
            int count = 0;
            while( iter != iter.end() ) {
                iter.erase();
                count++;
            }
            REQUIRE( iter.size() == size_pre - 15 );
            REQUIRE( iter == iter.end() );
        }
    }

    return true;
}

template<class T>
static bool test_mutable_iterator_ops(const std::string& type_id, T& data,
        std::enable_if_t< !is_cow_type<T>::value, bool> = true )
{
    printf("**** test_mutable_iterator_ops(___): %s\n", type_id.c_str());
    {
        typename T::iterator begin = data.begin();
        typename T::iterator end = data.end();
        typename T::difference_type data_size = static_cast<typename T::difference_type>(data.size());
        REQUIRE( end  - begin               == data_size );
        REQUIRE( end - data_size            == begin     );
        REQUIRE( begin + data_size          == end       );
        REQUIRE( *( end - data_size )       == *begin    );
        REQUIRE( *( begin + data_size )     == *end      );
        test_iterator_dereference<T, typename T::iterator>(data.size(), begin, end);
    }

    {
        typename T::iterator begin = data.begin();
        typename T::iterator end = data.end();
        test_iterator_arithmetic<T, typename T::iterator>(data.size(), begin, end);
    }

    // iterator-op: darray/vector-op
    // -------------------------------------------
    // 1 pop_back()
    // 1 erase (const_iterator pos)
    // 3 erase (iterator first, const_iterator last)
    // 1 iterator insert(const_iterator pos, const value_type& x)
    // 0 iterator insert(const_iterator pos, value_type&& x)
    // 1 emplace(const_iterator pos, Args&&... args)
    // 2 insert( const_iterator pos, InputIt first, InputIt last )
    // 1 void push_back(value_type& x)
    // 1 void push_back(value_type&& x)
    // 1 reference emplace_back(Args&&... args)
    // 0 [void push_back( InputIt first, InputIt last )]
    // 1 rewind()
    {
        typename T::iterator iter             = data.end();
        typename T::size_type size_pre        = data.size();
        typename T::value_type          elem  = iter[-2];

        // pop_back()
        int i;
        (void)i;
        data.pop_back();
        iter--;
        REQUIRE( data.size() == size_pre-1 );
        REQUIRE( iter == data.end() );
        REQUIRE( iter == data.begin()+size_pre-1 );
        REQUIRE( iter[-1] == elem );

        // insert( const_iterator pos, InputIt first, InputIt last )
        REQUIRE( iter == data.end() );
        size_pre = data.size();
        jau_darray_DataType01 data2;
        test_00_seq_fill(data2, 10);
        // data.push_back(data2.cbegin(), data2.cend()); // FIXME: Only in jau::darray not stl::vector
        iter = data.insert(iter, data2.cbegin(), data2.cend()); // same as push_pack(..) since pointing to end() - but data points here to first new elem
        REQUIRE( data.size() == size_pre+10 );
        REQUIRE( iter == data.end()-10 );

        // erase (count): erase (iterator first, const_iterator last)
        REQUIRE( iter == data.end()-10 );
        size_pre = data.size();
        // std::cout << "data.5" << data << (data - data.begin()) << "/" << data.size() << ", size2 " << size2 << std::endl;
        iter = data.erase(iter, iter+10);
        // std::cout << "data.6" << data << (data - data.begin()) << "/" << data.size() << std::endl;
        REQUIRE( data.size() == size_pre-10 );
        REQUIRE( iter == data.end() );

        // erase ()
        size_pre = data.size();
        iter = data.begin();
        REQUIRE( iter == data.begin() );
        elem  = iter[1];
        // i=0; jau::for_each(data.begin(), data.begin()+3, [&](const typename T::value_type & e) { printf("data[%d]: %s\n", i++, e.toString().c_str()); } );
        iter = data.erase(iter);
        // i=0; jau::for_each(data.begin(), data.begin()+3, [&](const typename T::value_type & e) { printf("data[%d]: %s\n", i++, e.toString().c_str()); } );
        REQUIRE( data.size() == size_pre-1 );
        REQUIRE( iter == data.begin() );
        REQUIRE( *iter == elem );

        // void push_back(value_type& x)
        size_pre = data.size();
        REQUIRE( iter == data.begin() );
        elem  = data.end()[-1];
        data.push_back(data2[0]);
        data.push_back(data2[1]);
        data.push_back(data2[2]);
        iter = data.end();
        REQUIRE( data.size() == size_pre+3 );
        REQUIRE( iter == data.end() );
        REQUIRE( iter[-3] == data2[0] );
        REQUIRE( iter[-2] == data2[1] );
        REQUIRE( iter[-1] == data2[2] );

        // erase (count): erase (iterator first, const_iterator last)
        size_pre = data.size();
        REQUIRE( iter == data.end() );
        iter -= 3;
        iter = data.erase(iter, iter+3);
        REQUIRE( data.size() == size_pre-3 );
        REQUIRE( iter == data.end() );

        // void push_back(value_type&& x)
        size_pre = data.size();
        REQUIRE( iter == data.end() );
        {
            typename T::value_type elem0 = data.begin()[0];
            data.push_back( std::move(elem0));
        }
        {
            typename T::value_type elem0 = data.begin()[1];
            data.push_back( std::move(elem0));
        }
        {
            typename T::value_type elem0 = data.begin()[2];
            data.push_back( std::move(elem0));
        }
        iter = data.end();
        REQUIRE( data.size() == size_pre+3 );
        REQUIRE( iter == data.end() );
        REQUIRE( iter[-3] == data.begin()[0] );
        REQUIRE( iter[-2] == data.begin()[1] );
        REQUIRE( iter[-1] == data.begin()[2] );

        // iterator insert(const_iterator pos, const value_type& x)
        iter = data.begin();
        iter += 20;
        REQUIRE( iter == data.begin()+20 );
        size_pre = data.size();
        iter = data.insert(iter, data2[0]);
        iter = data.insert(iter, data2[1]);
        iter = data.insert(iter, data2[2]);
        REQUIRE( data.size() == size_pre+3 );
        REQUIRE( iter == data.begin()+20 );
        iter = data.begin();
        REQUIRE( iter[20] == data2[2] );
        REQUIRE( iter[21] == data2[1] );
        REQUIRE( iter[22] == data2[0] );

        // insert( const_iterator pos, InputIt first, InputIt last )
        iter += 20;
        REQUIRE( iter == data.begin()+20 );
        size_pre = data.size();
        iter = data.insert(iter, data2.cbegin(), data2.cbegin()+11);
        REQUIRE( data.size() == size_pre+11 );
        REQUIRE( iter == data.begin()+20 );

        // erase (count): erase (iterator first, const_iterator last)
        REQUIRE( iter == data.begin()+20 );
        size_pre = data.size();
        iter -= 10;
        REQUIRE( iter == data.begin()+10 );
        iter = data.erase(iter, iter+11);
        REQUIRE( data.size() == size_pre-11 );
        REQUIRE( iter == data.begin()+10 );

        // 1 emplace(Args&&... args): emplace(const_iterator pos, Args&&... args)
        Addr48Bit a0(start_addr);
        size_pre = data.size();
        REQUIRE( iter == data.begin()+10 );
        iter = data.emplace(iter, a0, static_cast<uint8_t>(2) );
        a0.next();
        iter = data.emplace(iter, a0, static_cast<uint8_t>(3) );
        a0.next();
        iter = data.emplace(iter, a0, static_cast<uint8_t>(4) );
        a0.next();
        REQUIRE( iter == data.begin()+10 );
        REQUIRE( iter[0].type == 4 );
        REQUIRE( iter[1].type == 3 );
        REQUIRE( iter[2].type == 2 );

        // 1 reference emplace_back(Args&&... args)
        size_pre = data.size();
        REQUIRE( iter == data.begin()+10 );
        data.emplace_back( a0, static_cast<uint8_t>(2) );
        a0.next();
        data.emplace_back( a0, static_cast<uint8_t>(3) );
        a0.next();
        data.emplace_back( a0, static_cast<uint8_t>(4) );
        a0.next();
        iter = data.end();
        REQUIRE( iter == data.end() );
        REQUIRE( iter[-1].type == 4 );
        REQUIRE( iter[-2].type == 3 );
        REQUIRE( iter[-3].type == 2 );

        // multiple erase()
        size_pre = data.size();
        REQUIRE( iter == data.end() );
        iter -= 15;
        REQUIRE( iter == data.end()-15 );
        {
            int count = 0;
            while( iter != data.end() ) {
                iter = data.erase(iter);
                count++;
            }
            REQUIRE( data.size() == size_pre - 15 );
            REQUIRE( iter == data.end() );
        }
    }
    return true;
}

/****************************************************************************************
 ****************************************************************************************/

template<class T>
static bool test_01_validate_iterator_ops(const std::string& type_id) {
    const std::size_t size0 = 100;

    T data;
    REQUIRE(0 == data.get_allocator().memory_usage);
    REQUIRE(data.size() == 0);
    REQUIRE(data.capacity() == 0);
    REQUIRE(data.empty() == true);

    test_00_seq_fill(data, size0);
    REQUIRE(0 != data.get_allocator().memory_usage);
    REQUIRE(data.size() == size0);
    REQUIRE(data.size() <= data.capacity());

    test_00_list_itr(data, false);
    REQUIRE(0 != data.get_allocator().memory_usage);
    REQUIRE(data.size() == size0);

    test_const_iterator_ops<T>(type_id, data);

    test_mutable_iterator_ops<T>(type_id, data);

    data.clear();
    REQUIRE(data.size() == 0);
    // REQUIRE(0 == data.get_allocator().memory_usage);
    return data.size() == 0;
}

template<class T>
static bool test_01_cow_iterator_properties(const std::string& type_id) {
    typedef typename T::const_iterator const_iterator;
    typedef typename T::iterator       write_iterator;

    printf("**** test_cow_iterator_properties: %s\n", type_id.c_str());
    print_iterator_info<const_iterator>("const_iterator");
    print_iterator_info<write_iterator>("write_iterator");

    const std::size_t size0 = 100;

    T data;
    REQUIRE(0 == data.get_allocator().memory_usage);
    REQUIRE(data.size() == 0);
    REQUIRE(data.capacity() == 0);
    REQUIRE(data.empty() == true);

    test_00_seq_fill(data, size0);
    REQUIRE(0 != data.get_allocator().memory_usage);
    REQUIRE(data.size() == size0);
    REQUIRE(data.size() <= data.capacity());

    // test relationship and distance with mixed iterator and const_iterator
    // in both direction using the free overloaded operator of cow_ro_* and cow_rw_*
    {
        write_iterator citer1 = data.begin();
        const_iterator citer2(citer1);

        REQUIRE( (  citer1 == citer2  ) == true);  // iter op==()
        REQUIRE( (  citer2 == citer1 ) == true);  // iter op==()

        ++citer2;
        REQUIRE( (  citer2 != citer1 ) == true);  // iter op==()
        REQUIRE( (  citer1 != citer2 ) == true);  // iter op==()
        REQUIRE( (  citer2 >  citer1 ) == true);  // iter op==()
        REQUIRE( (  citer2 >= citer1 ) == true);  // iter op==()
        REQUIRE( (  citer1 <  citer2 ) == true);  // iter op==()
        REQUIRE( (  citer1 <= citer2 ) == true);  // iter op==()
        REQUIRE( (  citer2 -  citer1 ) ==  1);  // iter op==()
        REQUIRE( (  citer1 -  citer2 ) == -1);  // iter op==()

        --citer2;
        ++citer1;
        REQUIRE( (  citer1 != citer2 ) == true);  // iter op==()
        REQUIRE( (  citer2 != citer1 ) == true);  // iter op==()
        REQUIRE( (  citer1 >  citer2 ) == true);  // iter op==()
        REQUIRE( (  citer1 >= citer2 ) == true);  // iter op==()
        REQUIRE( (  citer2 <  citer1 ) == true);  // iter op==()
        REQUIRE( (  citer2 <= citer1 ) == true);  // iter op==()
        REQUIRE( (  citer1 -  citer2 ) ==  1);  // iter op==()
        REQUIRE( (  citer2 -  citer1 ) == -1);  // iter op==()

    }

    // test mutable non-const 'new store' behavior
    // inclusive 'iterator -> const_iterator' conversion if 'iterator_is_const == true'
    const_iterator c_begin0 = data.cbegin();  // orig store
    {
        // iterator_type is const_iterator.
        // The cow_rw_iterator is being fetched via data.begin(), which creates a new store.
        // The cow_rw_iterator is converted immediately to cow_ro_iterator and the cow_rw_iterator gets destructed.
        // This destruction moves the cow_rw_iterator's new store into the cow container right away.

        printf("testing mutable non-const behavior incl 'iterator -> const_iterator' conversion.\n");

        const_iterator c_begin1;
        {
            const_iterator m_begin1( data.begin() ); // mutable iterator first, converts to const_iterator and
                                                     // mutable iterator destructs (new_store -> cow)
            c_begin1 = data.cbegin();
            REQUIRE(*c_begin1 == *m_begin1);
            REQUIRE( c_begin1 ==  m_begin1);
            REQUIRE( ( c_begin1 - m_begin1 ) == 0);
            printf("       1st store: %s == %s, dist %u\n",
                    c_begin1->toString().c_str(), m_begin1->toString().c_str(), (unsigned int)(c_begin1 - m_begin1));

            REQUIRE(*c_begin1 == *c_begin0);
            REQUIRE( c_begin1 !=  c_begin0);
            REQUIRE( ( c_begin1 - c_begin0 ) != 0);
            printf("1st -> 0st store: %s == %s, dist %u != 0\n",
                    c_begin1->toString().c_str(), c_begin0->toString().c_str(), (unsigned int)(c_begin1 - c_begin0));

            const_iterator c_begin2;
            {
                const_iterator m_begin2 ( data.begin() );  // mutable iterator first, converts to const_iterator and
                                                           // mutable iterator destructs (new_store -> cow)
                c_begin2 = data.cbegin();
                REQUIRE(*c_begin2 == *m_begin2);
                REQUIRE( c_begin2 ==  m_begin2);
                REQUIRE( ( c_begin2 - m_begin2 ) == 0);
                printf("       2nd store: %s == %s, dist %u\n",
                        c_begin2->toString().c_str(), m_begin2->toString().c_str(), (unsigned int)(c_begin2 - m_begin2));

                REQUIRE(*c_begin2 == *c_begin1);
                REQUIRE( c_begin2 !=  c_begin1);
                REQUIRE( ( c_begin2 - c_begin1 ) != 0);
                printf("2nd -> 1st store: %s == %s, dist %u != 0\n",
                        c_begin1->toString().c_str(), c_begin0->toString().c_str(), (unsigned int)(c_begin1 - c_begin0));

            }
        }
    }

    {
        // iterator_type is mutable iterator.
        // The cow_rw_iterator is being fetched via data.begin(), which creates a new store.
        // The cow_rw_iterator is not converted into cow_ro_iterator.
        // The cow_rw_iterator's new store is moved into the cow container
        // when the cow_rw_iterator gets destructed later on (out of scope).

        printf("testing mutable non-const behavior.\n");
        const_iterator c_begin1;
        {
            write_iterator m_begin1 = data.begin();  // mutable new_store non-const iterator, gets held until destruction
            c_begin1 = m_begin1;                     // get immutable const_iterator from newly created store

            REQUIRE(*c_begin1 == *m_begin1);
            REQUIRE( c_begin1 ==  m_begin1);
            REQUIRE( ( c_begin1 - m_begin1 ) == 0);
            printf("       1st store: %s == %s, dist %u\n",
                    c_begin1->toString().c_str(), m_begin1->toString().c_str(), (unsigned int)(c_begin1 - m_begin1));
            const_iterator c_begin2;
            {
                write_iterator m_begin2 = data.begin();  // mutable new_store non-const iterator, gets held until destruction
                c_begin2 = m_begin2;                     // get immutable const_iterator from newly created store

                REQUIRE(*c_begin2 == *m_begin2);
                REQUIRE( c_begin2 ==  m_begin2);
                REQUIRE( ( c_begin2 - m_begin2 ) == 0);
                printf("       2nd store: %s == %s, dist %u\n",
                        c_begin2->toString().c_str(), m_begin2->toString().c_str(), (unsigned int)(c_begin2 - m_begin2));

                REQUIRE(*c_begin2 == *c_begin1);
                REQUIRE( c_begin2 !=  c_begin1);
                REQUIRE( ( c_begin2 - c_begin1 ) != 0);
                printf("2nd -> 1st store: %s == %s, dist %u\n",
                        c_begin2->toString().c_str(), c_begin1->toString().c_str(), (unsigned int)(c_begin2 - c_begin1));
            }
            // 2nd store -> cow_xxx
            const_iterator c_begin2b = data.cbegin();
            REQUIRE(*c_begin2 == *c_begin2b);
            REQUIRE( c_begin2 ==  c_begin2b);
            REQUIRE( ( c_begin2 - c_begin2b ) == 0);
            printf("2nd -> cow == cbegin: %s == %s, dist %u\n",
                    c_begin2->toString().c_str(), c_begin2b->toString().c_str(), (unsigned int)(c_begin2 - c_begin2b));
            printf("2nd -> 1st          : %s == %s, dist %u\n",
                    c_begin1->toString().c_str(), c_begin2->toString().c_str(), (unsigned int)(c_begin1 - c_begin2));
        }
        // 1st store -> cow_xxx
        typename T::const_iterator c_begin1b = data.cbegin();
        printf("1st -> cow == cbegin: %s == %s, dist %u\n",
                c_begin1->toString().c_str(), c_begin1b->toString().c_str(), (unsigned int)(c_begin1 - c_begin1b));
        REQUIRE(*c_begin1 == *c_begin1b);
        REQUIRE( c_begin1 ==  c_begin1b);
        REQUIRE( ( c_begin1 - c_begin1b ) == 0);
    }
    return true;
}

/****************************************************************************************
 ****************************************************************************************/

TEST_CASE( "Iterator Test 00 - Inspect all Iterator Types", "[datatype][std][vector][darray][cow_vector][cow_darray]" ) {
    test_00_inspect_iterator_types< std_vector_DataType01 >("std::vector<T>");
    test_00_inspect_iterator_types< jau_darray_DataType01 >("jau::darray<T>");
    test_00_inspect_iterator_types< jau_cow_vector_DataType01 >("jau::cow_vector<T>");
    test_00_inspect_iterator_types< jau_cow_darray_DataType01 >("jau::cow_darray<T>");
}

TEST_CASE( "STD Vector Test 01 - Validate Iterator and Index Operations", "[datatype][std][vector]" ) {
    test_01_validate_iterator_ops< std_vector_DataType01 >("std::vector<T>");
}

TEST_CASE( "JAU DArray Test 02 - Validate Iterator and Index Operations", "[datatype][jau][darray]" ) {
    test_01_validate_iterator_ops< jau_darray_DataType01 >("jau::darray<T>");
}

TEST_CASE( "JAU COW_Vector Test 11 - Validate Iterator Operations", "[datatype][jau][cow_vector]" ) {
    test_01_validate_iterator_ops< jau_cow_vector_DataType01 >("jau::cow_vector<T>");

    test_01_cow_iterator_properties<jau_cow_vector_DataType01>("jau::cow_vector<T>");
}

TEST_CASE( "JAU COW_DArray Test 21 - Validate Iterator Operations", "[datatype][jau][cow_darray]" ) {
    test_01_validate_iterator_ops< jau_cow_darray_DataType01 >("jau::cow_darray<T>");

    test_01_cow_iterator_properties<jau_cow_darray_DataType01>("jau::cow_darray<T>");
}
