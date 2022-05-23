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

#ifndef JAU_FUNCTION_HPP_
#define JAU_FUNCTION_HPP_

#include <cstring>
#include <string>
#include <memory>
#include <cstdint>
#include <vector>
#include <functional>

#include <jau/basic_types.hpp>

namespace jau {

    /** @defgroup FunctionPtr Function Pointer
     *  Function pointer support via FunctionDef inclusive capturing lambdas.
     *
     * @anchor func_def_overview
     * ### FunctionDef Overview
     * #### FunctionDef Motivation
     * One goal of FunctionDef is to allow a class member-function
     * to be described by its return type `R` and arguments `A...` only.<br />
     * Hence to be agnostic to the method owning class type.
     *
     * A toolkit storing callback functions, shall not enforce any constrains
     * on the source of such a provided user function.<br />
     * It shall not require to be sourced as a free function
     * nor require the user to put a specific super interface
     * on the implementing class of the callback.<br />
     * Only the user shall have knowledge about the source of the function,
     * whether it be a static free function or a member function.
     *
     * A toolkit API shall only expose and use a callback function
     * by its return type `R` and arguments `A...` only.
     *
     * Further a toolkit needs to identify the stored callback functions,
     * e.g. to allow the user to remove one from its list of callbacks.<br/>
     * Therefore a FunctionDef instance must provide the equality operator to support its identity check.<br/>
     * This requirement is not fulfilled by `std::function`,
     * which lacks details about the member-function-call identity
     * and hence lacks the equality operator.
     *
     * Last but not least, `std::bind` doesn't provide specifics about the function return type
     * and hence is not suitable for our goals.
     *
     * #### C++11 Capturing Lambda Restrictions
     * A capturing lambda in C++11 produces decoration code accessing the captured elements,<br />
     * i.e. an anonymous helper class.<br />
     * Due to this fact, the return type is an undefined lambda specific<br/>
     * and hence we can't use it to feed the InvocationFunc into FunctionDef requiring a well specified type.
     *
     * <pre>
        template<typename R, typename C, typename... A>
        inline FunctionDef<R, A...>
        bindClassFunction(C *base, R(C::*mfunc)(A...)) {
            return ClassFunction<R, A...>(
                    (void*)base,
                    (void*)(*((void**)&mfunc)),
                    [&](A... args)->R{ (base->*mfunc)(args...); });
                     ^
                     | Capturing lambda function-pointer are undefined!
        }
        </pre>
     *
     * @anchor func_def_solutions
     * #### FunctionDef Solution
     * Due to the goals and limitations described above,
     * we need to store the reference of class's instance,<br />
     * which holds the member-function, in the InvocationFunc, see MemberInvocationFunc.<br />
     * The latter can then be used by the FunctionDef instance anonymously,
     * only defined by the function return type `R` and arguments `A...`.<br />
     * MemberInvocationFunc then invokes the member-function
     * by using the class's reference as its `this` pointer.<br />
     * MemberInvocationFunc checks for equality simply by comparing the references.
     *
     * This methodology is also usable to handle capturing lambdas,
     * since the user is able to group all captured variables<br />
     * into an ad-hoc data struct and can pass its reference to  e.g. CaptureRefInvocationFunc similar to a member function.<br />
     * CaptureRefInvocationFunc checks for equality simply by comparing the references,
     * optionally this can include the reference to the captured data.
     *
     * We also allow std::function to be bound to a FunctionDef
     * using StdInvocationFunc by passing a unique identifier, overcoming std::function lack of equality operator.
     *
     * Last but not least, naturally we also allow free-functions to be bound to a FunctionDef
     * using FreeInvocationFunc.
     *
     * All resulting FunctionDef bindings support the equality operator
     * and hence can be identified by a toolkit, e.g. for their removal from a list.
     *
     * @anchor func_def_usage
     * #### FunctionDef Usage
     * The following bind methods are available to produce an anonymous FunctionDef
     * only being defined by the function return type `R` and arguments `A...`,<br />
     * while being given a InvocationFunc instance resolving a potentially missing link.
     *
     * As an example, let's assume we like to bind to the following
     * function prototype `bool func(int)`, which results to `FunctionDef<bool, int>`:
     *
     * - Class member functions via bindMemberFunc()
     *   - `FunctionDef<R, A...> bindMemberFunc(C *base, R(C::*mfunc)(A...))`
     *   ```
     *   struct MyClass {
     *      bool m_func(int v) { return 0 == v; }
     *   };
     *   MyClass i1;
     *   FunctionDef<bool, int> func = bindMemberFunc(&i1, &MyClass::m_func);
     *   ```
     *
     * - Free functions via bindFreeFunc()
     *   - `FunctionDef<R, A...> bindFreeFunc(R(*func)(A...))`
     *   ```
     *   struct MyClass {
     *      static bool func(int v) { return 0 == v; }
     *   };
     *   FunctionDef<bool, int> func0 = bindFreeFunc(&MyClass:func);
     *
     *   bool my_func(int v) { return 0 == v; }
     *   FunctionDef<bool, int> func1 = bindFreeFunc(my_func);
     *   ```
     *
     * - Capture by reference to value via bindCaptureRefFunc()
     *   - `FunctionDef<R, A...> bindCaptureRefFunc(I* data_ptr, R(*func)(I*, A...), bool dataIsIdentity)`
     *   ```
     *   struct big_data {
     *       int sum;
     *   };
     *   big_data data { 0 };
     *
     *   // bool my_func(int v)
     *   FunctionDef<bool, int> func = bindCaptureRefFunc(&data,
     *       ( bool(*)(big_data*, int) ) // help template type deduction of function-ptr
     *           ( [](big_data* data, int v) -> bool {
     *                 stats_ptr->sum += v;
     *                 return 0 == v;
     *             } ) );
     *   ```
     *
     * - Capture by copy of value via bindCaptureValueFunc()
     *   - `FunctionDef<R, A...> bindCaptureValueFunc(const I& data, R(*func)(I&, A...), bool dataIsIdentity)`
     *   - `FunctionDef<R, A...> bindCaptureValueFunc(I&& data, R(*func)(I&, A...), bool dataIsIdentity)` <br />
     *   See example of *Capture by reference to value* above.
     *
     * - std::function function via bindStdFunc()
     *   - `FunctionDef<R, A...> bindStdFunc(uint64_t id, std::function<R(A...)> func)`
     *   ```
     *   // bool my_func(int v)
     *   std::function<bool(int i)> func_stdlambda = [](int i)->bool {
     *       return 0 == i;
     *   };
     *   FunctionDef<bool, int> func = bindStdFunc(100, func_stdlambda);
     *   ```
     *  @{
     */

