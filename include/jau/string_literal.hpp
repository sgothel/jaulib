/*
 * Copyright (c) 2024 Gothel Software e.K.
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

#ifndef JAU_STRING_LITERAL_HPP_
#define JAU_STRING_LITERAL_HPP_

#include <algorithm>
#include <climits>
#include <string_view>
#include <cstring>

#include <jau/cpp_lang_util.hpp>

namespace jau {

    /** \addtogroup StringUtils
     *
     *  @{
     */

    /**
     * Static compile-time string literal storage.
     *
     * Properties
     * - includes buffered EOS
     * -
     *
     * Implementation has been aligned to [p3094r5](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p3094r5.html),
     * but maintains original properties (see above).
     *
     * @tparam N string length excluding EOS
     */
    template<typename CharT, std::size_t N>
    class BasicStringLiteral
    {
      public:
        using value_type = CharT;
        using size_type = std::size_t;
        using difference_type = ptrdiff_t;
        using pointer = CharT*;
        using const_pointer = const CharT*;
        using reference = CharT&;
        using const_reference = const CharT&;
        using const_iterator = const CharT*;
        using iterator = const_iterator;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

      private:
        CharT buf[N+1];

      public:
        template<std::convertible_to<CharT>... Chars>
        requires(sizeof...(Chars) == N) && (... && !std::is_pointer_v<Chars>)
        constexpr explicit BasicStringLiteral(Chars... chars) noexcept
        : buf {chars..., CharT{}} // add EOS
        { }

        /// Implicit constructor from string literal `const CharT (&str)[N+1]`, i.e. including EOS
        constexpr BasicStringLiteral(const CharT (&str)[N+1]) noexcept {
            std::copy(str, str+N+1, buf); // include EOS
        }

        template<std::size_t S, std::size_t T>
        constexpr explicit BasicStringLiteral(const BasicStringLiteral<CharT, S> &s1, const BasicStringLiteral<CharT, T> &s2) noexcept
        requires(N == S + T)
        {
            std::copy(s1.cbegin(), s1.cend(), buf);
            std::copy(s2.cbegin(), s2.cend(), buf+S);
            buf[N] = CharT{}; // EOS
        }

        constexpr BasicStringLiteral(const BasicStringLiteral& o) noexcept = default;
        constexpr BasicStringLiteral& operator=(const BasicStringLiteral& o) noexcept = default;

        constexpr BasicStringLiteral(BasicStringLiteral&& o) noexcept = default;
        constexpr BasicStringLiteral& operator=(BasicStringLiteral&& o) noexcept = default;

        constexpr const_iterator begin() const noexcept { return data(); }
        constexpr const_iterator end() const noexcept { return data() + N; }
        constexpr const_iterator cbegin() const noexcept { return begin(); }
        constexpr const_iterator cend() const noexcept { return end(); }
        constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
        constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
        constexpr const_reverse_iterator crbegin() const noexcept { return rbegin(); }
        constexpr const_reverse_iterator crend() const noexcept { return rend(); }

        constexpr bool operator==(const BasicStringLiteral &o) const noexcept {
            return std::equal(o.cbegin(), o.cend(), buf);
        }
        template<std::size_t N2>
        constexpr bool operator==(const BasicStringLiteral<CharT, N2>) const noexcept {
            return false;
        }
        constexpr CharT operator[](std::size_t n) const noexcept {
            return n < N ? buf[n] : CharT{};
        }

        template<std::size_t N2>
        constexpr BasicStringLiteral<CharT, N+N2> operator+(const BasicStringLiteral<CharT, N2> &o) const {
            return BasicStringLiteral<CharT, N+N2>(*this, o);
        }

        //
        // std::string_view functions
        //

        /// string literal size w/o EOS.
        constexpr size_type size() const noexcept { return N; };
        /// string literal size w/o EOS.
        constexpr size_type length() const noexcept { return N; };
        /// string literal size w/o EOS.
        constexpr size_type max_size() const noexcept { return N; };
        constexpr bool empty() const noexcept { return 0==N; };
        /// Returns c-string w/ EOS.
        constexpr const char* c_str() const noexcept { return buf; }
        /// Returns c-string w/ EOS.
        constexpr const_pointer data() const noexcept { return static_cast<const_pointer>(buf); }
        constexpr std::basic_string_view<CharT> view() const noexcept {
            return std::basic_string_view<CharT>(cbegin(), cend());
        }
        constexpr operator std::basic_string_view<CharT>() const noexcept { return view(); }
    };

    // deduction guide for char sequence
    template<typename CharT, std::convertible_to<CharT>... Rest>
    BasicStringLiteral(CharT, Rest...) -> BasicStringLiteral<CharT, 1 + sizeof...(Rest)>;

    // deduction guide for c-string literal
    template<typename CharT, size_t O>
    BasicStringLiteral(const CharT (&str)[O]) -> BasicStringLiteral<CharT, O - 1>;

    // deduction guide
    // template<typename CharT, size_t N>
    // BasicStringLiteral(from_range_t, std::array<CharT, N>) -> BasicStringLiteral<CharT, N>;

    template<typename CharT, std::size_t N, std::size_t O>
    constexpr BasicStringLiteral<CharT, N+O-1> operator+(const BasicStringLiteral<CharT, N> &lhs, const char (&rhs) [O]) {
        return lhs + BasicStringLiteral<CharT, O-1>(rhs);
    }

    template<typename CharT, size_t O, std::size_t N>
    constexpr BasicStringLiteral<CharT, N+O-1> operator+(const char (&lhs) [O], const BasicStringLiteral<CharT, N> rhs) {
        return BasicStringLiteral<CharT, O-1>(lhs) + rhs;
    }

    template<size_t N>
    using StringLiteral = BasicStringLiteral<char, N>;

    template<size_t N>
    using WStringLiteral = BasicStringLiteral<wchar_t, N>;

    /**@}*/

} // namespace jau

#endif /* JAU_STRING_LITERAL_HPP_ */
