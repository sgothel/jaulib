/**
 * Copyright the Collabora Online contributors.
 * Copyright Gothel Software e.K.
 *
 * ***
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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

#ifndef JAU_STRING_CFMT2_HPP_
#define JAU_STRING_CFMT2_HPP_

#include <bits/types/wint_t.h>
#include <sys/types.h>
#include <cassert>
#include <cerrno>
#include <cstddef>
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
namespace jau::cfmt2 {

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
        template <typename T>
        class Parser;  // fwd
    }
    
    struct PResult {
        const std::string_view fmt;
        const size_t pos;
        const ssize_t arg_count;
        const int line;
        const pstate_t state;
        const plength_t length_mod;
        const bool precision_set;
        
        template <size_t N>
        constexpr PResult(const char (&fmt_)[N]) noexcept
        : fmt(fmt_, N), pos(0), arg_count(0), line(0), state(pstate_t::outside), length_mod(plength_t::none), precision_set(false) { }

        constexpr PResult(const std::string_view fmt_) noexcept
        : fmt(fmt_), pos(0), arg_count(0), line(0), state(pstate_t::outside), length_mod(plength_t::none), precision_set(false) { }
        
        constexpr PResult(const PResult &pre) noexcept = default;

        constexpr PResult &operator=(const PResult &x) noexcept = delete;

        constexpr bool hasNext() const noexcept {
            return !error() && pos < fmt.length() - 1;
        }

        constexpr ssize_t argCount() const noexcept { return arg_count; }
        constexpr bool error() const noexcept { return pstate_t::error == state; }
        constexpr char sym() const noexcept { return fmt[pos]; }

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
        template <typename T>
        friend class impl::Parser;

        constexpr PResult(const PResult &pre, size_t pos2) noexcept
        : fmt(pre.fmt), pos(pos2), arg_count(pre.arg_count), line(pre.line), state(pre.state), length_mod(pre.length_mod), precision_set(pre.precision_set) {} 
        
        constexpr PResult(const PResult &pre, size_t pos2, ssize_t arg_count2) noexcept
        : fmt(pre.fmt), pos(pos2), arg_count(arg_count2), line(pre.line), state(pre.state), length_mod(pre.length_mod), precision_set(pre.precision_set) {}
        
        constexpr PResult(const PResult &pre, pstate_t state2) noexcept
        : fmt(pre.fmt), pos(pre.pos), arg_count(pre.arg_count), line(pre.line), state(state2), length_mod(pre.length_mod), precision_set(pre.precision_set) {} 
        
        constexpr PResult(const PResult &pre, pstate_t state2, size_t pos2) noexcept
        : fmt(pre.fmt), pos(pos2), arg_count(pre.arg_count), line(pre.line), state(state2), length_mod(pre.length_mod), precision_set(pre.precision_set) {} 
        
        constexpr PResult(const PResult &pre, pstate_t state2, size_t pos2, ssize_t arg_count2, int line2) noexcept
        : fmt(pre.fmt), pos(pos2), arg_count(arg_count2), line(line2), state(state2), length_mod(pre.length_mod), precision_set(pre.precision_set) {} 
         
        constexpr PResult(const PResult &pre, plength_t length_mod2, bool precision_set2) noexcept
        : fmt(pre.fmt), pos(pre.pos), arg_count(pre.arg_count), line(pre.line), state(pre.state), length_mod(length_mod2), precision_set(precision_set2) {} 
        
        constexpr PResult nextSymbol() const noexcept {
            if( pos < fmt.length() - 1 ) {
                return PResult(*this, pos+1);
            } else {
                return *this;
            }
        }
        constexpr PResult toConversion() const noexcept {
            if( pstate_t::outside != state ) {
                return PResult(*this);  // inside conversion specifier
            } else if( fmt[pos] == '%' ) {
                return PResult(*this, pstate_t::start, pos); // just at start of conversion specifier
            } else {
                // seek next conversion specifier
                const size_t q = fmt.find('%', pos + 1);
                if( q == std::string::npos ) {
                    // no conversion specifier found
                    return PResult(*this, fmt.length());
                } else {
                    // new conversion specifier found
                    return PResult(*this, pstate_t::start, q);
                }
            }
        }

        constexpr PResult setError(int l) const noexcept {
            ssize_t arg_count2;
            if( 0 == arg_count ) {
                arg_count2 = std::numeric_limits<ssize_t>::min();
            } else if( 0 < arg_count ) {
                arg_count2 = arg_count * -1;
            } else {
                arg_count2 = arg_count;
            }
            return PResult(*this, pstate_t::error, pos, arg_count2, l);
        }
    };

    inline std::ostream &operator<<(std::ostream &out, const PResult &pc) {
        out << pc.toString();
        return out;
    }

    namespace impl {

        static constexpr bool verbose_error = true;

        enum class no_type_t {};

        template <typename T>
        class Parser {
          public:
            constexpr Parser() noexcept = delete;

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
            static constexpr const PResult parseOne(const PResult pc) noexcept {
                if( !pc.hasNext() ) {
                    return pc; // done or error
                }

                const PResult pc2 = pc.toConversion();
                if( !pc2.hasNext() ) {
                    return pc2; // done
                }
                
                // pstate_t::outside != _state
                
                /* skip '%' or previous `*` */
                const PResult pc3 = pc2.nextSymbol();

                if( pstate_t::start == pc3.state ) {
                    const PResult pc4 = parseFlags(PResult(pc3, pstate_t::field_width));

                    /* parse field width */
                    bool next_arg = false;
                    const PResult pc5 = parseFieldWidth(pc4, next_arg);
                    if( next_arg || pc5.error() ) {
                        return pc5;  // error or continue with next argument for same conversion -> field_width
                    }
                    return parseP2(pc5);
                } else {
                    return parseP2(pc3);
                }
            }

          private:
            
            static constexpr const PResult parseP2(const PResult pc) noexcept {
                if( pstate_t::field_width == pc.state ) {
                    /* parse precision */
                    const PResult pc2 = PResult(pc, pstate_t::precision);
                    if( pc2.sym() == '.' ) {
                        const PResult pc3 = PResult(pc2, pc2.length_mod, true);
                        bool next_arg = false;
                        const PResult pc4 = parsePrecision(pc3, next_arg);
                        if( next_arg || pc4.error() ) {
                            return pc4;  // error or continue with next argument for same conversion -> precision
                        }
                        return parseP3(pc4);
                    } else {
                        return parseP3(pc2);
                    }
                } else {
                    return parseP3(pc);
                }
            }
            
            static constexpr const PResult parseP3(const PResult pc) noexcept {
                const PResult pc2 = parseLengthMods(pc);
                if( pc2.error() ) {
                    return pc2;  // error
                }

                const PResult pc3 = parseFmtSpec(pc2);
                if( pc3.error() ) {
                    return pc3;  // error
                }

                // next conversion specifier
                const PResult pc4 = PResult(pc3, pstate_t::outside);
                if( !pc4.hasNext() ) {
                    return pc4; // done
                }
                return PResult(pc4.nextSymbol(), plength_t::none, false); // clear length_mod and precision_set
            }
          
            static constexpr const PResult parseFlags(const PResult pc) noexcept {
                switch( pc.sym() ) {
                    case '0':  break;
                    case '-':  break;
                    case '+':  break;
                    case ' ':  break;
                    case '#':  break;
                    case '\'': break;
                    default:   return pc;  // done, not a flag
                }
                if( pc.hasNext() ) {
                    return parseFlags(pc.nextSymbol());
                } else {
                    return pc;
                }
            }

            static constexpr const PResult parseDigit(const PResult pc) noexcept {
                if( !pc.hasNext() || !isDigit(pc.sym()) ) { return pc; }
                return parseDigit( pc.nextSymbol() );
            }
            
            /* parse field width, returns true if parsing can continue or false if next argument is required or error */            
            static constexpr const PResult parseFieldWidth(const PResult pc, bool& next_arg) noexcept {
                next_arg = false;
                if( pc.sym() == '*' ) {
                    if( !pc.hasNext() ) { return pc; }
                    const PResult pc2 = pc.nextSymbol();

                    using U = std::remove_cv_t<T>;

                    if constexpr( std::is_same_v<no_type_t, T> ) {
                        return pc2.setError(__LINE__);
                    }
                    const PResult pc3 = PResult(pc2, pc.arg_count+1);
                    if constexpr( !std::is_same_v<int, U> ) {
                        return pc3.setError(__LINE__);
                    }
                    next_arg = true;
                    return pc3; // next argument is required
                } else {
                    // continue with current argument
                    return parseDigit(pc);                    
                }
            }

            /* parse precision, returns true if parsing can continue or false if next argument is required or error */            
            static constexpr const PResult parsePrecision(const PResult pc, bool &next_arg) noexcept {
                next_arg = false;                
                if( !pc.hasNext() ) { return pc; }
                const PResult pc2 = pc.nextSymbol();
                const char c = pc.fmt[pc.pos];
                if( c == '*' ) {
                    if( !pc2.hasNext() ) { return pc2; }
                    const PResult pc3 = pc2.nextSymbol();

                    using U = std::remove_cv_t<T>;

                    if constexpr( std::is_same_v<no_type_t, T> ) {
                        return pc3.setError(__LINE__);
                    }
                    const PResult pc4 = PResult(pc3, pc.arg_count+1);
                    if constexpr( !std::is_same_v<int, U> ) {
                        return pc4.setError(__LINE__);
                    }
                    next_arg = true;
                    return pc4;  // next argument is required
                } else {
                    // continue with current argument
                    return parseDigit(pc2);                    
                }
            }

            static constexpr const PResult parseLengthMods(const PResult pc) noexcept {
                const char sym = pc.sym();
                if( 'h' == sym ) {
                    if( !pc.hasNext() ) { return pc.setError(__LINE__); }
                    const PResult pc2 = pc.nextSymbol();
                    if( 'h' == pc2.sym() ) {
                        if( !pc2.hasNext() ) { return pc2.setError(__LINE__); }
                        return PResult(pc2.nextSymbol(), plength_t::hh, pc2.precision_set);
                    } else {
                        return PResult(pc2, plength_t::h, pc2.precision_set);
                    }
                } else if( 'l' == sym ) {
                    if( !pc.hasNext() ) { return pc.setError(__LINE__); }
                    const PResult pc2 = pc.nextSymbol();
                    if( 'l' == pc2.sym() ) {
                        if( !pc2.hasNext() ) { return pc2.setError(__LINE__); }
                        return PResult(pc2.nextSymbol(), plength_t::ll, pc2.precision_set);
                    } else {
                        return PResult(pc2, plength_t::l, pc2.precision_set);
                    }
                } else if( 'j' == sym ) {
                    if( !pc.hasNext() ) { return pc.setError(__LINE__); }
                    return PResult(pc.nextSymbol(), plength_t::j, pc.precision_set);
                } else if( 'z' == sym ) {
                    if( !pc.hasNext() ) { return pc.setError(__LINE__); }
                    return PResult(pc.nextSymbol(), plength_t::z, pc.precision_set);
                } else if( 't' == sym ) {
                    if( !pc.hasNext() ) { return pc.setError(__LINE__); }
                    return PResult(pc.nextSymbol(), plength_t::t, pc.precision_set);
                } else if( 'L' == sym ) {
                    if( !pc.hasNext() ) { return pc.setError(__LINE__); }
                    return PResult(pc.nextSymbol(), plength_t::L, pc.precision_set);
                } else {
                    return PResult(pc, plength_t::none, pc.precision_set);
                }
            }
            
            static constexpr const PResult parseFmtSpec(const PResult pc) noexcept {
                const char fmt_literal = unaliasFmtSpec( pc.sym() );

                switch( fmt_literal ) {
                    case '%':
                    case 'c':
                    case 's':
                        return parseStringFmtSpec(pc, fmt_literal);
                    case 'p':
                        return parseAPointerFmtSpec(pc);
                    case 'd':
                        return parseSignedFmtSpec(pc);
                    case 'o':
                    case 'x':
                    case 'X':
                    case 'u':
                        return parseUnsignedFmtSpec(pc, fmt_literal);
                    case 'f':
                    case 'e':
                    case 'E':
                    case 'a':
                    case 'A':
                    case 'g':
                    case 'G':
                        return parseFloatFmtSpec(pc, fmt_literal);
                    default:
                        return pc.setError(__LINE__);
                }  // switch( fmt_literal )
            }

            static constexpr char unaliasFmtSpec(const char fmt_literal) noexcept {
                switch( fmt_literal ) {
                    case 'i': return 'd';
                    case 'F': return 'f';
                    default:  return fmt_literal;
                }
            }
            
            static constexpr const PResult parseStringFmtSpec(const PResult pc, const char fmt_literal) noexcept {
                if constexpr( std::is_same_v<no_type_t, T> ) {
                    return pc.setError(__LINE__);
                }
                switch( fmt_literal ) {
                    case '%':
                        break;
                    case 'c': {
                        const PResult pc2 = PResult(pc, pc.pos, pc.arg_count+1);
                        using U = std::remove_cv_t<T>;
                        switch( pc2.length_mod ) {
                            case plength_t::none:
                                if constexpr( !std::is_same_v<char, U> ||
                                              !std::is_same_v<int, U> ) {
                                    return pc2.setError(__LINE__);
                                }
                                break;
                            case plength_t::l:
                                if constexpr( !std::is_same_v<wchar_t, U> ||
                                              !std::is_same_v<wint_t, U> ) {
                                    return pc2.setError(__LINE__);
                                }
                                break;
                            default:
                                return pc2.setError(__LINE__);
                        }
                        return pc2;
                    }
                    case 's': {
                        const PResult pc2 = PResult(pc, pc.pos, pc.arg_count+1);
                        switch( pc2.length_mod ) {
                            case plength_t::none:
                                if constexpr( !std::is_pointer_v<T> ||
                                              !std::is_same_v<char, std::remove_cv_t<std::remove_pointer_t<T>>> ) {
                                    return pc2.setError(__LINE__);
                                }
                                break;
                            case plength_t::l:
                                if constexpr( !std::is_pointer_v<T> ||
                                              !std::is_same_v<wchar_t, std::remove_cv_t<std::remove_pointer_t<T>>> ) {
                                    return pc2.setError(__LINE__);
                                }
                                break;
                            default:
                                return pc2.setError(__LINE__);
                        }
                        return pc2;
                    }
                    default: return pc.setError(__LINE__);
                }
                return pc;
            }
            
            static constexpr const PResult parseAPointerFmtSpec(const PResult pc) noexcept {
                const PResult pc2 = PResult(pc, plength_t::none, pc.precision_set);
                if constexpr( std::is_same_v<no_type_t, T> ) {
                    return pc2.setError(__LINE__);
                }
                const PResult pc3 = PResult(pc2, pc2.pos, pc2.arg_count+1);
                if constexpr( !std::is_pointer_v<T> ) {
                    return pc3.setError(__LINE__);
                }
                return pc3;
            }
            
            static constexpr const PResult parseSignedFmtSpec(const PResult pc) noexcept {
                if constexpr( std::is_same_v<no_type_t, T> ) {
                    return pc.setError(__LINE__);
                }
                const PResult pc2 = PResult(pc, pc.pos, pc.arg_count+1);

                using U = std::remove_cv_t<T>;
                // using U = std::conditional<std::is_integral_v<T> && std::is_unsigned_v<T>, std::make_signed<T>, T>::type; // triggers the instantiating the 'other' case and hence fails
                using V = typename std::conditional_t<std::is_integral_v<U> && std::is_unsigned_v<U>, std::make_signed<U>, std::type_identity<U>>::type;  // NOLINT

                switch( pc2.length_mod ) {
                    case plength_t::hh:
                        if constexpr( !std::is_same_v<char, V> && (!std::is_integral_v<V> || sizeof(V) > sizeof(char)) ) {
                            return pc2.setError(__LINE__);
                        }
                        break;
                    case plength_t::h:
                        if constexpr( !std::is_same_v<short, V> && (!std::is_integral_v<V> || sizeof(V) > sizeof(short)) ) {
                            return pc2.setError(__LINE__);
                        }
                        break;
                    case plength_t::none:
                        if constexpr( !std::is_same_v<int, V> && (!std::is_integral_v<V> || sizeof(V) > sizeof(int)) ) {
                            return pc2.setError(__LINE__);
                        }
                        break;
                    case plength_t::l:
                        if constexpr( !std::is_same_v<long, V> && (!std::is_integral_v<V> || sizeof(V) > sizeof(long)) ) {
                            return pc2.setError(__LINE__);
                        }
                        break;
                    case plength_t::ll:
                        if constexpr( !std::is_same_v<long long, V> && (!std::is_integral_v<V> || sizeof(V) > sizeof(long long)) ) {
                            return pc2.setError(__LINE__);
                        }
                        break;
                    case plength_t::j:
                        if constexpr( !std::is_same_v<intmax_t, V> && (!std::is_integral_v<V> || sizeof(V) > sizeof(intmax_t)) ) {
                            return pc2.setError(__LINE__);
                        }
                        break;
                    case plength_t::z:
                        if constexpr( !std::is_same_v<ssize_t, V> && (!std::is_integral_v<V> || sizeof(V) > sizeof(ssize_t)) ) {
                            return pc2.setError(__LINE__);
                        }
                        break;
                    case plength_t::t:
                        if constexpr( !std::is_same_v<ptrdiff_t, V> && (!std::is_integral_v<V> || sizeof(V) > sizeof(ptrdiff_t)) ) {
                            return pc2.setError(__LINE__);
                        }
                        break;
                    default:
                        return pc2.setError(__LINE__);
                }
                return pc2;
            }
            
            static constexpr const PResult parseUnsignedFmtSpec(const PResult pc, const char /*fmt_literal*/) noexcept {
                if constexpr( std::is_same_v<no_type_t, T> ) {
                    return pc.setError(__LINE__);
                }
                const PResult pc2 = PResult(pc, pc.pos, pc.arg_count+1);

                using U = std::remove_cv_t<T>;
                // using U = std::conditional_t<std::is_integral_v<T> && std::is_signed_v<T>, std::make_unsigned_t<T>, T>; // triggers the instantiating the 'other' case and hence fails
                using V = typename std::conditional_t<std::is_integral_v<U> && std::is_signed_v<U>, std::make_unsigned<U>, std::type_identity<U>>::type;  // NOLINT

                switch( pc2.length_mod ) {
                    case plength_t::hh:
                        if constexpr( !std::is_same_v<unsigned char, V> && (!std::is_integral_v<V> || sizeof(V) > sizeof(unsigned char)) ) {
                            return pc2.setError(__LINE__);
                        }
                        break;
                    case plength_t::h:
                        if constexpr( !std::is_same_v<unsigned short, V> && (!std::is_integral_v<V> || sizeof(V) > sizeof(unsigned short)) ) {
                            return pc2.setError(__LINE__);
                        }
                        break;
                    case plength_t::none:
                        if constexpr( !std::is_same_v<unsigned int, V> && (!std::is_integral_v<V> || sizeof(V) > sizeof(unsigned int)) ) {
                            return pc2.setError(__LINE__);
                        }
                        break;
                    case plength_t::l:
                        if constexpr( !std::is_same_v<unsigned long, V> && (!std::is_integral_v<V> || sizeof(V) > sizeof(unsigned long)) ) {
                            return pc2.setError(__LINE__);
                        }
                        break;
                    case plength_t::ll:
                        if constexpr( !std::is_same_v<unsigned long long, V> && (!std::is_integral_v<V> || sizeof(V) > sizeof(unsigned long long)) ) {
                            return pc2.setError(__LINE__);
                        }
                        break;
                    case plength_t::j:
                        if constexpr( !std::is_same_v<uintmax_t, V> && (!std::is_integral_v<V> || sizeof(V) > sizeof(uintmax_t)) ) {
                            return pc2.setError(__LINE__);
                        }
                        break;
                    case plength_t::z:
                        if constexpr( !std::is_same_v<size_t, V> && (!std::is_integral_v<V> || sizeof(V) > sizeof(size_t)) ) {
                            return pc2.setError(__LINE__);
                        }
                        break;
#if 0
                    case plength_t::t:
                        if constexpr( !std::is_same_v<unsigned ptrdiff_t, U> ) {
                            return pc2.setError(__LINE__);
                        }
                        break;
#endif
                    default:
                        return pc2.setError(__LINE__);
                }
                return pc2;
            }
            
            static constexpr const PResult parseFloatFmtSpec(const PResult pc, const char /*fmt_literal*/) noexcept {
                if constexpr( std::is_same_v<no_type_t, T> ) {
                    return pc.setError(__LINE__);
                }
                const PResult pc2 = PResult(pc, pc.pos, pc.arg_count+1);

                using U = std::remove_cv_t<T>;

                switch( pc2.length_mod ) {
                    case plength_t::none:
                    case plength_t::l:
                        if constexpr( !std::is_same_v<float, U> &&
                                      !std::is_same_v<double, U> ) {
                            return pc2.setError(__LINE__);
                        }
                        break;
                    case plength_t::L:
                        if constexpr( !std::is_same_v<float, U> &&
                                      !std::is_same_v<double, U> &&
                                      !std::is_same_v<long double, U> ) {
                        } else {
                            return pc2.setError(__LINE__);
                        }
                        break;
                    default:
                        return pc2.setError(__LINE__);
                }
                return pc2;
            }
        };

        constexpr const PResult checkRec(const PResult ctx) noexcept {
            return Parser<no_type_t>::parseOne(ctx);
        }
        
        template <typename Targ, typename... Tnext>
        constexpr const PResult checkRec(const PResult ctx) noexcept {
            if constexpr( 0 < sizeof...(Tnext) ) {
                return checkRec<Tnext...>( Parser<Targ>::parseOne(ctx) );
            } else {
                return Parser<no_type_t>::parseOne( Parser<Targ>::parseOne(ctx) );
            }
        }

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
        return !impl::checkRec<Targs...>( PResult(fmt) ).error();
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
        return !impl::checkRec<Targs...>( PResult(fmt) ).error();
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
    constexpr const PResult checkR(const std::string_view fmt, const Targs &...) noexcept {
        return impl::checkRec<Targs...>( PResult(fmt) );
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
    constexpr const PResult checkR2(const std::string_view fmt) noexcept {
        return impl::checkRec<Targs...>( PResult(fmt) );
    }

    /**@}*/

} // namespace jau::cfmt2

#endif  // JAU_STRING_CFMT2_HPP_
