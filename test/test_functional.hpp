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
#include <string>
#include <typeindex>

#ifndef FUNCTIONAL_PROVIDED
    #define FUNCTIONAL_IMPL 1
    #include <jau/functional.hpp>
    static std::string impl_name = "jau/functional.hpp";
#endif

#include <jau/type_traits_queries.hpp>

#include <jau/test/catch2_ext.hpp>

using namespace jau;

// Test examples.

static int Func0a_free(int i) noexcept {
    int res = i+100;
    return res;
}

static void Func1a_free(int&r, int i) noexcept {
    r = i+100;
}

static void Func2a_free() noexcept {
    // nop
}

class TestFunction01 {
  public:

    /**
     * Unit test covering most variants of jau::function<R(A...)
     */
    void test00_usage() {
        INFO("Test 00_usage: START: Implementation = functional "+std::to_string( FUNCTIONAL_IMPL )+".hpp");
        fprintf(stderr, "Implementation: functional %d\n", FUNCTIONAL_IMPL);
        {
            // Test capturing lambdas
            volatile int i = 100;

            function<int(int)> fa0 = [&](int a) -> int {
                return i + a;
            };
            fprintf(stderr, "lambda.0: %s, signature %s\n", fa0.toString().c_str(), fa0.signature().name());
            REQUIRE( jau::func::target_type::lambda == fa0.type() );

            function<int(int)> fa1 = lambda_01();
            fprintf(stderr, "lambda.1: %s, signature %s\n", fa1.toString().c_str(), fa1.signature().name());
            REQUIRE( jau::func::target_type::lambda == fa1.type() );

            auto fa2_stub = [&](int a) -> int {
                return i + a;
            };
            function<int(int)> fa2_a = fa2_stub;
            fprintf(stderr, "lambda.2_a: %s, signature %s\n", fa2_a.toString().c_str(), fa2_a.signature().name());
            REQUIRE( jau::func::target_type::lambda == fa2_a.type() );

            function<int(int)> fa2_b = fa2_stub;
            fprintf(stderr, "lambda.2_b: %s, signature %s\n", fa2_b.toString().c_str(), fa2_b.signature().name());
            REQUIRE( jau::func::target_type::lambda == fa2_b.type() );

            test_function0_result_____("lambda.0_1_",       1, 101, fa0, fa1);
            test_function0________type("lambda.0_1_", false,        fa0, fa1);
            test_function0_result_____("lambda.0_2a",       1, 101, fa0, fa2_a);
            test_function0_result_____("lambda.0_2b",       1, 101, fa0, fa2_b);
            if constexpr ( jau::type_info::limited_lambda_id ) {
                if( fa0 == fa2_a ) {
                    fprintf(stderr, "INFO: limited_lambda_id: %s:%d\n", __FILE__, __LINE__);
                } else {
                    fprintf(stderr, "INFO: limited_lambda_id FIXED: %s:%d\n", __FILE__, __LINE__);
                    test_function0________type("lambda.0_2a", false,        fa0, fa2_a);
                }
                if( fa0 == fa2_b ) {
                    fprintf(stderr, "INFO: limited_lambda_id: %s:%d\n", __FILE__, __LINE__);
                } else {
                    fprintf(stderr, "INFO: limited_lambda_id FIXED: %s:%d\n", __FILE__, __LINE__);
                    test_function0________type("lambda.0_2b", false,        fa0, fa2_b);
                }
            } else {
                fprintf(stderr, "INFO: !limited_lambda_id: %s:%d\n", __FILE__, __LINE__);
                test_function0________type("lambda.0_2a", false,        fa0, fa2_a);
                test_function0________type("lambda.0_2b", false,        fa0, fa2_b);
            }
            test_function0_result_____("lambda.2a2b",       1, 101, fa2_a, fa2_b);
            test_function0________type("lambda.2a2b", true,         fa2_a, fa2_b);
        }

#if ( FUNCTIONAL_IMPL == 1 )
        {
            // Test non-capturing lambdas
            function<int(int)> f_1 = [](int a) -> int {
                return a + 100;
            } ;
            fprintf(stderr, "lambda.3_1 (plain) %s, signature %s\n", f_1.toString().c_str(), f_1.signature().name());
            REQUIRE( jau::func::target_type::lambda == f_1.type() );
            test_function0_result_type("lambda.3131", true, 1, 101, f_1, f_1);

            function<int(int)> f_2 = function<int(int)>::bind_lambda( [](int x) -> int {
                return x + 100;
            } );
            fprintf(stderr, "lambda.3_2 (plain) %s, signature %s\n", f_2.toString().c_str(), f_2.signature().name());
            REQUIRE( jau::func::target_type::lambda == f_2.type() );
            test_function0_result_type("lambda.3232", true, 1, 101, f_2, f_2);
        }
        {
            // Test non-capturing y-lambdas
            function<int(int)> f_1 = function<int(int)>::bind_ylambda( [](auto& self, int x) -> int {
                if( 0 == x ) {
                    return 1;
                } else {
                    return x * self(x-1);
                }
            } );
            fprintf(stderr, "ylambda.1_1 (plain) %s, signature %s\n", f_1.toString().c_str(), f_1.signature().name());
            REQUIRE( jau::func::target_type::ylambda == f_1.type() );
            test_function0_result_type("ylambda.1111", true, 4, 24, f_1, f_1);
        }
#endif
        {
            // Test non-capturing lambdas -> forced free functions
            typedef int(*cfunc)(int); // to force non-capturing lambda into a free function template type deduction
            volatile int i = 100;

            auto f = ( [](int a) -> int {
                // return i + a;
                return a + 100;
            } );
            function<int(int)> fl_ = bind_free<int, int>( (cfunc) f);
            fprintf(stderr, "plain lambda.0 %s\n", fl_.toString().c_str());
            REQUIRE( jau::func::target_type::free == fl_.type() );

            test_function0_result_type("FuncPtr1a_free_10", true, 1, 101, fl_, fl_);
            (void)i;

        }
        {
            // free, result void and no params
            typedef void(*cfunc)();
            function<void()> fl_0 = (cfunc) ( []() -> void {
                // nop
            } );
            fprintf(stderr, "freeA.0 %s\n", fl_0.toString().c_str());
            REQUIRE( jau::func::target_type::free == fl_0.type() );

            function<void()> f2a_0 = Func2a_free;
            fprintf(stderr, "freeA.1 %s\n", f2a_0.toString().c_str());
            REQUIRE( jau::func::target_type::free == f2a_0.type() );

            function<void()> f2a_1 = bind_free(Func2a_free);
            fprintf(stderr, "freeA.2 %s\n", f2a_1.toString().c_str());
            REQUIRE( jau::func::target_type::free == f2a_1.type() );

            function<void()> f20a_1 = bind_free(&TestFunction01::Func20a_static);
            fprintf(stderr, "freeA.3 %s\n", f20a_1.toString().c_str());
            REQUIRE( jau::func::target_type::free == f20a_1.type() );

            function<void()> f20a_2 = bind_free(&TestFunction01::Func20a_static);
            fprintf(stderr, "freeA.4 %s\n", f20a_2.toString().c_str());
            REQUIRE( jau::func::target_type::free == f20a_2.type() );

            test_function2________type("FuncPtr1a_free_10", true,   fl_0, fl_0);
            test_function2________type("FuncPtr1a_free_10", true,   f2a_0, f2a_1);
            test_function2________type("FuncPtr1a_free_10", true,   f2a_1, f2a_1);
            test_function2________type("FuncPtr3a_free_11", true,   f20a_1, f20a_1);
            test_function2________type("FuncPtr3a_free_12", true,   f20a_1, f20a_2);
            test_function2________type("FuncPtr1a_free_10", false,  f2a_1, f20a_1);

        }
        {
            // free, result non-void
            typedef int(*cfunc)(int); // to force non-capturing lambda into a free function template type deduction
            function<int(int)> fl_0 = (cfunc) ( [](int i) -> int {
                int res = i+100;
                return res;
            } );
            fprintf(stderr, "freeB.0 %s\n", fl_0.toString().c_str());
            REQUIRE( jau::func::target_type::free == fl_0.type() );

            function<int(int)> f1a_0 = Func0a_free;
            fprintf(stderr, "freeB.1 %s\n", f1a_0.toString().c_str());
            REQUIRE( jau::func::target_type::free == f1a_0.type() );

            function<int(int)> f1a_1 = bind_free(Func0a_free);
            function<int(int)> f3a_1 = bind_free(&TestFunction01::Func03a_static);
            function<int(int)> f3a_2 = bind_free(&TestFunction01::Func03a_static);
            test_function0_result_type("FuncPtr1a_free_10", true,   1, 101, fl_0, fl_0);
            test_function0_result_type("FuncPtr1a_free_10", true,   1, 101, f1a_0, f1a_1);
            test_function0_result_type("FuncPtr1a_free_10", true,   1, 101, f1a_1, f1a_1);
            test_function0_result_type("FuncPtr3a_free_11", true,   1, 101, f3a_1, f3a_1);
            test_function0_result_type("FuncPtr3a_free_12", true,   1, 101, f3a_1, f3a_2);
            test_function0_result_type("FuncPtr1a_free_10", false,  1, 101, f1a_1, f3a_1);
        }
        {
            // free, result void
            typedef void(*cfunc)(int&, int); // to force non-capturing lambda into a free function template type deduction
            function<void(int&, int)> fl_0 = (cfunc) ( [](int& res, int i) -> void {
                res = i+100;
            } );
            function<void(int&, int)> f1a_0 = Func1a_free;
            function<void(int&, int)> f1a_1 = bind_free(Func1a_free);
            function<void(int&, int)> f3a_0 = &TestFunction01::Func13a_static;
            function<void(int&, int)> f3a_1 = bind_free(&TestFunction01::Func13a_static);
            function<void(int&, int)> f3a_2 = bind_free(&TestFunction01::Func13a_static);
            test_function1_result_type("FuncPtr1a_free_10", true,   1, 101, fl_0, fl_0);
            test_function1_result_type("FuncPtr1a_free_10", true,   1, 101, f1a_1, f1a_0);
            test_function1_result_type("FuncPtr3a_free_11", true,   1, 101, f3a_1, f3a_0);
            test_function1_result_type("FuncPtr3a_free_11", true,   1, 101, f3a_1, f3a_1);
            test_function1_result_type("FuncPtr3a_free_12", true,   1, 101, f3a_1, f3a_2);
            test_function1_result_type("FuncPtr1a_free_10", false,  1, 101, f1a_1, f3a_1);
        }
        {
            // member, result non-void
            function<int(int)> f2a_0(this, &TestFunction01::func02a_member);
            fprintf(stderr, "memberA.0 %s\n", f2a_0.toString().c_str());
            REQUIRE( jau::func::target_type::member == f2a_0.type() );

            function<int(int)> f2a_1 = bind_member(this, &TestFunction01::func02a_member);
            fprintf(stderr, "memberA.1 %s\n", f2a_1.toString().c_str());
            REQUIRE( jau::func::target_type::member == f2a_1.type() );

            function<int(int)> f2a_2 = bind_member(this, &TestFunction01::func02a_member);
            function<int(int)> f2b_1 = bind_member(this, &TestFunction01::func02b_member);
            test_function0_result_type("FuncPtr2a_member_12", true,  1, 101, f2a_1, f2a_0);
            test_function0_result_type("FuncPtr2a_member_12", true,  1, 101, f2a_1, f2a_2);
            test_function0_result_type("FuncPtr2a_member_12", false, 1, 101, f2a_1, f2b_1);
        }
        {
            // member, result void
            function<void(int&, int)> f2a_0(this, &TestFunction01::func12a_member);
            function<void(int&, int)> f2a_1 = bind_member(this, &TestFunction01::func12a_member);
            function<void(int&, int)> f2a_2 = bind_member(this, &TestFunction01::func12a_member);
            function<void(int&, int)> f2b_1 = bind_member(this, &TestFunction01::func12b_member);
            test_function1_result_type("FuncPtr2a_member_12", true,  1, 101, f2a_1, f2a_0);
            test_function1_result_type("FuncPtr2a_member_12", true,  1, 101, f2a_1, f2a_2);
            test_function1_result_type("FuncPtr2a_member_12", false, 1, 101, f2a_1, f2b_1);
        }
        {
            // Lambda alike w/ explicit capture by value, result non-void
            int offset100 = 100;

            typedef int(*cfunc)(int&, int); // to force non-capturing lambda into a free function template type deduction

            int(*func5a_capture)(int&, int) = [](int& capture, int i)->int {
                int res = i+10000+capture;
                return res;
            };

            int(*func5b_capture)(int&, int) = [](int& capture, int i)->int {
                int res = i+100000+capture;
                return res;
            };

            function<int(int)> f5_o100_0(offset100,
                    (cfunc) ( [](int& capture, int i)->int {
                        int res = i+10000+capture;
                        return res;
                    } ) );
            fprintf(stderr, "capvalA.0 %s\n", f5_o100_0.toString().c_str());
            REQUIRE( jau::func::target_type::capval == f5_o100_0.type() );

            function<int(int)> f5_o100_1 = bind_capval(offset100,
                    (cfunc) ( [](int& capture, int i)->int {
                        int res = i+10000+capture;
                        return res;
                    } ) );
            function<int(int)> f5_o100_2 = bind_capval(offset100,
                    (cfunc) ( [](int& capture, int i)->int {
                        int res = i+10000+capture;
                        return res;
                    } ) );
            test_function0________type("FuncPtr5a_o100_capture_00", true,  f5_o100_0, f5_o100_0);
            test_function0________type("FuncPtr5a_o100_capture_00", true,  f5_o100_1, f5_o100_1);
            test_function0________type("FuncPtr5a_o100_capture_00", false, f5_o100_1, f5_o100_2);

            function<int(int)> f5a_o100_0(offset100, func5a_capture);
            fprintf(stderr, "capvalA.1 %s\n", f5a_o100_0.toString().c_str());
            REQUIRE( jau::func::target_type::capval == f5a_o100_0.type() );

            function<int(int)> f5a_o100_1 = bind_capval(offset100, func5a_capture);
            function<int(int)> f5a_o100_2 = bind_capval(offset100, func5a_capture);
            function<int(int)> f5b_o100_1 = bind_capval(offset100, func5b_capture);
            test_function0________type("FuncPtr5a_o100_capture_12", true,  f5a_o100_1, f5a_o100_0);
            test_function0________type("FuncPtr5a_o100_capture_12", true,  f5a_o100_1, f5a_o100_2);
            test_function0________type("FuncPtr5a_o100_capture_12", false, f5a_o100_1, f5b_o100_1);
            test_function0_result_type("FuncPtr5a_o100_capture_11", true,  1, 10101, f5a_o100_1, f5a_o100_1);
            test_function0_result_type("FuncPtr5a_o100_capture_12", true,  1, 10101, f5a_o100_1, f5a_o100_2);
            test_function0_result_type("FuncPtr5a_o100_capture_12", false, 1, 10101, f5a_o100_1, f5b_o100_1);
        }
        {
            // Lambda alike w/ explicit capture by reference, result non-void
            IntOffset offset100(100);

            typedef int(*cfunc)(IntOffset*, int); // to force non-capturing lambda into a free function template type deduction

            int(*func7a_capture)(IntOffset*, int) = [](IntOffset* capture, int i)->int {
                int res = i+10000+capture->value;
                return res;
            };
            int(*func7b_capture)(IntOffset*, int) = [](IntOffset* capture, int i)->int {
                int res = i+100000+capture->value;
                return res;
            };

            function<int(int)> f7_o100_1 = bind_capref<int, IntOffset, int>(&offset100,
                    (cfunc) ( [](IntOffset* capture, int i)->int {
                        int res = i+10000+capture->value;
                        return res;;
                    } ) );
            fprintf(stderr, "caprefA.0 %s\n", f7_o100_1.toString().c_str());
            REQUIRE( jau::func::target_type::capref == f7_o100_1.type() );

            function<int(int)> f7_o100_2 = bind_capref<int, IntOffset, int>(&offset100,
                    (cfunc) ( [](IntOffset* capture, int i)->int {
                        int res = i+10000+capture->value;
                        return res;;
                    } ) );
            test_function0________type("FuncPtr7a_o100_capture_00", true,  f7_o100_1, f7_o100_1);
            test_function0________type("FuncPtr7a_o100_capture_00", false, f7_o100_1, f7_o100_2);

            function<int(int)> f7a_o100_1 = bind_capref(&offset100, func7a_capture);
            fprintf(stderr, "caprefA.1 %s\n", f7a_o100_1.toString().c_str());
            REQUIRE( jau::func::target_type::capref == f7a_o100_1.type() );
            function<int(int)> f7a_o100_2 = bind_capref(&offset100, func7a_capture);
            function<int(int)> f7b_o100_1 = bind_capref(&offset100, func7b_capture);
            test_function0________type("FuncPtr7a_o100_capture_12", true,  f7a_o100_1, f7a_o100_2);
            test_function0________type("FuncPtr7a_o100_capture_12", false, f7a_o100_1, f7b_o100_1);
            test_function0_result_type("FuncPtr7a_o100_capture_11", true,  1, 10101, f7a_o100_1, f7a_o100_1);
            test_function0_result_type("FuncPtr7a_o100_capture_12", true,  1, 10101, f7a_o100_1, f7a_o100_2);
            test_function0_result_type("FuncPtr7a_o100_capture_12", false, 1, 10101, f7a_o100_1, f7b_o100_1);
        }
        {
            // std::function lambda
            std::function<int(int i)> func4a_stdlambda = [](int i)->int {
                int res = i+100;
                return res;;
            };
            std::function<int(int i)> func4b_stdlambda = [](int i)->int {
                int res = i+1000;
                return res;;
            };
            function<int(int)> f4a_1 = bind_std(100, func4a_stdlambda);
            fprintf(stderr, "stdfunc.0 %s\n", f4a_1.toString().c_str());
            REQUIRE( jau::func::target_type::std == f4a_1.type() );

            function<int(int)> f4a_2 = bind_std(100, func4a_stdlambda);
            test_function0_result_type("FuncPtr4a_stdlambda_11", true, 1, 101, f4a_1, f4a_1);
            test_function0_result_type("FuncPtr4a_stdlambda_12", true, 1, 101, f4a_1, f4a_2);
        }

        INFO("Test 00_usage: END");
    }

