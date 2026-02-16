/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2025 Gothel Software e.K.
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

#ifndef JAU_TYPE_CONCEPTS_HPP_
#define JAU_TYPE_CONCEPTS_HPP_

#include <concepts>
#include <string>
#include <type_traits>
#include <jau/cpp_lang_util.hpp>
#include <jau/string_literal.hpp>
#include <jau/type_traits_queries.hpp>

/** Requirement (concept) Definitions */
namespace jau::req {

    /** \addtogroup CppLang
     *
     *  @{
     */

    /** Concept of type-trait std::is_pointer */
    template<typename T>
    concept pointer = std::is_pointer_v<T>;

    /** Concept of type-trait std::is_standard_layout */
    template<typename T>
    concept standard_layout = std::is_standard_layout_v<T>;

    /** Concept of type-trait std::is_trivially_copyable */
    template<typename T>
    concept trivially_copyable = std::is_trivially_copyable_v<T>;

    /** Concept of type-trait for `bool` type */
    template<typename T>
    concept boolean = std::is_same_v<bool, std::remove_cv_t<T>>;

    /** Concept of type-trait std::is_arithmetic */
    template<typename T>
    concept arithmetic = std::is_arithmetic_v<T>;

    /** Concept of type-trait std::is_unsigned and std::is_arithmetic */
    template<typename T>
    concept unsigned_arithmetic = std::is_arithmetic_v<T> && std::is_unsigned_v<T>;

    /** Concept of type-trait std::is_signed and std::is_arithmetic */
    template<typename T>
    concept signed_arithmetic = std::is_arithmetic_v<T> && std::is_signed_v<T>;

    /** Concept of type-trait std::is_unsigned and std::is_integral */
    template<typename T>
    concept unsigned_integral = std::is_integral_v<T> && std::is_unsigned_v<T>;

    /** Concept of type-trait std::is_signed and std::is_integral */
    template<typename T>
    concept signed_integral = std::is_integral_v<T> && std::is_signed_v<T>;

    /** Concept of type-trait std::is_integral and sizeof(T) == alignof(T) (packed) */
    template<typename T>
    concept packed_integral = std::is_integral_v<T> && sizeof(T) == alignof(T);

    /** Concept of type-trait std::is_floating_point and sizeof(T) == alignof(T) (packed) */
    template <typename T>
    concept packed_floating_point = std::is_floating_point_v<T> && sizeof(T) == alignof(T);

    /** Returns underlying pointer type w/o constant'ness */
    template<class T>
    requires pointer<T>
    using underlying_pointer_type = std::remove_const_t<std::remove_pointer_t<std::remove_cv_t<T>>>;

    /** Returns base pointer type w/o constant'ness in pointer nor value */
    template<class T>
    requires pointer<T>
    using base_pointer = std::add_pointer_t<underlying_pointer_type<T>>;

    /** Returns all const pointer type w/ constant'ness in pointer and value */
    template<class T>
    requires pointer<T>
    using const2_pointer = std::add_const_t<std::add_pointer_t<std::add_const_t<underlying_pointer_type<T>>>>;

    /// A `char*`
    template<typename T>
    concept char_pointer = std::is_same_v<char*, base_pointer<T>>;

    /**
     * A string literal: `char (&)[N]`, jau::StringLiteral
     */
    template<typename T>
    concept string_literal = std::constructible_from<decltype(jau::StringLiteral(T{})), T>
                          && (!std::is_integral_v<std::remove_reference_t<T>>)
                          && (!std::is_floating_point_v<std::remove_reference_t<T>>)
                          && (!std::is_enum_v<std::remove_reference_t<T>>);

    /// A std::string
    template<typename T>
    concept string_type = std::is_base_of_v<std::string, std::remove_cv_t<std::remove_reference_t<T>>>;

    /// A string class, i.e. std::string, std::string_view or jau::StringLiteral
    template<typename T>
    concept string_class = string_type<T>
                        || std::is_base_of_v<std::string_view, std::remove_cv_t<std::remove_reference_t<T>>>
                        || std::is_same_v<decltype(jau::StringLiteral(T{})), T>;

    /**
     * Like a string:
     * - string literal:
     *   - `CharT (&)[N]`
     *   - jau::StringLiteral
     * - string_class:
     *   - std::string<CharT>
     *   - std::string_view<CharT>
     * - char_pointer: `char*`
     */
    template<typename T>
    concept string_alike = string_literal<T> || string_class<T> || char_pointer<T>;

    template<typename T>
    concept string_alike0 = string_literal<T> || string_class<T>;

