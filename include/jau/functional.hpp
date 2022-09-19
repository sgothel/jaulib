/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020-2022 Gothel Software e.K.
 * Copyright (c) 2020 ZAFENA AB
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

#ifndef JAU_FUNCTIONAL_HPP_
#define JAU_FUNCTIONAL_HPP_

#include <cstring>
#include <string>
#include <memory>
#include <cstdint>
#include <type_traits>
#include <functional>
#include <typeindex>

#include <jau/basic_types.hpp>
#include <jau/cpp_lang_util.hpp>

namespace jau {

    /** @defgroup FunctionWrap Function Wrapper
     *  Supporting general-purpose polymorphic function wrapper via jau::function<R(A...)>.
     *
     * @anchor function_overview
     * ### Function Overview
     * Similar to std::function, [jau::function<R(A...)>](@ref function_def) stores any callable target function
     * solely described by its return type `R` and arguments types `A...` from any source,
     * e.g. free functions, capturing and non-capturing lambda function, member functions, etc.
     *
     * [jau::function<R(A...)>](@ref function_def) supports equality operations for all func::target_t source types,
     * allowing to manage container of [jau::function](@ref function_def)s.
     *
     * See [limitations](@ref function_limitations) below.
     *
     * If a [jau::function](@ref function_def) contains no target, see jau::function<R(A...)>::is_null(), it is empty.
     * Invoking the target of an empty [jau::function](@ref function_def) is a no-operation and has no side effects.
     *
     * [jau::function](@ref function_def) satisfies the requirements of CopyConstructible and CopyAssignable.
     *
     * Compared to `std::function<R(A...)>`, `jau::function<R(A...)>`
     * - supports equality operations,
     * - supports capturing lambda functions
     *   - See [limitations on their equality operator w/o RTTI on `gcc`](@ref function_limitations).
     * - most operations are `noexcept`, except for the user given function invocation.
     *
     * Implementation utilizes a fast path target function [delegate](@ref delegate_class), see [func::target_t<R(A...)>::delegate_t:invoke()](@ref delegate_invoke).
     *
     * Instances of [jau::function](@ref function_def) can store, copy, and invoke any of its callable targets
     * - free functions
     *   - bind_free()
     *     - includes non-capturing lambda
     *   - constructor [jau::function<R(A...)>::function(R(*func)(A...))](@ref function_ctor_free)
     * - member functions
     *   - bind_member()
     *   - constructor [`function(C *base, R(C::*mfunc)(A...))`](@ref function_ctor_member)
     * - lambda functions
     *   - constructor [`template<typename L> function(L func)`](@ref function_ctor_lambda)
     *   - see [limitations on their equality operator w/o RTTI on `gcc`](@ref function_limitations).
     * - lambda alike functions using a captured data type by reference
     *   - bind_capref()
     *   - constructor [function(I* data_ptr, R(*func)(I*, A...), bool dataIsIdentity=true)](@ref function_ctor_capref)
     * - lambda alike functions using a captured data type by value
     *   - bind_capval()
     *   - constructor copy [function(const I& data, R(*func)(I&, A...), bool dataIsIdentity=true)](@ref function_ctor_capval_copy)
     *   - constructor move [function(I&& data, R(*func)(I&, A...), bool dataIsIdentity=true)](@ref function_ctor_capval_move)
     * - std::function
     *   - bind_std()
     *   - constructor [function(uint64_t id, std::function<R(A...)> func)](@ref function_ctor_std)
     *
     * @anchor function_usage
     * #### Function Usage
     *
     * A detailed API usage is covered within [test_functional01.cpp](test_functional01_8cpp-example.html), see function `test00_usage()`.
     *
     * Let's assume we like to bind to the following
     * function prototype `bool func(int)`, which results to `jau::function<bool(int)>`:
     *
     * - Free functions via [constructor](@ref function_ctor_free) and jau::bind_free()
     *   - Prologue
     *   ```
     *   typedef bool(*cfunc)(int); // to force non-capturing lambda into a free function template type deduction
     *
     *   bool my_func(int v) { return 0 == v; }
     *
     *   struct MyClass {
     *      static bool func(int v) { return 0 == v; }
     *   };
     *   ```
     *
     *   - Constructor [function(R(*func)(A...))](@ref function_ctor_free)
     *   ```
     *   jau::function<bool(int)> func0 = my_func;
     *
     *   jau::function<bool(int)> func1 = &MyClass:func;
     *
     *   jau::function<bool(int)> func2 = (cfunc) ( [](int v) -> bool { // forced free via cast
     *           return 0 == v;
     *       } );
     *
     *   ```
     *
     *   - Bind `jau::function<R(A...)> jau::bind_free(R(*func)(A...))`
     *   ```
     *   jau::function<bool(int)> func0 = jau::bind_free(&MyClass:func);
     *
     *   jau::function<bool(int)> func1 = jau::bind_free(my_func);
     *   ```
     *
     * - Class member functions via [constructor](@ref function_ctor_member) and jau::bind_member()
     *   - Prologue
     *   ```
     *   struct MyClass {
     *      bool m_func(int v) { return 0 == v; }
     *   };
     *   MyClass i1;
     *   ```
     *
     *   - Constructor [function(C *base, R(C::*mfunc)(A...))](@ref function_ctor_member)
     *   ```
     *   jau::function<bool(int)> func(&i1, &MyClass::m_func);
     *   ```
     *
     *   - Bind `jau::function<R(A...)> jau::bind_member(C *base, R(C::*mfunc)(A...))`
     *   ```
     *   jau::function<bool(int)> func = jau::bind_member(&i1, &MyClass::m_func);
     *   ```
     *
     * - Lambda functions via [constructor](@ref function_ctor_lambda)
     *   - Prologue
     *   ```
     *   int sum = 0;
     *   ```
     *
     *   - Constructor [template<typename L> function(L func)](@ref function_ctor_lambda)
     *   ```
     *   jau::function<bool(int)> func0 = [](int v) -> bool {
     *          return 0 == v;
     *      };
     *
     *   jau::function<bool(int)> func1 = [&](int v) -> bool {
     *           sum += v;
     *           return 0 == v;
     *       };
     *
     *   auto func2_stub = [&](int v) -> bool {
     *           sum += v;
     *           return 0 == v;
     *       };
     *
     *   jau::function<bool(int)> func2 = func2_stub;
     *   ```
     *
     * - Lambda alike capture by reference to value via [constructor](@ref function_ctor_capref) and jau::bind_capref()
     *   - Prologue
     *   ```
     *   struct big_data {
     *       int sum;
     *   };
     *   big_data data { 0 };
     *
     *   typedef bool(*cfunc)(big_data*, int); // to force non-capturing lambda into a free function template type deduction
     *   ```
     *
     *   - Constructor [function(I* data_ptr, R(*func)(I*, A...), bool dataIsIdentity=true)](@ref function_ctor_capref)
     *   ```
     *   function<int(int)> func(&data,
     *       (cfunc) ( [](big_data* data, int v) -> bool {
     *                 stats_ptr->sum += v;
     *                 return 0 == v;
     *             } ) );
     *   ```
     *
     *   - Bind `jau::function<R(A...)> jau::bind_capref(I* data_ptr, R(*func)(I*, A...), bool dataIsIdentity)`
     *   ```
     *   jau::function<bool(int)> func = jau::bind_capref(&data,
     *       (cfunc) ( [](big_data* data, int v) -> bool {
     *                 stats_ptr->sum += v;
     *                 return 0 == v;
     *             } ) );
     *   ```
     *
     * - Lambda alike capture by copy of value via constructor and jau::bind_capval()
     *   - Constructor copy [function(const I& data, R(*func)(I&, A...), bool dataIsIdentity=true)](@ref function_ctor_capval_copy)
     *   - Constructor move [function(I&& data, R(*func)(I&, A...), bool dataIsIdentity=true)](@ref function_ctor_capval_move)
     *   - Bind copy `jau::function<R(A...)> jau::bind_capval(const I& data, R(*func)(I&, A...), bool dataIsIdentity)`
     *   - Bind move `jau::function<R(A...)> jau::bind_capval(I&& data, R(*func)(I&, A...), bool dataIsIdentity)` <br />
     *   See example of *Capture by reference to value* above.
     *
     * - std::function function via [constructor](@ref function_ctor_std) and jau::bind_std()
     *   - Prologue
     *   ```
     *   std::function<bool(int)> func_stdlambda = [](int i)->bool {
     *       return 0 == i;
     *   };
     *   ```
     *
     *   - Constructor [function(uint64_t id, std::function<R(A...)> func)](@ref function_ctor_std)
     *   ```
     *   jau::function<bool(int)> func(100, func_stdlambda);
     *   ```
     *
     *   - Bind `function<R(A...)> jau::bind_std(uint64_t id, std::function<R(A...)> func)`
     *   ```
     *   jau::function<bool(int)> func = jau::bind_std(100, func_stdlambda);
     *   ```
     * @anchor function_limitations
     * #### Function Limitations
     *
     * ##### Equality operation on lambda function without RTTI
     * Equality operator on a [jau::function<R(A...)>](@ref function_def) instance delegates
     * to its shared func::target_t. <br />
     * This removes the lambda function's type when invoking the polymorphic operator. <br />
     * Hence `decltype(lambda_instance)` is to no use from within the func::lambda_target_t's equality operator.
     *
     * Implementation relies on the stored type information at func::lambda_target_t construction
     * where the lambda `decltype(T)` is available.
     *
     * Due to the lack of standardized *Compile-Time Type Information (CTTI)*,
     * we rely either on the *Runtime Type Information (RTTI)*, which works well on `clang` and `gcc` at least,
     * or we have to utilize the non-standardized macro extensions
     * - `__PRETTY_FUNCTION__`
     *   - `clang` produces a unique tag using filename and line number, compatible.
     *   - `gcc` produces a non-unique tag using the parent function of the lambda location and its too brief signature, not fully compatible.
     * - `__FUNCSIG__`
     *   - `msvc++` not tested
     * - Any other compiler is not supported yet
     *
     * Due to these restrictions, *not using RTTI on `gcc`* will *erroneously mistake* two different lambda
     * *functions defined within one function* to be the same.
     *
     *
     * #### C++11 Capturing Lambda Restrictions using std::function
     * A capturing lambda in C++11 produces decoration code accessing the captured elements,<br />
     * i.e. an anonymous helper class.<br />
     * Due to this fact, the return type is an undefined lambda specific
     * and hence `std::function` didn't support it when specified, probably.
     *
     * <pre>
        template<typename R, typename C, typename... A>
        inline function<R(A...)>
        bind_member(C *base, R(C::*mfunc)(A...)) {
            return ClassFunction<R, A...>(
                    (void*)base,
                    (void*)(*((void**)&mfunc)),
                    [&](A... args)->R{ (base->*mfunc)(args...); });
                     ^
                     | Capturing lambda function-pointer are undefined!
        }
        </pre>
     *
     * Capturing lambdas are supported by jau::function using func::lambda_target_t
     * via constructor [`template<typename L> function(L func)`](@ref function_ctor_lambda), see above.
     *
     *
     *  @{
     */

