/*
 * Multiple Authors: Roman Perepelitsa (comp.lang.c++.moderated),
 *                   firda (post @ stackoverflow),
 *                   Sven Gothel <sgothel@jausoft.com>
 *
 * Editor: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021 Gothel Software e.K.
 * Copyright (c) 2021 The Authors (see above)
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

#ifndef JAU_TYPE_TRAITS_QUERIES_HPP_
#define JAU_TYPE_TRAITS_QUERIES_HPP_

#include <cstring>
#include <string>
#include <cstdint>
#include <type_traits>

namespace jau {

    // #include "jau/string_util.hpp"
    std::string format_string(const char* format, ...);

    /** \addtogroup CppLang
     *
     *  @{
     */

    // Author: Sven Gothel

    /**
     * Enumerating the different groups of type traits
     */
    enum class TypeTraitGroup : uint8_t {
        NONE                    = 0b00000000,/**< NONE */
        PRIMARY_TYPE_CAT        = 0b00000001,/**< PRIMARY_TYPE_CAT */
        TYPE_PROPERTIES         = 0b00000010,/**< TYPE_PROPERTIES */
        COMPOSITE_TYPE_CAT      = 0b00000100,/**< COMPOSITE_TYPE_CAT */
        SUPPORTED_OPERATIONS    = 0b00001000,/**< SUPPORTED_OPERATIONS */
        ALL                     = 0b11111111,/**< ALL */
    };
    constexpr uint8_t number(const TypeTraitGroup rhs) noexcept {
        return static_cast<uint64_t>(rhs);
    }
    constexpr TypeTraitGroup operator ^(const TypeTraitGroup lhs, const TypeTraitGroup rhs) noexcept {
        return static_cast<TypeTraitGroup> ( number(lhs) ^ number(rhs) );
    }
    constexpr TypeTraitGroup operator |(const TypeTraitGroup lhs, const TypeTraitGroup rhs) noexcept {
        return static_cast<TypeTraitGroup> ( number(lhs) | number(rhs) );
    }
    constexpr TypeTraitGroup operator &(const TypeTraitGroup lhs, const TypeTraitGroup rhs) noexcept {
        return static_cast<TypeTraitGroup> ( number(lhs) & number(rhs) );
    }
    constexpr bool operator ==(const TypeTraitGroup lhs, const TypeTraitGroup rhs) noexcept {
        return number(lhs) == number(rhs);
    }
    constexpr bool operator !=(const TypeTraitGroup lhs, const TypeTraitGroup rhs) noexcept {
        return !( lhs == rhs );
    }
    constexpr bool isTypeTraitBitSet(const TypeTraitGroup mask, const TypeTraitGroup bit) noexcept {
        return TypeTraitGroup::NONE != ( mask & bit );
    }

    /**
    // *************************************************
    // *************************************************
    // *************************************************
     */

    // Author: Sven Gothel

    /**
     * Helper, allowing simple access and provision of a typename string representation
     * at compile time, see jau::type_cue for usage.
     * <p>
     * You may use the macro <code>JAU_TYPENAME_CUE(TYPENAME)</code> to set a single type
     * or maybe <code>JAU_TYPENAME_CUE_ALL(TYPENAME)</code> to set the single type and
     * all pointer and reference variations (mutable and const).
     * </p>
     * <p>
     * Without user override, implementation will use <code>typeid(T).name()</code>.
     * </p>
     * @tparam T the typename to name
     * @see jau::type_cue
     */
    template <typename T>
    struct type_name_cue
    {
        /**
         * Return the string representation of this type.
         * <p>
         * This might be a compile time user override, see jau::type_name_cue.
         * </p>
         * <p>
         * If no user override has been provides, the default implementation
         * either returns <code>typeid(T).name()</code> if RTTI is enabled
         * or <code>"unnamed_type"</code> if RTTI is disabled.
         * </p>
         */
        static const char * name() noexcept {
#if defined(__cxx_rtti_available__)
            return typeid(T).name();
#else
            return "unnamed_type";
#endif
        }
    };
    /**
     * Helper, allowing simple access to compile time typename and <i>Type traits</i> information,
     * see jau::type_name_cue to setup typename's string representation.
     *
     * @tparam T the typename to introspect
     * @see jau::type_name_cue
     */
    template <typename T>
    struct type_cue : public type_name_cue<T>
    {

        static std::string to_string(const bool withSize=true) noexcept {
            if( withSize ) {
                return jau::format_string("%s[%zu bytes]", type_name_cue<T>::name(), sizeof(T));
            } else {
                return type_name_cue<T>::name();
            }
        }

        /**
         * Print information of this type to stdout, potentially with all <i>Type traits</i> known.
         * @param stream output stream
         * @param typedefname the code typedefname (or typename) as a string, should match T
         * @param verbose if true, prints all <i>Type traits</i> known for this type. Be aware of the long output. Defaults to false.
         */
        static void fprint(std::FILE *stream, const std::string& typedefname, const TypeTraitGroup verbosity=TypeTraitGroup::NONE) {
            std::fprintf(stream, "Type: %s -> %s, %zu bytes\n", typedefname.c_str(), type_name_cue<T>::name(), sizeof(T));

            if( isTypeTraitBitSet(verbosity, TypeTraitGroup::PRIMARY_TYPE_CAT) ) {
                std::fprintf(stream, "  Primary Type Categories\n");
                std::fprintf(stream, "    void            %d\n", std::is_void_v<T>);
                std::fprintf(stream, "    null ptr        %d\n", std::is_null_pointer_v<T>);
                std::fprintf(stream, "    integral        %d\n", std::is_integral_v<T>);
                std::fprintf(stream, "    floating point  %d\n", std::is_floating_point_v<T>);
                std::fprintf(stream, "    array           %d\n", std::is_array_v<T>);
                std::fprintf(stream, "    enum            %d\n", std::is_enum_v<T>);
                std::fprintf(stream, "    union           %d\n", std::is_union_v<T>);
                std::fprintf(stream, "    class           %d\n", std::is_class_v<T>);
                std::fprintf(stream, "    function        %d\n", std::is_function_v<T>);
                std::fprintf(stream, "    pointer         %d\n", std::is_pointer_v<T>);
                std::fprintf(stream, "    lvalue ref      %d\n", std::is_lvalue_reference_v<T>);
                std::fprintf(stream, "    rvalue ref      %d\n", std::is_rvalue_reference_v<T>);
                std::fprintf(stream, "    member obj ptr  %d\n", std::is_member_object_pointer_v<T>);
                std::fprintf(stream, "    member func ptr %d\n", std::is_member_function_pointer_v<T>);
                std::fprintf(stream, "\n");
            }
            if( isTypeTraitBitSet(verbosity, TypeTraitGroup::TYPE_PROPERTIES) ) {
                std::fprintf(stream, "  Type Properties\n");
                std::fprintf(stream, "    const           %d\n", std::is_const_v<T>);
                std::fprintf(stream, "    volatile        %d\n", std::is_volatile_v<T>);
                std::fprintf(stream, "    trivial         %d\n", std::is_trivial_v<T>);
                std::fprintf(stream, "    trivially_copy. %d\n", std::is_trivially_copyable_v<T>);
                std::fprintf(stream, "    standard_layout %d\n", std::is_standard_layout_v<T>);
                std::fprintf(stream, "    pod             %d\n", std::is_standard_layout_v<T> && std::is_trivial_v<T>); // is_pod<>() is deprecated
                std::fprintf(stream, "    unique_obj_rep  %d\n", std::has_unique_object_representations_v<T>);
                std::fprintf(stream, "    empty           %d\n", std::is_empty_v<T>);
                std::fprintf(stream, "    polymorphic     %d\n", std::is_polymorphic_v<T>);
                std::fprintf(stream, "    abstract        %d\n", std::is_abstract_v<T>);
                std::fprintf(stream, "    final           %d\n", std::is_final_v<T>);
                std::fprintf(stream, "    aggregate       %d\n", std::is_aggregate_v<T>);
                std::fprintf(stream, "    signed          %d\n", std::is_signed_v<T>);
                std::fprintf(stream, "    unsigned        %d\n", std::is_unsigned_v<T>);
    #if __cplusplus > 202002L
                // C++ 23
                std::fprintf(stream, "    bounded_array   %d\n", std::is_bounded_array_v<T>);
                std::fprintf(stream, "    unbounded_array %d\n", std::is_unbounded_array_v<T>);
                std::fprintf(stream, "    scoped enum     %d\n", std::is_scoped_enum_v<T>);
    #endif
                std::fprintf(stream, "\n");
            }
            if( isTypeTraitBitSet(verbosity, TypeTraitGroup::COMPOSITE_TYPE_CAT) ) {
                std::fprintf(stream, "  Composite Type Categories\n");
                std::fprintf(stream, "    fundamental     %d\n", std::is_fundamental_v<T>);
                std::fprintf(stream, "    arithmetic      %d\n", std::is_arithmetic_v<T>);
                std::fprintf(stream, "    scalar          %d\n", std::is_scalar_v<T>);
                std::fprintf(stream, "    object          %d\n", std::is_object_v<T>);
                std::fprintf(stream, "    compound        %d\n", std::is_compound_v<T>);
                std::fprintf(stream, "    reference       %d\n", std::is_reference_v<T>);
                std::fprintf(stream, "    member ptr      %d\n", std::is_member_pointer_v<T>);
                std::fprintf(stream, "\n");
            }
            if( isTypeTraitBitSet(verbosity, TypeTraitGroup::SUPPORTED_OPERATIONS) ) {
                std::fprintf(stream, "  Supported Operations\n");
                std::fprintf(stream, "    constructible         %d {trivially %d, nothrow %d}\n",
                        std::is_constructible_v<T>,
                        std::is_trivially_constructible_v<T>, std::is_nothrow_constructible_v<T>);
                std::fprintf(stream, "    default_constructible %d {trivially %d, nothrow %d}\n",
                        std::is_default_constructible_v<T>,
                        std::is_trivially_default_constructible_v<T>, std::is_nothrow_default_constructible_v<T>);
                std::fprintf(stream, "    copy_constructible    %d {trivially %d, nothrow %d}\n",
                        std::is_copy_constructible_v<T>,
                        std::is_trivially_copy_constructible_v<T>, std::is_nothrow_copy_constructible_v<T>);
                std::fprintf(stream, "    move_constructible    %d {trivially %d, nothrow %d}\n",
                        std::is_move_constructible_v<T>,
                        std::is_trivially_move_constructible_v<T>, std::is_nothrow_move_constructible_v<T>);
                std::fprintf(stream, "    assignable            %d {trivially %d, nothrow %d}\n",
                        std::is_assignable_v<T, T>,
                        std::is_trivially_assignable_v<T, T>, std::is_nothrow_assignable_v<T, T>);
                std::fprintf(stream, "    copy_assignable       %d {trivially %d, nothrow %d}\n",
                        std::is_copy_assignable_v<T>,
                        std::is_trivially_copy_assignable_v<T>, std::is_nothrow_copy_assignable_v<T>);
                std::fprintf(stream, "    move_assignable       %d {trivially %d, nothrow %d}\n",
                        std::is_move_assignable_v<T>,
                        std::is_trivially_move_assignable_v<T>, std::is_nothrow_move_assignable_v<T>);
                std::fprintf(stream, "    destructible          %d {trivially %d, nothrow %d, virtual %d}\n",
                        std::is_destructible_v<T>,
                        std::is_trivially_destructible_v<T>, std::is_nothrow_destructible_v<T>,
                        std::has_virtual_destructor_v<T>);
                std::fprintf(stream, "    swappable             %d {nothrow %d}\n",
                        std::is_swappable_v<T>, std::is_nothrow_swappable_v<T>);
            }
        }
        /**
         * Print information of this type to stdout, potentially with all <i>Type traits</i> known.
         * @param typedefname the code typedefname (or typename) as a string, should match T
         * @param verbose if true, prints all <i>Type traits</i> known for this type. Be aware of the long output. Defaults to false.
         */
        static void print(const std::string& typedefname, const TypeTraitGroup verbosity=TypeTraitGroup::NONE) {
            fprint(stdout, typedefname, verbosity);
        }

    };
    #define JAU_TYPENAME_CUE(A) template<> struct jau::type_name_cue<A> { static const char * name() { return #A; } };
    #define JAU_TYPENAME_CUE_ALL(A) JAU_TYPENAME_CUE(A) JAU_TYPENAME_CUE(A*) JAU_TYPENAME_CUE(const A*) JAU_TYPENAME_CUE(A&) JAU_TYPENAME_CUE(const A&) // NOLINT(bugprone-macro-parentheses)

    /**
    // *************************************************
    // *************************************************
    // *************************************************
     */

    // Author: Sven Gothel

    /**
     * <code>template< class T > is_container_memmove_compliant<T>::value</code> compile-time Type Trait,
     * determining whether the given template class claims to be container memmove compliant, see @Ref darray_memmove.
     */
    template< class, class = void >
    struct is_container_memmove_compliant : std::false_type { };

    /**
     * <code>template< class T > is_container_memmove_compliant<T>::value</code> compile-time Type Trait,
     * determining whether the given template class claims to be container memmove compliant, see @Ref darray_memmove.
     */
    template< class T >
    struct is_container_memmove_compliant<T, std::void_t<typename T::container_memmove_compliant>> : T::container_memmove_compliant { };

    template <typename T> inline constexpr bool is_container_memmove_compliant_v = is_container_memmove_compliant<T>::value;

    /**
     * <code>template< class T > is_enforcing_secmem<T>::value</code> compile-time Type Trait,
     * determining whether the given template class enforces secmem, see @Ref darray_secmem.
     */
    template< class, class = void >
    struct is_enforcing_secmem : std::false_type { };

    /**
     * <code>template< class T > is_enforcing_secmem<T>::value</code> compile-time Type Trait,
     * determining whether the given template class enforces secmem, see @Ref darray_secmem.
     */
    template< class T >
    struct is_enforcing_secmem<T, std::void_t<typename T::enforce_secmem>> : T::enforce_secmem { };

    template <typename T> inline constexpr bool is_enforcing_secmem_v = is_enforcing_secmem<T>::value;

    template<typename... Ts>
    using first_type = std::tuple_element_t<0, std::tuple<Ts...>>;

    template<typename... Ts>
    using is_all_same = std::conjunction<std::is_same<first_type<Ts...>, Ts>...>;

    template<typename... Ts>
    inline constexpr bool is_all_same_v = is_all_same<Ts...>::value; // NOLINT(modernize-type-traits)

    /**
    // *************************************************
    // *************************************************
    // *************************************************
     */

    // Author: firda @ stackoverflow (posted the following there)
    // Location: https://stackoverflow.com/questions/1966362/sfinae-to-check-for-inherited-member-functions/25448020#25448020
