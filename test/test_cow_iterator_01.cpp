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
#include <cstring>
#include <vector>
#include <type_traits>

#include <jau/test/catch2_ext.hpp>

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

typedef std::vector<uint64_t, counting_allocator<uint64_t>> std_vector_uint64_t;
typedef jau::darray<uint64_t, jau::nsize_t, counting_callocator<uint64_t>> jau_darray_uint64_t;
typedef jau::cow_vector<uint64_t, counting_allocator<uint64_t>> jau_cow_vector_uint64_t;
typedef jau::cow_darray<uint64_t, jau::nsize_t, counting_callocator<uint64_t>> jau_cow_darray_uint64_t;

JAU_TYPENAME_CUE_ALL(std_vector_uint64_t)
JAU_TYPENAME_CUE_ALL(jau_darray_uint64_t)
JAU_TYPENAME_CUE_ALL(jau_cow_vector_uint64_t)
JAU_TYPENAME_CUE_ALL(jau_cow_darray_uint64_t)

template<class T>
static void print_list(T& data) {
    printf("list: %d { ", (int)data.size());
    jau::for_each_const(data, [](const uint64_t & e) {
        printf("%s, ", to_decstring(e, ',', 2).c_str());
    } );
    printf("}\n");
}

template<class T>
static void print_list(const std::string& pre, T& data) {
    printf("%s: %d { ", pre.c_str(), (int)data.size());
    jau::for_each_const(data, [](const uint64_t & e) {
        printf("%s, ", to_decstring(e, ',', 2).c_str());
    } );
    printf("}\n");
}

template<class T>
static void fill_list(T& data, const std::size_t size) {
    std::size_t i=0;

    for(; i<size; i++) {
        data.emplace_back( static_cast<uint64_t>(i+1) );
    }
    REQUIRE(i == data.size());
}

/****************************************************************************************
 ****************************************************************************************/

template< class Iter >
static void print_iterator_info(const std::string& typedefname,
        typename std::enable_if_t<
                std::is_class_v<Iter>
            >* = nullptr
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
        typename std::enable_if_t<
                !std::is_class_v<Iter>
            >* = nullptr
) {
    jau::type_cue<Iter>::print(typedefname);
}

template<class T>
static bool test_00_inspect_iterator_types(const std::string& type_id) {
    typedef typename T::size_type       size_type;
    typedef typename T::iterator        iter_type;
    typedef typename T::difference_type diff_type;
    typedef typename T::const_iterator  citer_type;

    printf("**** Type Info: %s\n", type_id.c_str());
    jau::type_cue<T>::print("T");
    jau::type_cue<typename T::value_type>::print("T::value_type");
    jau::type_cue<size_type>::print("T::size_type");
    jau::type_cue<diff_type>::print("T::difference_type");
    jau::type_cue<typename T::reference>::print("T::reference");
    jau::type_cue<typename T::pointer>::print("T::pointer");
    print_iterator_info<iter_type>("T::iterator");
    print_iterator_info<citer_type>("T::citer_type");
    printf("\n\n");

    return true;
}

/****************************************************************************************
 ****************************************************************************************/

template<class T, typename iterator_type1, typename iterator_type2>
static void test_iterator_equal(iterator_type1& citer1, iterator_type2& citer2)
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
template<class T, typename iterator_type1, typename iterator_type2>
static void test_iterator_notequal(iterator_type1& citer1, iterator_type2& citer2)
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

// iterator_type1 .. can be all the same, but leave it to the caller which is which
template<class T, typename iterator_type1, typename iterator_type2, typename iterator_type3, typename iterator_type4>
static void test_iterator_compare(const typename T::size_type size,
                                  iterator_type1& begin,
                                  iterator_type2& end,
                                  iterator_type3& citer1, iterator_type4& citer2,
                                  const typename T::difference_type citer1_idx,
                                  const typename T::difference_type citer2_idx)
{
    typedef typename T::difference_type diff_type;

    diff_type d_size = static_cast<diff_type>(size);
    diff_type distance = citer2_idx - citer1_idx;

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
        diff_type d_citer1_end = end - citer1;
        diff_type d_citer2_end = end - citer2;
        REQUIRE( ( citer1_idx + d_citer1_end ) == d_size); // validate op-(iter1, iter2)
        REQUIRE( ( citer2_idx + d_citer2_end ) == d_size); // validate op-(iter1, iter2)

        REQUIRE( ( citer1 + d_citer1_end ) == end);
        REQUIRE( ( citer2 + d_citer2_end ) == end);
    }

    // Adding redundant switched operands comparison
    // to test all relational combination of the overloading. (Leave it as is)

    if( 0 == distance ) {
        test_iterator_equal<T>(citer1, citer2);
        REQUIRE( !( citer2 >   citer1 ) );      // iter op>(iter1, iter2)
        REQUIRE(    citer2 >=  citer1   );      // iter op>=(iter1, iter2)
        REQUIRE( !( citer2 <   citer1 ) );      // iter op<(iter1, iter2)
        REQUIRE(    citer2 <=  citer1   );      // iter op<=(iter1, iter2)
        REQUIRE(    citer1 <=  citer2   );      // iter op>=(iter1, iter2)
        REQUIRE(    citer1 >=  citer2   );      // iter op>=(iter1, iter2)
    } else if( distance > 0 ) { // citer2 > citer1
        test_iterator_notequal<T>(citer1, citer2);
        REQUIRE(    citer2 >   citer1   );      // iter op>(iter1, iter2)
        REQUIRE(    citer2 >=  citer1   );      // iter op>=(iter1, iter2)
        REQUIRE( !( citer2 <   citer1 ) );      // iter op<(iter1, iter2)
        REQUIRE( !( citer2 <=  citer1 ) );      // iter op<=(iter1, iter2)
        REQUIRE(    citer1 <=  citer2   );      // iter op>(iter1, iter2)
        REQUIRE(    citer1 <   citer2   );      // iter op>=(iter1, iter2)
    } else { // distance < 0: citer2 < citer1
        test_iterator_notequal<T>(citer1, citer2);
        REQUIRE( !( citer2 >   citer1 ) );      // iter op>(iter1, iter2)
        REQUIRE( !( citer2 >=  citer1 ) );      // iter op>=(iter1, iter2)
        REQUIRE(    citer2 <   citer1   );      // iter op<(iter1, iter2)
        REQUIRE(    citer2 <=  citer1   );      // iter op<=(iter1, iter2)
        REQUIRE(    citer1 >   citer2   );      // iter op<(iter1, iter2)
        REQUIRE(    citer1 >=  citer2   );      // iter op<=(iter1, iter2)
    }
}

