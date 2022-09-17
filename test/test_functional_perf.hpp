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
  private:

    typedef int(*native_func_t)(int);
    typedef std::function<int(int)> std_func_t;
    typedef jau::function<int(int)> jau_func_t;

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

    struct IntOffset {
        int value;
        IntOffset(int v) : value(v) {}

        IntOffset(const IntOffset &o)
        : value(o.value)
        {
            INFO("IntOffset::copy_ctor");
        }
        IntOffset(IntOffset &&o)
        : value(std::move(o.value))
        {
            INFO("IntOffset::move_ctor");
        }
        IntOffset& operator=(const IntOffset &o) {
            INFO("IntOffset::copy_assign");
            if( &o == this ) {
                return *this;
            }
            value = o.value;
            return *this;
        }
        IntOffset& operator=(IntOffset &&o) {
            INFO("IntOffset::move_assign");
            value = std::move(o.value);
            (void)value;
            return *this;
        }

        bool operator==(const IntOffset& rhs) const {
            if( &rhs == this ) {
                return true;
            }
            return value == rhs.value;
        }

        bool operator!=(const IntOffset& rhs) const
        { return !( *this == rhs ); }

    };

    static int test_FunctionPointer00(const int value, jau_func_t & f1) noexcept {
        const int f1r = f1(value);
        return f1r;
    }

    static int test_FunctionPointer01(const int value, native_func_t f1) noexcept {
        const int f1r = (*f1)(value);
        return f1r;
    }

    static int test_Func03a_static(const int value) noexcept {
        const int f1r = TestFunction01::Func03a_static(value);
        return f1r;
    }

    static int test_Func02a_member(const int value, TestFunction01& i) noexcept {
        const int f1r = i.func02a_member(value);
        return f1r;
    }

    template<typename F>
    int test_Func02a_stdmember(const int value, F& i) noexcept {
        const int f1r = i(value);
        return f1r;
    }

    static int test_FunctionPointer02(const int value, std_func_t f1) noexcept {
        const int f1r = f1(value);
        return f1r;
    }

  public:
    const int loops = 1000000;

    void test00_perf() {
        INFO("Test 00_usage: START");

        // free raw func
        {
            BENCHMARK("free_rawfunc") {
                volatile int r=0;
                for(int i=0; i<loops; ++i) {
                    r += test_Func03a_static(i);
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
                    r += test_FunctionPointer01(i, f);
                }
                return r;
            };
        }

#if 1
        // free std::function
        {
            std_func_t f = TestFunction01::Func03a_static;

            BENCHMARK("free_stdfunc") {
                volatile int r=0;
                for(int i=0; i<loops; ++i) {
                    r += test_FunctionPointer02(i, f);
                }
                return r;
            };
        }
#endif

        // free, jau::function
        {
            jau::function<int(int)> f = jau::bind_free(&TestFunction01::Func03a_static);

            BENCHMARK("free_jaufunc") {
                volatile int r=0;
                for(int i=0; i<loops; ++i) {
                    r += test_FunctionPointer00(i, f);
                }
                return r;
            };
        }

        // member raw function
        {
            BENCHMARK("member_rawfunc") {
                volatile int r=0;
                for(int i=0; i<loops; ++i) {
                    r += test_Func02a_member(i, *this);
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
                    r += test_Func02a_stdmember(i, f);
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
                    r += test_FunctionPointer00(i, f);
                }
                return r;
            };
        }

        // lambda w/ explicit capture by value, jau::function
        {
            int offset100 = 100;

            int(*func5a_capture)(int&, int) = [](int& capture, int i)->int {
                int res = i+10000+capture;
                return res;
            };
            jau::function<int(int)> f = jau::bind_capval(offset100, func5a_capture);

            BENCHMARK("capval_lambda_jaufunc") {
                volatile int r=0;
                for(int i=0; i<loops; ++i) {
                    r += test_FunctionPointer00(i, f);
                }
                return r;
            };
        }

#if 1
        // lambda w/ explicit capture by reference, jau::function
        {
            IntOffset offset100(100);

            int(*func7a_capture)(IntOffset*, int) = [](IntOffset* capture, int i)->int {
                int res = i+10000+capture->value;
                return res;
            };
            jau::function<int(int)> f = jau::bind_capref(&offset100, func7a_capture);

            BENCHMARK("capref_lambda_jaufunc") {
                volatile int r=0;
                for(int i=0; i<loops; ++i) {
                    r += test_FunctionPointer00(i, f);
                }
                return r;
            };
        }

        // std::function lambda, jau::function
        {
            std::function<int(int i)> l = [](int i)->int {
                int res = i+100;
                return res;;
            };
            jau::function<int(int)> f = jau::bind_std(100, l);

            BENCHMARK("std_function_lambda_jaufunc") {
                volatile int r=0;
                for(int i=0; i<loops; ++i) {
                    r += test_FunctionPointer00(i, f);
                }
                return r;
            };
        }

#endif

        REQUIRE( true == true );

        INFO("Test 00_usage: END");
    }
};

METHOD_AS_TEST_CASE( TestFunction01::test00_perf, "00_perf");
