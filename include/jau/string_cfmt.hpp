/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021-2026 Gothel Software e.K.
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
#include <concepts>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <numbers>
#include <ostream>
#include <string>
#include <string_view>
#include <type_traits>
#include <iostream>

#include <jau/base_math.hpp>
#include <jau/byte_util.hpp>
#include <jau/cpp_lang_util.hpp>
#include <jau/int_math.hpp>
#include <jau/int_types.hpp>
#include <jau/exceptions.hpp>
#include <jau/float_types.hpp>
#include <jau/packed_attribute.hpp>
#include <jau/type_traits_queries.hpp>
#include <jau/type_concepts.hpp>
#include <jau/string_util.hpp>
#include <jau/cpp_pragma.hpp>
#include <jau/enum_util.hpp>

/**
 * @anchor jau_cfmt_header
 * ## `jau::cfmt`, a `snprintf` compliant runtime string format and compile-time validator
 *
 * ### Features
 * - Compile type validation of arguments against the format string
 *   - Possible via `consteval` capable `constexpr` implementation
 *   - Live response via `static_assert` expressions within your IDE
 *   - Example using `jau_string_check(fmt, ...)` macro utilizing `jau::cfmt::check2`
 *     The macro resolves the passed arguments types via `decltype` to be utilized for `jau::cfmt::check2`
 *   ```
 *     jau_string_check("Hello %s %u", "World", 2.0); // shows static_assert() argument error for argument 2 (float, not unsigned integral)
 *     jau_string_checkLine("Hello %s %u", "World", 2.0); // shows static_assert() source-line error for argument 2 (float, not unsigned integral)
 *   ```
 * -  Runtime safe string formatting via `jau::format_string`
 *   ```
 *     std::string s0 = "World";
 *     std::string s1 = jau::format_string("Hello %s, %d + %d = %'d", s0, 1, 1, 2000);
 *     std::string s3 = jau::format_string_h(100, "Hello %s, %d + %d = %'d", s0, 1, 1, 2000); // using a string w/ reserved size of 100
 *
 *     // string concatenation, each `formatR` appends to the given referenced string
 *     std::string concat;
 *     concat.reserve(1000);
 *     jau::cfmt::formatR(concat, "Hello %s, %d + %d = %'d", s0, 1, 1, 2000);
 *     ...
 *     jau::cfmt::formatR(concat, "%#b", 2U);
 *   ```
 * - Both, compile time check and runtime formatting via `jau_format_string` macro (the actual goal)
 *   ```
 *     std::string s1 = jau_format_string("Hello %s, %d + %d = %'d", s0, 1, 1, 2000);
 *     std::string s2 = jau_format_string_h(100, "Hello %s, %d + %d = %'d", s0, 1, 1, 2000); // using a string w/ reserved size of 100
 *   ```
 * - Compatible with [C++ Reference](https://en.cppreference.com/w/cpp/io/c/fprintf)
 *
 * ### Type Conversion
 * Implementation follows type conversion rules as described
 * in [Variadic Default Conversion](https://en.cppreference.com/w/cpp/language/variadic_arguments#Default_conversions)
 * - float to double promotion
 * - bool, char, short, and unscoped enumerations are converted to int or wider integer types,
 *   see also [va_arg](https://en.cppreference.com/w/cpp/utility/variadic/va_arg)
 * - void pointer tolerance
 * - Exception signedness conversion
 *   - Only allow positive signed to unsigned type conversion, otherwise fails (intention, see below)
 *
 * ### Implementation Details
 * General
 * - Validates argument types against format string at compile time (consteval)
 * - Formats resulting string using argument values against format string at runtime
 * - Written in C++20 using template argument pack w/ save argument type checks
 * - Mostly written as constexpr, capable to be utilized at compile-time (consteval).
 *
 * Behavior
 * - `nullptr` conversion value similar to `glibc`
 *   - string produces `(null)`
 *   - pointer produces `(nil)`
 * - Signedness Conversion
 *   - Not accepting `unsigned` -> `signed` conversion to avoid overflow (compile time check)
 *   - Not accepting negative integral value for `unsigned` (runtime check)
 *   - Not accepting over- or underflow, including sign to/from unsigned conversion,
 *     i.e. safe signedness conversion.
 * - Accept given type <= integral target type, conversion to wider types
 * - Accept `enum` types for integer conversion.
 *   - Only if underlying type is `unsigned`, it can't be used for signed integer conversion (see above)
 * - Accept direct std::string and std::string_view for `%s` string arguments
 *
 * #### Supported Format String
 *
 * `%[flags][width][.precision][length modifier]conversion`
 *
 * ##### Flags
 * The following flags are supported
 * - `#`: hash, C99. Adds leading prefix for `radix != 10`.
 * - `0`: zeropad, C99
 * - `-`: left, C99
 * - ` `: space, C99
 * - `+`: plus, C99
 * - `'`: thousands, POSIX
 * - `,`: thousands, OpenJDK (alias for Java users)
 *
 * ##### Width and Precision
 * Width and precision also supports `*` to use the next argument for its value.
 *
 * However, `*m$` (decimal integer `m`) for the `m`-th argument is not yet supported.
 *
 * ##### Length Modifiers
 * The following length modifiers are supported
 * - `hh` [unsigned] char, ignored for floating point
 * - `h` [unsigned] short, ignored for floating point
 * - `l` [unsigned] long, ignored for floating point
 * - `ll` [unsigned] long long
 * - `q` deprecated synonym for `ll`
 * - 'L' long double
 * - 'j' uintmax_t or intmax_t
 * - 'z' size_t or ssize_t
 * - `Z` deprecated synonym for `z`
 * - 't' ptrdiff_t
 *
 * ##### Conversion Specifiers
 * The following standard conversion specifiers are supported:
 * - Basic
 *   - `c` character
 *   - `s` string
 *   - `p` pointer
 *   - `d` signed integral or `i`
 * - Unsigned integral
 *   - `o` octal unsigned
 *   - `x` `X` hexadecimal unsigned low and capital chars
 *   - `b` binary unsigned presentation (extension)
 *   - `u` decimal unsigned
 * - Floating point
 *   - `f` or `F` double floating point
 *   - `e`, `E` exponential low- and capital E
 *   - `g`, `G` alternate exponential low- and capital E
 *   - `a`, `A` hexadecimal low- and capital chars
 * - Aliases
 *   - `i` -> `d`
 *   - `F` -> `f`
 *
 * ###### Extended conversion specifier
 * - `b` bitpattern of unsigned integral w/ prefix `0b` (if `#` flag is added)
 *
 * ### Special Thanks
 * To the project [A printf / sprintf Implementation for Embedded Systems](https://github.com/mpaland/printf)
 * worked on by Marco Paland and many others. I have used their `test_suite.cpp` code within our unit test
 * `test_stringfmt_format.cpp` and also utilized their floating point parsing within
 * `append_float` and `append_efloat`.
 *
 * ### Further Documentation
 * - [C++ fprintf Reference](https://en.cppreference.com/w/cpp/io/c/fprintf)
 * - [Linux snprintf(3) man page](https://www.man7.org/linux/man-pages/man3/snprintf.3p.html)
 * - [FreeBSD snprintf(3) man page](https://man.freebsd.org/cgi/man.cgi?snprintf(3))
 * - [C++20 idioms for parameter packs](https://www.scs.stanford.edu/~dm/blog/param-pack.html)
 * - [A printf / sprintf Implementation for Embedded Systems](https://github.com/mpaland/printf)
 */
namespace jau::cfmt {
    using namespace jau::enums;

    /** \addtogroup StringUtils
     *
     *  @{
     */

    /// Maximum net number string len w/o EOS, up to uint64_t
    constexpr inline const size_t num_max_slen = 31;

    /// Default string reserved capacity w/o EOS (511)
    constexpr inline const size_t default_string_capacity = 511;

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

    /// Format flags
    enum class flags_t : uint16_t {
        none        = 0,                ///< no flag

        hash        = (uint16_t)1 << 1, ///< actual flag `#`, C99
        zeropad     = (uint16_t)1 << 2, ///< actual flag `0`, C99
        left        = (uint16_t)1 << 3, ///< actual flag `-`, C99
        space       = (uint16_t)1 << 4, ///< actual flag ` `, C99
        plus        = (uint16_t)1 << 5, ///< actual flag `+`, C99
        thousands   = (uint16_t)1 << 6, ///< actual flag `'`, POSIX

        uppercase   = (uint16_t)1 << 8  ///< uppercase, via conversion spec
    };
    JAU_MAKE_BITFIELD_ENUM_STRING(flags_t, hash, zeropad, left, space, plus, thousands, uppercase);

    /// Format length modifiers
    enum class plength_t {
        none,
        hh,  ///< char integer
        h,   ///< short integer
        l,   ///< long integer
        ll,  ///< long long integer
        L,   ///< long double float
        j,   ///< intmax_t or uintmax_t integer
        z,   ///< size_t or ssize_t integer
        t    ///< ptrdiff_t
    };
    JAU_MAKE_ENUM_STRING(plength_t, hh, h, l, ll, L, j, z, t);

    /// Format conversion specifier (fully defined w/ radix)
    enum class cspec_t : uint16_t {
        none,             ///< none
        character,        ///< `c`
        string,           ///< `s`
        pointer,          ///< `p`
        signed_int,       ///< `d` or `i`
        unsigned_int,     ///< `o`, `x` or `X`, `u`, `b`
        floating_point,   ///< `f` or `F`
        exp_float,        ///< `e` or `E`
        alt_float,        ///< `g` or `G`
        hex_float,        ///< `a` or `A`
    };
    JAU_MAKE_ENUM_STRING(cspec_t, character, string, pointer, signed_int, unsigned_int,
                         floating_point, exp_float, alt_float, hex_float);

    struct FormatOpts {
        std::string_view fmt;
        flags_t flags;
        size_t width;
        bool width_set;
        size_t precision;
        bool precision_set;
        plength_t length_mod;
        cspec_t conversion;
        nsize_t radix;

        constexpr FormatOpts() noexcept
        : fmt(), flags(flags_t::none),
          width(0), width_set(false),
          precision(0), precision_set(false),
          length_mod(plength_t::none),
          conversion(cspec_t::none), radix(0) { }

        constexpr void setWidth(size_t v) { width = v; width_set = true; }
        constexpr void setPrecision(size_t v) { precision = v; precision_set = true; }
        constexpr bool addFlag(char c) noexcept {
            switch( c ) {
                case '#':  flags |= flags_t::hash; break;
                case '0':  flags |= flags_t::zeropad; break;
                case '-':  flags |= flags_t::left; break;
                case ' ':  flags |= flags_t::space; break;
                case '+':  flags |= flags_t::plus; break;
                case '\'': flags |= flags_t::thousands; break;
                case ',':  flags |= flags_t::thousands; break;
                default:   return false;  // done, not a flag
            }
            return true; // a flag
        }
        constexpr void validateFlags() noexcept {
            switch (conversion) {
                case cspec_t::unsigned_int:
                    // plus or space flag only for signed conversions
                    flags &= ~(flags_t::plus | flags_t::space);
                    [[fallthrough]];
                case cspec_t::signed_int:
                    if (precision_set) {
                        flags &= ~flags_t::zeropad;
                    }
                    break;
                default:
                    break;
            }  // switch( fmt_literal )
            if (is_set(flags, flags_t::left)) {
                flags &= ~flags_t::zeropad;
            }
            if (is_set(flags, flags_t::plus)) {
                flags &= ~flags_t::space;
            }
            if( 10 == radix ) {
                flags &= ~flags_t::hash;
            }
        }