    /**
     * Function type identifier for InvocationFunc specializations
     * used by FunctionDef.
     *
     * @see @ref func_def_overview "FunctionDef Overview" etc.
     */
    enum class FunctionType : int {
        /** Denotes a NullInvocationFunc */
        Null = 0,
        /** Denotes a MemberInvocationFunc */
        Member = 1,
        /** Denotes a FreeInvocationFunc */
        Free = 2,
        /** Denotes a CaptureValueInvocationFunc */
        CaptureValue = 3,
        /** Denotes a CaptureRefInvocationFunc */
        CaptureRef = 4,
        /** Denotes a StdInvocationFunc */
        Std = 5
    };
    constexpr int number(const FunctionType rhs) noexcept {
        return static_cast<int>(rhs);
    }

    /**
     * InvocationFunc pure-virtual interface for FunctionDef.
     *
     * @tparam R function return type
     * @tparam A function arguments
     * @see @ref func_def_overview "FunctionDef Overview" etc.
     */
    template<typename R, typename... A>
    class InvocationFunc {
        protected:
            InvocationFunc() noexcept {}

        public:
            virtual ~InvocationFunc() noexcept {}

            InvocationFunc(const InvocationFunc &o) noexcept = default;
            InvocationFunc(InvocationFunc &&o) noexcept = default;
            InvocationFunc& operator=(const InvocationFunc &o) noexcept = default;
            InvocationFunc& operator=(InvocationFunc &&o) noexcept = default;

            /** Return the FunctionType of this invocation function wrapper */
            virtual FunctionType getType() const noexcept = 0;

            /** Returns if this this invocation function wrapper's is of FunctionType::Null  */
            virtual bool isNullType() const noexcept = 0;

            virtual InvocationFunc<R, A...> * clone() const noexcept = 0;

            virtual R invoke(A... args) = 0;

            virtual bool operator==(const InvocationFunc<R, A...>& rhs) const noexcept = 0;

            virtual bool operator!=(const InvocationFunc<R, A...>& rhs) const noexcept = 0;

            virtual std::string toString() const = 0;
    };

