/*
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
 *
 */

#ifndef JAU_CPP_LANG_EXT_HPP_
#define JAU_CPP_LANG_EXT_HPP_

#include <climits>
#include <cstdint>
#include <string_view>
#include <type_traits>
#include <exception>
#include <string>
#include <cstring>
#include <ostream>
#if __cplusplus > 201703L
    // C++20
    #include <bit>
#endif
#if __cplusplus > 202002L
    // C++23
    #include <stdbit>
#endif

#include <jau/packed_attribute.hpp>

namespace jau {

    /** @defgroup CppLang C++ Language Utilities
     *  C++ language utilities, language feature alignment, type trails, data alignment and intrinsics.
     *
     * Used predefined `__cplusplus` macro identifier for C++ language specs:
     * - `199711L`: pre C++11
     * - `201103L`: C++11
     * - `201402L`: C++14
     * - `201703L`: C++17
     * - `202002L`: C++20
     * - `202302L`: C++23
     * - `??????L`: C++26 ??
     *
     * Used predefined macros denoting the compiler:
     * - `__clang__`     : LLVM's clang, clang++
     * - `__GNUC__`      : GNU Compiler Collection (GCC)'s gcc, g++
     * - `_MSC_VER`      : Microsoft Compiler
     * - `__MINGW32__`   : MinGW 32
     * - `__MINGW64__`   : MinGW 64
     * - `__EMSCRIPTEN__`: emscripten for asm.js and WebAssembly
     *
     * Further infos:
     * - [Unix standards](https://sourceforge.net/p/predef/wiki/Standards/)
     * - [GNU glibc](https://sourceforge.net/p/predef/wiki/Libraries/)
     * - [glibc 1.3.4 Feature Test Macros](https://www.gnu.org/software/libc/manual/html_node/Feature-Test-Macros.html)
     * - [Architectures](https://sourceforge.net/p/predef/wiki/Architectures/)
     *
     *  @{
     */

    /**
     * `consteval` qualifier replacement for C++20 `consteval`.
     *
     * > A `consteval` specifier implies `inline`.
     * > At most one of the `constexpr`, `consteval`, and `constinit` specifiers is allowed to appear within the same sequence of declaration specifiers.
     * > ...
     * > An immediate function is a `constexpr` function,
     * > and must satisfy the requirements applicable to `constexpr` functions or `constexpr` constructors, as the case may be.
     *
     * <p>
     * Evaluated using the alternative qualifier `constexpr` for C++ < 20,
     * as it is almost contained within `consteval` but lacks the `immediate function` constraint.
     * </p>
     * <p>
     * Evaluated as `consteval` for C++20.
     * </p>
     */
#if __cplusplus > 201703L
    #define consteval_cxx20 consteval
#else
    #define consteval_cxx20 constexpr
#endif

    /** Returns true if compiled with >= C++17 */
    consteval_cxx20 bool is_cxx17() noexcept {
        #if __cplusplus > 201402L
            return true;
        #else
            return false;
        #endif
    }
    /** Returns true if compiled with >= C++20 */
    consteval_cxx20 bool is_cxx20() noexcept {
        #if __cplusplus > 201703L
            return true;
        #else
            return false;
        #endif
    }
    /** Returns true if compiled with >= C++23 */
    consteval_cxx20 bool is_cxx23() noexcept {
        #if __cplusplus > 202002L
            return true;
        #else
            return false;
        #endif
    }
    /** Returns true if compiled with >= C++26 */
    consteval_cxx20 bool is_cxx26() noexcept {
        #if __cplusplus > 202302L
            return true;
        #else
            return false;
        #endif
    }

