/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020 Gothel Software e.K.
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
#include <vector>
#include <functional>

#include <jau/basic_types.hpp>

namespace jau {

    /** @defgroup FunctionWrap Function Wrapper
     *  Supporting general-purpose polymorphic function wrapper via jau::function<R(A...)>.
     *
     * @anchor function_overview
     * ### Function Overview
     * Similar to std::function, [jau::function](@ref function_def) stores any callable target function
     * described by its return type `R` and arguments `A...` from any source,
     * e.g. free functions, member functions, etc.
     *
     * [jau::function](@ref function_def) supports equality operations for all func::target_t source types, allowing to manage container of [jau::function](@ref function_def)s.
     * This property distinguishes itself from std::function.
     *
     * Provided bind methods, see below, produce specific [jau::function](@ref function_def) instances,
     * where std::bind() is unspecific about the returned function type.
     *
     * Instances of [jau::function](@ref function_def) can store, copy, and invoke any of its callable targets
     * - free functions
     * - member functions
     * - lambda alike functions using a captured data type by value or reference
     * - std::function
     *
     * Instances of [jau::function](@ref function_def) are usually instantiated by its bind expressions
     * - bind_free()
     * - bind_member()
     * - bind_capref()
     * - bind_capval()
     * - bind_std()
     *
     * If a [jau::function](@ref function_def) contains no target, see jau::function<R(A...)>::is_null(), it is empty.
     * Invoking the target of an empty [jau::function](@ref function_def) has not operation nor side effects.
     *
     * [jau::function](@ref function_def) satisfies the requirements of CopyConstructible and CopyAssignable.
     *
     * #### C++11 Capturing Lambda Restrictions
     * A capturing lambda in C++11 produces decoration code accessing the captured elements,<br />
     * i.e. an anonymous helper class.<br />
     * Due to this fact, the return type is an undefined lambda specific<br/>
     * and hence we can't use it to feed the func::target_t into function requiring a well specified type.
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
     * A workaround is to use the provided func::capref_target_t and bind_capref().
     *
     * @anchor function_solutions
     * #### Function Solution
     * Due to the goals and limitations described above,
     * we need to store the reference of class's instance,<br />
     * which holds the member-function, in the func::target_t, see func::member_target_t.<br />
     * The latter can then be used by the function instance anonymously,
     * only defined by the function return type `R` and arguments `A...`.<br />
     * func::member_target_t then invokes the member-function
     * by using the class's reference as its `this` pointer.<br />
     * func::member_target_t checks for equality simply by comparing the references.<br />
     * Use bind_member() to instantiate the appropriate [jau::function](@ref function_def) for member functions.
     *
     * This methodology is also usable to handle lambdas with manual capture,
     * since the user is able to explicitly group all captured variables<br />
     * into an ad-hoc data struct and can pass its reference to  e.g. func::capref_target_t similar to a member function.<br />
     * func::capref_target_t checks for equality simply by comparing the references,
     * optionally this can include the reference to the captured data.<br />
     * Use bind_capref() to instantiate the appropriate [jau::function](@ref function_def) for lambda functions with manual capture.
     *
     * We also support std::function to be bound to a [jau::function](@ref function_def)
     * using func::std_target_t by passing a unique identifier, overcoming std::function lack of equality operator. <br />
     * Use bind_std() to instantiate the appropriate [jau::function](@ref function_def) for std::function.
     *
     * Last but not least, naturally we also allow free-functions to be bound to a function
     * using func::free_target_t.
     * Use bind_free() to instantiate the appropriate [jau::function](@ref function_def) for free-functions.
     *
     * All resulting function bindings support the equality operator
     * and hence can be identified by a toolkit, e.g. for their removal from a list.
     *
     * @anchor function_usage
     * #### Function Usage
     * The following bind methods are available to produce an anonymous [jau::function](@ref function_def)
     * only being defined by the function return type `R` and arguments `A...`,<br />
     * while being given a func::target_t instance resolving the binding to the target function of any kind.
     *
     * Let's assume we like to bind to the following
     * function prototype `bool func(int)`, which results to `jau::function<bool(int)>`:
     *
     * - Class member functions via jau::bind_member()
     *   - `jau::function<R(A...)> jau::bind_member(C *base, R(C::*mfunc)(A...))`
     *   ```
     *   struct MyClass {
     *      bool m_func(int v) { return 0 == v; }
     *   };
     *   MyClass i1;
     *   jau::function<bool(int)> func = jau::bind_member(&i1, &MyClass::m_func);
     *   ```
     *
     * - Free functions via jau::bind_free()
     *   - `jau::function<R(A...)> jau::bind_free(R(*func)(A...))`
     *   ```
     *   struct MyClass {
     *      static bool func(int v) { return 0 == v; }
     *   };
     *   jau::function<bool(int)> func0 = jau::bind_free(&MyClass:func);
     *
     *   bool my_func(int v) { return 0 == v; }
     *   jau::function<bool(int)> func1 = jau::bind_free(my_func);
     *   ```
     *
     * - Capture by reference to value via jau::bind_capref()
     *   - `jau::function<R(A...)> jau::bind_capref(I* data_ptr, R(*func)(I*, A...), bool dataIsIdentity)`
     *   ```
     *   struct big_data {
     *       int sum;
     *   };
     *   big_data data { 0 };
     *
     *   // bool my_func(int v)
     *   jau::function<bool(int)> func = jau::bind_capref(&data,
     *       ( bool(*)(big_data*, int) ) // help template type deduction of function-ptr
     *           ( [](big_data* data, int v) -> bool {
     *                 stats_ptr->sum += v;
     *                 return 0 == v;
     *             } ) );
     *   ```
     *
     * - Capture by copy of value via jau::bind_capval()
     *   - `jau::function<R(A...)> jau::bind_capval(const I& data, R(*func)(I&, A...), bool dataIsIdentity)`
     *   - `jau::function<R(A...)> jau::bind_capval(I&& data, R(*func)(I&, A...), bool dataIsIdentity)` <br />
     *   See example of *Capture by reference to value* above.
     *
     * - std::function function via jau::bind_std()
     *   - `function<R(A...)> jau::bind_std(uint64_t id, std::function<R(A...)> func)`
     *   ```
     *   // bool my_func(int v)
     *   std::function<bool(int)> func_stdlambda = [](int i)->bool {
     *       return 0 == i;
     *   };
     *   jau::function<bool(int)> func = jau::bind_std(100, func_stdlambda);
     *   ```
     *  @{
     */