template<class T, typename iterator_type1, typename iterator_type2>
static void test_iterator_dereference(const typename T::size_type size,
                                      iterator_type1& begin, iterator_type2& end)
{
    printf("**** test_iterator_dereference:\n");
    print_iterator_info<iterator_type1>("iterator_type1");
    print_iterator_info<iterator_type2>("iterator_type2");

    {
        T data_exp = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        T data_has( begin, end );
        REQUIRE(data_has == data_exp);
    }

    // dereferencing, pointer, equality
    iterator_type1 citer1 = begin;
    iterator_type2 citer2 = begin;

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

    REQUIRE( *(citer2+0) == begin[0] );
    REQUIRE( *(citer2+1) == begin[1] );
    REQUIRE( *(citer2+2) == begin[2] );
    REQUIRE( *(citer2+3) == begin[3] );
    REQUIRE( *(citer2+size-1) == end[-1] );

    test_iterator_compare<T>(size, begin, end, citer1, citer2, 0, 0);
}

template<class T, typename iterator_type1, typename iterator_type2>
static void test_iterator_arithmetic(const typename T::size_type size,
                                     iterator_type1& begin, iterator_type2& end)
{
    printf("**** test_iterator_arithmetic:\n");
    print_iterator_info<iterator_type1>("iterator_type1");
    print_iterator_info<iterator_type2>("iterator_type2");

    // citer_type operations
    // op++(), op--(), op++(int), op--(int),
    // op+=(difference_type), op+(iter a, difference_type) ..
    {
        iterator_type1 citer1 = begin;
        iterator_type1 citer2 = begin;
        test_iterator_compare<T>(size, begin, end, citer1, citer2, 0, 0);

        // iter op++(int)
        citer2++;
        test_iterator_compare<T>(size, begin, end, citer1, citer2, 0, 1);

        // iter op++(int)
        citer1++;
        test_iterator_compare<T>(size, begin, end, citer1, citer2, 1, 1);

        // iter op--(int)
        citer2--;
        test_iterator_compare<T>(size, begin, end, citer1, citer2, 1, 0);

        // iter op--(int)
        citer1--;
        test_iterator_compare<T>(size, begin, end, citer1, citer2, 0, 0);
        REQUIRE( *citer2 == begin[0] );

        // iter op++(int)
        citer2++;
        test_iterator_compare<T>(size, begin, end, citer1, citer2, 0, 1);
        REQUIRE(   *citer2 == *(begin+1)    );  // iter op*(), op+(iter, difference_type) and value_type ==
        REQUIRE(   *citer2 == begin[1]      );  // iter op*(), op[](difference_type) and value_type ==

        // iter op++(int)
        citer2++;
        test_iterator_compare<T>(size, begin, end, citer1, citer2, 0, 2);
        REQUIRE(   *citer2 == *(begin+2)    );  // iter op*(), op+(iter, difference_type) and value_type ==
        REQUIRE(   *citer2 == begin[2]      );  // iter op*(), op[](difference_type) and value_type ==

        // iter op++(int)
        citer2++;
        test_iterator_compare<T>(size, begin, end, citer1, citer2, 0, 3);
        REQUIRE(   *citer2 == *(begin+3)    );  // iter op*(), op+(iter, difference_type) and value_type ==
        REQUIRE(   *citer2 == begin[3]      );  // iter op*(), op[](difference_type) and value_type ==

        // iter op++()
        --citer2;
        --citer2;
        --citer2;
        test_iterator_compare<T>(size, begin, end, citer1, citer2, 0, 0);
        REQUIRE(   *citer2 == *(begin+0)    );  // iter op*(), op+(iter, difference_type) and value_type ==
        REQUIRE(   *citer2 == begin[0]      );    // iter op*(), op[](difference_type) and value_type ==

        // iter +=(diff)
        citer2 += 3;
        test_iterator_compare<T>(size, begin, end, citer1, citer2, 0, 3);

        // iter +=(diff)
        citer2 += 6;
        test_iterator_compare<T>(size, begin, end, citer1, citer2, 0, 9);

        // iter -=(diff)
        citer2 -= 9;
        test_iterator_compare<T>(size, begin, end, citer1, citer2, 0, 0);
    }
    {
        // Adding redundant switched operands comparison
        // to test all relational combination of the overloading. (Leave it as is)

        iterator_type1 citer1 = begin;
        iterator_type1 citer2 = begin;

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
static bool test_citer_type_ops(const std::string& type_id,
                std::enable_if_t< is_cow_type<T>::value, bool> = true )
{
    typedef typename T::const_iterator  citer_type;
    typedef typename T::difference_type diff_type;

    T data;
    fill_list(data, 10);

    printf("**** test_citer_type_ops(CoW): %s\n", type_id.c_str());
    {
        citer_type begin = data.cbegin(); // immutable new_store non-const iterator, gets held until destruction
        citer_type end = begin.cend();    // no new store iterator, on same store as begin, obtained from begin
        diff_type data_size = static_cast<diff_type>(data.size());
        diff_type begin_size = static_cast<diff_type>(begin.size());
        diff_type end_size = static_cast<diff_type>(end.size());
        REQUIRE( begin_size                 == data_size );
        REQUIRE( end_size                   == data_size );
        REQUIRE( end  - begin               == data_size );
        REQUIRE( end - end_size             == begin     );
        REQUIRE( begin + begin_size         == end       );
        REQUIRE( *( end - end_size )        == *begin    );
        REQUIRE( *( begin + begin_size -1 ) == *(end-1)  );
        test_iterator_dereference<T, citer_type>(begin.size(), begin, end);
    }

    {
        citer_type begin = data.cbegin(); // no new store citer_type
        citer_type end = begin.cend();     // no new store citer_type, on same store as begin, obtained from begin
        test_iterator_arithmetic<T, citer_type>(data.size(), begin, end);
    }
    {
        T data2 = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        print_list("work", data);
        print_list("expt", data2);
        REQUIRE(data == data2);
    }
    return true;
}
template<class T>
static bool test_citer_type_ops(const std::string& type_id,
        std::enable_if_t< !is_cow_type<T>::value, bool> = true )
{
    typedef typename T::const_iterator  citer_type;
    typedef typename T::difference_type diff_type;
    T data;
    fill_list(data, 10);

    printf("**** test_citer_type_ops: %s\n", type_id.c_str());
    {
        citer_type begin = data.cbegin(); // mutable new_store non-const iterator, gets held until destruction
        citer_type end = data.cend();    // no new store iterator, on same store as begin and from begin
        diff_type data_size = static_cast<diff_type>(data.size());
        REQUIRE( end  - begin               == data_size );
        REQUIRE( end - data_size            == begin     );
        REQUIRE( begin + data_size          == end       );
        REQUIRE( *( end - data_size )       == *begin    );
        REQUIRE( *( begin + data_size - 1 ) == *( end - 1 ) );
        REQUIRE( end[-data_size]            == begin[0]    );
        REQUIRE( begin[data_size - 1]       == end[-1]     );
        test_iterator_dereference<T, citer_type>(data.size(), begin, end);
    }

    {
        citer_type begin = data.cbegin(); // no new store citer_type
        citer_type end = data.cend();     // no new store citer_type, on same store as begin
        test_iterator_arithmetic<T, citer_type, citer_type>(data.size(), begin, end);
    }
    {
        T data2 = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        REQUIRE(data == data2);
    }
    return true;
}

template<class T>
static bool test_mutable_iterator_ops(const std::string& type_id,
        std::enable_if_t< is_cow_type<T>::value, bool> = true )
{
    typedef typename T::size_type       size_type;
    typedef typename T::const_iterator  citer_type;
    typedef typename T::iterator        iter_type;
    typedef typename T::difference_type diff_type;
    typedef typename T::value_type      value_type;
    typedef typename T::storage_t       storage_t;

    printf("**** test_mutable_iterator_ops(CoW): %s\n", type_id.c_str());
    {
        T data;
        fill_list(data, 10);
        iter_type begin = data.begin(); // mutable new_store non-const iterator, gets held until destruction
        iter_type end = begin.end();    // no new store iterator, on same store as begin and from begin
        diff_type data_size = static_cast<diff_type>(data.size());
        diff_type begin_size = static_cast<diff_type>(begin.size());
        diff_type end_size = static_cast<diff_type>(end.size());
        REQUIRE( begin_size                 == data_size );
        REQUIRE( end_size                   == data_size );
        REQUIRE( end  - begin               == data_size );
        REQUIRE( end - end_size             == begin     );
        REQUIRE( begin + begin_size         == end       );
        REQUIRE( *( end - end_size )        == *begin    );
        REQUIRE( *( begin + begin_size - 1 ) == *( end - 1 ) );
        REQUIRE( end[-end_size]              == begin[0]    );
        REQUIRE( begin[begin_size - 1]       == end[-1]     );
        test_iterator_dereference<T, iter_type>(begin.size(), begin, end);
    }

    {
        T data;
        fill_list(data, 10);

        // all 4 combinations of iter, citer:
        iter_type begin = data.begin(); // mutable new_store non-const iterator, gets held until destruction
        iter_type end = begin.end();
        citer_type cbegin = begin.immutable();
        citer_type cend = cbegin.cend();

        test_iterator_arithmetic<T>(data.size(), begin, end);
        test_iterator_arithmetic<T>(data.size(), cbegin, cend);
        test_iterator_arithmetic<T>(data.size(), begin, cend);
        test_iterator_arithmetic<T>(data.size(), cbegin, end);
    }

    // iterator-op: darray/vector-op
    // -------------------------------------------
    // 1 pop_back()
    // 1 erase ():      erase (citer_type pos)
    // 3 erase (count): erase (iterator first, citer_type last)
    // 1 insert(const value_type& x): iterator insert(citer_type pos, const value_type& x)
    // 0 insert(value_type&& x): iterator insert(citer_type pos, value_type&& x)
    // 1 emplace(Args&&... args): emplace(citer_type pos, Args&&... args)
    // 2 insert(InputIt first, InputIt last ): insert( citer_type pos, InputIt first, InputIt last )
    // 1 void push_back(value_type& x)
    // 1 void push_back(value_type&& x)
    // 1 reference emplace_back(Args&&... args)
    // 0 [void push_back( InputIt first, InputIt last )]
    // 1 to_begin()
    {
        T data;
        fill_list(data, 10);
        citer_type          citer0      = data.cbegin(); // immutable orig store iterator
        {
            T data_exp = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
            REQUIRE(data == data_exp);
        }
        REQUIRE( *data.snapshot() == citer0.storage() );

        iter_type           iter        = data.begin(); // mutable new_store non-const iterator, gets held until destruction
        size_type           size_pre    = iter.size();
        value_type          elem        = iter.end()[-2];

        REQUIRE( iter != citer0 );
        REQUIRE( iter.storage() == citer0.storage() );
        REQUIRE( iter.storage() == *data.snapshot() );

        REQUIRE( iter.dist_begin() == 0       );
        REQUIRE( iter.dist_end()   == static_cast<diff_type>(size_pre));

        int i;
        (void)i;
        // pop_back()
        iter.pop_back();
        REQUIRE( iter.size() == size_pre-1 );
        REQUIRE( iter == iter.end() );
        REQUIRE( iter == iter.begin()+size_pre-1 );
        REQUIRE( iter.dist_begin() == static_cast<diff_type>(size_pre)-1 );
        REQUIRE( iter.dist_end()   == 0          );
        REQUIRE( iter[-1] == elem );
        {
            storage_t data_exp = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
            REQUIRE(iter.storage() == data_exp);
        }
        REQUIRE(iter.storage() != citer0.storage());
        REQUIRE(iter.storage() != *data.snapshot());

        // insert( citer_type pos, InputIt first, InputIt last )
        REQUIRE( iter == iter.end() );
        size_pre = iter.size();
        REQUIRE( iter.dist_begin() == static_cast<diff_type>(size_pre));
        REQUIRE( iter.dist_end()   == 0);
        {
            T data2;
            fill_list(data2, 10);
            // iter.push_back(data2.cbegin(), data2.cend()); // FIXME: Only in jau::darray not stl::vector
            iter.insert(data2.cbegin(), data2.cbegin()+data2.size()); // same as push_pack(..) since pointing to end() - but iter points here to first new elem
        }
        REQUIRE( iter.size() == size_pre+10 );
        REQUIRE( iter == iter.end()-10 );
        REQUIRE( iter.dist_begin() == static_cast<diff_type>(size_pre));
        REQUIRE( iter.dist_end()   == 10);
        {
            storage_t data_exp = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
            REQUIRE(iter.storage() == data_exp);
        }

        // erase (count): erase (iterator first, citer_type last)
        REQUIRE( iter == iter.end()-10 );
        size_pre = iter.size();
        // std::cout << "iter.5" << iter << (iter - iter.begin()) << "/" << iter.size() << ", size2 " << size2 << std::endl;
        iter.erase(10);
        // std::cout << "iter.6" << iter << (iter - iter.begin()) << "/" << iter.size() << std::endl;
        REQUIRE( iter.size() == size_pre-10 );
        REQUIRE( iter == iter.end() );
        {
            storage_t data_exp = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
            REQUIRE(iter.storage() == data_exp);
        }

        // erase ()
        size_pre = iter.size();
        iter.to_begin();
        REQUIRE( iter == iter.begin() );
        elem  = iter.begin()[1];
        iter.erase();
        REQUIRE( iter.size() == size_pre-1 );
        REQUIRE( iter == iter.begin() );
        REQUIRE( *iter == elem );
        {
            storage_t data_exp = { 2, 3, 4, 5, 6, 7, 8, 9 };
            REQUIRE(iter.storage() == data_exp);
        }

        // void push_back(value_type& x)
        size_pre = iter.size();
        REQUIRE( iter == iter.begin() );
        elem  = iter.end()[-1];
        {
            T data2;
            fill_list(data2, 10);
            citer_type data2_iter = data2.cbegin();
            iter.push_back(data2_iter[0]);
            iter.push_back(data2_iter[1]);
            iter.push_back(data2_iter[2]);
            REQUIRE( iter.size() == size_pre+3 );
            REQUIRE( iter == iter.end() );
            REQUIRE( iter[-3] == data2_iter[0] );
            REQUIRE( iter[-2] == data2_iter[1] );
            REQUIRE( iter[-1] == data2_iter[2] );
        }
        {
            storage_t data_exp = { 2, 3, 4, 5, 6, 7, 8, 9, 1, 2, 3 };
            REQUIRE(iter.storage() == data_exp);
        }

        // erase (count): erase (iterator first, citer_type last)
        size_pre = iter.size();
        REQUIRE( iter == iter.end() );
        iter -= 3;
        iter.erase(3);
        REQUIRE( iter.size() == size_pre-3 );
        REQUIRE( iter == iter.end() );
        {
            storage_t data_exp = { 2, 3, 4, 5, 6, 7, 8, 9 };
            REQUIRE(iter.storage() == data_exp);
        }

        // void push_back(value_type&& x)
        size_pre = iter.size();
        REQUIRE( iter == iter.end() );
        {
            value_type elem0 = iter.begin()[0];
            iter.push_back( std::move(elem0));
        }
        {
            value_type elem0 = iter.begin()[1];
            iter.push_back( std::move(elem0));
        }
        {
            value_type elem0 = iter.begin()[2];
            iter.push_back( std::move(elem0));
        }
        REQUIRE( iter.size() == size_pre+3 );
        REQUIRE( iter == iter.end() );
        REQUIRE( iter[-3] == iter.begin()[0] );
        REQUIRE( iter[-2] == iter.begin()[1] );
        REQUIRE( iter[-1] == iter.begin()[2] );
        {
            storage_t data_exp = { 2, 3, 4, 5, 6, 7, 8, 9, 2, 3, 4 };
            REQUIRE(iter.storage() == data_exp);
        }

        // erase last three
        REQUIRE( iter == iter.end() );
        iter -= 3;
        iter.erase();
        iter.erase();
        iter.erase();
        REQUIRE( iter == iter.end() );
        {
            storage_t data_exp = { 2, 3, 4, 5, 6, 7, 8, 9 };
            REQUIRE(iter.storage() == data_exp);
        }

        // iterator insert(citer_type pos, const value_type& x)
        iter.to_begin();
        iter += 5;
        REQUIRE( iter == iter.begin()+5 );
        REQUIRE( iter.dist_begin() == 5 );

        size_pre = iter.size();
        {
            T data2;
            fill_list(data2, 10);
            citer_type data2_iter = data2.cbegin();
            iter.insert(data2_iter[0]);
            iter.insert(data2_iter[1]);
            iter.insert(data2_iter[2]);
            REQUIRE( iter.size() == size_pre+3 );
            REQUIRE( iter == iter.begin()+5 );
            iter.to_begin();
            REQUIRE( iter[5] == data2_iter[2] );
            REQUIRE( iter[6] == data2_iter[1] );
            REQUIRE( iter[7] == data2_iter[0] );
        }
        {
            storage_t data_exp = { 2, 3, 4, 5, 6, 3, 2, 1, 7, 8, 9 };
            REQUIRE(iter.storage() == data_exp);
        }

        // insert( citer_type pos, InputIt first, InputIt last )
        iter += 5;
        REQUIRE( iter == iter.begin()+5 );
        size_pre = iter.size();
        {
            T data2;
            fill_list(data2, 10);
            iter.insert(data2.cbegin(), data2.cbegin()+3);
        }
        REQUIRE( iter.size() == size_pre+3 );
        REQUIRE( iter == iter.begin()+5 );
        {
            storage_t data_exp = { 2, 3, 4, 5, 6, 1, 2, 3, 3, 2, 1, 7, 8, 9 };
            REQUIRE(iter.storage() == data_exp);
        }

        // erase (count): erase (iterator first, citer_type last)
        REQUIRE( iter == iter.begin()+5 );
        size_pre = iter.size();
        iter.erase(6);
        REQUIRE( iter.size() == size_pre-6 );
        REQUIRE( iter == iter.begin()+5 );
        {
            storage_t data_exp = { 2, 3, 4, 5, 6, 7, 8, 9 };
            REQUIRE(iter.storage() == data_exp);
        }

        // 1 emplace(Args&&... args): emplace(citer_type pos, Args&&... args)
        size_pre = iter.size();
        REQUIRE( iter == iter.begin()+5 );
        iter.emplace( static_cast<uint64_t>(2) );
        iter.emplace( static_cast<uint64_t>(3) );
        iter.emplace( static_cast<uint64_t>(4) );
        REQUIRE( iter == iter.begin()+5 );
        REQUIRE( iter[0] == 4 );
        REQUIRE( iter[1] == 3 );
        REQUIRE( iter[2] == 2 );
        {
            storage_t data_exp = { 2, 3, 4, 5, 6, 4, 3, 2, 7, 8, 9 };
            REQUIRE(iter.storage() == data_exp);
        }

        // 1 reference emplace_back(Args&&... args)
        size_pre = iter.size();
        REQUIRE( iter == iter.begin()+5 );
        iter.emplace_back( static_cast<uint64_t>(2) );
        iter.emplace_back( static_cast<uint64_t>(3) );
        iter.emplace_back( static_cast<uint64_t>(4) );
        REQUIRE( iter == iter.end() );
        REQUIRE( iter[-1] == 4 );
        REQUIRE( iter[-2] == 3 );
        REQUIRE( iter[-3] == 2 );
        {
            storage_t data_exp = { 2, 3, 4, 5, 6, 4, 3, 2, 7, 8, 9, 2, 3, 4 };
            REQUIRE(iter.storage() == data_exp);
        }

        // multiple erase()
        size_pre = iter.size();
        REQUIRE( iter == iter.end() );
        iter -= 10;
        REQUIRE( iter == iter.end()-10 );
        {
            while( iter != iter.end() ) {
                iter.erase();
            }
            REQUIRE( iter.size() == size_pre - 10 );
        }
        {
            storage_t data_exp = { 2, 3, 4, 5 };
            REQUIRE(iter.storage() == data_exp);
        }
        iter.to_begin(); // set to its begin

        // write back ..
        REQUIRE( iter != data.cbegin() );   // still not the same
        REQUIRE( iter.storage() != *data.snapshot() );  // neither content
        {
            T data_exp = { 2, 3, 4, 5 };
            REQUIRE(data != data_exp);
        }
        iter.write_back(); // invalidates iter and ...
        {
            T data_exp = { 2, 3, 4, 5 };
            REQUIRE(data == data_exp);      // now got the newt content
        }
    }

    return true;
}

template<class T>
static bool test_mutable_iterator_ops(const std::string& type_id,
        std::enable_if_t< !is_cow_type<T>::value, bool> = true )
{
    typedef typename T::iterator        iter_type;
    typedef typename T::const_iterator  citer_type;
    typedef typename T::difference_type diff_type;

    printf("**** test_mutable_iterator_ops(___): %s\n", type_id.c_str());
    {
        T data;
        fill_list(data, 10);
        iter_type begin = data.begin();
        iter_type end = data.end();
        diff_type data_size = static_cast<diff_type>(data.size());
        REQUIRE( end  - begin               == data_size );
        REQUIRE( end - data_size            == begin     );
        REQUIRE( begin + data_size          == end       );
        REQUIRE( *( end - data_size )       == *begin    );
        REQUIRE( *( begin + data_size - 1 ) == *(end -1) );
        test_iterator_dereference<T, iter_type>(data.size(), begin, end);
    }

    {
        T data;
        fill_list(data, 10);

        // all 4 combinations of iter, citer:
        iter_type begin = data.begin();
        iter_type end = data.end();
        citer_type cend = data.cend();
        citer_type cbegin = data.cbegin();

        test_iterator_arithmetic<T>(data.size(), begin, end);
        test_iterator_arithmetic<T>(data.size(), cbegin, cend);
        test_iterator_arithmetic<T>(data.size(), begin, cend);
        test_iterator_arithmetic<T>(data.size(), cbegin, end);
    }

    // iterator-op: darray/vector-op
    // -------------------------------------------
    // 1 pop_back()
    // 1 erase (citer_type pos)
    // 3 erase (iterator first, citer_type last)
    // 1 iterator insert(citer_type pos, const value_type& x)
    // 0 iterator insert(citer_type pos, value_type&& x)
    // 1 emplace(citer_type pos, Args&&... args)
    // 2 insert( citer_type pos, InputIt first, InputIt last )
    // 1 void push_back(value_type& x)
    // 1 void push_back(value_type&& x)
    // 1 reference emplace_back(Args&&... args)
    // 0 [void push_back( InputIt first, InputIt last )]
    {
        T data;
        fill_list(data, 10);
        {
            T data_exp = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
            REQUIRE(data == data_exp);
        }

        iter_type               iter     = data.end();
        typename T::size_type   size_pre = data.size();
        typename T::value_type  elem     = iter[-2];

        // pop_back()
        int i;
        (void)i;
        data.pop_back();
        iter--;
        REQUIRE( data.size() == size_pre-1 );
        REQUIRE( iter == data.end() );
        REQUIRE( iter == data.begin()+size_pre-1 );
        REQUIRE( iter[-1] == elem );
        {
            T data_exp = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
            REQUIRE(data == data_exp);
        }

        // insert( citer_type pos, InputIt first, InputIt last )
        REQUIRE( iter == data.end() );
        size_pre = data.size();
        {
            T data2;
            fill_list(data2, 10);
            iter = data.insert(iter, data2.cbegin(), data2.cbegin()+data2.size()); // same as push_pack(..) since pointing to end() - but data points here to first new elem
        }
        REQUIRE( data.size() == size_pre+10 );
        REQUIRE( iter == data.end()-10 );
        {
            T data_exp = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
            REQUIRE(data == data_exp);
        }

        // erase (count): erase (iterator first, citer_type last)
        REQUIRE( iter == data.end()-10 );
        size_pre = data.size();
        // std::cout << "data.5" << data << (data - data.begin()) << "/" << data.size() << ", size2 " << size2 << std::endl;
        iter = data.erase(iter, iter+10);
        // std::cout << "data.6" << data << (data - data.begin()) << "/" << data.size() << std::endl;
        REQUIRE( data.size() == size_pre-10 );
        REQUIRE( iter == data.end() );
        {
            T data_exp = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
            REQUIRE(data == data_exp);
        }

        // erase ()
        size_pre = data.size();
        iter = data.begin();
        REQUIRE( iter == data.begin() );
        elem  = iter[1];
        iter = data.erase(iter);
        REQUIRE( data.size() == size_pre-1 );
        REQUIRE( iter == data.begin() );
        REQUIRE( *iter == elem );
        {
            T data_exp = { 2, 3, 4, 5, 6, 7, 8, 9 };
            REQUIRE(data == data_exp);
        }

        // void push_back(value_type& x)
        size_pre = data.size();
        REQUIRE( iter == data.begin() );
        elem  = data.end()[-1];
        {
            T data2;
            fill_list(data2, 10);
            data.push_back(data2[0]);
            data.push_back(data2[1]);
            data.push_back(data2[2]);
            iter = data.end();
            REQUIRE( data.size() == size_pre+3 );
            REQUIRE( iter == data.end() );
            REQUIRE( iter[-3] == data2[0] );
            REQUIRE( iter[-2] == data2[1] );
            REQUIRE( iter[-1] == data2[2] );
        }
        {
            T data_exp = { 2, 3, 4, 5, 6, 7, 8, 9, 1, 2, 3 };
            REQUIRE(data == data_exp);
        }

        // erase (count): erase (iterator first, citer_type last)
        size_pre = data.size();
        REQUIRE( iter == data.end() );
        iter -= 3;
        iter = data.erase(iter, iter+3);
        REQUIRE( data.size() == size_pre-3 );
        REQUIRE( iter == data.end() );
        {
            T data_exp = { 2, 3, 4, 5, 6, 7, 8, 9 };
            REQUIRE(data == data_exp);
        }

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
        {
            T data_exp = { 2, 3, 4, 5, 6, 7, 8, 9, 2, 3, 4 };
            REQUIRE(data == data_exp);
        }

        // erase last three
        REQUIRE( iter == data.end() );
        iter -= 3;
        iter = data.erase(iter);
        iter = data.erase(iter);
        iter = data.erase(iter);
        REQUIRE( iter == data.end() );
        {
            T data_exp = { 2, 3, 4, 5, 6, 7, 8, 9 };
            REQUIRE(data == data_exp);
        }

        // iterator insert(citer_type pos, const value_type& x)
        iter = data.begin();
        iter += 5;
        REQUIRE( iter == data.begin()+5 );
        size_pre = data.size();
        {
            T data2;
            fill_list(data2, 10);
            iter = data.insert(iter, data2[0]);
            iter = data.insert(iter, data2[1]);
            iter = data.insert(iter, data2[2]);
            REQUIRE( data.size() == size_pre+3 );
            REQUIRE( iter == data.begin()+5 );
            iter = data.begin();
            REQUIRE( iter[5] == data2[2] );
            REQUIRE( iter[6] == data2[1] );
            REQUIRE( iter[7] == data2[0] );
        }
        {
            T data_exp = { 2, 3, 4, 5, 6, 3, 2, 1, 7, 8, 9 };
            REQUIRE(data == data_exp);
        }

        // insert( citer_type pos, InputIt first, InputIt last )
        iter += 5;
        REQUIRE( iter == data.begin()+5 );
        size_pre = data.size();
        {
            T data2;
            fill_list(data2, 10);
            iter = data.insert(iter, data2.cbegin(), data2.cbegin()+3);
        }
        REQUIRE( data.size() == size_pre+3 );
        REQUIRE( iter == data.begin()+5 );
        {
            T data_exp = { 2, 3, 4, 5, 6, 1, 2, 3, 3, 2, 1, 7, 8, 9 };
            REQUIRE(data == data_exp);
        }

        // erase (count): erase (iterator first, citer_type last)
        REQUIRE( iter == data.begin()+5 );
        size_pre = data.size();
        iter = data.erase(iter, iter+6);
        REQUIRE( data.size() == size_pre-6 );
        REQUIRE( iter == data.begin()+5 );
        {
            T data_exp = { 2, 3, 4, 5, 6, 7, 8, 9 };
            REQUIRE(data == data_exp);
        }

        // 1 emplace(Args&&... args): emplace(citer_type pos, Args&&... args)
        size_pre = data.size();
        REQUIRE( iter == data.begin()+5 );
        iter = data.emplace(iter, static_cast<uint64_t>(2) );
        iter = data.emplace(iter, static_cast<uint64_t>(3) );
        iter = data.emplace(iter, static_cast<uint64_t>(4) );

        REQUIRE( iter == data.begin()+5 );
        REQUIRE( iter[0] == 4 );
        REQUIRE( iter[1] == 3 );
        REQUIRE( iter[2] == 2 );
        {
            T data_exp = { 2, 3, 4, 5, 6, 4, 3, 2, 7, 8, 9 };
            REQUIRE(data == data_exp);
        }

        // 1 reference emplace_back(Args&&... args)
        size_pre = data.size();
        REQUIRE( iter == data.begin()+5 );
        data.emplace_back( static_cast<uint64_t>(2) );
        data.emplace_back( static_cast<uint64_t>(3) );
        data.emplace_back( static_cast<uint64_t>(4) );
        iter = data.end();
        REQUIRE( iter == data.end() );
        REQUIRE( iter[-1] == 4 );
        REQUIRE( iter[-2] == 3 );
        REQUIRE( iter[-3] == 2 );
        {
            T data_exp = { 2, 3, 4, 5, 6, 4, 3, 2, 7, 8, 9, 2, 3, 4 };
            REQUIRE(data == data_exp);
        }

        // multiple erase()
        size_pre = data.size();
        REQUIRE( iter == data.end() );
        iter -= 10;
        REQUIRE( iter == data.end()-10 );
        {
            while( iter != data.end() ) {
                iter = data.erase(iter);
            }
            REQUIRE( data.size() == size_pre - 10 );
            REQUIRE( iter == data.end() );
        }
        {
             T data_exp = { 2, 3, 4, 5 };
             REQUIRE(data == data_exp);
        }
    }
    {
        T data;
        fill_list(data, 10);
        T data2 = data;
        T data3(data);
        print_list("orig", data2);
        print_list("copy1", data2);
        print_list("copy2", data3);
        REQUIRE(data == data2);
        REQUIRE(data == data3);
    }
    return true;
}

/****************************************************************************************
 ****************************************************************************************/

template<class T>
static bool test_01_validate_iterator_ops(const std::string& type_id) {

    test_citer_type_ops<T>(type_id);

    test_mutable_iterator_ops<T>(type_id);

    return true;
}

template<class T>
static bool test_01_cow_iterator_properties(const std::string& type_id) {
    typedef typename T::size_type    size_type;
    typedef typename T::const_iterator  citer_type;
    typedef typename T::iterator        iter_type;

    printf("**** test_cow_iterator_properties: %s\n", type_id.c_str());
    print_iterator_info<citer_type>("citer_type");
    print_iterator_info<iter_type>("iter_type");

    const size_type size0 = 100;

    T data;
    REQUIRE(0 == data.get_allocator().memory_usage);
    REQUIRE(data.size() == 0);
    REQUIRE(data.capacity() == 0);
    REQUIRE(data.empty() == true);

    fill_list(data, size0);
    REQUIRE(0 != data.get_allocator().memory_usage);
    REQUIRE(data.size() == size0);
    REQUIRE(data.size() <= data.capacity());

    // test relationship and distance with mixed iterator and citer_type
    // in both direction using the free overloaded operator of cow_ro_* and cow_rw_*
    {
        iter_type   iter1 = data.begin();
        citer_type citer2 =  iter1.immutable();
        citer_type citer3 =  iter1.immutable().to_end();

        REQUIRE(     iter1.is_begin() );
        REQUIRE(    citer2.is_begin() );
        REQUIRE(    citer3.is_end() );

        REQUIRE(  iter1.dist_begin() == 0);
        REQUIRE(  iter1.dist_end()   == size0);
        REQUIRE( citer2.dist_begin() == 0);
        REQUIRE( citer2.dist_end()   == size0);
        REQUIRE( citer3.dist_begin() == size0);
        REQUIRE( citer3.dist_end()   == 0);

        REQUIRE( (   iter1 == citer2  ) == true);  // iter op==()
        REQUIRE( (  citer2 ==  iter1 ) == true);  // iter op==()

        ++citer2;
        REQUIRE( (  citer2 !=  iter1 ) == true);  // iter op==()
        REQUIRE( (   iter1 != citer2 ) == true);  // iter op==()
        REQUIRE( (  citer2 >   iter1 ) == true);  // iter op==()
        REQUIRE( (  citer2 >=  iter1 ) == true);  // iter op==()
        REQUIRE( (   iter1 <  citer2 ) == true);  // iter op==()
        REQUIRE( (   iter1 <= citer2 ) == true);  // iter op==()
        REQUIRE( (  citer2 -   iter1 ) ==  1);  // iter op==()
        REQUIRE( (   iter1 -  citer2 ) == -1);  // iter op==()
        REQUIRE( citer2.dist_begin()   ==  1);
        REQUIRE( citer2.dist_end()     == size0-1);

        --citer2;
        ++iter1;
        REQUIRE( (   iter1 != citer2 ) == true);  // iter op==()
        REQUIRE( (  citer2 !=  iter1 ) == true);  // iter op==()
        REQUIRE( (   iter1 >  citer2 ) == true);  // iter op==()
        REQUIRE( (   iter1 >= citer2 ) == true);  // iter op==()
        REQUIRE( (  citer2 <   iter1 ) == true);  // iter op==()
        REQUIRE( (  citer2 <=  iter1 ) == true);  // iter op==()
        REQUIRE( (   iter1 -  citer2 ) ==  1);  // iter op==()
        REQUIRE( (  citer2 -   iter1 ) == -1);  // iter op==()
        REQUIRE(  iter1.dist_begin() == 1);
        REQUIRE(  iter1.dist_end()   == size0-1);
        REQUIRE( citer2.dist_begin() == 0);
        REQUIRE( citer2.dist_end()   == size0);

        REQUIRE( (   iter1.end() == citer3  ) == true);  // iter op==()
        REQUIRE( (   iter1.to_end() == citer3  ) == true);  // iter op==()
        REQUIRE(     iter1.is_end() );
        REQUIRE(    citer3.is_end() );
        REQUIRE(  iter1.dist_begin() == size0);
        REQUIRE(  iter1.dist_end()   == 0);
    }

    // test mutable non-const 'new store' behavior
    citer_type c_begin0 = data.cbegin();  // orig store
    {
        // iterator_type is mutable iterator.
        // The cow_rw_iterator is being fetched via data.begin(), which creates a new store and stays.
        // The cow_rw_iterator's new store is moved into the cow container via write_back() later.

        printf("testing mutable non-const behavior.\n");
        citer_type c_begin1;
        {
            iter_type m_begin1 = data.begin();      // mutable new_store non-const iterator, gets held until write_back() or destruction
            c_begin1 = m_begin1.immutable();        // get immutable citer_type from newly created store

            REQUIRE(*c_begin1 == *m_begin1);
            REQUIRE( c_begin1 ==  m_begin1);
            REQUIRE( ( c_begin1 - m_begin1 ) == 0);
            printf("       1st store: %s == %s, dist %u\n",
                    to_decstring(*c_begin1, ',', 2).c_str(), to_decstring(*m_begin1, ',', 2).c_str(), (unsigned int)(c_begin1 - m_begin1));
            citer_type c_begin2;
            {
                iter_type m_begin2 = data.begin();  // mutable new_store non-const iterator, gets held until write_back() or destruction
                c_begin2 = m_begin2.immutable();    // get immutable citer_type from newly created store

                REQUIRE(*c_begin2 == *m_begin2);
                REQUIRE( c_begin2 ==  m_begin2);
                REQUIRE( ( c_begin2 - m_begin2 ) == 0);
                printf("       2nd store: %s == %s, dist %u\n",
                        to_decstring(*c_begin2, ',', 2).c_str(), to_decstring(*m_begin2, ',', 2).c_str(), (unsigned int)(c_begin2 - m_begin2));

                REQUIRE(*c_begin2 == *c_begin1);
                REQUIRE( c_begin2 !=  c_begin1);
                REQUIRE( ( c_begin2 - c_begin1 ) != 0);
                printf("2nd -> 1st store: %s == %s, dist %u\n",
                        to_decstring(*c_begin2, ',', 2).c_str(), to_decstring(*c_begin1, ',', 2).c_str(), (unsigned int)(c_begin2 - c_begin1));

                m_begin2.write_back();              // write back storage of m_begin2 to parent CoW and invalidate m_begin2
            }
            // 2nd store -> cow_xxx
            citer_type c_begin2b = data.cbegin();
            REQUIRE(*c_begin2 == *c_begin2b);
            REQUIRE( c_begin2 ==  c_begin2b);
            REQUIRE( ( c_begin2 - c_begin2b ) == 0);
            printf("2nd -> cow == cbegin: %s == %s, dist %u\n",
                    to_decstring(*c_begin2, ',', 2).c_str(), to_decstring(*c_begin2b, ',', 2).c_str(), (unsigned int)(c_begin2 - c_begin2b));
            printf("2nd -> 1st          : %s == %s, dist %u\n",
                    to_decstring(*c_begin1, ',', 2).c_str(), to_decstring(*c_begin2, ',', 2).c_str(), (unsigned int)(c_begin1 - c_begin2));

            m_begin1.write_back();                  // write back storage of m_begin1 to parent CoW and invalidate m_begin2
        }
        // 1st store -> cow_xxx
        citer_type c_begin1b = data.cbegin();
        printf("1st -> cow == cbegin: %s == %s, dist %u\n",
                to_decstring(*c_begin1, ',', 2).c_str(), to_decstring(*c_begin1b, ',', 2).c_str(), (unsigned int)(c_begin1 - c_begin1b));
        REQUIRE(*c_begin1 == *c_begin1b);
        REQUIRE( c_begin1 ==  c_begin1b);
        REQUIRE( ( c_begin1 - c_begin1b ) == 0);
    }
    return true;
}

/****************************************************************************************
 ****************************************************************************************/

TEST_CASE( "Iterator Test 00 - Inspect all Iterator Types", "[datatype][std][vector][darray][cow_vector][cow_darray]" ) {
    test_00_inspect_iterator_types< std_vector_uint64_t >("std::vector<T>");
    test_00_inspect_iterator_types< jau_darray_uint64_t >("jau::darray<T>");
    test_00_inspect_iterator_types< jau_cow_vector_uint64_t >("jau::cow_vector<T>");
    test_00_inspect_iterator_types< jau_cow_darray_uint64_t >("jau::cow_darray<T>");
}

TEST_CASE( "STD Vector Test 01 - Validate Iterator and Index Operations", "[datatype][std][vector]" ) {
    test_01_validate_iterator_ops< std_vector_uint64_t >("std::vector<T>");
}

TEST_CASE( "JAU DArray Test 02 - Validate Iterator and Index Operations", "[datatype][jau][darray]" ) {
    test_01_validate_iterator_ops< jau_darray_uint64_t >("jau::darray<T>");
}

TEST_CASE( "JAU COW_Vector Test 11 - Validate Iterator Operations", "[datatype][jau][cow_vector]" ) {
    test_01_validate_iterator_ops< jau_cow_vector_uint64_t >("jau::cow_vector<T>");

    test_01_cow_iterator_properties<jau_cow_vector_uint64_t>("jau::cow_vector<T>");
}

TEST_CASE( "JAU COW_DArray Test 21 - Validate Iterator Operations", "[datatype][jau][cow_darray]" ) {
    test_01_validate_iterator_ops< jau_cow_darray_uint64_t >("jau::cow_darray<T>");

    test_01_cow_iterator_properties<jau_cow_darray_uint64_t>("jau::cow_darray<T>");
}