    /**
     * `constinit` qualifier replacement for C++20 `constinit`.
     *
     * > `constinit` cannot be used together with `constexpr` or `consteval`.
     * > When the declared variable is a reference, `constinit` is equivalent to `constexpr`.
     * > When the declared variable is an object,
     * > `constexpr` mandates that the object must have static initialization and constant destruction
     * > and makes the object const-qualified, however, `constinit` does not mandate constant destruction and const-qualification.
     *
     * <p>
     * Evaluated using the alternative qualifier `constexpr` for C++ < 20,
     * as it is almost contained within `constinit` but lacks the loosening of not mandating constant destruction and const-qualification.<br>
     * FIXME: Due to the above, this replacement might not be suitable: TBD!
     * </p>
     * <p>
     * Evaluated as `constinit` for C++20.
     * </p>
     */
#if __cplusplus > 201703L
    #define constinit_cxx20 constinit
#else
    #define constinit_cxx20 constexpr
#endif

    /**
     * `constexpr` qualifier replacement for C++20 `constexpr`.
     *
     * > A `constexpr` specifier used in a function or static member variable (since C++17) declaration implies `inline`.
     *
     * <p>
     * Evaluated using the alternative qualifier `inline` for C++ < 20,
     * as it is implied for `constexpr` functions or static member variables, see above.
     * </p>
     * <p>
     * Evaluated as `constexpr` for C++20, i.e. std::string literals, virtual functions, etc.
     * </p>
     */
#if __cplusplus > 201703L
    #define constexpr_cxx20 constexpr
#else
    #define constexpr_cxx20 inline
#endif

#if __cplusplus > 202002L
    #define constexpr_cxx23 constexpr
#else
    #define constexpr_cxx23 inline
#endif

#if __cplusplus > 202302L
    #define constexpr_cxx26 constexpr
#else
    #define constexpr_cxx26 inline
#endif


    /**
     * Used when designed to declare a function `constexpr`,
     * but prohibited by its specific implementation.
     * <p>
     * Evaluated using the alternative qualifier `inline` for C++ < 23,
     * as it is implied for `constexpr` functions or static member variables, see constexpr_cxx23.
     * </p>
     * <p>
     * Here it but uses non-literal variables, such as std::lock_guard etc.
     * As these can't be evaluated at compile time, the standard does
     * not allow using `constexpr` here.
     * </p>
     * <p>
     * Empty until standard defines otherwise.
     * </p>
     * @see constexpr_cxx23
     */
    #define constexpr_non_literal_var constexpr_cxx23

    /**
     * Used when designed to declare a function `constexpr`,
     * but prohibited by its specific implementation.
     * <p>
     * Evaluated using the alternative qualifier `inline` for C++ < 23,
     * as it is implied for `constexpr` functions or static member variables, see constexpr_cxx23.
     * </p>
     * <p>
     * Here it uses thread-safety related measures like atomic storage
     * or mutex locks, which are non-literal variables and hence
     * prohibit the use of `constexpr`.
     * </p>
     * @see constexpr_cxx23
     * @see constexpr_non_literal_var
     */
    #define constexpr_atomic constexpr_cxx23

    /**
     * Wrap C++ extension `__restrict__` covering C99's `restrict` feature keyword.
     */
    #if defined(__clang__)
        #define __restrict_cxx__ __restrict__
    #elif defined(__GNUC__) && !defined(__clang__)
        #define __restrict_cxx__ __restrict__
    #elif defined(_MSC_VER)
        #define __restrict_cxx__ __restrict
    #else
        #define __restrict_cxx__
    #endif

    #if defined(__clang__)
        /** Consider using [jau::ctti_name<R, L, A...>()](@ref ctti_name_lambda). */
        #define JAU_PRETTY_FUNCTION __PRETTY_FUNCTION__
    #elif defined(__GNUC__) && !defined(__clang__)
        /** Consider using [jau::ctti_name<R, L, A...>()](@ref ctti_name_lambda). */
        #define JAU_PRETTY_FUNCTION __PRETTY_FUNCTION__
    #elif defined(_MSC_VER)
        /** Consider using [jau::ctti_name<R, L, A...>()](@ref ctti_name_lambda). */
        #define JAU_PRETTY_FUNCTION __FUNCSIG__
    #else
        #error "JAU_PRETTY_FUNCTION not available"
    #endif

