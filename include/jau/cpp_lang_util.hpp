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

#ifndef CPP_LANG_EXT_HPP_
#define CPP_LANG_EXT_HPP_

#include <type_traits>

namespace jau {

    /** @defgroup CppLang C++ Language Utilities
     *  C++ language utilities, language feature alignment, type trails, data alignment and intrinsics.
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

    /**
     * Used when designed to declare a function `constexpr`,
     * but prohibited by its specific implementation.
     * <p>
     * Evaluated using the alternative qualifier `inline` for C++ < 20,
     * as it is implied for `constexpr` functions or static member variables, see constexpr_cxx20.
     * </p>
     * <p>
     * Here it but uses non-literal variables, such as std::lock_guard etc.
     * As these can't be evaluated at compile time, the standard does
     * not allow using `constexpr` here.
     * </p>
     * <p>
     * Empty until standard defines otherwise.
     * </p>
     * @see constexpr_cxx20
     */
    #define constexpr_non_literal_var inline

    /**
     * Used when designed to declare a function `constexpr`,
     * but prohibited by its specific implementation.
     * <p>
     * Evaluated using the alternative qualifier `inline` for C++ < 20,
     * as it is implied for `constexpr` functions or static member variables, see constexpr_cxx20.
     * </p>
     * <p>
     * Here it uses thread-safety related measures like atomic storage
     * or mutex locks, which are non-literal variables and hence
     * prohibit the use of `constexpr`.
     * </p>
     * @see constexpr_cxx20
     * @see constexpr_non_literal_var
     */
    #define constexpr_atomic inline

    #if defined(__clang__)
        #if __has_feature(cxx_rtti)
            /**
             * Set define if RTTI is enabled during compilation,
             * implying its runtime availability.
             * <pre>
             * - clang ('__clang__') may have '__has_feature(cxx_rtti)'
             * - g++   ('__GNUC__')  may have '__GXX_RTTI'
             * - msvc  (_MSC_VER)    may have: '_CPPRTTI'
             * </pre>
             */
            #define __cxx_rtti_available__ 1
        #endif
    #else
        #if defined(__GXX_RTTI) || defined(_CPPRTTI)
            /**
             * Set define if RTTI is enabled during compilation,
             * implying its runtime availability.
             * <pre>
             * - clang ('__clang__') may have '__has_feature(cxx_rtti)'
             * - g++   ('__GNUC__')  may have '__GXX_RTTI'
             * - msvc  (_MSC_VER)    may have: '_CPPRTTI'
             * </pre>
             */
            #define __cxx_rtti_available__ 1
        #endif
    #endif

    #if defined(__clang__)
        #define JAU_PRETTY_FUNCTION __PRETTY_FUNCTION__
    #elif defined(__GNUC__) && !defined(__clang__)
        #define JAU_PRETTY_FUNCTION __PRETTY_FUNCTION__
    #elif defined(_MSC_VER)
        #define JAU_PRETTY_FUNCTION __FUNCSIG__
    #else
        #error "JAU_PRETTY_FUNCTION not available"
    #endif

    template<typename T>
    constexpr const char* pretty_function() {
        return JAU_PRETTY_FUNCTION;
    }

    /**
    // *************************************************
    // *************************************************
    // *************************************************
     */

    #if defined __has_builtin
        #if __has_builtin(__builtin_bit_cast)
            #define __has_builtin_bit_cast 1
        #endif
    #endif

    /**
     * Convenience type trait for `__has_builtin(__builtin_bit_cast)`.
     * @tparam Dummy_type just to make template `SFINAE` happy
     * @see jau::is_builtin_bit_cast_available()
     * @see jau::bit_cast()
     * @see jau::pointer_cast()
     */
    template <typename Dummy_type>
    struct has_builtin_bit_cast
        #if defined __has_builtin_bit_cast
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
    template <typename Dummy_type> constexpr bool has_builtin_bit_cast_v = has_builtin_bit_cast<Dummy_type>::value;

    #if !defined __has_builtin_bit_cast
        /**
         * Dummy definition in the absence of this builtin function
         * as required to have this compilation unit compile clean.
         * @param Dest_type the target type
         * @param Value_arg the source value argument
         */
        #define __builtin_bit_cast(Dest_type,Value_arg) 0
    #endif

    namespace impl {
        template<class Dummy_type>
        constexpr bool has_builtin_bit_cast_impl(
                std::enable_if_t< has_builtin_bit_cast_v<Dummy_type>, bool> = true ) noexcept
        {
            return true;
        }

        template<class Dummy_type>
        constexpr bool has_builtin_bit_cast_impl(
                std::enable_if_t< !has_builtin_bit_cast_v<Dummy_type>, bool> = true ) noexcept
        {
            return false;
        }
    }

    /**
     * Query whether `__builtin_bit_cast(Dest_type, arg)` is available, using jau::has_builtin_bit_cast.
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
     *  clang      |   9.0.1  | amd64, arm64        | yes       |
     *  clang      |  11.0.1  | amd64               | yes       |
     *
     * @return `true` if query subject is available, otherwise not.
     * @see has_builtin_bit_cast
     * @see bit_cast()
     * @see pointer_cast()
     */
    constexpr bool is_builtin_bit_cast_available() noexcept {
        return impl::has_builtin_bit_cast_impl<bool>();
    }

    /**
     * C++20 `bit_cast<>(arg)` implementation for C++17.
     * <p>
     * Functional if is_builtin_bit_cast_available() evaluates `true`.
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
        if constexpr ( is_builtin_bit_cast_available() ) {
            return __builtin_bit_cast(Dest, src);
        } else {
            (void)src;
            return 0;
        }
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
        sizeof(Dest) == sizeof(Source) &&
        std::is_pointer_v<Source> &&
        std::is_pointer_v<Dest>,
        Dest>
    pointer_cast(const Source& src) noexcept
    {
        if constexpr ( is_builtin_bit_cast_available() ) {
            return __builtin_bit_cast(Dest, src);
        } else {
            // not 'really' constexpr .. oops, working though
            return reinterpret_cast<Dest>( const_cast< std::remove_const_t< std::remove_pointer_t<Source> >* >( src ) );
        }
    }

    /**@}*/

} // namespace jau

#endif /* CPP_LANG_EXT_HPP_ */
