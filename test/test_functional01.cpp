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

#include <jau/functional.hpp>

using namespace jau;

// Test examples.

static int Func0a_free(int i) noexcept {
    int res = i+100;
    return res;
}

static void Func1a_free(int&r, int i) noexcept {
    r = i+100;
}

class TestFunction01 {
  private:

    // template<typename R, typename... A>
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

    void test_FunctionPointer00(std::string msg, bool expEqual, const int value, int expRes, MyClassFunction0 & f1, MyClassFunction0 &f2) {
        // test std::function identity
        INFO(msg+": FunctionPointer00 Fun f1p == f2p : " + std::to_string( f1 == f2 ) + ", f1p: " + f1.toString() + ", f2 "+f2.toString() );
        int f1r = f1(value);
        int f2r = f2(value);
        INFO(msg+": FunctionPointer00 Res f1r == f2r : " + std::to_string( f1r == f2r ) + ", f1r: " + std::to_string( f1r ) + ", f2r "+std::to_string( f2r ) );
        if( expEqual ) {
            REQUIRE(f1r == expRes);
            REQUIRE(f2r == expRes);
            REQUIRE(f1 == f2);
        } else {
            REQUIRE(f1 != f2);
        }
    }
    void test_FunctionPointer01(std::string msg, bool expEqual, MyClassFunction0 & f1, MyClassFunction0 &f2) {
        // test std::function identity
        INFO(msg+": FunctionPointer01 Fun f1p == f2p : " + std::to_string( f1 == f2 ) + ", f1p: " + f1.toString() + ", f2 "+f2.toString() );
        if( expEqual ) {
            REQUIRE(f1 == f2);
        } else {
            REQUIRE(f1 != f2);
        }
    }

    void test_FunctionPointer10(std::string msg, bool expEqual, const int value, int expRes, MyClassFunction1 & f1, MyClassFunction1 &f2) noexcept {
        // test std::function identity
        INFO(msg+": FunctionPointer10 Fun f1p == f2p : " + std::to_string( f1 == f2 ) + ", f1p: " + f1.toString() + ", f2 "+f2.toString() );
        int f1r, f2r;
        f1(f1r, value);
        f2(f2r, value);
        INFO(msg+": FunctionPointer10 Res f1r == f2r : " + std::to_string( f1r == f2r ) + ", f1r: " + std::to_string( f1r ) + ", f2r "+std::to_string( f2r ) );
        if( expEqual ) {
            REQUIRE(f1r == expRes);
            REQUIRE(f2r == expRes);
            REQUIRE(f1 == f2);
        } else {
            REQUIRE(f1 != f2);
        }
    }
    void test_FunctionPointer11(std::string msg, bool expEqual, MyClassFunction1 & f1, MyClassFunction1 &f2) noexcept {
        // test std::function identity
        INFO(msg+": FunctionPointer11 Fun f1p == f2p : " + std::to_string( f1 == f2 ) + ", f1p: " + f1.toString() + ", f2 "+f2.toString() );
        if( expEqual ) {
            REQUIRE(f1 == f2);
        } else {
            REQUIRE(f1 != f2);
        }
    }

