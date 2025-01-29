/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021 Gothel Software e.K.
 *
 * ***
 *
 * SPDX-License-Identifier: MIT
 *
 * This Source Code Form is subject to the terms of the MIT License
 * If a copy of the MIT was not distributed with this
 * file, You can obtain one at https://opensource.org/license/mit/.
 *
 */

#ifndef JAU_STRING_CFMT_HPP_
#define JAU_STRING_CFMT_HPP_

#include <sys/types.h>
#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <string>
#include <string_view>
#include <type_traits>
#include <iostream>

#include <jau/byte_util.hpp>
#include <jau/cpp_lang_util.hpp>
#include <jau/packed_attribute.hpp>
#include <jau/type_traits_queries.hpp>

#include <jau/cpp_pragma.hpp>

/**
 * @anchor jau_cfmt_header
 * ## `snprintf` argument type checker `jau::cfmt`
 *
 * ### Features
 * - jau::cfmt::check() provides strict type matching of arguments against the format string.
 * - Have jau::cfmt::check() to be passed _before_ using std::snprintf(),
 *   removing safety concerns of the latter and benefit from its formatting and performance.
 * - Follows [C++ Reference](https://en.cppreference.com/w/cpp/io/c/fprintf)
 *
 * ### Type Conversion
 * Implementation follows type conversion rules as described
 * in [Variadic Default Conversion](https://en.cppreference.com/w/cpp/language/variadic_arguments#Default_conversions)
 *   - float to double promotion
 *   - bool, char, short, and unscoped enumerations are converted to int or wider integer types
 * as well as in [va_arg](https://en.cppreference.com/w/cpp/utility/variadic/va_arg)
 *   - ignore signed/unsigned type differences for integral types
 *   - void pointer tolerance
 *
 * ### Implementation Details
 * - Validates arguments against format string at compile time or runtime,
 *   depending whether passed arguments are of constexpr nature (compile time).
 * - Written in C++20 using template argument pack w/ save argument type checks
 * - Written as constexpr, capable to be utilized at compile-time.
 *
 * #### Supported Conversion Specifiers and Data Types
 *
 * The following conversion specifiers are supported:
 * - `c`, `s`, `d`, `o`, `x`, `X`, `u`, `f`, `e`, `E`, `a`, `A`, `g`, `G`, `p`
 * - Their synonyms
 *   - `i` -> `d`
 *   - `F` -> `f`
 * - Flags `-`, `+`, ` `, `0` and `#`
 * - Asterisk `*` for field width and precision
 *
 * The following length modifiers are supported where allowed
 * - `hh` [unsigned] char
 * - `h` [unsigned] short
 * - `l` [unsigned] long
 * - `ll` [unsigned] long long
 * - 'j' uintmax_t or intmax_t
 * - 'z' size_t or ssize_t
 * - 't' ptrdiff_t
 * - 'L' long double
 *
 * See [C++ Reference](https://en.cppreference.com/w/cpp/io/c/fprintf) for details.
 *
 * ### Further Documentation
 * - [C++ Reference](https://en.cppreference.com/w/cpp/io/c/fprintf)
 * - [Linux snprintf(3) man page](https://www.man7.org/linux/man-pages/man3/snprintf.3p.html)
 * - [FreeBSD snprintf(3) man page](https://man.freebsd.org/cgi/man.cgi?snprintf(3))
 */
namespace jau::cfmt {

    /** \addtogroup StringUtils
     *
     *  @{
     */

    enum class pstate_t {
        error,
        outside,
        start,
        field_width,
        precision
    };
    constexpr static const char *to_string(pstate_t s) noexcept {
        switch( s ) {
            case pstate_t::outside:     return "outside";
            case pstate_t::start:       return "start";
            case pstate_t::field_width: return "width";
            case pstate_t::precision:   return "precision";
            default:                    return "error";
        }
    }
    enum class plength_t {
        hh,
        h,
        none,
        l,
        ll,
        j,
        z,
        t,
        L
    };
    constexpr static const char *to_string(plength_t s) noexcept {
        switch( s ) {
            case plength_t::hh:   return "hh";
            case plength_t::h:    return "h";
            case plength_t::none: return "";
            case plength_t::l:    return "l";
            case plength_t::ll:   return "ll";
            case plength_t::j:    return "j";
            case plength_t::z:    return "z";
            case plength_t::t:    return "t";
            case plength_t::L:    return "L";
            default:              return "n/a";
        }
    }