    namespace func {

        /** \addtogroup FunctionWrap
         *
         *  @{
         */

        /**
         * func::target_type identifier for specializations of func::target_t
         * used by jau::function<R(A...)>::type().
         *
         * @see @ref function_overview "Function Overview" etc.
         */
        enum class target_type : int {
            /** Denotes a func::null_target_t */
            null = 0,
            /** Denotes a func::member_target_t */
            member = 1,
            /** Denotes a func::free_target_t */
            free = 2,
            /** Denotes a func::lambda_target_t */
            lambda = 3,
            /** Denotes a func::capval_target_t */
            capval = 4,
            /** Denotes a func::capref_target_t */
            capref = 5,
            /** Denotes a func::std_target_t */
            std = 6
        };
        constexpr int number(const target_type rhs) noexcept {
            return static_cast<int>(rhs);
        }

        /**
         * func::target_t pure-virtual interface for [jau::function](@ref function_def).
         *
         * Implementation utilizes a fast path target function [delegate](@ref delegate_class), see [func::target_t<R(A...)>::delegate_t:invoke()](@ref delegate_invoke).
         *
         * @tparam R function return type
         * @tparam A function arguments
         * @see @ref function_overview "Function Overview" etc.
         */
        template<typename R, typename... A>
        class target_t {
            public:
                typedef R(*invocation_t)(void* base, A... args);

                /**
                 * @anchor delegate_class
                 * Delegated target function details from specialization, allowing a fast path target function invocation, see [invoke()](@ref delegate_invoke).
                 */
                class delegate_t final {
                    private:
                        /** Delegated specialization base pointer. */
                        void* base;
                        /** Delegated specialization callback. */
                        invocation_t cb;

                    public:
                        delegate_t(void* base_, invocation_t cb_) noexcept
                        : base(base_), cb(cb_) {}

                        /**
                         * \brief Delegated fast path target function invocation.
                         *
                         * @anchor delegate_invoke Fast path target function invocation using delegated artifacts
                         * from the specialized instance allowing to
                         * - bypass the virtual function table
                         * - hence allowing constexpr inline here and at function<R(A...)> caller
                         *
                         * @param args target function arguments
                         * @return target function result
                         */
                        constexpr R invoke(A... args) const {
                            return cb(base, args...);
                        }
                };