    /**
     * Returns the demangled given mangled_name if successful,
     * otherwise the mangled_name.
     *
     * Implementation utilizes the [cross-vendor C++ ABI abi::__cxa_demangle()](https://gcc.gnu.org/onlinedocs/libstdc++/libstdc++-html-USERS-4.3/a01696.html)
     * as supported at least on on `gcc` and `clang`.
     *
     * May be used to demangle the result of jau::type_name() or jau::type_info::name() if jau::type_info::rtti_available == true,
     * i.e. RTTI typeif(T) symbols are being used.
     *
     * See also [gcc libstdc++ FAQ, Chapter 28](https://gcc.gnu.org/onlinedocs/libstdc++/manual/ext_demangling.html).
     *
     * Further, implementation also checks whether the mangled_name results from [jau::ctti_name<T>()](@ref ctti_name_type)
     * and cleans up the known `gcc` and `clang` variant of JAU_PRETTY_FUNCTION.
     *
     * @param mangled_name mangled name
     */
    std::string demangle_name(const char* mangled_name) noexcept;

    #if defined(__clang__)
        /** Optional generic usage of final keyword w/o negative performance impact. (Disabled) */
        #define final_opt
    #elif defined(__GNUC__) && !defined(__clang__)
        /** Optional generic usage of final keyword w/o negative performance impact. (Enabled) */
        #define final_opt final
    #elif defined(_MSC_VER)
        /** Optional generic usage of final keyword w/o negative performance impact. (Enabled, OK?) */
        #define final_opt final
    #else
        /** Optional generic usage of final keyword w/o negative performance impact. (Enabled, OK?) */
        #define final_opt final
    #endif

    /**
    // *************************************************
    // *************************************************
    // *************************************************
     */

    /**
     * Convenience type trait for `__has_builtin(__builtin_bit_cast)`.
     * @tparam Dummy_type just to make template `SFINAE` happy
     * @see jau::has_builtin_bit_cast()
     * @see jau::bit_cast()
     * @see jau::pointer_cast()
     */
    template <typename Dummy_type>
    struct has_builtin_bit_cast_t
        #if defined __has_builtin && __has_builtin(__builtin_bit_cast)
            : std::true_type
        #else
            : std::false_type
        #endif
            {};
    /**
     * Value access of has_builtin_bit_cast type trait for convenience ..
     * @tparam Dummy_type just to make template `SFINAE` happy
     * @see has_builtin_bit_cast
     */
    template <typename Dummy_type> constexpr bool has_builtin_bit_cast_v = has_builtin_bit_cast_t<Dummy_type>::value;

    /**
     * Query whether `__builtin_bit_cast(Dest_type, arg)` is available
     * via `__has_builtin(__builtin_bit_cast)`.
     *
     * - - - - - - - - - - - - - - -
     *
     * Availability of `__builtin_bit_cast(Dest_type, arg)`
     *
     * Reflecting my manual platform tests using `test_basictypeconv.cpp`
     *
     *  Compiler   | Version  | Architecture        | Available |
     *  :--------- | -------: | :------------------ | :-------- |
     *  GCC        |   8.3.0  | amd64, arm64, arm32 | no        |
     *  GCC        |  10.2.1  | amd64               | no        |
     *  GCC        |  12.2.0  | amd64               | yes       |
     *  clang      |   9.0.1  | amd64, arm64        | yes       |
     *  clang      |  11.0.1  | amd64               | yes       |
     *
     * @return `true` if query subject is available, otherwise not.
     * @see has_builtin_bit_cast_t
     * @see bit_cast()
     * @see pointer_cast()
     */
    consteval_cxx20 bool has_builtin_bit_cast() noexcept {
        #if defined __has_builtin && __has_builtin(__builtin_bit_cast)
            return true;
        #else
            return false;
        #endif
    }