  public:
    void test00_usage() {
        INFO("Test 00_usage: START");
#if 0
        {
            typedef int(*cfunc)(int);

            function<int(int)> f1a_1 = (cfunc) ( [](int i)->int {
                int res = i+10000;
                return res;
            } );
        }
        {
            typedef int(*cfunc)(int);
            function<int(int)> f1a_1 ( (cfunc) TestFunction01::Func03a_static );
            // function<int(int)> f1a_1 = (cfunc) TestFunction01::Func03a_static;
        }
#endif
        {
            // free, result non-void
            function<int(int)> f1a_1 = bind_free(Func0a_free);
            function<int(int)> f3a_1 = bind_free(&TestFunction01::Func03a_static);
            function<int(int)> f3a_2 = bind_free(&TestFunction01::Func03a_static);
            test_FunctionPointer00("FuncPtr1a_free_10", true,   1, 101, f1a_1, f1a_1);
            test_FunctionPointer00("FuncPtr3a_free_11", true,   1, 101, f3a_1, f3a_1);
            test_FunctionPointer00("FuncPtr3a_free_12", true,   1, 101, f3a_1, f3a_2);
            test_FunctionPointer00("FuncPtr1a_free_10", false,  1, 101, f1a_1, f3a_1);
        }
        {
            // free, result void
            function<void(int&, int)> f1a_1 = bind_free(Func1a_free);
            function<void(int&, int)> f3a_1 = bind_free(&TestFunction01::Func13a_static);
            function<void(int&, int)> f3a_2 = bind_free(&TestFunction01::Func13a_static);
            test_FunctionPointer10("FuncPtr1a_free_10", true,   1, 101, f1a_1, f1a_1);
            test_FunctionPointer10("FuncPtr3a_free_11", true,   1, 101, f3a_1, f3a_1);
            test_FunctionPointer10("FuncPtr3a_free_12", true,   1, 101, f3a_1, f3a_2);
            test_FunctionPointer10("FuncPtr1a_free_10", false,  1, 101, f1a_1, f3a_1);
        }
        {
            // member, result non-void
            function<int(int)> f2a_1 = bind_member(this, &TestFunction01::func02a_member);
            function<int(int)> f2a_2 = bind_member(this, &TestFunction01::func02a_member);
            function<int(int)> f2b_1 = bind_member(this, &TestFunction01::func02b_member);
            test_FunctionPointer00("FuncPtr2a_member_12", true,  1, 101, f2a_1, f2a_2);
            test_FunctionPointer00("FuncPtr2a_member_12", false, 1, 101, f2a_1, f2b_1);
        }
        {
            // member, result void
            function<void(int&, int)> f2a_1 = bind_member(this, &TestFunction01::func12a_member);
            function<void(int&, int)> f2a_2 = bind_member(this, &TestFunction01::func12a_member);
            function<void(int&, int)> f2b_1 = bind_member(this, &TestFunction01::func12b_member);
            test_FunctionPointer10("FuncPtr2a_member_12", true,  1, 101, f2a_1, f2a_2);
            test_FunctionPointer10("FuncPtr2a_member_12", false, 1, 101, f2a_1, f2b_1);
        }
        {
            // lambda w/ explicit capture by value, result non-void
            int offset100 = 100;

            typedef int(*cfunc)(int&, int);

            int(*func5a_capture)(int&, int) = [](int& capture, int i)->int {
                int res = i+10000+capture;
                return res;
            };
            int(*func5b_capture)(int&, int) = [](int& capture, int i)->int {
                int res = i+100000+capture;
                return res;
            };

            function<int(int)> f5_o100_1 = bind_capval(offset100,
                    (cfunc) ( [](int& capture, int i)->int {
                        int res = i+10000+capture;
                        return res;;
                    } ) );
            function<int(int)> f5_o100_2 = bind_capval(offset100,
                    (cfunc) ( [](int& capture, int i)->int {
                        int res = i+10000+capture;
                        return res;;
                    } ) );
            test_FunctionPointer01("FuncPtr5a_o100_capture_00", true,  f5_o100_1, f5_o100_1);
            test_FunctionPointer01("FuncPtr5a_o100_capture_00", false, f5_o100_1, f5_o100_2);

            function<int(int)> f5a_o100_1 = bind_capval(offset100, func5a_capture);
            function<int(int)> f5a_o100_2 = bind_capval(offset100, func5a_capture);
            function<int(int)> f5b_o100_1 = bind_capval(offset100, func5b_capture);
            test_FunctionPointer01("FuncPtr5a_o100_capture_12", true,  f5a_o100_1, f5a_o100_2);
            test_FunctionPointer01("FuncPtr5a_o100_capture_12", false, f5a_o100_1, f5b_o100_1);
            test_FunctionPointer00("FuncPtr5a_o100_capture_11", true,  1, 10101, f5a_o100_1, f5a_o100_1);
            test_FunctionPointer00("FuncPtr5a_o100_capture_12", true,  1, 10101, f5a_o100_1, f5a_o100_2);
            test_FunctionPointer00("FuncPtr5a_o100_capture_12", false, 1, 10101, f5a_o100_1, f5b_o100_1);
        }
        {
            // lambda w/ explicit capture by reference, result non-void
            IntOffset offset100(100);

            typedef int(*cfunc)(IntOffset*, int);

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
            function<int(int)> f7_o100_2 = bind_capref<int, IntOffset, int>(&offset100,
                    (cfunc) ( [](IntOffset* capture, int i)->int {
                        int res = i+10000+capture->value;
                        return res;;
                    } ) );
            test_FunctionPointer01("FuncPtr7a_o100_capture_00", true,  f7_o100_1, f7_o100_1);
            test_FunctionPointer01("FuncPtr7a_o100_capture_00", false, f7_o100_1, f7_o100_2);

            function<int(int)> f7a_o100_1 = bind_capref(&offset100, func7a_capture);
            function<int(int)> f7a_o100_2 = bind_capref(&offset100, func7a_capture);
            function<int(int)> f7b_o100_1 = bind_capref(&offset100, func7b_capture);
            test_FunctionPointer01("FuncPtr7a_o100_capture_12", true,  f7a_o100_1, f7a_o100_2);
            test_FunctionPointer01("FuncPtr7a_o100_capture_12", false, f7a_o100_1, f7b_o100_1);
            test_FunctionPointer00("FuncPtr7a_o100_capture_11", true,  1, 10101, f7a_o100_1, f7a_o100_1);
            test_FunctionPointer00("FuncPtr7a_o100_capture_12", true,  1, 10101, f7a_o100_1, f7a_o100_2);
            test_FunctionPointer00("FuncPtr7a_o100_capture_12", false, 1, 10101, f7a_o100_1, f7b_o100_1);
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
            function<int(int)> f4a_2 = bind_std(100, func4a_stdlambda);
            test_FunctionPointer00("FuncPtr4a_stdlambda_11", true, 1, 101, f4a_1, f4a_1);
            test_FunctionPointer00("FuncPtr4a_stdlambda_12", true, 1, 101, f4a_1, f4a_2);
        }

        INFO("Test 00_usage: END");
    }