        constexpr bool setConversion(char fmt_literal) noexcept {
            radix = 10; // default
            switch (fmt_literal) {
                case 'c':
                    conversion = cspec_t::character;
                    break;
                case 's':
                    conversion = cspec_t::string;
                    break;
                case 'p':
                    radix = 16;
                    flags |= flags_t::hash;
                    conversion = cspec_t::pointer;
                    break;

                case 'd':
                case 'i':
                    conversion = cspec_t::signed_int;
                    break;

                case 'o':
                    radix = 8;
                    conversion = cspec_t::unsigned_int;
                    break;

                case 'X':
                    flags |= flags_t::uppercase;
                    [[fallthrough]];
                case 'x':
                    radix = 16;
                    conversion = cspec_t::unsigned_int;
                    break;

                case 'u':
                    conversion = cspec_t::unsigned_int;
                    break;
                case 'b':
                    radix = 2;
                    conversion = cspec_t::unsigned_int;
                    break;

                case 'F':
                    flags |= flags_t::uppercase;
                    [[fallthrough]];
                case 'f':
                    conversion = cspec_t::floating_point;
                    break;

                case 'E':
                    flags |= flags_t::uppercase;
                    [[fallthrough]];
                case 'e':
                    conversion = cspec_t::exp_float;
                    break;

                case 'G':
                    flags |= flags_t::uppercase;
                    [[fallthrough]];
                case 'g':
                    conversion = cspec_t::alt_float;
                    break;

                case 'A':
                    flags |= flags_t::uppercase;
                    [[fallthrough]];
                case 'a':
                    conversion = cspec_t::hex_float;
                    break;

                default:
                    return false;
            }  // switch( fmt_literal )
            validateFlags();
            return true;
        }

        /// Reconstructs format string
        std::string toFormat() const {
            std::string s;
            s.reserve(31);
            s.append("%");
            if( is_set(flags, flags_t::hash) ) { s.append("#"); }
            if( is_set(flags, flags_t::zeropad) ) { s.append("0"); }
            if( is_set(flags, flags_t::left) ) { s.append("-"); }
            if( is_set(flags, flags_t::space) ) { s.append(" "); }
            if( is_set(flags, flags_t::plus) ) { s.append("+"); }
            if( width_set ) {
                s.append(std::to_string(width));
            }
            if( precision_set) {
                s.append(".").append(std::to_string(precision));
            }
            if( plength_t::none != length_mod ) {
                s.append(to_string(length_mod));
            }
            switch( conversion ) {
                case cspec_t::character:
                    s.append("c");
                    break;
                case cspec_t::string:
                    s.append("s");
                    break;
                case cspec_t::pointer:
                    s.append("p");
                    break;
                case cspec_t::signed_int:
                    s.append("d");
                    break;
                case cspec_t::unsigned_int: {
                        if( 16 == radix ) {
                            s.append( is_set(flags, flags_t::uppercase) ? "X" : "x" );
                        } else if( 8 == radix ) {
                            s.append("o");
                        } else if( 2 == radix ) {
                            s.append("b");
                        } else {
                            s.append("u");
                        }
                    } break;
                case cspec_t::floating_point:
                    s.append( is_set(flags, flags_t::uppercase) ? "F" : "f" );
                    break;
                case cspec_t::exp_float:
                    s.append( is_set(flags, flags_t::uppercase) ? "E" : "e" );
                    break;
                case cspec_t::hex_float:
                    s.append( is_set(flags, flags_t::uppercase) ? "A" : "a" );
                    break;
                case cspec_t::alt_float:
                    s.append( is_set(flags, flags_t::uppercase) ? "G" : "g" );
                    break;
                default:
                    s.append("E");
                    break;
            }  // switch( fmt_literal )
            return s;
        }

        constexpr void reset() noexcept {
            fmt = std::string_view();
            flags = flags_t::none;
            width = 0;
            width_set = false;
            precision = 0;
            precision_set = false;
            length_mod = plength_t::none;
            conversion = cspec_t::none;
            radix = 0;
        }

        std::string toString() const {
            std::string s = "fmt `";
            s.append(fmt).append("` -> `").append(toFormat())
             .append("`, flags ")
             .append(to_string(flags))
             .append(", width ");
            if( width_set ) {
                s.append(std::to_string(width));
            } else {
                s.append("no");
            }
            s.append(", precision ");
            if( precision_set ) {
                s.append(std::to_string(precision));
            } else {
                s.append("no");
            }
            s.append(", length `")
             .append(to_string(length_mod))
             .append("`, cspec ").append(to_string(conversion))
             .append(", radix ").append(std::to_string(radix));
            return s;
        }
    };

    class Result {
      private:
        std::string_view m_fmt;
        FormatOpts m_opts;
        size_t m_pos;        ///< position of next fmt character to be read
        ssize_t m_arg_count;
        int m_line;
        bool m_success; ///< true if operation was successful, otherwise indicates error

      public:
        constexpr Result(std::string_view f, FormatOpts o, size_t pos, ssize_t acount, int line, bool ok)
        : m_fmt(f), m_opts(o), m_pos(pos), m_arg_count(acount), m_line(line), m_success(ok) {}

        /// true if operation was successful, otherwise indicates error
        constexpr bool success() const noexcept { return m_success; }

        /// Arguments processed
        constexpr ssize_t argumentCount() const noexcept { return m_arg_count; }

        /// format string_view
        constexpr const std::string_view& fmt() const noexcept { return m_fmt; }

        /// Last argument FormatOpts (error analysis)
        constexpr const FormatOpts& opts() const noexcept { return m_opts; }
        /// Position of next fmt character to be read (error analysis)
        constexpr size_t pos() const noexcept { return m_pos; }
        /// error line of implementation source code or zero if success (error analysis)
        constexpr int errorLine() const noexcept { return m_line; }

        std::string toString() const {
            const char c = m_pos < m_fmt.length() ? m_fmt[m_pos] : '@';
            std::string s = "args ";
            s.append(std::to_string(m_arg_count))
            .append(", ok ")
            .append(jau::to_string(m_success))
            .append(", line ")
            .append(std::to_string(m_line))
            .append(", pos ")
            .append(std::to_string(m_pos))
            .append(", char `")
            .append(std::string(1, c))
            .append("`, last[").append(m_opts.toString())
            .append("], fmt `").append(m_fmt)
            .append("`");
            return s;
        }
    };

    inline std::ostream &operator<<(std::ostream &out, const Result &pc) {
        out << pc.toString();
        return out;
    }

    namespace impl {
        inline constexpr const size_t float_charbuf_maxlen = 32;
        inline constexpr const size_t default_float_precision = 6;
        inline constexpr const double_t max_append_float = (double_t)1e9;

        inline void append_rev(std::string &dest, const size_t dest_maxlen, std::string_view src, bool prec_cut, bool reverse, const FormatOpts &opts) noexcept
        {
            if (!dest_maxlen) {
                return;
            }
            size_t src_len = src.size();
            const char *p = src.data();
            const size_t dest_start_len = dest.size();

            // pre padding
            if (prec_cut && opts.precision_set) {
                src_len = std::min(src_len, opts.precision);
            }
            size_t space_left = 0, space_right = 0;
            {
                // string optional re-capacity and resize
                const size_t maxlen = dest_maxlen - dest_start_len;
                size_t len = std::min(src_len, maxlen); // already cut to precision if applicable
                if (!is_set(opts.flags, flags_t::left) && opts.width_set && opts.width > len) {
                    space_left = std::min(opts.width-len, maxlen-len);
                    len += space_left;
                }
                // p2: append pad spaces left/right up to given width
                if (opts.width_set && len < opts.width) {
                    if (is_set(opts.flags, flags_t::left)) {
                        space_right = std::min(opts.width-len, maxlen-len);
                        len += space_right;
                    } else if (!is_set(opts.flags, flags_t::zeropad)) {
                        space_left = std::min(opts.width-len, maxlen-len);
                        len += space_left;
                    }
                }
                size_t new_size = dest_start_len + len;
                dest.reserve(new_size + 1); // +EOS, not shrinking!
                dest.resize(new_size, ' ');
            }
            char *d_left = dest.data() + dest_start_len + space_left;
            char *d_end = dest.data() + dest.size() - space_right;
            assert(d_left <= d_end);

            // string
            if (!reverse) {
                std::memcpy(d_left, p, std::min<size_t>(src_len, d_end-d_left));
                // std::copy(p, p+(d_end-d), d);
            } else {
                while (d_left<d_end) {
                    *(--d_end) = *(p++);
                }
            }
            assert(d_left <= d_end);

            *(dest.data()+dest.size()) = 0; // EOS (is reserved)
        }
        inline void append_string(std::string &dest, const size_t dest_maxlen, std::string_view src, const FormatOpts &opts) noexcept
        {
            append_rev(dest, dest_maxlen, src, true /*prec*/, false /*rev**/, opts);
        }