    void test01_memberfunc_this() {
        INFO("Test 01_member: bind_member<int, TestFunction01, int>: START");
        {
            // function(TestFunction01 &base, Func1Type func)
            MyClassFunction0 f2a_1 = bind_member<int, TestFunction01, int>(this, &TestFunction01::func02a_member);
            MyClassFunction0 f2a_2 = bind_member(this, &TestFunction01::func02a_member);
            test_function0_result_type("FuncPtr2a_member_11", true, 1, 101, f2a_1, f2a_1);
            test_function0_result_type("FuncPtr2a_member_12", true, 1, 101, f2a_1, f2a_2);

            MyClassFunction0 f2b_1 = bind_member(this, &TestFunction01::func02b_member);
            MyClassFunction0 f2b_2 = bind_member(this, &TestFunction01::func02b_member);
            test_function0_result_type("FuncPtr2b_member_11", true, 1, 1001, f2b_1, f2b_1);
            test_function0_result_type("FuncPtr2b_member_12", true, 1, 1001, f2b_1, f2b_2);

            test_function0_result_type("FuncPtr2ab_member_11", false, 1, 0, f2a_1, f2b_1);
            test_function0_result_type("FuncPtr2ab_member_22", false, 1, 0, f2a_2, f2b_2);
        }

        {
            std::string msg = "member01_c1";

            struct c1_t {
                int offset;

                int f(int i) noexcept {
                    int res = i+offset;
                    return res;
                }
            };
            c1_t c_1a {  100 };
            c1_t c_1b {  100 };
            function<int(int)> f_1a(&c_1a, &c1_t::f);
            function<int(int)> f_1b(&c_1b, &c1_t::f);
            fprintf(stderr, "%s 1a %s\n", msg.c_str(), f_1a.toString().c_str());
            REQUIRE( jau::func::target_type::member == f_1a.type() );
            fprintf(stderr, "%s 1b %s\n", msg.c_str(), f_1b.toString().c_str());
            REQUIRE( jau::func::target_type::member == f_1b.type() );

            c1_t c_2a { 1000 };
            c1_t c_2b { 1000 };
            function<int(int)> f_2a(&c_2a, &c1_t::f);
            function<int(int)> f_2b(&c_2b, &c1_t::f);
            fprintf(stderr, "%s 2a %s\n", msg.c_str(), f_2a.toString().c_str());
            REQUIRE( jau::func::target_type::member == f_2a.type() );
            fprintf(stderr, "%s 2b %s\n", msg.c_str(), f_2b.toString().c_str());
            REQUIRE( jau::func::target_type::member == f_2b.type() );

            test_function0_result_____(msg+" 1aa", 1,  101, f_1a, f_1a);
            test_function0_result_____(msg+" 1ab", 1,  101, f_1a, f_1b);
            test_function0________type(msg+" 1aa", true,    f_1a, f_1a);
            test_function0________type(msg+" 1ab", false,   f_1a, f_1b);

            test_function0_result_____(msg+" 2aa", 1, 1001, f_2a, f_2a);
            test_function0_result_____(msg+" 2ab", 1, 1001, f_2a, f_2b);
            test_function0________type(msg+" 2aa", true,    f_2a, f_2a);
            test_function0________type(msg+" 2ab", false,   f_2a, f_2b);
        }

        {
            struct c1_t {
                int offset;

                c1_t() : offset(10) {}
                c1_t(int v) : offset(v) {}

                int f(int i) noexcept {
                    int res = i+offset; /** (B) EXPECTED if c2_t is referenced. **/
                    return res;
                }
            };

            struct c2_t : public c1_t {
                c2_t() : c1_t() {}
                c2_t(int v) : c1_t(v) {}

                int f(int i) noexcept {
                    int res = i+1000; /** (A) EXPECTED if c2_t is referenced. **/
                    return res;
                }
            };

            /**
             * (A) Create a function delegate using c2_t spec and c2_t reference for actual c2_t instance,
             * expect to use c2_t function definition!
             */
            {
                std::string msg = "member02_func_c2";

                c2_t c_1a ( 100 );
                c2_t c_1b ( 100 );

                function<int(int)> f_1a(&c_1a, &c2_t::f);
                function<int(int)> f_1b(&c_1b, &c2_t::f);
                fprintf(stderr, "%s 1a %s\n", msg.c_str(), f_1a.toString().c_str());
                REQUIRE( jau::func::target_type::member == f_1a.type() );
                fprintf(stderr, "%s 1b %s\n", msg.c_str(), f_1b.toString().c_str());
                REQUIRE( jau::func::target_type::member == f_1b.type() );

                test_function0_result_____(msg+" 1aa", 1, 1001, f_1a, f_1a);
                test_function0_result_____(msg+" 1ab", 1, 1001, f_1a, f_1b);
                test_function0________type(msg+" 1aa", true,    f_1a, f_1a);
                test_function0________type(msg+" 1ab", false,   f_1a, f_1b);
            }

            /**
             * (B) Create a function delegate using c1_t spec and c1_t reference for actual c2_t instance,
             * expect to use c1_t function definition!
             */
            {
                std::string msg = "member03_func_c1_ref";

                c2_t c_1a_ ( 100 );
                c2_t c_1b_ ( 100 );
                c1_t& c_1a = c_1a_;
                c1_t& c_1b = c_1b_;

                function<int(int)> f_1a(&c_1a, &c1_t::f);
                function<int(int)> f_1b(&c_1b, &c1_t::f);
                fprintf(stderr, "%s 1a %s\n", msg.c_str(), f_1a.toString().c_str());
                REQUIRE( jau::func::target_type::member == f_1a.type() );
                fprintf(stderr, "%s 1b %s\n", msg.c_str(), f_1b.toString().c_str());
                REQUIRE( jau::func::target_type::member == f_1b.type() );

                test_function0_result_____(msg+" 1aa", 1,  101, f_1a, f_1a);
                test_function0_result_____(msg+" 1ab", 1,  101, f_1a, f_1b);
                test_function0________type(msg+" 1aa", true,    f_1a, f_1a);
                test_function0________type(msg+" 1ab", false,   f_1a, f_1b);
            }
        }

        {
            struct c1_t {
                int offset; /** (A) EXPECTED if c1_t is referenced. **/

                c1_t() : offset(10) {}

                int f(int i) noexcept {
                    int res = i+offset;
                    return res;
                }
            };

            struct c2_t : public c1_t {
                int offset; /** (B) EXPECTED if c2_t is referenced. **/

                c2_t() : c1_t(), offset(20) {}
                c2_t(int v) : c1_t(), offset(v) {}
            };

            struct c3_t : public c2_t {
                c3_t() : c2_t() {}
                c3_t(int v) : c2_t(v) {}
            };

            /**
             * (0) Compile error, since given this base-pointer type c4_t (C1)
             *     is not derived from type c1_t (C0) holding the member-function.
             */
            {
#if 0
                struct c4_t {
                };
                c4_t c_1a;
                function<int(int)> f_1a(&c_1a, &c1_t::f);
#endif
            }

            /**
             * (A) Create a function delegate using c2_t spec and c2_t reference for actual c2_t instance,
             * expect to use c1_t offset member!
             */
            {
                std::string msg = "member04_field_c2";

                c2_t c_1a( 1000 );
                c3_t c_1b( 1000 );

                REQUIRE( 1000 == c_1a.offset);
                fprintf(stderr, "%s offset: c2_t %d\n", msg.c_str(), c_1a.offset);

                function<int(int)> f_1a(&c_1a, &c1_t::f);
                function<int(int)> f_1b(&c_1b, &c1_t::f);
                fprintf(stderr, "%s 1a %s\n", msg.c_str(), f_1a.toString().c_str());
                REQUIRE( jau::func::target_type::member == f_1a.type() );
                fprintf(stderr, "%s 1b %s\n", msg.c_str(), f_1b.toString().c_str());
                REQUIRE( jau::func::target_type::member == f_1b.type() );

                test_function0_result_____(msg+" 1aa", 1,   11, f_1a, f_1a);
                test_function0_result_____(msg+" 1ab", 1,   11, f_1a, f_1b);
                test_function0________type(msg+" 1aa", true,    f_1a, f_1a);
                test_function0________type(msg+" 1ab", false,   f_1a, f_1b);
            }
            /**
             * (B) Create a function delegate using c1_t spec and c1_t reference for actual c2_t instance,
             * expect to use c1_t offset member!
             */
            {
                std::string msg = "member05_field_c1_ref";

                c2_t c_1a_( 1000 );
                c3_t c_1b_( 1000 );
                c1_t& c_1a = c_1a_;
                c1_t& c_1b = c_1b_;

                REQUIRE( 1000 == c_1a_.offset);
                REQUIRE(   10 == c_1a.offset);
                fprintf(stderr, "%s offset: c2_t %d, c1_t ref %d\n", msg.c_str(), c_1a_.offset, c_1a.offset);

                function<int(int)> f_1a(&c_1a, &c1_t::f);
                function<int(int)> f_1b(&c_1b, &c1_t::f);
                fprintf(stderr, "%s 1a %s\n", msg.c_str(), f_1a.toString().c_str());
                REQUIRE( jau::func::target_type::member == f_1a.type() );
                fprintf(stderr, "%s 1b %s\n", msg.c_str(), f_1b.toString().c_str());
                REQUIRE( jau::func::target_type::member == f_1b.type() );

                test_function0_result_____(msg+" 1aa", 1,   11, f_1a, f_1a);
                test_function0_result_____(msg+" 1ab", 1,   11, f_1a, f_1b);
                test_function0________type(msg+" 1aa", true,    f_1a, f_1a);
                test_function0________type(msg+" 1ab", false,   f_1a, f_1b);
            }
        }

        /**
         * Create a function delegate using c1_t spec and c1_t reference for actual c2_t instance,
         * expect to use c2_t virtual override!
         */
        {
            std::string msg = "member06_vfunc_c1_ref";

            struct c1_t {
                int offset;

                c1_t() : offset(10) {}
                c1_t(int v) : offset(v) {}

                virtual ~c1_t() noexcept {}

                virtual int f(int i) noexcept {
                    int res = i+offset;
                    return res;
                }
            };

            struct c2_t : public c1_t {
                c2_t() : c1_t() {}
                c2_t(int v) : c1_t(v) {}

                int f(int i) noexcept override {
                    int res = i+1000;
                    return res;
                }
            };
            c2_t c_1a_( 100 );
            c2_t c_1b_( 100 );
            c1_t& c_1a = c_1a_;
            c1_t& c_1b = c_1b_;

            function<int(int)> f_1a(&c_1a, &c1_t::f);
            function<int(int)> f_1b(&c_1b, &c1_t::f);
            fprintf(stderr, "%s 1a %s\n", msg.c_str(), f_1a.toString().c_str());
            REQUIRE( jau::func::target_type::member == f_1a.type() );
            fprintf(stderr, "%s 1b %s\n", msg.c_str(), f_1b.toString().c_str());
            REQUIRE( jau::func::target_type::member == f_1b.type() );

            test_function0_result_____(msg+" 1aa", 1, 1001, f_1a, f_1a);
            test_function0_result_____(msg+" 1ab", 1, 1001, f_1a, f_1b);
            test_function0________type(msg+" 1aa", true,    f_1a, f_1a);
            test_function0________type(msg+" 1ab", false,   f_1a, f_1b);
        }
        INFO("Test 01_member: bind_member<int, TestFunction01, int>: END");
    }