    /**
     * InvocationFunc implementation for no function.
     * identifiable as FunctionType::Null via FunctionDef::getType().
     *
     * This special type is used for an empty FunctionDef instance w/o holding a function,
     * e.g. when created with the default constructor.
     *
     * @tparam R function return type
     * @tparam A function arguments
     * @see @ref func_def_overview "FunctionDef Overview" etc.
     */
    template<typename R, typename... A>
    class NullInvocationFunc : public InvocationFunc<R, A...> {
        public:
            NullInvocationFunc() noexcept { }

            FunctionType getType() const noexcept override { return FunctionType::Null; }
            bool isNullType() const noexcept override { return true; }

            InvocationFunc<R, A...> * clone() const noexcept override { return new NullInvocationFunc(); }

            R invoke(A...) override { return R(); }

            bool operator==(const InvocationFunc<R, A...>& rhs) const noexcept override
            {
                return getType() == rhs.getType();
            }

            bool operator!=(const InvocationFunc<R, A...>& rhs) const noexcept override
            {
                return !( *this == rhs );
            }

            std::string toString() const override {
                return "NullInvocation";
            }
    };

    /**
     * InvocationFunc implementation for class member functions,
     * identifiable as FunctionType::Member via FunctionDef::getType().
     *
     * @tparam R function return type
     * @tparam C class type holding the member-function
     * @tparam A function arguments
     * @see @ref func_def_overview "FunctionDef Overview" etc.
     */
    template<typename R, typename C, typename... A>
    class MemberInvocationFunc : public InvocationFunc<R, A...> {
        private:
            C* base;
            R(C::*member)(A...);

        public:
            MemberInvocationFunc(C *_base, R(C::*_member)(A...)) noexcept
            : base(_base), member(_member) {
            }

            FunctionType getType() const noexcept override { return FunctionType::Member; }
            bool isNullType() const noexcept override { return false; }

            InvocationFunc<R, A...> * clone() const noexcept override { return new MemberInvocationFunc(*this); }

            R invoke(A... args) override { return (base->*member)(args...); }

            bool operator==(const InvocationFunc<R, A...>& rhs) const noexcept override
            {
                if( &rhs == this ) {
                    return true;
                }
                if( getType() != rhs.getType() ) {
                    return false;
                }
                const MemberInvocationFunc<R, C, A...> * prhs = static_cast<const MemberInvocationFunc<R, C, A...>*>(&rhs);
                return base == prhs->base && member == prhs->member;
            }

            bool operator!=(const InvocationFunc<R, A...>& rhs) const noexcept override
            {
                return !( *this == rhs );
            }

            std::string toString() const override {
                // hack to convert member pointer to void *: '*((void**)&member)'
                return "MemberInvocation "+to_hexstring((uint64_t)base)+"->"+to_hexstring( *((void**)&member) );
            }
    };

    /**
     * InvocationFunc implementation for free functions,
     * identifiable as FunctionType::Free via FunctionDef::getType().
     *
     * @tparam R function return type
     * @tparam A function arguments
     * @see @ref func_def_overview "FunctionDef Overview" etc.
     */
    template<typename R, typename... A>
    class FreeInvocationFunc : public InvocationFunc<R, A...> {
        private:
            R(*function)(A...);

        public:
            FreeInvocationFunc(R(*_function)(A...)) noexcept
            : function(_function) {
            }

            FunctionType getType() const noexcept override { return FunctionType::Free; }
            bool isNullType() const noexcept override { return false; }

            InvocationFunc<R, A...> * clone() const noexcept override { return new FreeInvocationFunc(*this); }

            R invoke(A... args) override { return (*function)(args...); }

            bool operator==(const InvocationFunc<R, A...>& rhs) const noexcept override
            {
                if( &rhs == this ) {
                    return true;
                }
                if( getType() != rhs.getType() ) {
                    return false;
                }
                const FreeInvocationFunc<R, A...> * prhs = static_cast<const FreeInvocationFunc<R, A...>*>(&rhs);
                return function == prhs->function;
            }

            bool operator!=(const InvocationFunc<R, A...>& rhs) const noexcept override
            {
                return !( *this == rhs );
            }

            std::string toString() const override {
                // hack to convert function pointer to void *: '*((void**)&function)'
                return "PlainInvocation "+to_hexstring( *((void**)&function) );
            }
    };