            private:
                /** Delegated details from specialization. */
                delegate_t ddetail;

            protected:
                target_t(delegate_t ddetail_) noexcept
                : ddetail(ddetail_) {}

            public:
                virtual ~target_t() noexcept {}

                target_t(const target_t &o) noexcept = default;
                target_t(target_t &&o) noexcept = default;
                target_t& operator=(const target_t &o) noexcept = default;
                target_t& operator=(target_t &&o) noexcept = default;

                /** Return the func::target_type of this invocation function wrapper */
                virtual target_type type() const noexcept = 0;

                /** Returns if this this invocation function wrapper's is of jau::func::target_type::null  */
                virtual bool is_null() const noexcept = 0;

                virtual target_t<R, A...> * clone() const noexcept = 0;

                /** Return pointer to delegated details from specialization. */
                constexpr delegate_t& delegate() noexcept { return ddetail; }

                virtual bool operator==(const target_t<R, A...>& rhs) const noexcept = 0;

                bool operator!=(const target_t<R, A...>& rhs) const noexcept {
                    return !( *this == rhs );
                }

                virtual std::string toString() const = 0;

                /** Net size of this target_t instance's function details. */
                virtual size_t detail_size() const noexcept = 0;

                /** Total size of this target_t instance. */
                virtual size_t size() const noexcept = 0;
        };

        /**
         * func::null_target_t implementation for no function.
         * identifiable as jau::func::target_type::null via jau::function<R(A...)>::type().
         *
         * This special type is used for an empty [jau::function](@ref function_def) instance w/o holding a function,
         * e.g. when created with the default constructor.
         *
         * @tparam R function return type
         * @tparam A function arguments
         * @see @ref function_overview "Function Overview" etc.
         */
        template<typename R, typename... A>
        class null_target_t final : public target_t<R, A...> {
            private:
                typedef typename target_t<R, A...>::delegate_t delegate_t;

                constexpr static R invoke_impl(void* base, A... args) {
                    (void)base;
                    (void)(... , args);
                    return R();
                }

            public:
                null_target_t() noexcept
                : target_t<R, A...>( delegate_t(static_cast<void*>(this), invoke_impl) )
                { }

                target_type type() const noexcept override { return target_type::null; }

                bool is_null() const noexcept override { return true; }

                target_t<R, A...> * clone() const noexcept override { return new null_target_t(); }

                bool operator==(const target_t<R, A...>& rhs) const noexcept override
                {
                    return type() == rhs.type();
                }

                std::string toString() const override {
                    return "null(sz " + std::to_string(detail_size()) + "/" + std::to_string(size()) + ")";
                }

                size_t detail_size() const noexcept override { return 0; }

                size_t size() const noexcept override { return sizeof(*this); }
        };

        /**
         * func::member_target_t implementation for class member functions,
         * identifiable as func::target_type::member via jau::function<R(A...)>::type().
         *
         * @tparam R function return type
         * @tparam C class type holding the member-function
         * @tparam A function arguments
         * @see @ref function_overview "Function Overview" etc.
         */
        template<typename R, typename C, typename... A>
        class member_target_t final : public target_t<R, A...> {
            private:
                typedef typename target_t<R, A...>::delegate_t delegate_t;

                C* base;
                R(C::*member)(A...);

                constexpr static R invoke_impl(void* vbase, A... args) {
                    member_target_t<R, C, A...>* base = static_cast<member_target_t<R, C, A...>*>(vbase);
                    return ((base->base)->*(base->member))(args...);
                }

            public:
                member_target_t(C *_base, R(C::*_member)(A...)) noexcept
                : target_t<R, A...>( delegate_t(static_cast<void*>(this), invoke_impl) ),
                  base(_base), member(_member) { }

                target_type type() const noexcept override { return target_type::member; }

                bool is_null() const noexcept override { return false; }

                target_t<R, A...> * clone() const noexcept override { return new member_target_t(*this); }

                bool operator==(const target_t<R, A...>& rhs) const noexcept override
                {
                    if( &rhs == this ) {
                        return true;
                    }
                    if( type() != rhs.type() ) {
                        return false;
                    }
                    const member_target_t<R, C, A...> * prhs = static_cast<const member_target_t<R, C, A...>*>(&rhs);
                    return base == prhs->base && member == prhs->member;
                }

                std::string toString() const override {
                    // hack to convert member pointer to void *: '*((void**)&member)'
                    return "member("+to_hexstring((uint64_t)base)+"->"+to_hexstring( *((void**)&member) ) + ", sz "+std::to_string(detail_size())+"/"+std::to_string(size())+")";
                }

                size_t detail_size() const noexcept override { return sizeof(base)+sizeof(member); }

                size_t size() const noexcept override { return sizeof(*this); }
        };

        /**
         * func::free_target_t implementation for free functions,
         * identifiable as func::target_type::free via jau::function<R(A...)>::type().
         *
         * @tparam R function return type
         * @tparam A function arguments
         * @see @ref function_overview "Function Overview" etc.
         */
        template<typename R, typename... A>
        class free_target_t final : public target_t<R, A...> {
            private:
                typedef typename target_t<R, A...>::delegate_t delegate_t;

                R(*function)(A...);

                constexpr static R invoke_impl(void* vbase, A... args) {
                    free_target_t<R, A...>* base = static_cast<free_target_t<R, A...>*>(vbase);
                    return (*base->function)(args...);
                }

            public:
                free_target_t(R(*_function)(A...)) noexcept
                : target_t<R, A...>( delegate_t(static_cast<void*>(this), invoke_impl) ),
                  function(_function) { }

                target_type type() const noexcept override { return target_type::free; }
                bool is_null() const noexcept override { return false; }

                target_t<R, A...> * clone() const noexcept override { return new free_target_t(*this); }

                bool operator==(const target_t<R, A...>& rhs) const noexcept override
                {
                    if( &rhs == this ) {
                        return true;
                    }
                    if( type() != rhs.type() ) {
                        return false;
                    }
                    const free_target_t<R, A...> * prhs = static_cast<const free_target_t<R, A...>*>(&rhs);
                    return function == prhs->function;
                }

                std::string toString() const override {
                    // hack to convert function pointer to void *: '*((void**)&function)'
                    return "free("+to_hexstring( *( (void**) &function ) )  + ", sz "+std::to_string(detail_size())+"/"+std::to_string(size())+")";
                }

                size_t detail_size() const noexcept override { return sizeof(function); }

                size_t size() const noexcept override { return sizeof(*this); }
        };

// Assume runtime lifecycle of typeid(L).name()
#define TYPEID_LIFECYCLE_BIG 1

