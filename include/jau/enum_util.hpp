/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright Gothel Software e.K.
 *
 * SPDX-License-Identifier: MIT
 *
 * This Source Code Form is subject to the terms of the MIT License
 * If a copy of the MIT was not distributed with this
 * file, You can obtain one at https://opensource.org/license/mit/.
 */

#ifndef JAU_ENUM_UTIL_HPP_
#define JAU_ENUM_UTIL_HPP_

#include <type_traits>
#include <string_view>
#include <cstring>
#include <ostream>
#include <jau/cpp_lang_util.hpp>

/**
 * Provides scoped enum type support functionality, including `to_string`,
 * enum bitfield operations, etc.
 *
 * Due to lack of C++26 reflection,
 * basic enum function definition is performed via macros to access value names outside constexpr templates.
 *
 * Overview
 * - Define your `enum class` manually
 * - Use one of the following `enum class` helper function generator
 *   - `JAU_MAKE_ENUM_IMPL` for non-bitfield enum values
 *   - `JAU_MAKE_BITFIELD_ENUM_IMPL` for bitfield enum values
 * - Following methods are made available via the generator,
 *   assuming `E` is the enum class type and `U` the underlying type,
 *   - `constexpr std::string_view enum_longname(const E v) noexcept`
 *     - returns a long string, e.g. `E::value`
 *   - `constexpr std::string_view enum_name(const E v) noexcept`
 *     - returns a short string, e.g. `value`
 *   - `constexpr std::string to_string(const E v)`
 *     - returns a short string, e.g. `value`.
 *     - if generated via `JAU_MAKE_BITFIELD_ENUM_IMPL`, returns all set bit-values using names like `[cat, mouse]`
 *   - `constexpr size_t E_count() noexcept`
 *     - returns number of enum names
 * - General support `constexpr` template functions are available in the `jau::enums` namespace,
 *   assuming `E` is the enum class type, `v` an enum value and `U` the underlying type,
 *   - `constexpr U number(const E v) noexcept`
 *     - returns the integral underlying value
 *   - `constexpr bool is_set(const E mask, const E bits) noexcept`
 *     - returns `true` if mask contains given `bits`, otherwise `false`
 *   - `constexpr void append_bitstr(std::string& out, E mask, E bit, const std::string& bitstr, bool& comma)`
 *     - appends `bitstr` to `out` if `mask` contains `bit`, prepends a comma if `comma` and sets `comma` to `true`.
 *   - Equality- and bit-operations, e.g.
 *     - `constexpr bool operator==(const E lhs, const E rhs) noexcept`
 *     - `constexpr E operator&(const E lhs, const E rhs) noexcept`
 *   - `constexpr std::string_view enum_longname<auto v>() noexcept`
 *     - returns a long string, e.g. `E::value`
 *   - `constexpr std::string_view enum_name<auto v>() noexcept`
 *     - returns a short string, e.g. `value`
 *   - `get_names`, `get_values`
*/
namespace jau::enums {

    /** \addtogroup CppLang
     *
     *  @{
     */

    ///
    /// clang + gcc
    ///

    template<typename E, E V>
    consteval_cxx20 const char* enum_funcname() noexcept {
        // const char *enum_funcname() [E = test_type1_t, V = test_type1_t::one]
        return JAU_PRETTY_FUNCTION;
    }

    template <auto V,
              std::enable_if_t<std::is_enum_v<decltype(V)>>* = nullptr>
    consteval_cxx20 bool is_enum() noexcept {
      // bool is_name() [E = test_type_t, V = test_type_t::one]
      constexpr std::string_view name(enum_funcname<decltype(V), V>());

      size_t i = name.rfind(' ');
      if ( std::string_view::npos == i || i+1 == name.length() ) {
        return false;
      }
      ++i;
      const char c = name[i];
      return !(c >= '0' && c <= '9'); // true if character, not-a-number
    }

    template <auto V,
              std::enable_if_t<std::is_enum_v<decltype(V)>>* = nullptr>
    consteval_cxx20 std::string_view enum_longname() noexcept {
      // const char *enum_funcname() [E = test_type1_t, V = test_type1_t::one]
      constexpr std::string_view name(enum_funcname<decltype(V), V>());
      size_t i = name.rfind(' ');
      if ( std::string_view::npos == i || i+1 == name.length() ) {
        return name;
      }
      ++i;
      const char c = name[i];
      if ( c >= '0' && c <= '9' ) {
        return name;
      }
      if constexpr ( name[name.length() - 1] == ']' ) {
        return name.substr(i, name.length()-i-1);
      }
      return name.substr(i);
    }