    static constexpr bool isDigit(const char c) noexcept { return '0' <= c && c <= '9'; }

    namespace impl {
        class Parser;  // fwd
    }
    struct PResult {
        std::string_view fmt;
        size_t pos;
        ssize_t arg_count;
        int line;
        pstate_t state;
        plength_t length_mod;
        bool precision_set;

        constexpr PResult(const std::string_view fmt_) noexcept
        : fmt(fmt_), pos(0), arg_count(0), line(0), state(pstate_t::outside), length_mod(plength_t::none), precision_set(false) { }

        constexpr PResult(const PResult &pre) noexcept = default;

        constexpr PResult &operator=(const PResult &x) noexcept = default;

        constexpr bool hasNext() const noexcept {
            return !error() && pos < fmt.length() - 1;
        }

        constexpr ssize_t argCount() const noexcept { return arg_count; }
        constexpr bool error() const noexcept { return pstate_t::error == state; }

        std::string toString() const {
            const char c = pos < fmt.length() ? fmt[pos] : '@';
            std::string s = "args ";
            s.append(std::to_string(arg_count))
            .append(", state ")
            .append(to_string(state))
            .append(", line ")
            .append(std::to_string(line))
            .append(", pos ")
            .append(std::to_string(pos))
            .append(", char `")
            .append(std::string(1, c))
            .append("`, length `")
            .append(to_string(length_mod))
            .append("`, precision ")
            .append(std::to_string(precision_set))
            .append(", fmt `")
            .append(fmt)
            .append("`");
            return s;
        }

      private:
        friend class impl::Parser;

        constexpr bool nextSymbol(char &c) noexcept {
            if( pos < fmt.length() - 1 ) {
                c = fmt[++pos];
                return true;
            } else {
                return false;
            }
        }
        constexpr bool toConversion() noexcept {
            if( pstate_t::outside != state ) {
                return true;  // inside conversion specifier
            } else if( fmt[pos] == '%' ) {
                state = pstate_t::start;  // just at start of conversion specifier
                return true;
            } else if( pos < fmt.length() - 1 ) {
                // seek next conversion specifier
                const size_t q = fmt.find('%', pos + 1);
                if( q == std::string::npos ) {
                    // no conversion specifier found
                    pos = fmt.length();
                    return false;
                } else {
                    // new conversion specifier found
                    pos = q;
                    state = pstate_t::start;
                    return true;
                }
            } else {
                // end of format
                return false;
            }
        }

        constexpr void setError(int l) noexcept {
            line = l;
            state = pstate_t::error;
            if( 0 == arg_count ) {
                arg_count = std::numeric_limits<ssize_t>::min();
            } else if( 0 < arg_count ) {
                arg_count *= -1;
            }
        }

        constexpr void resetArgMods() noexcept {
            length_mod = plength_t::none;
            precision_set = false;
        }
    };

    inline std::ostream &operator<<(std::ostream &out, const PResult &pc) {
        out << pc.toString();
        return out;
    }

    namespace impl {

        static constexpr bool verbose_error = true;

        enum class no_type_t {};

        class Parser {
          public:
            constexpr Parser() noexcept = default;

            /**
             * Parse the given argument against the current conversion specifier of the format string.
             *
             * Multiple rounds of parsing calls might be required, each passing the next argument or null.
             *
             * Parsing is completed when method returns false, either signaling an error() or completion.
             *
             * @tparam T The type of the given argument
             * @return true if no error _and_ not complete, i.e. further calls with subsequent parameter required. Otherwise parsing is done due to error or completeness.
             */
            template <typename T>
            constexpr bool parseOne(PResult &pc) const noexcept {
                if( !pc.hasNext() ) {
                    return false;  // done or error
                }

                if( !pc.toConversion() ) {
                    return false;  // done
                }
                // pstate_t::outside != _state

                char c;
                /* skip '%' or previous `*` */
                if( !pc.nextSymbol(c) ) {
                    pc.setError(__LINE__);
                    return false;  // error
                }

                if( pstate_t::start == pc.state ) {
                    pc.state = pstate_t::field_width;
                    parseFlags(pc, c);

                    /* parse field width */
                    if( !parseFieldWidth<T>(pc, c) ) {
                        return !pc.error();  // error or continue with next argument for same conversion -> field_width
                    }
                }

                if( pstate_t::field_width == pc.state ) {
                    /* parse precision */
                    pc.state = pstate_t::precision;
                    if( c == '.' ) {
                        if( !parsePrecision<T>(pc, c) ) {
                            return !pc.error();  // error or continue with next argument for same conversion -> precision
                        }
                    }
                }
                if( !parseLengthMods(pc, c) ) {
                    return false;  // error
                }

                if( !parseFmtSpec<T>(pc, c) ) {
                    return false;  // error
                }

                // next conversion specifier
                pc.state = pstate_t::outside;
                if( !pc.nextSymbol(c) ) {
                    return false;  // done
                }
                pc.resetArgMods();
                return pc.hasNext();
            }