    void test01_memberfunc_this() {
        INFO("Test 01_member: bind_member<int, TestFunction01, int>: START");
        // function(TestFunction01 &base, Func1Type func)
        MyClassFunction0 f2a_1 = bind_member<int, TestFunction01, int>(this, &TestFunction01::func02a_member);
        MyClassFunction0 f2a_2 = bind_member(this, &TestFunction01::func02a_member);
        test_FunctionPointer00("FuncPtr2a_member_11", true, 1, 101, f2a_1, f2a_1);
        test_FunctionPointer00("FuncPtr2a_member_12", true, 1, 101, f2a_1, f2a_2);

        MyClassFunction0 f2b_1 = bind_member(this, &TestFunction01::func02b_member);
        MyClassFunction0 f2b_2 = bind_member(this, &TestFunction01::func02b_member);
        test_FunctionPointer00("FuncPtr2b_member_11", true, 1, 1001, f2b_1, f2b_1);
        test_FunctionPointer00("FuncPtr2b_member_12", true, 1, 1001, f2b_1, f2b_2);

        test_FunctionPointer00("FuncPtr2ab_member_11", false, 1, 0, f2a_1, f2b_1);
        test_FunctionPointer00("FuncPtr2ab_member_22", false, 1, 0, f2a_2, f2b_2);
        INFO("Test 01_member: bind_member<int, TestFunction01, int>: END");
    }

    void test11_memberfunc_this() {
        INFO("Test 11_member: bind_member<int, TestFunction01, int>: START");
        // function(TestFunction01 &base, Func1Type func)
        MyClassFunction1 f2a_1 = bind_member<TestFunction01, int&, int>(this, &TestFunction01::func12a_member);
        MyClassFunction1 f2a_2 = bind_member(this, &TestFunction01::func12a_member);
        test_FunctionPointer10("FuncPtr2a_member_11", true, 1, 101, f2a_1, f2a_1);
        test_FunctionPointer10("FuncPtr2a_member_12", true, 1, 101, f2a_1, f2a_2);

        MyClassFunction1 f2b_1 = bind_member(this, &TestFunction01::func12b_member);
        MyClassFunction1 f2b_2 = bind_member(this, &TestFunction01::func12b_member);
        test_FunctionPointer10("FuncPtr2b_member_11", true, 1, 1001, f2b_1, f2b_1);
        test_FunctionPointer10("FuncPtr2b_member_12", true, 1, 1001, f2b_1, f2b_2);

        test_FunctionPointer10("FuncPtr2ab_member_11", false, 1, 0, f2a_1, f2b_1);
        test_FunctionPointer10("FuncPtr2ab_member_22", false, 1, 0, f2a_2, f2b_2);
        INFO("Test 11_member: bind_member<int, TestFunction01, int>: END");
    }