    void test11_memberfunc_this() {
        INFO("Test 11_member: bind_member<int, TestFunction01, int>: START");
        // function(TestFunction01 &base, Func1Type func)
        MyClassFunction1 f2a_1 = bind_member<TestFunction01, int&, int>(this, &TestFunction01::func12a_member);
        MyClassFunction1 f2a_2 = bind_member(this, &TestFunction01::func12a_member);
        test_function1_result_type("FuncPtr2a_member_11", true, 1, 101, f2a_1, f2a_1);
        test_function1_result_type("FuncPtr2a_member_12", true, 1, 101, f2a_1, f2a_2);

        MyClassFunction1 f2b_1 = bind_member(this, &TestFunction01::func12b_member);
        MyClassFunction1 f2b_2 = bind_member(this, &TestFunction01::func12b_member);
        test_function1_result_type("FuncPtr2b_member_11", true, 1, 1001, f2b_1, f2b_1);
        test_function1_result_type("FuncPtr2b_member_12", true, 1, 1001, f2b_1, f2b_2);

        test_function1_result_type("FuncPtr2ab_member_11", false, 1, 0, f2a_1, f2b_1);
        test_function1_result_type("FuncPtr2ab_member_22", false, 1, 0, f2a_2, f2b_2);
        INFO("Test 11_member: bind_member<int, TestFunction01, int>: END");
    }

    void test02_freefunc_static() {
        INFO("Test 02_free: bind_free<int, int>: START");
        // function(Func1Type func)
        MyClassFunction0 f1a_1 = bind_free<int, int>(Func0a_free);
        MyClassFunction0 f3a_1 = bind_free<int, int>(&TestFunction01::Func03a_static);
        MyClassFunction0 f3a_2 = bind_free(&TestFunction01::Func03a_static);
        test_function0_result_type("FuncPtr1a_free_10", true,  1, 101, f1a_1, f1a_1);
        test_function0_result_type("FuncPtr3a_free_11", true,  1, 101, f3a_1, f3a_1);
        test_function0_result_type("FuncPtr3a_free_12", true,  1, 101, f3a_1, f3a_2);

        MyClassFunction0 f3b_1 = bind_free(&TestFunction01::Func03b_static);
        MyClassFunction0 f3b_2 = bind_free(&Func03b_static);
        test_function0_result_type("FuncPtr3b_free_11", true, 1, 1001, f3b_1, f3b_1);
        test_function0_result_type("FuncPtr3b_free_12", true, 1, 1001, f3b_1, f3b_2);

        test_function0_result_type("FuncPtr1a3a_free_10", false, 1, 0, f1a_1, f3a_1);
        test_function0_result_type("FuncPtr1a3b_free_10", false, 1, 0, f1a_1, f3b_1);
        test_function0_result_type("FuncPtr3a3b_free_11", false, 1, 0, f3a_1, f3b_1);
        test_function0_result_type("FuncPtr3a3b_free_22", false, 1, 0, f3a_2, f3b_2);
        INFO("Test 02_free: bind_free<int, int>: END");
    }

