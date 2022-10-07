/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2022 Gothel Software e.K.
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

#if !FUNCTIONAL_PROVIDED
    #include <jau/functional.hpp>
#endif

#include <jau/test/catch2_ext.hpp>

// Test examples.

class TestFunction01 {
  public:
    const int loops = 1000000;

    /**
     * Unit test covering most variants of jau::function<R(A...)
     */
    void test00_usage() {
        {
            // Test capturing lambdas
            volatile int i = 100;

            {
                jau::function<int(int)> fa0 = [&](int a) -> int {
                    return i + a;
                };
                fprintf(stderr, "lambda.ref:    %s\n", fa0.toString().c_str());
                REQUIRE( jau::func::target_type::lambda == fa0.type() );
            }

            {
                jau::function<int(int)> fa0 = [i](int a) -> int {
                    return i + a;
                };
                fprintf(stderr, "lambda.copy:   %s\n", fa0.toString().c_str());
                REQUIRE( jau::func::target_type::lambda == fa0.type() );
            }

            {
                // and this non-capturing lambda is also detected as lambda
                jau::function<int(int)> fl3_1 = [](int a) -> int {
                    return a + 100;
                } ;
                fprintf(stderr, "lambda.plain   %s\n", fl3_1.toString().c_str());
                REQUIRE( jau::func::target_type::lambda == fl3_1.type() );
            }
        }
        {
            // free, result void and no params
            typedef void(*cfunc)();
            jau::function<void()> fl_0 = (cfunc) ( []() -> void {
                // nop
            } );
            fprintf(stderr, "freeA.0        %s\n", fl_0.toString().c_str());
            REQUIRE( jau::func::target_type::free == fl_0.type() );
        }
        {
            // member, result non-void
            jau::function<int(int)> f2a_0(this, &TestFunction01::func02a_member);
            fprintf(stderr, "member:        %s\n", f2a_0.toString().c_str());
            REQUIRE( jau::func::target_type::member == f2a_0.type() );
        }
        {
            // member, result void
            jau::function<void(int&, int)> f2a_0(this, &TestFunction01::func12a_member);
            fprintf(stderr, "member:        %s\n", f2a_0.toString().c_str());
            REQUIRE( jau::func::target_type::member == f2a_0.type() );
        }
        {
            // Lambda alike w/ explicit capture by value, result non-void
            int offset100 = 100;

            typedef int(*cfunc)(int&, int); // to force non-capturing lambda into a free function template type deduction

            jau::function<int(int)> f5_o100_1 = jau::bind_capval(offset100,
                    (cfunc) ( [](int& capture, int i)->int {
                        int res = i+10000+capture;
                        return res;
                    } ) );
            fprintf(stderr, "capval.small:  %s\n", f5_o100_1.toString().c_str());
        }
        {
            // Lambda alike w/ explicit capture by value, result non-void
            struct blob {
                int offset100 = 100;
                uint64_t lala0 = 0;
                uint64_t lala1 = 1;
                uint64_t lala2 = 2;
                uint64_t lala3 = 3;

                bool operator==(const blob& rhs) const noexcept {
                    return offset100 == rhs.offset100 &&
                           lala0 == rhs.lala0 &&
                           lala1 == rhs.lala1 &&
                           lala2 == rhs.lala2 &&
                           lala3 == rhs.lala3;
                }
                bool operator!=(const blob& rhs) const noexcept
                { return !( *this == rhs ); }

            };
            blob b0;

            typedef int(*cfunc)(blob&, int); // to force non-capturing lambda into a free function template type deduction

            jau::function<int(int)> f5_o100_1 = jau::bind_capval(b0,
                    (cfunc) ( [](blob& capture, int i)->int {
                        int res = i+10000+capture.offset100;
                        return res;
                    } ) );
            fprintf(stderr, "capval.big:    %s\n", f5_o100_1.toString().c_str());
        }
        {
            // Lambda alike w/ explicit capture by reference, result non-void
            int offset100 = 100;

            typedef int(*cfunc)(int*, int); // to force non-capturing lambda into a free function template type deduction

            jau::function<int(int)> f7_o100_1 = jau::bind_capref<int, int, int>(&offset100,
                    (cfunc) ( [](int* capture, int i)->int {
                        int res = i+10000+(*capture);
                        return res;;
                    } ) );
            fprintf(stderr, "capref:        %s\n", f7_o100_1.toString().c_str());
            REQUIRE( jau::func::target_type::capref == f7_o100_1.type() );
        }
        {
            // std::function lambda
            std::function<int(int i)> func4a_stdlambda = [](int i)->int {
                int res = i+100;
                return res;;
            };
            jau::function<int(int)> f = jau::bind_std(100, func4a_stdlambda);
            fprintf(stderr, "std.lambda:    %s\n", f.toString().c_str());
            fprintf(stderr, "  (net std.lambda):    sizeof %zu\n", sizeof(func4a_stdlambda));
            REQUIRE( jau::func::target_type::std == f.type() );
        }
    }