    namespace func {

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
            /** Denotes a func::capval_target_t */
            capval = 3,
            /** Denotes a func::capref_target_t */
            capref = 4,
            /** Denotes a func::std_target_t */
            std = 5
        };
        constexpr int number(const target_type rhs) noexcept {
            return static_cast<int>(rhs);
        }

        /**
         * func::target_t pure-virtual interface for [jau::function](@ref function_def).
         *
         * @tparam R function return type
         * @tparam A function arguments
         * @see @ref function_overview "Function Overview" etc.
         */
        template<typename R, typename... A>
        class target_t {
            protected:
                target_t() noexcept {}

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

                virtual R invoke(A... args) = 0;

                virtual bool operator==(const target_t<R, A...>& rhs) const noexcept = 0;

                virtual bool operator!=(const target_t<R, A...>& rhs) const noexcept = 0;

                virtual std::string toString() const = 0;
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
        class null_target_t : public target_t<R, A...> {
            public:
                null_target_t() noexcept { }

                target_type type() const noexcept override { return target_type::null; }
                bool is_null() const noexcept override { return true; }

                target_t<R, A...> * clone() const noexcept override { return new null_target_t(); }

                R invoke(A...) override { return R(); }

                bool operator==(const target_t<R, A...>& rhs) const noexcept override
                {
                    return type() == rhs.type();
                }

                bool operator!=(const target_t<R, A...>& rhs) const noexcept override
                {
                    return !( *this == rhs );
                }

                std::string toString() const override {
                    return "null()";
                }
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
        class member_target_t : public target_t<R, A...> {
            private:
                C* base;
                R(C::*member)(A...);

            public:
                member_target_t(C *_base, R(C::*_member)(A...)) noexcept
                : base(_base), member(_member) {
                }

                target_type type() const noexcept override { return target_type::member; }
                bool is_null() const noexcept override { return false; }

                target_t<R, A...> * clone() const noexcept override { return new member_target_t(*this); }

                R invoke(A... args) override { return (base->*member)(args...); }

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

                bool operator!=(const target_t<R, A...>& rhs) const noexcept override
                {
                    return !( *this == rhs );
                }

