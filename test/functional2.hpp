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
#include <jau/debug.hpp>

namespace jau {

    namespace func {

        /** \addtogroup FunctionWrap
         *
         *  @{
         */

        /**
         * func::target_type identifier for the target function [delegate_t<R, A...>](@ref delegate_class) object,
         * exposed by jau::function<R(A...)>::type().
         *
         * @see @ref function_overview "Function Overview"
         */
        enum class target_type : uint16_t {
            /** Denotes a func::null_target_t */
            null = 0,
            /** Denotes a func::member_target_t */
            member = 1,
            /** Denotes a func::free_target_t */
            free = 2,
            /** Denotes a func::lambda_target_t */
            lambda = 3,
            /** Denotes a func::ylambda_target_t */
            ylambda = 4,
            /** Denotes a func::capval_target_t */
            capval = 5,
            /** Denotes a func::capref_target_t */
            capref = 6,
            /** Denotes a func::std_target_t */
            std = 7
        };

        /**
         * Delegated target function object, providing a fast path target function invocation.
         *
         * @anchor delegate_class This static polymorphic target function delegate_t<R, A...> is contained by [function<R(A...)>](@ref function_def)
         * and completely holds up to 24 bytes sized `TriviallyCopyable` target function objects to avoiding cache misses.
         * - contained within [function<R(A...)>](@ref function_def) instance as a member
         *   - avoiding need for dynamic polymorphism, i.e. heap allocated specialization referenced by base type
         *   - using non-heap cache for up to 24 bytes sized `TriviallyCopyable` target function objects
         *     - enhancing performance on most target function types by avoiding cache misses
         * - not using virtual function table indirection
         * - utilize constexpr inline for function invocation (callbacks)
         *
         * @tparam R function return type
         * @tparam A function arguments
         * @see @ref function_overview "Function Overview"
         */
        template<typename R, typename... A>
        class delegate_t final { // 56 [ + vsize ]
            public:
                /**
                 * Utilize a reduced size type of int32_t.
                 * Negative sign indicates limited 24 bytes cache usage, otherwise heap is being used.
                 */
                typedef int32_t size_type;

            protected:
                typedef R(*invocation_t)(delegate_t* __restrict_cxx__ const data, A... args);
                typedef bool(*equal_op_t)(const delegate_t& data_lhs, const delegate_t& data_rhs) noexcept;

            private:
                struct non_trivial_t final { // 3 * 8 = 24
                    typedef void(*dtor_t)       (delegate_t*);
                    typedef void(*copy_ctor_t)  (delegate_t*, const delegate_t*);
                    typedef void(*move_ctor_t)  (delegate_t*,       delegate_t*);
                    dtor_t dtor;
                    copy_ctor_t copy_ctor;
                    move_ctor_t move_ctor;

                    constexpr non_trivial_t() noexcept
                    : dtor(nullptr), copy_ctor(nullptr), move_ctor(nullptr)
                    { }
                };

                struct heap_t final { // 16 [ + vsize ] [ + 24 ]
                    void* memory;
                    non_trivial_t* non_trivial;
                };

                /**
                 * Cases
                 * -  trivial +  sdata (fast path)
                 * -  trivial +  vdata
                 * - !trivial +  vdata
                 */
                union target_data_t { // 32
                    uint8_t cache[32]; // size <= 0, 32 bytes local high perf cached chunk
                    heap_t heap;       // size >  0, 32
                };

                target_data_t udata;

                /** Delegated specialization callback. (local) */
                invocation_t m_cb; // 8
                /** Delegated specialization equality operator. (local) */
                equal_op_t m_eqop; // 8

                size_type m_size;   // 4
                target_type m_type; // 4

                /**
                 * For `TriviallyCopyable` only, using sdata or vdata.
                 */
                delegate_t(target_type type_, size_type size_, invocation_t cb_, equal_op_t eqop_) noexcept
                : m_cb(cb_), m_eqop(eqop_), m_size( size_ ), m_type(type_)
                {
                    if( static_cast<size_type>( sizeof(udata.cache) ) >= m_size ) {
                        // using sdata
                        m_size *= -1;
                    } else {
                        // using vdata
                        udata.heap.memory = ::malloc(m_size);
                        udata.heap.non_trivial = nullptr;
                    }
                }

                /**
                 * Non `TriviallyCopyable` only, using vdata.
                 */
                delegate_t(target_type type_, size_type size_, const non_trivial_t& nt, invocation_t cb_, equal_op_t eqop_) noexcept
                : m_cb(cb_), m_eqop(eqop_), m_size( size_ ), m_type(type_)
                {
                    try {
                        udata.heap.non_trivial = new non_trivial_t(nt);
                    } catch (const std::bad_alloc &e) {
                        ABORT("Error: bad_alloc: non_trivial allocation failed");
                        return; // unreachable
                    }
                    udata.heap.memory = ::malloc(m_size);
                }

                void clear() noexcept {
                    if( m_size > 0 ) {
                        if( nullptr != udata.heap.non_trivial ) {
                            udata.heap.non_trivial->dtor(this);
                            delete udata.heap.non_trivial;
                            udata.heap.non_trivial = nullptr;
                        }
                        ::free(udata.heap.memory);
                        udata.heap.memory = nullptr;
                    }
                    m_size = 0;
                }