    /**
     * InvocationFunc implementation for functions using a copy of a captured value,
     * identifiable as FunctionType::CaptureValue via FunctionDef::getType().
     *
     * @tparam R function return type
     * @tparam I typename holding the captured data used by the function
     * @tparam A function arguments
     * @see @ref func_def_overview "FunctionDef Overview" etc.
     */
    template<typename R, typename I, typename... A>
    class CaptureValueInvocationFunc : public InvocationFunc<R, A...> {
        private:
            I data;
            R(*function)(I&, A...);
            bool dataIsIdentity;

        public:
            /** Utilizes copy-ctor from 'const I& _data' */
            CaptureValueInvocationFunc(const I& _data, R(*_function)(I&, A...), bool dataIsIdentity_) noexcept
            : data(_data), function(_function), dataIsIdentity(dataIsIdentity_) {
            }

            /** Utilizes move-ctor from moved 'I&& _data' */
            CaptureValueInvocationFunc(I&& _data, R(*_function)(I&, A...), bool dataIsIdentity_) noexcept
            : data(std::move(_data)), function(_function), dataIsIdentity(dataIsIdentity_) {
            }

            FunctionType getType() const noexcept override { return FunctionType::CaptureValue; }
            bool isNullType() const noexcept override { return false; }

            InvocationFunc<R, A...> * clone() const noexcept override { return new CaptureValueInvocationFunc(*this); }

            R invoke(A... args) override { return (*function)(data, args...); }

            bool operator==(const InvocationFunc<R, A...>& rhs) const noexcept override
            {
                if( &rhs == this ) {
                    return true;
                }
                if( getType() != rhs.getType() ) {
                    return false;
                }
                const CaptureValueInvocationFunc<R, I, A...> * prhs = static_cast<const CaptureValueInvocationFunc<R, I, A...>*>(&rhs);
                return dataIsIdentity == prhs->dataIsIdentity && function == prhs->function && ( !dataIsIdentity || data == prhs->data );
            }

            bool operator!=(const InvocationFunc<R, A...>& rhs) const noexcept override
            {
                return !( *this == rhs );
            }

            std::string toString() const override {
                // hack to convert function pointer to void *: '*((void**)&function)'
                return "CaptureValueInvocation "+to_hexstring( *((void**)&function) );
            }
    };

    /**
     * InvocationFunc implementation for functions using a reference to a captured value,
     * identifiable as FunctionType::CaptureRef via FunctionDef::getType().
     *
     * @tparam R function return type
     * @tparam I typename holding the captured data used by the function
     * @tparam A function arguments
     * @see @ref func_def_overview "FunctionDef Overview" etc.
     */
    template<typename R, typename I, typename... A>
    class CaptureRefInvocationFunc : public InvocationFunc<R, A...> {
        private:
            I* data_ptr;
            R(*function)(I*, A...);
            bool dataIsIdentity;

        public:
            CaptureRefInvocationFunc(I* _data_ptr, R(*_function)(I*, A...), bool dataIsIdentity_) noexcept
            : data_ptr(_data_ptr), function(_function), dataIsIdentity(dataIsIdentity_) {
            }

            FunctionType getType() const noexcept override { return FunctionType::CaptureRef; }
            bool isNullType() const noexcept override { return false; }

            InvocationFunc<R, A...> * clone() const noexcept override { return new CaptureRefInvocationFunc(*this); }

            R invoke(A... args) override { return (*function)(data_ptr, args...); }

            bool operator==(const InvocationFunc<R, A...>& rhs) const noexcept override
            {
                if( &rhs == this ) {
                    return true;
                }
                if( getType() != rhs.getType() ) {
                    return false;
                }
                const CaptureRefInvocationFunc<R, I, A...> * prhs = static_cast<const CaptureRefInvocationFunc<R, I, A...>*>(&rhs);
                return dataIsIdentity == prhs->dataIsIdentity && function == prhs->function && ( !dataIsIdentity || data_ptr == prhs->data_ptr );
            }

            bool operator!=(const InvocationFunc<R, A...>& rhs) const noexcept override
            {
                return !( *this == rhs );
            }

            std::string toString() const override {
                // hack to convert function pointer to void *: '*((void**)&function)'
                return "CaptureRefInvocation "+to_hexstring( *((void**)&function) );
            }
    };

    /**
     * InvocationFunc implementation for std::function instances,
     * identifiable as FunctionType::Std via FunctionDef::getType().
     *
     * Notable, instance is holding a unique uint64_t identifier
     * to allow implementing the equality operator, not supported by std::function.
     *
     * @tparam R function return type
     * @tparam A function arguments
     * @see @ref func_def_overview "FunctionDef Overview" etc.
     */
    template<typename R, typename... A>
    class StdInvocationFunc : public InvocationFunc<R, A...> {
        private:
            uint64_t id;
            std::function<R(A...)> function;