          private:
            constexpr void parseFlags(PResult &pc, char &c) const noexcept {
                do {
                    switch( c ) {
                        case '0':  break;
                        case '-':  break;
                        case '+':  break;
                        case ' ':  break;
                        case '#':  break;
                        case '\'': break;
                        default:   return;  // done, not a flag
                    }
                } while( pc.nextSymbol(c) );
            }

            /* parse field width, returns true if parsing can continue or false if next argument is required or error */
            template <typename T>
            constexpr bool parseFieldWidth(PResult &pc, char &c) const noexcept {
                if( c == '*' ) {
                    if( !pc.nextSymbol(c) ) { return false; }

                    using U = std::remove_cv_t<T>;

                    if constexpr( std::is_same_v<no_type_t, T> ) {
                        pc.setError(__LINE__);
                        return false;  // error
                    }
                    ++pc.arg_count;
                    if constexpr( !std::is_same_v<int, U> ) {
                        pc.setError(__LINE__);
                        return false;  // error
                    }
                    return false;  // next argument is required
                } else if( isDigit(c) ) {
                    if( !pc.nextSymbol(c) ) { return false; }
                    while( isDigit(c) ) {
                        if( !pc.nextSymbol(c) ) { return false; }
                    }
                }
                return true;  // continue with current argument
            }

            /* parse precision, returns true if parsing can continue or false if next argument is required or error */
            template <typename T>
            constexpr bool parsePrecision(PResult &pc, char &c) const noexcept {
                pc.precision_set = true;
                if( !pc.nextSymbol(c) ) { return false; }
                if( c == '*' ) {
                    if( !pc.nextSymbol(c) ) { return false; }

                    using U = std::remove_cv_t<T>;

                    if constexpr( std::is_same_v<no_type_t, T> ) {
                        pc.setError(__LINE__);
                        return false;  // error
                    }
                    ++pc.arg_count;
                    if constexpr( !std::is_same_v<int, U> ) {
                        pc.setError(__LINE__);
                        return false;  // error
                    }
                    return false;  // next argument is required
                } else if( isDigit(c) ) {
                    if( !pc.nextSymbol(c) ) { return false; }
                    while( isDigit(c) ) {
                        if( !pc.nextSymbol(c) ) { return false; }
                    }
                }
                return true;  // continue with current argument
            }

            /* parse length modifier, returns true if parsing can continue or false on error. */
            constexpr bool parseLengthMods(PResult &pc, char &c) const noexcept {
                if( 'h' == c ) {
                    if( !pc.nextSymbol(c) ) { return false; }
                    if( 'h' == c ) {
                        if( !pc.nextSymbol(c) ) { return false; }
                        pc.length_mod = plength_t::hh;
                    } else {
                        pc.length_mod = plength_t::h;
                    }
                } else if( 'l' == c ) {
                    if( !pc.nextSymbol(c) ) { return false; }
                    if( 'l' == c ) {
                        if( !pc.nextSymbol(c) ) { return false; }
                        pc.length_mod = plength_t::ll;
                    } else {
                        pc.length_mod = plength_t::l;
                    }
                } else if( 'j' == c ) {
                    if( !pc.nextSymbol(c) ) { return false; }
                    pc.length_mod = plength_t::j;
                } else if( 'z' == c ) {
                    if( !pc.nextSymbol(c) ) { return false; }
                    pc.length_mod = plength_t::z;
                } else if( 't' == c ) {
                    if( !pc.nextSymbol(c) ) { return false; }
                    pc.length_mod = plength_t::t;
                } else if( 'L' == c ) {
                    if( !pc.nextSymbol(c) ) { return false; }
                    pc.length_mod = plength_t::L;
                } else {
                    pc.length_mod = plength_t::none;
                }
                return true;
            }