    void test02_freefunc_static() {
        INFO("Test 02_free: bind_free<int, int>: START");
        // function(Func1Type func)
        MyClassFunction0 f1a_1 = bind_free<int, int>(Func0a_free);
        MyClassFunction0 f3a_1 = bind_free<int, int>(&TestFunction01::Func03a_static);
        MyClassFunction0 f3a_2 = bind_free(&TestFunction01::Func03a_static);
        test_FunctionPointer00("FuncPtr1a_free_10", true,  1, 101, f1a_1, f1a_1);
        test_FunctionPointer00("FuncPtr3a_free_11", true,  1, 101, f3a_1, f3a_1);
        test_FunctionPointer00("FuncPtr3a_free_12", true,  1, 101, f3a_1, f3a_2);

        MyClassFunction0 f3b_1 = bind_free(&TestFunction01::Func03b_static);
        MyClassFunction0 f3b_2 = bind_free(&Func03b_static);
        test_FunctionPointer00("FuncPtr3b_free_11", true, 1, 1001, f3b_1, f3b_1);
        test_FunctionPointer00("FuncPtr3b_free_12", true, 1, 1001, f3b_1, f3b_2);

        test_FunctionPointer00("FuncPtr1a3a_free_10", false, 1, 0, f1a_1, f3a_1);
        test_FunctionPointer00("FuncPtr1a3b_free_10", false, 1, 0, f1a_1, f3b_1);
        test_FunctionPointer00("FuncPtr3a3b_free_11", false, 1, 0, f3a_1, f3b_1);
        test_FunctionPointer00("FuncPtr3a3b_free_22", false, 1, 0, f3a_2, f3b_2);
        INFO("Test 02_free: bind_free<int, int>: END");
    }

    void test12_freefunc_static() {
        INFO("Test 12_free: bind_free<int, int>: START");
        // function(Func1Type func)
        MyClassFunction1 f1a_1 = bind_free<int&, int>(Func1a_free);
        MyClassFunction1 f3a_1 = bind_free<int&, int>(&TestFunction01::Func13a_static);
        MyClassFunction1 f3a_2 = bind_free(&TestFunction01::Func13a_static);
        test_FunctionPointer10("FuncPtr1a_free_10", true,  1, 101, f1a_1, f1a_1);
        test_FunctionPointer10("FuncPtr3a_free_11", true,  1, 101, f3a_1, f3a_1);
        test_FunctionPointer10("FuncPtr3a_free_12", true,  1, 101, f3a_1, f3a_2);

        MyClassFunction1 f3b_1 = bind_free(&TestFunction01::Func13b_static);
        MyClassFunction1 f3b_2 = bind_free(&Func13b_static);
        test_FunctionPointer10("FuncPtr3b_free_11", true, 1, 1001, f3b_1, f3b_1);
        test_FunctionPointer10("FuncPtr3b_free_12", true, 1, 1001, f3b_1, f3b_2);

        test_FunctionPointer10("FuncPtr1a3a_free_10", false, 1, 0, f1a_1, f3a_1);
        test_FunctionPointer10("FuncPtr1a3b_free_10", false, 1, 0, f1a_1, f3b_1);
        test_FunctionPointer10("FuncPtr3a3b_free_11", false, 1, 0, f3a_1, f3b_1);
        test_FunctionPointer10("FuncPtr3a3b_free_22", false, 1, 0, f3a_2, f3b_2);
        INFO("Test 12_free: bind_free<int, int>: END");
    }