                std::string toString() const override {
                    // hack to convert member pointer to void *: '*((void**)&member)'
                    return "member("+to_hexstring((uint64_t)base)+"->"+to_hexstring( *((void**)&member) ) + ")";
                }
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
        class free_target_t : public target_t<R, A...> {
            private:
                R(*function)(A...);

            public:
                free_target_t(R(*_function)(A...)) noexcept
                : function(_function) {
                }

                target_type type() const noexcept override { return target_type::free; }
                bool is_null() const noexcept override { return false; }

                target_t<R, A...> * clone() const noexcept override { return new free_target_t(*this); }

                R invoke(A... args) override { return (*function)(args...); }

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

                bool operator!=(const target_t<R, A...>& rhs) const noexcept override
                {
                    return !( *this == rhs );
                }

                std::string toString() const override {
                    // hack to convert function pointer to void *: '*((void**)&function)'
                    return "free("+to_hexstring( *((void**)&function) ) + ")";
                }
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
        class capval_target_t : public target_t<R, A...> {
            private:
                I data;
                R(*function)(I&, A...);
                bool dataIsIdentity;

            public:
                /** Utilizes copy-ctor from 'const I& _data' */
                capval_target_t(const I& _data, R(*_function)(I&, A...), bool dataIsIdentity_) noexcept
                : data(_data), function(_function), dataIsIdentity(dataIsIdentity_) {
                }

                /** Utilizes move-ctor from moved 'I&& _data' */
                capval_target_t(I&& _data, R(*_function)(I&, A...), bool dataIsIdentity_) noexcept
                : data(std::move(_data)), function(_function), dataIsIdentity(dataIsIdentity_) {
                }

                target_type type() const noexcept override { return target_type::capval; }
                bool is_null() const noexcept override { return false; }

                target_t<R, A...> * clone() const noexcept override { return new capval_target_t(*this); }

                R invoke(A... args) override { return (*function)(data, args...); }

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

                bool operator!=(const target_t<R, A...>& rhs) const noexcept override
                {
                    return !( *this == rhs );
                }

                std::string toString() const override {
                    // hack to convert function pointer to void *: '*((void**)&function)'
                    return "capval("+to_hexstring( *((void**)&function) ) + ")";
                }
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
        class capref_target_t : public target_t<R, A...> {
            private:
                I* data_ptr;
                R(*function)(I*, A...);
                bool dataIsIdentity;

            public:
                capref_target_t(I* _data_ptr, R(*_function)(I*, A...), bool dataIsIdentity_) noexcept
                : data_ptr(_data_ptr), function(_function), dataIsIdentity(dataIsIdentity_) {
                }

                target_type type() const noexcept override { return target_type::capref; }
                bool is_null() const noexcept override { return false; }

                target_t<R, A...> * clone() const noexcept override { return new capref_target_t(*this); }

                R invoke(A... args) override { return (*function)(data_ptr, args...); }

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

                bool operator!=(const target_t<R, A...>& rhs) const noexcept override
                {
                    return !( *this == rhs );
                }

                std::string toString() const override {
                    // hack to convert function pointer to void *: '*((void**)&function)'
                    return "capref("+to_hexstring( *((void**)&function) ) + ")";
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
         * @see @ref function_overview "Function Overview" etc.
         */
        template<typename R, typename... A>
        class std_target_t : public target_t<R, A...> {
            private:
                uint64_t id;
                std::function<R(A...)> function;

            public:
                std_target_t(uint64_t _id, std::function<R(A...)> _function) noexcept
                : id(_id), function(_function) {
                }
                std_target_t(uint64_t _id) noexcept
                : id(_id), function() {
                }

                target_type type() const noexcept override { return target_type::std; }
                bool is_null() const noexcept override { return false; }

                target_t<R, A...> * clone() const noexcept override { return new std_target_t(*this); }

                R invoke(A... args) override { return function(args...); }

                bool operator==(const target_t<R, A...>& rhs) const noexcept override
                {
                    if( &rhs == this ) {
                        return true;
                    }
                    if( type() != rhs.type() ) {
                        return false;
                    }
                    const std_target_t<R, A...> * prhs = static_cast<const std_target_t<R, A...>*>(&rhs);
                    return id == prhs->id;
                }

                bool operator!=(const target_t<R, A...>& rhs) const noexcept override
                {
                    return !( *this == rhs );
                }

                std::string toString() const override {
                    return "std("+to_hexstring( id )+")";
                }
        };

    } /* namespace func */