        /**
         * func::lambda_target_t implementation for lambda closures,
         * identifiable as func::target_type::lambda via jau::function<R(A...)>::type().
         *
         * @tparam R function return type
         * @tparam L typename holding the lambda closure
         * @tparam A function arguments
         * @see @ref function_overview "Function Overview" etc.
         */
        template<typename R, typename L, typename...A>
        class lambda_target_t final : public target_t<R, A...>
        {
            private:
                typedef typename target_t<R, A...>::delegate_t delegate_t;

#if TYPEID_LIFECYCLE_BIG
                // No static template field to avoid error: self-comparison always evaluates to false [-Werror=tautological-compare]
                const char* funcsig;
#else
                std::string funcsig;
#endif
                size_t hash_value;
                L function; // intentionally last due to pot invalid cast

                constexpr static R invoke_impl(void* vbase, A... args) {
                    lambda_target_t<R, L, A...>* base = static_cast<lambda_target_t<R, L, A...>*>(vbase);
                    return (base->function)(args...);
                }

            public:
                typedef L closure_t;

                lambda_target_t(L function_) noexcept
                : target_t<R, A...>( delegate_t(static_cast<void*>(this), invoke_impl) ),
                  function(function_)
                {
#if TYPEID_LIFECYCLE_BIG
    #if defined(__cxx_rtti_available__)
                  funcsig = typeid(L).name();
    #else
                  funcsig = JAU_PRETTY_FUNCTION; // jau::pretty_function<L>();
    #endif
                  hash_value = std::hash<std::string_view>{}(std::string_view(funcsig));
#elif defined(__cxx_rtti_available__)
                  const std::type_index t(typeid(L));
                  funcsig = t.name();
                  hash_value = std::hash<std::string>{}(funcsig);
#else
                  funcsig = jau::pretty_function<L>();
                  hash_value = std::hash<std::string>{}(funcsig);
#endif
                }

                target_type type() const noexcept override { return target_type::lambda; }
                bool is_null() const noexcept override { return false; }

                target_t<R, A...> * clone() const noexcept override { return new lambda_target_t(*this); }

                bool operator==(const target_t<R, A...>& rhs) const noexcept override
                {
                    if( &rhs == this ) {
                        return true;
                    }
                    if( type() != rhs.type() ) {
                        return false;
                    }
                    const lambda_target_t<R, L, A...> * prhs = static_cast<const lambda_target_t<R, L, A...>*>(&rhs);
                    // Note: Potential invalid cast due to different `L` type of rhs renders decltype(prhs->function) useless
                    if( detail_size() != prhs->detail_size() ||  // fast: size first
                        hash_value != prhs->hash_value ||        // fast: hash value of signature
                        funcsig != prhs->funcsig                 // slower: signature itself, potential hash collision
                      )
                    {
                        return false;
                    }
                    // finally compare the anonymous data chunk of the lambda
                    const void *d1 = (void*)&function;
                    const void *d2 = (void*)&prhs->function;
                    return 0 == ::memcmp(d1, d2, sizeof(function));
                }

                std::string toString() const override {
                    return "lambda(sz "+std::to_string(detail_size())+"/"+std::to_string(size())+", sig "+std::string(funcsig)+", hash "+to_hexstring(hash_value)+")";
                }

                size_t detail_size() const noexcept override { return sizeof(function); }

                size_t size() const noexcept override { return sizeof(*this); }
        };

        /**
         * func::capval_target_t implementation for functions using a copy of a captured value,
         * identifiable as func::target_type::capval via jau::function<R(A...)>::type().
         *
         * @tparam R function return type
         * @tparam I typename holding the captured data used by the function
         * @tparam A function arguments
         * @see @ref function_overview "Function Overview" etc.
         */
        template<typename R, typename I, typename... A>
        class capval_target_t final : public target_t<R, A...> {
            private:
                typedef typename target_t<R, A...>::delegate_t delegate_t;

                I data;
                R(*function)(I&, A...);
                bool dataIsIdentity;

                constexpr static R invoke_impl(void* vbase, A... args) {
                    capval_target_t<R, I, A...>* base = static_cast<capval_target_t<R, I, A...>*>(vbase);
                    return (*base->function)(base->data, args...);
                }

            public:
                /** Utilizes copy-ctor from 'const I& _data' */
                capval_target_t(const I& _data, R(*_function)(I&, A...), bool dataIsIdentity_) noexcept
                : target_t<R, A...>( delegate_t(static_cast<void*>(this), invoke_impl) ),
                  data(_data), function(_function), dataIsIdentity(dataIsIdentity_) { }

                /** Utilizes move-ctor from moved 'I&& _data' */
                capval_target_t(I&& _data, R(*_function)(I&, A...), bool dataIsIdentity_) noexcept
                : target_t<R, A...>( delegate_t(static_cast<void*>(this), invoke_impl) ),
                  data(std::move(_data)), function(_function), dataIsIdentity(dataIsIdentity_) { }

                target_type type() const noexcept override { return target_type::capval; }
                bool is_null() const noexcept override { return false; }

                target_t<R, A...> * clone() const noexcept override { return new capval_target_t(*this); }

                bool operator==(const target_t<R, A...>& rhs) const noexcept override
                {
                    if( &rhs == this ) {
                        return true;
                    }
                    if( type() != rhs.type() ) {
                        return false;
                    }
                    const capval_target_t<R, I, A...> * prhs = static_cast<const capval_target_t<R, I, A...>*>(&rhs);
                    return dataIsIdentity == prhs->dataIsIdentity && function == prhs->function && ( !dataIsIdentity || data == prhs->data );
                }

                std::string toString() const override {
                    // hack to convert function pointer to void *: '*((void**)&function)'
                    return "capval("+to_hexstring( *((void**)&function) ) + ", sz " +std::to_string(detail_size())+"/"+std::to_string(size())+")";
                }

                size_t detail_size() const noexcept override { return sizeof(data)+sizeof(function)+sizeof(dataIsIdentity); }

                size_t size() const noexcept override { return sizeof(*this); }
        };

        /**
         * func::capref_target_t implementation for functions using a reference to a captured value,
         * identifiable as func::target_type::capref via jau::function<R(A...)>::type().
         *
         * @tparam R function return type
         * @tparam I typename holding the captured data used by the function
         * @tparam A function arguments
         * @see @ref function_overview "Function Overview" etc.
         */
        template<typename R, typename I, typename... A>
        class capref_target_t final : public target_t<R, A...> {
            private:
                typedef typename target_t<R, A...>::delegate_t delegate_t;

                I* data_ptr;
                R(*function)(I*, A...);
                bool dataIsIdentity;

