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

#include <jau/test/catch2_ext.hpp>

#include <jau/type_traits_queries.hpp>
#include <jau/basic_types.hpp>

#include "test_datatype01.hpp"

using namespace jau;

typedef std::vector<int> std_vec_int;
JAU_TYPENAME_CUE_ALL(std_vec_int)

typedef std_vec_int::iterator std_vec_int_iter;
JAU_TYPENAME_CUE_ALL(std_vec_int_iter)

typedef std_vec_int::const_iterator std_vec_int_citer;
JAU_TYPENAME_CUE_ALL(std_vec_int_citer)

typedef std_vec_int_citer::pointer std_vec_int_citer_pointer;
JAU_TYPENAME_CUE_ALL(std_vec_int_citer_pointer)

typedef decltype( std::declval<std_vec_int_citer>().operator->() ) std_vec_int_citer_ptrop_retval;

TEST_CASE( "JAU to_string() Test 00 - jau::to_string() std::string conversion", "[jau][std::string][to_string()]" ) {
    int i1 = 1;
    uint64_t u64_1 = 1116791496961ull;
    void * p_v_1 = (void *)0xAFFE;
    float float_1 = 1.65f;

    Addr48Bit addr48bit_1(u64_1);

    CHECK( "1" == jau::to_string<int>(i1) );
    CHECK( "1116791496961" == jau::to_string(u64_1) );
    CHECK( "0xaffe" == jau::to_string(p_v_1) );
    CHECK( "1.650000" == jau::to_string(float_1) );

    CHECK( "01:04:05:F5:E1:01" == jau::to_string(addr48bit_1) );

    //
    // Validating 'pointer std::vector::const_iterator.operator->()'
    // and the to_string type trait logic of it.
    //

    // jau::type_cue<std_vec_int_citer>::print("std_vec_int_citer", jau::TypeTraitGroup::ALL);
    // jau::type_cue<std_vec_int_citer_pointer>::print("std_vec_int_citer_pointer", jau::TypeTraitGroup::ALL);

    // jau::type_cue<std_vec_int_citer_ptrop_retval>::print("std_vec_int_citer_ptrop_retval", jau::TypeTraitGroup::ALL);
    printf("jau::has_member_of_pointer<std_vec_int_citer>) %d\n", jau::has_member_of_pointer<std_vec_int_citer>::value);

    std_vec_int vec_int_1;
    vec_int_1.push_back(1); vec_int_1.push_back(2); vec_int_1.push_back(3);
    std_vec_int_citer vec_int_citer_1B = vec_int_1.cbegin();
    uint8_t* vec_int_citer_1B_ptr = (uint8_t*)(vec_int_citer_1B.operator->());
    std::string vec_int_citer_1B_str = to_hexstring(vec_int_citer_1B_ptr);

    std_vec_int_citer vec_int_citer_1E = vec_int_1.cend();
    uint8_t* vec_int_citer_1E_ptr = (uint8_t*)(vec_int_citer_1E.operator->());
    std::string vec_int_citer_1E_str = to_hexstring(vec_int_citer_1E_ptr);

    std::ptrdiff_t vec_int_citer_1E_1B_ptrdiff = vec_int_citer_1E_ptr - vec_int_citer_1B_ptr;
    size_t vec_int_citer_1E_1B_ptr_count = vec_int_citer_1E_1B_ptrdiff / sizeof(int);
    size_t vec_int_citer_1E_1B_itr_count = vec_int_citer_1E - vec_int_citer_1B;

    printf("vec_int_citer_1E - vec_int_citer_1B = itr_count %zu, ptr_count %zu\n",
            vec_int_citer_1E_1B_itr_count, vec_int_citer_1E_1B_ptr_count);
    printf("vec_int_citer_1E - vec_int_citer_1B = %zu\n", vec_int_citer_1E_1B_itr_count);
    printf("vec_int_citer_1B_ptr %s, vec_int_citer_1E1_ptr = %s\n", vec_int_citer_1B_str.c_str(), vec_int_citer_1E_str.c_str());

    CHECK(vec_int_citer_1E_1B_itr_count == 3);
    CHECK(vec_int_citer_1E_1B_itr_count == vec_int_citer_1E_1B_ptr_count);

    CHECK( vec_int_citer_1E_str == jau::to_string(vec_int_citer_1E) );
}