    /**
     * A `constexpr` pointer cast implementation for C++17,
     * inspired by C++20 `bit_cast<>(arg)`.
     * <p>
     * If is_builtin_bit_cast_available() evaluates `true`,
     * implementation uses `__builtin_bit_cast(Dest, src)`.<br>
     *
     * Otherwise a simple `reinterpret_cast<Dest>(src)` is utilized,
     * which officially is questionable to deliver a `constexpr`.
     * </p>
     * @tparam Dest the target pointer type
     * @tparam Source the source pointer argument type
     * @param src the pointer to convert to Dest pointer type
     * @return the converted Dest pointer type value
     * @see jau::has_builtin_bit_cast
     * @see is_builtin_bit_cast_available()
     * @see bit_cast()
     */
    template <class Dest, class Source>
    constexpr
    typename std::enable_if_t<
        sizeof(Dest) == sizeof(Source) &&   // NOLINT(bugprone-sizeof-expression): Intended, same pointer size
        std::is_pointer_v<Source> &&
        std::is_pointer_v<Dest>,
        Dest>
    pointer_cast(const Source& src) noexcept
    {
        if constexpr ( is_cxx20() ) {
            return std::bit_cast<Dest, Source>(src); // NOLINT(bugprone-bitwise-pointer-cast): intentional
        } else if constexpr ( has_builtin_bit_cast() ) {
            return __builtin_bit_cast(Dest, src);
        } else {
            // not 'really' constexpr .. oops, working though
            return reinterpret_cast<Dest>( const_cast< std::remove_const_t< std::remove_pointer_t<Source> >* >( src ) );
        }
    }

    /**
     * C++20 `bit_cast<>(arg)` implementation for C++17.
     * <p>
     * Utilizing native bit_cast if is_builtin_bit_cast_available(), otherwise `pointer_cast<const packed_t<Dest>*>( &src )->store`.
     * </p>
     * @tparam Dest the target type
     * @tparam Source the source argument type
     * @param src the value to convert to Dest type
     * @return the converted Dest type value
     * @see jau::has_builtin_bit_cast
     * @see is_builtin_bit_cast_available()
     * @see pointer_cast()
     */
    template <class Dest, class Source>
    constexpr
    typename std::enable_if_t<
        sizeof(Dest) == sizeof(Source) &&
        std::is_trivially_copyable_v<Dest> &&
        std::is_trivially_copyable_v<Source>,
        Dest>
    bit_cast(const Source& src) noexcept
    {
        if constexpr ( is_cxx20() ) {
            return std::bit_cast<Dest, Source>(src);
        } else if constexpr ( has_builtin_bit_cast() ) {
            return __builtin_bit_cast(Dest, src);
        } else {
            return pointer_cast<const packed_t<Dest>*>( &src )->store;
        }
    }

    consteval_cxx20 bool is_builtin_int128_available() noexcept {
        #if defined(__SIZEOF_INT128__)
            return true;
        #else
            return false;
        #endif
    }

    #if defined(__SIZEOF_INT128__)
       // Prefer TI mode over __int128 as GCC rejects the latter in pendantic mode
       #if defined(__GNUG__)
         typedef          int  int128_t __attribute__((mode(TI)));
         typedef unsigned int uint128_t __attribute__((mode(TI)));
       #else
         typedef          __int128  int128_t;
         typedef unsigned __int128 uint128_t;
       #endif
    #endif

    /** Returns true if compiled with debug information and w/o optimization, i.e. not `defined(NDEBUG) && !defined(DEBUG)`. */
    consteval_cxx20 bool is_debug_enabled() noexcept {
        #if defined(NDEBUG) && !defined(DEBUG)
            return false;
        #else
            return true;
        #endif
    }