    void test12_freefunc_static() {
        INFO("Test 12_free: bind_free<int, int>: START");
        // function(Func1Type func)
        MyClassFunction1 f1a_1 = bind_free<int&, int>(Func1a_free);
        MyClassFunction1 f3a_1 = bind_free<int&, int>(&TestFunction01::Func13a_static);
        MyClassFunction1 f3a_2 = bind_free(&TestFunction01::Func13a_static);
        test_function1_result_type("FuncPtr1a_free_10", true,  1, 101, f1a_1, f1a_1);
        test_function1_result_type("FuncPtr3a_free_11", true,  1, 101, f3a_1, f3a_1);
        test_function1_result_type("FuncPtr3a_free_12", true,  1, 101, f3a_1, f3a_2);

        MyClassFunction1 f3b_1 = bind_free(&TestFunction01::Func13b_static);
        MyClassFunction1 f3b_2 = bind_free(&Func13b_static);
        test_function1_result_type("FuncPtr3b_free_11", true, 1, 1001, f3b_1, f3b_1);
        test_function1_result_type("FuncPtr3b_free_12", true, 1, 1001, f3b_1, f3b_2);

        test_function1_result_type("FuncPtr1a3a_free_10", false, 1, 0, f1a_1, f3a_1);
        test_function1_result_type("FuncPtr1a3b_free_10", false, 1, 0, f1a_1, f3b_1);
        test_function1_result_type("FuncPtr3a3b_free_11", false, 1, 0, f3a_1, f3b_1);
        test_function1_result_type("FuncPtr3a3b_free_22", false, 1, 0, f3a_2, f3b_2);
        INFO("Test 12_free: bind_free<int, int>: END");
    }

    void test03_stdfunc_lambda() {
        INFO("Test 03_stdlambda: bind_std<int, int>: START");
        // function(Func1Type func) <int, int>
        std::function<int(int i)> func4a_stdlambda = [](int i)->int {
            int res = i+100;
            return res;;
        };
        jau::type_cue<std::function<int(int i)>>::print("std::function<int(int i)> type", TypeTraitGroup::ALL);

        std::function<int(int i)> func4b_stdlambda = [](int i)->int {
            int res = i+1000;
            return res;;
        };
        MyClassFunction0 f4a_1 = bind_std<int, int>(100, func4a_stdlambda);
        MyClassFunction0 f4a_2 = bind_std(100, func4a_stdlambda);
        test_function0_result_type("FuncPtr4a_stdlambda_11", true, 1, 101, f4a_1, f4a_1);
        test_function0_result_type("FuncPtr4a_stdlambda_12", true, 1, 101, f4a_1, f4a_2);

        MyClassFunction0 f4b_1 = bind_std(200, func4b_stdlambda);
        MyClassFunction0 f4b_2 = bind_std(200, func4b_stdlambda);
        test_function0_result_type("FuncPtr4b_stdlambda_11", true, 1, 1001, f4b_1, f4b_1);
        test_function0_result_type("FuncPtr4b_stdlambda_12", true, 1, 1001, f4b_1, f4b_2);

        test_function0_result_type("FuncPtr4ab_stdlambda_11", false, 1, 0, f4a_1, f4b_1);
        test_function0_result_type("FuncPtr4ab_stdlambda_22", false, 1, 0, f4a_2, f4b_2);

        INFO("Test 03_stdlambda: bind_std<int, int>: END");
    }

    void test13_stdfunc_lambda() {
        INFO("Test 13_stdlambda: bind_std<int, int>: START");
        // function(Func1Type func) <int, int>
        std::function<void(int& r, int i)> func4a_stdlambda = [](int& r, int i)->void {
            r = i+100;
        };
        jau::type_cue<std::function<void(int& r, int i)>>::print("std::function<int(int i)> type", TypeTraitGroup::ALL);

        std::function<void(int& r, int i)> func4b_stdlambda = [](int& r, int i)->void {
            r = i+1000;
        };
        MyClassFunction1 f4a_1 = bind_std<int&, int>(100, func4a_stdlambda);
        MyClassFunction1 f4a_2 = bind_std(100, func4a_stdlambda);
        test_function1_result_type("FuncPtr4a_stdlambda_11", true, 1, 101, f4a_1, f4a_1);
        test_function1_result_type("FuncPtr4a_stdlambda_12", true, 1, 101, f4a_1, f4a_2);

        MyClassFunction1 f4b_1 = bind_std(200, func4b_stdlambda);
        MyClassFunction1 f4b_2 = bind_std(200, func4b_stdlambda);
        test_function1_result_type("FuncPtr4b_stdlambda_11", true, 1, 1001, f4b_1, f4b_1);
        test_function1_result_type("FuncPtr4b_stdlambda_12", true, 1, 1001, f4b_1, f4b_2);

        test_function1_result_type("FuncPtr4ab_stdlambda_11", false, 1, 0, f4a_1, f4b_1);
        test_function1_result_type("FuncPtr4ab_stdlambda_22", false, 1, 0, f4a_2, f4b_2);

        INFO("Test 13_stdlambda: bind_std<int, int>: END");
    }

    void test04_capval_lambda() {
        INFO("Test 04_capval: bindCapture<int, int, int>: START");
        // bindCapture(I& data, R(*func)(I&, A...))
        // function(Func1Type func) <int, int>
        int offset100 = 100;
        int offset1000 = 1000;

        typedef int(*cfunc)(int&, int); // to force non-capturing lambda into a free function template type deduction

        int(*func5a_capture)(int&, int) = [](int& capture, int i)->int {
            int res = i+10000+capture;
            return res;
        };
        int(*func5b_capture)(int&, int) = [](int& capture, int i)->int {
            int res = i+100000+capture;
            return res;
        };

        MyClassFunction0 f5a_o100_0 = bind_capval<int, int, int>(offset100,
                (cfunc) ( [](int& capture, int i)->int {
                    int res = i+10000+capture;
                    return res;;
                } ) );
        test_function0________type("FuncPtr5a_o100_capture_00", true, f5a_o100_0, f5a_o100_0);

        MyClassFunction0 f5a_o100_1 = bind_capval<int, int, int>(offset100, func5a_capture);
        MyClassFunction0 f5a_o100_2 = bind_capval(offset100, func5a_capture);
        test_function0________type("FuncPtr5a_o100_capture_12", true, f5a_o100_1, f5a_o100_2);
        test_function0_result_type("FuncPtr5a_o100_capture_11", true, 1, 10101, f5a_o100_1, f5a_o100_1);
        test_function0_result_type("FuncPtr5a_o100_capture_12", true, 1, 10101, f5a_o100_1, f5a_o100_2);
        // test_FunctionPointer01("FuncPtr5a_o100_capture_01", false, f5a_o100_0, f5a_o100_1);
        MyClassFunction0 f5a_o1000_1 = bind_capval(offset1000, func5a_capture);
        MyClassFunction0 f5a_o1000_2 = bind_capval(offset1000, func5a_capture);
        test_function0________type("FuncPtr5a_o1000_capture_12", true, f5a_o1000_1, f5a_o1000_2);
        test_function0________type("FuncPtr5a_o100_o1000_capture_11", false, f5a_o100_1, f5a_o1000_1);

        MyClassFunction0 f5b_o100_1 = bind_capval(offset100, func5b_capture);
        MyClassFunction0 f5b_o100_2 = bind_capval(offset100, func5b_capture);
        test_function0_result_type("FuncPtr5b_o100_capture_11", true, 1, 100101, f5b_o100_1, f5b_o100_1);
        test_function0_result_type("FuncPtr5b_o100_capture_12", true, 1, 100101, f5b_o100_1, f5b_o100_2);

        test_function0_result_type("FuncPtr5ab_o100_capture_11", false, 1, 0, f5a_o100_1, f5b_o100_1);
        test_function0_result_type("FuncPtr5ab_o100_capture_22", false, 1, 0, f5a_o100_2, f5b_o100_2);
        INFO("Test 04_capval: bindCapture<int, int, int>: END");
    }

    void test14_capval_lambda() {
        INFO("Test 14_capval: bindCapture<int, int, int>: START");
        // bindCapture(I& data, R(*func)(I&, A...))
        // function(Func1Type func) <int, int>
        int offset100 = 100;
        int offset1000 = 1000;

        typedef void(*cfunc)(int&, int&, int); // to force non-capturing lambda into a free function template type deduction

        void(*func5a_capture)(int&, int&, int) = [](int& capture, int& res, int i)->void {
            res = i+10000+capture;
        };
        void(*func5b_capture)(int&, int&, int) = [](int& capture, int& res, int i)->void {
            res = i+100000+capture;
        };

        MyClassFunction1 f5a_o100_0 = bind_capval<int, int&, int>(offset100,
                (cfunc) ( [](int& capture, int& res, int i)->void {
                    res = i+10000+capture;
                } ) );
        test_function1________type("FuncPtr5a_o100_capture_00", true, f5a_o100_0, f5a_o100_0);

        MyClassFunction1 f5a_o100_1 = bind_capval<int, int&, int>(offset100, func5a_capture);
        MyClassFunction1 f5a_o100_2 = bind_capval(offset100, func5a_capture);
        test_function1________type("FuncPtr5a_o100_capture_12", true, f5a_o100_1, f5a_o100_2);
        test_function1_result_type("FuncPtr5a_o100_capture_11", true, 1, 10101, f5a_o100_1, f5a_o100_1);
        test_function1_result_type("FuncPtr5a_o100_capture_12", true, 1, 10101, f5a_o100_1, f5a_o100_2);
        // test_FunctionPointer01("FuncPtr5a_o100_capture_01", false, f5a_o100_0, f5a_o100_1);
        MyClassFunction1 f5a_o1000_1 = bind_capval(offset1000, func5a_capture);
        MyClassFunction1 f5a_o1000_2 = bind_capval(offset1000, func5a_capture);
        test_function1________type("FuncPtr5a_o1000_capture_12", true, f5a_o1000_1, f5a_o1000_2);
        test_function1________type("FuncPtr5a_o100_o1000_capture_11", false, f5a_o100_1, f5a_o1000_1);

        MyClassFunction1 f5b_o100_1 = bind_capval(offset100, func5b_capture);
        MyClassFunction1 f5b_o100_2 = bind_capval(offset100, func5b_capture);
        test_function1_result_type("FuncPtr5b_o100_capture_11", true, 1, 100101, f5b_o100_1, f5b_o100_1);
        test_function1_result_type("FuncPtr5b_o100_capture_12", true, 1, 100101, f5b_o100_1, f5b_o100_2);

        test_function1_result_type("FuncPtr5ab_o100_capture_11", false, 1, 0, f5a_o100_1, f5b_o100_1);
        test_function1_result_type("FuncPtr5ab_o100_capture_22", false, 1, 0, f5a_o100_2, f5b_o100_2);
        INFO("Test 14_capval: bindCapture<int, int, int>: END");
    }