        public:
            StdInvocationFunc(uint64_t _id, std::function<R(A...)> _function) noexcept
            : id(_id), function(_function) {
            }
            StdInvocationFunc(uint64_t _id) noexcept
            : id(_id), function() {
            }

            FunctionType getType() const noexcept override { return FunctionType::Std; }
            bool isNullType() const noexcept override { return false; }

            InvocationFunc<R, A...> * clone() const noexcept override { return new StdInvocationFunc(*this); }

            R invoke(A... args) override { return function(args...); }

            bool operator==(const InvocationFunc<R, A...>& rhs) const noexcept override
            {
                if( &rhs == this ) {
                    return true;
                }
                if( getType() != rhs.getType() ) {
                    return false;
                }
                const StdInvocationFunc<R, A...> * prhs = static_cast<const StdInvocationFunc<R, A...>*>(&rhs);
                return id == prhs->id;
            }

            bool operator!=(const InvocationFunc<R, A...>& rhs) const noexcept override
            {
                return !( *this == rhs );
            }

            std::string toString() const override {
                return "StdInvocation "+to_hexstring( id );
            }
    };

    /**
     * FunctionDef encapsulating an arbitrary InvocationFunc shared reference
     * to allow anonymous function invocation w/o knowledge about its origin,
     * i.e. free-function, member-function, capture-function, ..
     *
     * @tparam R function return type
     * @tparam A function arguments
     * @see @ref func_def_overview "FunctionDef Overview" etc.
     * @see @ref func_def_usage "FunctionDef Usage"
     */
    template<typename R, typename... A>
    class FunctionDef {
        private:
            std::shared_ptr<InvocationFunc<R, A...>> func;

        public:
            /**
             * Constructs an instance with a null function.
             */
            FunctionDef() noexcept
            : func( new NullInvocationFunc<R, A...>() ) { }

            /**
             * Constructs an instance by wrapping the given naked InvocationFunc<R, A...> function pointer
             * in a shared_ptr and taking ownership.
             */
            FunctionDef(InvocationFunc<R, A...> * _funcPtr) noexcept
            : func( _funcPtr ) { }

            /**
             * Constructs an instance using the shared InvocationFunc<R, A...> function.
             */
            explicit FunctionDef(std::shared_ptr<InvocationFunc<R, A...>> _func) noexcept
            : func( _func ) { }

            FunctionDef(const FunctionDef &o) noexcept = default;
            FunctionDef(FunctionDef &&o) noexcept = default;
            FunctionDef& operator=(const FunctionDef &o) noexcept = default;
            FunctionDef& operator=(FunctionDef &&o) noexcept= default;

            bool operator==(const FunctionDef<R, A...>& rhs) const noexcept
            { return *func == *rhs.func; }

            bool operator!=(const FunctionDef<R, A...>& rhs) const noexcept
            { return *func != *rhs.func; }

            /** Return the FunctionType of this instance */
            FunctionType getType() const noexcept { return func->getType(); }

            /** Returns if this this instance is of FunctionType::Null  */
            bool isNullType() const noexcept { return func->isNullType(); }

            /** Returns the shared InvocationFunc<R, A...> function */
            std::shared_ptr<InvocationFunc<R, A...>> getFunction() noexcept { return func; }

            /** Returns a new instance of the held InvocationFunc<R, A...> function. */
            InvocationFunc<R, A...> * cloneFunction() const noexcept { return func->clone(); }

            std::string toString() const {
                return "FunctionDef["+func->toString()+"]";
            }

            R invoke(A... args) const { return func->invoke(args...); }
            R invoke(A... args) { return func->invoke(args...); }

            R operator()(A... args) const { return func->invoke(args...); }
            R operator()(A... args) { return func->invoke(args...); }
    };

    /**
     * Bind given class instance and member function to
     * an anonymous FunctionDef using MemberInvocationFunc.
     *
     * @tparam R function return type
     * @tparam C class type holding the member-function
     * @tparam A function arguments
     * @param base class instance `this` pointer
     * @param mfunc member-function with `R` return value and `A...` arguments.
     * @return anonymous FunctionDef
     * @see @ref func_def_overview "FunctionDef Overview" etc.
     * @see @ref func_def_usage "FunctionDef Usage"
     */
    template<typename R, typename C, typename... A>
    inline jau::FunctionDef<R, A...>
    bindMemberFunc(C *base, R(C::*mfunc)(A...)) noexcept {
        return FunctionDef<R, A...>( new MemberInvocationFunc<R, C, A...>(base, mfunc) );
    }