    /**
     * A strict convertible type to `std::string` or `std::string_view` via `jau::to_string(T)` or `to_string(T)` (custom free function)
     * - has member `toString()`
     * - has member `to_string()`
     * - has free function `to_string(T)`
     */
    template<typename T>
    concept string_convertible0_jau = jau::has_toString_v<T> ||
                                      jau::has_to_string_v<T> ||
                                      jau::has_free_to_string_v<T>;

    /**
     * A loose convertible type to `std::string` or `std::string_view` via `jau::to_string(T)` or `to_string(T)` (custom free function)
     * - integral
     * - floating_point
     * - pointer
     *   - including has member function `operator->()`
     * - string_convertible0_jau
     *   - has member `toString()`
     *   - has member `to_string()`
     *   - has free function `to_string(T)`
     *
     * Convertible to string via `std::to_string(T)` or `jau::to_string(T)`
     */
    template<typename T>
    concept string_convertible1_jau = std::is_integral_v<T> ||
                                      std::is_floating_point_v<T> ||
                                      std::is_pointer_v<T> ||
                                      jau::has_member_of_pointer_v<T> ||
                                      string_convertible0_jau<T>;

    /**
     * A convertible type to `std::string` or a `std::string` itself.
     * - string_alike: std::string, std::string_view, jau::StringLiteral, `CharT (&)[N]`, `char*`
     * - integral
     * - floating_point
     *
     * Convertible to string via `std::to_string(T)` or `jau::to_string(T)`
     */
    template<typename T>
    concept stringifyable_std = string_alike<T>
                             || std::is_integral_v<T>
                             || std::is_floating_point_v<T>;

    /**
     * A strict convertible type to `std::string`, `std::string_view` or a `std::string` itself.
     * - string_alike: std::string, std::string_view, jau::StringLiteral, `char (&)[N]`, `char*`
     * - string_convertible0_jau
     *   - has member `toString()`
     *   - has member `to_string()`
     *   - has free function `to_string(T)`
     *
     * Convertible to string via `jau::to_string(T)` or `to_string(T)` (custom free function)
     */
    template<typename T>
    concept stringifyable0_jau = string_alike<T>
                              || string_convertible0_jau<T>;

    /**
     * A loose convertible type to `std::string`, `std::string_view` or a `std::string` itself.
     * - string_alike: std::string, std::string_view, jau::StringLiteral, `char (&)[N]`, `char*`
     * - integral
     * - floating_point
     * - string_convertible1_jau
     *   - integral
     *   - floating_point
     *   - pointer
     *     - including has member function `operator->()`
     *   - string_convertible0_jau
     *     - has member `toString()`
     *     - has member `to_string()`
     *     - has free function `to_string(T)`
     *
     * Convertible to string via `jau::to_string(T)` or `to_string(T)` (custom free function)
     */
    template<typename T>
    concept stringifyable1_jau = stringifyable_std<T>
                              || string_convertible1_jau<T>;

    /** C++ Named Requirement Container (partial) */
    template<typename T>
    concept container = requires(T t) {
        typename T::value_type;
        typename T::reference;
        typename T::const_reference;
        typename T::iterator;
        typename T::const_iterator;
        typename T::difference_type;
        typename T::size_type;

        { t.begin() } -> std::same_as<typename T::iterator>;
        { t.end() } -> std::same_as<typename T::iterator>;
        { t.size() } -> std::same_as<typename T::size_type>;
    };

    /** Query whether type is a C++ Named Requirement Container (partial) */
    template<typename T>
    requires container<T>
    constexpr bool is_container() { return true; }

    /** Query whether type is a C++ Named Requirement Container (partial) */
    template<typename T>
    requires (!container<T>)
    constexpr bool is_container() { return false; }

    /** C++ Named Requirement ContiguousContainer (partial) */
    template<typename T>
    concept contiguous_container = container<T> && requires(T t) {
        typename T::pointer;
        typename T::const_pointer;

        { t.data() } -> std::same_as<typename T::pointer>;
    };

    /** Query whether type is a C++ Named Requirement ContiguousContainer (partial) */
    template<typename T>
    requires contiguous_container<T>
    constexpr bool is_contiguous_container() { return true; }

    /** Query whether type is a C++ Named Requirement ContiguousContainer (partial) */
    template<typename T>
    requires (!contiguous_container<T>)
    constexpr bool is_contiguous_container() { return false; }

    /**@}*/

} // namespace jau::req


/** \example test_type_concepts01.cpp
 * This C++ unit test validates the jau::req concepts
 */

#endif /* JAU_TYPE_CONCEPTS_HPP_ */