            template <typename T>
            constexpr bool parseFmtSpec(PResult &pc, char fmt_literal) const noexcept {
                fmt_literal = unaliasFmtSpec(fmt_literal);

                switch( fmt_literal ) {
                    case '%':
                    case 'c':
                    case 's':
                        return parseStringFmtSpec<T>(pc, fmt_literal);
                    case 'p':
                        return parseAPointerFmtSpec<T>(pc);
                    case 'd':
                        return parseSignedFmtSpec<T>(pc);
                    case 'o':
                    case 'x':
                    case 'X':
                    case 'u':
                        return parseUnsignedFmtSpec<T>(pc, fmt_literal);
                    case 'f':
                    case 'e':
                    case 'E':
                    case 'a':
                    case 'A':
                    case 'g':
                    case 'G':
                        return parseFloatFmtSpec<T>(pc, fmt_literal);
                    default:
                        pc.setError(__LINE__);
                        return false;
                }  // switch( fmt_literal )
            }

            constexpr char unaliasFmtSpec(const char fmt_literal) const noexcept {
                switch( fmt_literal ) {
                    case 'i': return 'd';
                    case 'F': return 'f';
                    default:  return fmt_literal;
                }
            }

            template <typename T>
            constexpr bool parseStringFmtSpec(PResult &pc, const char fmt_literal) const noexcept {
                if constexpr( std::is_same_v<no_type_t, T> ) {
                    pc.setError(__LINE__);
                    return false;
                }
                switch( fmt_literal ) {
                    case '%':
                        break;
                    case 'c': {
                        ++pc.arg_count;
                        using U = std::remove_cv_t<T>;
                        switch( pc.length_mod ) {
                            case plength_t::none:
                                if constexpr( !std::is_same_v<char, U> ||
                                              !std::is_same_v<int, U> ) {
                                    pc.setError(__LINE__);
                                    return false;
                                }
                                break;
                            case plength_t::l:
                                if constexpr( !std::is_same_v<wchar_t, U> ||
                                              !std::is_same_v<wint_t, U> ) {
                                    pc.setError(__LINE__);
                                    return false;
                                }
                                break;
                            default:
                                pc.setError(__LINE__);
                                return false;
                        }
                        break;
                    }
                    case 's':
                        ++pc.arg_count;
                        switch( pc.length_mod ) {
                            case plength_t::none:
                                if constexpr( !std::is_pointer_v<T> ||
                                              !std::is_same_v<char, std::remove_cv_t<std::remove_pointer_t<T>>> ) {
                                    pc.setError(__LINE__);
                                    return false;
                                }
                                break;
                            case plength_t::l:
                                if constexpr( !std::is_pointer_v<T> ||
                                              !std::is_same_v<wchar_t, std::remove_cv_t<std::remove_pointer_t<T>>> ) {
                                    pc.setError(__LINE__);
                                    return false;
                                }
                                break;
                            default:
                                // setError();
                                pc.setError(__LINE__);
                                return false;
                        }
                        break;
                    default:
                        pc.setError(__LINE__);
                        return false;
                }
                return true;
            }

            template <typename T>
            constexpr bool parseAPointerFmtSpec(PResult &pc) const noexcept {
                pc.length_mod = plength_t::none;
                if constexpr( std::is_same_v<no_type_t, T> ) {
                    pc.setError(__LINE__);
                    return false;
                }
                ++pc.arg_count;
                if constexpr( !std::is_pointer_v<T> ) {
                    pc.setError(__LINE__);
                    return false;
                }
                return true;
            }