#if 0
    /// Checker for typedef with given name and convertible type
    #define TYPEDEF_CHECKER(checker, name) \
    template<class C, typename T, typename = void> struct checker : std::false_type {}; \
    template<class C, typename T> struct checker<C, T, typename std::enable_if_t< \
      std::is_convertible_v<typename C::name, T>>> : std::true_type {}

    /// Checker for typedef with given name and exact type
    #define TYPEDEF_CHECKER_STRICT(checker, name) \
    template<class C, typename T, typename = void> struct checker : std::false_type {}; \
    template<class C, typename T> struct checker<C, T, typename std::enable_if_t< \
      std::is_same_v<typename C::name, T>>> : std::true_type {}

    /// Checker for typedef with given name and any type
    #define TYPEDEF_CHECKER_ANY(checker, name) \
    template<class C, typename = void> struct checker : std::false_type {}; \
    template<class C> struct checker<C, typename std::enable_if_t< \
      !std::is_same_v<typename C::name*, void>>> : std::true_type {}

    /// Checker for static const variable with given name and value
    #define MVALUE_CHECKER(checker, name, val) \
    template<class C, typename = void> struct checker : std::false_type {}; \
    template<class C> struct checker<C, typename std::enable_if_t< \
      std::is_convertible_v<decltype(C::name), const decltype(val)> && C::name == val>> : std::true_type {}
    /// Checker for static const variable with given name, value and type
    #define MVALUE_CHECKER_STRICT(checker, name, val) \
    template<class C, typename = void> struct checker : std::false_type {}; \
    template<class C> struct checker<C, typename std::enable_if_t< \
      std::is_same_v<decltype(C::name), const decltype(val)> && C::name == val>> : std::true_type {}