        template<std::integral value_type, bool inject_dot=false>
        void append_integral(std::string &dest, const size_t dest_maxlen, const value_type val, const FormatOpts &opts) noexcept
        {
            if (!dest_maxlen) {
                return;
            }
            const size_t dest_start_len = dest.size();

            const nsize_t radix = opts.radix;
            nsize_t shift;
            switch ( radix ) {
                case 16: shift = 4; break;
                case 10: shift = 0; break;
                case 8:  shift = 3; break;
                case 2:  shift = 1; break;
                default: return;
            }
            typedef std::make_unsigned_t<value_type> unsigned_value_type;
            unsigned_value_type v = unsigned_value_type( jau::abs(val) );
            const bool negative = !jau::is_positive(val);
            const char *hex_array = is_set(opts.flags, flags_t::uppercase) ? HexadecimalArrayBig : HexadecimalArrayLow;
            const nsize_t mask = radix - 1;  // ignored for radix 10
            const char separator = is_set(opts.flags, flags_t::thousands) ? '\'' : 0;
            const nsize_t sep_gap = 10 == radix ? 3 : 4;
            const nsize_t val_digits = opts.precision_set && opts.precision == 0 && jau::is_zero(v) ? 0 : jau::digits<unsigned_value_type>(v, radix);
            const nsize_t sep_count = val_digits > 0 && separator ? (val_digits - 1) / sep_gap : 0;
            const size_t prec = opts.precision_set ? opts.precision : 0;
            constexpr static const nsize_t xtra_dot = inject_dot ? 1 : 0;
            size_t width = opts.width_set ? opts.width : 0;
            size_t zeros_left = 0, space_left = 0, space_right = 0;
            size_t xtra_left = 0; ///< contains hash, sign and prec_left and single space
            {
                size_t len = val_digits + xtra_dot + sep_count; // current total of the number string
                // p1: pad leading zeros
                if (!is_set(opts.flags, flags_t::left)) {
                    if (width && is_set(opts.flags, flags_t::zeropad) && (negative || has_any(opts.flags, flags_t::plus | flags_t::space))) {
                        --width;
                    }
                    if( len < prec ) {
                        zeros_left = prec - len;
                        xtra_left += zeros_left; len += zeros_left;
                    }
                    if (len < width && is_set(opts.flags, flags_t::zeropad)) {
                        size_t n =  width - len;
                        zeros_left += n;
                        xtra_left += n; len += n;
                    }
                }

                // p1: handle hash
                if (is_set(opts.flags, flags_t::hash)) {
                    if (!opts.precision_set && len && ((len == prec) || (len == width))) {
                        --xtra_left; --len;
                        if (zeros_left) { --zeros_left; }
                        if (len && (radix == 16)) {
                            --xtra_left; --len;
                            if (zeros_left) { --zeros_left; }
                        }
                    }
                    if (radix == 16 || radix == 2) {
                        ++xtra_left; ++len;
                    }
                    ++xtra_left; ++len; // hash zero
                }

                // p1: sign
                if (negative) {
                    ++xtra_left; ++len; // '-';
                } else if (is_set(opts.flags, flags_t::plus)) {
                    ++xtra_left; ++len; // '+';  // ignore the space if the '+' exists
                }

                // p1: space
                if (!negative && is_set(opts.flags, flags_t::space)) {
                    ++xtra_left; ++len; // ' ';
                }

                // p2: append pad spaces left/right up to given width
                if (len < width) {
                    if (is_set(opts.flags, flags_t::left)) {
                        space_right = width - len; len += space_right;
                    } else if (!is_set(opts.flags, flags_t::zeropad)) {
                        space_left = width - len; len += space_left;
                    }
                }

                const size_t added_maxlen = dest_maxlen - dest_start_len;
                const size_t added_len = std::min<size_t>(added_maxlen, val_digits + xtra_dot + sep_count + xtra_left + space_left + space_right);
                dest.reserve(dest_start_len + added_len + 1); // +EOS, not shrinking!
                dest.resize(dest_start_len + added_len, ' ');

#if !defined(NDEBUG) && 0
                fprintf(stderr, "XXX.80: opts: %s\n", opts.toString().c_str());
                if( negative ) {
                    fprintf(stderr, "XXX.80: val %zd, abs %zu\n", (ssize_t)val, (size_t)v);
                } else {
                    fprintf(stderr, "XXX.80: val %zu, abs %zu\n", (size_t)val, (size_t)v);
                }
                fprintf(stderr, "XXX.80: idx[digits %zu, sep %zu, xleft %zu (zeros %zu)], space[l %zu, r %zu] -> len %zu\n",
                    val_digits, sep_count, xtra_left, zeros_left, space_left, space_right, len);
                fprintf(stderr, "XXX.80: total len[old %zu, added %zu, len %zu], number_start %zu\n", dest_start_len, added_len, dest.size(), xtra_left+space_left);
#endif
            }
            const size_t dest_len = dest.size();
            const char * const d_start = dest.data() + dest_start_len;
            const char * const d_start_num = d_start + space_left + xtra_left;
            char *d = dest.data() + dest_len - space_right;
            const char * const d_end_num = d;
#if !defined(NDEBUG) && 0
            fprintf(stderr, "XXX.80: total len %zu, d_start_num %zd - d_end_num %zd (num_len %zd)\n",
                dest.size()-dest_start_len, d_start_num-d_start, d_end_num-d_start, d_end_num - d_start_num);
#endif

            assert(d_end_num >= d_start_num);
            assert(size_t(d_end_num - d_start_num) == val_digits + xtra_dot + sep_count);
            assert(d >= d_start_num);
            assert(d >= d_start);

            nsize_t digit_cnt = 0, separator_idx = 0;
            while ( d > d_start_num) {
                if ( separator_idx < sep_count && 0 < digit_cnt && 0 == digit_cnt % sep_gap ) {
                    *(--d) = separator;
                    ++separator_idx;
                }
                assert(d > d_start_num);
                // if ( d > d_start_num ) {
                    assert(digit_cnt < val_digits);
                    if ( 10 == radix ) {
                        *(--d) = '0' + (v % 10);
                        v /= 10;
                    } else {
                        *(--d) = hex_array[v & mask];
                        v >>= shift;
                    }
                    ++digit_cnt;
                    if constexpr (xtra_dot) {
                        if (d == d_start_num + 1 + xtra_dot) {
                            *(--d) = '.';
                        }
                    }
                    // }
            }
            assert(d == d_start_num);
            assert(d >= d_start);

            // p1: pad leading zeros (prec_left + space_left)
            assert(d_start <= d_start_num - zeros_left);
            if (zeros_left) {
                std::memset(d-zeros_left, '0', zeros_left);
                // std::fill(d-zeros_left, d, '0');
                d -= zeros_left;
            }

            // p1: handle hash
            if (d > d_start && is_set(opts.flags, flags_t::hash)) {
                size_t len = d_end_num - d; // total length so far
                if (!opts.precision_set && len && ((len == prec) || (len == width))) {
                    ++d; --len;
                    if (len && (radix == 16)) {
                        ++d; --len;
                    }
                }
                assert(d > d_start);
                if (radix == 16) {
                    *(--d) = is_set(opts.flags, flags_t::uppercase) ? 'X' : 'x';
                } else if (radix == 2) {
                    *(--d) = 'b';
                }

                assert(d > d_start);
                *(--d) = '0';
            }
            assert(d >= d_start);

            if (negative) {
                assert(d > d_start);
                *(--d) = '-';
            } else if (is_set(opts.flags, flags_t::plus)) {
                assert(d > d_start);
                *(--d) = '+';  // ignore the space if the '+' exists
            } else if (is_set(opts.flags, flags_t::space)) {
                assert(d > d_start);
                *(--d) = ' ';
            }
#if !defined(NDEBUG) && 0
            if(d != d_start + space_left) {
                fprintf(stderr, "ERROR d %p, d_start %p, space_left %zu, dist %zd",
                    d, d_start, space_left, d_start + space_left - d);
            }
#endif
            assert(d == d_start + space_left); // string space fully written

            *(dest.data()+dest_len) = 0; // EOS (is reserved)
        }

        // check for NaN and special values
        template<std::floating_point value_type>
        inline bool is_float_valid(std::string &dest, const size_t dest_maxlen, const value_type value, const FormatOpts &opts) noexcept {
            const bool up = is_set(opts.flags, flags_t::uppercase);
            if (value != value) {
                append_string(dest, dest_maxlen, up ? "NAN" : "nan", opts);
                return false;
            } else if (value < -std::numeric_limits<value_type>::max()) {
                append_string(dest, dest_maxlen, up ? "-INF" : "-inf", opts);
                return false;;
            } else if (value > std::numeric_limits<value_type>::max()) {
                const bool plus = is_set(opts.flags, flags_t::plus);
                append_string(dest, dest_maxlen, plus ? ( up ? "+INF" : "+inf" ) : ( up ? "INF" : "inf" ), opts);
                return false;
            }
            return true;
        }
        template<>
        inline bool is_float_valid<double>(std::string &dest, const size_t dest_maxlen, const double value, const FormatOpts &opts) noexcept {
            const uint64_t r = jau::bit_value_raw( value );
            const bool up = is_set(opts.flags, flags_t::uppercase);
            if (r == jau::double_iec559_nan_bitval) {
                append_string(dest, dest_maxlen, up ? "NAN" : "nan", opts);
                return false;
            } else if (r == jau::double_iec559_negative_inf_bitval) {
                append_string(dest, dest_maxlen, up ? "-INF" : "-inf", opts);
                return false;;
            } else if (r == jau::double_iec559_positive_inf_bitval) {
                const bool plus = is_set(opts.flags, flags_t::plus);
                append_string(dest, dest_maxlen, plus ? ( up ? "+INF" : "+inf" ) : ( up ? "INF" : "inf" ), opts);
                return false;
            }
            return true;
        }

        template<std::floating_point ivalue_type>
        void append_float(std::string &dest, const size_t dest_maxlen, const ivalue_type ivalue, const FormatOpts &opts) noexcept;

        template<std::floating_point ifloat_type>
        void append_afloat(std::string &dest, const size_t dest_maxlen, const ifloat_type ivalue, const FormatOpts &iopts) noexcept {
            using namespace jau::float_literals;

            if (!dest_maxlen) {
                return;
            }
            if( !is_float_valid(dest, dest_maxlen, ivalue, iopts)) {
                return;
            }
            typedef double float_type; // enforce 64bit only double type (spec)
            constexpr const int significand_shift = sizeof(float_type) > sizeof(ifloat_type) ? 8 * ( sizeof(float_type) - sizeof(ifloat_type) ) - 4 : 0;

            // determine the sign
            const bool negative = ivalue < 0;
            float_type value = float_type(negative ? -ivalue : ivalue);

            // default precision
            size_t prec = iopts.precision_set ? iopts.precision : default_float_precision;

            uint64_t significand = jau::significand_raw(value) >> significand_shift;
            const int32_t expval = jau::exponent_unbiased(value);

            // the exponent format is "%+03d" and largest value is "307", so set aside 4-5 characters
            unsigned int minwidth = ((expval < 100) && (expval > -100)) ? 4U : 5U;
            FormatOpts fopts;

            // will everything fit?
            const size_t width = iopts.width_set ? iopts.width : 0;
            unsigned int fwidth = width;
            if (width > minwidth) {
                // we didn't fall-back so subtract the characters required for the exponent
                fwidth -= minwidth;
            } else {
                // not enough characters, so go back to default sizing
                fwidth = 0U;
            }
            if (is_set(iopts.flags, flags_t::left) && minwidth) {
                // if we're padding on the right, DON'T pad the floating part
                fwidth = 0U;
            }

#if !defined(NDEBUG) && 0
            fprintf(stderr, "AAA.10: v %f, frac %" PRIx64 ", expval %d, dest '%s' (len %zu), iopts %s\n",
                ivalue, significand, expval, dest.c_str(), dest.size(), iopts.toString().c_str());
#endif

            const size_t start_idx = dest.size();
            // output the floating part
            {
                fopts.conversion = cspec_t::signed_int;
                fopts.radix = 16;
                fopts.flags = iopts.flags | flags_t::hash;

                if(iopts.precision_set) {
                    fopts.precision_set = true;
                }
                if(fopts.precision_set) {
                    fopts.precision = prec;
                } else {
                    fopts.precision = 0;
                }
                fopts.width_set = true;
                fopts.width = fwidth;
#if !defined(NDEBUG) && 0
                fprintf(stderr, "AAA.31: v %f, frac %" PRIx64 ", expval %d, dest '%s' (len %zu), fopts %s\n",
                    ivalue, significand, expval, dest.c_str(), dest.size(), fopts.toString().c_str());
#endif
                append_integral<decltype(significand), true>(dest, dest_maxlen, significand, fopts);
            }

            // output the exponent part
            if (minwidth) {
                // output the exponential symbol
                {
                    const size_t idx = dest.size();
                    dest.reserve(idx+float_charbuf_maxlen+1); // add EOS
                    dest.resize(idx+1, ' ');
                    dest[idx] = is_set(iopts.flags, flags_t::uppercase) ? 'P' : 'p';
                }
                // output the exponent value
                fopts.conversion = cspec_t::unsigned_int;
                fopts.radix = 10;
                fopts.flags = flags_t::plus;
                fopts.precision_set = false;
                fopts.precision = 0;
                fopts.width_set = false;
                fopts.width = 0;
#if !defined(NDEBUG) && 0
                fprintf(stderr, "AAA.32: v %f, frac %" PRIx64 ", expval %d, dest '%s' (len %zu), fopts %s\n",
                    ivalue, significand, expval, dest.c_str(), dest.size(), fopts.toString().c_str());
#endif
                append_integral(dest, dest_maxlen, expval, fopts);
                // might need to right-pad spaces
                if (is_set(iopts.flags, flags_t::left)) {
                    const size_t idx = dest.size();
                    if (idx - start_idx < width) {
                        size_t space_right = width - ( idx - start_idx );
                        dest.reserve(idx+space_right+1); // add EOS
                        dest.resize(idx+space_right, ' ');
                    }
                }
            }
#if !defined(NDEBUG) && 0
            fprintf(stderr, "AAA.88: expval %d, dest '%s' (len %zu, cap %zu)\n", expval, dest.c_str(), dest.size(), dest.capacity());
#endif
        }