    void test05_capval_lambda() {
        INFO("Test 05_capval: bindCapture<int, std::shared_ptr<IntOffset>, int>: START");
        // bindCapture(I& data, R(*func)(I&, A...))
        // function(Func1Type func) <int, int>
        std::shared_ptr<IntOffset> offset100(new IntOffset(100));
        std::shared_ptr<IntOffset> offset1000(new IntOffset(1000));

        typedef int(*cfunc)(std::shared_ptr<IntOffset>&, int); // to force non-capturing lambda into a free function template type deduction

        int(*func6a_capture)(std::shared_ptr<IntOffset>&, int) = [](std::shared_ptr<IntOffset>& capture, int i)->int {
            int res = i+10000+capture->value;
            return res;
        };
        int(*func6b_capture)(std::shared_ptr<IntOffset>&, int) = [](std::shared_ptr<IntOffset>& capture, int i)->int {
            int res = i+100000+capture->value;
            return res;
        };

        MyClassFunction0 f6a_o100_0 = bind_capval<int, std::shared_ptr<IntOffset>, int>(offset100,
                (cfunc) ( [](std::shared_ptr<IntOffset>& sharedOffset, int i)->int {
                    int res = i+10000+sharedOffset->value;
                    return res;;
                } ) );
        test_function0________type("FuncPtr6a_o100_capture_00", true, f6a_o100_0, f6a_o100_0);

        MyClassFunction0 f6a_o100_1 = bind_capval<int, std::shared_ptr<IntOffset>, int>(offset100, func6a_capture);
        MyClassFunction0 f6a_o100_2 = bind_capval(offset100, func6a_capture);
        test_function0________type("FuncPtr6a_o100_capture_12", true, f6a_o100_1, f6a_o100_2);
        test_function0_result_type("FuncPtr6a_o100_capture_11", true, 1, 10101, f6a_o100_1, f6a_o100_1);
        test_function0_result_type("FuncPtr6a_o100_capture_12", true, 1, 10101, f6a_o100_1, f6a_o100_2);
        // test_FunctionPointer01("FuncPtr6a_o100_capture_01", false, f6a_o100_0, f6a_o100_1);
        MyClassFunction0 f6a_o1000_1 = bind_capval(offset1000, func6a_capture);
        MyClassFunction0 f6a_o1000_2 = bind_capval(offset1000, func6a_capture);
        test_function0________type("FuncPtr6a_o1000_capture_12", true, f6a_o1000_1, f6a_o1000_2);
        test_function0________type("FuncPtr6a_o100_o1000_capture_11", false, f6a_o100_1, f6a_o1000_1);

        MyClassFunction0 f6b_o100_1 = bind_capval(offset100, func6b_capture);
        MyClassFunction0 f6b_o100_2 = bind_capval(offset100, func6b_capture);
        test_function0_result_type("FuncPtr6b_o100_capture_11", true, 1, 100101, f6b_o100_1, f6b_o100_1);
        test_function0_result_type("FuncPtr6b_o100_capture_12", true, 1, 100101, f6b_o100_1, f6b_o100_2);

        test_function0_result_type("FuncPtr6ab_o100_capture_11", false, 1, 0, f6a_o100_1, f6b_o100_1);
        test_function0_result_type("FuncPtr6ab_o100_capture_22", false, 1, 0, f6a_o100_2, f6b_o100_2);
        INFO("Test 05_capval: bindCapture<int, std::shared_ptr<IntOffset>, int>: END");
    }

    void test06_capval_lambda() {
        INFO("Test 06_capval: bindCapture<int, IntOffset, int>: START");
        // bindCapture(I& data, R(*func)(I&, A...))
        // function(Func1Type func) <int, int>
        IntOffset offset100(100);
        IntOffset offset1000(1000);

        typedef int(*cfunc)(IntOffset&, int); // to force non-capturing lambda into a free function template type deduction

        int(*func7a_capture)(IntOffset&, int) = [](IntOffset& capture, int i)->int {
            int res = i+10000+capture.value;
            return res;
        };
        int(*func7b_capture)(IntOffset&, int) = [](IntOffset& capture, int i)->int {
            int res = i+100000+capture.value;
            return res;
        };

        MyClassFunction0 f7a_o100_0 = bind_capval<int, IntOffset, int>(offset100,
                (cfunc) ( [](IntOffset& capture, int i)->int {
                    int res = i+10000+capture.value;
                    return res;;
                } ) );
        test_function0________type("FuncPtr7a_o100_capture_00", true, f7a_o100_0, f7a_o100_0);

        INFO("f7a_o100_1 copy_ctor");
        MyClassFunction0 f7a_o100_1 = bind_capval<int, IntOffset, int>(offset100, func7a_capture);
        INFO("f7a_o100_1 copy_ctor done");
        INFO("f7a_o100_2 move_ctor");
        MyClassFunction0 f7a_o100_2 = bind_capval(IntOffset(100), func7a_capture);
        INFO("f7a_o100_2 move_ctor done");
        test_function0________type("FuncPtr7a_o100_capture_12", true, f7a_o100_1, f7a_o100_2);
        test_function0_result_type("FuncPtr7a_o100_capture_11", true, 1, 10101, f7a_o100_1, f7a_o100_1);
        test_function0_result_type("FuncPtr7a_o100_capture_12", true, 1, 10101, f7a_o100_1, f7a_o100_2);
        // test_FunctionPointer01("FuncPtr7a_o100_capture_01", false, f7a_o100_0, f7a_o100_1);
        MyClassFunction0 f7a_o1000_1 = bind_capval(offset1000, func7a_capture);
        MyClassFunction0 f7a_o1000_2 = bind_capval(offset1000, func7a_capture);
        test_function0________type("FuncPtr7a_o1000_capture_12", true, f7a_o1000_1, f7a_o1000_2);
        test_function0________type("FuncPtr7a_o100_o1000_capture_11", false, f7a_o100_1, f7a_o1000_1);

        MyClassFunction0 f7b_o100_1 = bind_capval(offset100, func7b_capture);
        MyClassFunction0 f7b_o100_2 = bind_capval(offset100, func7b_capture);
        test_function0_result_type("FuncPtr7b_o100_capture_11", true, 1, 100101, f7b_o100_1, f7b_o100_1);
        test_function0_result_type("FuncPtr7b_o100_capture_12", true, 1, 100101, f7b_o100_1, f7b_o100_2);

        test_function0_result_type("FuncPtr7ab_o100_capture_11", false, 1, 0, f7a_o100_1, f7b_o100_1);
        test_function0_result_type("FuncPtr7ab_o100_capture_22", false, 1, 0, f7a_o100_2, f7b_o100_2);
        INFO("Test 06_capval: bindCapture<int, IntOffset, int>: END");
    }

    void test07_capref_lambda() {
        INFO("Test 07_capref: bindCapture<int, IntOffset, int>: START");
        // bindCapture(I& data, R(*func)(I&, A...))
        // function(Func1Type func) <int, int>
        IntOffset offset100(100);
        IntOffset offset1000(1000);

        typedef int(*cfunc)(IntOffset*, int); // to force non-capturing lambda into a free function template type deduction

        int(*func7a_capture)(IntOffset*, int) = [](IntOffset* capture, int i)->int {
            int res = i+10000+capture->value;
            return res;
        };
        int(*func7b_capture)(IntOffset*, int) = [](IntOffset* capture, int i)->int {
            int res = i+100000+capture->value;
            return res;
        };

        MyClassFunction0 f7a_o100_0 = bind_capref<int, IntOffset, int>(&offset100,
                (cfunc) ( [](IntOffset* capture, int i)->int {
                    int res = i+10000+capture->value;
                    return res;;
                } ) );
        test_function0________type("FuncPtr7a_o100_capture_00", true, f7a_o100_0, f7a_o100_0);

        INFO("f7a_o100_1 copy_ctor");
        MyClassFunction0 f7a_o100_1 = bind_capref<int, IntOffset, int>(&offset100, func7a_capture);
        INFO("f7a_o100_1 copy_ctor done");
        INFO("f7a_o100_2 move_ctor");
        MyClassFunction0 f7a_o100_2 = bind_capref(&offset100, func7a_capture);
        INFO("f7a_o100_2 move_ctor done");
        test_function0________type("FuncPtr7a_o100_capture_12", true, f7a_o100_1, f7a_o100_2);
        test_function0_result_type("FuncPtr7a_o100_capture_11", true, 1, 10101, f7a_o100_1, f7a_o100_1);
        test_function0_result_type("FuncPtr7a_o100_capture_12", true, 1, 10101, f7a_o100_1, f7a_o100_2);
        // test_FunctionPointer01("FuncPtr7a_o100_capture_01", false, f7a_o100_0, f7a_o100_1);
        MyClassFunction0 f7a_o1000_1 = bind_capref(&offset1000, func7a_capture);
        MyClassFunction0 f7a_o1000_2 = bind_capref(&offset1000, func7a_capture);
        test_function0________type("FuncPtr7a_o1000_capture_12", true, f7a_o1000_1, f7a_o1000_2);
        test_function0________type("FuncPtr7a_o100_o1000_capture_11", false, f7a_o100_1, f7a_o1000_1);

        MyClassFunction0 f7b_o100_1 = bind_capref(&offset100, func7b_capture);
        MyClassFunction0 f7b_o100_2 = bind_capref(&offset100, func7b_capture);
        test_function0_result_type("FuncPtr7b_o100_capture_11", true, 1, 100101, f7b_o100_1, f7b_o100_1);
        test_function0_result_type("FuncPtr7b_o100_capture_12", true, 1, 100101, f7b_o100_1, f7b_o100_2);

        test_function0_result_type("FuncPtr7ab_o100_capture_11", false, 1, 0, f7a_o100_1, f7b_o100_1);
        test_function0_result_type("FuncPtr7ab_o100_capture_22", false, 1, 0, f7a_o100_2, f7b_o100_2);
        INFO("Test 07_capref: bindCapture<int, IntOffset, int>: END");
    }

    void test08_lambda() {
        {
            volatile int i = 100;

            auto fa0_stub = ( [&](int a) -> int {
                return i + a;
            } );
            typedef decltype(fa0_stub) fa0_type;
            jau::type_cue<fa0_type>::print("lambda.2.fa0_type", TypeTraitGroup::ALL);

            // function<int(int)> fa0 = jau::bind_lambda<int, fa0_type, int>( fa0_stub );
            function<int(int)> fa0 = fa0_stub;

            fprintf(stderr, "fa0.2: %s\n", fa0.toString().c_str());
            REQUIRE( jau::func::target_type::lambda == fa0.type() );

            test_function0_result_type("lambda.2", true, 1, 101, fa0, fa0);
        }
        {
            volatile int i = 100;

            auto fa0_stub = ( [i](int a) -> int {
                return i + a;
            } );
            typedef decltype(fa0_stub) fa0_type;
            jau::type_cue<fa0_type>::print("lambda.3.fa0_type", TypeTraitGroup::ALL);

            function<int(int)> fa0( fa0_stub );

            fprintf(stderr, "fa0.3: %s\n", fa0.toString().c_str());
            REQUIRE( jau::func::target_type::lambda == fa0.type() );

            test_function0_result_type("lambda.3", true, 1, 101, fa0, fa0);
        }
        {
            volatile int i = 100;

            function<int(int)> fa0 = [i](int a) -> int {
                return i + a;
            };

            fprintf(stderr, "fa0.4: %s\n", fa0.toString().c_str());
            REQUIRE( jau::func::target_type::lambda == fa0.type() );

            test_function0_result_type("lambda.4", true, 1, 101, fa0, fa0);
        }
        {
            volatile int i = 100;

            function<int(int)> fa0 = [&](int a) -> int {
                return i + a;
            };

            fprintf(stderr, "fa0.4: %s\n", fa0.toString().c_str());
            REQUIRE( jau::func::target_type::lambda == fa0.type() );

            test_function0_result_type("lambda.4", true, 1, 101, fa0, fa0);
        }
        {
#if 0
            function<void(int)> f0 = jau::bind_lambda( [&i](int a) -> void {
                int r = i + a;
                (void)r;
            } );
            (void)f0;

            function<int(int)> f = jau::bind_lambda( [&i](int a) -> int {
                return i + a;
            } );
            test_function0_result_type("FuncPtr1a_free_10", true, 1, 101, f, f);
#endif
        }
    }

