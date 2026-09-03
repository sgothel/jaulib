/*
 * Multiple Authors: firda (post @ stackoverflow),
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

#include <tuple>
#include <type_traits>

namespace jau {

    /** \addtogroup CppLang
     *
     *  @{
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

    /**@}*/

} // namespace jau

/** \example test_type_traits_queries01.cpp
 * This C++ unit test validates the jau::has_toString and other type traints queries
 */

#endif /* JAU_TYPE_TRAITS_QUERIES_HPP_ */