            template <typename S>
            constexpr bool parseSignedFmtSpec(PResult &pc) const noexcept {
                if constexpr( std::is_same_v<no_type_t, S> ) {
                    pc.setError(__LINE__);
                    return false;
                }
                ++pc.arg_count;

                using T = std::remove_cv_t<S>;
                // using U = std::conditional<std::is_integral_v<T> && std::is_unsigned_v<T>, std::make_signed<T>, T>::type; // triggers the instantiating the 'other' case and hence fails
                using U = typename std::conditional_t<std::is_integral_v<T> && std::is_unsigned_v<T>, std::make_signed<T>, std::type_identity<T>>::type;  // NOLINT

                switch( pc.length_mod ) {
                    case plength_t::hh:
                        if constexpr( !std::is_same_v<char, U> && (!std::is_integral_v<U> || sizeof(U) > sizeof(char)) ) {
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;
                    case plength_t::h:
                        if constexpr( !std::is_same_v<short, U> && (!std::is_integral_v<U> || sizeof(U) > sizeof(short)) ) {
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;
                    case plength_t::none:
                        if constexpr( !std::is_same_v<int, U> && (!std::is_integral_v<U> || sizeof(U) > sizeof(int)) ) {
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;
                    case plength_t::l:
                        if constexpr( !std::is_same_v<long, U> && (!std::is_integral_v<U> || sizeof(U) > sizeof(long)) ) {
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;
                    case plength_t::ll:
                        if constexpr( !std::is_same_v<long long, U> && (!std::is_integral_v<U> || sizeof(U) > sizeof(long long)) ) {
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;
                    case plength_t::j:
                        if constexpr( !std::is_same_v<intmax_t, U> && (!std::is_integral_v<U> || sizeof(U) > sizeof(intmax_t)) ) {
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;
                    case plength_t::z:
                        if constexpr( !std::is_same_v<ssize_t, U> && (!std::is_integral_v<U> || sizeof(U) > sizeof(ssize_t)) ) {
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;
                    case plength_t::t:
                        if constexpr( !std::is_same_v<ptrdiff_t, U> && (!std::is_integral_v<U> || sizeof(U) > sizeof(ptrdiff_t)) ) {
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;
                    default:
                        pc.setError(__LINE__);
                        return false;
                }
                return true;
            }

            template <typename S>
            constexpr bool parseUnsignedFmtSpec(PResult &pc, const char /*fmt_literal*/) const noexcept {
                if constexpr( std::is_same_v<no_type_t, S> ) {
                    pc.setError(__LINE__);
                    return false;
                }
                ++pc.arg_count;

                using T = std::remove_cv_t<S>;
                // using U = std::conditional_t<std::is_integral_v<T> && std::is_signed_v<T>, std::make_unsigned_t<T>, T>; // triggers the instantiating the 'other' case and hence fails
                using U = typename std::conditional_t<std::is_integral_v<T> && std::is_signed_v<T>, std::make_unsigned<T>, std::type_identity<T>>::type;  // NOLINT

                switch( pc.length_mod ) {
                    case plength_t::hh:
                        if constexpr( !std::is_same_v<unsigned char, U> && (!std::is_integral_v<U> || sizeof(U) > sizeof(unsigned char)) ) {
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;
                    case plength_t::h:
                        if constexpr( !std::is_same_v<unsigned short, U> && (!std::is_integral_v<U> || sizeof(U) > sizeof(unsigned short)) ) {
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;
                    case plength_t::none:
                        if constexpr( !std::is_same_v<unsigned int, U> && (!std::is_integral_v<U> || sizeof(U) > sizeof(unsigned int)) ) {
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;
                    case plength_t::l:
                        if constexpr( !std::is_same_v<unsigned long, U> && (!std::is_integral_v<U> || sizeof(U) > sizeof(unsigned long)) ) {
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;
                    case plength_t::ll:
                        if constexpr( !std::is_same_v<unsigned long long, U> && (!std::is_integral_v<U> || sizeof(U) > sizeof(unsigned long long)) ) {
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;
                    case plength_t::j:
                        if constexpr( !std::is_same_v<uintmax_t, U> && (!std::is_integral_v<U> || sizeof(U) > sizeof(uintmax_t)) ) {
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;
                    case plength_t::z:
                        if constexpr( !std::is_same_v<size_t, U> && (!std::is_integral_v<U> || sizeof(U) > sizeof(size_t)) ) {
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;
#if 0
                    case plength_t::t:
                        if constexpr( !std::is_same_v<unsigned ptrdiff_t, U> ) {
                            setError();
                            return false;
                        }
                        break;
#endif
                    default:
                        pc.setError(__LINE__);
                        return false;
                }
                return true;
            }

            template <typename T>
            constexpr bool parseFloatFmtSpec(PResult &pc, const char /*fmt_literal*/) const noexcept {
                if constexpr( std::is_same_v<no_type_t, T> ) {
                    pc.setError(__LINE__);
                    return false;
                }
                ++pc.arg_count;

                using U = std::remove_cv_t<T>;

                switch( pc.length_mod ) {
                    case plength_t::none:
                    case plength_t::l:
                        if constexpr( !std::is_same_v<float, U> &&
                                      !std::is_same_v<double, U> ) {
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;
                    case plength_t::L:
                        if constexpr( !std::is_same_v<float, U> &&
                                      !std::is_same_v<double, U> &&
                                      !std::is_same_v<long double, U> ) {
                        } else {
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;
                    default:
                        pc.setError(__LINE__);
                        return false;
                }
                return true;
            }
        };

    }  // namespace impl

    /**
     * Strict type validation of arguments against the format string.
     *
     * See @ref jau_cfmt_header for details
     *
     * @tparam Targs the argument template type pack to be validated against the format string
     * @param fmt the snprintf format string
     * @param args passed arguments, used for template type deduction only
     * @return true if successfully parsed format and arguments, false otherwise.
     * @see @ref jau_cfmt_header
     */
    template <typename... Targs>
    constexpr bool check(const std::string_view fmt, const Targs &...) noexcept {
        PResult ctx(fmt);
        constexpr const impl::Parser p;
        if constexpr( 0 < sizeof...(Targs) ) {
            ((p.template parseOne<Targs>(ctx)), ...);
        }
        p.template parseOne<impl::no_type_t>(ctx);
        return !ctx.error();
    }
    /**
     * Strict type validation of arguments against the format string.
     *
     * See @ref jau_cfmt_header for details
     *
     * @tparam Targs the argument template type pack to be validated against the format string
     * @param fmt the snprintf format string
     * @return true if successfully parsed format and arguments, false otherwise.
     * @see @ref jau_cfmt_header
     */
    template <typename... Targs>
    constexpr bool check2(const std::string_view fmt) noexcept {
        PResult ctx(fmt);
        constexpr const impl::Parser p;
        if constexpr( 0 < sizeof...(Targs) ) {
            ((p.template parseOne<Targs>(ctx)), ...);
        }
        p.template parseOne<impl::no_type_t>(ctx);
        return !ctx.error();
    }

    template <typename StrView, typename... Targs>
    constexpr bool check3(StrView fmt) noexcept {
        PResult ctx(fmt);
        constexpr const impl::Parser p;
        if constexpr( 0 < sizeof...(Targs) ) {
            ((p.template parseOne<Targs>(ctx)), ...);
        }
        p.template parseOne<impl::no_type_t>(ctx);
        return !ctx.error();
    }

    /**
     * Strict type validation of arguments against the format string.
     *
     * See @ref jau_cfmt_header for details
     *
     * @tparam Targs the argument template type pack to be validated against the format string
     * @param fmt the snprintf format string
     * @param args passed arguments, used for template type deduction only
     * @return PContext result object for further inspection.
     * @see @ref jau_cfmt_header
     */
    template <typename... Targs>
    constexpr PResult checkR(const std::string_view fmt, const Targs &...) noexcept {
        PResult ctx(fmt);
        constexpr const impl::Parser p;
        if constexpr( 0 < sizeof...(Targs) ) {
            ((p.template parseOne<Targs>(ctx)), ...);
        }
        p.template parseOne<impl::no_type_t>(ctx);
        return ctx;
    }

    /**
     * Strict type validation of arguments against the format string.
     *
     * See @ref jau_cfmt_header for details
     *
     * @tparam Targs the argument template type pack to be validated against the format string
     * @param fmt the snprintf format string
     * @return PContext result object for further inspection.
     * @see @ref jau_cfmt_header
     */
    template <typename... Targs>
    constexpr PResult checkR2(const std::string_view fmt) noexcept {
        PResult ctx(fmt);
        constexpr const impl::Parser p;
        if constexpr( 0 < sizeof...(Targs) ) {
            ((p.template parseOne<Targs>(ctx)), ...);
        }
        p.template parseOne<impl::no_type_t>(ctx);
        return ctx;
    }

    /**@}*/

}  // namespace jau::cfmt

#endif  // JAU_STRING_CFMT_HPP_