    #if defined(__clang__)
        #define __attrdecl_no_optimize__ __attribute__ ((optnone))
        #define __attrdef_no_optimize__ __attribute__ ((optnone))
    #elif defined(__GNUC__) && !defined(__clang__)
        #define __attrdecl_no_optimize__ __attribute__((optimize("O0")))
        // #define __attrdecl_no_optimize__ [[gnu::optimize("O0")]]
        #define __attrdef_no_optimize__
    #else
        #define __attrdecl_no_optimize__
        #define __attrdef_no_optimize__
    #endif

    /**
     * Simple unary function wrapper which ensures function call to happen in order and not optimized away.
     */
    template <typename UnaryFunc>
    inline void callNotOptimize(UnaryFunc f) __attrdef_no_optimize__ {
        // asm asm-qualifiers ( AssemblerTemplate : OutputOperands [ : InputOperands [ : Clobbers ] ] )
        asm volatile("" : "+r,m"(f) : : "memory"); // a nop asm, usually guaranteeing synchronized order and non-optimization
        f();
    }

    /**@}*/

    /** \addtogroup Exceptions
     *
     *  @{
     */

    /**
     * Handle given optional exception (nullable std::exception_ptr) and send std::exception::what() message to `stderr`
     * @param eptr contains optional exception, may be `nullptr`
     * @return true if `eptr` contained an exception pointer, false otherwise (`nullptr`)
     */
    inline __attribute__((always_inline))
    bool handle_exception(std::exception_ptr eptr, const char* file, int line) noexcept { // NOLINT(performance-unnecessary-value-param) passing by value is OK
        if (eptr) {
            try {
                std::rethrow_exception(eptr);
            } catch (const std::exception &e) {
                ::fprintf(stderr, "Exception caught @ %s:%d: %s\n", file, line, e.what());
                return true;
            }
        }
        return false;
    }

    /// No throw wrap for given unary predicate `p` action. Returns true for success (no exception), otherwise false (exception occurred).
    template<class UnaryPredicate>
    inline bool do_noexcept(UnaryPredicate p) noexcept {
        std::exception_ptr eptr;
        try {
            p();
            return true;
        } catch (...) {
            eptr = std::current_exception();
        }
        return !handle_exception(eptr, __FILE__, __LINE__);
    }

    /**@}*/

    /** \addtogroup StringUtils
     *
     *  @{
     */

    /// No throw wrap for given unary predicate `p` producing a `std::string`. Returns an empty string if `p` causes an exception.
    template<class UnaryPredicate>
    inline std::string string_noexcept(UnaryPredicate p) noexcept {
        std::exception_ptr eptr;
        try {
            return p();
        } catch (...) {
            eptr = std::current_exception();
        }
        handle_exception(eptr, __FILE__, __LINE__);
        return std::string();
    }

    /// No throw `std::string(std::string_view)` instantiation
    inline std::string string_noexcept(std::string_view v) noexcept {
        return string_noexcept([v]() { return std::string(v); });
    }

    /**@}*/

    /** \addtogroup CppLang
     *
     *  @{
     */

    /// Boolean type without implicit conversion, safe for function parameter
    enum class Bool : bool {
        False = false,
        True = true
    };
    constexpr Bool True() noexcept { return Bool::True; }
    constexpr Bool False() noexcept { return Bool::False; }
    constexpr Bool makeBool(bool v) noexcept { return v ? Bool::True : Bool::False; }

