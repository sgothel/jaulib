/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020-2026 Gothel Software e.K.
 *
 * SPDX-License-Identifier: MIT
 *
 * This Source Code Form is subject to the terms of the MIT License
 * If a copy of the MIT was not distributed with this file,
 * you can obtain one at https://opensource.org/license/mit/.
 */

#ifndef JAU_TYPE_INFO_HPP_
#define JAU_TYPE_INFO_HPP_

#include <jau/cpp_lang_util.hpp>
#include <jau/enum_util.hpp>
#include <jau/hash_fnv1a.hpp>

namespace jau {

    /** \addtogroup CppLang
     *
     *  @{
     */

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

    /**
     * @anchor ctti_name_type
     * Returns the type name of given type `T`
     * at compile-type for *Compile Time Type Information (CTTI)* with static and constant storage duration.
     *
     * TODO: Evaluate C++20 [source_location's function_name](https://en.cppreference.com/cpp/utility/source_location/function_name) across compiler
     *
     * @tparam T the type
     * @return instance of jau::type_info
     * @see @ref make_ctti_type "jau::make_ctti<T>"
     * @see @ref ctti_name_lambda
     * @see [__PRETTY_FUNCTION__](https://gcc.gnu.org/onlinedocs/gcc-14.2.0/gcc/Function-Names.html)
     * @see [__FUNCSIG__](https://learn.microsoft.com/en-us/cpp/preprocessor/predefined-macros?view=msvc-170)
     */
    template<typename T>
    constexpr const char* ctti_name() noexcept {
        return JAU_PRETTY_FUNCTION;
    }

    /**
     * @anchor ctti_name_lambda
     * Returns the type name of given function type `R(*L)(A...)`
     * at compile-type for *Compile Time Type Information (CTTI)* with static and constant storage duration.
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
     * - [__PRETTY_FUNCTION__](https://gcc.gnu.org/onlinedocs/gcc-14.2.0/gcc/Function-Names.html)
     *   - `clang` produces a unique tag using filename and line number, compatible.
     *   - `gcc` produces a non-unique tag using the parent function of the lambda location and its too brief signature, not fully compatible.
     * - `__FUNCSIG__`
     *   - `msvc++` not yet tested, but should work according to their spec
     * - Any other compiler is not supported yet
     *
     * Due to these restrictions, *not using RTTI on `gcc` or non `clang` compiler* will *erroneously mistake* different lambda
     * *functions defined within one function and using same function prototype `R<A...>`* to be the same.
     *
     * jau::type_info::limited_lambda_id exposes the potential limitation.
     *
     * TODO: Evaluate C++20 [source_location's function_name](https://en.cppreference.com/cpp/utility/source_location/function_name) across compiler
     *
     * @tparam R function return type
     * @tparam L main function type, e.g. a lambda type.
     * @tparam A function argument types
     * @return instance of jau::type_info
     * @see @ref make_ctti_lambda "jau::make_ctti<R, L, A...>"
     * @see @ref ctti_name_type
     * @see jau::type_info::limited_lambda_id
     * @see [__PRETTY_FUNCTION__](https://gcc.gnu.org/onlinedocs/gcc-14.2.0/gcc/Function-Names.html)
     * @see [__FUNCSIG__](https://learn.microsoft.com/en-us/cpp/preprocessor/predefined-macros?view=msvc-170)
     */
    template<typename R, typename L, typename...A>
    consteval const char* ctti_name() noexcept {
        return JAU_PRETTY_FUNCTION;
    }

    /**
     * *Compile-time Type Info* (CTTI)
     *
     * @anchor type_info  jau::type_info exposes same properties as RTTI std::type_index,
     * i.e. can be used as index in associative and unordered associative containers
     * and is CopyConstructible and CopyAssignable - but at compile-time.
     *
     * jau::type_info is compatible with std::type_index operations.
     *
     * ### CTTI - Compile-time Info Nature
     *
     * CTTI via `consteval` is achieved via [jau::ctti_name<T>()](@ref ctti_name_type) and [jau::ctti_name<R, L, A...>()](@ref ctti_name_lambda).
     *
     * jau::type_info solely operates on `const char* signature`
     * and its hash value, aligning memory footprint and operations.
     *
     * @anchor type_info_identity
     * ### Notes about lifecycle and identity
     *
     * #### Prologue
     * We assume block scope and static storage duration (runtime lifecycle) of
     * - (1) JAU_PRETTY_FUNCTION, aka [__PRETTY_FUNCTION__](https://gcc.gnu.org/onlinedocs/gcc-14.2.0/gcc/Function-Names.html)
     *       with properties of [__func__](https://en.cppreference.com/w/cpp/language/function)
     *   - (1.1) This variable has block scope and static storage duration
     *   - (1.2) There is no guarantee that the same instance will be referred to by all evaluations
     *           of the same type, but they shall compare equal.
     *
     * #### Equality Comparison
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
            std::string_view m_signature;
            size_t m_hash_value;

            template<typename T>
            friend consteval jau::type_info make_ctti() noexcept;

            template<typename T>
            friend const jau::type_info& static_ctti() noexcept;

            template<typename R, typename L, typename...A>
            friend consteval jau::type_info make_ctti() noexcept;

            template<typename R, typename L, typename...A>
            friend const jau::type_info& static_ctti() noexcept;

            /**
             * Constructor using a `const char*` compile-time signature with a static storage duration
             *
             * @param signature_ valid string signature of type with length > 0 with static storage duration.
             *
             * @see @ref make_ctti_lambda "jau::make_ctti<R, L, A...>"
             */
            consteval type_info(const char* signature_) noexcept
            : m_signature( nullptr != signature_ ? signature_ : "<nullsig>"),
              m_hash_value( jau::fnv1a_size_t::hash(m_signature) )
            { }

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
            static constexpr bool limited_lambda_id =
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
            static constexpr bool is_valid(std::string_view signature) noexcept {
                return nullptr != signature.data() && 0 < signature.length();
            }

