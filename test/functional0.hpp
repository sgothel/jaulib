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
     * - free functions including non-capturing lambda
     *   - bind_free()
     *   - constructor [jau::function<R(A...)>::function(R(*func)(A...))](@ref function_ctor_free)
     * - member functions
     *   - bind_member()
     *   - constructor [`function(C *base, R(C::*mfunc)(A...))`](@ref function_ctor_member)
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
     * @anchor function_usage
     * #### Function Usage
     *
     * A detailed API usage is covered within [test_functional01.cpp](test_functional01_8cpp-example.html), see function `test00_usage()`.
     *
     * Let's assume we like to bind to the following
     * function prototype `bool func(int)`, which results to `jau::function<bool(int)>`:
     *
     * - Free function and non-capturing lambda via constructor and jau::bind_free()
     *   - Prologue
     *   ```
     *   typedef bool(*cfunc)(int); // help template type deduction
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
     *   jau::function<bool(int)> func0 = (cfunc) ( [](int v) -> bool {
     *          return 0 == v;
     *      } );
     *
     *   jau::function<bool(int)> func1 = my_func;
     *
     *   jau::function<bool(int)> func0 = &MyClass:func;
     *   ```
     *
     *   - Bind `jau::function<R(A...)> jau::bind_free(R(*func)(A...))`
     *   ```
     *   jau::function<bool(int)> func0 = jau::bind_free(&MyClass:func);
     *
     *   jau::function<bool(int)> func1 = jau::bind_free(my_func);
     *   ```
     *
     * - Class member functions via constructor and jau::bind_member()
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
     * - Capture by reference to value via constructor and jau::bind_capref()
     *   - Prologue
     *   ```
     *   struct big_data {
     *       int sum;
     *   };
     *   big_data data { 0 };
     *
     *   typedef bool(*cfunc)(big_data*, int); // help template type deduction
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
     * - Capture by copy of value via constructor and jau::bind_capval()
     *   - Constructor copy [function(const I& data, R(*func)(I&, A...), bool dataIsIdentity=true)](@ref function_ctor_capval_copy)
     *   - Constructor move [function(I&& data, R(*func)(I&, A...), bool dataIsIdentity=true)](@ref function_ctor_capval_move)
     *   - Bind copy `jau::function<R(A...)> jau::bind_capval(const I& data, R(*func)(I&, A...), bool dataIsIdentity)`
     *   - Bind move `jau::function<R(A...)> jau::bind_capval(I&& data, R(*func)(I&, A...), bool dataIsIdentity)` <br />
     *   See example of *Capture by reference to value* above.
     *
     * - std::function function via constructor and jau::bind_std()
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
            : target_func( std::make_shared<func::null_target_t<R, A...>>() ) { }

            /**
             * \brief Null function constructor
             *
             * @anchor function_ctor_nullptr
             * Constructs an instance with a null target function.
             * @see @ref function_overview "function Overview" etc.
             * @see @ref function_usage "function Usage"
             */
            function(std::nullptr_t ) noexcept
            : target_func( std::make_shared<func::null_target_t<R, A...>>() ) { }

            /**
             * \brief target_type constructor
             *
             * @anchor function_ctor_target_type
             * Constructs an instance with the given shared target function pointer.
             * @see @ref function_overview "function Overview" etc.
             * @see @ref function_usage "function Usage"
             */
            function(std::shared_ptr<target_type> _funcPtr) noexcept
            : target_func( _funcPtr ) { }

            /**
             * \brief Free function constructor
             *
             * @anchor function_ctor_free
             * Constructs an instance by taking a free target function, which may also be a non-capturing lambda.
             * @see @ref function_overview "function Overview" etc.
             * @see @ref function_usage "function Usage"
             */
            function(R(*func)(A...)) noexcept
            : target_func( std::make_shared<func::free_target_t<R, A...>>(func) ) { }

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
            : target_func( std::make_shared<func::member_target_t<R, C, A...>>(base, mfunc) ) { }

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
            : target_func( std::make_shared<func::capval_target_t<R, I, A...>>(data, func, dataIsIdentity) ) { }

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
            : target_func( std::make_shared<func::capval_target_t<R, I, A...>>(std::move(data), func, dataIsIdentity) ) { }

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
            : target_func( std::make_shared<func::capref_target_t<R, I, A...>>(data_ptr, func, dataIsIdentity) ) { }

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
            : target_func( std::make_shared<func::std_target_t<R, A...>>(id, func) ) { }

            function(const function &o) noexcept = default;
            function(function &&o) noexcept = default;
            function& operator=(const function &o) noexcept = default;
            function& operator=(function &&o) noexcept = default;

            bool operator==(const function<R(A...)>& rhs) const noexcept
            { return *target_func == *rhs.target_func; }

            bool operator!=(const function<R(A...)>& rhs) const noexcept
            { return *target_func != *rhs.target_func; }

            /** Return the jau::func::type of this instance */
            func::target_type type() const noexcept { return target_func->type(); }

            /** Returns true if this instance does not hold a callable target function, i.e. is of func::target_type::null.  */
            bool is_null() const noexcept { return target_func->is_null(); }

            /** Returns true if this instance holds a callable target function, i.e. is not of func::target_type::null.  */
            explicit operator bool() const noexcept { return !target_func->is_null(); }

            /** Returns the shared target function. */
            std::shared_ptr<target_type> target() noexcept { return target_func; }

            std::string toString() const {
                return "function["+target_func->toString()+"]";
            }

            inline R operator()(A... args) const { return target_func->invoke(args...); }
            inline R operator()(A... args) { return target_func->invoke(args...); }
    };

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
     * @see @ref function_overview "function Overview" etc.
     * @see @ref function_usage "function Usage"
     */
    template<typename R, typename C, typename... A>
    inline jau::function<R(A...)>
    bind_member(C *base, R(C::*mfunc)(A...)) noexcept {
        return function<R(A...)>( std::make_shared<func::member_target_t<R, C, A...>>(base, mfunc) );
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
     * @see @ref function_overview "function Overview" etc.
     * @see @ref function_usage "function Usage"
     */
    template<typename C, typename... A>
    inline jau::function<void(A...)>
    bind_member(C *base, void(C::*mfunc)(A...)) noexcept {
        return function<void(A...)>( std::make_shared<func::member_target_t<void, C, A...>>(base, mfunc) );
    }

    /**
     * Bind given non-void free-function to
     * an anonymous function using func::free_target_t.
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
        return function<R(A...)>( std::make_shared<func::free_target_t<R, A...>>(func) );
    }

    /**
     * Bind given void free-function to
     * an anonymous function using func::free_target_t.
     *
     * @tparam A function arguments
     * @param func free-function with `A...` arguments.
     * @return anonymous function
     * @see @ref function_overview "function Overview" etc.
     * @see @ref function_usage "function Usage"
     */
    template<typename... A>
    inline jau::function<void(A...)>
    bind_free(void(*func)(A...)) noexcept {
        return function<void(A...)>( std::make_shared<func::free_target_t<void, A...>>(func) );
    }

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
     * @see @ref function_overview "function Overview" etc.
     * @see @ref function_usage "function Usage"
     */
    template<typename R, typename I, typename... A>
    inline jau::function<R(A...)>
    bind_capval(const I& data, R(*func)(I&, A...), bool dataIsIdentity=true) noexcept {
        return function<R(A...)>( std::make_shared<func::capval_target_t<R, I, A...>>(data, func, dataIsIdentity) );
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
     * @see @ref function_overview "function Overview" etc.
     * @see @ref function_usage "function Usage"
     */
    template<typename I, typename... A>
    inline jau::function<void(A...)>
    bind_capval(const I& data, void(*func)(I&, A...), bool dataIsIdentity=true) noexcept {
        return function<void(A...)>( std::make_shared<func::capval_target_t<void, I, A...>>(data, func, dataIsIdentity) );
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
     * @see @ref function_overview "function Overview" etc.
     * @see @ref function_usage "function Usage"
     */
    template<typename R, typename I, typename... A>
    inline jau::function<R(A...)>
    bind_capval(I&& data, R(*func)(I&, A...), bool dataIsIdentity=true) noexcept {
        return function<R(A...)>( std::make_shared<func::capval_target_t<R, I, A...>>(std::move(data), func, dataIsIdentity) );
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
     * @see @ref function_overview "function Overview" etc.
     * @see @ref function_usage "function Usage"
     */
    template<typename I, typename... A>
    inline jau::function<void(A...)>
    bind_capval(I&& data, void(*func)(I&, A...), bool dataIsIdentity=true) noexcept {
        return function<void(A...)>( std::make_shared<func::capval_target_t<void, I, A...>>(std::move(data), func, dataIsIdentity) );
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
     * @see @ref function_overview "function Overview" etc.
     * @see @ref function_usage "function Usage"
     */
    template<typename R, typename I, typename... A>
    inline jau::function<R(A...)>
    bind_capref(I* data_ptr, R(*func)(I*, A...), bool dataIsIdentity=true) noexcept {
        return function<R(A...)>( std::make_shared<func::capref_target_t<R, I, A...>>(data_ptr, func, dataIsIdentity) );
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
     * @see @ref function_overview "function Overview" etc.
     * @see @ref function_usage "function Usage"
     */
    template<typename I, typename... A>
    inline jau::function<void(A...)>
    bind_capref(I* data_ptr, void(*func)(I*, A...), bool dataIsIdentity=true) noexcept {
        return function<void(A...)>( std::make_shared<func::capref_target_t<void, I, A...>>(data_ptr, func, dataIsIdentity) );
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
     * @see @ref function_overview "function Overview" etc.
     * @see @ref function_usage "function Usage"
     */
    template<typename R, typename... A>
    inline jau::function<R(A...)>
    bind_std(uint64_t id, std::function<R(A...)> func) noexcept {
        return function<R(A...)>( std::make_shared<func::std_target_t<R, A...>>(id, func) );
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
     * @see @ref function_overview "function Overview" etc.
     * @see @ref function_usage "function Usage"
     */
    template<typename... A>
    inline jau::function<void(A...)>
    bind_std(uint64_t id, std::function<void(A...)> func) noexcept {
        return function<void(A...)>( std::make_shared<func::std_target_t<void, A...>>(id, func) );
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
