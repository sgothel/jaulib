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
#include <type_traits>

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