    /**
     * Class template [jau::function](@ref function_def) is a general-purpose polymorphic function wrapper.
     *
     * See @ref function_overview "Function Overview".
     *
     * This is the dummy template variant, allowing the void- and non-void return type target specializations.
     *
     * @see @ref function_def "Non-void function template definition"
     * @see @ref function_def_void "Void function template definition"
     */
    template<typename Signature>
    class function;

    /**
     * @anchor function_def
     * Class template [jau::function](@ref function_def) is a general-purpose polymorphic function wrapper.
     *
     * See @ref function_overview "Function Overview".
     *
     * This is the template variant for the non-void return type target function.
     *
     * @tparam R function return type
     * @tparam A function arguments
     * @see @ref function_overview "Function Overview" etc.
     * @see @ref function_usage "Function Usage"
     * @see @ref function_def_void "Void function template definition"
     */
    template<typename R, typename... A>
    class function<R(A...)> {
        private:
            std::shared_ptr<func::target_t<R, A...>> target_func;

        public:
            /** The target function return type R */
            typedef R result_type;

            /**
             * Constructs an instance with a null function.
             */
            function() noexcept
            : target_func( new func::null_target_t<R, A...>() ) { }

            /**
             * Constructs an instance by wrapping the given naked InvocationFunc<R, A...> function pointer
             * in a shared_ptr and taking ownership.
             */
            function(func::target_t<R, A...> * _funcPtr) noexcept
            : target_func( _funcPtr ) { }

            /**
             * Constructs an instance using the shared InvocationFunc<R, A...> function.
             */
            explicit function(std::shared_ptr<func::target_t<R, A...>> _func) noexcept
            : target_func( _func ) { }

            function(const function &o) noexcept = default;
            function(function &&o) noexcept = default;
            function& operator=(const function &o) noexcept = default;
            function& operator=(function &&o) noexcept= default;

            bool operator==(const function<R(A...)>& rhs) const noexcept
            { return *target_func == *rhs.target_func; }

            bool operator!=(const function<R(A...)>& rhs) const noexcept
            { return *target_func != *rhs.target_func; }

            /** Return the jau::func::type of this instance */
            func::target_type type() const noexcept { return target_func->type(); }

            /** Returns true if this instance does not hold a callable function target, i.e. is of jau::func::target_type::null.  */
            bool is_null() const noexcept { return target_func->is_null(); }

            /** Returns true if this instance holds a callable function target, i.e. is not of jau::func::target_type::null.  */
            explicit operator bool() const noexcept { return !target_func->is_null(); }

            /** Returns the shared InvocationFunc<R, A...> target function */
            std::shared_ptr<func::target_t<R, A...>> target() noexcept { return target_func; }

            /** Returns a new instance of the held InvocationFunc<R, A...> target function. */
            func::target_t<R, A...> * clone() const noexcept { return target_func->clone(); }

            std::string toString() const {
                return "function["+target_func->toString()+"]";
            }

            R operator()(A... args) const { return target_func->invoke(args...); }
            R operator()(A... args) { return target_func->invoke(args...); }
    };

    /**
     * @anchor function_def_void
     * Class template [jau::function](@ref function_def) is a general-purpose polymorphic function wrapper.
     *
     * See @ref function_overview "Function Overview".
     *
     * This is the template variant for the void return type target function.
     *
     * @tparam A function arguments
     * @see @ref function_overview "Function Overview" etc.
     * @see @ref function_usage "Function Usage"
     * @see @ref function_def "Non-void function template definition"
     */
    template<typename... A>
    class function<void(A...)> {
        private:
            std::shared_ptr<func::target_t<void, A...>> target_func;

        public:
            /** The target function return type `void` */
            typedef void result_type;

            /**
             * Constructs an instance with a null function target.
             */
            function() noexcept
            : target_func( new func::null_target_t<void, A...>() ) { }

            /**
             * Constructs an instance with a null function target.
             */
            function(std::nullptr_t ) noexcept
            : target_func( new func::null_target_t<void, A...>() ) { }