        template<std::floating_point ifloat_type>
        void append_efloat(std::string &dest, const size_t dest_maxlen, const ifloat_type ivalue, const FormatOpts &iopts) noexcept {
            using namespace jau::float_literals;

            if (!dest_maxlen) {
                return;
            }
            if( !is_float_valid(dest, dest_maxlen, ivalue, iopts)) {
                return;
            }
            // either `double` or `long double`
            // typedef jau::float_bytes_t<std::max(sizeof(ivalue_type), sizeof(double))> float_type;
            typedef double float_type; // enforce 64bit only double type (see below)

            // determine the sign
            const bool negative = ivalue < 0;
            float_type value = float_type(negative ? -ivalue : ivalue);

            // default precision
            size_t prec = iopts.precision_set ? iopts.precision : default_float_precision;

            // determine the decimal exponent
            // based on the algorithm by David Gay (https://www.ampl.com/netlib/fp/dtoa.c)
            // NOTE: 64bit double type specific
            union {
                uint64_t U;
                double F;
            } conv;

            conv.F = value;
            int expval;
            {
                int exp2 = int((conv.U >> 52U) & 0x07FFU) - 1023; // effectively log2
                conv.U = (conv.U & ((1_u64 << 52U) - 1U)) | (1023_u64 << 52U);  // drop the exponent so conv.F is now in [1,2)
                // now approximate log10 from the log2 integer part and an expansion of ln around 1.5
                expval = int(0.1760912590558 + exp2 * 0.301029995663981 + (conv.F - 1.5) * 0.289529654602168);
                // now we want to compute 10^expval but we want to be sure it won't overflow
                exp2 = int(expval * 3.321928094887362 + 0.5); // NOLINT(bugprone-incorrect-roundings)
                const double z = expval * std::numbers::ln10 - exp2 * std::numbers::ln2;
                const double z2 = z * z;
                conv.U = (uint64_t)(exp2 + 1023) << 52U;
                // compute exp(z) using continued fractions, see https://en.wikipedia.org/wiki/Exponential_function#Continued_fractions_for_ex
                conv.F *= 1 + 2 * z / (2 - z + (z2 / (6 + (z2 / (10 + z2 / 14)))));
            }
            // correct for rounding errors
            if (value < conv.F) {
                expval--;
                conv.F /= 10;
            }

            // the exponent format is "%+03d" and largest value is "307", so set aside 4-5 characters
            unsigned int minwidth = ((expval < 100) && (expval > -100)) ? 4U : 5U;
            FormatOpts fopts;

            // in "%g" mode, "prec" is the number of *significant figures* not decimals
            if (cspec_t::alt_float == iopts.conversion) {
                // do we want to fall-back to "%f" mode?
                if ((value >= 1e-4) && (value < 1e6)) {
                    if ((int)prec > expval) {
                        prec = (unsigned)((int)prec - expval - 1);
                    } else {
                        prec = 0;
                    }
                    fopts.precision_set = true; // make sure _ftoa respects precision
                    fopts.precision = prec;
                    // no characters in exponent
                    minwidth = 0U;
                    expval = 0;
                } else {
                    // we use one sigfig for the whole part
                    if ((prec > 0) && iopts.precision_set) {
                        --prec;
                    }
                }
            }

            // will everything fit?
            const size_t width = iopts.width_set ? iopts.width : 0;
            unsigned int fwidth = width;
            if (width > minwidth) {
                // we didn't fall-back so subtract the characters required for the exponent
                fwidth -= minwidth;
            } else {
                // not enough characters, so go back to default sizing
                fwidth = 0U;
            }
            if (is_set(iopts.flags, flags_t::left) && minwidth) {
                // if we're padding on the right, DON'T pad the floating part
                fwidth = 0U;
            }

            // rescale the float value
            if (expval) {
                value /= conv.F;
            }

#if !defined(NDEBUG) && 0
            fprintf(stderr, "EEE.10: expval %d, dest '%s' (len %zu), iopts %s\n", expval, dest.c_str(), dest.size(), iopts.toString().c_str());
#endif

            // output the floating part
            const size_t start_idx = dest.size();
            {
                fopts.conversion = cspec_t::floating_point;
                fopts.radix = 10;
                fopts.flags = iopts.flags;
                if(iopts.precision_set) {
                    fopts.precision_set = true;
                }
                if(fopts.precision_set) {
                    fopts.precision = prec;
                } else {
                    fopts.precision = 0;
                }
                fopts.width_set = true;
                fopts.width = fwidth;
                append_float(dest, dest_maxlen, negative ? -value : value, fopts);
            }

            // output the exponent part
            if (minwidth) {
                // output the exponential symbol
                {
                    const size_t idx = dest.size();
                    dest.reserve(idx+float_charbuf_maxlen+1); // add EOS
                    dest.resize(idx+1, ' ');
                    dest[idx] = is_set(iopts.flags, flags_t::uppercase) ? 'E' : 'e';
                }
                // output the exponent value
                fopts.conversion = cspec_t::unsigned_int;
                fopts.radix = 10;
                fopts.flags = flags_t::zeropad | flags_t::plus;
                fopts.precision_set = false;
                fopts.precision = 0;
                fopts.width_set = true;
                fopts.width = minwidth-1;
#if !defined(NDEBUG) && 0
                fprintf(stderr, "EEE.31: v %f (exp %d), dest '%s' (len %zu), fopts %s\n",
                    value, expval, dest.c_str(), dest.size(), fopts.toString().c_str());
#endif
                append_integral(dest, dest_maxlen, expval, fopts);
                // might need to right-pad spaces
                if (is_set(iopts.flags, flags_t::left)) {
                    const size_t idx = dest.size();
                    if (idx - start_idx < width) {
                        size_t space_right = width - ( idx - start_idx );
                        dest.reserve(idx+space_right+1); // add EOS
                        dest.resize(idx+space_right, ' ');
                    }
                }
            }
#if !defined(NDEBUG) && 0
            fprintf(stderr, "EEE.88: expval %d, dest '%s' (len %zu, cap %zu)\n", expval, dest.c_str(), dest.size(), dest.capacity());
#endif
        }

        template<std::floating_point ifloat_type>
        void append_float(std::string &dest, const size_t dest_maxlen, const ifloat_type ivalue, const FormatOpts &opts) noexcept {
            using namespace jau::float_literals;

            if (!dest_maxlen) {
                return;
            }
            if( !is_float_valid(dest, dest_maxlen, ivalue, opts)) {
                return;
            }
            // either `double` or `long double`
            typedef jau::float_bytes_t<std::max(sizeof(ifloat_type), sizeof(double))> float_type;

            char buf_[float_charbuf_maxlen];
            char *d = buf_;
            const char * const d_start = d;
            const char * const d_end = d + float_charbuf_maxlen;
            float_type diff = 0;

            // powers of 10
            static constexpr const float_type pow10[] = {  1e0_f64,  1e1_f64,  1e2_f64,  1e3_f64,  1e4_f64,  1e5_f64,  1e6_f64,  1e7_f64,  1e8_f64,  1e9_f64,
                                                        1e10_f64, 1e11_f64, 1e12_f64, 1e13_f64, 1e14_f64 };
            static constexpr const size_t prec_max = sizeof(pow10) / sizeof(float_type) - 1; // 14

            // test for negative
            const bool negative = ivalue < 0;
            float_type value = float_type(negative ? -ivalue : ivalue);

            // test for very large values
            // standard printf behavior is to print EVERY whole number digit -- which could be 100s of characters overflowing your buffers == bad
            if ((value > max_append_float) || (value < -max_append_float)) {
                append_efloat(dest, dest_maxlen, ivalue, opts);
                return;
            }

            // set default precision, if not set explicitly
            size_t prec = opts.precision_set ? opts.precision : default_float_precision;

            // limit precision to prec_max, cause a prec > prec_max (14) can lead to overflow errors (orig 9)
            while (prec > prec_max) {
                *(d++) = '0';
                prec--;
            }

            uint64_t whole = (uint64_t)value;
            float_type tmp = (value - whole) * pow10[prec];
            uint64_t frac = (uint64_t)tmp;
            diff = tmp - (float_type)frac;

            if (diff > 0.5) {
                ++frac;
                // handle rollover, e.g. case 0.99 with prec 1 is 1.0
                if (frac >= pow10[prec]) {
                    frac = 0;
                    ++whole;
                }
            } else if (diff < 0.5) {
            } else if ((frac == 0) || (frac & 1)) {
                // if halfway, round up if odd OR if last digit is 0
                ++frac;
            }

#if !defined(NDEBUG) && 0
            fprintf(stderr, "FFF.10: val %f, positive %d, len %zu/%zu, prec %zu/%zu, width %zu: whole %" PRIu64 ", frac %" PRIu64 ", double_t %s\n",
                value, !negative, len, float_charbuf_maxlen, prec, prec_max, width, whole, frac, jau::static_ctti<double_t>().toString().c_str());
#endif

            if (prec == 0) {
                diff = value - (float_type)whole;
                if ((!(diff < 0.5) || (diff > 0.5)) && (whole & 1)) {
                    // exactly 0.5 and ODD, then round up
                    // 1.5 -> 2, but 2.5 -> 2
                    ++whole;
                }
            } else {
                unsigned int count = prec;
                // now do fractional part, as an unsigned number
                if (d < d_end) {
                    do {
                        --count;
                        *(d++) = char('0' + (frac % 10));
                    } while ((frac /= 10) && d < d_end);
                }

                // add extra 0s
                while (d < d_end && count-- > 0) {
                    *(d++) = '0';
                }
                if (d < d_end) {
                    // add decimal
                    *(d++) = '.';
                }
            }

            // do whole part, number is reversed
            if (d < d_end) {
                do {
                    *(d++) = char('0' + (whole % 10));
                } while ((whole /= 10) && d < d_end);
            }

            // pad leading zeros
            size_t width = opts.width_set ? opts.width : 0;
            if (!is_set(opts.flags, flags_t::left) && is_set(opts.flags, flags_t::zeropad)) {
                if (width && (negative || has_any(opts.flags, flags_t::plus | flags_t::space))) {
                    width--;
                }
                while ((d < d_start+width) && d < d_end) {
                    *(d++) = '0';
                }
            }

            if (d < d_end) {
                if (negative) {
                    *(d++) = '-';
                } else if (is_set(opts.flags, flags_t::plus)) {
                    *(d++) = '+';  // ignore the space if the '+' exists
                } else if (is_set(opts.flags, flags_t::space)) {
                    *(d++) = ' ';
                }
            }

            append_rev(dest, dest_maxlen, std::string_view(d_start, d-d_start), false /*prec*/, true /*rev**/, opts);
        }