    void test03_stdfunc_lambda() {
        INFO("Test 03_stdlambda: bind_std<int, int>: START");
        // function(Func1Type func) <int, int>
        std::function<int(int i)> func4a_stdlambda = [](int i)->int {
            int res = i+100;
            return res;;
        };
        std::function<int(int i)> func4b_stdlambda = [](int i)->int {
            int res = i+1000;
            return res;;
        };
        MyClassFunction0 f4a_1 = bind_std<int, int>(100, func4a_stdlambda);
        MyClassFunction0 f4a_2 = bind_std(100, func4a_stdlambda);
        test_FunctionPointer00("FuncPtr4a_stdlambda_11", true, 1, 101, f4a_1, f4a_1);
        test_FunctionPointer00("FuncPtr4a_stdlambda_12", true, 1, 101, f4a_1, f4a_2);

        MyClassFunction0 f4b_1 = bind_std(200, func4b_stdlambda);
        MyClassFunction0 f4b_2 = bind_std(200, func4b_stdlambda);
        test_FunctionPointer00("FuncPtr4b_stdlambda_11", true, 1, 1001, f4b_1, f4b_1);
        test_FunctionPointer00("FuncPtr4b_stdlambda_12", true, 1, 1001, f4b_1, f4b_2);

        test_FunctionPointer00("FuncPtr4ab_stdlambda_11", false, 1, 0, f4a_1, f4b_1);
        test_FunctionPointer00("FuncPtr4ab_stdlambda_22", false, 1, 0, f4a_2, f4b_2);

        INFO("Test 03_stdlambda: bind_std<int, int>: END");
    }

    void test13_stdfunc_lambda() {
        INFO("Test 13_stdlambda: bind_std<int, int>: START");
        // function(Func1Type func) <int, int>
        std::function<void(int& r, int i)> func4a_stdlambda = [](int& r, int i)->void {
            r = i+100;
        };
        std::function<void(int& r, int i)> func4b_stdlambda = [](int& r, int i)->void {
            r = i+1000;
        };
        MyClassFunction1 f4a_1 = bind_std<int&, int>(100, func4a_stdlambda);
        MyClassFunction1 f4a_2 = bind_std(100, func4a_stdlambda);
        test_FunctionPointer10("FuncPtr4a_stdlambda_11", true, 1, 101, f4a_1, f4a_1);
        test_FunctionPointer10("FuncPtr4a_stdlambda_12", true, 1, 101, f4a_1, f4a_2);

        MyClassFunction1 f4b_1 = bind_std(200, func4b_stdlambda);
        MyClassFunction1 f4b_2 = bind_std(200, func4b_stdlambda);
        test_FunctionPointer10("FuncPtr4b_stdlambda_11", true, 1, 1001, f4b_1, f4b_1);
        test_FunctionPointer10("FuncPtr4b_stdlambda_12", true, 1, 1001, f4b_1, f4b_2);

        test_FunctionPointer10("FuncPtr4ab_stdlambda_11", false, 1, 0, f4a_1, f4b_1);
        test_FunctionPointer10("FuncPtr4ab_stdlambda_22", false, 1, 0, f4a_2, f4b_2);

        INFO("Test 13_stdlambda: bind_std<int, int>: END");
    }

    void test04_capval_lambda() {
        INFO("Test 04_capval: bindCapture<int, int, int>: START");
        // bindCapture(I& data, R(*func)(I&, A...))
        // function(Func1Type func) <int, int>
        int offset100 = 100;
        int offset1000 = 1000;

        typedef int(*cfunc)(int&, int);

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
        test_FunctionPointer01("FuncPtr5a_o100_capture_00", true, f5a_o100_0, f5a_o100_0);

        MyClassFunction0 f5a_o100_1 = bind_capval<int, int, int>(offset100, func5a_capture);
        MyClassFunction0 f5a_o100_2 = bind_capval(offset100, func5a_capture);
        test_FunctionPointer01("FuncPtr5a_o100_capture_12", true, f5a_o100_1, f5a_o100_2);
        test_FunctionPointer00("FuncPtr5a_o100_capture_11", true, 1, 10101, f5a_o100_1, f5a_o100_1);
        test_FunctionPointer00("FuncPtr5a_o100_capture_12", true, 1, 10101, f5a_o100_1, f5a_o100_2);
        // test_FunctionPointer01("FuncPtr5a_o100_capture_01", false, f5a_o100_0, f5a_o100_1);
        MyClassFunction0 f5a_o1000_1 = bind_capval(offset1000, func5a_capture);
        MyClassFunction0 f5a_o1000_2 = bind_capval(offset1000, func5a_capture);
        test_FunctionPointer01("FuncPtr5a_o1000_capture_12", true, f5a_o1000_1, f5a_o1000_2);
        test_FunctionPointer01("FuncPtr5a_o100_o1000_capture_11", false, f5a_o100_1, f5a_o1000_1);

        MyClassFunction0 f5b_o100_1 = bind_capval(offset100, func5b_capture);
        MyClassFunction0 f5b_o100_2 = bind_capval(offset100, func5b_capture);
        test_FunctionPointer00("FuncPtr5b_o100_capture_11", true, 1, 100101, f5b_o100_1, f5b_o100_1);
        test_FunctionPointer00("FuncPtr5b_o100_capture_12", true, 1, 100101, f5b_o100_1, f5b_o100_2);

        test_FunctionPointer00("FuncPtr5ab_o100_capture_11", false, 1, 0, f5a_o100_1, f5b_o100_1);
        test_FunctionPointer00("FuncPtr5ab_o100_capture_22", false, 1, 0, f5a_o100_2, f5b_o100_2);
        INFO("Test 04_capval: bindCapture<int, int, int>: END");
    }