            /**
             * Constructs an instance by wrapping the given naked InvocationFunc<R, A...> function pointer
             * in a shared_ptr and taking ownership.
             */
            function(func::target_t<void, A...> * _funcPtr) noexcept
            : target_func( _funcPtr ) { }

            /**
             * Constructs an instance using the shared InvocationFunc<R, A...> function.
             */
            explicit function(std::shared_ptr<func::target_t<void, A...>> _func) noexcept
            : target_func( _func ) { }

            function(const function &o) noexcept = default;
            function(function &&o) noexcept = default;
            function& operator=(const function &o) noexcept = default;
            function& operator=(function &&o) noexcept= default;

            bool operator==(const function<void(A...)>& rhs) const noexcept
            { return *target_func == *rhs.target_func; }

            bool operator!=(const function<void(A...)>& rhs) const noexcept
            { return *target_func != *rhs.target_func; }

            /** Return the jau::func::type of this instance */
            func::target_type type() const noexcept { return target_func->type(); }

            /** Returns true if this instance does not hold a callable function target, i.e. is of jau::func::target_type::null.  */
            bool is_null() const noexcept { return target_func->is_null(); }

            /** Returns true if this instance holds a callable function target, i.e. is not of jau::func::target_type::null.  */
            explicit operator bool() const noexcept { return !target_func->is_null(); }

            /** Returns the shared InvocationFunc<R, A...> target function */
            std::shared_ptr<func::target_t<void, A...>> target() noexcept { return target_func; }

            /** Returns a new instance of the held InvocationFunc<R, A...> target function. */
            func::target_t<void, A...> * clone() const noexcept { return target_func->clone(); }

            std::string toString() const {
                return "function["+target_func->toString()+"]";
            }

            void operator()(A... args) const { target_func->invoke(args...); }
            void operator()(A... args) { target_func->invoke(args...); }
    };

    /**
     * Bind given class instance and member function to
     * an anonymous function using MemberInvocationFunc.
     *
     * @tparam R function return type
     * @tparam C class type holding the member-function
     * @tparam A function arguments
     * @param base class instance `this` pointer
     * @param mfunc member-function with `R` return value and `A...` arguments.
     * @return anonymous function
     * @see @ref function_overview "function Overview" etc.
     * @see @ref function_usage "function Usage"
     */
    template<typename R, typename C, typename... A>
    inline jau::function<R(A...)>
    bind_member(C *base, R(C::*mfunc)(A...)) noexcept {
        return function<R(A...)>( new func::member_target_t<R, C, A...>(base, mfunc) );
    }

    template<typename C, typename... A>
    inline jau::function<void(A...)>
    bind_member(C *base, void(C::*mfunc)(A...)) noexcept {
        return function<void(A...)>( new func::member_target_t<void, C, A...>(base, mfunc) );
    }

    /**
     * Bind given free-function to
     * an anonymous function using FreeInvocationFunc.
     *
     * @tparam R function return type
     * @tparam A function arguments
     * @param func free-function with `R` return value and `A...` arguments.
     * @return anonymous function
     * @see @ref function_overview "function Overview" etc.
     * @see @ref function_usage "function Usage"
     */
    template<typename R, typename... A>
    inline jau::function<R(A...)>
    bind_free(R(*func)(A...)) noexcept {
        return function<R(A...)>( new func::free_target_t<R, A...>(func) );
    }

    template<typename... A>
    inline jau::function<void(A...)>
    bind_free(void(*func)(A...)) noexcept {
        return function<void(A...)>( new func::free_target_t<void, A...>(func) );
    }

    /**
     * Bind given data by copying the value and the given function to
     * an anonymous function using CaptureValueInvocationFunc.
     *
     * `const I& data` will be copied into CaptureValueInvocationFunc and hence captured by copy.
     *
     * The function call will have the reference of the copied data being passed for efficiency.
     *
     * @tparam R function return type
     * @tparam I typename holding the captured data used by the function
     * @tparam A function arguments
     * @param data data type instance holding the captured data
     * @param func function with `R` return value and `A...` arguments.
     * @param dataIsIdentity if true (default), equality requires equal data. Otherwise equality only compares the function pointer.
     * @return anonymous function
     * @see @ref function_overview "function Overview" etc.
     * @see @ref function_usage "function Usage"
     */
    template<typename R, typename I, typename... A>
    inline jau::function<R(A...)>
    bind_capval(const I& data, R(*func)(I&, A...), bool dataIsIdentity=true) noexcept {
        return function<R(A...)>( new func::capval_target_t<R, I, A...>(data, func, dataIsIdentity) );
    }