            /**
             * Constructor for an empty type_info instance, i.e. empty name() signature.
             */
            constexpr type_info() noexcept
            : m_signature("<notype>"),
              m_hash_value( jau::fnv1a_size_t::hash(m_signature) )
            { }

            constexpr type_info(const type_info&) noexcept = default;
            constexpr type_info(type_info&&) noexcept = default;
            constexpr type_info& operator=(const type_info&) noexcept = default;
            constexpr type_info& operator=(type_info&&) noexcept = default;

            /**
             * Return true if both instances are equal.
             *
             * @param rhs
             * @return
             * @see @ref type_info_identity "Identity"
             */
            constexpr bool operator==(const type_info& rhs) const noexcept {
                return &rhs == this ||                                 // fast: equal obj address
                       m_signature.data() == rhs.m_signature.data() || // fast: signature address comparison, which may fail on same types, _or_
                       ( m_hash_value == rhs.m_hash_value &&           // fast: wrong hash value -> false, otherwise avoid hash collision case ...
                         m_signature == rhs.m_signature                // slow: string comparison if !m_use_hash_only
                       );
            }

            constexpr bool operator!=(const type_info& rhs) const noexcept
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
            constexpr size_t hash_code() const noexcept { return m_hash_value; }

            /** Returns the type internal signature name, compiler implementation specific.  */
            constexpr std::string_view signature() const noexcept
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
     * Constructs a jau::type_info instance based on given type `T` at compile-time.
     *
     * @anchor make_ctti_type
     * This construction function uses [jau::ctti_name<T>()](@ref ctti_name_type).
     *
     * @tparam T type for which the type_info is generated
     * @return instance of jau::type_info
     * @see @ref ctti_name_type "jau::ctti_name<T>"
     */
    template<typename T>
    consteval jau::type_info make_ctti() noexcept {
        return jau::type_info(ctti_name<T>());
    }

    /**
     * Returns a static global reference of make_ctti<T>(true) w/ identity instance
     */
    template<typename T>
    const jau::type_info& static_ctti() noexcept {
        static const type_info sig = jau::type_info(ctti_name<T>());
        return sig;
    }

    /**
     * Constructs a jau::type_info instance based on given function types `R(*L)(A...)` at compile-time.
     *
     * @anchor make_ctti_lambda
     * This construction function uses [jau::ctti_name<R, L, A...>()](@ref ctti_name_lambda).
     *
     * @tparam R function return type used for type_info in case of jau::type_info::rtti_available == false
     * @tparam L main function type for which the type_info is generated, e.g. a lambda type.
     * @tparam A function argument types used for type_info in case of jau::type_info::rtti_available == false
     * @return instance of jau::type_info
     * @see @ref ctti_name_lambda "jau::ctti_name<R, L, A...>"
     */
    template<typename R, typename L, typename...A>
    consteval jau::type_info make_ctti() noexcept {
        return jau::type_info(ctti_name<R, L, A...>());
    }

    /**
     * Returns a static global reference of make_ctti<R, L, A...>() w/ identity instance
     */
    template<typename R, typename L, typename...A>
    const jau::type_info& static_ctti() noexcept {
        static const type_info sig = jau::type_info(ctti_name<R, L, A...>());
        return sig;
    }

    /**@}*/

} // namespace jau

// injecting specialization of std::hash to namespace std of our types above
namespace std
{
    /** \addtogroup CppLang
     *
     */

    template<> struct hash<jau::type_info> {
        std::size_t operator()(jau::type_info const& a) const noexcept {
            return a.hash_code();
        }
    };

    /**@}*/
}

#endif /* JAU_TYPE_INFO_HPP_ */