    void test14_capval_lambda() {
        INFO("Test 14_capval: bindCapture<int, int, int>: START");
        // bindCapture(I& data, R(*func)(I&, A...))
        // function(Func1Type func) <int, int>
        int offset100 = 100;
        int offset1000 = 1000;

        typedef void(*cfunc)(int&, int&, int);

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
        test_FunctionPointer11("FuncPtr5a_o100_capture_00", true, f5a_o100_0, f5a_o100_0);

        MyClassFunction1 f5a_o100_1 = bind_capval<int, int&, int>(offset100, func5a_capture);
        MyClassFunction1 f5a_o100_2 = bind_capval(offset100, func5a_capture);
        test_FunctionPointer11("FuncPtr5a_o100_capture_12", true, f5a_o100_1, f5a_o100_2);
        test_FunctionPointer10("FuncPtr5a_o100_capture_11", true, 1, 10101, f5a_o100_1, f5a_o100_1);
        test_FunctionPointer10("FuncPtr5a_o100_capture_12", true, 1, 10101, f5a_o100_1, f5a_o100_2);
        // test_FunctionPointer01("FuncPtr5a_o100_capture_01", false, f5a_o100_0, f5a_o100_1);
        MyClassFunction1 f5a_o1000_1 = bind_capval(offset1000, func5a_capture);
        MyClassFunction1 f5a_o1000_2 = bind_capval(offset1000, func5a_capture);
        test_FunctionPointer11("FuncPtr5a_o1000_capture_12", true, f5a_o1000_1, f5a_o1000_2);
        test_FunctionPointer11("FuncPtr5a_o100_o1000_capture_11", false, f5a_o100_1, f5a_o1000_1);

        MyClassFunction1 f5b_o100_1 = bind_capval(offset100, func5b_capture);
        MyClassFunction1 f5b_o100_2 = bind_capval(offset100, func5b_capture);
        test_FunctionPointer10("FuncPtr5b_o100_capture_11", true, 1, 100101, f5b_o100_1, f5b_o100_1);
        test_FunctionPointer10("FuncPtr5b_o100_capture_12", true, 1, 100101, f5b_o100_1, f5b_o100_2);

        test_FunctionPointer10("FuncPtr5ab_o100_capture_11", false, 1, 0, f5a_o100_1, f5b_o100_1);
        test_FunctionPointer10("FuncPtr5ab_o100_capture_22", false, 1, 0, f5a_o100_2, f5b_o100_2);
        INFO("Test 14_capval: bindCapture<int, int, int>: END");
    }