    void test09_lambda_ctti() {
        volatile int i = 100;
        // volatile int j = 100;

        MyCFunc0 f_0 = (MyCFunc0) ( [](int a) -> int {
            return 100 + a;
        } );
        const char* f0_name = jau::ctti_name<decltype(f_0)>();
        REQUIRE( jau::type_info::is_valid( f0_name ) );
        jau::type_info f_0_type(f0_name);
        std::string f0_str(f0_name);
        fprintf(stderr, "f_0: %s\n", f0_name);

        auto f_a = [&](int a) -> int {
            return i + a;
        };
        const char* fa_name = jau::ctti_name<decltype(f_a)>();
        REQUIRE( jau::type_info::is_valid( fa_name ) );
        std::string fa_str(fa_name);
        fprintf(stderr, "f_a: %s\n", fa_name);

        {
            // Limitation: Non unique function pointer type names with same prototype
            jau::type_info f_b_type;
            fprintf(stderr, "empty type: %s\n", f_b_type.name());

            MyCFunc0 f_b = cfunction_00(f_b_type);
            // We must instantiate the ctti_name from its source location,
            // otherwise it is missing for RTTI and CTTI - rendering it the same!
            //
            // const char* fb_name = jau::ctti_name<decltype(f_b)>();
            // REQUIRE( jau::type_info::is_valid( fb_name ) );
            const char* fb_name = f_b_type.name();
            std::string fb_str(fb_name);
            fprintf(stderr, "f_b: %s\n", fb_name);

#if defined(__cxx_rtti_available__)
            std::type_index f_0_t(typeid(f_0));
            fprintf(stderr, "f_0_t: %s\n", f_0_t.name());
            std::type_index f_b_t(typeid(f_b));
            fprintf(stderr, "f_b_t: %s\n", f_b_t.name());

            if( f_0_t == f_b_t ) {
                fprintf(stderr, "INFO: RTTI limitation on functions exists: f_b_t: %s\n", f_b_t.name());
            } else {
                fprintf(stderr, "INFO: RTTI limitation on functions FIXED: f_b_t: %s\n", f_b_t.name());
            }
#else
            (void)f_b;
#endif
            if( f0_str == fb_str ) {
                fprintf(stderr, "INFO: CTTI limitation on functions exists: f_b: %s\n", fb_str.c_str());
            } else {
                fprintf(stderr, "INFO: CTTI limitation on functions FIXED: f_b: %s\n", fb_str.c_str());
            }
            if( f_0_type == f_b_type ) {
                fprintf(stderr, "INFO: CTTI limitation on functions exists: f_b_type: %s\n", f_b_type.name());
            } else {
                fprintf(stderr, "INFO: CTTI limitation on functions FIXED: f_b_type: %s\n", f_b_type.name());
            }
        }

        {
            jau::function<int(int)> f_c = lambda_01();
            const char* fc_name = jau::ctti_name<decltype(f_c)>();
            REQUIRE( jau::type_info::is_valid( fc_name ) );
            std::string fc_str(fc_name);
            fprintf(stderr, "fc_name: %s\n", fc_name);
            fprintf(stderr, "fc:      %s\n", f_c.toString().c_str());
        }
        {
            // NOTE-E: f_e != f_a: Different function prototype (hit), equivalent but different code and same capture than fa2_1!
            auto f_e = [&](int a, bool dummy) -> int {
                (void)dummy;
                return i + a;
            };
            const char* fe_name = jau::ctti_name<decltype(f_e)>();
            REQUIRE( jau::type_info::is_valid( fe_name ) );
            std::string fe_str(fe_name);
            fprintf(stderr, "fe_name: %s\n", fe_name);

            REQUIRE(fa_str != fe_str );
        }
    }