    /**
     * Bind given free-function to
     * an anonymous FunctionDef using FreeInvocationFunc.
     *
     * @tparam R function return type
     * @tparam A function arguments
     * @param func free-function with `R` return value and `A...` arguments.
     * @return anonymous FunctionDef
     * @see @ref func_def_overview "FunctionDef Overview" etc.
     * @see @ref func_def_usage "FunctionDef Usage"
     */
    template<typename R, typename... A>
    inline jau::FunctionDef<R, A...>
    bindFreeFunc(R(*func)(A...)) noexcept {
        return FunctionDef<R, A...>( new FreeInvocationFunc<R, A...>(func) );
    }

    /**
     * Bind given data by copying the value and the given function to
     * an anonymous FunctionDef using CaptureValueInvocationFunc.
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
     * @return anonymous FunctionDef
     * @see @ref func_def_overview "FunctionDef Overview" etc.
     * @see @ref func_def_usage "FunctionDef Usage"
     */
    template<typename R, typename I, typename... A>
    inline jau::FunctionDef<R, A...>
    bindCaptureValueFunc(const I& data, R(*func)(I&, A...), bool dataIsIdentity=true) noexcept {
        return FunctionDef<R, A...>( new CaptureValueInvocationFunc<R, I, A...>(data, func, dataIsIdentity) );
    }

    /**
     * Bind given data by moving the given value and copying the given function to
     * an anonymous FunctionDef using CaptureValueInvocationFunc.
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
     * @return anonymous FunctionDef
     * @see @ref func_def_overview "FunctionDef Overview" etc.
     * @see @ref func_def_usage "FunctionDef Usage"
     */
    template<typename R, typename I, typename... A>
    inline jau::FunctionDef<R, A...>
    bindCaptureValueFunc(I&& data, R(*func)(I&, A...), bool dataIsIdentity=true) noexcept {
        return FunctionDef<R, A...>( new CaptureValueInvocationFunc<R, I, A...>(std::move(data), func, dataIsIdentity) );
    }

    /**
     * Bind given data by passing the given reference to the value and function to
     * an anonymous FunctionDef using CaptureRefInvocationFunc.
     *
     * @tparam R function return type
     * @tparam I typename holding the captured data used by the function
     * @tparam A function arguments
     * @param data_ptr data type reference to instance holding the captured data
     * @param func function with `R` return value and `A...` arguments.
     * @param dataIsIdentity if true (default), equality requires same data_ptr. Otherwise equality only compares the function pointer.
     * @return anonymous FunctionDef
     * @see @ref func_def_overview "FunctionDef Overview" etc.
     * @see @ref func_def_usage "FunctionDef Usage"
     */
    template<typename R, typename I, typename... A>
    inline jau::FunctionDef<R, A...>
    bindCaptureRefFunc(I* data_ptr, R(*func)(I*, A...), bool dataIsIdentity=true) noexcept {
        return FunctionDef<R, A...>( new CaptureRefInvocationFunc<R, I, A...>(data_ptr, func, dataIsIdentity) );
    }

    /**
     * Bind given std::function to
     * an anonymous FunctionDef using StdInvocationFunc.
     *
     * Notable, instance is holding the given unique uint64_t identifier
     * to allow implementing the equality operator, not supported by std::function.
     *
     * @tparam R function return type
     * @tparam A function arguments
     * @param func free-function with `R` return value and `A...` arguments.
     * @return anonymous FunctionDef
     * @see @ref func_def_overview "FunctionDef Overview" etc.
     * @see @ref func_def_usage "FunctionDef Usage"
     */
    template<typename R, typename... A>
    inline jau::FunctionDef<R, A...>
    bindStdFunc(uint64_t id, std::function<R(A...)> func) noexcept {
        return FunctionDef<R, A...>( new StdInvocationFunc<R, A...>(id, func) );
    }

    /**@}*/

} // namespace jau

/** \example test_functiondef01.cpp
 * This C++ unit test validates the jau::FunctionDef and all its jau::InvocationFunc specializations.
 */

#endif /* JAU_FUNCTION_HPP_ */