        template<typename T>
        concept OutputType = requires(T t) {
            { t.maxLen() }      -> std::same_as<size_t>;
            // { t.fits(size_t) }  -> std::same_as<bool>;

            { t.get() }        -> std::same_as<std::string_view>;
        };

        /// A null OutputType for `constexpr` and `consteval` formatting dropping all output
        class NullOutput {
          public:
            constexpr NullOutput() noexcept = default;

            constexpr size_t maxLen() const noexcept { return 0; }
            constexpr bool fits(size_t) const noexcept { return false; }

            std::string_view get() const noexcept { return "(nil)"; }

            template<typename T>
            requires jau::req::stringifyable_jau<T>
            constexpr void appendFormatted(const FormatOpts&, const T&) noexcept { }

            constexpr void appendText(std::string_view ) noexcept { }
            constexpr void appendError(size_t, int , const std::string_view ) noexcept {}
        };

        /// A std::string OutputType for runtime formatting into a std::string
        class StringOutput {
          private:
            std::size_t m_maxLen;
            std::string &m_s;

          public:
            StringOutput(std::size_t maxLen, std::string &s) noexcept : m_maxLen(maxLen), m_s(s) {}

            constexpr size_t maxLen() const noexcept { return m_maxLen; }
            constexpr bool fits(size_t n) const noexcept { return 0 < m_maxLen && n <= m_maxLen - m_s.size(); }

            std::string_view get() const noexcept { return m_s; }

            template<typename T>
            requires jau::req::string_literal<T> || jau::req::string_class<T>
            inline void appendFormatted(const FormatOpts& opts, const T& v) noexcept {
                std::exception_ptr eptr;
                try {
                    impl::append_string(m_s, m_maxLen, v, opts);
                } catch (...) {
                    eptr = std::current_exception();
                }
                handle_exception(eptr);
            }
            template<typename T>
            requires jau::req::char_pointer<T>
            inline void appendFormatted(const FormatOpts& opts, const T& v) noexcept {
                std::exception_ptr eptr;
                try {
                    if( nullptr != v ) {
                        impl::append_string(m_s, m_maxLen, v, opts);
                    } else {
                        impl::append_string(m_s, m_maxLen, "(null)", opts);
                    }
                } catch (...) {
                    eptr = std::current_exception();
                }
                handle_exception(eptr);
            }
            template<typename T>
            requires jau::req::boolean<T>
            inline void appendFormatted(const FormatOpts& opts, const T& v) noexcept {
                std::exception_ptr eptr;
                try {
                    impl::append_string(m_s, m_maxLen, jau::to_string(v), opts);
                } catch (...) {
                    eptr = std::current_exception();
                }
                handle_exception(eptr);
            }
            template<typename T>
            requires jau::req::pointer<T> && (!jau::req::string_alike<T>)
            inline void appendFormatted(const FormatOpts& opts, const T& v) noexcept {
                std::exception_ptr eptr;
                try {
                    if( nullptr != v ) {
                        const uintptr_t v_le = jau::cpu_to_le(reinterpret_cast<uintptr_t>(v));
                        impl::append_integral(m_s, m_maxLen, v_le, opts);
                    } else {
                        impl::append_string(m_s, m_maxLen, "(nil)", opts);
                    }
                } catch (...) {
                    eptr = std::current_exception();
                }
                handle_exception(eptr);
            }
            template<typename T>
            requires std::is_integral_v<T> && (!jau::req::boolean<T>)
            inline void appendFormatted(const FormatOpts& opts, const T& v) noexcept {
                std::exception_ptr eptr;
                try {
                    impl::append_integral(m_s, m_maxLen, v, opts);
                } catch (...) {
                    eptr = std::current_exception();
                }
                handle_exception(eptr);
            }
            template<typename T>
            requires std::is_floating_point_v<T>
            inline void appendFormatted(const FormatOpts& opts, const T& v) noexcept {
                std::exception_ptr eptr;
                try {
                    if( opts.conversion == cspec_t::floating_point ) {
                        impl::append_float(m_s, m_maxLen, v, opts);
                    } else if( opts.conversion == cspec_t::hex_float ) {
                        impl::append_afloat(m_s, m_maxLen, v, opts);
                    } else {
                        // cspec_t::exp_float, cspec_t::alt_float
                        impl::append_efloat(m_s, m_maxLen, v, opts);
                    }
                } catch (...) {
                    eptr = std::current_exception();
                }
                handle_exception(eptr);
            }
            inline void appendText(const std::string_view v) noexcept {
                if( fits(v.size()) ) {
                    std::exception_ptr eptr;
                    try {
                        m_s.append(v);
                    } catch (...) {
                        eptr = std::current_exception();
                    }
                    handle_exception(eptr);
                }
            }
            inline void appendError(ssize_t argIdx, int line, const std::string_view tag) noexcept {
                std::exception_ptr eptr;
                try {
                    std::string m;
                    m.append("<E#").append(std::to_string(jau::abs(argIdx))).append("@").append(std::to_string(line)).append(":").append(tag).append(">");
                    if( fits(m.size()) ) {
                        m_s.append(m);
                    }
                } catch (...) {
                    eptr = std::current_exception();
                }
                handle_exception(eptr);
            }
        };

        template<OutputType Output>
        class Parser;  // fwd

        class no_type_t {};

        template<OutputType Output>
        class FResult {
          public:
            std::string_view fmt;
            size_t pos;  ///< position of next fmt character to be read
            ssize_t arg_count;
            int line;
            pstate_t state;
            FormatOpts opts;

          private:
            Output m_out;
            size_t pos_lstart;  ///< start of last conversion spec

          public:
            constexpr FResult(Output p, std::string_view fmt_) noexcept
            : fmt(fmt_), pos(0), arg_count(0), line(0), state(pstate_t::outside), opts(),
              m_out(std::move(p)), pos_lstart(0) { }

            constexpr FResult(const FResult &pre) noexcept = default;
            constexpr FResult &operator=(const FResult &x) noexcept = default;

            constexpr operator Result() const noexcept {
                return Result(fmt, opts, pos, arg_count, line, pstate_t::outside == state);
            }

            constexpr bool hasNext() const noexcept {
                return !error() && pos < fmt.length();
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
                .append("`, last[")
                .append(opts.toString())
                .append("], fmt `")
                .append(fmt)
                .append("`, out `")
                .append(m_out.get())
                .append("`");
                return s;
            }

          private:
            friend class impl::Parser<Output>;

            constexpr bool nextSymbol(char &c) noexcept {
                if (pos < fmt.length()) {
                    c = fmt[pos++];
                    return true;
                } else {
                    return false;
                }
            }
            constexpr bool toConversion() noexcept {
                if (pstate_t::outside != state) {
                    return true;  // inside conversion specifier
                } else if (fmt[pos] == '%') {
                    state = pstate_t::start;  // just at start of conversion specifier
                    pos_lstart = pos++;
                    opts.reset();
                    return true;
                } else if (pos < fmt.length()) {
                    // seek next conversion specifier
                    const size_t q = fmt.find('%', pos + 1);
                    if (q == std::string::npos) {
                        // no conversion specifier found
                        appendText(fmt.substr(pos, fmt.length() - pos));
                        pos = fmt.length();
                        return false;
                    } else {
                        // new conversion specifier found
                        appendText(fmt.substr(pos, q - pos));
                        state = pstate_t::start;
                        pos_lstart = pos;
                        pos = q + 1;
                        opts.reset();
                        return true;
                    }
                } else {
                    // end of format
                    return false;
                }
            }

            constexpr void setLastSpec(size_t endpos) noexcept {
                if (endpos > pos_lstart) {
                    opts.fmt = fmt.substr(pos_lstart, endpos - pos_lstart);
                }
            }

            constexpr void setError(int l) noexcept {
                line = l;
                state = pstate_t::error;
                if (0 == arg_count) {
                    arg_count = std::numeric_limits<ssize_t>::min();
                } else if (0 < arg_count) {
                    arg_count *= -1;
                }
            }

            template<typename T>
                requires jau::req::stringifyable_jau<T>
            constexpr FResult &appendFormatted(const T &v) noexcept {
                m_out.appendFormatted(opts, v);
                return *this;
            }

            constexpr FResult &appendText(const std::string_view v) noexcept {
                m_out.appendText(v);
                return *this;
            }
            constexpr FResult &appendError(const std::string_view tag) noexcept {
                ssize_t c = arg_count == std::numeric_limits<ssize_t>::min() ? 0 : arg_count;
                m_out.appendError(c, line, tag);
                return *this;
            }
        };

        // A NullOutput formatting result, capable of `constexpr` and `consteval`
        typedef FResult<NullOutput> CheckResult;

        /// A StringOutput formatting result for runtime formatting into a std::string
        typedef FResult<StringOutput> SFormatResult;
        template<OutputType Output>
        class Parser {
          public:
            typedef FResult<Output> Result;
            constexpr Parser() noexcept = default;

            /**
             * Parse the given argument against the current conversion specifier of the format string.
             *
             * Multiple rounds of parsing calls might be required, each passing the next argument or null.
             *
             * Parsing is completed when method returns false, either signaling an error() or completion.
             *
             * Caller only checks error status in the end, since all arguments will be processed due to folding parameter pack
             *
             * @tparam T The type of the given argument
             * @return true if no error _and_ not complete, i.e. further calls with subsequent parameter required. Otherwise parsing ended due to error or completeness.
             */
            template <typename T>
            constexpr void parseOne(Result &pc, const T &val) const noexcept {
                if( !pc.hasNext() ) {
                    return;  // done or error
                }

                bool loop_next;
                char c;
                do {
                    if( !pc.toConversion() ) {
                        return;  // done
                    }
                    // pstate_t::outside != _state

                    /* skip '%' or previous `*` */
                    if( !pc.nextSymbol(c) ) {
                        pc.setError(__LINE__);
                        return;  // error
                    }

                    if( pstate_t::start == pc.state ) {
                        pc.state = pstate_t::field_width;
                        parseFlags(pc, c);

                        /* parse field width */
                        if( c == '*' ) {
                            // error or continue with next argument for same conversion -> field_width
                            parseArgWidthPrecision<T>(true, pc, val);
                            return;
                        } else {
                            if( !parseFmtWidthPrecision(true, pc, c) ) {
                                if( pc.error() ) {
                                    return;
                                }
                                // no width, continue with same argument for same conversion -> field_width
                            }
                        }
                    }

                    if( pstate_t::field_width == pc.state ) {
                        /* parse precision */
                        pc.state = pstate_t::precision;
                        if( c == '.' ) {
                            if( !pc.nextSymbol(c) ) {
                                pc.setError(__LINE__); // missing number + spec
                                return; // error
                            }
                            if( c == '*' ) {
                                // error or continue with next argument for same conversion -> field_width
                                parseArgWidthPrecision<T>(false, pc, val);
                                return;
                            } else {
                                if( !parseFmtWidthPrecision(false, pc, c) ) {
                                    if( pc.error() ) {
                                        return;
                                    }
                                    // no explicit precision -> zero precision, continue with same argument
                                    pc.opts.setPrecision(0);
                                }
                            }
                        }
                    }
                    if( !parseLengthMods(pc, c) ) {
                        pc.appendError("Len");
                        return;  // error
                    }
                    pc.setLastSpec(pc.pos);

                    if( c == '%' ) {
                        loop_next = true;
                        pc.appendText("%");
                    } else {
                        loop_next = false;
                        if( !parseFmtSpec<T>(pc, c, val) ) {
                            pc.appendError("Cnv");
                            return;  // error
                        }
                    }

                    // next conversion specifier
                    pc.state = pstate_t::outside;

                } while (loop_next);

                // return pc.hasNext(); // true: no-error and not-complete
            }