    void test05_capval_lambda() {
        INFO("Test 05_capval: bindCapture<int, std::shared_ptr<IntOffset>, int>: START");
        // bindCapture(I& data, R(*func)(I&, A...))
        // function(Func1Type func) <int, int>
        std::shared_ptr<IntOffset> offset100(new IntOffset(100));
        std::shared_ptr<IntOffset> offset1000(new IntOffset(1000));

        typedef int(*cfunc)(std::shared_ptr<IntOffset>&, int);

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
        test_FunctionPointer01("FuncPtr6a_o100_capture_00", true, f6a_o100_0, f6a_o100_0);

        MyClassFunction0 f6a_o100_1 = bind_capval<int, std::shared_ptr<IntOffset>, int>(offset100, func6a_capture);
        MyClassFunction0 f6a_o100_2 = bind_capval(offset100, func6a_capture);
        test_FunctionPointer01("FuncPtr6a_o100_capture_12", true, f6a_o100_1, f6a_o100_2);
        test_FunctionPointer00("FuncPtr6a_o100_capture_11", true, 1, 10101, f6a_o100_1, f6a_o100_1);
        test_FunctionPointer00("FuncPtr6a_o100_capture_12", true, 1, 10101, f6a_o100_1, f6a_o100_2);
        // test_FunctionPointer01("FuncPtr6a_o100_capture_01", false, f6a_o100_0, f6a_o100_1);
        MyClassFunction0 f6a_o1000_1 = bind_capval(offset1000, func6a_capture);
        MyClassFunction0 f6a_o1000_2 = bind_capval(offset1000, func6a_capture);
        test_FunctionPointer01("FuncPtr6a_o1000_capture_12", true, f6a_o1000_1, f6a_o1000_2);
        test_FunctionPointer01("FuncPtr6a_o100_o1000_capture_11", false, f6a_o100_1, f6a_o1000_1);

        MyClassFunction0 f6b_o100_1 = bind_capval(offset100, func6b_capture);
        MyClassFunction0 f6b_o100_2 = bind_capval(offset100, func6b_capture);
        test_FunctionPointer00("FuncPtr6b_o100_capture_11", true, 1, 100101, f6b_o100_1, f6b_o100_1);
        test_FunctionPointer00("FuncPtr6b_o100_capture_12", true, 1, 100101, f6b_o100_1, f6b_o100_2);

        test_FunctionPointer00("FuncPtr6ab_o100_capture_11", false, 1, 0, f6a_o100_1, f6b_o100_1);
        test_FunctionPointer00("FuncPtr6ab_o100_capture_22", false, 1, 0, f6a_o100_2, f6b_o100_2);
        INFO("Test 05_capval: bindCapture<int, std::shared_ptr<IntOffset>, int>: END");
    }

    void test06_capval_lambda() {
        INFO("Test 06_capval: bindCapture<int, IntOffset, int>: START");
        // bindCapture(I& data, R(*func)(I&, A...))
        // function(Func1Type func) <int, int>
        IntOffset offset100(100);
        IntOffset offset1000(1000);

        typedef int(*cfunc)(IntOffset&, int);

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
        test_FunctionPointer01("FuncPtr7a_o100_capture_00", true, f7a_o100_0, f7a_o100_0);

        INFO("f7a_o100_1 copy_ctor");
        MyClassFunction0 f7a_o100_1 = bind_capval<int, IntOffset, int>(offset100, func7a_capture);
        INFO("f7a_o100_1 copy_ctor done");
        INFO("f7a_o100_2 move_ctor");
        MyClassFunction0 f7a_o100_2 = bind_capval(IntOffset(100), func7a_capture);
        INFO("f7a_o100_2 move_ctor done");
        test_FunctionPointer01("FuncPtr7a_o100_capture_12", true, f7a_o100_1, f7a_o100_2);
        test_FunctionPointer00("FuncPtr7a_o100_capture_11", true, 1, 10101, f7a_o100_1, f7a_o100_1);
        test_FunctionPointer00("FuncPtr7a_o100_capture_12", true, 1, 10101, f7a_o100_1, f7a_o100_2);
        // test_FunctionPointer01("FuncPtr7a_o100_capture_01", false, f7a_o100_0, f7a_o100_1);
        MyClassFunction0 f7a_o1000_1 = bind_capval(offset1000, func7a_capture);
        MyClassFunction0 f7a_o1000_2 = bind_capval(offset1000, func7a_capture);
        test_FunctionPointer01("FuncPtr7a_o1000_capture_12", true, f7a_o1000_1, f7a_o1000_2);
        test_FunctionPointer01("FuncPtr7a_o100_o1000_capture_11", false, f7a_o100_1, f7a_o1000_1);

        MyClassFunction0 f7b_o100_1 = bind_capval(offset100, func7b_capture);
        MyClassFunction0 f7b_o100_2 = bind_capval(offset100, func7b_capture);
        test_FunctionPointer00("FuncPtr7b_o100_capture_11", true, 1, 100101, f7b_o100_1, f7b_o100_1);
        test_FunctionPointer00("FuncPtr7b_o100_capture_12", true, 1, 100101, f7b_o100_1, f7b_o100_2);

        test_FunctionPointer00("FuncPtr7ab_o100_capture_11", false, 1, 0, f7a_o100_1, f7b_o100_1);
        test_FunctionPointer00("FuncPtr7ab_o100_capture_22", false, 1, 0, f7a_o100_2, f7b_o100_2);
        INFO("Test 06_capval: bindCapture<int, IntOffset, int>: END");
    }

