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

#include <array>
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
 *   - `JAU_MAKE_ENUM_STRING` for non-bitfield enum values
 *   - `JAU_MAKE_BITFIELD_ENUM_STRING` for bitfield enum values
 *   - `JAU_MAKE_ENUM_INFO` for typedef `E_info_t` with E enum class type using `enum_info<E, ...>`
 *     -  `enum_info<E, ...>` provides iterator and further basic information for `E`
 * - Following methods are made available via the generator `JAU_MAKE_ENUM_STRING`,
 *   assuming `E` is the enum class type
 *   - `constexpr std::string_view long_name(const E v) noexcept`
 *     - returns a long string, e.g. `E::value`
 *   - `constexpr std::string_view name(const E v) noexcept`
 *     - returns a short string, e.g. `value`
 *   - `constexpr std::string to_string(const E v)`
 *     - returns a short string, e.g. `value`.
 * - Following methods are made available via the generator `JAU_MAKE_BITFIELD_ENUM_STRING`
 *   - all methods from `JAU_MAKE_ENUM_STRING`, while `to_string(const E v)` returns the set bit values, e.g. `[cat, mouse]`
 * - General support `constexpr` template functions are available in the `jau::enums` namespace,
 *   assuming `E` is the enum class type, `v` an enum value and `U` the underlying type
 *   - `constexpr U number(const E v) noexcept`
 *     - returns the integral underlying value
 *   - `constexpr U operator*(const E v) noexcept`
 *     - this dereferencing overload operator also returns the integral underlying value like `number`
 *   - `constexpr bool is_set(const E mask, const E bits) noexcept`
 *     - returns `true` if mask contains given `bits`, otherwise `false`
 *   - `constexpr void append_bitstr(std::string& out, E mask, E bit, const std::string& bitstr, bool& comma)`
 *     - appends `bitstr` to `out` if `mask` contains `bit`, prepends a comma if `comma` and sets `comma` to `true`.
 *   - Equality- and bit-operations, e.g.
 *     - `constexpr bool operator==(const E lhs, const E rhs) noexcept`
 *     - `constexpr E operator&(const E lhs, const E rhs) noexcept`
 *   - `constexpr std::string_view long_name<auto v>() noexcept`
 *     - returns a long string, e.g. `E::value`
 *   - `constexpr std::string_view name<auto v>() noexcept`
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
    consteval_cxx20 std::string_view long_name() noexcept {
      // const char *enum_funcname() [E = test_type1_t, V = test_type1_t::one]
      constexpr std::string_view sym(enum_funcname<decltype(V), V>());
      size_t i = sym.rfind(' ');
      if ( std::string_view::npos == i || i+1 == sym.length() ) {
        return sym;
      }
      ++i;
      const char c = sym[i];
      if ( c >= '0' && c <= '9' ) {
        return sym;
      }
      if constexpr ( sym[sym.length() - 1] == ']' ) {
        return sym.substr(i, sym.length()-i-1);
      }
      return sym.substr(i);
    }

    template <auto V,
              std::enable_if_t<std::is_enum_v<decltype(V)>>* = nullptr>
    consteval_cxx20 std::string_view name() noexcept {
      // const char *enum_funcname() [E = test_type1_t, V = test_type1_t::one]
      constexpr std::string_view sym(enum_funcname<decltype(V), V>());
      // Find the final space character in the pretty name.
      size_t i = sym.rfind(' ');
      if ( std::string_view::npos == i || i+1 == sym.length() ) {
        return sym;
      }
      ++i;
      const char c = sym[i];
      if ( c >= '0' && c <= '9' ) {
        return sym;
      }

      size_t j = sym.find("::", i);
      if ( std::string_view::npos == j || j+2 >= sym.length() ) {
        return sym;
      }
      j+=2;
      if constexpr ( sym[sym.length() - 1] == ']' ) {
        return sym.substr(j, sym.length()-j-1);
      }
      return sym.substr(j);
    }

    template <auto... Vargs>
    struct NameTable {
        std::string_view names[sizeof...(Vargs)];
    };
    template <auto... Vargs>
    consteval_cxx20 NameTable<Vargs...>
    get_names() noexcept {
        return { { long_name<Vargs>()... } };
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

    template <typename E, std::enable_if_t<std::is_enum_v<E>>* = nullptr>
    constexpr std::underlying_type_t<E>
    operator*(const E v) noexcept { return static_cast<std::underlying_type_t<E>>(v); }

    template<typename E, std::enable_if_t<std::is_enum_v<E>>* = nullptr>
    constexpr E operator~(const E rhs) noexcept {
        return E(~number(rhs));
    }

    template<typename E, std::enable_if_t<std::is_enum_v<E>>* = nullptr>
    constexpr E operator^(const E lhs, const E rhs) noexcept {
        return E(*lhs ^ *rhs);
    }

    template<typename E, std::enable_if_t<std::is_enum_v<E>>* = nullptr>
    constexpr E operator|(const E lhs, const E rhs) noexcept {
        return E(*lhs | *rhs);
    }

    template<typename E, std::enable_if_t<std::is_enum_v<E>>* = nullptr>
    constexpr E operator&(const E lhs, const E rhs) noexcept {
        return E(*lhs & *rhs);
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
        return *lhs == *rhs;
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
        return std::underlying_type_t<E>(0) != ( *mask & *bits );
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
        os << name(v);
        return os;
    }

    template <typename EnumType, auto... Vargs> class enum_iterator; // fwd

    /**
     * Enumeration info template class including iterator (enum_iterator)
     */
    template <typename EnumType, auto... Vargs>
    class enum_info {
        public:
            typedef size_t                                      size_type;
            typedef ssize_t                                     difference_type;

            /// enum value type, i.e. the enum type itself
            typedef EnumType                                    value_type;

            /// pointer to value_type
            typedef const value_type*                           pointer;
            /// reference to value_type
            typedef const value_type&                           reference;

            /// enum_iterator to iterate over all enum values (const enum values)
            typedef enum_iterator<value_type, Vargs...>         iterator;
            /// enum_iterator to iterate over all enum values (const enum values)
            typedef enum_iterator<value_type, Vargs...>         const_iterator;

            /** Used to determine whether this type is an enum_info, see is_enum_info<T> */
            typedef bool                                        enum_info_tag;

            /// array type for all enum values (value_type), see size()
            typedef std::array<value_type, sizeof...(Vargs)>    value_array_t;

            /// number of all enum values
            static constexpr size_type size() noexcept { return sizeof...(Vargs); }

        private:
            typedef const value_type*                           iterator_base_t;
            value_array_t                                       values_;
            constexpr iterator_base_t begin_iter() const noexcept { return &values_[0]; }
            constexpr iterator_base_t end_iter() const noexcept { return &values_[0] + size(); }
            friend class enum_iterator<value_type, Vargs...>;

            constexpr explicit enum_info() noexcept
            : values_({{ Vargs... }})
            { }

        public:
            constexpr enum_info(const enum_info& o) noexcept = delete;

            constexpr_cxx23 static const enum_info& get() noexcept {
                static enum_info singleton;
                return singleton;
            }

            constexpr std::string_view name() const noexcept { return type_name(*begin()); }
            constexpr value_array_t values() const noexcept { return values_; }

            constexpr const_iterator cbegin() const noexcept { return const_iterator(*this, begin_iter()); }
            constexpr const_iterator cend() const noexcept { return const_iterator(*this, end_iter()); }

            constexpr iterator begin() const noexcept { return const_iterator(*this, begin_iter()); }
            constexpr iterator end() const noexcept { return const_iterator(*this, end_iter()); }
    };

    /**
     * <code>template< class T > is_enum_info<T>::value</code> compile-time Type Trait,
     * determining whether the given template class is a enum_info type.
     */
    template< class, class = void >
    struct is_enum_info : std::false_type { };

    /**
     * <code>template< class T > is_enum_info<T>::value</code> compile-time Type Trait,
     * determining whether the given template class is a enum_info type.
     */
    template< class T >
    struct is_enum_info<T, std::void_t<typename T::enum_info_tag>> : std::true_type { };

    template <typename enum_info_t, std::enable_if_t<is_enum_info<enum_info_t>::value>* = nullptr>
    inline std::ostream& operator<<(std::ostream& os, const enum_info_t& v) {
        os << v.name() << "[";
        typename enum_info_t::iterator end = v.end();
        bool comma = false;
        for(typename enum_info_t::iterator iter = v.begin(); iter != end; ++iter, comma=true) {
            typename enum_info_t::value_type ev = *iter;
            if( comma ) {
                os << ", ";
            }
            os << ev << " (" << std::to_string( *ev ) << ")";
        }
        os << "]";
        return os;
    }

    /**
     * Enumeration iterator, see enum_info
     */
    template <typename EnumType, auto... Vargs>
    class enum_iterator {
        public:
            typedef enum_info<EnumType, Vargs...>               enum_info_t;

            typedef enum_info_t::size_type                      size_type;
            typedef enum_info_t::difference_type                difference_type;

            /// enum value type, i.e. the enum type itself
            typedef enum_info_t::value_type                     value_type;

            /// pointer to value_type
            typedef enum_info_t::pointer                        pointer;

            /// reference to value_type
            typedef enum_info_t::reference                      reference;

            /// enum_iterator to iterate over all enum values (const enum values)
            typedef enum_info_t::iterator_base_t                iterator_type;

        private:
            const enum_info_t&                                  info_;
            iterator_type                                       iterator_;
            friend class enum_info<EnumType, Vargs...>;

            constexpr explicit enum_iterator(const enum_info_t& info, iterator_type iter) noexcept
            : info_(info), iterator_(iter)
            { }

        public:
            /**
             * C++ named requirements: LegacyIterator: CopyConstructible
             */
            constexpr enum_iterator(const enum_iterator& o) noexcept = default;

            /**
             * Assigns content of other mutable iterator to this one,
             * if they are not identical.
             * <p>
             * C++ named requirements: LegacyIterator: CopyAssignable
             * </p>
             * @param o the new identity value to be copied into this iterator
             * @return reference to this
             */
            constexpr enum_iterator& operator=(const enum_iterator& o) noexcept = default;


            /**
             * C++ named requirements: LegacyIterator: MoveConstructable
             */
            constexpr enum_iterator(enum_iterator && o) noexcept  = default;

            /**
             * Assigns identity of given mutable iterator,
             * if they are not identical.
             * <p>
             * C++ named requirements: LegacyIterator: MoveAssignable
             * </p>
             * @param o the new identity to be taken
             * @return reference to this
             */
            constexpr enum_iterator& operator=(enum_iterator&& o) noexcept = default;

            const enum_info_t& description() const noexcept { return info_; }

            /**
             * C++ named requirements: LegacyIterator: Swappable
             */
            void swap(enum_iterator& o) noexcept {
                std::swap( info_, o.info_);
                std::swap( iterator_, o.iterator_);
            }

            /**
             * Returns the distance to_end() using zero as first index. A.k.a the remaining elements iterable.
             */
            constexpr difference_type dist_end() const noexcept { return info_.end_iter() - iterator_; }

            /**
             * Returns true, if this iterator points to end().
             */
            constexpr bool is_end() const noexcept { return iterator_ == info_.end_iter(); }

            /**
             * This iterator is set to the last element, end(). Returns *this;
             */
            constexpr enum_iterator& to_end() noexcept
            { iterator_ = info_.end_iter(); return *this; }

            /**
             * Returns the distance to_begin() using zero as first index. A.k.a the index from start.
             */
            constexpr difference_type dist_begin() const noexcept { return iterator_ - info_.begin_iter(); }

            /**
             * Returns true, if this iterator points to begin().
             */
            constexpr bool is_begin() const noexcept { return iterator_ == info_.begin_iter(); }

            /**
             * This iterator is set to the first element, begin(). Returns *this;
             */
            constexpr enum_iterator& to_begin() noexcept
            { iterator_ = info_.begin_iter(); return *this; }

            /**
             * Returns a copy of the underlying storage iterator.
             */
            constexpr iterator_type base() const noexcept { return iterator_; }

            // Multipass guarantee equality

            /**
             * Returns signum or three-way comparison value
             * <pre>
             *    0 if equal (both, store and iteratore),
             *   -1 if this->iterator_ < rhs_iter and
             *    1 if this->iterator_ > rhs_iter (otherwise)
             * </pre>
             * @param rhs_store right-hand side store
             * @param rhs_iter right-hand side iterator
             */
            constexpr int compare(const enum_iterator& rhs) const noexcept {
                return iterator_ == rhs.iterator_ ? 0
                       : ( iterator_ < rhs.iterator_ ? -1 : 1);
            }

            /** Two way comparison operator, `!=` is implicit, C++20 */
            constexpr bool operator==(const enum_iterator& rhs) const noexcept
            { return iterator_ == rhs.iterator_; }

            /** Three way std::strong_ordering comparison operator, C++20 */
            std::strong_ordering operator<=>(const enum_iterator& rhs) const noexcept {
                return iterator_ == rhs.iterator_ ? std::strong_ordering::equal :
                       ( iterator_ < rhs.iterator_ ? std::strong_ordering::less : std::strong_ordering::greater );
            }

            // Forward iterator requirements

            /**
             * Dereferencing iterator to value_type reference
             * @return immutable reference to value_type
             */
            constexpr value_type operator*() const noexcept {
                return *iterator_;
            }

            /**
             * Pointer to member access.
             * @return immutable pointer to value_type
             */
            constexpr const pointer operator->() const noexcept {
                return &(*iterator_); // just in case iterator_type is a class, trick via dereference
            }

            /** Pre-increment; Well performing, return *this.  */
            constexpr enum_iterator& operator++() noexcept {
                ++iterator_;
                return *this;
            }

            /** Post-increment. */
            constexpr enum_iterator operator++(int) noexcept
            { return enum_iterator(info_, iterator_++); }

            // Bidirectional iterator requirements

            /** Pre-decrement; Well performing, return *this.  */
            constexpr enum_iterator& operator--() noexcept {
                --iterator_;
                return *this;
            }

            /** Post-decrement. */
            constexpr enum_iterator operator--(int) noexcept
            { return enum_iterator(info_, iterator_--); }

            // Random access iterator requirements

            /** Subscript of 'element_index', returning immutable value_type. */
            constexpr value_type operator[](difference_type i) const noexcept
            { return iterator_[i]; }

            /** Addition-assignment of 'element_count'; Returns *this.  */
            constexpr enum_iterator& operator+=(difference_type i) noexcept
            { iterator_ += i; return *this; }

            /** Binary 'iterator + element_count'  */
            constexpr enum_iterator operator+(difference_type rhs) const noexcept
            { return enum_iterator(info_, iterator_ + rhs); }

            /** Subtraction-assignment of 'element_count'; Returns *this.  */
            constexpr enum_iterator& operator-=(difference_type i) noexcept
            { iterator_ -= i; return *this; }

            /** Binary 'iterator - element_count' */
            constexpr enum_iterator operator-(difference_type rhs) const noexcept
            { return enum_iterator(info_, iterator_ - rhs); }

            // Distance or element count, binary subtraction of two iterator.

            /** Binary 'iterator - iterator -> element_count'; Returns element_count of type difference_type. */
            constexpr difference_type operator-(const enum_iterator& rhs) const noexcept
            { return iterator_ - rhs.iterator_; }
    };

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

#define JAU_MAKE_ENUM_STRING(type, ...)                       \
    JAU_MAKE_ENUM_STRING_SUB(type, type, __VA_ARGS__)         \
                                                            \
    constexpr std::string                                   \
    to_string(const type e)                                 \
    { return std::string(name(e)); }                        \

#define JAU_APPEND_BITSTR(U,V,M) jau::enums::append_bitstr(out, M, U::V, #V, comma);

#define JAU_MAKE_BITFIELD_ENUM_STRING(type, ...)              \
    JAU_MAKE_ENUM_STRING_SUB(type, type, __VA_ARGS__)         \
                                                            \
    inline std::string                                      \
    to_string(const type mask) {                            \
        std::string out("[");                               \
        bool comma = false;                                 \
        JAU_FOR_EACH_VALUE(JAU_APPEND_BITSTR, type, mask, __VA_ARGS__); \
        out.append("]");                                    \
        return out;                                         \
    }                                                       \

#define JAU_MAKE_BITFIELD_ENUM_STRING2(type, stype, ...)      \
    JAU_MAKE_ENUM_STRING_SUB(type, stype, __VA_ARGS__)        \
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
#define JAU_MAKE_ENUM_STRING_SUB(type, stype, ...)          \
    constexpr std::string_view                              \
    long_name(const type v) noexcept                        \
    {                                                       \
        switch (v) {                                        \
            JAU_FOR_EACH(JAU_ENUM_CASE_LONG, type, __VA_ARGS__) \
            default:                                        \
                return "undef " #stype;                     \
        }                                                   \
    }                                                       \
    constexpr std::string_view                              \
    name(const type v) noexcept                             \
    {                                                       \
        switch (v) {                                        \
            JAU_FOR_EACH(JAU_ENUM_CASE_SHORT, type, __VA_ARGS__) \
            default:                                        \
                return "undef";                             \
        }                                                   \
    }                                                       \
    constexpr std::string_view                              \
    type_name(const type) noexcept                          \
    {                                                       \
        return #stype;                                      \
    }                                                       \

#define JAU_MAKE_ENUM_INFO(type, ...)                   \
    JAU_MAKE_ENUM_INFO2(type, type, __VA_ARGS__)        \

#define JAU_MAKE_ENUM_INFO2(type, stype, ...)           \
    typedef jau::enums::enum_info<type, JAU_FOR_EACH_LIST(JAU_ENUM_TYPE_VALUE, type, __VA_ARGS__)> stype##_info_t; \


/**@}*/

#endif /* JAU_ENUM_UTIL_HPP_ */