    void test10_lambda_id() {
        {
            volatile int i = 100;
            volatile int j = 100;

            auto fa0_stub = ( [&](int a) -> int {
                return i + a;
            } );

            function<int(int)> fa0_a( fa0_stub );
            fprintf(stderr, "fa0_a: %s\n", fa0_a.toString().c_str());
            REQUIRE( jau::func::target_type::lambda == fa0_a.type() );
            {
                auto fa0c_stub = ( [&](int a) -> int {
                    return i + a;
                } );
                function<int(int)> fa0_c( fa0c_stub );
                fprintf(stderr, "fa0_c: %s\n", fa0_c.toString().c_str());
                fprintf(stderr, "fa0_stub is_same fa0c_stub: %d\n",
                        std::is_same_v<decltype(fa0_stub), decltype(fa0c_stub)> );
                fprintf(stderr, "fa0_a == fa0_c: %d\n",
                        fa0_a == fa0_c );
            }

            // Note-0: Based on same fa0_stub, hence same code and capture!
            function<int(int)> fa0_b( fa0_stub );
            fprintf(stderr, "fa1: %s\n", fa0_b.toString().c_str());
            REQUIRE( jau::func::target_type::lambda == fa0_a.type() );

            function<int(int)> fa2_1 = [&](int a) -> int {
                return i + a;
            };
            fprintf(stderr, "fa2_1: %s\n", fa2_1.toString().c_str());
            REQUIRE( jau::func::target_type::lambda == fa2_1.type() );

            // NOTE-1: fa2_2 != fa2_1: Different code location from function lambda_01(), equivalent code but not same, same capture!
            function<int(int)> fa2_2 = lambda_01();
            fprintf(stderr, "fa2_2: %s\n", fa2_2.toString().c_str());
            REQUIRE( jau::func::target_type::lambda == fa2_2.type() );

            // NOTE-2: fa2_3 != fa2_1: Equivalent code but not same, same capture!
            // FIXME: No RTTI on GCC produces same __PRETTY_FUNCTION__ based id (just parent function + generic lambda),
            //        where clang uses filename + line, which works.
            function<int(int)> fa2_3 = [&](int a) -> int {
                return i + a;
            };
            fprintf(stderr, "fa2_3: %s\n", fa2_3.toString().c_str());
            REQUIRE( jau::func::target_type::lambda == fa2_3.type() );

            // NOTE-3: fa2_4 != fa2_1: Different capture type than fa2_1 (but equivalent code)
            function<int(int)> fa2_4 = [i](int a) -> int {
                return i + a;
            };
            fprintf(stderr, "fa2_4: %s\n", fa2_4.toString().c_str());
            REQUIRE( jau::func::target_type::lambda == fa2_4.type() );

            // NOTE-B: f_b != fa2_1: Equivalent but different code and different capture than fa2_1!
            // !RTTI GCC: OK (different capture)
            function<int(int)> f_b = [&](int a) -> int {
                return j + a;
            };
            fprintf(stderr, "f_b:   %s\n", f_b.toString().c_str());
            REQUIRE( jau::func::target_type::lambda == f_b.type() );

            // NOTE-C: f_c != fa2_1: Different code type and different capture than fa2_1!
            // !RTTI GCC: OK (different capture)
            function<int(int)> f_c = [&](int a) -> int {
                return 2 * ( j + a );
            };
            fprintf(stderr, "f_c:   %s\n", f_c.toString().c_str());
            REQUIRE( jau::func::target_type::lambda == f_c.type() );

            // NOTE-D: f_d != fa2_1: Different code type than fa2_1, but same capture!
            // FIXME: See Note-2 !!!
            function<int(int)> f_d = [&](int a) -> int {
                return 2 * ( i + a );
            };
            fprintf(stderr, "f_d:   %s\n", f_d.toString().c_str());
            REQUIRE( jau::func::target_type::lambda == f_d.type() );

            // NOTE-E: f_e != fa2_1: Different function prototype (hit), equivalent but different code and same capture than fa2_1!
            function<int(int, bool)> f_e = [&](int a, bool dummy) -> int {
                (void)dummy;
                return i + a;
            };
            fprintf(stderr, "f_e:   %s\n", f_e.toString().c_str());
            REQUIRE( jau::func::target_type::lambda == f_d.type() );

            test_function0_result_type("lambda.5b", true,  1, 101, fa2_1, fa2_1); // Same function instance
            test_function0_result_type("lambda.5a", true,  1, 101, fa0_a, fa0_b); // Note-0: Same code and capture

            test_function0_result_____("lambda.5c",        1, 101, fa2_1, fa2_2); // NOTE-1: Equal result
            test_function0________type("lambda.5c", false,         fa2_1, fa2_2); // NOTE-1: Diff code
            test_function0_result_____("lambda.5e",        1, 101, fa2_1, fa2_4); // NOTE-3: Equal result
            test_function0________type("lambda.5e", false,         fa2_1, fa2_4); // NOTE-3: Diff capture / code

            test_function0________type("lambda.5B", false,         fa2_1, f_b);   // NOTE-B
            test_function0________type("lambda.5C", false,         fa2_1, f_c);   // NOTE-C

            test_function0_result_____("lambda.5d",        1, 101, fa2_1, fa2_3); // NOTE-2: Equal result
            if constexpr ( jau::type_info::limited_lambda_id ) {
                if( fa2_1 == fa2_3 ) {
                    fprintf(stderr, "INFO: limited_lambda_id: %s:%d\n", __FILE__, __LINE__);
                } else {
                    fprintf(stderr, "INFO: limited_lambda_id FIXED: %s:%d\n", __FILE__, __LINE__);
                    test_function0________type("lambda.5d", false,         fa2_1, fa2_3); // NOTE-2: Diff code
                }
                if( fa2_1 == f_d ) {
                    fprintf(stderr, "INFO: limited_lambda_id: %s:%d\n", __FILE__, __LINE__);
                } else {
                    fprintf(stderr, "INFO: limited_lambda_id FIXED: %s:%d\n", __FILE__, __LINE__);
                    test_function0________type("lambda.5D", false,         fa2_1, f_d);   // NOTE-D
                }
            } else {
                fprintf(stderr, "INFO: !limited_lambda_id: %s:%d\n", __FILE__, __LINE__);
                test_function0________type("lambda.5d", false,         fa2_1, fa2_3); // NOTE-2: Diff code
                test_function0________type("lambda.5D", false,         fa2_1, f_d);   // NOTE-D
            }
            CHECK(fa2_1 != f_e);                                                      // NOTE-D: Diff function prototype
        }
        {
            // lambda capture by reference-1, plain
            int i = 100;
            int j = 100;
            function<int(int)> f_1 = [&i](int a) -> int {
                return i + a;
            };
            fprintf(stderr, "l6 f_1 ref: %s\n", f_1.toString().c_str());
            REQUIRE( jau::func::target_type::lambda == f_1.type() );

            // NOTE-C: f_1 != f_1: Different code type and different capture than f_1!
            // !RTTI GCC: OK (different capture)
            function<int(int)> f_2 = [&j](int a) -> int {
                return j + a;
            };
            fprintf(stderr, "l6 f_2 ref:   %s\n", f_2.toString().c_str());
            REQUIRE( jau::func::target_type::lambda == f_2.type() );

            test_function0_result_____("lambda.6",        1, 101, f_1, f_2);
            test_function0________type("lambda.6", false,         f_1, f_2);
            test_function0________type("lambda.6", true,          f_1, f_1);
        }
        {
            // lambda capture by reference-2, state-test: mutate used captured reference field
            int i = 100;
            int j = 100;
            function<int(int)> f_1 = [&i](int a) -> int {
                int res = i + a;
                i+=1;
                return res;
            };
            fprintf(stderr, "l7 f_1 ref: %s\n", f_1.toString().c_str());
            REQUIRE( jau::func::target_type::lambda == f_1.type() );

            // NOTE-C: f_1 != f_1: Different code type and different capture than f_1!
            // !RTTI GCC: OK (different capture)
            function<int(int)> f_2 = [&j](int a) -> int {
                int res = j + a;
                j+=1;
                return res;
            };
            fprintf(stderr, "l7 f_2 ref:   %s\n", f_2.toString().c_str());
            REQUIRE( jau::func::target_type::lambda == f_2.type() );

            test_function0_result_copy("lambda.7.1a",        1, 101, f_1, f_2); // increment of referenced i,j, f_x passed by copy!
            test_function0_result_copy("lambda.7.1b",        1, 102, f_1, f_2); // increment of referenced i,j, f_x passed by copy!
            test_function0_result_copy("lambda.7.1c",        1, 103, f_1, f_2); // increment of referenced i,j, f_x passed by copy!

            test_function0_result_____("lambda.7.2a",        1, 104, f_1, f_2); // increment of referenced i,j, f_x passed by ref
            test_function0_result_____("lambda.7.2b",        1, 105, f_1, f_2); // increment of referenced i,j, f_x passed by ref
            test_function0_result_____("lambda.7.2c",        1, 106, f_1, f_2); // increment of referenced i,j, f_x passed by ref

            test_function0________type("lambda.7.5", false,         f_1, f_2);
            test_function0________type("lambda.7.5", true,          f_1, f_1);
        }
        {
            // lambda capture by copy, plain
            int i = 100;
            int j = 100;
            function<int(int)> f_1 = [i](int a) -> int {
                return i + a;
            };
            fprintf(stderr, "l8 f_1 cpy: %s\n", f_1.toString().c_str());
            REQUIRE( jau::func::target_type::lambda == f_1.type() );

            // NOTE-C: f_1 != f_1: Different code type and different capture than f_1!
            // !RTTI GCC: OK (different capture)
            function<int(int)> f_2 = [j](int a) -> int {
                return j + a;
            };
            fprintf(stderr, "l8 f_2 cpy: %s\n", f_2.toString().c_str());
            REQUIRE( jau::func::target_type::lambda == f_2.type() );

            test_function0_result_____("lambda.8.1",        1, 101, f_1, f_2);
            if constexpr ( !jau::type_info::limited_lambda_id ) {
                test_function0________type("lambda.8.2", false,         f_1, f_2);
            } else {
                if( f_1 == f_2 ) {
                    fprintf(stderr, "INFO: limited_lambda_id: %s:%d\n", __FILE__, __LINE__);
                } else {
                    fprintf(stderr, "INFO: limited_lambda_id FIXED: %s:%d\n", __FILE__, __LINE__);
                    test_function0________type("lambda.8.2", false,         f_1, f_2); // NOTE-2: Diff code
                }
            }
            test_function0________type("lambda.8.3", true,          f_1, f_1);
        }
        {
            // lambda capture by copy-2, state-test: mutate a static variable
            int i = 100;
            int j = 100;
            function<int(int)> f_1 = [i](int a) -> int {
                static int store = i;
                int res = store + a;
                store+=1;
                return res;
            };
            fprintf(stderr, "l9 f_1 cpy: %s\n", f_1.toString().c_str());
            REQUIRE( jau::func::target_type::lambda == f_1.type() );

            // NOTE-C: f_1 != f_1: Different code type and different capture than f_1!
            // !RTTI GCC: OK (different capture)
            function<int(int)> f_2 = [j](int a) -> int {
                static int store = j;
                int res = store + a;
                store+=1;
                return res;
            };
            fprintf(stderr, "l9 f_2 cpy: %s\n", f_2.toString().c_str());
            REQUIRE( jau::func::target_type::lambda == f_2.type() );

            test_function0_result_copy("lambda.9.1a",        1, 101, f_1, f_2); // increment of static, f_x passed by copy!
            test_function0_result_copy("lambda.9.1b",        1, 102, f_1, f_2); // increment of static, f_x passed by copy!
            test_function0_result_copy("lambda.9.1c",        1, 103, f_1, f_2); // increment of static, f_x passed by copy!

            test_function0_result_____("lambda.9.2a",        1, 104, f_1, f_2); // increment of static, f_x passed by ref
            test_function0_result_____("lambda.9.2b",        1, 105, f_1, f_2); // increment of static, f_x passed by ref
            test_function0_result_____("lambda.9.2c",        1, 106, f_1, f_2); // increment of static, f_x passed by ref

            if constexpr ( !jau::type_info::limited_lambda_id ) {
                test_function0________type("lambda.9.5", false,         f_1, f_2);
            } else {
                if( f_1 == f_2 ) {
                    fprintf(stderr, "INFO: limited_lambda_id: %s:%d\n", __FILE__, __LINE__);
                } else {
                    fprintf(stderr, "INFO: limited_lambda_id FIXED: %s:%d\n", __FILE__, __LINE__);
                    test_function0________type("lambda.9.5", false,         f_1, f_2); // NOTE-2: Diff code
                }
            }
            test_function0________type("lambda.9.5", true,          f_1, f_1);
        }
        {
            // lambda capture by copy-3, state-test: mutate used captured copied field, lambda marked as mutable!
            //
            // Note: This fails w/ old implementation functional2.hpp, i.e. FUNCTIONAL_BROKEN_COPY_WITH_MUTATING_CAPTURE
            //
            int i = 100;
            int j = 100;
            function<int(int)> f_1 = [i](int a) mutable -> int {
                int res = i + a;
                i+=1;
                return res;
            };
            fprintf(stderr, "l10 f_1 cpy: %s\n", f_1.toString().c_str());
            REQUIRE( jau::func::target_type::lambda == f_1.type() );

            // NOTE-C: f_1 != f_1: Different code type and different capture than f_1!
            // !RTTI GCC: OK (different capture)
            function<int(int)> f_2 = [j](int a) mutable -> int {
                int res = j + a;
                j+=1;
                return res;
            };
            fprintf(stderr, "l10 f_2 cpy: %s\n", f_2.toString().c_str());
            REQUIRE( jau::func::target_type::lambda == f_2.type() );

#if FUNCTIONAL_IMPL == 1
            test_function0_result_copy("lambda.10.1a",        1, 101, f_1, f_2); // increment of copied i,j, f_x passed by copy!
            test_function0_result_copy("lambda.10.1b",        1, 101, f_1, f_2); // increment of copied i,j, f_x passed by copy!
            test_function0_result_copy("lambda.10.1c",        1, 101, f_1, f_2); // increment of copied i,j, f_x passed by copy!
#else
            fprintf(stderr, "l10 f_2 cpy: FUNCTIONAL_BROKEN_COPY_WITH_MUTABLE_LAMBDA\n");
#endif

            test_function0_result_____("lambda.10.2a",        1, 101, f_1, f_2); // increment of copied i,j, f_x passed by ref
            test_function0_result_____("lambda.10.2b",        1, 102, f_1, f_2); // increment of copied i,j, f_x passed by ref
            test_function0_result_____("lambda.10.2c",        1, 103, f_1, f_2); // increment of copied i,j, f_x passed by ref

            if constexpr ( !jau::type_info::limited_lambda_id ) {
                test_function0________type("lambda.10.5", false,         f_1, f_2);
            } else {
                if( f_1 == f_2 ) {
                    fprintf(stderr, "INFO: limited_lambda_id: %s:%d\n", __FILE__, __LINE__);
                } else {
                    fprintf(stderr, "INFO: limited_lambda_id FIXED: %s:%d\n", __FILE__, __LINE__);
                    test_function0________type("lambda.10.5", false,         f_1, f_2); // NOTE-2: Diff code
                }
            }
            test_function0________type("lambda.10.5", true,          f_1, f_1);
        }
#if ( FUNCTIONAL_IMPL == 1 )
        {
            function<int(int)> f_1 = function<int(int)>::bind_ylambda( [](auto& self, int x) -> int {
                if( 0 == x ) {
                    return 1;
                } else {
                    return x * self(x-1);
                }
            } );
            fprintf(stderr, "ylambda 1 f_1: %s\n", f_1.toString().c_str());
            REQUIRE( jau::func::target_type::ylambda == f_1.type() );
            REQUIRE( 24 == f_1(4) ); // `self` is bound to delegate<R(A...)> `f_1.target`, `x` is 4

            // f_1 != f_2 since both reference a different `self`
            function<int(int)> f_2 = function<int(int)>::bind_ylambda( [](auto& self, int x) -> int {
                if( 0 == x ) {
                    return 1;
                } else {
                    return x * self(x-1);
                }
            } );
            test_function0________type("ylambda.1.1", true,          f_1, f_1);
            test_function0________type("ylambda.1.2", false,         f_1, f_2);
        }
#endif
    }

    template<typename R, typename L, typename... A>
    class y_combinator_lambda {
        private:
            L f;
        public:
            // template<typename L>
            y_combinator_lambda(L func) noexcept
            : f( func )
            { }

            static constexpr y_combinator_lambda make(L func) {
                return y_combinator_lambda<R, L, A...>(func);
            }

            constexpr R operator()(A... args) const {
                return f(*this, args...);
            }
            constexpr R operator()(A... args) {
                return f(*this, args...);
            }
    };