    void test07_capref_lambda() {
        INFO("Test 07_capref: bindCapture<int, IntOffset, int>: START");
        // bindCapture(I& data, R(*func)(I&, A...))
        // function(Func1Type func) <int, int>
        IntOffset offset100(100);
        IntOffset offset1000(1000);

        typedef int(*cfunc)(IntOffset*, int);

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
        test_FunctionPointer01("FuncPtr7a_o100_capture_00", true, f7a_o100_0, f7a_o100_0);

        INFO("f7a_o100_1 copy_ctor");
        MyClassFunction0 f7a_o100_1 = bind_capref<int, IntOffset, int>(&offset100, func7a_capture);
        INFO("f7a_o100_1 copy_ctor done");
        INFO("f7a_o100_2 move_ctor");
        MyClassFunction0 f7a_o100_2 = bind_capref(&offset100, func7a_capture);
        INFO("f7a_o100_2 move_ctor done");
        test_FunctionPointer01("FuncPtr7a_o100_capture_12", true, f7a_o100_1, f7a_o100_2);
        test_FunctionPointer00("FuncPtr7a_o100_capture_11", true, 1, 10101, f7a_o100_1, f7a_o100_1);
        test_FunctionPointer00("FuncPtr7a_o100_capture_12", true, 1, 10101, f7a_o100_1, f7a_o100_2);
        // test_FunctionPointer01("FuncPtr7a_o100_capture_01", false, f7a_o100_0, f7a_o100_1);
        MyClassFunction0 f7a_o1000_1 = bind_capref(&offset1000, func7a_capture);
        MyClassFunction0 f7a_o1000_2 = bind_capref(&offset1000, func7a_capture);
        test_FunctionPointer01("FuncPtr7a_o1000_capture_12", true, f7a_o1000_1, f7a_o1000_2);
        test_FunctionPointer01("FuncPtr7a_o100_o1000_capture_11", false, f7a_o100_1, f7a_o1000_1);

        MyClassFunction0 f7b_o100_1 = bind_capref(&offset100, func7b_capture);
        MyClassFunction0 f7b_o100_2 = bind_capref(&offset100, func7b_capture);
        test_FunctionPointer00("FuncPtr7b_o100_capture_11", true, 1, 100101, f7b_o100_1, f7b_o100_1);
        test_FunctionPointer00("FuncPtr7b_o100_capture_12", true, 1, 100101, f7b_o100_1, f7b_o100_2);

        test_FunctionPointer00("FuncPtr7ab_o100_capture_11", false, 1, 0, f7a_o100_1, f7b_o100_1);
        test_FunctionPointer00("FuncPtr7ab_o100_capture_22", false, 1, 0, f7a_o100_2, f7b_o100_2);
        INFO("Test 07_capref: bindCapture<int, IntOffset, int>: END");
    }

};

METHOD_AS_TEST_CASE( TestFunction01::test00_usage,               "00_usage");

METHOD_AS_TEST_CASE( TestFunction01::test01_memberfunc_this,     "01_memberfunc");
METHOD_AS_TEST_CASE( TestFunction01::test02_freefunc_static,     "02_freefunc");
METHOD_AS_TEST_CASE( TestFunction01::test03_stdfunc_lambda,      "03_stdfunc");
METHOD_AS_TEST_CASE( TestFunction01::test04_capval_lambda,       "04_capval");
METHOD_AS_TEST_CASE( TestFunction01::test05_capval_lambda,       "05_capval");
METHOD_AS_TEST_CASE( TestFunction01::test06_capval_lambda,       "06_capval");
METHOD_AS_TEST_CASE( TestFunction01::test07_capref_lambda,       "06_capref");

METHOD_AS_TEST_CASE( TestFunction01::test11_memberfunc_this,     "11_memberfunc");
METHOD_AS_TEST_CASE( TestFunction01::test12_freefunc_static,     "12_freefunc");
METHOD_AS_TEST_CASE( TestFunction01::test13_stdfunc_lambda,      "13_stdfunc");
METHOD_AS_TEST_CASE( TestFunction01::test14_capval_lambda,       "14_capval");