                constexpr static R invoke_impl(void* vbase, A... args) {
                    capref_target_t<R, I, A...>* base = static_cast<capref_target_t<R, I, A...>*>(vbase);
                    return (*base->function)(base->data_ptr, args...);
                }

            public:
                capref_target_t(I* _data_ptr, R(*_function)(I*, A...), bool dataIsIdentity_) noexcept
                : target_t<R, A...>( delegate_t(static_cast<void*>(this), invoke_impl) ),
                  data_ptr(_data_ptr), function(_function), dataIsIdentity(dataIsIdentity_) { }

                target_type type() const noexcept override { return target_type::capref; }
                bool is_null() const noexcept override { return false; }

                target_t<R, A...> * clone() const noexcept override { return new capref_target_t(*this); }

                bool operator==(const target_t<R, A...>& rhs) const noexcept override
                {
                    if( &rhs == this ) {
                        return true;
                    }
                    if( type() != rhs.type() ) {
                        return false;
                    }
                    const capref_target_t<R, I, A...> * prhs = static_cast<const capref_target_t<R, I, A...>*>(&rhs);
                    return dataIsIdentity == prhs->dataIsIdentity && function == prhs->function && ( !dataIsIdentity || data_ptr == prhs->data_ptr );
                }

                std::string toString() const override {
                    // hack to convert function pointer to void *: '*((void**)&function)'
                    return "capref("+to_hexstring( *((void**)&function) ) + ", sz "+std::to_string(detail_size())+"/"+std::to_string(size())+")";
                }

                size_t detail_size() const noexcept override { return sizeof(data_ptr)+sizeof(function)+sizeof(dataIsIdentity); }

                size_t size() const noexcept override { return sizeof(*this); }
        };

        /**
         * func::std_target_t implementation for std::function instances,
         * identifiable as func::target_type::std via jau::function<R(A...)>::type().
         *
         * Notable, instance is holding a unique uint64_t identifier
         * to allow implementing the equality operator, not supported by std::function.
         *
         * @tparam R function return type
         * @tparam A function arguments
         * @see @ref function_overview "Function Overview" etc.
         */
        template<typename R, typename... A>
        class std_target_t : public target_t<R, A...> {
            private:
                typedef typename target_t<R, A...>::delegate_t delegate_t;

                uint64_t id;
                std::function<R(A...)> function;

                constexpr static R invoke_impl(void* vbase, A... args) {
                    std_target_t<R, A...>* base = static_cast<std_target_t<R, A...>*>(vbase);
                    return (base->function)(args...);
                }

            public:
                std_target_t(uint64_t _id, std::function<R(A...)> _function) noexcept
                : target_t<R, A...>( delegate_t(static_cast<void*>(this), invoke_impl) ),
                  id(_id), function(_function) { }

                std_target_t(uint64_t _id) noexcept
                : target_t<R, A...>( delegate_t(static_cast<void*>(this), invoke_impl) ),
                  id(_id), function() { }

                target_type type() const noexcept override { return target_type::std; }
                bool is_null() const noexcept override { return false; }

                target_t<R, A...> * clone() const noexcept override { return new std_target_t(*this); }

                bool operator==(const target_t<R, A...>& rhs) const noexcept override
                {
                    if( &rhs == this ) {
                        return true;
                    }
                    if( type() != rhs.type() ) {
                        return false;
                    }
                    const std_target_t<R, A...> * prhs = static_cast<const std_target_t<R, A...>*>(&rhs);
#if 1
                    return id == prhs->id;
#else
                    const void *d1 = (void*)&function;
                    const void *d2 = (void*)&prhs->function;
                    return 0 == ::memcmp(d1, d2, sizeof(function));
#endif
                }

                std::string toString() const override {
                    return "std("+to_hexstring( id ) + ", sz "+std::to_string(detail_size())+"/"+std::to_string(size())+")";
                }

                size_t detail_size() const noexcept override { return sizeof(id)+sizeof(function); }

                size_t size() const noexcept override { return sizeof(*this); }
        };

        /**@}*/

    } /* namespace func */

    /**
     * Class template [jau::function](@ref function_def) is a general-purpose polymorphic function wrapper.
     *
     * See @ref function_overview "Function Overview".
     *
     * This is the dummy template variant, allowing the void- and non-void return type target specializations.
     *
     * @see @ref function_def "Function template definition"
     */
    template<typename Signature>
    class function;

    /**
     * @anchor function_def
     * Class template [jau::function](@ref function_def) is a general-purpose polymorphic function wrapper.
     *
     * See @ref function_overview "Function Overview".
     *
     * @tparam R function return type
     * @tparam A function arguments
     * @see @ref function_overview "Function Overview" etc.
     * @see @ref function_usage "Function Usage"
     */
    template<typename R, typename... A>
    class function<R(A...)> {
        public:
            /** The target function type, i.e. func::target_t<R, A...> */
            typedef func::target_t<R, A...> target_type;

        private:
            std::shared_ptr<target_type> target_func;
            typename func::target_t<R, A...>::delegate_t* ddetail;

        public:
            /** The target function return type R */
            typedef R result_type;

            /**
             * \brief Null function constructor
             *
             * @anchor function_ctor_def
             * Constructs an instance with a null target function.
             * @see @ref function_overview "function Overview" etc.
             * @see @ref function_usage "function Usage"
             */
            function() noexcept
            : target_func( std::make_shared<func::null_target_t<R, A...>>() ),
              ddetail( &target_func->delegate() )
            { }

            /**
             * \brief Null function constructor
             *
             * @anchor function_ctor_nullptr
             * Constructs an instance with a null target function.
             * @see @ref function_overview "function Overview" etc.
             * @see @ref function_usage "function Usage"
             */
            function(std::nullptr_t ) noexcept
            : target_func( std::make_shared<func::null_target_t<R, A...>>() ),
              ddetail( &target_func->delegate() )
            { }

            /**
             * \brief target_type constructor
             *
             * @anchor function_ctor_target_type
             * Constructs an instance with the given shared target function pointer.
             * @see @ref function_overview "function Overview" etc.
             * @see @ref function_usage "function Usage"
             */
            explicit function(const void* dummy, std::shared_ptr<target_type> _funcPtr) noexcept
            : target_func( _funcPtr ),
              ddetail( &target_func->delegate() )
            { (void)dummy; }

            /**
             * \brief Free function constructor
             *
             * @anchor function_ctor_free
             * Constructs an instance by taking a free target function, which may also be a non-capturing lambda if explicitly bind_free().
             * @see @ref function_overview "function Overview" etc.
             * @see @ref function_usage "function Usage"
             */
            function(R(*func)(A...)) noexcept
            : target_func( std::make_shared<func::free_target_t<R, A...>>(func) ),
              ddetail( &target_func->delegate() )
            { }