    constexpr bool value(const Bool rhs) noexcept {
        return static_cast<bool>(rhs);
    }
    constexpr bool operator*(const Bool rhs) noexcept {
        return static_cast<bool>(rhs);
    }
    constexpr Bool operator!(const Bool rhs) noexcept {
        return Bool(!*rhs);
    }
    constexpr Bool operator&&(const Bool lhs, const Bool rhs) noexcept {
        return Bool(*lhs && *rhs);
    }
    constexpr Bool operator||(const Bool lhs, const Bool rhs) noexcept {
        return Bool(*lhs || *rhs);
    }
    constexpr Bool operator^(const Bool lhs, const Bool rhs) noexcept {
        return Bool(*lhs ^ *rhs);
    }
    constexpr Bool operator|(const Bool lhs, const Bool rhs) noexcept {
        return Bool(*lhs || *rhs);
    }
    constexpr Bool operator&(const Bool lhs, const Bool rhs) noexcept {
        return Bool(*lhs && *rhs);
    }
    constexpr Bool& operator|=(Bool& lhs, const Bool rhs) noexcept {
        lhs = lhs | rhs;
        return lhs;
    }
    constexpr Bool& operator&=(Bool& lhs, const Bool rhs) noexcept {
        lhs = lhs & rhs;
        return lhs;
    }
    constexpr Bool& operator^=(Bool& lhs, const Bool rhs) noexcept {
        lhs = lhs ^ rhs;
        return lhs;
    }
    constexpr bool operator==(const Bool lhs, const Bool rhs) noexcept {
        return *lhs == *rhs;
    }
    constexpr std::string_view name(const Bool v) noexcept {
        if( *v ) {
            return "true";
        } else {
            return "false";
        }
    }
    inline std::string to_string(const Bool v) noexcept {
        return string_noexcept([v]() { return std::string(name(v)); });
    }
    inline std::ostream & operator << (std::ostream &out, const Bool c) {
        out << to_string(c);
        return out;
    }

    //
    //
    //

    /** Simple pre-defined value pair [size_t, bool] for structured bindings to multi-values. */
    struct SizeBoolPair {
        /** a size_t value, e.g. index, length, etc */
        size_t s;
        /** a boolean value, e.g. success, etc */
        bool b;
    };
    /** Simple pre-defined value pair [uint8_t*, size_t, bool] for structured bindings to multi-values. */
    struct UInt8PtrSizeBoolPair {
        /** a uint8_t* pointer value */
        uint8_t* p;
        /** a size_t value, e.g. index, length, etc */
        size_t s;
        /** a boolean value, e.g. success, etc */
        bool b;
    };

    /** Simple pre-defined value tuple [uint64_t, size_t, bool] for structured bindings to multi-values. */
    struct UInt64SizeBoolTuple {
        /** a uint64_t value, e.g. compute result value, etc */
        uint64_t v;
        /** a size_t value, e.g. index, length, etc */
        size_t s;
        /** a boolean value, e.g. success, etc */
        bool b;
    };

    /** Simple pre-defined value tuple [int64_t, size_t, bool] for structured bindings to multi-values. */
    struct Int64SizeBoolTuple {
        /** a int64_t value, e.g. compute result value, etc */
        int64_t v;
        /** a size_t value, e.g. index, length, etc */
        size_t s;
        /** a boolean value, e.g. success, etc */
        bool b;
    };

    //
    //
    //

    namespace impl {
        inline bool runtime_eval(bool v) { return v; }
    }

    consteval_cxx20 void consteval_assert(bool v) {
        [[maybe_unused]] bool r = v ? true : impl::runtime_eval(v);
    }

    /**@}*/

} // namespace jau

/** \addtogroup CppLang
 *
 *  @{
 */

#define E_FILE_LINE __FILE__, __LINE__

//
// JAU_FOR_EACH macros inspired by David Mazières, June 2021
// <https://www.scs.stanford.edu/~dm/blog/va-opt.html>
//
// All hacks below to circumvent lack of C++26 reflection.
//

// Note space before (), so object-like macro
#define JAU_PARENS ()