            public:
                // null type
                static delegate_t make(invocation_t cb_, equal_op_t eqop_) noexcept
                {
                    return delegate_t(target_type::null, 0, cb_, eqop_);
                }

                // `TriviallyCopyable` using cache or heap
                template<typename T, typename... P,
                         std::enable_if_t<std::is_trivially_copyable_v<T> &&
                                          sizeof(T) <= std::numeric_limits<size_type>::max(),
                                          bool> = true>
                static delegate_t make(target_type type_, invocation_t cb_, equal_op_t eqop_, P... params) noexcept
                {
                    delegate_t target(type_, static_cast<size_type>( sizeof(T) ), cb_, eqop_);
                    new( target.template data<T>() ) T(params...); // placement new
                    return target;
                }

                // Non `TriviallyCopyable` using heap
                template<typename T, typename... P,
                         std::enable_if_t<!std::is_trivially_copyable_v<T> &&
                                           sizeof(T) <= std::numeric_limits<size_type>::max() &&
                                           std::is_destructible_v<T> &&
                                           std::is_copy_constructible_v<T> &&
                                           std::is_move_constructible_v<T>,
                                           bool> = true>
                static delegate_t make(target_type type_, invocation_t cb_, equal_op_t eqop_, P... params) noexcept
                {
                    non_trivial_t nt;
                    nt.dtor = [](delegate_t* i) -> void {
                        T* t = i->template data<T>();
                        if( nullptr != t ) {
                            t->T::~T(); // placement new -> manual destruction!
                        }
                    };
                    nt.copy_ctor = [](delegate_t* i, const delegate_t* o) -> void {
                        new( i->template data<T>() ) T( *( o->template data<T>() ) ); // placement new copy-ctor
                    };
                    nt.move_ctor = [](delegate_t* i, delegate_t* o) -> void {
                        new( i->template data<T>() ) T( std::move( *( o->template data<T>() ) ) ); // placement new move-ctor
                    };
                    delegate_t target(type_, static_cast<size_type>( sizeof(T) ), nt, cb_, eqop_);
                    new( target.template data<T>() ) T(params...); // placement new
                    return target;
                }

                ~delegate_t() noexcept { clear(); }

                delegate_t(const delegate_t& o) noexcept
                : m_cb( o.m_cb ), m_eqop( o.m_eqop ),
                  m_size(o.m_size), m_type( o.m_type )
                {
                    if( m_size > 0 ) {
                        udata.heap.memory = ::malloc(m_size);
                        if( nullptr == udata.heap.memory ) {
                            ABORT("Error: bad_alloc: heap.memory allocation failed");
                            return; // unreachable
                        }
                        if( nullptr != o.udata.heap.non_trivial ) {
                            try {
                                udata.heap.non_trivial = new non_trivial_t( *o.udata.heap.non_trivial );
                            } catch (const std::bad_alloc &e) {
                                ABORT("Error: bad_alloc: non_trivial allocation failed");
                                return; // unreachable
                            }
                            udata.heap.non_trivial->copy_ctor(this, &o);
                        } else {
                            udata.heap.non_trivial = nullptr;
                            ::memcpy(udata.heap.memory, o.udata.heap.memory, m_size);
                        }
                    } else {
                        ::memcpy(udata.cache, o.udata.cache, std::abs(m_size));
                    }
                }

                delegate_t(delegate_t&& o) noexcept
                : m_cb( std::move( o.m_cb ) ), m_eqop( std::move( o.m_eqop ) ),
                  m_size( std::move( o.m_size ) ), m_type( std::move( o.m_type ) )
                {
                    if( m_size > 0 ) {
                        udata.heap.non_trivial = std::move( o.udata.heap.non_trivial );
                        o.udata.heap.non_trivial = nullptr;
                        if( nullptr != udata.heap.non_trivial ) {
                            udata.heap.memory = ::malloc(o.m_size);
                            if( nullptr == udata.heap.memory ) {
                                ABORT("Error: bad_alloc: heap.memory allocation failed");
                                return; // unreachable
                            }
                            udata.heap.non_trivial->move_ctor(this, &o);
                            udata.heap.non_trivial->dtor(&o);
                            ::free(o.udata.heap.memory);
                        } else {
                            udata.heap.memory = std::move( o.udata.heap.memory );
                        }
                        o.udata.heap.memory = nullptr;
                    } else {
                        ::memcpy(udata.cache, o.udata.cache, std::abs(m_size));
                    }
                    o.m_size = 0;
                }

