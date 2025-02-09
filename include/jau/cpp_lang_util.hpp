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

#include <type_traits>
#include <typeinfo>
#include <string>
#include <cstring>
#include <ostream>
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

#if defined(__cxx_rtti_available__)
    /**
     * Template type trait evaluating std::true_type{} if RTTI is available, otherwise std::false_type{}
     * @tparam _Dummy unused dummy type to satisfy SFINAE
     */
    template<typename _Dummy> struct is_rtti_available_t : std::true_type {};
#else
    /**
     * Template type trait evaluating std::true_type{} if RTTI is available, otherwise std::false_type{}
     * @tparam _Dummy unused dummy type to satisfy SFINAE
     */
    template<typename _Dummy> struct is_rtti_available_t : std::false_type {};
#endif

    /**
     * Template type trait helper evaluating true if RTTI is available, otherwise false
     * @tparam _Dummy unused dummy type to satisfy SFINAE
     */
    template <typename _Dummy> inline constexpr bool is_rtti_available_v = is_rtti_available_t<_Dummy>::value;

    /** Returns true if compiled with RTTI available */
    consteval_cxx20 bool is_rtti_available() noexcept {
        #if defined(__cxx_rtti_available__)
            return true;
        #else
            return false;
        #endif
    }

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
     * @anchor ctti_name_type
     * Returns the type name of given type `T`
     * using template *Compile Time Type Information (CTTI)* only
     * with static constant storage duration.
     *
     * @tparam T the type
     * @return instance of jau::type_info
     * @see @ref make_ctti_type "jau::make_ctti<T>"
     * @see @ref ctti_name_lambda
     */
    template<typename T>
    constexpr const char* ctti_name() noexcept {
        return JAU_PRETTY_FUNCTION;
    }

    /**
     * @anchor ctti_name_lambda
     * Returns the type name of given function types `R(*L)(A...)`
     * using template *Compile Time Type Information (CTTI)* only
     * with static constant storage duration.
     *
     * @anchor ctti_name_lambda_limitations
     * #### Limitations
     *
     * ##### Non unique function pointer type names with same prototype
     * With RTTI or wihout, c-alike function pointer type names like `int(*)(int)` do not expose their
     * source location like lambda functions do.
     * Hence they can't be used to compare code identity, but lambda functions can be used.
     *
     * ##### Non unique lambda type names without RTTI using `gcc` or non `clang` compiler
     * Due to the lack of standardized *Compile-Time Type Information (CTTI)*,
     * we rely on the non-standardized macro extensions
     * - `__PRETTY_FUNCTION__`
     *   - `clang` produces a unique tag using filename and line number, compatible.
     *   - `gcc` produces a non-unique tag using the parent function of the lambda location and its too brief signature, not fully compatible.
     * - `__FUNCSIG__`
     *   - `msvc++` not tested
     * - Any other compiler is not supported yet
     *
     * Due to these restrictions, *not using RTTI on `gcc` or non `clang` compiler* will *erroneously mistake* different lambda
     * *functions defined within one function and using same function prototype `R<A...>`* to be the same.
     *
     * jau::type_info::limited_lambda_id exposes the potential limitation.
     *
     * @tparam R function return type
     * @tparam L main function type, e.g. a lambda type.
     * @tparam A function argument types
     * @return instance of jau::type_info
     * @see @ref make_ctti_lambda "jau::make_ctti<R, L, A...>"
     * @see @ref ctti_name_type
     * @see jau::type_info::limited_lambda_id
     */
    template<typename R, typename L, typename...A>
    constexpr const char* ctti_name() noexcept {
        return JAU_PRETTY_FUNCTION;
    }

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

    /**
     * Generic type information using either *Runtime type information* (RTTI) or *Compile time type information* (CTTI)
     *
     * @anchor type_info  jau::type_info exposes same properties as RTTI std::type_index,
     * i.e. can be used as index in associative and unordered associative containers
     * and is CopyConstructible and CopyAssignable.
     *
     * jau::type_info is compatible with std::type_index operations.
     *
     * jau::type_info can be utilized w/o RTTI using
     * *Compile time type information* (CTTI) information, i.e. JAU_PRETTY_FUNCTION via [jau::ctti_name<R, L, A...>()](@ref ctti_name_lambda).
     *
     * Consider using [jau::make_ctti<R, L, A...>()](@ref make_ctti_lambda) for construction,
     * as it removes the RTTI and CTTI code path differences.
     *
     * ### RTTI and Compile time type information (CTTI) Natures
     *
     * If RTTI is being used, see __cxx_rtti_available__,
     * jau::type_info may be instantiated with a std::type_info reference as returned from typeid(T).
     *
     * Without RTTI jau::type_info may be instantiated using JAU_PRETTY_FUNCTION.
     * Hence utilizes Compile time type information (CTTI) only.
     *
     * In both cases, jau::type_info will solely operate on the `const char* signature`
     * and its hash value, aligning memory footprint and operations.
     *
     * Use [jau::make_ctti<R, L, A...>()](@ref make_ctti_lambda) for construction,
     * as it removes the RTTI and CTTI code path differences.
     *
     * @anchor type_info_identity
     * ### Notes about lifecycle and identity
     *
     * #### Prologue
     * We assume block scope and static storage duration (runtime lifecycle) of
     * - (1) [typeid(T)]((https://en.cppreference.com/w/cpp/language/typeid)
     *   - (1.1) The typeid expression is an lvalue expression which refers to an object with static storage duration.
     *   - (1.2) There is no guarantee that the same std::type_info instance will be referred to by all evaluations
     *           of the typeid expression on the same type, although they would compare equal.
     * - (2) JAU_PRETTY_FUNCTION, aka __PRETTY_FUNCTION__ with properties of [__func__](https://en.cppreference.com/w/cpp/language/function)
     *   - (2.1) This variable has block scope and static storage duration.
     *
     * #### Equality Comparison
     *
     * - compare the static name storage references (pointer) and return `true` if equal (fast path)
     * - due to (1.2), see above, the static name storage strings must be compared
     *   - compare the names' hash value and return `false` if not matching (fast path)
     *   - compare the static names' string and return the result (slow `strcmp()` equality)
     *     - this avoids a potential hash collision.
     *
     * @anchor type_info_limitations
     * #### Limitations
     *
     * ##### Non unique lambda type names without RTTI using `gcc` or non `clang` compiler
     *
     * Due to [limitations of jau::make_ctti<R, L, A...>()](@ref ctti_name_lambda_limitations),
     * *not using RTTI on `gcc` or non `clang` compiler* will *erroneously mistake* different lambda
     * *functions defined within one function and using same function prototype `R<A...>`* to be the same.
     *
     * jau::type_info::limited_lambda_id exposes the potential limitation.
     *
     * @see @ref type_info_identity "Identity"
     * @see @ref make_ctti_lambda "jau::make_ctti<R, L, A...>"
     */
    class type_info {
        private:
            const char* m_signature;
            size_t m_hash_value;
            bool m_identity_obj;
            bool m_identity_sig;

        public:
            /**
             * Static constexpr boolean indicating whether resulting type_info
             * uniqueness is limited for lambda function types.
             *
             * Always is `false` if rtti_available == `true`,
             * i.e. lambda function types are always unique using RTTI.
             *
             * May return `true` if:
             * - no RTTI and using `gcc`
             * - no RTTI and not using `clang`
             *
             * @see @ref ctti_name_lambda_limitations "CTTI lambda name limitations"
             */
            static constexpr const bool limited_lambda_id =
                #if defined(__cxx_rtti_available__)
                    false;
                #else
                    #if defined(__clang__)
                        false;
                    #elif defined(__GNUC__) && !defined(__clang__)
                        true;
                    #else
                        true; // unknown
                    #endif
                #endif

            /** Returns true if given signature is not nullptr and has a string length > 0, otherwise false. */
            static constexpr bool is_valid(const char* signature) noexcept {
                return nullptr != signature && 0 < ::strlen(signature);
            }

            /** Aborts program execution if given signature is nullptr or has a string length == 0. */
            static void abort_invalid(const char* signature) noexcept {
                if( nullptr == signature ) {
                    fprintf(stderr, "ABORT @ %s:%d %s: CTTI signature nullptr\n", __FILE__, __LINE__, __func__);
                    ::abort();
                } else if( 0 == ::strlen(signature) ) {
                    fprintf(stderr, "ABORT @ %s:%d %s: CTTI signature zero sized\n", __FILE__, __LINE__, __func__);
                    ::abort();
                }
            }

            /**
             * Constructor for an empty type_info instance, i.e. empty name() signature.
             */
            type_info() noexcept
            : m_signature(""), m_hash_value( std::hash<std::string_view>{}(std::string_view(m_signature)) ),
              m_identity_obj(false), m_identity_sig(true)
            { }

            /**
             * Constructor using an RTTI std::type_info reference, i.e. typeid(T) result.
             *
             * Consider using [jau::make_ctti<R, L, A...>()](@ref make_ctti_lambda) for construction,
             * as it removes the RTTI and CTTI code path differences.
             *
             * @param info_ RTTI std::type_info reference
             *
             * @see @ref make_ctti_lambda "jau::make_ctti<R, L, A...>"
             */
            type_info(const std::type_info& info_, bool identity_instance=false) noexcept
            : m_signature(info_.name()), m_hash_value( info_.hash_code() ),
              m_identity_obj(identity_instance),
              m_identity_sig(false) // RTTI type_info can't guarantee identity for name()
            { }

            /**
             * Constructor using a `const char*` signature with a static storage duration
             *
             * Aborts program execution if given signature is nullptr or has a string length == 0.
             *
             * @param signature_ valid string signature of type with length > 0 with static storage duration.
             *
             * @see @ref make_ctti_lambda "jau::make_ctti<R, L, A...>"
             */
            type_info(const char* signature_, bool identity_instance=false, bool identity_signature=false) noexcept
            : m_signature( signature_ ),
              m_hash_value( nullptr != m_signature ? std::hash<std::string_view>{}(std::string_view(m_signature)) : 0 ),
              m_identity_obj(identity_instance),
              m_identity_sig(identity_signature)
            { abort_invalid(m_signature); }

            /**
             * Return true if both instances are equal.
             *
             * @param rhs
             * @return
             * @see @ref type_info_identity "Identity"
             */
            constexpr bool operator==(const type_info& rhs) const noexcept {
                if( &rhs == this ) {                                     // fast: equal obj address
                    return true;
                }
                return ( !m_identity_obj || !rhs.m_identity_obj ) &&     // fast: fail if both instances use an identity obj address
                       ( m_signature == rhs.m_signature ||               // fast: signature address comparison, which may fail on same types, _or_
                         ( ( !m_identity_sig || !rhs.m_identity_sig ) && // fast: fail if both instances use an identity signature address
                           ( m_hash_value == rhs.m_hash_value &&         // fast: wrong hash value -> false, otherwise avoid hash collision case ...
                             0 == ::strcmp(m_signature, rhs.m_signature) // slow: string comparison if !m_use_hash_only
                           )
                         )
                       );
            }

            bool operator!=(const type_info& rhs) const noexcept
            { return !operator==(rhs); }

            /**
             * Returns an unspecified hash code of this instance.
             *
             * @anchor type_info_hash_code
             * Properties
             * - for all type_info objects referring to the same type, their hash code is the same.
             * - type_info objects referring to different types may have the same hash code, i.e. due to hash collision.
             *
             * Compatible with std::type_info definition.
             *
             * @return Unspecified hash code of this instance.
             * @see @ref type_info_identity "Identity"
             */
            size_t hash_code() const noexcept { return m_hash_value; }

            /** Returns true if this instance has a unique address (for same type_info). */
            bool identityInstance() const noexcept { return m_identity_obj; }

            /** Returns true if internal_name() has a unique identity address (for same strings). */
            bool identityName() const noexcept { return m_identity_sig; }

            /** Returns the type name, compiler implementation specific.  */
            const char* internal_name() const noexcept
            { return m_signature; }

            /** Returns the demangled name of internal_name(). */
            std::string name() const noexcept {
                return demangle_name( m_signature );
            }
            std::string toString() const noexcept;
    };
    inline std::ostream& operator<<(std::ostream& out, const type_info& v) {
        return out << v.toString();
    }

    /**
     * Constructs a jau::type_info instance based on given type `T`
     * using template *Compile Time Type Information (CTTI)* only.
     *
     * @anchor make_ctti_type
     * This construction function either uses `typeid(T)` if jau::type_info::rtti_available == true
     * or [jau::ctti_name<T>()](@ref ctti_name_type) otherwise.
     *
     * @tparam T type for which the type_info is generated
     * @return instance of jau::type_info
     * @see @ref ctti_name_type "jau::ctti_name<T>"
     */
    template<typename T>
    jau::type_info make_ctti(bool identity_instance=false) noexcept {
#if defined(__cxx_rtti_available__)
        return jau::type_info(typeid(T), identity_instance);
#else
        return jau::type_info(ctti_name<T>(), identity_instance, true /* identity_signature */);
#endif
    }

    /** Returns a static global reference of make_ctti<T>(true) w/ identity instance */
    template<typename T>
    const jau::type_info& static_ctti() noexcept {
        static const type_info sig = make_ctti<T>(true);
        return sig;
    }

    /**
     * Constructs a jau::type_info instance based on given function types `R(*L)(A...)`
     * using template *Compile Time Type Information (CTTI)* only
     * via RTTI's `typeid(L) if available or [jau::ctti_name<R, L, A...>()](@ref ctti_name_lambda) otherwise.
     *
     * @anchor make_ctti_lambda
     * This construction function either uses `typeid(L)` if jau::type_info::rtti_available == true
     * or [jau::ctti_name<R, L, A...>()](@ref ctti_name_lambda) otherwise.
     *
     * @tparam R function return type used for type_info in case of jau::type_info::rtti_available == false
     * @tparam L main function type for which the type_info is generated, e.g. a lambda type.
     * @tparam A function argument types used for type_info in case of jau::type_info::rtti_available == false
     * @return instance of jau::type_info
     * @see @ref ctti_name_lambda "jau::ctti_name<R, L, A...>"
     */
    template<typename R, typename L, typename...A>
    jau::type_info make_ctti(bool identity_instance=false) noexcept {
#if defined(__cxx_rtti_available__)
        return jau::type_info(typeid(L), identity_instance);
#else
        return jau::type_info(ctti_name<R, L, A...>(), identity_instance, true /* identity_signature */);
#endif
    }

    /** Returns a static global reference of make_ctti<R, L, A...>(true) w/ identity instance */
    template<typename R, typename L, typename...A>
    const jau::type_info& static_ctti() noexcept {
        static const type_info sig = make_ctti<R, L, A...>(true);
        return sig;
    }

    /**
     * Returns the type name of given type `T`
     * using template *Compile Time Type Information (CTTI)* only
     * via RTTI's `typeid(T).name()` if available or [jau::ctti_name<T>()](@ref ctti_name_type) otherwise.
     *
     * @tparam T type for which the type_info is generated
     * @return type name
     * @see @ref ctti_name_type "jau::ctti_name<T>"
     */
    template<typename T>
    const char* type_name() noexcept {
#if defined(__cxx_rtti_available__)
        return typeid(T).name();
#else
        return ctti_name<T>();
#endif
    }

    /**
     * Returns the type name of given function types `R(*L)(A...)`
     * using template *Compile Time Type Information (CTTI)* only
     * via RTTI's `typeid(L).name()` if available or [jau::ctti_name<R, L, A...>()](@ref ctti_name_lambda) otherwise.
     *
     * @tparam R function return type used for type_info in case of jau::type_info::rtti_available == false
     * @tparam L main function type for which the type_info is generated, e.g. a lambda type.
     * @tparam A function argument types used for type_info in case of jau::type_info::rtti_available == false
     * @return type name
     * @see @ref ctti_name_lambda "jau::ctti_name<R, L, A...>"
     */
    template<typename R, typename L, typename...A>
    const char* type_name() noexcept {
#if defined(__cxx_rtti_available__)
        return typeid(L).name();
#else
        return ctti_name<R, L, A...>();
#endif
    }

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
        if constexpr ( has_builtin_bit_cast() ) {
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
        if constexpr ( has_builtin_bit_cast() ) {
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

    /// Boolean type without implicit conversion, safe for function parameter
    enum class Bool : bool {
        False = false,
        True = true
    };
    constexpr Bool True() noexcept { return Bool::True; }
    constexpr Bool False() noexcept { return Bool::False; }

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
    constexpr std::string to_string(const Bool v) noexcept { return std::string(name(v)); }
    inline std::ostream & operator << (std::ostream &out, const Bool c) {
        out << to_string(c);
        return out;
    }

    /**@}*/

} // namespace jau

#endif /* JAU_CPP_LANG_EXT_HPP_ */