            template <typename T>
            requires (!jau::req::string_alike<T>) // not: jau::req::string_literal<T> || jau::req::string_class<T> || jau::req::char_pointer<T>
            constexpr void checkOne(Result &pc) const noexcept {
                parseOne<T>(pc, T());
            }

            template <typename T>
            requires jau::req::string_literal<T> || jau::req::string_class<T>
            constexpr void checkOne(Result &pc) const noexcept {
                parseOne<std::string_view>(pc, std::string_view());
            }

            template <typename T>
            requires jau::req::char_pointer<T> // also allows passing `char*` for `%p`
            constexpr void checkOne(Result &pc) const noexcept {
                parseOne<T>(pc, T());
            }

          private:
            constexpr void parseFlags(Result &pc, char &c) const noexcept {
                while( pc.opts.addFlag(c) && pc.nextSymbol(c) ) { }
            }

            /// Parse argument field width or precision, returns false on error. Otherwise next argument is required.
            template <typename T>
            requires (!std::integral<T>)
            constexpr void parseArgWidthPrecision(bool , Result &pc, const T &) const noexcept {
                pc.setError(__LINE__);
            }
            /// Parse argument field width or precision, returns false on error. Otherwise next argument is required.
            template <typename T>
            requires (std::integral<T>)
            constexpr void parseArgWidthPrecision(bool is_width, Result &pc, const T &val) const noexcept {
                using U = std::remove_cv_t<T>;

                if constexpr( std::is_same_v<no_type_t, T> ) {
                    pc.setError(__LINE__);
                    return;  // error
                }
                ++pc.arg_count;
                if constexpr( !std::is_same_v<int, U> ) {
                    pc.setError(__LINE__);
                    return;  // error
                }
                if( !jau::is_positive(val) ) {
                    if( is_width ) {
                        pc.opts.flags |= flags_t::left; // reverse padding
                        pc.opts.setWidth((size_t)jau::abs(val));
                    } else {
                        pc.opts.setPrecision(0);
                    }
                } else {
                    if( is_width ) {
                        pc.opts.setWidth((size_t)val);
                    } else {
                        pc.opts.setPrecision((size_t)val);
                    }
                }
                // next argument is required
            }

            /// Parse format field width or precision, returns true if field is consumed and parsing can continue
            /// or false if field has not been consumed or definite error
            constexpr bool parseFmtWidthPrecision(bool is_width, Result &pc, char &c) const noexcept {
                char buffer[num_max_slen+1];
                char *s = &buffer[0];
                const char *s_begin = s;
                const char *s_end = s + num_max_slen;
                while( jau::is_digit(c) && s < s_end ) {
                    *s = c; ++s;
                    if( !pc.nextSymbol(c) ) {
                        pc.setError(__LINE__); // no digit nor spec
                        return false;
                    }
                }
                if( jau::is_digit(c) ) {
                    pc.setError(__LINE__); // s >= s_end
                    return false;
                }
                std::string_view sv(s_begin, s - s_begin);
                int64_t num = 0;
                if( sv.empty() ) {
                    return false; // no digits, may continue
                }
                if( !from_chars(num, sv) ) {
                    // pc.setError(__LINE__); // number syntax
                    return false;  // error
                }
                if( is_width ) {
                    pc.opts.setWidth((size_t)num);
                } else {
                    pc.opts.setPrecision((size_t)num);
                }
                return true;  // continue with current argument
            }

            /* parse length modifier, returns true if parsing can continue or false on error. */
            constexpr bool parseLengthMods(Result &pc, char &c) const noexcept {
                if( 'h' == c ) {
                    if( !pc.nextSymbol(c) ) { return false; }
                    if( 'h' == c ) {
                        if( !pc.nextSymbol(c) ) { return false; }
                        pc.opts.length_mod = plength_t::hh;
                    } else {
                        pc.opts.length_mod = plength_t::h;
                    }
                } else if( 'l' == c ) {
                    if( !pc.nextSymbol(c) ) { return false; }
                    if( 'l' == c ) {
                        if( !pc.nextSymbol(c) ) { return false; }
                        pc.opts.length_mod = plength_t::ll;
                    } else {
                        pc.opts.length_mod = plength_t::l;
                    }
                } else if( 'q' == c ) {
                    if( !pc.nextSymbol(c) ) { return false; }
                    pc.opts.length_mod = plength_t::ll;
                } else if( 'L' == c ) {
                    if( !pc.nextSymbol(c) ) { return false; }
                    pc.opts.length_mod = plength_t::L;
                } else if( 'j' == c ) {
                    if( !pc.nextSymbol(c) ) { return false; }
                    pc.opts.length_mod = plength_t::j;
                } else if( 'z' == c || 'Z' == c ) {
                    if( !pc.nextSymbol(c) ) { return false; }
                    pc.opts.length_mod = plength_t::z;
                } else if( 't' == c ) {
                    if( !pc.nextSymbol(c) ) { return false; }
                    pc.opts.length_mod = plength_t::t;
                } else {
                    pc.opts.length_mod = plength_t::none;
                }
                return true;
            }

            template <typename T>
            constexpr bool parseFmtSpec(Result &pc, char fmt_literal, const T &val) const noexcept {
                if( !pc.opts.setConversion(fmt_literal) ) {
                    pc.setError(__LINE__);
                    return false;
                }

                switch( pc.opts.conversion ) {
                    case cspec_t::character:
                        return parseCharFmtSpec<T>(pc, val);
                    case cspec_t::string:
                        return parseStringFmtSpec<T>(pc, val);
                    case cspec_t::pointer:
                        return parseAPointerFmtSpec<T>(pc, val);
                    case cspec_t::signed_int:
                        return parseSignedFmtSpec<T>(pc, val);
                    case cspec_t::unsigned_int:
                        return parseUnsignedFmtSpec<T>(pc, val);
                    case cspec_t::floating_point:
                    case cspec_t::exp_float:
                    case cspec_t::hex_float:
                    case cspec_t::alt_float:
                        return parseFloatFmtSpec<T>(pc, fmt_literal, val);
                    default:
                        pc.setError(__LINE__);
                        return false;
                }  // switch( fmt_literal )
            }

            template <typename T>
            requires (!std::integral<T>)
            constexpr bool parseCharFmtSpec(Result &pc, const T &) const noexcept
            {
                if constexpr( !std::is_same_v<no_type_t, T> ) {
                    ++pc.arg_count;
                }
                pc.setError(__LINE__);
                return false;
            }

            template <typename T>
            requires std::integral<T>
            constexpr bool parseCharFmtSpec(Result &pc, const T &val) const noexcept
            {
                ++pc.arg_count;

                using U = std::remove_cv_t<T>;
                switch( pc.opts.length_mod ) {
                    case plength_t::none: {
                        if constexpr( !std::is_same_v<char, U> &&
                                      !std::is_same_v<int, U> ) {
                            pc.setError(__LINE__);
                            return false;
                        }
                        std::string s(1, (char)val);
                        pc.appendFormatted(std::string_view(s));
                    } break;
                    case plength_t::l: {
                        if constexpr (!std::is_same_v<wchar_t, U> &&
                                      !std::is_same_v<wint_t, U>) {
                            pc.setError(__LINE__);
                            return false;
                        }
                        std::string s(1, (char)val);
                        pc.appendFormatted(std::string_view(s));  // FIXME: Support UTF16? UTF8 default
                    } break;
                    default:
                        pc.setError(__LINE__);
                        return false;
                }
                return true;
            }

            template <typename T>
            requires (!jau::req::string_alike<T>)
            constexpr bool parseStringFmtSpec(Result &pc, const T &) const noexcept
            {
                if constexpr( !std::is_same_v<no_type_t, T> ) {
                    ++pc.arg_count;
                }
                pc.setError(__LINE__);
                return false;
            }