    void test10_perf() {
        INFO("Test 00_usage: START");

        // free raw func
        {
            BENCHMARK("free_rawfunc") {
                volatile int r=0;
                for(int i=0; i<loops; ++i) {
                    r += TestFunction01::Func03a_static(i);
                }
                return r;
            };
        }

        // free native function pointer
        {
            native_func_t f = TestFunction01::Func03a_static;

            BENCHMARK("free_cfuncptr") {
                volatile int r=0;
                for(int i=0; i<loops; ++i) {
                    r += f(i);
                }
                return r;
            };
        }

        // free std::function
        {
            std::function<int(int)> f = TestFunction01::Func03a_static;

            BENCHMARK("free_stdfunc") {
                volatile int r=0;
                for(int i=0; i<loops; ++i) {
                    r += f(i);
                }
                return r;
            };
        }

        // free, jau::function
        {
            jau::function<int(int)> f = jau::bind_free(&TestFunction01::Func03a_static);

            BENCHMARK("free_jaufunc") {
                volatile int r=0;
                for(int i=0; i<loops; ++i) {
                    r += f(i);
                }
                return r;
            };
        }

        // member raw function
        {
            BENCHMARK("member_rawfunc") {
                volatile int r=0;
                for(int i=0; i<loops; ++i) {
                    r += func02a_member(i);
                }
                return r;
            };
        }

        // member std::bind unspecific
        {
            using namespace std::placeholders;  // for _1, _2, _3...
            auto f = std::bind(&TestFunction01::func02a_member, this, _1);

            BENCHMARK("member_stdbind_unspec") {
                volatile int r=0;
                for(int i=0; i<loops; ++i) {
                    r += f(i);
                }
                return r;
            };
        }

        // member jau::function
        {
            jau::function<int(int)> f = jau::bind_member(this, &TestFunction01::func02a_member);

            BENCHMARK("member_jaufunc") {
                volatile int r=0;
                for(int i=0; i<loops; ++i) {
                    r += f(i);
                }
                return r;
            };
        }

        // lambda w/ explicit capture by value, jau::function
        {
            int offset100 = 100;

            int(*func5a_capture)(int&, int) = [](int& capture, int i)->int {
                int res = i+capture;
                return res;
            };
            jau::function<int(int)> f = jau::bind_capval(offset100, func5a_capture);

            BENCHMARK("capval_small_jaufunc") {
                volatile int r=0;
                for(int i=0; i<loops; ++i) {
                    r += f(i);
                }
                return r;
            };
        }

        {
            // Lambda alike w/ explicit capture by value, result non-void
            struct blob {
                int offset100 = 100;
                uint64_t lala0 = 0;
                uint64_t lala1 = 1;
                uint64_t lala2 = 2;
                uint64_t lala3 = 3;

                bool operator==(const blob& rhs) const noexcept {
                    return offset100 == rhs.offset100 &&
                           lala0 == rhs.lala0 &&
                           lala1 == rhs.lala1 &&
                           lala2 == rhs.lala2 &&
                           lala3 == rhs.lala3;
                }
                bool operator!=(const blob& rhs) const noexcept
                { return !( *this == rhs ); }

            };
            blob b0;

            typedef int(*cfunc)(blob&, int); // to force non-capturing lambda into a free function template type deduction

            jau::function<int(int)> f = jau::bind_capval(b0,
                    (cfunc) ( [](blob& capture, int i)->int {
                        int res = i+10000+capture.offset100;
                        return res;
                    } ) );

            BENCHMARK("capval_big_jaufunc") {
                volatile int r=0;
                for(int i=0; i<loops; ++i) {
                    r += f(i);
                }
                return r;
            };
        }

#if 1
        // lambda w/ explicit capture by reference, jau::function
        {
            int offset100 = 100;

            int(*func7a_capture)(int*, int) = [](int* capture, int i)->int {
                int res = i+*capture;
                return res;
            };
            jau::function<int(int)> f = jau::bind_capref(&offset100, func7a_capture);

            BENCHMARK("capref_jaufunc") {
                volatile int r=0;
                for(int i=0; i<loops; ++i) {
                    r += f(i);
                }
                return r;
            };
        }

        // std::function lambda
        {
            std::function<int(int i)> f = [](int i)->int {
                int res = i+100;
                return res;;
            };

            BENCHMARK("lambda_std_function") {
                volatile int r=0;
                for(int i=0; i<loops; ++i) {
                    r += f(i);
                }
                return r;
            };
        }
#endif

#if !SKIP_JAU_LAMBDAS
        {
            volatile int captured = 100;

            jau::function<int(int)> f = [&](int a) -> int {
                return captured + a;
            };

            BENCHMARK("lambda_jaufunc") {
                volatile int r=0;
                for(int i=0; i<loops; ++i) {
                    r += f(i);
                }
                return r;
            };
        }
#endif

        REQUIRE( true == true );

        INFO("Test 00_usage: END");
    }

  private:

    typedef int(*native_func_t)(int);
    typedef std::function<int(int)> std_func_t;
    typedef jau::function<int(int)> jau_func_t;

    // template<typename R, typename... A>
    typedef int(*MyCFunc0)(int);

    typedef jau::function<int(int)> MyClassFunction0;
    typedef jau::function<void(int&, int)> MyClassFunction1;
    typedef jau::function<void()> MyClassFunction2;

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

    void func20a_member() {
        // nop
    }
    static void Func20a_static() {
        // nop
    }

    static jau::function<int(int)> lambda_01() {
        static int i = 100;
        jau::function<int(int)> f = [&](int a) -> int {
            return i + a;
        };
        return f;
    }
    static jau::function<int(int)> lambda_02() {
        int i = 100;
        jau::function<int(int)> f = [i](int a) -> int {
            return i + a;
        };
        return f;
    }

};

METHOD_AS_TEST_CASE( TestFunction01::test00_usage, "00_usage");
METHOD_AS_TEST_CASE( TestFunction01::test10_perf,  "10_perf");