            /**
             * \brief Lambda function constructor
             *
             * @anchor function_ctor_lambda
             * Constructs an instance by taking a lambda function.
             * @tparam L typename holding the lambda closure
             * @param func the lambda reference
             * @see @ref function_overview "function Overview" etc.
             * @see @ref function_usage "function Usage"
             */
            template<typename L,
                     std::enable_if_t<!std::is_same_v<L, std::shared_ptr<target_type>> &&
                                      !std::is_pointer_v<L> &&
                                      !std::is_same_v<L, R(A...)> &&
                                      !std::is_same_v<L, function<R(A...)>>
                     , bool> = true>
            function(L func) noexcept
            : target_func( std::make_shared<func::lambda_target_t<R, L, A...>>(func) ),
              ddetail( &target_func->delegate() )
            { }

            /**
             * \brief Member function constructor
             *
             * \anchor function_ctor_member
             * Constructs an instance by taking a member target function.
             * @tparam C typename holding the class type of the member function
             * @param base pointer to the class instance of the member function
             * @param mfunc member function of class
             * @see @ref function_overview "function Overview" etc.
             * @see @ref function_usage "function Usage"
             */
            template<typename C>
            function(C *base, R(C::*mfunc)(A...)) noexcept
            : target_func( std::make_shared<func::member_target_t<R, C, A...>>(base, mfunc) ),
              ddetail( &target_func->delegate() )
            { }

            /**
             * \brief Capture by value (copy) function constructor
             *
             * @anchor function_ctor_capval_copy
             * Constructs an instance by copying the captured value and the given non-void function to
             * an anonymous function using func::capval_target_t.
             *
             * `const I& data` will be copied into func::capval_target_t and hence captured by copy.
             *
             * The function invocation will have the reference of the copied data being passed to the target function for efficiency.
             *
             * @tparam I typename holding the captured data used by the function
             * @param data data type instance holding the captured data
             * @param func function with `R` return value and `A...` arguments.
             * @param dataIsIdentity if true (default), equality requires equal data. Otherwise equality only compares the function pointer.
             * @see @ref function_overview "function Overview" etc.
             * @see @ref function_usage "function Usage"
             */
            template<typename I>
            function(const I& data, R(*func)(I&, A...), bool dataIsIdentity=true) noexcept
            : target_func( std::make_shared<func::capval_target_t<R, I, A...>>(data, func, dataIsIdentity) ),
              ddetail( &target_func->delegate() )
            { }

            /**
             * \brief Capture by value (move) function constructor
             *
             * @anchor function_ctor_capval_move
             * Constructs an instance by moving the captured value and copying the given non-void function to
             * an anonymous function using func::capval_target_t.
             *
             * `I&& data` will be moved into func::capval_target_t.
             *
             * The function invocation will have the reference of the moved data being passed to the target function for efficiency.
             *
             * @tparam I typename holding the captured data used by the function
             * @param data data type instance holding the captured data
             * @param func function with `R` return value and `A...` arguments.
             * @param dataIsIdentity if true (default), equality requires equal data. Otherwise equality only compares the function pointer.
             * @see @ref function_overview "function Overview" etc.
             * @see @ref function_usage "function Usage"
             */
            template<typename I>
            function(I&& data, R(*func)(I&, A...), bool dataIsIdentity=true) noexcept
            : target_func( std::make_shared<func::capval_target_t<R, I, A...>>(std::move(data), func, dataIsIdentity) ),
              ddetail( &target_func->delegate() )
            { }

            /**
             * \brief Capture by reference function constructor
             *
             * @anchor function_ctor_capref
             * Constructs an instance by passing the captured reference (pointer) to the value and non-void function to
             * an anonymous function using func::capref_target_t.
             *
             * The function invocation will have the reference of the data being passed to the target function.
             *
             * @tparam I typename holding the captured data used by the function
             * @param data_ptr data type reference to instance holding the captured data
             * @param func function with `R` return value and `A...` arguments.
             * @param dataIsIdentity if true (default), equality requires same data_ptr. Otherwise equality only compares the function pointer.
             * @see @ref function_overview "function Overview" etc.
             * @see @ref function_usage "function Usage"
             */
            template<typename I>
            function(I* data_ptr, R(*func)(I*, A...), bool dataIsIdentity=true) noexcept
            : target_func( std::make_shared<func::capref_target_t<R, I, A...>>(data_ptr, func, dataIsIdentity) ),
              ddetail( &target_func->delegate() )
            { }

            /**
             * \brief std::function constructor
             *
             * @anchor function_ctor_std
             * Constructs an instance by copying the std::function to
             * an anonymous function using func::std_target_t.
             *
             * Notable, instance is holding the given unique uint64_t identifier
             * to allow implementing the equality operator w/o RTTI, not supported by std::function.
             *
             * @param func free-function with `R` return value and `A...` arguments.
             * @see @ref function_overview "function Overview" etc.
             * @see @ref function_usage "function Usage"
             */
            function(uint64_t id, std::function<R(A...)> func) noexcept
            : target_func( std::make_shared<func::std_target_t<R, A...>>(id, func) ),
              ddetail( &target_func->delegate() )
            { }

            function(const function &o) noexcept
            : target_func( o.target_func ),
              ddetail( &target_func->delegate() )
            { }

            function(function &&o) noexcept
            : target_func( std::move( o.target_func ) ),
              ddetail( &target_func->delegate() )
            { o.target_func = nullptr; }

            function& operator=(const function &o) noexcept
            {
                target_func = o.target_func;
                ddetail = &target_func->delegate();
                return *this;
            }

            function& operator=(function &&o) noexcept
            {
                target_func = std::move(o.target_func);
                ddetail = &target_func->delegate();
                o.target_func = nullptr;
                return *this;
            }

            /** Return the jau::func::type of this instance */
            func::target_type type() const noexcept { return target_func->type(); }

            /** Returns true if this instance does not hold a callable target function, i.e. is of func::target_type::null.  */
            bool is_null() const noexcept { return target_func->is_null(); }

            /** Returns true if this instance holds a callable target function, i.e. is not of func::target_type::null.  */
            explicit operator bool() const noexcept { return !target_func->is_null(); }

            /** Returns the shared target function. */
            std::shared_ptr<target_type> target() const noexcept { return target_func; }

            std::string toString() const {
                return "function[ "+target_func->toString()+", sz "+std::to_string(size())+" ]";
            }

            /** Net size of this target_t instance's function details. */
            size_t target_detail_size() const noexcept { return target_func->detail_size(); }

            /** Total size of this target_t instance. */
            size_t target_size() const noexcept { return target_func->size(); }

