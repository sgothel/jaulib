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
     * e.g. free functions, capturing and non-capturing lambda function, member functions,
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
     * - exposes the target function signature jau::type_info via jau::function::signature()
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
     *   - constructor [function(I* data_ptr, R(*func)(I*, A...))](@ref function_ctor_capref)
     * - lambda alike functions using a captured data type by value
     *   - bind_capval()
     *   - constructor copy [function(const I& data, R(*func)(I&, A...))](@ref function_ctor_capval_copy)
     *   - constructor move [function(I&& data, R(*func)(I&, A...))](@ref function_ctor_capval_move)
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
     *   - Constructor [function(I* data_ptr, R(*func)(I*, A...))](@ref function_ctor_capref)
     *   ```
     *   function<int(int)> func(&data,
     *       (cfunc) ( [](big_data* data, int v) -> bool {
     *                 stats_ptr->sum += v;
     *                 return 0 == v;
     *             } ) );
     *   ```
     *
     *   - Bind `jau::function<R(A...)> jau::bind_capref(I* data_ptr, R(*func)(I*, A...))`
     *   ```
     *   jau::function<bool(int)> func = jau::bind_capref(&data,
     *       (cfunc) ( [](big_data* data, int v) -> bool {
     *                 stats_ptr->sum += v;
     *                 return 0 == v;
     *             } ) );
     *   ```
     *
     * - Lambda alike capture by copy of value via constructor and jau::bind_capval()
     *   - Constructor copy [function(const I& data, R(*func)(I&, A...))](@ref function_ctor_capval_copy)
     *   - Constructor move [function(I&& data, R(*func)(I&, A...))](@ref function_ctor_capval_move)
     *   - Bind copy `jau::function<R(A...)> jau::bind_capval(const I& data, R(*func)(I&, A...))`
     *   - Bind move `jau::function<R(A...)> jau::bind_capval(I&& data, R(*func)(I&, A...))` <br />
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
     * ##### Non unique lambda type names without RTTI using `gcc` or non `clang` compiler
     *
     * Due to [limitations of jau::make_ctti<R, L, A...>()](@ref ctti_name_lambda_limitations),
     * *not using RTTI on `gcc` or non `clang` compiler* will *erroneously mistake* different lambda
     * *functions defined within one function and using same function prototype `R<A...>`* to be the same.
     *
     * jau::type_info::limited_lambda_id will expose the potential limitation.
     *
     * See [CTTI lambda name limitations](@ref ctti_name_lambda_limitations) and [limitations of jau::type_info](@ref type_info_limitations).
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
         * @see @ref function_overview "Function Overview"
         */
        enum class target_type : uint32_t {
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

        /**
         * func::target_t pure-virtual interface for [jau::function](@ref function_def).
         *
         * @tparam R function return type
         * @tparam A function arguments
         * @see @ref function_overview "Function Overview"
         */
        template<typename R, typename... A>
        class target_t {
            private:
                target_type ttype;

            protected:
                target_t(target_type ttype_) noexcept
                : ttype(ttype_) {}

            public:
                virtual ~target_t() noexcept {}

                target_t(const target_t &o) noexcept = default;
                target_t(target_t &&o) noexcept = default;
                target_t& operator=(const target_t &o) noexcept = default;
                target_t& operator=(target_t &&o) noexcept = default;

                /** Return the func::target_type of this invocation function wrapper */
                constexpr target_type type() const noexcept { return ttype; }

                virtual R operator()(A... args) = 0;
                virtual bool operator==(const target_t<R, A...>& rhs) const noexcept = 0;
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
         * @see @ref function_overview "Function Overview"
         */
        template<typename R, typename... A>
        class null_target_t final : public target_t<R, A...> {
            public:
                null_target_t() noexcept
                : target_t<R, A...>( target_type::null )
                { }

                R operator()(A... args) override {
                    (void)(... , args);
                    return R();
                }

                bool operator==(const target_t<R, A...>& rhs) const noexcept override {
                    return target_t<R, A...>::type() == rhs.type();
                }
        };

        /**
         * func::member_target_t implementation for class member functions,
         * identifiable as func::target_type::member via jau::function<R(A...)>::type().
         *
         * @tparam R function return type
         * @tparam C0 class type holding the member-function
         * @tparam C1 class derived from C0 or C0 of this base-pointer used to invoke the member-function
         * @tparam A function arguments
         * @see @ref function_overview "Function Overview"
         */
        template<typename R, typename C0, typename C1, typename... A>
        class member_target_t final : public target_t<R, A...> {
            private:
                C1* base;
                R(C0::*member)(A...);

            public:
                /**
                 * Construct a target_t<R, A...> instance from given this base-pointer and member-function.
                 *
                 * This factory function is only enabled if C0 is base of C1.
                 *
                 * @param _base this base-pointer of class C1 derived from C0 or C0 used to invoke the member-function
                 * @param _member member-function of class C0
                 * @return target_t<R, A...> instance holding the target-function details.
                 */
                member_target_t(C1 *_base, R(C0::*_member)(A...),
                                std::enable_if_t<std::is_base_of_v<C0, C1>, bool> = true) noexcept
                : target_t<R, A...>( target_type::member ),
                  base(_base), member(_member) { }

                R operator()(A... args) override {
                    return (base->*member)(args...);
                }

                bool operator==(const target_t<R, A...>& rhs) const noexcept override {
                    const member_target_t<R, C0, C1, A...>* prhs = static_cast<const member_target_t<R, C0, C1, A...>*>(&rhs);
                    return this == prhs ||
                           ( target_t<R, A...>::type() == rhs.type() &&
                             base == prhs->base &&
                             member == prhs->member
                           );
                }
        };

        /**
         * func::free_target_t implementation for free functions,
         * identifiable as func::target_type::free via jau::function<R(A...)>::type().
         *
         * @tparam R function return type
         * @tparam A function arguments
         * @see @ref function_overview "Function Overview"
         */
        template<typename R, typename... A>
        class free_target_t final : public target_t<R, A...> {
            private:
                R(*function)(A...);

            public:
                free_target_t(R(*_function)(A...)) noexcept
                : target_t<R, A...>( target_type::free ),
                  function(_function) { }

                R operator()(A... args) override {
                    return (*function)(args...);
                }

                bool operator==(const target_t<R, A...>& rhs) const noexcept override {
                    const free_target_t<R, A...>* prhs = static_cast<const free_target_t<R, A...>*>(&rhs);
                    return this == prhs ||
                           ( target_t<R, A...>::type() == rhs.type() &&
                             function == prhs->function
                           );
                }
        };

        /**
         * func::lambda_target_t implementation for lambda closures,
         * identifiable as func::target_type::lambda via jau::function<R(A...)>::type().
         *
         * @tparam R function return type
         * @tparam L typename holding the lambda closure
         * @tparam A function arguments
         * @see @ref function_overview "Function Overview"
         */
        template<typename R, typename L, typename...A>
        class lambda_target_t final : public target_t<R, A...>
        {
            private:
                jau::type_info sig;
                L function;

                constexpr size_t detail_size() const noexcept { return sizeof(function); }

            public:
                lambda_target_t(L function_) noexcept
                : target_t<R, A...>( target_type::lambda ),
                  sig( jau::make_ctti<R, L, A...>() ), function(function_)
                { }

                R operator()(A... args) override {
                    return function(args...);
                }

                bool operator==(const target_t<R, A...>& rhs) const noexcept override {
                    const lambda_target_t<R, L, A...>* prhs = static_cast<const lambda_target_t<R, L, A...>*>(&rhs);
                    return this == prhs ||
                           ( target_t<R, A...>::type() == rhs.type() &&
                             detail_size() == prhs->detail_size() &&                                     // fast:  wrong size -> false, otherwise ...
                             sig == prhs->sig &&                                                         // mixed: wrong jau::type_info -> false, otherwise ...
                             0 == ::memcmp((void*)&function, (void*)&prhs->function, sizeof(function))   // slow:  compare the anonymous data chunk of the lambda
                           );
                }
        };

        /**
         * func::capval_target_t implementation for functions using a copy of a captured value,
         * identifiable as func::target_type::capval via jau::function<R(A...)>::type().
         *
         * @tparam R function return type
         * @tparam I typename holding the captured data used by the function
         * @tparam A function arguments
         * @see @ref function_overview "Function Overview"
         */
        template<typename R, typename I, typename... A>
        class capval_target_t final : public target_t<R, A...> {
            private:
                R(*function)(I&, A...);
                I data;

            public:
                /** Utilizes copy-ctor from 'const I& _data' */
                capval_target_t(const I& _data, R(*_function)(I&, A...)) noexcept
                : target_t<R, A...>( target_type::capval ),
                  function(_function), data(_data) { }

                /** Utilizes move-ctor from moved 'I&& _data' */
                capval_target_t(I&& _data, R(*_function)(I&, A...)) noexcept
                : target_t<R, A...>( target_type::capval ),
                  function(_function), data(std::move(_data)) { }

                R operator()(A... args) override {
                    return (*function)(data, args...);
                }

                bool operator==(const target_t<R, A...>& rhs) const noexcept override {
                    const capval_target_t<R, I, A...>* prhs = static_cast<const capval_target_t<R, I, A...>*>(&rhs);
                    return this == prhs ||
                           ( target_t<R, A...>::type() == rhs.type() &&
                             function == prhs->function &&
                             data == prhs->data
                           );
                }
        };

        /**
         * func::capref_target_t implementation for functions using a reference to a captured value,
         * identifiable as func::target_type::capref via jau::function<R(A...)>::type().
         *
         * @tparam R function return type
         * @tparam I typename holding the captured data used by the function
         * @tparam A function arguments
         * @see @ref function_overview "Function Overview"
         */
        template<typename R, typename I, typename... A>
        class capref_target_t final : public target_t<R, A...> {
            private:
                R(*function)(I*, A...);
                I* data_ptr;

            public:
                capref_target_t(I* _data_ptr, R(*_function)(I*, A...)) noexcept
                : target_t<R, A...>( target_type::capref ),
                  function(_function), data_ptr(_data_ptr) { }

                R operator()(A... args) override {
                    return (*function)(data_ptr, args...);
                }

                bool operator==(const target_t<R, A...>& rhs) const noexcept override {
                    const capref_target_t<R, I, A...>* prhs = static_cast<const capref_target_t<R, I, A...>*>(&rhs);
                    return this == prhs ||
                           ( target_t<R, A...>::type() == rhs.type() &&
                             function == prhs->function &&
                             data_ptr == prhs->data_ptr
                           );
                }
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
         * @see @ref function_overview "Function Overview"
         */
        template<typename R, typename... A>
        class std_target_t : public target_t<R, A...> {
            private:
                uint64_t id;
                std::function<R(A...)> function;

                constexpr size_t detail_size() const noexcept { return sizeof(id)+sizeof(function); }

            public:
                std_target_t(uint64_t _id, std::function<R(A...)> _function) noexcept
                : target_t<R, A...>( target_type::std ),
                  id(_id), function(_function) { }

                std_target_t(uint64_t _id) noexcept
                : target_t<R, A...>( target_type::std ),
                  id(_id), function() { }

                R operator()(A... args) override {
                    return function(args...);
                }

                bool operator==(const target_t<R, A...>& rhs) const noexcept override {
                    const std_target_t<R, A...>* prhs = static_cast<const std_target_t<R, A...>*>(&rhs);
                    return this == prhs ||
                           ( target_t<R, A...>::type() == rhs.type() &&
                             id == prhs->id &&
                             detail_size() && prhs->detail_size()
                           );
                }
        };

        /**@}*/

    } /* namespace func */

     constexpr uint32_t number(const func::target_type rhs) noexcept {
         return static_cast<uint32_t>(rhs);
     }
     std::string to_string(const func::target_type v) noexcept;

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
     * @see @ref function_overview "Function Overview"
     * @see @ref function_usage "Function Usage"
     */
    template<typename R, typename... A>
    class function<R(A...)> final {
        public:
            /** The target function type, i.e. func::target_t<R, A...> */
            typedef func::target_t<R, A...> target_type;

        private:
            std::shared_ptr<target_type> target_func;
            size_t target_func_size;

        public:
            /** The target function return type R */
            typedef R result_type;

            /**
             * \brief Null function constructor
             *
             * @anchor function_ctor_def
             * Constructs an instance with a null target function.
             * @see @ref function_overview "function Overview"
             * @see @ref function_usage "function Usage"
             */
            function() noexcept
            : target_func( std::make_shared<func::null_target_t<R, A...>>() ),
              target_func_size( sizeof(func::null_target_t<R, A...>))
            { }

            /**
             * \brief Null function constructor
             *
             * @anchor function_ctor_nullptr
             * Constructs an instance with a null target function.
             * @see @ref function_overview "function Overview"
             * @see @ref function_usage "function Usage"
             */
            function(std::nullptr_t ) noexcept
            : target_func( std::make_shared<func::null_target_t<R, A...>>() ),
              target_func_size( sizeof(func::null_target_t<R, A...>))
            { }

            /**
             * \brief target_type constructor
             *
             * @anchor function_ctor_target_type
             * Constructs an instance with the given shared target function pointer.
             * @see @ref function_overview "function Overview"
             * @see @ref function_usage "function Usage"
             */
            explicit function(std::shared_ptr<target_type> _funcPtr, size_t asize_) noexcept
            : target_func( _funcPtr ),
              target_func_size( asize_ )
            { }

            /**
             * \brief Free function constructor
             *
             * @anchor function_ctor_free
             * Constructs an instance by taking a free target function, which may also be a non-capturing lambda if explicitly bind_free().
             * @see @ref function_overview "function Overview"
             * @see @ref function_usage "function Usage"
             */
            function(R(*func)(A...)) noexcept
            : target_func( std::make_shared<func::free_target_t<R, A...>>(func) ),
              target_func_size( sizeof(func::free_target_t<R, A...>))
            { }

            /**
             * \brief Lambda function constructor
             *
             * @anchor function_ctor_lambda
             * Constructs an instance by taking a lambda function.
             * @tparam L typename holding the lambda closure
             * @param func the lambda reference
             * @see @ref function_overview "function Overview"
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
              target_func_size( sizeof(func::lambda_target_t<R, L, A...>))
            { }

            /**
             * \brief Member function constructor
             *
             * \anchor function_ctor_member
             * Constructs an instance by taking a member target function.
             * @tparam C0 class type holding the member-function
             * @tparam C1 class derived from C0 or C0 of this base-pointer used to invoke the member-function
             * @param base pointer to the class instance of the member function
             * @param mfunc member function of class
             * @see @ref function_overview "function Overview"
             * @see @ref function_usage "function Usage"
             */
            template<typename C0, typename C1>
            function(C1 *base, R(C0::*mfunc)(A...)) noexcept
            : target_func( std::make_shared<func::member_target_t<R, C0, C1, A...>>(base, mfunc) ),
              target_func_size( sizeof(func::member_target_t<R, C0, C1, A...>))
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
             * @see @ref function_overview "function Overview"
             * @see @ref function_usage "function Usage"
             */
            template<typename I>
            function(const I& data, R(*func)(I&, A...)) noexcept
            : target_func( std::make_shared<func::capval_target_t<R, I, A...>>(data, func) ),
              target_func_size( sizeof(func::capval_target_t<R, I, A...>))
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
             * @see @ref function_overview "function Overview"
             * @see @ref function_usage "function Usage"
             */
            template<typename I>
            function(I&& data, R(*func)(I&, A...)) noexcept
            : target_func( std::make_shared<func::capval_target_t<R, I, A...>>(std::move(data), func) ),
              target_func_size( sizeof(func::capval_target_t<R, I, A...>))
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
             * @see @ref function_overview "function Overview"
             * @see @ref function_usage "function Usage"
             */
            template<typename I>
            function(I* data_ptr, R(*func)(I*, A...)) noexcept
            : target_func( std::make_shared<func::capref_target_t<R, I, A...>>(data_ptr, func) ),
              target_func_size( sizeof(func::capref_target_t<R, I, A...>))
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
             * @see @ref function_overview "function Overview"
             * @see @ref function_usage "function Usage"
             */
            function(uint64_t id, std::function<R(A...)> func) noexcept
            : target_func( std::make_shared<func::std_target_t<R, A...>>(id, func) ),
              target_func_size( sizeof(func::std_target_t<R, A...>))
            { }

            function(const function &o) noexcept = default;
            function(function &&o) noexcept = default;
            function& operator=(const function &o) noexcept = default;
            function& operator=(function &&o) noexcept = default;

            /** Return the jau::func::type of this instance */
            constexpr func::target_type type() const noexcept { return target_func->type(); }

            /** Returns true if this instance does not hold a callable target function, i.e. is of func::target_type::null.  */
            constexpr bool is_null() const noexcept { return func::target_type::null == target_func->type(); }

            /** Returns true if this instance holds a callable target function, i.e. is not of func::target_type::null.  */
            explicit constexpr operator bool() const noexcept { return !is_null(); }

            /** Returns signature of this function prototype R(A...) w/o underlying target function details. */
            jau::type_info signature() const noexcept {
                return jau::make_ctti<R(A...)>();
            }

            /** Returns the size of the target type  */
            constexpr size_t target_size() const noexcept { return target_func_size; }

            std::string toString() const {
                return "function<" + to_string( type() ) + ", " + signature().demangled_name() + ">( sz target_data " +
                        std::to_string( target_func_size ) + " + shared_ptr " +
                        std::to_string( sizeof( target_func ) ) + " + extra " +
                        std::to_string( sizeof( target_func_size ) ) + " -> " +
                        std::to_string( sizeof( *this ) + target_func_size ) + " ) ";
            }

            constexpr R operator()(A... args) const {
                return (*target_func)(args...);
            }
            constexpr R operator()(A... args) {
                return (*target_func)(args...);
            }

            constexpr bool operator==(const function<R(A...)>& rhs) const noexcept {
                return target_func->operator==(*rhs.target_func);
            }
            constexpr bool operator!=(const function<R(A...)>& rhs) const noexcept {
                return !operator==(rhs);
            }
    };

    /**
     * Equal operator using different jau::function<R(A...)> return and argument types for both arguments,
     * always returns false.
     * @tparam Rl left function return type
     * @tparam Al left function arguments
     * @tparam Fl left function Fl<Rl(<Al...)>
     * @tparam Rr right function return type
     * @tparam Ar right function arguments
     * @tparam Fr right function Fr<Rr(<Ar...)>
     * @param lhs left function Fl<Rl(<Al...)>
     * @param rhs right function Fr<Rr(<Ar...)>
     * @return false
     * @see @ref function_overview "function Overview"
     * @see @ref function_usage "function Usage"
     */
    template<typename Rl, typename... Al, template <typename...> class Fl = function,
             typename Rr, typename... Ar, template <typename...> class Fr = function,
             std::enable_if_t< !std::is_same_v< Fl<Rl(Al...)>, Fr<Rr(Ar...)> >
             , bool> = true>
    bool operator==(const function<Rl(Al...)>& lhs, const function<Rr(Ar...)>& rhs) noexcept
    {
        (void)lhs;
        (void)rhs;
        return false;
    }

    /**
     * Equal operator using same jau::function<R(A...)> return and argument types for both arguments,
     * returning actual result of equality operation.
     * @tparam Rl left function return type
     * @tparam Al left function arguments
     * @tparam Fl left function Fl<Rl(<Al...)>
     * @tparam Rr right function return type
     * @tparam Ar right function arguments
     * @tparam Fr right function Fr<Rr(<Ar...)>
     * @param lhs left function Fl<Rl(<Al...)>
     * @param rhs right function Fr<Rr(<Ar...)>
     * @return equality result of same type functions
     * @see @ref function_overview "function Overview"
     * @see @ref function_usage "function Usage"
     */
    template<typename Rl, typename... Al, template <typename...> class Fl = function,
             typename Rr, typename... Ar, template <typename...> class Fr = function,
             std::enable_if_t< std::is_same_v< Fl<Rl(Al...)>, Fr<Rr(Ar...)> >
             , bool> = true>
    bool operator==(const function<Rl(Al...)>& lhs, const function<Rr(Ar...)>& rhs) noexcept
    { return lhs.operator==( rhs ); }

    /**
     * Unequal operator using two jau::function<R(A...)> types for both arguments.
     * @tparam Rl left function return type
     * @tparam Al left function arguments
     * @tparam Fl left function Fl<Rl(<Al...)>
     * @tparam Rr right function return type
     * @tparam Ar right function arguments
     * @tparam Fr right function Fr<Rr(<Ar...)>
     * @param lhs left function Fl<Rl(<Al...)>
     * @param rhs right function Fr<Rr(<Ar...)>
     * @return unequality result of same type functions
     * @see @ref function_overview "function Overview"
     * @see @ref function_usage "function Usage"
     */
    template<typename Rl, typename... Al, template <typename...> typename Fl = function,
             typename Rr, typename... Ar, template <typename...> typename Fr = function>
    bool operator!=(const function<Rl(Al...)>& lhs, const function<Rr(Ar...)>& rhs) noexcept
    { return !( lhs == rhs ); }

    /**
     * Bind given class instance and non-void member function to
     * an anonymous function using func_member_targer_t.
     *
     * @tparam R function return type
     * @tparam C0 class type holding the member-function
     * @tparam C1 class derived from C0 or C0 of this base-pointer used to invoke the member-function
     * @tparam A function arguments
     * @param base class instance `this` pointer
     * @param mfunc member-function with `R` return value and `A...` arguments.
     * @return anonymous function
     * @see @ref function_ctor_member "function constructor for member function"
     * @see @ref function_overview "function Overview"
     * @see @ref function_usage "function Usage"
     */
    template<typename R, typename C0, typename C1, typename... A>
    inline jau::function<R(A...)>
    bind_member(C1 *base, R(C0::*mfunc)(A...)) noexcept {
        return function<R(A...)>( std::make_shared<func::member_target_t<R, C0, C1, A...>>(base, mfunc),
                                  sizeof(func::member_target_t<R, C0, C1, A...>) );
    }

    /**
     * Bind given class instance and non-void member function to
     * an anonymous function using func_member_targer_t.
     *
     * @tparam R function return type
     * @tparam C class type holding the member-function and of this base pointer
     * @tparam A function arguments
     * @param base class instance `this` pointer
     * @param mfunc member-function with `R` return value and `A...` arguments.
     * @return anonymous function
     * @see @ref function_ctor_member "function constructor for member function"
     * @see @ref function_overview "function Overview"
     * @see @ref function_usage "function Usage"
     */
    template<typename R, typename C, typename... A>
    inline jau::function<R(A...)>
    bind_member(C *base, R(C::*mfunc)(A...)) noexcept {
        return function<R(A...)>( std::make_shared<func::member_target_t<R, C, C, A...>>(base, mfunc),
                                  sizeof(func::member_target_t<R, C, C, A...>) );
    }

    /**
     * Bind given class instance and void member function to
     * an anonymous function using func_member_targer_t.
     *
     * @tparam C0 class type holding the member-function
     * @tparam C1 class derived from C0 or C0 of this base-pointer used to invoke the member-function
     * @tparam A function arguments
     * @param base class instance `this` pointer
     * @param mfunc member-function with `A...` arguments.
     * @return anonymous function
     * @see @ref function_ctor_member "function constructor for member function"
     * @see @ref function_overview "function Overview"
     * @see @ref function_usage "function Usage"
     */
    template<typename C0, typename C1, typename... A>
    inline jau::function<void(A...)>
    bind_member(C1 *base, void(C0::*mfunc)(A...)) noexcept {
        return function<void(A...)>( std::make_shared<func::member_target_t<void, C0, C1, A...>>(base, mfunc),
                                     sizeof(func::member_target_t<void, C0, C1, A...>) );
    }

    /**
     * Bind given class instance and void member function to
     * an anonymous function using func_member_targer_t.
     *
     * @tparam C class type holding the member-function and of this base pointer
     * @tparam A function arguments
     * @param base class instance `this` pointer
     * @param mfunc member-function with `A...` arguments.
     * @return anonymous function
     * @see @ref function_ctor_member "function constructor for member function"
     * @see @ref function_overview "function Overview"
     * @see @ref function_usage "function Usage"
     */
    template<typename C, typename... A>
    inline jau::function<void(A...)>
    bind_member(C *base, void(C::*mfunc)(A...)) noexcept {
        return function<void(A...)>( std::make_shared<func::member_target_t<void, C, C, A...>>(base, mfunc),
                                     sizeof(func::member_target_t<void, C, C, A...>) );
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
     * @see @ref function_overview "function Overview"
     * @see @ref function_usage "function Usage"
     */
    template<typename R, typename... A>
    inline jau::function<R(A...)>
    bind_free(R(*func)(A...)) noexcept {
        return function<R(A...)>( std::make_shared<func::free_target_t<R, A...>>(func),
                                  sizeof(func::free_target_t<R, A...>) );
    }

    /**
     * Bind given void free-function to
     * an anonymous function using func::free_target_t.
     *
     * @tparam A function arguments
     * @param func free-function with `A...` arguments.
     * @return anonymous function
     * @see @ref function_ctor_free "function constructor for free function"
     * @see @ref function_overview "function Overview"
     * @see @ref function_usage "function Usage"
     */
    template<typename... A>
    inline jau::function<void(A...)>
    bind_free(void(*func)(A...)) noexcept {
        return function<void(A...)>( std::make_shared<func::free_target_t<void, A...>>(func),
                                     sizeof(func::free_target_t<void, A...>) );
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
     * @see @ref function_overview "function Overview"
     * @see @ref function_usage "function Usage"
     */
    template<typename R, typename L, typename... A>
    inline jau::function<R(A...)>
    bind_lambda(L func) noexcept {
        return function<R(A...)>( std::make_shared<func::lambda_target_t<R, L, A...>>(func),
                                  sizeof(func::lambda_target_t<R, L, A...>) );
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
     * @see @ref function_overview "function Overview"
     * @see @ref function_usage "function Usage"
     */
    template<typename L, typename... A>
    inline jau::function<void(A...)>
    bind_lambda(L func) noexcept {
        return function<void(A...)>( std::make_shared<func::lambda_target_t<void, L, A...>>(func),
                                     sizeof(func::lambda_target_t<void, L, A...>) );
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
     * @return anonymous function
     * @see @ref function_ctor_capval_copy "function constructor for copying capturing value"
     * @see @ref function_overview "function Overview"
     * @see @ref function_usage "function Usage"
     */
    template<typename R, typename I, typename... A>
    inline jau::function<R(A...)>
    bind_capval(const I& data, R(*func)(I&, A...)) noexcept {
        return function<R(A...)>( std::make_shared<func::capval_target_t<R, I, A...>>(data, func),
                                  sizeof(func::capval_target_t<R, I, A...>) );
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
     * @return anonymous function
     * @see @ref function_ctor_capval_copy "function constructor for copying capturing value"
     * @see @ref function_overview "function Overview"
     * @see @ref function_usage "function Usage"
     */
    template<typename I, typename... A>
    inline jau::function<void(A...)>
    bind_capval(const I& data, void(*func)(I&, A...)) noexcept {
        return function<void(A...)>( std::make_shared<func::capval_target_t<void, I, A...>>(data, func),
                                     sizeof(func::capval_target_t<void, I, A...>) );
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
     * @return anonymous function
     * @see @ref function_ctor_capval_move "function constructor for moving capturing value"
     * @see @ref function_overview "function Overview"
     * @see @ref function_usage "function Usage"
     */
    template<typename R, typename I, typename... A>
    inline jau::function<R(A...)>
    bind_capval(I&& data, R(*func)(I&, A...)) noexcept {
        return function<R(A...)>( std::make_shared<func::capval_target_t<R, I, A...>>(std::move(data), func),
                                  sizeof(func::capval_target_t<R, I, A...>) );
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
     * @return anonymous function
     * @see @ref function_ctor_capval_move "function constructor for moving capturing value"
     * @see @ref function_overview "function Overview"
     * @see @ref function_usage "function Usage"
     */
    template<typename I, typename... A>
    inline jau::function<void(A...)>
    bind_capval(I&& data, void(*func)(I&, A...)) noexcept {
        return function<void(A...)>( std::make_shared<func::capval_target_t<void, I, A...>>(std::move(data), func),
                                     sizeof(func::capval_target_t<void, I, A...>) );
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
     * @return anonymous function
     * @see @ref function_ctor_capref "function constructor for capturing reference"
     * @see @ref function_overview "function Overview"
     * @see @ref function_usage "function Usage"
     */
    template<typename R, typename I, typename... A>
    inline jau::function<R(A...)>
    bind_capref(I* data_ptr, R(*func)(I*, A...)) noexcept {
        return function<R(A...)>( std::make_shared<func::capref_target_t<R, I, A...>>(data_ptr, func),
                                  sizeof(func::capref_target_t<R, I, A...>) );
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
     * @return anonymous function
     * @see @ref function_ctor_capref "function constructor for capturing reference"
     * @see @ref function_overview "function Overview"
     * @see @ref function_usage "function Usage"
     */
    template<typename I, typename... A>
    inline jau::function<void(A...)>
    bind_capref(I* data_ptr, void(*func)(I*, A...)) noexcept {
        return function<void(A...)>( std::make_shared<func::capref_target_t<void, I, A...>>(data_ptr, func),
                                     sizeof(func::capref_target_t<void, I, A...>) );
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
     * @see @ref function_overview "function Overview"
     * @see @ref function_usage "function Usage"
     */
    template<typename R, typename... A>
    inline jau::function<R(A...)>
    bind_std(uint64_t id, std::function<R(A...)> func) noexcept {
        return function<R(A...)>( std::make_shared<func::std_target_t<R, A...>>(id, func),
                                  sizeof(func::std_target_t<R, A...>) );
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
     * @see @ref function_overview "function Overview"
     * @see @ref function_usage "function Usage"
     */
    template<typename... A>
    inline jau::function<void(A...)>
    bind_std(uint64_t id, std::function<void(A...)> func) noexcept {
        return function<void(A...)>( std::make_shared<func::std_target_t<void, A...>>(id, func),
                                     sizeof(func::std_target_t<void, A...>) );
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