#endif

    /// Checker for member with given name and convertible type
    #define MTYPE_CHECKER(checker, name) \
    template<class C, typename T, typename = void> struct checker : std::false_type {}; \
    template<class C, typename T> struct checker<C, T, typename std::enable_if_t< \
      std::is_convertible_v<decltype(C::name), T>>> : std::true_type {}

    /// Checker for member with given name and exact type
    #define MTYPE_CHECKER_STRICT(checker, name) \
    template<class C, typename T, typename = void> struct checker : std::false_type {}; \
    template<class C, typename T> struct checker<C, T, typename std::enable_if_t< \
      std::is_same_v<decltype(C::name), T>>> : std::true_type {}

    /// Checker for member with given name and any type
    #define MTYPE_CHECKER_ANY(checker, name) \
    template<class C, typename = void> struct checker : std::false_type {}; \
    template<class C> struct checker<C, typename std::enable_if_t< \
      !std::is_same_v<decltype(C::name)*, void>>> : std::true_type {}

    /// Checker for member function with convertible return type and accepting given arguments
    #define METHOD_CHECKER(checker, name, ret, args) \
    template<class C, typename=void> struct checker : std::false_type {}; \
    template<class C> struct checker<C, typename std::enable_if_t< \
      std::is_convertible_v<decltype(std::declval<C>().name args), ret>>> : std::true_type {}; // NOLINT(bugprone-macro-parentheses)

    /// Checker for member function with exact retutn type and accepting given arguments
    #define METHOD_CHECKER_STRICT_RET(name, fn, ret, args) \
    template<class C, typename=void> struct name : std::false_type {}; \
    template<class C> struct name<C, typename std::enable_if_t< \
      std::is_same_v<decltype(std::declval<C>().fn args), ret>>> : std::true_type {};  // NOLINT(bugprone-macro-parentheses)

    /// Checker for member function accepting given arguments
    #define METHOD_CHECKER_ANY(name, fn, args) \
    template<class C, typename=void> struct name : std::false_type {}; \
    template<class C> struct name<C, typename std::enable_if_t< \
      !std::is_same_v<decltype(std::declval<C>().fn args)*, void>>> : std::true_type {};

    METHOD_CHECKER(has_toString, toString, std::string, ())
    template <typename _Tp> inline constexpr bool has_toString_v = has_toString<_Tp>::value;

    METHOD_CHECKER(has_to_string, to_string, std::string, ())
    template <typename _Tp> inline constexpr bool has_to_string_v = has_to_string<_Tp>::value;

    // Author: Sven Gothel

    /// Checker for member of pointer '->' operator with convertible pointer return, no arguments
    template<class C, typename=void> struct has_member_of_pointer : std::false_type {};
    template<class C> struct has_member_of_pointer<C, typename std::enable_if_t<
      std::is_pointer_v<decltype(std::declval<C>().operator->())>>> : std::true_type {};

    template <typename _Tp> inline constexpr bool has_member_of_pointer_v = has_member_of_pointer<_Tp>::value;

    /**@}*/

} // namespace jau

/** \example test_type_traits_queries01.cpp
 * This C++ unit test validates the jau::has_toString and other type traints queries
 */

#endif /* JAU_TYPE_TRAITS_QUERIES_HPP_ */