                delegate_t& operator=(const delegate_t &o) noexcept
                {
                    if( this == &o ) {
                        return *this;
                    }
                    m_cb = o.m_cb;
                    m_eqop = o.m_eqop;
                    m_type = o.m_type;

                    if( 0 >= m_size && 0 >= o.m_size ) {
                        // sdata: copy
                        m_size = o.m_size;

                        ::memcpy(udata.cache, o.udata.cache, std::abs(m_size));
                    } else if( m_size > 0 && o.m_size > 0 && m_size >= o.m_size ) {
                        // vdata: reuse memory
                        m_size = o.m_size;

                        if( nullptr != udata.heap.non_trivial ) {
                            udata.heap.non_trivial->dtor(this);
                            if( nullptr != o.udata.heap.non_trivial ) {
                                *udata.heap.non_trivial = *o.udata.heap.non_trivial;
                            } else {
                                delete udata.heap.non_trivial;
                                udata.heap.non_trivial = nullptr;
                            }
                        }
                        if( nullptr != udata.heap.non_trivial ) {
                            udata.heap.non_trivial->copy_ctor(this, &o);
                        } else {
                            ::memcpy(udata.heap.memory, o.udata.heap.memory, m_size);
                        }
                    } else {
                        // reset
                        clear();
                        m_size = o.m_size;

                        if( m_size > 0 ) {
                            udata.heap.memory = ::malloc(m_size);
                            if( nullptr == udata.heap.memory ) {
                                ABORT("Error: bad_alloc: heap.memory allocation failed");
                                return *this; // unreachable
                            }
                            if( nullptr != o.udata.heap.non_trivial ) {
                                try {
                                    udata.heap.non_trivial = new non_trivial_t( *o.udata.heap.non_trivial );
                                } catch (const std::bad_alloc &e) {
                                    ABORT("Error: bad_alloc: non_trivial allocation failed");
                                    return *this; // unreachable
                                }
                                udata.heap.non_trivial->copy_ctor(this, &o);
                            } else {
                                udata.heap.non_trivial = nullptr;
                                ::memcpy(udata.heap.memory, o.udata.heap.memory, m_size);
                            }
                        } else {
                            ::memcpy(udata.cache, o.udata.cache, std::abs(m_size));
                        }
                    }
                    return *this;
                }

                delegate_t& operator=(delegate_t &&o) noexcept
                {
                    if( this == &o ) {
                        return *this;
                    }
                    clear();

                    m_cb = std::move( o.m_cb );
                    m_eqop = std::move( o.m_eqop );
                    m_size = std::move( o.m_size );
                    m_type = std::move( o.m_type );

                    if( m_size > 0 ) {
                        udata.heap.non_trivial = std::move( o.udata.heap.non_trivial );
                        o.udata.heap.non_trivial = nullptr;
                        if( nullptr != udata.heap.non_trivial ) {
                            udata.heap.memory = ::malloc(o.m_size);
                            if( nullptr == udata.heap.memory ) {
                                ABORT("Error: bad_alloc: heap.memory allocation failed");
                                return *this; // unreachable
                            }
                            udata.heap.non_trivial->move_ctor(this, &o);
                            udata.heap.non_trivial->dtor(&o);
                            ::free(o.udata.heap.memory);
                        } else {
                            udata.heap.memory = std::move( o.udata.heap.memory );
                        }
                        o.udata.heap.memory = nullptr;
                    } else {
                        ::memcpy(udata.cache, o.udata.cache, std::abs(m_size));
                    }
                    o.m_size = 0;
                    return *this;
                }

                template<typename T,
                         std::enable_if_t<std::is_trivially_copyable_v<T> &&
                                          sizeof(udata.cache) >= sizeof(T),
                                          bool> = true>
                constexpr const T* data() const noexcept {
                    // Alignment OK, udata.cache at start of delegate_t
                    return static_cast<const T*>( static_cast<const void*>( udata.cache ) ); // NOLINT(bugprone-casting-through-void): Same as reinterpret_cast<T*>( p )
                }

                template<typename T,
                         std::enable_if_t<!std::is_trivially_copyable_v<T> ||
                                          sizeof(udata.cache) < sizeof(T),
                                          bool> = true>
                constexpr const T* data() const noexcept {
                    return static_cast<const T*>( udata.heap.memory );
                }

                template<typename T,
                         std::enable_if_t<std::is_trivially_copyable_v<T> &&
                                          sizeof(udata.cache) >= sizeof(T),
                                          bool> = true>
                constexpr T* data() noexcept {
                    // Alignment OK, udata.cache at start of delegate_t
                    return static_cast<T*>( static_cast<void*>( udata.cache ) ); // NOLINT(bugprone-casting-through-void): Same as reinterpret_cast<T*>( p )
                }

                template<typename T,
                         std::enable_if_t<!std::is_trivially_copyable_v<T> ||
                                          sizeof(udata.cache) < sizeof(T),
                                          bool> = true>
                constexpr T* data() noexcept {
                    return static_cast<T*>( udata.heap.memory );
                }

                /**
                 * \brief Delegated fast path target function invocation, [see above](@ref delegate_class)
                 *
                 * @param args target function arguments
                 * @return target function result
                 */
                constexpr R operator()(A... args) const {
                    return m_cb(const_cast<delegate_t*>(this), args...);
                }

                /**
                 * \brief Delegated fast path target function equality operator
                 *
                 * @param rhs
                 * @return
                 */
                constexpr bool operator==(const delegate_t<R, A...>& rhs) const noexcept {
                    return m_eqop(*this, rhs);
                }

                /** Returns true if the underlying target function is `TriviallyCopyable`. */
                constexpr bool is_trivially_copyable() const noexcept { return 0 >= m_size || nullptr == udata.heap.non_trivial; }

                constexpr size_t heap_size() const noexcept { return 0 >= m_size ? 0 : ( m_size + ( nullptr != udata.heap.non_trivial ? sizeof(*udata.heap.non_trivial) : 0 ) ); }
                constexpr size_t cached_size() const noexcept { return 0 >= m_size ? std::abs(m_size) : 0; }