            /** Total size of this instance, incl. shared target_t size. */
            size_t size() const noexcept { return sizeof(*this) + target_func->size(); }

            constexpr R operator()(A... args) const {
                return ddetail->invoke(args...);
            }
            constexpr R operator()(A... args) {
                return ddetail->invoke(args...);
            }
    };

    template<typename R, typename... A>
    bool operator==(const function<R(A...)>& lhs, const function<R(A...)>& rhs) noexcept
    { return *lhs.target() == *rhs.target(); }

    template<typename R, typename... A>
    bool operator!=(const function<R(A...)>& lhs, const function<R(A...)>& rhs) noexcept
    { return !( lhs == rhs ); }

    /**
     * Bind given class instance and non-void member function to
     * an anonymous function using func_member_targer_t.
     *
     * @tparam R function return type
     * @tparam C class type holding the member-function
     * @tparam A function arguments
     * @param base class instance `this` pointer
     * @param mfunc member-function with `R` return value and `A...` arguments.
     * @return anonymous function
     * @see @ref function_ctor_member "function constructor for member function"
     * @see @ref function_overview "function Overview" etc.
     * @see @ref function_usage "function Usage"
     */
    template<typename R, typename C, typename... A>
    inline jau::function<R(A...)>
    bind_member(C *base, R(C::*mfunc)(A...)) noexcept {
        return function<R(A...)>( nullptr, std::make_shared<func::member_target_t<R, C, A...>>(base, mfunc) );
    }

    /**
     * Bind given class instance and void member function to
     * an anonymous function using func_member_targer_t.
     *
     * @tparam C class type holding the member-function
     * @tparam A function arguments
     * @param base class instance `this` pointer
     * @param mfunc member-function with `A...` arguments.
     * @return anonymous function
     * @see @ref function_ctor_member "function constructor for member function"
     * @see @ref function_overview "function Overview" etc.
     * @see @ref function_usage "function Usage"
     */
    template<typename C, typename... A>
    inline jau::function<void(A...)>
    bind_member(C *base, void(C::*mfunc)(A...)) noexcept {
        return function<void(A...)>( nullptr, std::make_shared<func::member_target_t<void, C, A...>>(base, mfunc) );
    }

    /**
     * Bind given non-void free-function to
     * an anonymous function using func::free_target_t.
     *
     * @tparam R function return type
     * @tparam A function arguments
     * @param func free-function with `R` return value and `A...` arguments.
     * @return anonymous function
     * @see @ref function_ctor_free "function constructor for free function"
     * @see @ref function_overview "function Overview" etc.
     * @see @ref function_usage "function Usage"
     */
    template<typename R, typename... A>
    inline jau::function<R(A...)>
    bind_free(R(*func)(A...)) noexcept {
        return function<R(A...)>( nullptr, std::make_shared<func::free_target_t<R, A...>>(func) );
    }

    /**
     * Bind given void free-function to
     * an anonymous function using func::free_target_t.
     *
     * @tparam A function arguments
     * @param func free-function with `A...` arguments.
     * @return anonymous function
     * @see @ref function_ctor_free "function constructor for free function"
     * @see @ref function_overview "function Overview" etc.
     * @see @ref function_usage "function Usage"
     */
    template<typename... A>
    inline jau::function<void(A...)>
    bind_free(void(*func)(A...)) noexcept {
        return function<void(A...)>( nullptr, std::make_shared<func::free_target_t<void, A...>>(func) );
    }

#if 0
    // Lacks proper template type deduction, sadly

    /**
     * Bind given capturing non-void returning lambda by copying the it to
     * an anonymous function using func::lambda_target_t.
     *
     * @tparam R function return type
     * @tparam L typename holding the lambda closure
     * @tparam A function arguments
     * @param func the lambda reference
     * @return anonymous function
     * @see @ref function_ctor_lambda "function constructor for lambda"
     * @see @ref function_overview "function Overview" etc.
     * @see @ref function_usage "function Usage"
     */
    template<typename R, typename L, typename... A>
    inline jau::function<R(A...)>
    bind_lambda(L func) noexcept {
        return function<R(A...)>( nullptr, std::make_shared<func::lambda_target_t<R, L, A...>>(func) );
    }

    /**
     * Bind given capturing void returning lambda by copying the it to
     * an anonymous function using func::lambda_target_t.
     *
     * @tparam L typename holding the lambda closure
     * @tparam A function arguments
     * @param func the lambda reference
     * @return anonymous function
     * @see @ref function_ctor_lambda "function constructor for lambda"
     * @see @ref function_overview "function Overview" etc.
     * @see @ref function_usage "function Usage"
     */
    template<typename L, typename... A>
    inline jau::function<void(A...)>
    bind_lambda(L func) noexcept {
        return function<void(A...)>( nullptr, std::make_shared<func::lambda_target_t<void, L, A...>>(func) );
    }
#endif

    /**
     * Bind given data by copying the captured value and the given non-void function to
     * an anonymous function using func::capval_target_t.
     *
     * `const I& data` will be copied into func::capval_target_t and hence captured by copy.
     *
     * The function invocation will have the reference of the copied data being passed to the target function for efficiency.
     *
     * @tparam R function return type
     * @tparam I typename holding the captured data used by the function
     * @tparam A function arguments
     * @param data data type instance holding the captured data
     * @param func function with `R` return value and `A...` arguments.
     * @param dataIsIdentity if true (default), equality requires equal data. Otherwise equality only compares the function pointer.
     * @return anonymous function
     * @see @ref function_ctor_capval_copy "function constructor for copying capturing value"
     * @see @ref function_overview "function Overview" etc.
     * @see @ref function_usage "function Usage"
     */
    template<typename R, typename I, typename... A>
    inline jau::function<R(A...)>
    bind_capval(const I& data, R(*func)(I&, A...), bool dataIsIdentity=true) noexcept {
        return function<R(A...)>( nullptr, std::make_shared<func::capval_target_t<R, I, A...>>(data, func, dataIsIdentity) );
    }

    /**
     * Bind given data by copying the captured value and the given void function to
     * an anonymous function using func::capval_target_t.
     *
     * `const I& data` will be copied into func::capval_target_t and hence captured by copy.
     *
     * The function invocation will have the reference of the copied data being passed to the target function for efficiency.
     *
     * @tparam I typename holding the captured data used by the function
     * @tparam A function arguments
     * @param data data type instance holding the captured data
     * @param func function with `A...` arguments.
     * @param dataIsIdentity if true (default), equality requires equal data. Otherwise equality only compares the function pointer.
     * @return anonymous function
     * @see @ref function_ctor_capval_copy "function constructor for copying capturing value"
     * @see @ref function_overview "function Overview" etc.
     * @see @ref function_usage "function Usage"
     */
    template<typename I, typename... A>
    inline jau::function<void(A...)>
    bind_capval(const I& data, void(*func)(I&, A...), bool dataIsIdentity=true) noexcept {
        return function<void(A...)>( nullptr, std::make_shared<func::capval_target_t<void, I, A...>>(data, func, dataIsIdentity) );
    }