    void test15_ylambda() {
        {
            // Using the manual template type y_combinator_lambda, 1st-try
            auto stub = [](auto& self, int x) -> int {
                if( 0 == x ) {
                    return 1;
                } else {
                    return x * self(x-1);
                }
            };
            jau::type_cue<decltype(stub)>::print("y_combinator.0.stub", TypeTraitGroup::ALL);
            y_combinator_lambda<int, decltype(stub), int> f_1 = stub;
            REQUIRE( 24 == f_1(4) );
        }
#if ( FUNCTIONAL_IMPL == 1 )
        {
            // Using an auto stub taking the lambda first, then assign to explicit template typed function<R(A...)>
            // Notable: While the `auto stub` is TriviallyCopyable, the delegated jau::func::ylambda_target_t::data_type is not.
            //          However, direct assignment in the next example is all TriviallyCopyable and hence efficient.
            auto stub = [](auto& self, int x) -> int {
                if( 0 == x ) {
                    return 1;
                } else {
                    return x * self(x-1);
                }
            };
            typedef decltype(stub) stub_type;
            jau::type_cue<stub_type>::print("ylambda 1.stub", TypeTraitGroup::ALL);

            function<int(int)> f_1( jau::func::ylambda_target_t<int, stub_type, int>::delegate(stub), 0 );

            fprintf(stderr, "ylambda 1 f_1: %s\n", f_1.toString().c_str());
            REQUIRE( jau::func::target_type::ylambda == f_1.type() );
            REQUIRE( 24 == f_1(4) );
        }
        {
            function<int(int)> f_1 = function<int(int)>::bind_ylambda( [](auto& self, int x) -> int {
                if( 0 == x ) {
                    return 1;
                } else {
                    return x * self(x-1);
                }
            } );

            fprintf(stderr, "ylambda 2 f_1: %s\n", f_1.toString().c_str());
            REQUIRE( jau::func::target_type::ylambda == f_1.type() );
            REQUIRE( 24 == f_1(4) ); // `self` is bound to delegate<R(A...)> `f_1.target`, `x` is 4
        }
#endif
    }

  private:

    // template<typename R, typename... A>
    typedef int(*MyCFunc0)(int);
    typedef function<int(int)> MyClassFunction0;

    int func02a_member(int i) {
        int res = i+100;
        return res;;
    }
    int func02b_member(int i) noexcept {
        int res = i+1000;
        return res;
    }
    static int Func03a_static(int i) {
        int res = i+100;
        return res;
    }
    static int Func03b_static(int i) noexcept {
        int res = i+1000;
        return res;
    }

    typedef function<void(int&, int)> MyClassFunction1;

    void func12a_member(int& r, const int i) {
        r = i+100;
    }
    void func12b_member(int& r, const int i) noexcept {
        r = i+1000;
    }
    static void Func13a_static(int& r, const int i) {
        r = i+100;
    }
    static void Func13b_static(int& r, const int i) noexcept {
        r = i+1000;
    }

    typedef function<void()> MyClassFunction2;

    void func20a_member() {
        // nop
    }
    static void Func20a_static() {
        // nop
    }

    void test_function0_result_type(std::string msg, bool expEqual, const int value, int expRes, MyClassFunction0& f1, MyClassFunction0& f2) {
        // test std::function identity
        INFO(msg+": Func0.rt Func f1p == f2p : " + std::to_string( f1 == f2 ) + ", f1p: " + f1.toString() + ", f2 "+f2.toString() );
        int f1r = f1(value);
        int f2r = f2(value);
        INFO(msg+": Func0.rt Res_ f1r == f2r : " + std::to_string( f1r == f2r ) + ", f1r: " + std::to_string( f1r ) + ", f2r "+std::to_string( f2r ) );
        if( expEqual ) {
            REQUIRE(f1r == expRes);
            REQUIRE(f2r == expRes);
            REQUIRE(f1 == f2);
        } else {
            REQUIRE(f1 != f2);
        }
    }
    void test_function0________type(std::string msg, bool expEqual, MyClassFunction0& f1, MyClassFunction0& f2) {
        // test std::function identity
        INFO(msg+": Func0._t Func f1p == f2p : " + std::to_string( f1 == f2 ) + ", f1p: " + f1.toString() + ", f2 "+f2.toString() );
        {
            int f1r = f1(0);
            int f2r = f2(0);
            (void)f1r;
            (void)f2r;
        }
        if( expEqual ) {
            CHECK(f1 == f2);
        } else {
            CHECK(f1 != f2);
        }
    }
    void test_function0_result_____(std::string msg, const int value, int expRes, MyClassFunction0& f1, MyClassFunction0& f2) {
        // test std::function identity
        INFO(msg+": Func0.ref.r_ Func f1p == f2p : " + std::to_string( f1 == f2 ) + ", f1p: " + f1.toString() + ", f2 "+f2.toString() );
        int f1r = f1(value);
        int f2r = f2(value);
        INFO(msg+": Func0.ref.r_ Res_ f1r == f2r : " + std::to_string( f1r == f2r ) + ", f1r: " + std::to_string( f1r ) + ", f2r "+std::to_string( f2r ) );
        REQUIRE(f1r == expRes);
        REQUIRE(f2r == expRes);
    }
    void test_function0_result_copy(std::string msg, const int value, int expRes, MyClassFunction0 f1, MyClassFunction0 f2) {
        // test std::function identity
        INFO(msg+": Func0.cpy.r_ Func f1p == f2p : " + std::to_string( f1 == f2 ) + ", f1p: " + f1.toString() + ", f2 "+f2.toString() );
        int f1r = f1(value);
        int f2r = f2(value);
        INFO(msg+": Func0.cpy.r_ Res_ f1r == f2r : " + std::to_string( f1r == f2r ) + ", f1r: " + std::to_string( f1r ) + ", f2r "+std::to_string( f2r ) );
        REQUIRE(f1r == expRes);
        REQUIRE(f2r == expRes);
    }

    void test_function1_result_type(std::string msg, bool expEqual, const int value, int expRes, MyClassFunction1& f1, MyClassFunction1& f2) noexcept {
        // test std::function identity
        INFO(msg+": Func1.ref.rt Func f1p == f2p : " + std::to_string( f1 == f2 ) + ", f1p: " + f1.toString() + ", f2 "+f2.toString() );
        int f1r, f2r;
        f1(f1r, value);
        f2(f2r, value);
        INFO(msg+": Func1.ref.rt Res_ f1r == f2r : " + std::to_string( f1r == f2r ) + ", f1r: " + std::to_string( f1r ) + ", f2r "+std::to_string( f2r ) );
        if( expEqual ) {
            REQUIRE(f1r == expRes);
            REQUIRE(f2r == expRes);
            REQUIRE(f1 == f2);
        } else {
            REQUIRE(f1 != f2);
        }
    }
    void test_function1________type(std::string msg, bool expEqual, MyClassFunction1& f1, MyClassFunction1& f2) noexcept {
        // test std::function identity
        INFO(msg+": Func1.ref._t Func f1p == f2p : " + std::to_string( f1 == f2 ) + ", f1p: " + f1.toString() + ", f2 "+f2.toString() );
        {
            int f1r, f2r;
            f1(f1r, 0);
            f2(f2r, 0);
            (void)f1r;
            (void)f2r;
        }
        if( expEqual ) {
            CHECK(f1 == f2);
        } else {
            CHECK(f1 != f2);
        }
    }

    void test_function2________type(std::string msg, bool expEqual, MyClassFunction2& f1, MyClassFunction2& f2) noexcept {
        // test std::function identity
        INFO(msg+": Func2.ref._t Func f1p == f2p : " + std::to_string( f1 == f2 ) + ", f1p: " + f1.toString() + ", f2 "+f2.toString() );
        {
            f1();
            f2();
        }
        if( expEqual ) {
            CHECK(f1 == f2);
        } else {
            CHECK(f1 != f2);
        }
    }

    static MyCFunc0 cfunction_00(jau::type_info& type) {
        MyCFunc0 f = (MyCFunc0) ( [](int a) -> int {
            return 100 + a;
        } );
        type = jau::type_info( jau::ctti_name<decltype(f)>() );
        return f;
    }
    static function<int(int)> lambda_01() {
        static int i = 100;
        function<int(int)> f = [&](int a) -> int {
            return i + a;
        };
        return f;
    }
    static function<int(int)> lambda_02() {
        int i = 100;
        function<int(int)> f = [i](int a) -> int {
            return i + a;
        };
        return f;
    }

    struct IntOffset {
        int value;
        IntOffset(int v) : value(v) {}

        bool operator==(const IntOffset& rhs) const {
            if( &rhs == this ) {
                return true;
            }
            return value == rhs.value;
        }

        bool operator!=(const IntOffset& rhs) const
        { return !( *this == rhs ); }

    };

    struct IntOffset2 {
        int value;
        IntOffset2(int v) : value(v) {}

        IntOffset2(const IntOffset2 &o)
        : value(o.value)
        {
            INFO("IntOffset2::copy_ctor");
        }
        IntOffset2(IntOffset2 &&o)
        : value(std::move(o.value))
        {
            INFO("IntOffset2::move_ctor");
        }
        IntOffset2& operator=(const IntOffset2 &o) {
            INFO("IntOffset2::copy_assign");
            if( &o == this ) {
                return *this;
            }
            value = o.value;
            return *this;
        }
        IntOffset2& operator=(IntOffset2 &&o) {
            INFO("IntOffset2::move_assign");
            value = std::move(o.value);
            (void)value;
            return *this;
        }

        bool operator==(const IntOffset2& rhs) const {
            if( &rhs == this ) {
                return true;
            }
            return value == rhs.value;
        }

        bool operator!=(const IntOffset2& rhs) const
        { return !( *this == rhs ); }

    };
};


METHOD_AS_TEST_CASE( TestFunction01::test00_usage,               "00_usage");

METHOD_AS_TEST_CASE( TestFunction01::test01_memberfunc_this,     "01_memberfunc");
METHOD_AS_TEST_CASE( TestFunction01::test02_freefunc_static,     "02_freefunc");
METHOD_AS_TEST_CASE( TestFunction01::test03_stdfunc_lambda,      "03_stdfunc");
METHOD_AS_TEST_CASE( TestFunction01::test04_capval_lambda,       "04_capval");
METHOD_AS_TEST_CASE( TestFunction01::test05_capval_lambda,       "05_capval");
METHOD_AS_TEST_CASE( TestFunction01::test06_capval_lambda,       "06_capval");
METHOD_AS_TEST_CASE( TestFunction01::test07_capref_lambda,       "07_capref");
METHOD_AS_TEST_CASE( TestFunction01::test08_lambda,              "08_lambda");
METHOD_AS_TEST_CASE( TestFunction01::test09_lambda_ctti,         "09_lambda_ctti");
METHOD_AS_TEST_CASE( TestFunction01::test10_lambda_id,           "10_lambda_id");

METHOD_AS_TEST_CASE( TestFunction01::test11_memberfunc_this,     "11_memberfunc");
METHOD_AS_TEST_CASE( TestFunction01::test12_freefunc_static,     "12_freefunc");
METHOD_AS_TEST_CASE( TestFunction01::test13_stdfunc_lambda,      "13_stdfunc");
METHOD_AS_TEST_CASE( TestFunction01::test14_capval_lambda,       "14_capval");

METHOD_AS_TEST_CASE( TestFunction01::test15_ylambda,             "15_ylambda");