                /** Returns the size of underlying target function */
                constexpr size_t target_size() const noexcept { return std::abs(m_size); }

                /** Return the func::target_type of this invocation function wrapper */
                constexpr target_type type() const noexcept { return m_type; }
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
        class null_target_t final {
            public:
                typedef delegate_t<R, A...> delegate_type;

            private:
                constexpr static R invoke_impl(delegate_type* __restrict_cxx__ const, A...) {
                    return R();
                }

                constexpr static bool equal_op_impl(const delegate_type& lhs, const delegate_type& rhs) noexcept {
                    return lhs.type() == rhs.type();
                }

                constexpr static void dtor(delegate_type* target) noexcept {
                    (void)target;
                }

            public:
                static delegate_type delegate() noexcept {
                    return delegate_type::make(invoke_impl, equal_op_impl);
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
        class member_target_t final {
            public:
                typedef delegate_t<R, A...> delegate_type;

            private:

#if defined(__GNUC__) && !defined(__clang__)
                /**
                 * Utilizing GCC C++ Extension: Pointer to Member Function (PMF) Conversion to function pointer
                 * - Reduces function pointer size, i.e.  PMF 16 (total 24) -> function 8 (total 16)
                 * - Removes vtable lookup at invocation (performance)
                 * - Pass object this pointer to function as 1st argument
                 * - See [GCC PMF Conversion](https://gcc.gnu.org/onlinedocs/gcc/Bound-member-functions.html#Bound-member-functions)
                 */
                typedef R(*function_t)(C0*, A...);

                struct data_type final {
                    function_t function;
                    C1* base;

                    constexpr data_type(C1 *_base, R(C0::*_method)(A...)) noexcept
                    : base(_base)
                    {
                        PRAGMA_DISABLE_WARNING_PUSH
                        PRAGMA_DISABLE_WARNING_PMF_CONVERSIONS
                        PRAGMA_DISABLE_WARNING_PEDANTIC
                        function = (function_t)(_base->*_method);
                        PRAGMA_DISABLE_WARNING_POP
                    }
                };

                constexpr static R invoke_impl(delegate_type* __restrict_cxx__ const data, A... args) {
                    data_type * __restrict_cxx__ const d = data->template data<data_type>();
                    return ( *(d->function) )(d->base, args...);
                }

                constexpr static bool equal_op_impl(const delegate_type& lhs_, const delegate_type& rhs_) noexcept {
                    const data_type* lhs = lhs_.template data<const data_type>();
                    const data_type* rhs = rhs_.template data<const data_type>();
                    return lhs == rhs ||
                           ( lhs_.type() == rhs_.type() &&
                             lhs->base == rhs->base &&
                             lhs->function== rhs->function
                           );
                }
#else
                /**
                 * C++ conform Pointer to Member Function (PMF)
                 */
                struct data_type final {
                    C1* base;
                    R(C0::*method)(A...);

                    constexpr data_type(C1 *_base, R(C0::*_method)(A...)) noexcept
                    : base(_base), method(_method) { }
                };

                constexpr static R invoke_impl(delegate_type* __restrict_cxx__ const data, A... args) {
                    data_type * __restrict_cxx__ const d = data->template data<data_type>();
                    return (d->base->*d->method)(args...);
                }

                constexpr static bool equal_op_impl(const delegate_type& lhs_, const delegate_type& rhs_) noexcept {
                    const data_type* lhs = lhs_.template data<const data_type>();
                    const data_type* rhs = rhs_.template data<const data_type>();
                    return lhs == rhs ||
                           ( lhs_.type() == rhs_.type() &&
                             lhs->base == rhs->base &&
                             lhs->method == rhs->method
                           );
                }
#endif

            public:
                /**
                 * Construct a delegate_t<R, A...> instance from given this base-pointer and member-function.
                 *
                 * This factory function is only enabled if C0 is base of C1.
                 *
                 * @param base this base-pointer of class C1 derived from C0 or C0 used to invoke the member-function
                 * @param method member-function of class C0
                 * @return delegate_t<R, A...> instance holding the target-function object.
                 */
                static delegate_type delegate(C1 *base, R(C0::*method)(A...),
                                            std::enable_if_t<std::is_base_of_v<C0, C1>, bool> = true) noexcept
                {
                    return delegate_type::template make<data_type>( target_type::member, invoke_impl, equal_op_impl, base, method );
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
        class free_target_t final {
            public:
                typedef delegate_t<R, A...> delegate_type;

            private:
                struct data_type final {
                    R(*function)(A...);

                    data_type(R(*_function)(A...)) noexcept
                    : function(_function) { }
                };

                constexpr static R invoke_impl(delegate_type* __restrict_cxx__ const data, A... args) {
                    return ( *(data->template data<data_type>()->function) )(args...);
                }

                constexpr static bool equal_op_impl(const delegate_type& lhs_, const delegate_type& rhs_) noexcept {
                    const data_type* lhs = lhs_.template data<const data_type>();
                    const data_type* rhs = rhs_.template data<const data_type>();
                    return lhs == rhs ||
                           ( lhs_.type() == rhs_.type() &&
                             lhs->function== rhs->function
                           );
                }

            public:
                static delegate_type delegate(R(*function)(A...)) noexcept {
                    return delegate_type::template make<data_type>( target_type::free, invoke_impl, equal_op_impl, function );
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
        class lambda_target_t final {
            public:
                typedef delegate_t<R, A...> delegate_type;

            private:
                struct data_type final {
                    L function;
                    jau::type_info sig;

                    data_type(L _function) noexcept
                    : function( _function ), sig( jau::make_ctti<R, L, A...>() ) {}

                    constexpr size_t detail_size() const noexcept { return sizeof(function); }
                };

                constexpr static R invoke_impl(delegate_type* __restrict_cxx__ const data, A... args) {
                    return ( data->template data<data_type>()->function )(args...);
                }

                constexpr static bool equal_op_impl(const delegate_type& lhs_, const delegate_type& rhs_) noexcept {
                    const data_type* lhs = lhs_.template data<const data_type>();
                    const data_type* rhs = rhs_.template data<const data_type>();
                    return lhs == rhs ||
                           ( lhs_.type() == rhs_.type() &&
                             lhs->detail_size() == rhs->detail_size() &&                                        // fast:  wrong size -> false, otherwise ...
                             lhs->sig == rhs->sig &&                                                            // mixed: wrong jau::type_info -> false, otherwise ...
                             0 == ::memcmp((void*)&lhs->function, (void*)&rhs->function, sizeof(lhs->function)) // slow:  compare the anonymous data chunk of the lambda
                           );
                }

            public:
                static delegate_type delegate(L function) noexcept {
                    return delegate_type::template make<data_type>( target_type::lambda, invoke_impl, equal_op_impl, function );
                }
        };

        /**
         * func::ylambda_target_t is a [Y combinator](https://en.wikipedia.org/wiki/Fixed-point_combinator#Strict_functional_implementation)
         * and [deducing this](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p0847r7.html) implementation for lambda closures
         * usable for recursive algorithms.
         *
         * @anchor ylambda_target  The [Y combinator](https://en.wikipedia.org/wiki/Fixed-point_combinator#Strict_functional_implementation)
         * allows passing the unnamed lambda instance itself, enabling recursive invocation from within the lambda. <br />
         * In other words, a `this` reference of the unnamed lambda is passed to the lambda, similar to [`Deducing this`](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p0847r7.html).
         *
         * The ylambda [function<R(A...)>](@ref function_def) is invoked w/o explicitly passing the object parameter,
         * as it is implicitly passed down to the user's lambda implementation.
         *
         * Example implementing a recursive lambda factorial function
         *
         * ```
         *   function<int(int)> f1 = function<int(int)>::bind_ylambda( [](auto& self, int x) -> int {
         *       if( 0 == x ) {
         *           return 1;
         *       } else {
         *           return x * self(x-1); // recursion, calling itself w/o explicitly passing `self`
         *       }
         *   } );
         *   assert( 24 == f1(4) ); // `self` is bound to delegate<R(A...)> `f.target`, `x` is 4
         *
         *   // or using explicit function<R(A...)>::delegate_type
         *
         *   function<int(int)> f2 = function<int(int)>::bind_ylambda( [](function<int(int)>::delegate_type& self, int x) -> int {
         *       if( 0 == x ) {
         *           return 1;
         *       } else {
         *           return x * self(x-1); // recursion, calling itself w/o explicitly passing `self`
         *       }
         *   } );
         *   assert( 24 == f2(4) ); // `self` is bound to function<int(int)>::delegate_type `f.target`, `x` is 4
         * ```
         *
         * An instance is identifiable as func::target_type::ylambda via jau::function<R(A...)>::type().
         *
         * @tparam R function return type
         * @tparam L typename holding the lambda closure
         * @tparam A function arguments
         * @see @ref function_overview "Function Overview"
         * @see [Deducing this](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p0847r7.html)
         * @see [Explicit object parameter](https://en.cppreference.com/w/cpp/language/member_functions#Explicit_object_parameter)
         * @see [Curiously Recurring Template Pattern](https://en.cppreference.com/w/cpp/language/crtp)
         */
        template<typename R, typename L, typename...A>
        class ylambda_target_t final {
            public:
                typedef delegate_t<R, A...> delegate_type;

            private:
                struct data_type final {
                    L function;
                    jau::type_info sig;

                    data_type(L _function) noexcept
                    : function( _function ), sig( jau::make_ctti<R, L, A...>() ) {
                        // jau::type_cue<L>::print("ylambda_target_t.lambda", TypeTraitGroup::ALL);
                        // jau::type_cue<data_type>::print("ylambda_target_t.data_type", TypeTraitGroup::ALL);
                        // fprintf(stderr, "ylambda_target: %s\n\t\tsize %zu, %p: %s\n",
                        //         sig.name(), sizeof(L), &function, jau::bytesHexString(&function, 0, sizeof(L), true).c_str());
                    }

                    constexpr size_t detail_size() const noexcept { return sizeof(function); }
                };

                constexpr static R invoke_impl(delegate_type* __restrict_cxx__ const data, A... args) {
                    return ( data->template data<data_type>()->function )(*data, args...);
                }

                constexpr static bool equal_op_impl(const delegate_type& lhs_, const delegate_type& rhs_) noexcept {
                    const data_type* lhs = lhs_.template data<const data_type>();
                    const data_type* rhs = rhs_.template data<const data_type>();
                    return lhs == rhs ||
                           ( lhs_.type() == rhs_.type() &&
                             lhs->detail_size() == rhs->detail_size() &&                                        // fast:  wrong size -> false, otherwise ...
                             lhs->sig == rhs->sig &&                                                            // mixed: wrong jau::type_info -> false, otherwise ...
                             0 == ::memcmp((void*)&lhs->function, (void*)&rhs->function, sizeof(lhs->function)) // slow:  compare the anonymous data chunk of the lambda
                           );
                }

            public:
                static delegate_type delegate(L function) noexcept {
                    return delegate_type::template make<data_type>( target_type::ylambda, invoke_impl, equal_op_impl, function );
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
        class capval_target_t final {
            public:
                typedef delegate_t<R, A...> delegate_type;

            private:
                struct data_type final {
                    R(*function)(I&, A...);
                    I data;

                    data_type(const I& _data, R(*_function)(I&, A...)) noexcept
                    : function(_function), data(_data) {}

                    data_type(I&& _data, R(*_function)(I&, A...)) noexcept
                    : function(_function), data(std::move(_data)) {}
                };

                constexpr static R invoke_impl(delegate_type* __restrict_cxx__ const data, A... args) {
                    data_type* __restrict_cxx__ const d = data->template data<data_type>();
                    return (*d->function)(d->data, args...);
                }

                constexpr static bool equal_op_impl(const delegate_type& lhs_, const delegate_type& rhs_) noexcept {
                    const data_type* lhs = lhs_.template data<const data_type>();
                    const data_type* rhs = rhs_.template data<const data_type>();
                    return lhs == rhs ||
                           ( lhs_.type() == rhs_.type() &&
                             lhs->function == rhs->function &&
                             lhs->data == rhs->data
                           );
                }

            public:
                static delegate_type delegate(const I& data, R(*function)(I&, A...)) noexcept {
                    return delegate_type::template make<data_type>( target_type::capval, invoke_impl, equal_op_impl, data, function );
                }

                static delegate_type delegate(I&& data, R(*function)(I&, A...)) noexcept {
                    return delegate_type::template make<data_type>( target_type::capval, invoke_impl, equal_op_impl, std::move(data), function );
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
        class capref_target_t final {
            public:
                typedef delegate_t<R, A...> delegate_type;

            private:
                struct data_type final {
                    R(*function)(I*, A...);
                    I* data_ptr;

                    data_type(I* _data_ptr, R(*_function)(I*, A...)) noexcept
                    : function(_function), data_ptr(_data_ptr) {}
                };

                constexpr static R invoke_impl(delegate_type* __restrict_cxx__ const data, A... args) {
                    data_type* __restrict_cxx__ const d = data->template data<data_type>();
                    return (*d->function)(d->data_ptr, args...);
                }

                constexpr static bool equal_op_impl(const delegate_type& lhs_, const delegate_type& rhs_) noexcept {
                    const data_type* lhs = lhs_.template data<const data_type>();
                    const data_type* rhs = rhs_.template data<const data_type>();
                    return lhs == rhs ||
                           ( lhs_.type() == rhs_.type() &&
                             lhs->function == rhs->function &&
                             lhs->data_ptr == rhs->data_ptr
                           );
                }

            public:
                static delegate_type delegate(I* data_ptr, R(*function)(I*, A...)) noexcept {
                    return delegate_type::template make<data_type>( target_type::capref, invoke_impl, equal_op_impl, data_ptr, function );
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
        class std_target_t final {
            public:
                typedef delegate_t<R, A...> delegate_type;

            private:
                struct data_type final {
                    std::function<R(A...)> function;
                    uint64_t id;

                    data_type(uint64_t id_, std::function<R(A...)> function_) noexcept
                    : function(std::move(function_)), id(id_) {}

                    constexpr size_t detail_size() const noexcept { return sizeof(id)+sizeof(function); }
                };

                constexpr static R invoke_impl(delegate_type* __restrict_cxx__ const data, A... args) {
                    data_type* __restrict_cxx__ const d = data->template data<data_type>();
                    if( d->function ) {
                        return d->function(args...);
                    } else {
                        return R();
                    }
                }

                constexpr static bool equal_op_impl(const delegate_type& lhs_, const delegate_type& rhs_) noexcept {
                    const data_type* lhs = lhs_.template data<const data_type>();
                    const data_type* rhs = rhs_.template data<const data_type>();
                    return lhs == rhs ||
                           ( lhs_.type() == rhs_.type() &&
                             lhs->id == rhs->id &&
                             lhs->detail_size() && rhs->detail_size()
                           );
                }

            public:
                static delegate_type delegate(uint64_t id, std::function<R(A...)> function) noexcept {
                    return delegate_type::template make<data_type>( target_type::std, invoke_impl, equal_op_impl, id, function );
                }
        };
        /**@}*/

    } /* namespace func */

     constexpr uint32_t number(const func::target_type rhs) noexcept {
         return static_cast<uint32_t>(rhs);
     }
     std::string to_string(const func::target_type v) noexcept;

    /**
     * Class template [jau::function](@ref function_def) is a general-purpose static-polymorphic function wrapper.
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
     * @anchor function_def Class template [jau::function](@ref function_def) is a general-purpose static-polymorphic function wrapper.
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
            /** The delegated target function type, i.e. func::delegate_t<R, A...> */
            typedef func::delegate_t<R, A...> delegate_type;

        private:
            delegate_type target;

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
            : target( func::null_target_t<R, A...>::delegate() )
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
            : target( func::null_target_t<R, A...>::delegate() )
            { }

            /**
             * \brief Internally used delegate_t<R(A...)> constructor
             *
             * May utilize [copy elision](https://en.cppreference.com/w/cpp/language/copy_elision)
             * and/or named return value optimization (NRVO).
             *
             * @see @ref function_overview "function Overview"
             * @see @ref function_usage "function Usage"
             */
            explicit function(delegate_type _delegate, int dummy ) noexcept
            : target( std::move(_delegate) )
            { (void) dummy; }

            /**
             * \brief Free function constructor
             *
             * @anchor function_ctor_free
             * Constructs an instance by taking a free target function, which may also be a non-capturing lambda if explicitly bind_free().
             * @see @ref function_overview "function Overview"
             * @see @ref function_usage "function Usage"
             */
            function(R(*func)(A...)) noexcept
            : target( func::free_target_t<R, A...>::delegate(func) )
            { }

            /**
             * \brief Lambda function constructor
             *
             * @anchor function_ctor_lambda
             * Constructs an instance by taking a lambda function.
             *
             * @tparam L typename holding the lambda closure
             * @param func the lambda reference
             * @see @ref function_overview "function Overview"
             * @see @ref function_usage "function Usage"
             */
            template<typename L,
                     std::enable_if_t<!std::is_same_v<L, std::shared_ptr<delegate_type>> &&
                                      !std::is_pointer_v<L> &&
                                      !std::is_same_v<L, R(A...)> &&
                                      !std::is_same_v<L, function<R(A...)>>
                     , bool> = true>
            function(L func) noexcept
            : target( func::lambda_target_t<R, L, A...>::delegate(func) )
            { }

            /**
             * \brief Lambda function bind factory
             *
             * @anchor function_bind_lambda
             * Constructs an instance by taking a lambda function.
             *
             * @tparam L typename holding the lambda closure
             * @param func the lambda reference
             * @see @ref function_overview "function Overview"
             * @see @ref function_usage "function Usage"
             */
            template<typename L>
            static function<R(A...)> bind_lambda(L func) noexcept
            {
                return function<R(A...)>( jau::func::lambda_target_t<R, L, A...>::delegate(func), 0 );
            }

            /**
             * \brief Y combinator Lambda function bind factory
             *
             * @anchor function_bind_ylambda
             * Constructs an instance by taking a lambda function,
             * implementing a [Y combinator](https://en.wikipedia.org/wiki/Fixed-point_combinator#Strict_functional_implementation)
             * and [deducing this](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p0847r7.html) lambda closure,
             * see [ylambda_target_t](@ref ylambda_target).
             *
             * @tparam L typename holding the lambda closure
             * @param func the lambda reference
             * @see @ref ylambda_target "ylambda_target_t"
             * @see @ref function_overview "function Overview"
             * @see @ref function_usage "function Usage"
             */
            template<typename L>
            static function<R(A...)> bind_ylambda(L func) noexcept
            {
                return function<R(A...)>( jau::func::ylambda_target_t<R, L, A...>::delegate(func), 0 );
            }

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
            : target( func::member_target_t<R, C0, C1, A...>::delegate(base, mfunc) )
            { }

            /**
             * \brief Capture by value (copy) function constructor
             *
             * @anchor function_ctor_capval_copy
             * Constructs an instance by copying the captured value and the given non-void function to
             * an anonymous function using func::capval_target_t.
             *
             * `const I& data` will be copied into func::capval_target_t and hence captured by-copy.
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
            : target( func::capval_target_t<R, I, A...>::delegate(data, func) )
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
            : target( func::capval_target_t<R, I, A...>::delegate(std::forward<I>(data), func) )
            { }

            /**
             * \brief Capture by-reference function constructor
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
            : target( func::capref_target_t<R, I, A...>::delegate(data_ptr, func) )
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
            : target( func::std_target_t<R, A...>::delegate(id, func) )
            { }

            function(const function &o) noexcept = default;
            function(function &&o) noexcept = default;
            function& operator=(const function &o) noexcept = default;
            function& operator=(function &&o) noexcept = default;

            /** Return the jau::func::type of this instance */
            constexpr func::target_type type() const noexcept { return target.type(); }

            /** Returns true if this instance does not hold a callable target function, i.e. is of func::target_type::null.  */
            constexpr bool is_null() const noexcept { return func::target_type::null == target.type(); }

            /** Returns true if this instance holds a callable target function, i.e. is not of func::target_type::null.  */
            explicit constexpr operator bool() const noexcept { return !is_null(); }

            /** Returns signature of this function prototype R(A...) w/o underlying target function object. */
            jau::type_info signature() const noexcept {
                return jau::make_ctti<R(A...)>();
            }

            /** Returns true if the underlying target function is `TriviallyCopyable`. */
            constexpr bool is_target_trivially_copyable() const noexcept { return target.is_trivially_copyable(); }

            /** Return the total size of this instance, may include heap allocated by delegate for bigger target functions. */
            constexpr size_t size() const noexcept { return target.heap_size() + sizeof(*this); }

            /** Returns the size of underlying target function */
            constexpr size_t target_size() const noexcept { return target.target_size(); }

            /**
             * Return a string representation of this instance.
             * @anchor function_toString The string representation contains the complete signature and detailed memory footprint.
             */
            std::string toString() const {
                return "function<" + to_string( type() ) + ", " + signature().name() + ">( sz net " +
                        std::to_string( target_size() ) + " / ( delegate_t " +
                        std::to_string( sizeof( target ) ) + " + target_vdata " +
                        std::to_string( target.heap_size() ) + " -> "+
                        std::to_string( size() ) + " ), trivial_cpy "+
                        std::to_string( target.is_trivially_copyable() ) + " ) ";
            }

            constexpr R operator()(A... args) const {
                return target(args...);
            }
            constexpr R operator()(A... args) {
                return target(args...);
            }

            constexpr bool operator==(const function<R(A...)>& rhs) const noexcept {
                return target.operator==(rhs.target);
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
     * Equal operator of jau::function<R(A...)> with a right-hand-side nullptr
     * @tparam R left function return type
     * @tparam A left function arguments
     * @param lhs left function
     * @return true if function instance contains no function, i.e. `!lhs` negated function bool operator.
     */
    template< class R, class... A>
    bool operator==(const function<R(A...)>& lhs, std::nullptr_t) noexcept
    { return !lhs; }

    /**
     * Unequal operator of jau::function<R(A...)> with a right-hand-side nullptr
     * @tparam R left function return type
     * @tparam A left function arguments
     * @param lhs left function
     * @return true if function instance contains a function, i.e. `lhs` function bool operator.
     */
    template< class R, class... A>
    bool operator!=(const function<R(A...)>& lhs, std::nullptr_t) noexcept
    { return !( lhs == nullptr ); }

    /**
     * Equal operator of jau::function<R(A...)> with a left-hand-side nullptr
     * @tparam R right function return type
     * @tparam A right function arguments
     * @param rhs right function
     * @return true if function instance contains no function, i.e. `!lhs` negated function bool operator.
     */
    template< class R, class... A>
    bool operator==(std::nullptr_t, const function<R(A...)>& rhs) noexcept
    { return !rhs; }

    /**
     * Unequal operator of jau::function<R(A...)> with a left-hand-side nullptr
     * @tparam R right function return type
     * @tparam A right function arguments
     * @param rhs right function
     * @return true if function instance contains a function, i.e. `lhs` function bool operator.
     */
    template< class R, class... A>
    bool operator!=(std::nullptr_t, const function<R(A...)>& rhs) noexcept
    { return !( nullptr == rhs ); }

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
        return function<R(A...)>( func::member_target_t<R, C0, C1, A...>::delegate(base, mfunc), 0 );
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
        return function<R(A...)>( func::member_target_t<R, C, C, A...>::delegate(base, mfunc), 0 );
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
        return function<void(A...)>( func::member_target_t<void, C0, C1, A...>::delegate(base, mfunc), 0 );
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
        return function<void(A...)>( func::member_target_t<void, C, C, A...>::delegate(base, mfunc), 0 );
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
        return function<R(A...)>( func::free_target_t<R, A...>::delegate(func), 0 );
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
        return function<void(A...)>( func::free_target_t<void, A...>::delegate(func), 0 );
    }

    /**
     * Bind given data by copying the captured value and the given non-void function to
     * an anonymous function using func::capval_target_t.
     *
     * `const I& data` will be copied into func::capval_target_t and hence captured by-copy.
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
        return function<R(A...)>( func::capval_target_t<R, I, A...>::delegate(data, func), 0 );
    }

    /**
     * Bind given data by copying the captured value and the given void function to
     * an anonymous function using func::capval_target_t.
     *
     * `const I& data` will be copied into func::capval_target_t and hence captured by-copy.
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
        return function<void(A...)>( func::capval_target_t<void, I, A...>::delegate(data, func), 0 );
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
        return function<R(A...)>( func::capval_target_t<R, I, A...>::delegate(std::forward<I>(data), func), 0 );
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
        return function<void(A...)>( func::capval_target_t<void, I, A...>::delegate(std::forward<I>(data), func), 0 );
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
        return function<R(A...)>( func::capref_target_t<R, I, A...>::delegate(data_ptr, func), 0 );
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
        return function<void(A...)>( func::capref_target_t<void, I, A...>::delegate(data_ptr, func), 0 );
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
        return function<R(A...)>( func::std_target_t<R, A...>::delegate(id, func), 0 );
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
        return function<void(A...)>( func::std_target_t<void, A...>::delegate(id, func), 0 );
    }

} // namespace jau

/** \example test_functional.hpp
 * This C++ unit test validates the jau::function<R(A...)> and all its jau::func::target_t specializations.
 */

/** \example test_functional_perf.hpp
 * This C++ unit test benchmarks the jau::function<R(A...)> and all its jau::func::target_t specializations.
 */

#endif /* JAU_FUNCTIONAL_HPP_ */