    template <auto V,
              std::enable_if_t<std::is_enum_v<decltype(V)>>* = nullptr>
    consteval_cxx20 std::string_view enum_name() noexcept {
      // const char *enum_funcname() [E = test_type1_t, V = test_type1_t::one]
      constexpr std::string_view name(enum_funcname<decltype(V), V>());
      // Find the final space character in the pretty name.
      size_t i = name.rfind(' ');
      if ( std::string_view::npos == i || i+1 == name.length() ) {
        return name;
      }
      ++i;
      const char c = name[i];
      if ( c >= '0' && c <= '9' ) {
        return name;
      }

      size_t j = name.find("::", i);
      if ( std::string_view::npos == j || j+2 >= name.length() ) {
        return name;
      }
      j+=2;
      if constexpr ( name[name.length() - 1] == ']' ) {
        return name.substr(j, name.length()-j-1);
      }
      return name.substr(j);
    }

    template <auto... Vargs>
    struct CountArgs {
        static constexpr size_t value = sizeof...(Vargs);
    };

    template <auto... Vargs>
    struct NameTable {
        std::string_view names[sizeof...(Vargs)];
    };
    template <auto... Vargs>
    consteval_cxx20 NameTable<Vargs...>
    get_names() noexcept {
        return { { enum_longname<Vargs>()... } };
    }

    template <typename E, size_t N>
    struct ValueTable {
        E values[N];
    };
    template <typename... Args>
    constexpr ValueTable<std::common_type_t<Args...>, sizeof...(Args)>
    get_values(Args... args) noexcept {
        return { { args... } };
    }

    template <typename E, std::enable_if_t<std::is_enum_v<E>>* = nullptr>
    constexpr std::underlying_type_t<E>
    number(const E v) noexcept { return static_cast<std::underlying_type_t<E>>(v); }

    template<typename E, std::enable_if_t<std::is_enum_v<E>>* = nullptr>
    constexpr E operator~(const E rhs) noexcept {
        return static_cast<E>(~number(rhs));
    }

    template<typename E, std::enable_if_t<std::is_enum_v<E>>* = nullptr>
    constexpr E operator^(const E lhs, const E rhs) noexcept {
        return static_cast<E>(number(lhs) ^ number(rhs));
    }

    template<typename E, std::enable_if_t<std::is_enum_v<E>>* = nullptr>
    constexpr E operator|(const E lhs, const E rhs) noexcept {
        return static_cast<E>(number(lhs) | number(rhs));
    }

    template<typename E, std::enable_if_t<std::is_enum_v<E>>* = nullptr>
    constexpr E operator&(const E lhs, const E rhs) noexcept {
        return static_cast<E>(number(lhs) & number(rhs));
    }

    template<typename E, std::enable_if_t<std::is_enum_v<E>>* = nullptr>
    constexpr E& operator|=(E& lhs, const E rhs) noexcept {
        return lhs = lhs | rhs;
    }

    template<typename E, std::enable_if_t<std::is_enum_v<E>>* = nullptr>
    constexpr E& operator&=(E& lhs, const E rhs) noexcept {
        return lhs = lhs & rhs;
    }

    template<typename E, std::enable_if_t<std::is_enum_v<E>>* = nullptr>
    constexpr E& operator^=(E& lhs, const E rhs) noexcept {
        return lhs = lhs ^ rhs;
    }

    template<typename E, std::enable_if_t<std::is_enum_v<E>>* = nullptr>
    constexpr bool operator==(const E lhs, const E rhs) noexcept {
        return number(lhs) == number(rhs);
    }

    template<typename E, std::enable_if_t<std::is_enum_v<E>>* = nullptr>
    constexpr bool operator!=(const E lhs, const E rhs) noexcept {
        return !(lhs == rhs);
    }

    template<typename E, std::enable_if_t<std::is_enum_v<E>>* = nullptr>
    constexpr bool is_set(const E mask, const E bits) noexcept {
        return bits == (mask & bits);
    }

    template<typename E, std::enable_if_t<std::is_enum_v<E>>* = nullptr>
    constexpr bool has_any(const E mask, const E bits) noexcept {
        return std::underlying_type_t<E>(0) != ( number(mask) & number(bits) );
    }

    template<typename E, std::enable_if_t<std::is_enum_v<E>>* = nullptr>
    constexpr void append_bitstr(std::string& out, E mask, E bit, const std::string& bitstr, bool& comma) {
        if( bit == (mask & bit) ) {
            if( comma ) { out.append(", "); }
            out.append(bitstr); comma = true;
        }
    }

    template <typename T,
              std::enable_if_t<std::is_enum_v<T>>* = nullptr>
    inline std::ostream& operator<<(std::ostream& os, const T v) {
        os << enum_longname(v);
        return os;
    }

    /**@}*/

} // namespace jau::enums

/** \addtogroup CppLang
 *
 *  @{
 */

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