            template <typename T>
            requires jau::req::string_alike<T>
            constexpr bool parseStringFmtSpec(Result &pc, const T &val) const noexcept
            {
                ++pc.arg_count;
                switch( pc.opts.length_mod ) {
                    case plength_t::none:
                        break;
                    case plength_t::l:
                        if constexpr( !(std::is_pointer_v<T> &&
                                        std::is_same_v<wchar_t, std::remove_cv_t<std::remove_pointer_t<T>>>) ) {
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;
                    default:
                        // setError();
                        pc.setError(__LINE__);
                        return false;
                }
                pc.appendFormatted(val);
                return true;
            }

            template <typename T>
            requires (!jau::req::pointer<T>)
            constexpr bool parseAPointerFmtSpec(Result &pc, const T &) const noexcept {
                if constexpr( !std::is_same_v<no_type_t, T> ) {
                    ++pc.arg_count;
                }
                pc.setError(__LINE__);
                return false;
            }
            template <typename T>
            requires jau::req::pointer<T>
            constexpr bool parseAPointerFmtSpec(Result &pc, const T &val) const noexcept {
                pc.opts.length_mod = plength_t::none;
                ++pc.arg_count;
                pc.appendFormatted((void *)const_cast<std::remove_const_t<T>>(val)); // force pointer type
                return true;
            }

            template <typename T>
            requires (!(jau::req::signed_integral<T> || std::is_enum_v<T> || jau::req::boolean<T>))
            constexpr bool parseSignedFmtSpec(Result &pc, const T &) const {
                if constexpr( !std::is_same_v<no_type_t, T> ) {
                    ++pc.arg_count;
                }
                pc.setError(__LINE__);
                return false;
            }
            template <typename T>
            requires std::is_enum_v<T> && (!jau::req::signed_integral<std::underlying_type_t<T>>)
            constexpr bool parseSignedFmtSpec(Result &pc, const T &) const {
                if constexpr( !std::is_same_v<no_type_t, T> ) {
                    ++pc.arg_count;
                }
                pc.setError(__LINE__);
                return false;
            }
            template <typename T>
            requires std::is_enum_v<T> && jau::req::signed_integral<std::underlying_type_t<T>>
            constexpr bool parseSignedFmtSpec(Result &pc, const T &val0) const {
                return parseSignedFmtSpec(pc, std::underlying_type_t<T>(val0));
            }
            template <typename T>
            requires jau::req::signed_integral<T> || jau::req::boolean<T>
            constexpr bool parseSignedFmtSpec(Result &pc, const T &val0) const {
                ++pc.arg_count;

                using U = std::remove_cv_t<T>;
                // using V = std::conditional<std::is_integral_v<U> && std::is_unsigned_v<U>, std::make_signed<U>, U>::type; // triggers the instantiating the 'other' case and hence fails
                using V = typename std::conditional_t<!std::is_same_v<bool, U> && std::is_integral_v<U> && std::is_unsigned_v<U>, std::make_signed<U>, std::type_identity<U>>::type;  // NOLINT

                // Not accepting unsigned -> signed, hence `jau::req::signed_integral<T>`
                if( val0 > std::numeric_limits<V>::max() ) {
                    pc.setError(__LINE__); // signedness correct by T, never reached
                    return false;
                }
                const V val = V(val0);
                if (jau::is_zero(val)) {
                    pc.opts.flags &= ~flags_t::hash;  // no hash for 0 values
                }

                // we accept given type <= integral target type

                switch( pc.opts.length_mod ) {
                    case plength_t::hh:
                        if constexpr( !std::is_same_v<char, V> && !(std::is_integral_v<V> && sizeof(V) <= sizeof(char)) ) { // NOLINT(bugprone-sizeof-expression)
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;
                    case plength_t::h:
                        if constexpr( !std::is_same_v<short, V> && !(std::is_integral_v<V> && sizeof(V) <= sizeof(short)) ) { // NOLINT(bugprone-sizeof-expression)
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;
                    case plength_t::none:
                        if constexpr( !std::is_same_v<int, V> && !(std::is_integral_v<V> && sizeof(V) <= sizeof(int)) ) { // NOLINT(bugprone-sizeof-expression)
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;
                    case plength_t::l:
                        if constexpr( !std::is_same_v<long, V> && !(std::is_integral_v<V> && sizeof(V) <= sizeof(long)) ) { // NOLINT(bugprone-sizeof-expression)
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;
                    case plength_t::ll:
                        if constexpr( !std::is_same_v<long long, V> && !(std::is_integral_v<V> && sizeof(V) <= sizeof(long long)) ) { // NOLINT(bugprone-sizeof-expression)
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;
                    case plength_t::j:
                        if constexpr( !std::is_same_v<intmax_t, V> && !(std::is_integral_v<V> && sizeof(V) <= sizeof(intmax_t)) ) { // NOLINT(bugprone-sizeof-expression)
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;
                    case plength_t::z:
                        if constexpr( !std::is_same_v<ssize_t, V> && !(std::is_integral_v<V> && sizeof(V) <= sizeof(ssize_t)) ) { // NOLINT(bugprone-sizeof-expression)
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;
                    case plength_t::t:
                        if constexpr( !std::is_same_v<ptrdiff_t, V> && !(std::is_integral_v<V> && sizeof(V) <= sizeof(ptrdiff_t)) ) { // NOLINT(bugprone-sizeof-expression)
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;
                    default:
                        pc.setError(__LINE__);
                        return false;
                }
                pc.appendFormatted(val);
                return true;
            }

            template <typename T>
            requires (!(std::integral<T> || std::is_enum_v<T>))
            constexpr bool parseUnsignedFmtSpec(Result &pc, const T &) const noexcept {
                if constexpr( !std::is_same_v<no_type_t, T> ) {
                    ++pc.arg_count;
                }
                pc.setError(__LINE__);
                return false;
            }
            template <typename T>
            requires std::is_enum_v<T>
            constexpr bool parseUnsignedFmtSpec(Result &pc, const T &val0) const {
                return parseUnsignedFmtSpec(pc, std::underlying_type_t<T>(val0));
            }
            template <typename T>
            requires std::integral<T>
            constexpr bool parseUnsignedFmtSpec(Result &pc, const T &val0) const noexcept {
                ++pc.arg_count;

                using U = std::remove_cv_t<T>;
                // using V = std::conditional_t<std::is_integral_v<U> && std::is_signed_v<U>, std::make_unsigned_t<U>, U>; // triggers the instantiating the 'other' case and hence fails
                using V = typename std::conditional_t<std::is_integral_v<U> && std::is_signed_v<U>, std::make_unsigned<U>, std::type_identity<U>>::type;  // NOLINT

                if( !jau::is_positive(val0) ) {
                    pc.setError(__LINE__);
                    return false;
                }
                const V val = V(val0);
                if (jau::is_zero(val)) {
                    pc.opts.flags &= ~flags_t::hash;  // no hash for 0 values
                }

                // we accept given type <= integral target type

                switch( pc.opts.length_mod ) {
                    case plength_t::hh:
                        if constexpr( !std::is_same_v<unsigned char, V> && !(std::is_integral_v<V> && sizeof(V) <= sizeof(unsigned char)) ) { // NOLINT(bugprone-sizeof-expression)
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;
                    case plength_t::h:
                        if constexpr( !std::is_same_v<unsigned short, V> && !(std::is_integral_v<V> && sizeof(V) <= sizeof(unsigned short)) ) { // NOLINT(bugprone-sizeof-expression)
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;
                    case plength_t::none:
                        if constexpr( !std::is_same_v<unsigned int, V> && !(std::is_integral_v<V> && sizeof(V) <= sizeof(unsigned int)) ) { // NOLINT(bugprone-sizeof-expression)
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;
                    case plength_t::l:
                        if constexpr( !std::is_same_v<unsigned long, V> && !(std::is_integral_v<V> && sizeof(V) <= sizeof(unsigned long)) ) { // NOLINT(bugprone-sizeof-expression)
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;
                    case plength_t::ll:
                        if constexpr( !std::is_same_v<unsigned long long, V> && !(std::is_integral_v<V> && sizeof(V) <= sizeof(unsigned long long)) ) { // NOLINT(bugprone-sizeof-expression)
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;
                    case plength_t::j:
                        if constexpr( !std::is_same_v<uintmax_t, V> && !(std::is_integral_v<V> && sizeof(V) <= sizeof(uintmax_t)) ) { // NOLINT(bugprone-sizeof-expression)
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;
                    case plength_t::z:
                        if constexpr( !std::is_same_v<size_t, V> && !(std::is_integral_v<V> && sizeof(V) <= sizeof(size_t)) ) { // NOLINT(bugprone-sizeof-expression)
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;

                    case plength_t::t:
                        if constexpr( !std::is_same_v<ptrdiff_t, V> && !(std::is_integral_v<V> && sizeof(V) <= sizeof(ptrdiff_t)) ) { // NOLINT(bugprone-sizeof-expression)
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;

                    default:
                        pc.setError(__LINE__);
                        return false;
                }
                pc.appendFormatted(val);
                return true;
            }

            template <typename T>
            requires (!std::floating_point<T>)
            constexpr bool parseFloatFmtSpec(Result &pc, const char /*fmt_literal*/, const T &) const noexcept {
                if constexpr( !std::is_same_v<no_type_t, T> ) {
                    ++pc.arg_count;
                }
                pc.setError(__LINE__);
                return false;
            }
            template <typename T>
            requires std::floating_point<T>
            constexpr bool parseFloatFmtSpec(Result &pc, const char /*fmt_literal*/, const T &val) const noexcept {
                ++pc.arg_count;

                using U = std::remove_cv_t<T>;

                switch( pc.opts.length_mod ) {
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
                pc.appendFormatted(val);
                return true;
            }
        };

        typedef Parser<NullOutput> CheckParser;
        typedef Parser<StringOutput> FormatParser;

    }  // namespace impl

    /**
     * Strict format with type validation of arguments against the format string.
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
    inline std::string format(std::string_view fmt, const Targs &...args) noexcept {
        std::string s;
        impl::SFormatResult ctx(impl::StringOutput(s.max_size(), s), fmt);
        constexpr const impl::FormatParser p;
        if constexpr( 0 < sizeof...(Targs) ) {
            ((p.template parseOne<Targs>(ctx, args)), ...);
        }
        p.template parseOne<impl::no_type_t>(ctx, impl::no_type_t());
        return s;
    }

    /**
     * Strict format with type validation of arguments against the format string.
     *
     * See @ref jau_cfmt_header for details
     *
     * @tparam Targs the argument template type pack to be validated against the format string
     * @param maxLen maximum string length
     * @param fmt the snprintf format string
     * @param args passed arguments, used for template type deduction only
     * @return true if successfully parsed format and arguments, false otherwise.
     * @see @ref jau_cfmt_header
     */
    template <typename... Targs>
    inline std::string format(size_t maxLen, std::string_view fmt, const Targs &...args) noexcept {
        std::string s;
        impl::SFormatResult ctx(impl::StringOutput(std::min(maxLen, s.max_size()), s), fmt);
        constexpr const impl::FormatParser p;
        if constexpr( 0 < sizeof...(Targs) ) {
            ((p.template parseOne<Targs>(ctx, args)), ...);
        }
        p.template parseOne<impl::no_type_t>(ctx, impl::no_type_t());
        return s;
    }

    /**
     * Strict format with type validation of arguments against the format string,
     * appending to the given destination.
     *
     * See @ref jau_cfmt_header for details
     *
     * @tparam Targs the argument template type pack to be validated against the format string
     * @param s destination string to append the formatted string
     * @param fmt the snprintf format string
     * @param args passed arguments, used for template type deduction only
     * @return jau::cfmt::Result instance for further inspection
     * @see @ref jau_cfmt_header
     */
    template <typename... Targs>
    inline Result formatR(std::string &s, std::string_view fmt, const Targs &...args) noexcept {
        impl::SFormatResult ctx(impl::StringOutput(s.max_size(), s), fmt);
        constexpr const impl::FormatParser p;
        if constexpr( 0 < sizeof...(Targs) ) {
            ((p.template parseOne<Targs>(ctx, args)), ...);
        }
        p.template parseOne<impl::no_type_t>(ctx, impl::no_type_t());
        return ctx;
    }
    /**
     * Strict format with type validation of arguments against the format string,
     * appending to the given destination.
     *
     * See @ref jau_cfmt_header for details
     *
     * @tparam Targs the argument template type pack to be validated against the format string
     * @param s destination string to append the formatted string
     * @param maxLen maximum string length
     * @param fmt the snprintf format string
     * @param args passed arguments, used for template type deduction only
     * @return jau::cfmt::Result instance for further inspection
     * @see @ref jau_cfmt_header
     */
    template <typename... Targs>
    inline Result formatR(std::string &s, size_t maxLen, std::string_view fmt, const Targs &...args) noexcept {
        impl::SFormatResult ctx(impl::StringOutput(std::min(maxLen, s.max_size()), s), fmt);
        constexpr const impl::FormatParser p;
        if constexpr( 0 < sizeof...(Targs) ) {
            ((p.template parseOne<Targs>(ctx, args)), ...);
        }
        p.template parseOne<impl::no_type_t>(ctx, impl::no_type_t());
        return ctx;
    }

    /**
     * Strict type validation of arguments against the format string.
     *
     * See @ref jau_cfmt_header for details
     *
     * @tparam Targs the argument template type pack to be validated against the format string
     * @param fmt the snprintf format string
     * @param args passed arguments, used for template type deduction only
     * @return number of parser format arguments if successfully, otherwise negative number indicates first failed argument starting with -1
     *         w/ min(ssize_t) denoting the format string
     * @see @ref jau_cfmt_header
     */
    template <typename... Targs>
    consteval_cxx20 ssize_t check(std::string_view fmt, const Targs &...) noexcept {
        impl::CheckResult ctx(impl::NullOutput(), fmt);
        constexpr const impl::CheckParser p;
        if constexpr( 0 < sizeof...(Targs) ) {
            ((p.template checkOne<Targs>(ctx)), ...);
        }
        p.template checkOne<impl::no_type_t>(ctx);
        return ctx.arg_count;
    }
    /**
     * Strict type validation of arguments against the format string.
     *
     * See @ref jau_cfmt_header for details
     *
     * @tparam Targs the argument template type pack to be validated against the format string
     * @param fmt the snprintf format string
     * @param args passed arguments, used for template type deduction only
     * @return 0 if successfully, otherwise the source code line number detecting the failure
     * @see @ref jau_cfmt_header
     */
    template <typename... Targs>
    consteval_cxx20 int checkLine(std::string_view fmt, const Targs &...) noexcept {
        impl::CheckResult ctx(impl::NullOutput(), fmt);
        constexpr const impl::CheckParser p;
        if constexpr( 0 < sizeof...(Targs) ) {
            ((p.template checkOne<Targs>(ctx)), ...);
        }
        p.template checkOne<impl::no_type_t>(ctx);
        return ctx.line;
    }

    /**
     * Strict type validation of arguments against the format string.
     *
     * See @ref jau_cfmt_header for details
     *
     * @tparam Targs the argument template type pack to be validated against the format string
     * @param fmt the snprintf format string
     * @return number of parser format arguments if successfully, otherwise negative number indicates first failed argument starting with -1
     *         w/ min(ssize_t) denoting the format string
     * @see @ref jau_cfmt_header
     */
    template <typename... Targs>
    constexpr ssize_t check2(std::string_view fmt) noexcept {
        impl::CheckResult ctx(impl::NullOutput(), fmt);
        constexpr const impl::CheckParser p;
        if constexpr( 0 < sizeof...(Targs) ) {
            ((p.template checkOne<Targs>(ctx)), ...);
        }
        p.template checkOne<impl::no_type_t>(ctx);
        return ctx.arg_count;
    }

    /**
     * Strict type validation of arguments against the format string.
     *
     * See @ref jau_cfmt_header for details
     *
     * @tparam Targs the argument template type pack to be validated against the format string
     * @param fmt the snprintf format string
     * @return 0 if successfully, otherwise the source code line number detecting the failure
     * @see @ref jau_cfmt_header
     */
    template <typename... Targs>
    constexpr int check2Line(std::string_view fmt) noexcept {
        impl::CheckResult ctx(impl::NullOutput(), fmt);
        constexpr const impl::CheckParser p;
        if constexpr( 0 < sizeof...(Targs) ) {
            ((p.template checkOne<Targs>(ctx)), ...);
        }
        p.template checkOne<impl::no_type_t>(ctx);
        return ctx.line;
    }

    /**
     * Strict type validation of arguments against the format string.
     *
     * See @ref jau_cfmt_header for details
     *
     * @tparam Targs the argument template type pack to be validated against the format string
     * @param fmt the snprintf format string
     * @param args passed arguments, used for template type deduction only
     * @return jau::cfmt::Result instance for further inspection
     * @see @ref jau_cfmt_header
     */
    template <typename... Targs>
    constexpr Result checkR(std::string_view fmt, const Targs &...) noexcept {
        impl::CheckResult ctx(impl::NullOutput(), fmt);
        constexpr const impl::CheckParser p;
        if constexpr( 0 < sizeof...(Targs) ) {
            ((p.template checkOne<Targs>(ctx)), ...);
        }
        p.template checkOne<impl::no_type_t>(ctx);
        return ctx;
    }

    /**
     * Strict type validation of arguments against the format string.
     *
     * See @ref jau_cfmt_header for details
     *
     * @tparam Targs the argument template type pack to be validated against the format string
     * @param fmt the snprintf format string
     * @return jau::cfmt::Result instance for further inspection
     * @see @ref jau_cfmt_header
     */
    template <typename... Targs>
    consteval_cxx20 Result checkR2(std::string_view format) noexcept {
        impl::CheckResult ctx(impl::NullOutput(), format);
        constexpr const impl::CheckParser p;
        if constexpr( 0 < sizeof...(Targs) ) {
            ((p.template checkOne<Targs>(ctx)), ...);
        }
        p.template checkOne<impl::no_type_t>(ctx);
        return ctx;
    }

    /**@}*/

}  // namespace jau::cfmt

namespace jau {
    /**
     * Safely returns a (potentially truncated) string according to `snprintf()` formatting rules
     * and variable number of arguments following the `format` argument.
     *
     * jau::cfmt::format() is utilize to validate `format` against given arguments at *runtime*.
     *
     * Resulting string is truncated to `min(maxStrLen, formatLen)`,
     * with `formatLen` being the given formatted string length of output w/o limitation.
     *
     * See @ref jau_cfmt_header for details
     *
     * @param maxStrLen maximum resulting string length including
     * @param format `printf()` compliant format string
     * @param args optional arguments matching the format string
     */
    template<typename... Args>
    inline std::string format_string_n(const std::size_t maxStrLen, std::string_view format, const Args &...args) noexcept {
        return jau::cfmt::format(maxStrLen, format, args...);
    }

    /**
     * Safely returns a (non-truncated) string according to `snprintf()` formatting rules
     * and variable number of arguments following the `format` argument.
     *
     * jau::cfmt::formatR() is utilize to validate `format` against given arguments at *runtime*.
     *
     * Resulting string size matches formated output w/o limitation.
     *
     * See @ref jau_cfmt_header for details
     *
     * @param strLenHint initially used string length w/o EOS
     * @param format `printf()` compliant format string
     * @param args optional arguments matching the format string
     */
    template <typename... Args>
    inline std::string format_string_h(const std::size_t strLenHint, std::string_view format, const Args &...args) noexcept {
        std::string str;
        std::exception_ptr eptr;
        try {
            str.reserve(strLenHint);
            jau::cfmt::formatR(str, format, args...);
            str.shrink_to_fit();
        } catch (...) {
            eptr = std::current_exception();
        }
        handle_exception(eptr);
        return str;
    }

    /**
     * Safely returns a (non-truncated) string according to `snprintf()` formatting rules
     * using a reserved string length of jau::cfmt::default_string_capacity and
     * variable number of arguments following the `format` argument.
     *
     * jau::cfmt::formatR() is utilize to validate `format` against given arguments at *runtime*.
     *
     * Resulting string size matches formated output w/o limitation.
     *
     * See @ref jau_cfmt_header for details
     *
     * @param format `printf()` compliant format string
     * @param args optional arguments matching the format string
     */
    template <typename... Args>
    inline std::string format_string(std::string_view format, const Args &...args) noexcept {
        return jau::format_string_h(jau::cfmt::default_string_capacity, format, args...);
    }

    /**@}*/

} // namespace jau

/** \addtogroup StringUtils
 *
 *  @{
 */

/**
 * Macro, safely returns a (non-truncated) string according to `snprintf()` formatting rules
 * using a reserved string length of jau::cfmt::default_string_capacity and
 * variable number of arguments following the `format` argument.
 *
 * This macro also produces compile time validation using a `static_assert`
 * against jau::cfmt::check2.
 *
 * jau::cfmt::formatR() is utilize to validate `format` against given arguments at *runtime*.
 *
 * Resulting string size matches formated output w/o limitation.
 *
 * See @ref jau_cfmt_header for details
 *
 * @param format `printf()` compliant format string
 * @param args optional arguments matching the format string
 */
#define jau_format_string(fmt, ...) \
    jau::format_string((fmt) __VA_OPT__(,) __VA_ARGS__);  \
    static_assert(0 <= jau::cfmt::check2< JAU_FOR_EACH1_LIST(JAU_DECLTYPE_VALUE, __VA_ARGS__) >(fmt)); // compile time validation!

/**
 * Macro, safely returns a (non-truncated) string according to `snprintf()` formatting rules
 * and variable number of arguments following the `format` argument.
 *
 * This macro also produces compile time validation using a `static_assert`
 * against jau::cfmt::check2.
 *
 * jau::cfmt::formatR() is utilize to validate `format` against given arguments at *runtime*.
 *
 * Resulting string size matches formated output w/o limitation.
 *
 * See @ref jau_cfmt_header for details
 *
 * @param strLenHint initially used string length w/o EOS
 * @param format `printf()` compliant format string
 * @param args optional arguments matching the format string
 */
#define jau_format_string_h(strLenHint, fmt, ...) \
    jau::format_string((strLenHint), (fmt) __VA_OPT__(,) __VA_ARGS__);  \
    static_assert(0 <= jau::cfmt::check2< JAU_FOR_EACH1_LIST(JAU_DECLTYPE_VALUE, __VA_ARGS__) >(fmt)); // compile time validation!

/**
 * Macro, safely returns a (non-truncated) string according to `snprintf()` formatting rules
 * using a reserved string length of jau::cfmt::default_string_capacity and
 * variable number of arguments following the `format` argument.
 *
 * This macro also produces compile time validation using a `static_assert`
 * against jau::cfmt::check2Line.
 *
 * jau::cfmt::formatR() is utilize to validate `format` against given arguments at *runtime*.
 *
 * Resulting string size matches formated output w/o limitation.
 *
 * See @ref jau_cfmt_header for details
 *
 * @param format `printf()` compliant format string
 * @param args optional arguments matching the format string
 */
#define jau_format_string2(fmt, ...) \
    jau::format_string((fmt) __VA_OPT__(,) __VA_ARGS__);  \
    static_assert(0 == jau::cfmt::check2Line< JAU_FOR_EACH1_LIST(JAU_DECLTYPE_VALUE, __VA_ARGS__) >(fmt)); // compile time validation!

/**
 * Macro produces compile time validation using a `static_assert`
 * against jau::cfmt::check2.
 *
 * See @ref jau_cfmt_header for details
 *
 * @param format `printf()` compliant format string
 * @param args optional arguments matching the format string
 */
#define jau_string_check(fmt, ...) \
    static_assert(0 <= jau::cfmt::check2< JAU_FOR_EACH1_LIST(JAU_DECLTYPE_VALUE, __VA_ARGS__) >(fmt)); // compile time validation!

/**
 * Macro produces compile time validation using a `static_assert`
 * against jau::cfmt::check2Line.
 *
 * See @ref jau_cfmt_header for details
 *
 * @param format `printf()` compliant format string
 * @param args optional arguments matching the format string
 */
#define jau_string_checkLine(fmt, ...) \
    static_assert(0 == jau::cfmt::check2Line< JAU_FOR_EACH1_LIST(JAU_DECLTYPE_VALUE, __VA_ARGS__) >(fmt)); // compile time validation!

/**@}*/

#endif  // JAU_STRING_CFMT_HPP_
