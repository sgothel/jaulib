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

#ifndef JAU_TYPE_INFO_HPP_
#define JAU_TYPE_INFO_HPP_

#include <jau/cpp_lang_util.hpp>
#include <jau/enum_util.hpp>
#include <typeinfo>
#include <cstdint>

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

    enum class type_info_flags_t : uint16_t {
        none = 0,
        obj = 1 << 0,
        sig = 1 << 1,
    };
    JAU_MAKE_BITFIELD_ENUM_STRING(type_info_flags_t, obj, sig);

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
            type_info_flags_t m_idflags;

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
              m_idflags(type_info_flags_t::sig)
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
              m_idflags(identity_instance ? type_info_flags_t::obj : type_info_flags_t::none) // RTTI type_info can't guarantee identity for name()
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
              m_idflags( type_info_flags_t::none )
            {
                jau::enums::write(m_idflags, type_info_flags_t::obj, identity_instance);
                jau::enums::write(m_idflags, type_info_flags_t::sig, identity_signature);
                abort_invalid(m_signature);
            }

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
                return ( !identInst() || !rhs.identInst() ) &&           // fast: fail if both instances use an identity obj address
                       ( m_signature == rhs.m_signature ||               // fast: signature address comparison, which may fail on same types, _or_
                         ( ( !identName() || !rhs.identName() ) &&       // fast: fail if both instances use an identity signature address
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
            constexpr size_t hash_code() const noexcept { return m_hash_value; }

            /** Returns true if this instance has a unique address (for same type_info). */
            constexpr bool identInst() const noexcept { return jau::enums::is_set(m_idflags, type_info_flags_t::obj); }

            /** Returns true if internal_name() has a unique identity address (for same signature strings). */
            constexpr bool identName() const noexcept { return jau::enums::is_set(m_idflags, type_info_flags_t::sig); }

            /** Returns the type name, compiler implementation specific.  */
            constexpr const char* internal_name() const noexcept
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

    /**@}*/

} // namespace jau

#endif /* JAU_TYPE_INFO_HPP_ */