    template<typename I, typename... A>
    inline jau::function<void(A...)>
    bind_capval(const I& data, void(*func)(I&, A...), bool dataIsIdentity=true) noexcept {
        return function<void(A...)>( new func::capval_target_t<void, I, A...>(data, func, dataIsIdentity) );
    }

    /**
     * Bind given data by moving the given value and copying the given function to
     * an anonymous function using CaptureValueInvocationFunc.
     *
     * `I&& data` will be moved into CaptureValueInvocationFunc.
     *
     * The function call will have the reference of the moved data being passed for efficiency.
     *
     * @tparam R function return type
     * @tparam I typename holding the captured data used by the function
     * @tparam A function arguments
     * @param data data type instance holding the captured data
     * @param func function with `R` return value and `A...` arguments.
     * @param dataIsIdentity if true (default), equality requires equal data. Otherwise equality only compares the function pointer.
     * @return anonymous function
     * @see @ref function_overview "function Overview" etc.
     * @see @ref function_usage "function Usage"
     */
    template<typename R, typename I, typename... A>
    inline jau::function<R(A...)>
    bind_capval(I&& data, R(*func)(I&, A...), bool dataIsIdentity=true) noexcept {
        return function<R(A...)>( new func::capval_target_t<R, I, A...>(std::move(data), func, dataIsIdentity) );
    }

    template<typename I, typename... A>
    inline jau::function<void(A...)>
    bind_capval(I&& data, void(*func)(I&, A...), bool dataIsIdentity=true) noexcept {
        return function<void(A...)>( new func::capval_target_t<void, I, A...>(std::move(data), func, dataIsIdentity) );
    }

    /**
     * Bind given data by passing the given reference (pointer) to the value and function to
     * an anonymous function using CaptureRefInvocationFunc.
     *
     * The function call will have the reference (pointer) being passed for efficiency.
     *
     * @tparam R function return type
     * @tparam I typename holding the captured data used by the function
     * @tparam A function arguments
     * @param data_ptr data type reference to instance holding the captured data
     * @param func function with `R` return value and `A...` arguments.
     * @param dataIsIdentity if true (default), equality requires same data_ptr. Otherwise equality only compares the function pointer.
     * @return anonymous function
     * @see @ref function_overview "function Overview" etc.
     * @see @ref function_usage "function Usage"
     */
    template<typename R, typename I, typename... A>
    inline jau::function<R(A...)>
    bind_capref(I* data_ptr, R(*func)(I*, A...), bool dataIsIdentity=true) noexcept {
        return function<R(A...)>( new func::capref_target_t<R, I, A...>(data_ptr, func, dataIsIdentity) );
    }

    template<typename I, typename... A>
    inline jau::function<void(A...)>
    bind_capref(I* data_ptr, void(*func)(I*, A...), bool dataIsIdentity=true) noexcept {
        return function<void(A...)>( new func::capref_target_t<void, I, A...>(data_ptr, func, dataIsIdentity) );
    }

    /**
     * Bind given std::function to
     * an anonymous function using StdInvocationFunc.
     *
     * Notable, instance is holding the given unique uint64_t identifier
     * to allow implementing the equality operator, not supported by std::function.
     *
     * @tparam R function return type
     * @tparam A function arguments
     * @param func free-function with `R` return value and `A...` arguments.
     * @return anonymous function
     * @see @ref function_overview "function Overview" etc.
     * @see @ref function_usage "function Usage"
     */
    template<typename R, typename... A>
    inline jau::function<R(A...)>
    bind_std(uint64_t id, std::function<R(A...)> func) noexcept {
        return function<R(A...)>( new func::std_target_t<R, A...>(id, func) );
    }

    template<typename... A>
    inline jau::function<void(A...)>
    bind_std(uint64_t id, std::function<void(A...)> func) noexcept {
        return function<void(A...)>( new func::std_target_t<void, A...>(id, func) );
    }

    /**@}*/

} // namespace jau

/** \example test_functional01.cpp
 * This C++ unit test validates the jau::function<R(A...)> and all its jau::func::target_t specializations.
 */

#endif /* JAU_FUNCTIONAL_HPP_ */