#define JAU_EXPAND(...) JAU_EXPAND4(JAU_EXPAND4(JAU_EXPAND4(JAU_EXPAND4(__VA_ARGS__))))
#define JAU_EXPAND4(...) JAU_EXPAND3(JAU_EXPAND3(JAU_EXPAND3(JAU_EXPAND3(__VA_ARGS__))))
#define JAU_EXPAND3(...) JAU_EXPAND2(JAU_EXPAND2(JAU_EXPAND2(JAU_EXPAND2(__VA_ARGS__))))
#define JAU_EXPAND2(...) JAU_EXPAND1(JAU_EXPAND1(JAU_EXPAND1(JAU_EXPAND1(__VA_ARGS__))))
#define JAU_EXPAND1(...) __VA_ARGS__

// Macro w/ 1 arguments

#define JAU_FOR_EACH1_LIST(macro, ...)                              \
  __VA_OPT__(JAU_EXPAND(JAU_FOR_EACH1_LIST_HELPER(macro, __VA_ARGS__)))
#define JAU_FOR_EACH1_LIST_HELPER(macro, a1, ...)                   \
  macro(a1)                                                         \
  __VA_OPT__(, JAU_FOR_EACH1_LIST_AGAIN JAU_PARENS (macro, __VA_ARGS__))
#define JAU_FOR_EACH1_LIST_AGAIN() JAU_FOR_EACH1_LIST_HELPER

#define JAU_DECLTYPE_VALUE(type) decltype(type)
#define JAU_NOREF_DECLTYPE_VALUE(type) std::remove_reference_t<decltype(type)>

// Macro w/ 2 arguments

#define JAU_FOR_EACH2(macro, type, ...)                              \
  __VA_OPT__(JAU_EXPAND(JAU_FOR_EACH2_HELPER(macro, type, __VA_ARGS__)))
#define JAU_FOR_EACH2_HELPER(macro, type, a1, ...)                   \
  macro(type, a1)                                                    \
  __VA_OPT__(JAU_FOR_EACH2_AGAIN JAU_PARENS (macro, type, __VA_ARGS__))
#define JAU_FOR_EACH2_AGAIN() JAU_FOR_EACH2_HELPER

#define JAU_FOR_EACH2_LIST(macro, type, ...)                              \
  __VA_OPT__(JAU_EXPAND(JAU_FOR_EACH2_LIST_HELPER(macro, type, __VA_ARGS__)))
#define JAU_FOR_EACH2_LIST_HELPER(macro, type, a1, ...)                   \
  macro(type, a1)                                                         \
  __VA_OPT__(, JAU_FOR_EACH2_LIST_AGAIN JAU_PARENS (macro, type, __VA_ARGS__))
#define JAU_FOR_EACH2_LIST_AGAIN() JAU_FOR_EACH2_LIST_HELPER

#define JAU_FOR_EACH2_VALUE(macro, type, value, ...)             \
  __VA_OPT__(JAU_EXPAND(JAU_FOR_EACH2_VALUE_HELPER(macro, type, value, __VA_ARGS__)))
#define JAU_FOR_EACH2_VALUE_HELPER(macro, type, value, a1, ...)  \
  macro(type, a1, value)                                         \
  __VA_OPT__(JAU_FOR_EACH2_VALUE_AGAIN JAU_PARENS (macro, type, value, __VA_ARGS__))
#define JAU_FOR_EACH2_VALUE_AGAIN() JAU_FOR_EACH2_VALUE_HELPER

// Macro w/ 3 arguments: type, a1, a2
#define JAU_FOR_EACH3(macro, type, ...)                              \
  __VA_OPT__(JAU_EXPAND(JAU_FOR_EACH3_HELPER(macro, type, __VA_ARGS__)))
#define JAU_FOR_EACH3_HELPER(macro, type, a1, a2, ...)               \
  macro(type, a1, a2)                                                \
  __VA_OPT__(JAU_FOR_EACH3_AGAIN JAU_PARENS (macro, type, __VA_ARGS__))
#define JAU_FOR_EACH3_AGAIN() JAU_FOR_EACH3_HELPER

/**@}*/

#endif /* JAU_CPP_LANG_EXT_HPP_ */