    /**
     * Bind given data by moving the captured value and copying the given non-void function to
     * an anonymous function using func::capval_target_t.
     *
     * `I&& data` will be moved into func::capval_target_t.
     *
     * The function invocation will have the reference of the moved data being passed to the target function for efficiency.
     *
     * @tparam R function return type
     * @tparam I typename holding the captured data used by the function
     * @tparam A function arguments
     * @param data data type instance holding the captured data
     * @param func function with `R` return value and `A...` arguments.
     * @param dataIsIdentity if true (default), equality requires equal data. Otherwise equality only compares the function pointer.
     * @return anonymous function
     * @see @ref function_ctor_capval_move "function constructor for moving capturing value"
     * @see @ref function_overview "function Overview" etc.
     * @see @ref function_usage "function Usage"
     */
    template<typename R, typename I, typename... A>
    inline jau::function<R(A...)>
    bind_capval(I&& data, R(*func)(I&, A...), bool dataIsIdentity=true) noexcept {
        return function<R(A...)>( nullptr, std::make_shared<func::capval_target_t<R, I, A...>>(std::move(data), func, dataIsIdentity) );
    }

    /**
     * Bind given data by moving the captured value and copying the given void function to
     * an anonymous function using func::capval_target_t.
     *
     * `I&& data` will be moved into func::capval_target_t.
     *
     * The function invocation will have the reference of the moved data being passed to the target function for efficiency.
     *
     * @tparam I typename holding the captured data used by the function
     * @tparam A function arguments
     * @param data data type instance holding the captured data
     * @param func function with `A...` arguments.
     * @param dataIsIdentity if true (default), equality requires equal data. Otherwise equality only compares the function pointer.
     * @return anonymous function
     * @see @ref function_ctor_capval_move "function constructor for moving capturing value"
     * @see @ref function_overview "function Overview" etc.
     * @see @ref function_usage "function Usage"
     */
    template<typename I, typename... A>
    inline jau::function<void(A...)>
    bind_capval(I&& data, void(*func)(I&, A...), bool dataIsIdentity=true) noexcept {
        return function<void(A...)>( nullptr, std::make_shared<func::capval_target_t<void, I, A...>>(std::move(data), func, dataIsIdentity) );
    }

    /**
     * Bind given data by passing the captured reference (pointer) to the value and non-void function to
     * an anonymous function using func::capref_target_t.
     *
     * The function invocation will have the reference of the data being passed to the target function.
     *
     * @tparam R function return type
     * @tparam I typename holding the captured data used by the function
     * @tparam A function arguments
     * @param data_ptr data type reference to instance holding the captured data
     * @param func function with `R` return value and `A...` arguments.
     * @param dataIsIdentity if true (default), equality requires same data_ptr. Otherwise equality only compares the function pointer.
     * @return anonymous function
     * @see @ref function_ctor_capref "function constructor for capturing reference"
     * @see @ref function_overview "function Overview" etc.
     * @see @ref function_usage "function Usage"
     */
    template<typename R, typename I, typename... A>
    inline jau::function<R(A...)>
    bind_capref(I* data_ptr, R(*func)(I*, A...), bool dataIsIdentity=true) noexcept {
        return function<R(A...)>( nullptr, std::make_shared<func::capref_target_t<R, I, A...>>(data_ptr, func, dataIsIdentity) );
    }

    /**
     * Bind given data by passing the captured reference (pointer) to the value and void function to
     * an anonymous function using func::capref_target_t.
     *
     * The function invocation will have the reference of the data being passed to the target function.
     *
     * @tparam I typename holding the captured data used by the function
     * @tparam A function arguments
     * @param data_ptr data type reference to instance holding the captured data
     * @param func function with `A...` arguments.
     * @param dataIsIdentity if true (default), equality requires same data_ptr. Otherwise equality only compares the function pointer.
     * @return anonymous function
     * @see @ref function_ctor_capref "function constructor for capturing reference"
     * @see @ref function_overview "function Overview" etc.
     * @see @ref function_usage "function Usage"
     */
    template<typename I, typename... A>
    inline jau::function<void(A...)>
    bind_capref(I* data_ptr, void(*func)(I*, A...), bool dataIsIdentity=true) noexcept {
        return function<void(A...)>( nullptr, std::make_shared<func::capref_target_t<void, I, A...>>(data_ptr, func, dataIsIdentity) );
    }

    /**
     * Bind given non-void std::function to
     * an anonymous function using func::std_target_t.
     *
     * Notable, instance is holding the given unique uint64_t identifier
     * to allow implementing the equality operator w/o RTTI, not supported by std::function.
     *
     * @tparam R function return type
     * @tparam A function arguments
     * @param func free-function with `R` return value and `A...` arguments.
     * @return anonymous function
     * @see @ref function_ctor_std "function constructor for std::function"
     * @see @ref function_overview "function Overview" etc.
     * @see @ref function_usage "function Usage"
     */
    template<typename R, typename... A>
    inline jau::function<R(A...)>
    bind_std(uint64_t id, std::function<R(A...)> func) noexcept {
        return function<R(A...)>( nullptr, std::make_shared<func::std_target_t<R, A...>>(id, func) );
    }

    /**
     * Bind given void std::function to
     * an anonymous function using func::std_target_t.
     *
     * Notable, instance is holding the given unique uint64_t identifier
     * to allow implementing the equality operator w/o RTTI, not supported by std::function.
     *
     * @tparam A function arguments
     * @param func free-function with `A...` arguments.
     * @return anonymous function
     * @see @ref function_ctor_std "function constructor for std::function"
     * @see @ref function_overview "function Overview" etc.
     * @see @ref function_usage "function Usage"
     */
    template<typename... A>
    inline jau::function<void(A...)>
    bind_std(uint64_t id, std::function<void(A...)> func) noexcept {
        return function<void(A...)>( nullptr, std::make_shared<func::std_target_t<void, A...>>(id, func) );
    }

    /**@}*/

} // namespace jau

/** \example test_functional01.cpp
 * This C++ unit test validates the jau::function<R(A...)> and all its jau::func::target_t specializations.
 */

/** \example test_functional_perf.hpp
 * This C++ unit test benchmarks the jau::function<R(A...)> and all its jau::func::target_t specializations.
 */

#endif /* JAU_FUNCTIONAL_HPP_ */