#define JAU_FOR_EACH(macro, type, ...)                              \
  __VA_OPT__(JAU_EXPAND(JAU_FOR_EACH_HELPER(macro, type, __VA_ARGS__)))
#define JAU_FOR_EACH_HELPER(macro, type, a1, ...)                   \
  macro(type, a1)                                               \
  __VA_OPT__(JAU_FOR_EACH_AGAIN JAU_PARENS (macro, type, __VA_ARGS__))
#define JAU_FOR_EACH_AGAIN() JAU_FOR_EACH_HELPER

#define JAU_ENUM_CASE_SHORT(type, name) case type::name: return #name;
#define JAU_ENUM_CASE_LONG(type, name)  case type::name: return #type "::" #name;

#define JAU_FOR_EACH_LIST(macro, type, ...)                              \
  __VA_OPT__(JAU_EXPAND(JAU_FOR_EACH_LIST_HELPER(macro, type, __VA_ARGS__)))
#define JAU_FOR_EACH_LIST_HELPER(macro, type, a1, ...)                   \
  macro(type, a1)                                               \
  __VA_OPT__(, JAU_FOR_EACH_LIST_AGAIN JAU_PARENS (macro, type, __VA_ARGS__))
#define JAU_FOR_EACH_LIST_AGAIN() JAU_FOR_EACH_LIST_HELPER

#define JAU_ENUM_TYPE_VALUE(type, name) type::name
#define JAU_ENUM_VALUE(type, name) name

#define JAU_FOR_EACH_VALUE(macro, type, value, ...)             \
  __VA_OPT__(JAU_EXPAND(JAU_FOR_EACH_VALUE_HELPER(macro, type, value, __VA_ARGS__)))
#define JAU_FOR_EACH_VALUE_HELPER(macro, type, value, a1, ...)  \
  macro(type, a1, value)                                        \
  __VA_OPT__(JAU_FOR_EACH_VALUE_AGAIN JAU_PARENS (macro, type, value, __VA_ARGS__))
#define JAU_FOR_EACH_VALUE_AGAIN() JAU_FOR_EACH_VALUE_HELPER

#define JAU_MAKE_ENUM_IMPL(type, ...)                       \
    JAU_MAKE_ENUM_IMPL_SUB(type, type, __VA_ARGS__)         \
                                                            \
    constexpr std::string                                   \
    to_string(const type e)                                 \
    { return std::string(enum_name(e)); }                   \

#define JAU_APPEND_BITSTR(U,V,M) jau::enums::append_bitstr(out, M, U::V, #V, comma);

#define JAU_MAKE_BITFIELD_ENUM_IMPL(type, ...)              \
    JAU_MAKE_ENUM_IMPL_SUB(type, type, __VA_ARGS__)         \
                                                            \
    inline std::string                                      \
    to_string(const type mask) {                            \
        std::string out("[");                               \
        bool comma = false;                                 \
        JAU_FOR_EACH_VALUE(JAU_APPEND_BITSTR, type, mask, __VA_ARGS__); \
        out.append("]");                                    \
        return out;                                         \
    }                                                       \

#define JAU_MAKE_BITFIELD_ENUM_IMPL2(type, stype, ...)      \
    JAU_MAKE_ENUM_IMPL_SUB(type, stype, __VA_ARGS__)        \
                                                            \
    inline std::string                                      \
    to_string(const type mask) {                            \
        std::string out("[");                               \
        bool comma = false;                                 \
        JAU_FOR_EACH_VALUE(JAU_APPEND_BITSTR, type, mask, __VA_ARGS__); \
        out.append("]");                                    \
        return out;                                         \
    }                                                       \

// internal usage only
#define JAU_MAKE_ENUM_IMPL_SUB(type, stype, ...)            \
    constexpr std::string_view                              \
    enum_longname(const type v) noexcept                    \
    {                                                       \
        switch (v) {                                        \
            JAU_FOR_EACH(JAU_ENUM_CASE_LONG, type, __VA_ARGS__) \
            default:                                        \
                return "undef";                             \
        }                                                   \
    }                                                       \
    constexpr std::string_view                              \
    enum_name(const type v) noexcept                        \
    {                                                       \
        switch (v) {                                        \
            JAU_FOR_EACH(JAU_ENUM_CASE_SHORT, type, __VA_ARGS__) \
            default:                                        \
                return "undef";                             \
        }                                                   \
    }                                                       \
                                                            \
    constexpr size_t stype##_count() noexcept {             \
        return jau::enums::CountArgs<JAU_FOR_EACH_LIST(JAU_ENUM_TYPE_VALUE, type, __VA_ARGS__)>::value; \
    }                                                       \


/**@}*/

#endif /* JAU_ENUM_UTIL_HPP_ */
