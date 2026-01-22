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
 * ## jau::cfmt, a snprintf compliant runtime string format and compile-time validator
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
 * - Compatible with [fprintf](https://en.cppreference.com/w/cpp/io/c/fprintf), [snprintf](https://www.man7.org/linux/man-pages/man3/snprintf.3p.html), etc
 *
 * ### Type Conversion
 * Implementation follows type conversion rules as described
 * in [Variadic Default Conversion](https://en.cppreference.com/w/cpp/language/variadic_arguments#Default_conversions)
 * - float to double promotion
 * - bool, char, short, and unscoped enumerations are converted to int or wider integer types,
 *   see also [va_arg](https://en.cppreference.com/w/cpp/utility/variadic/va_arg)
 * - void pointer tolerance
 * - Exception signedness conversion
 *   - Allows positive signed to unsigned type conversion
 *     - Positive check at runtime only
 *   - Allows unsigned to signed type conversion if sizeof(unsigned type) < sizeof(signed type)
 *     - Compile time check
 *   - Otherwise fails intentionally
 *
 * ### Implementation Details
 * #### General
 * - Validates argument types against format string at compile time (consteval)
 * - Formats resulting string using argument values against format string at runtime
 * - Written in C++20 using template argument pack w/ save argument type checks
 * - Type erasure to wider common denominator, i.e. `uint64`, `const char* const`, `std::string_view`,
 *   reducing code footprint
 *
 * #### Behavior
 * - `nullptr` conversion value similar to `glibc`
 *   - string produces `(null)`
 *   - pointer produces `(nil)`
 * - Safe Signedness Conversion
 *   - Not accepting `unsigned` -> `signed` conversion if sizeof(unsigned type) >= sizeof(signed type)
 *     to avoid overflow (compile time check), otherwise OK
 *   - Not accepting negative integral value for `unsigned` (runtime check)
 * - Accept given type <= integral target type, conversion to wider types
 * - Accept `enum` types for integer conversion.
 *   - Only if underlying type is `unsigned`, it can't be used for signed integer conversion (see above)
 * - Accept direct std::string and std::string_view for `%s` string arguments
 * - Arithmetic integral + floating point types are limited to a maximum of 64-bit
 * - Runtime Errors
 *   - Failed runtime checks will inject an error market,
 *     e.g. `<E#1@1234:Cnv>` where the 1st argument caused a conversion error (`Cnv`)
 *     as detected in `string_cfmt.hpp` like 1234.
 *   - Argument 0 indicated an error in the format string.
 *   - `Len` tags a length modifier error
 *   - `Cnv` tags a conversion error
 *
 * ### Supported Format String
 *
 * `%[flags][width][.precision][length modifier]conversion`
 *
 * ### Flags
 * The following flags are supported
 * - `#`: hash, C99. Adds leading prefix for `radix != 10`.
 * - `0`: zeropad, C99
 * - `-`: left, C99
 * - <code>&nbsp;</code>: space, C99
 * - `+`: plus, C99
 * - ``'``: thousands, POSIX
 * - `,`: thousands, OpenJDK (alias for Java users)
 *
 * #### Width and Precision
 * Width and precision also supports `*` to use the next argument for its value.
 *
 * However, `*m$` (decimal integer `m`) for the `m`-th argument is not yet supported.
 *
 * #### Length Modifiers
 * The following length modifiers are supported
 * - `hh` [unsigned] char, ignored for floating point
 * - `h` [unsigned] short, ignored for floating point
 * - `l` [unsigned] long, ignored for floating point
 * - `ll` [unsigned] long long
 * - `q` deprecated synonym for `ll`
 * - `L` long double
 * - `j` uintmax_t or intmax_t
 * - `z` size_t or ssize_t
 * - `Z` deprecated synonym for `z`
 * - `t` ptrdiff_t
 *
 * #### Conversion Specifiers
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
 * #### Extended Conversion Specifier
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

    enum class pstate_t : uint16_t {
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
        thousands   = (uint16_t)1 << 6, ///< actual flag `\'`, POSIX

        uppercase   = (uint16_t)1 << 8  ///< uppercase, via conversion spec
    };
    JAU_MAKE_BITFIELD_ENUM_STRING(flags_t, hash, zeropad, left, space, plus, thousands, uppercase);

    /// Format length modifiers
    enum class plength_t : uint16_t {
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
        uint32_t width;
        uint32_t precision;
        uint32_t radix;
        flags_t flags;
        plength_t length_mod;
        cspec_t conversion;
        bool width_set:1;
        bool precision_set:1;

        constexpr FormatOpts() noexcept
        : fmt(), width(0), precision(0), radix(10),
          flags(flags_t::none),
          length_mod(plength_t::none),
          conversion(cspec_t::none),
          width_set(false), precision_set(false)
          { }

        constexpr void setWidth(uint32_t v) { width = v; width_set = true; }
        constexpr void setPrecision(uint32_t v) { precision = v; precision_set = true; }
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

        constexpr void reset() noexcept {
            fmt = std::string_view();
            width = 0;
            precision = 0;
            radix = 10;
            flags = flags_t::none;
            length_mod = plength_t::none;
            conversion = cspec_t::none;
            width_set = false;
            precision_set = false;
        }

        /// Reconstructs format string
        std::string toFormat() const;

        std::string toString() const;
    };

    inline std::ostream &operator<<(std::ostream &out, const FormatOpts &o) {
        out << o.toString();
        return out;
    }

    class Result {
      private:
        std::string_view m_fmt;
        size_t m_pos;        ///< position of next fmt character to be read
        ssize_t m_arg_count;
        int m_line;
        FormatOpts m_opts;
        bool m_success:1; ///< true if operation was successful, otherwise indicates error

      public:
        constexpr Result(std::string_view f, FormatOpts o, size_t pos, ssize_t acount, int line, bool ok)
        : m_fmt(f), m_pos(pos), m_arg_count(acount), m_line(line), m_opts(o), m_success(ok) {}

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

        std::string toString() const;
    };

    inline std::ostream &operator<<(std::ostream &out, const Result &pc) {
        out << pc.toString();
        return out;
    }

    namespace impl {
        inline constexpr const size_t char32buf_maxlen = 32;
        inline constexpr const size_t default_float_precision = 6;
        inline constexpr const double_t max_append_float = (double_t)1e9;

        void append_rev(std::string &dest, const size_t dest_maxlen, std::string_view src, bool prec_cut, bool reverse, const FormatOpts &opts);
        inline void append_string(std::string &dest, const size_t dest_maxlen, std::string_view src, const FormatOpts &opts) {
            append_rev(dest, dest_maxlen, src, true /*prec*/, false /*rev**/, opts);
        }
        void append_integral(std::string &dest, const size_t dest_maxlen, uint64_t v, const bool negative, const FormatOpts &opts, const bool inject_dot=false);
        // no width, nor precision, nor inject_dot
        void append_integral_simple(std::string &dest, const size_t dest_maxlen, uint64_t v, const bool negative, const FormatOpts &opts);

        // check for NaN and special values
        bool is_float_validF64(std::string &dest, const size_t dest_maxlen, const double value, const FormatOpts &opts);
        void append_floatF64(std::string &dest, const size_t dest_maxlen, const double value, const FormatOpts &opts);
        void append_efloatF64(std::string &dest, const size_t dest_maxlen, const double ivalue, const FormatOpts &iopts);
        void append_afloatF64(std::string &dest, const size_t dest_maxlen, const double ivalue, const size_t ivalue_size, const FormatOpts &iopts);

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

            template<typename T>
            requires jau::req::unsigned_integral<T> && (!jau::req::boolean<T>)
            constexpr void appendFormattedInt(const FormatOpts&, const T&, bool) noexcept { }

            template<typename T>
            requires std::is_floating_point_v<T>
            constexpr void appendFormattedFloat(const FormatOpts&, const T&, nsize_t) noexcept {}

            constexpr void appendText(std::string_view ) noexcept { }
            constexpr void appendError(size_t, int , const std::string_view ) noexcept {}
        };

        /// A std::string OutputType for runtime formatting into a std::string
        class StringOutput {
          private:
            std::size_t m_maxLen;
            std::string &m_s;

          public:
            constexpr StringOutput(std::size_t maxLen, std::string &s) noexcept
            : m_maxLen(maxLen), m_s(s) {}

            constexpr size_t maxLen() const noexcept { return m_maxLen; }
            constexpr bool fits(size_t n) const noexcept { return m_s.size() + n <= m_maxLen; }

            std::string_view get() const noexcept { return m_s; }

            template<typename T>
            requires jau::req::string_literal<T> || jau::req::string_class<T>
            void appendFormatted(const FormatOpts& opts, const T& v) {
                impl::append_string(m_s, m_maxLen, v, opts);
            }
            template<typename T>
            requires jau::req::char_pointer<T>
            void appendFormatted(const FormatOpts& opts, const T& v) {
                if( nullptr != v ) {
                    impl::append_string(m_s, m_maxLen, std::string_view(v), opts);
                } else {
                    impl::append_string(m_s, m_maxLen, "(null)", opts);
                }
            }
            template<typename T>
            requires jau::req::boolean<T>
            void appendFormatted(const FormatOpts& opts, const T& v) {
                impl::append_string(m_s, m_maxLen, jau::to_string(v), opts);
            }
            template<typename T>
            requires jau::req::pointer<T> && (!jau::req::string_alike<T>)
            void appendFormatted(const FormatOpts& opts, const T& v) {
                if( nullptr != v ) {
                    const uintptr_t v_le = jau::cpu_to_le(reinterpret_cast<uintptr_t>(v));
                    if (!opts.width_set && !opts.precision_set) {
                        impl::append_integral_simple(m_s, m_maxLen, v_le, false, opts);
                    } else {
                        impl::append_integral(m_s, m_maxLen, v_le, false, opts);
                    }
                } else {
                    impl::append_string(m_s, m_maxLen, "(nil)", opts);
                }
            }
            template<typename T>
            requires jau::req::unsigned_integral<T> && (!jau::req::boolean<T>)
            void appendFormattedInt(const FormatOpts& opts, const T& v, bool negative) {
                if (!opts.width_set && !opts.precision_set) {
                    impl::append_integral_simple(m_s, m_maxLen, uint64_t(v), negative, opts);
                } else {
                    impl::append_integral(m_s, m_maxLen, uint64_t(v), negative, opts);
                }
            }
            template<typename T>
            requires std::is_floating_point_v<T>
            void appendFormattedFloat(const FormatOpts& opts, const T& v, nsize_t floatSize) {
                if( opts.conversion == cspec_t::floating_point ) {
                    impl::append_floatF64(m_s, m_maxLen, v, opts);
                } else if( opts.conversion == cspec_t::hex_float ) {
                    impl::append_afloatF64(m_s, m_maxLen, v, floatSize, opts);
                } else {
                    // cspec_t::exp_float, cspec_t::alt_float
                    impl::append_efloatF64(m_s, m_maxLen, v, opts);
                }
            }
            inline void appendText(const std::string_view v) {
                if (fits(v.size())) {
                    m_s.append(v);
                }
            }

            void appendError(size_t argIdx, int line, const std::string_view tag);
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
            FormatOpts opts;
            pstate_t state;

          private:
            Output m_out;
            size_t pos_lstart;  ///< start of last conversion spec
            unsigned int m_argtype_size;
            bool m_argtype_signed:1;
            bool m_argval_negative:1;

          public:
            constexpr FResult(Output p, std::string_view fmt_) noexcept
            : fmt(fmt_), pos(0), arg_count(0), line(0), opts(),
              state(pstate_t::outside),
              m_out(std::move(p)), pos_lstart(0),
              m_argtype_size(0),
              m_argtype_signed(false),
              m_argval_negative(false)
              { }

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
                .append("`, type[signed ")
                .append(jau::to_string(m_argtype_signed))
                .append(", size ")
                .append(std::to_string(m_argtype_size))
                .append("], negative ")
                .append(jau::to_string(m_argval_negative))
                .append("], fmt `")
                .append(fmt)
                .append("`, out `")
                .append(m_out.get())
                .append("`");
                return s;
            }

          private:
            friend class impl::Parser<Output>;

            constexpr void reset() noexcept {
                opts.reset();
            }
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
                    reset();
                    return true;
                } else if (pos < fmt.length()) {
                    // seek next conversion specifier
                    const size_t q = fmt.find('%', pos + 1);
                    if (q == std::string::npos) {
                        // no conversion specifier found, end of format
                        appendText(fmt.substr(pos, fmt.length() - pos));
                        pos = fmt.length();
                        return false;
                    } else {
                        // new conversion specifier found
                        appendText(fmt.substr(pos, q - pos));
                        state = pstate_t::start;
                        pos_lstart = pos;
                        pos = q + 1;
                        reset();
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
                requires jau::req::stringifyable_jau<T> && (!jau::req::unsigned_integral<T>) && (!std::floating_point<T>)
            constexpr void appendFormatted(const T &v) {
                m_out.appendFormatted(opts, v);
            }
            template<typename T>
            requires jau::req::unsigned_integral<T> && (!jau::req::boolean<T>)
            constexpr void appendFormatted(const T &v) {
                m_out.appendFormattedInt(opts, v, m_argval_negative);
            }

            template<typename T>
                requires std::floating_point<T>
            constexpr void appendFormatted(const T &v) {
                m_out.appendFormattedFloat(opts, v, m_argtype_size);
            }

            constexpr void appendText(const std::string_view v) {
                m_out.appendText(v);
            }

            constexpr void appendError(const std::string_view tag) {
                const ssize_t c = arg_count == std::numeric_limits<ssize_t>::min() ? 0 : arg_count;
                m_out.appendError(jau::abs(c), line, tag);
            }
        };

        template <jau::req::signed_integral T>
        constexpr T abs_int(const T x) noexcept
        {
            return jau::sign<T>(x) < 0 ? jau::invert_sign<T>( x ) : x;
        }

        template<typename T>
        requires (!jau::req::signed_integral<T>)
        constexpr T abs_int(const T& x) noexcept {
            return x;
        }

        template<typename T>
        requires jau::req::signed_integral<T>
        constexpr bool is_positive(const T a) noexcept {
            return a >= 0;
        }

        template<typename T>
        requires std::floating_point<T>
        constexpr bool is_positive(const T a) noexcept {
            return a >= 0;
        }

        template<typename T>
        requires (!jau::req::signed_integral<T>) && (!std::floating_point<T>)
        constexpr bool is_positive(const T&) noexcept {
            return true;
        }

        /// Returns uint64_t type if: integral || boolean, otherwise returns orig type
        template<typename T>
        using make_int_unsigned_t = typename std::conditional_t<std::is_integral_v<T> || jau::req::boolean<T>, std::type_identity<uint64_t>, std::type_identity<T>>::type;  // NOLINT

        /// Returns signed-type variation if: unsigned-integral && !boolean, otherwise returns orig type
        template<typename T>
        using make_int_signed_t = typename std::conditional_t<jau::req::unsigned_integral<T> && !jau::req::boolean<T>, std::make_signed<T>, std::type_identity<T>>::type;  // NOLINT

        /// Returns a simple `const char * const` if: pointer, otherwise returns orig type
        template<typename T>
        using make_simple_pointer_t = typename std::conditional_t<jau::req::pointer<T>, std::type_identity<const char * const>, std::type_identity<T>>::type;  // NOLINT

        // A NullOutput formatting result, capable of `constexpr` and `consteval`
        typedef FResult<NullOutput> CheckResult;

        /// A StringOutput formatting result for runtime formatting into a std::string
        typedef FResult<StringOutput> StringResult;

        template<OutputType Output>
        class Parser {
          public:
            typedef FResult<Output> Result;
            constexpr Parser() noexcept = delete;

            // Note: parseOne using StringResult is not consteval nor noexcept (string ops)
            // Note: checkOne using CheckResult is consteval and noexcept (zero string ops)

            template <typename T>
            requires std::is_integral_v<T>
            static constexpr void parseOne(Result &pc, const T &val) {
                pc.m_argtype_size = sizeof(T);
                pc.m_argtype_signed = std::is_signed_v<T>;
                pc.m_argval_negative = !is_positive(val);
                using U = make_int_unsigned_t<T>;
                parseOneImpl<U>(pc, U(abs_int(val))); // uint64_t
            }

            template <typename T>
            requires std::is_floating_point_v<T>
            static constexpr void parseOne(Result &pc, const T &val) {
                pc.m_argtype_size = sizeof(T);
                pc.m_argtype_signed = true;
                pc.m_argval_negative = !is_positive(val);
                parseOneImpl<double>(pc, double(val)); // double
            }

            template <typename T>
            requires jau::req::pointer<T> // also allows passing `char*` for `%p`
            static constexpr void parseOne(Result &pc, const T &val) {
                pc.m_argtype_size = sizeof(T); // NOLINT(bugprone-sizeof-expression)
                pc.m_argtype_signed = false;
                pc.m_argval_negative = false;
                using U = make_simple_pointer_t<T>; // aliasing to 'char*'
                parseOneImpl<U>(pc, U(val)); // pass-through
            }

            template <typename T>
            requires jau::req::string_literal<T> || jau::req::string_class<T>
            static constexpr void parseOne(Result &pc, const T &val) {
                pc.m_argtype_size = sizeof(T); // NOLINT(bugprone-sizeof-expression)
                pc.m_argtype_signed = false;
                pc.m_argval_negative = false;
                parseOneImpl<std::string_view>(pc, std::string_view(val)); // pass as string_view
            }

            template <typename T>
            requires (!(std::is_integral_v<T> || std::is_floating_point_v<T> || jau::req::pointer<T> || jau::req::string_alike<T>)) // not: jau::req::string_literal<T> || jau::req::string_class<T> || jau::req::char_pointer<T>
            static constexpr void parseOne(Result &pc, const T &val) {
                pc.m_argtype_size = sizeof(T); // NOLINT(bugprone-sizeof-expression)
                pc.m_argtype_signed = std::is_signed_v<T>;
                pc.m_argval_negative = !is_positive(val);
                parseOneImpl<T>(pc, val); // pass-through
            }

            template <typename T>
            requires std::is_integral_v<T>
            static constexpr void checkOne(CheckResult &pc) noexcept {
                pc.m_argtype_size = sizeof(T);
                pc.m_argtype_signed = std::is_signed_v<T>;
                pc.m_argval_negative = false;
                using U = make_int_unsigned_t<T>;
                parseOneImpl<U>(pc, U()); // uint64_t
            }

            template <typename T>
            requires std::is_floating_point_v<T>
            static constexpr void checkOne(CheckResult &pc) noexcept {
                pc.m_argtype_size = sizeof(T);
                pc.m_argtype_signed = true;
                pc.m_argval_negative = false;
                parseOneImpl<double>(pc, double()); // double
            }

            template <typename T>
            requires jau::req::pointer<T> // also allows passing `char*` for `%p`
            static constexpr void checkOne(CheckResult &pc) noexcept {
                pc.m_argtype_size = sizeof(T); // NOLINT(bugprone-sizeof-expression)
                pc.m_argtype_signed = false;
                pc.m_argval_negative = false;
                using U = jau::req::const2_pointer<T>;
                parseOneImpl<U>(pc, U()); // pass-through
            }

            template <typename T>
            requires jau::req::string_literal<T> || jau::req::string_class<T>
            static constexpr void checkOne(CheckResult &pc) noexcept {
                pc.m_argtype_size = sizeof(T); // NOLINT(bugprone-sizeof-expression)
                pc.m_argtype_signed = false;
                pc.m_argval_negative = false;
                parseOneImpl<std::string_view>(pc, std::string_view()); // pass as string_view
            }

            template <typename T>
            requires (!(std::is_integral_v<T> || std::is_floating_point_v<T> || jau::req::pointer<T> || jau::req::string_alike<T>)) // not: jau::req::string_literal<T> || jau::req::string_class<T> || jau::req::char_pointer<T>
            static constexpr void checkOne(CheckResult &pc) noexcept {
                pc.m_argtype_size = sizeof(T); // NOLINT(bugprone-sizeof-expression)
                pc.m_argtype_signed = std::is_signed_v<T>;
                pc.m_argval_negative = false;
                parseOneImpl<T>(pc, T()); // pass-through
            }

          private:

            /**
             * Parse the given argument against the current conversion specifier of the format string.
             *
             * Multiple rounds of parsing calls might be required, each passing the next argument or null.
             *
             * Parsing is completed when method returns false, either signaling an error() or completion.
             *
             * Caller only checks error status in the end, since all arguments will be processed due to folding parameter pack
             *
             * Hidden in private to only access via public constrained template buddies!
             *
             * @tparam T The type of the given argument
             * @return true if no error _and_ not complete, i.e. further calls with subsequent parameter required. Otherwise parsing ended due to error or completeness.
             */
            template <typename T>
            static constexpr void parseOneImpl(Result &pc, const T &val) {
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

            static constexpr void parseFlags(Result &pc, char &c) noexcept {
                while( pc.opts.addFlag(c) && pc.nextSymbol(c) ) { }
            }

            /// Parse argument field width or precision, returns false on error. Otherwise next argument is required.
            template <typename T>
            requires (!jau::req::unsigned_integral<T>)
            static constexpr void parseArgWidthPrecision(bool, Result &pc, const T &) noexcept {
                pc.setError(__LINE__);
            }
            /// Parse argument field width or precision, returns false on error. Otherwise next argument is required.
            template <typename T>
            requires (jau::req::unsigned_integral<T>)
            static constexpr void parseArgWidthPrecision(bool is_width, Result &pc, const T &val) noexcept {
                if constexpr( std::is_same_v<no_type_t, T> ) {
                    pc.setError(__LINE__);
                    return;  // error
                }
                ++pc.arg_count;
                if ( pc.m_argtype_size > sizeof(int) ) { // NOLINT(bugprone-sizeof-expression)
                    pc.setError(__LINE__);
                    return;  // error
                }
                if( pc.m_argval_negative ) {
                    if( is_width ) {
                        pc.opts.flags |= flags_t::left; // reverse padding
                        pc.opts.setWidth((uint32_t)val);
                    } else {
                        pc.opts.setPrecision(0);
                    }
                } else {
                    if( is_width ) {
                        pc.opts.setWidth((uint32_t)val);
                    } else {
                        pc.opts.setPrecision((uint32_t)val);
                    }
                }
                // next argument is required
            }

            /// Parse format field width or precision, returns true if field is consumed and parsing can continue
            /// or false if field has not been consumed or definite error
            static constexpr bool parseFmtWidthPrecision(bool is_width, Result &pc, char &c) noexcept {
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
                if( num < 0 || num > std::numeric_limits<int>::max() ) {
                    // pc.setError(__LINE__); // number syntax
                    return false;  // error
                }
                if( is_width ) {
                    pc.opts.setWidth((uint32_t)num);
                } else {
                    pc.opts.setPrecision((uint32_t)num);
                }
                return true;  // continue with current argument
            }

            /* parse length modifier, returns true if parsing can continue or false on error. */
            static constexpr bool parseLengthMods(Result &pc, char &c) noexcept {
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
            static constexpr bool parseFmtSpec(Result &pc, char fmt_literal, const T &val) {
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
            requires (!jau::req::unsigned_integral<T>)
            static constexpr bool parseCharFmtSpec(Result &pc, const T &) noexcept {
                if constexpr( !std::is_same_v<no_type_t, T> ) {
                    ++pc.arg_count;
                }
                pc.setError(__LINE__);
                return false;
            }

            template <typename T>
            requires jau::req::unsigned_integral<T> //  && (!jau::req::boolean<T>)
            static constexpr bool parseCharFmtSpec(Result &pc, const T &val0) {
                ++pc.arg_count;

                using V = make_int_signed_t<T>; // restore signed type!
                const V val = V(val0);
                const V sign = pc.m_argval_negative ? -1 : 1;

                switch( pc.opts.length_mod ) {
                    case plength_t::none: {
                        if ( !pc.m_argtype_signed ||
                             ( sizeof(char) != pc.m_argtype_size &&
                               sizeof(int) != pc.m_argtype_size ) ) {
                            pc.setError(__LINE__);
                            return false;
                        }
                        std::string s(1, (char)(val*sign));
                        pc.appendFormatted(std::string_view(s));
                    } break;
                    case plength_t::l: {
                        if ( !pc.m_argtype_signed ||
                             ( sizeof(wchar_t) != pc.m_argtype_size && // NOLINT(misc-redundant-expression)
                               sizeof(wint_t) != pc.m_argtype_size ) ) {
                            pc.setError(__LINE__);
                            return false;
                        }
                        std::string s(1, (char)(val*sign));
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
            static constexpr bool parseStringFmtSpec(Result &pc, const T &) noexcept {
                if constexpr( !std::is_same_v<no_type_t, T> ) {
                    ++pc.arg_count;
                }
                pc.setError(__LINE__);
                return false;
            }

            template <typename T>
            requires jau::req::string_alike<T>
            static constexpr bool parseStringFmtSpec(Result &pc, const T &val) {
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
            static constexpr bool parseAPointerFmtSpec(Result &pc, const T &) noexcept {
                if constexpr( !std::is_same_v<no_type_t, T> ) {
                    ++pc.arg_count;
                }
                pc.setError(__LINE__);
                return false;
            }
            template <typename T>
            requires jau::req::pointer<T>
            static constexpr bool parseAPointerFmtSpec(Result &pc, const T &val) {
                pc.opts.length_mod = plength_t::none;
                ++pc.arg_count;
                pc.appendFormatted((void *)const_cast<std::remove_const_t<T>>(val)); // force pointer type
                return true;
            }

            template <typename T>
            requires (!(jau::req::unsigned_integral<T> || std::is_enum_v<T>))
            static constexpr bool parseSignedFmtSpec(Result &pc, const T &) noexcept {
                if constexpr( !std::is_same_v<no_type_t, T> ) {
                    ++pc.arg_count;
                }
                pc.setError(__LINE__);
                return false;
            }
            template <typename T>
            requires std::is_enum_v<T> && (!jau::req::signed_integral<std::underlying_type_t<T>>)
            static constexpr bool parseSignedFmtSpec(Result &pc, const T &) noexcept {
                if constexpr( !std::is_same_v<no_type_t, T> ) {
                    ++pc.arg_count;
                }
                pc.setError(__LINE__);
                return false;
            }
            template <typename T>
            requires std::is_enum_v<T> && jau::req::signed_integral<std::underlying_type_t<T>>
            static constexpr bool parseSignedFmtSpec(Result &pc, const T &val0) {
                using U = std::underlying_type_t<T>;
                using V = make_int_unsigned_t<U>;
                const U u = U(val0);
                pc.m_argtype_signed = true;
                pc.m_argtype_size = sizeof(U);
                pc.m_argval_negative = !is_positive(u);
                return parseSignedFmtSpec<V>(pc, V(abs_int(u)));
            }
            template <typename T>
            requires jau::req::unsigned_integral<T>
            static constexpr bool parseSignedFmtSpec(Result &pc, const T &val) {
                ++pc.arg_count;

                // Only accepting unsigned -> signed, if sizeof(unsigned) < sizeof(signed)
                const unsigned int signed_argtype_size = pc.m_argtype_signed ? pc.m_argtype_size : pc.m_argtype_size + 1;

                if (jau::is_zero(val)) {
                    pc.opts.flags &= ~flags_t::hash;  // no hash for 0 values
                }

                // we accept given type <= integral target type

                switch( pc.opts.length_mod ) {
                    case plength_t::hh:
                        if ( signed_argtype_size > sizeof(char) ) { // NOLINT(bugprone-sizeof-expression)
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;
                    case plength_t::h:
                        if ( signed_argtype_size > sizeof(short) ) { // NOLINT(bugprone-sizeof-expression)
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;
                    case plength_t::none:
                        if ( signed_argtype_size > sizeof(int) ) { // NOLINT(bugprone-sizeof-expression)
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;
                    case plength_t::l:
                        if ( signed_argtype_size > sizeof(long) ) { // NOLINT(bugprone-sizeof-expression)
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;
                    case plength_t::ll:
                        if ( signed_argtype_size > sizeof(long long) ) { // NOLINT(bugprone-sizeof-expression)
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;
                    case plength_t::j:
                        if ( signed_argtype_size > sizeof(intmax_t) ) { // NOLINT(bugprone-sizeof-expression)
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;
                    case plength_t::z:
                        if ( signed_argtype_size > sizeof(ssize_t) ) { // NOLINT(bugprone-sizeof-expression)
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;
                    case plength_t::t:
                        if ( signed_argtype_size > sizeof(ptrdiff_t) ) { // NOLINT(bugprone-sizeof-expression)
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
            requires (!(jau::req::unsigned_integral<T> || std::is_enum_v<T>))
            static constexpr bool parseUnsignedFmtSpec(Result &pc, const T &) noexcept {
                if constexpr( !std::is_same_v<no_type_t, T> ) {
                    ++pc.arg_count;
                }
                pc.setError(__LINE__);
                return false;
            }
            template <typename T>
            requires std::is_enum_v<T>
            static constexpr bool parseUnsignedFmtSpec(Result &pc, const T &val0) {
                using U = std::underlying_type_t<T>;
                using V = make_int_unsigned_t<U>;
                const U u = U(val0);
                pc.m_argtype_signed = std::is_signed_v<U>;
                pc.m_argtype_size = sizeof(U);
                pc.m_argval_negative = !is_positive(u);
                return parseUnsignedFmtSpec<V>(pc, V(abs_int(u)));
            }
            template <typename T>
            requires jau::req::unsigned_integral<T>
            static constexpr bool parseUnsignedFmtSpec(Result &pc, const T &val) {
                ++pc.arg_count;

                // Accepting signed, but not negative
                if( pc.m_argval_negative ) {
                    pc.setError(__LINE__);
                    return false;
                }

                if (jau::is_zero(val)) {
                    pc.opts.flags &= ~flags_t::hash;  // no hash for 0 values
                }

                // we accept given type <= integral target type

                switch( pc.opts.length_mod ) {
                    case plength_t::hh:
                        if ( pc.m_argtype_size > sizeof(unsigned char) ) { // NOLINT(bugprone-sizeof-expression)
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;
                    case plength_t::h:
                        if ( pc.m_argtype_size > sizeof(unsigned short) ) { // NOLINT(bugprone-sizeof-expression)
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;
                    case plength_t::none:
                        if ( pc.m_argtype_size > sizeof(unsigned int) ) { // NOLINT(bugprone-sizeof-expression)
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;
                    case plength_t::l:
                        if ( pc.m_argtype_size > sizeof(unsigned long) ) { // NOLINT(bugprone-sizeof-expression)
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;
                    case plength_t::ll:
                        if ( pc.m_argtype_size > sizeof(unsigned long long) ) { // NOLINT(bugprone-sizeof-expression)
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;
                    case plength_t::j:
                        if ( pc.m_argtype_size > sizeof(uintmax_t) ) { // NOLINT(bugprone-sizeof-expression)
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;
                    case plength_t::z:
                        if ( pc.m_argtype_size > sizeof(size_t) ) { // NOLINT(bugprone-sizeof-expression)
                            pc.setError(__LINE__);
                            return false;
                        }
                        break;

                    case plength_t::t:
                        if ( pc.m_argtype_size > sizeof(ptrdiff_t) ) { // NOLINT(bugprone-sizeof-expression)
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
            static constexpr bool parseFloatFmtSpec(Result &pc, const char /*fmt_literal*/, const T &) noexcept {
                if constexpr( !std::is_same_v<no_type_t, T> ) {
                    ++pc.arg_count;
                }
                pc.setError(__LINE__);
                return false;
            }
            template <typename T>
            requires std::floating_point<T>
            static constexpr bool parseFloatFmtSpec(Result &pc, const char /*fmt_literal*/, const T &val) {
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

    //
    // Public format functions
    //

    /**
     * Strict format with type validation of arguments against the format string.
     *
     * See @ref jau_cfmt_header for details
     *
     * @tparam Targs the argument template type pack for the given arguments `args`
     * @param fmt the snprintf compliant format string
     * @param args passed arguments, used for template type deduction only
     * @return true if successfully parsed format and arguments, false otherwise.
     * @see @ref jau_cfmt_header
     */
    template <typename... Targs>
    inline __attribute__((always_inline))
    std::string format(std::string_view fmt, const Targs &...args) noexcept {
        std::string s;
        impl::StringResult ctx(impl::StringOutput(s.max_size(), s), fmt);

        std::exception_ptr eptr;
        try {
            if constexpr( 0 < sizeof...(Targs) ) {
                ((impl::FormatParser::parseOne<Targs>(ctx, args)), ...);
            }
            impl::FormatParser::parseOne<impl::no_type_t>(ctx, impl::no_type_t());
        } catch (...) {
            eptr = std::current_exception();
        }
        handle_exception(eptr, E_FILE_LINE);
        return s;
    }

    /**
     * Strict format with type validation of arguments against the format string.
     *
     * See @ref jau_cfmt_header for details
     *
     * @tparam Targs the argument template type pack for the given arguments `args`
     * @param maxLen maximum string length
     * @param fmt the snprintf compliant format string
     * @param args passed arguments, used for template type deduction only
     * @return true if successfully parsed format and arguments, false otherwise.
     * @see @ref jau_cfmt_header
     */
    template <typename... Targs>
    inline __attribute__((always_inline))
    std::string format(size_t maxLen, std::string_view fmt, const Targs &...args) noexcept {
        std::string s;
        impl::StringResult ctx(impl::StringOutput(std::min(maxLen, s.max_size()), s), fmt);

        std::exception_ptr eptr;
        try {
            if constexpr( 0 < sizeof...(Targs) ) {
                ((impl::FormatParser::parseOne<Targs>(ctx, args)), ...);
            }
            impl::FormatParser::parseOne<impl::no_type_t>(ctx, impl::no_type_t());
        } catch (...) {
            eptr = std::current_exception();
        }
        handle_exception(eptr, E_FILE_LINE);
        return s;
    }

    /**
     * Strict format with type validation of arguments against the format string,
     * appending to the given destination.
     *
     * See @ref jau_cfmt_header for details
     *
     * @tparam Targs the argument template type pack for the given arguments `args`
     * @param s destination string to append the formatted string
     * @param maxLen maximum string length
     * @param fmt the snprintf compliant format string
     * @param args passed arguments, used for template type deduction only
     * @return jau::cfmt::Result instance for further inspection
     * @see @ref jau_cfmt_header
     */
    template <typename... Targs>
    inline __attribute__((always_inline))
    Result formatR(std::string &s, size_t maxLen, std::string_view fmt, const Targs &...args) noexcept {
        impl::StringResult ctx(impl::StringOutput(std::min(maxLen, s.max_size()), s), fmt);

        std::exception_ptr eptr;
        try {
            if constexpr( 0 < sizeof...(Targs) ) {
                ((impl::FormatParser::parseOne<Targs>(ctx, args)), ...);
            }
            impl::FormatParser::parseOne<impl::no_type_t>(ctx, impl::no_type_t());
        } catch (...) {
            eptr = std::current_exception();
        }
        handle_exception(eptr, E_FILE_LINE);
        return ctx;
    }
    /**
     * Strict format with type validation of arguments against the format string,
     * appending to the given destination.
     *
     * See @ref jau_cfmt_header for details
     *
     * @tparam Targs the argument template type pack for the given arguments `args`
     * @param s destination string to append the formatted string
     * @param fmt the snprintf compliant format string
     * @param args passed arguments, used for template type deduction only
     * @return jau::cfmt::Result instance for further inspection
     * @see @ref jau_cfmt_header
     */
    template <typename... Targs>
    inline __attribute__((always_inline))
    Result formatR(std::string &s, std::string_view fmt, const Targs &...args) noexcept {
        return formatR(s, s.max_size(), fmt, args...);
    }

    /**
     * Strict format with type validation of arguments against the format string,
     * appending to the given destination.
     *
     * Resulting string is truncated to `min(maxLen, formatLen)`,
     * with `formatLen` being the given formatted string length of output w/o limitation
     * and its capacity is left unchanged.
     *
     * Use `std::string::shrink_to_fit()` on the returned string,
     * if you desire efficiency for longer lifecycles (assuming `maxLen` hasn't been reached).
     *
     * See @ref jau_cfmt_header for details
     *
     * @tparam Targs the argument template type pack for the given arguments `args`
     * @param strLenHint initially used string length w/o EOS
     * @param s destination string to append the formatted string
     * @param maxLen maximum resulting string length including EOS
     * @param fmt the snprintf compliant format string
     * @param args arguments matching the format string
     */
    template <typename... Targs>
    inline __attribute__((always_inline))
    Result formatR(const std::size_t strLenHint, std::string &s, size_t maxLen, std::string_view fmt, const Targs &...args) noexcept {
        impl::StringResult ctx(impl::StringOutput(std::min(maxLen, s.max_size()), s), fmt);

        std::exception_ptr eptr;
        try {
            s.reserve(strLenHint+1); // +EOS
            if constexpr( 0 < sizeof...(Targs) ) {
                ((impl::FormatParser::parseOne<Targs>(ctx, args)), ...);
            }
            impl::FormatParser::parseOne<impl::no_type_t>(ctx, impl::no_type_t());
        } catch (...) {
            eptr = std::current_exception();
        }
        handle_exception(eptr, E_FILE_LINE);
        return ctx;
    }

    /**
     * Strict format with type validation of arguments against the format string,
     * appending to the given destination.
     *
     * Resulting string size matches formated output w/o limitation
     * and its capacity is left unchanged.
     *
     * Use `std::string::shrink_to_fit()` on the returned string,
     * if you desire efficiency for longer lifecycles.
     *
     * See @ref jau_cfmt_header for details
     *
     * @tparam Targs the argument template type pack for the given arguments `args`
     * @param strLenHint initially used string length w/o EOS
     * @param fmt the snprintf compliant format string
     * @param args arguments matching the format string
     */
    template <typename... Targs>
    inline __attribute__((always_inline))
    Result formatR(const std::size_t strLenHint, std::string &s, std::string_view fmt, const Targs &...args) noexcept {
        return formatR(strLenHint, s, s.max_size(), fmt, args...);
    }

    //
    // Public check functions
    //

    /**
     * Strict compile-time type validation of deduced argument-types against the format string.
     *
     * In case your can't provide constexpr arguments,
     * use `check2()`. Types can always be produced, see macro `jau_string_checkLine()`.
     *
     * See @ref jau_cfmt_header for details
     *
     * @tparam Targs the argument template type pack to be validated against the format string
     * @param fmt the snprintf compliant format string
     * @param args passed arguments, used for template type deduction only
     * @return number of parser format arguments if successfully, otherwise negative number indicates first failed argument starting with -1
     *         w/ min(ssize_t) denoting the format string
     * @see @ref jau_cfmt_header
     */
    template <typename... Targs>
    consteval_cxx20 ssize_t check(std::string_view fmt, const Targs &...) noexcept {
        impl::CheckResult ctx(impl::NullOutput(), fmt);
        if constexpr( 0 < sizeof...(Targs) ) {
            ((impl::CheckParser::checkOne<Targs>(ctx)), ...);
        }
        impl::CheckParser::checkOne<impl::no_type_t>(ctx);
        return ctx.arg_count;
    }
    /**
     * Strict compile-time type validation of deduced argument-types against the format string.
     *
     * In case your can't provide constexpr arguments,
     * use `check2Line()`. Types can always be produced, see macro `jau_string_checkLine()`.
     *
     * See @ref jau_cfmt_header for details
     *
     * @tparam Targs the argument template type pack to be validated against the format string
     * @param fmt the snprintf compliant format string
     * @param args passed arguments, used for template type deduction only
     * @return 0 if successfully, otherwise the source code line number detecting the failure
     * @see @ref jau_cfmt_header
     */
    template <typename... Targs>
    consteval_cxx20 int checkLine(std::string_view fmt, const Targs &...) noexcept {
        impl::CheckResult ctx(impl::NullOutput(), fmt);
        if constexpr( 0 < sizeof...(Targs) ) {
            ((impl::CheckParser::checkOne<Targs>(ctx)), ...);
        }
        impl::CheckParser::checkOne<impl::no_type_t>(ctx);
        return ctx.line;
    }

    /**
     * Strict compile-time type validation of explicit argument-types against the format string.
     *
     * See @ref jau_cfmt_header for details
     *
     * @tparam Targs the argument template type pack to be validated against the format string
     * @param fmt the snprintf compliant format string
     * @return number of parser format arguments if successfully, otherwise negative number indicates first failed argument starting with -1
     *         w/ min(ssize_t) denoting the format string
     * @see @ref jau_cfmt_header
     */
    template <typename... Targs>
    consteval_cxx20 ssize_t check2(std::string_view fmt) noexcept {
        impl::CheckResult ctx(impl::NullOutput(), fmt);
        if constexpr( 0 < sizeof...(Targs) ) {
            ((impl::CheckParser::checkOne<Targs>(ctx)), ...);
        }
        impl::CheckParser::checkOne<impl::no_type_t>(ctx);
        return ctx.arg_count;
    }

    /**
     * Strict compile-time type validation of explicit argument-types against the format string.
     *
     * See @ref jau_cfmt_header for details
     *
     * @tparam Targs the argument template type pack to be validated against the format string
     * @param fmt the snprintf compliant format string
     * @return 0 if successfully, otherwise the source code line number detecting the failure
     * @see @ref jau_cfmt_header
     */
    template <typename... Targs>
    consteval_cxx20 int check2Line(std::string_view fmt) noexcept {
        impl::CheckResult ctx(impl::NullOutput(), fmt);
        if constexpr( 0 < sizeof...(Targs) ) {
            ((impl::CheckParser::checkOne<Targs>(ctx)), ...);
        }
        impl::CheckParser::checkOne<impl::no_type_t>(ctx);
        return ctx.line;
    }

    /**
     * Strict compile-time type validation of deduced argument-types against the format string.
     *
     * See @ref jau_cfmt_header for details
     *
     * @tparam Targs the argument template type pack to be validated against the format string
     * @param fmt the snprintf compliant format string
     * @param args passed arguments, used for template type deduction only
     * @return jau::cfmt::Result instance for further inspection
     * @see @ref jau_cfmt_header
     */
    template <typename... Targs>
    consteval_cxx20 Result checkR(std::string_view fmt, const Targs &...) noexcept {
        impl::CheckResult ctx(impl::NullOutput(), fmt);
        if constexpr( 0 < sizeof...(Targs) ) {
            ((impl::CheckParser::checkOne<Targs>(ctx)), ...);
        }
        impl::CheckParser::checkOne<impl::no_type_t>(ctx);
        return ctx;
    }

    /**
     * Strict compile-time type validation of explicit argument-types against the format string.
     *
     * See @ref jau_cfmt_header for details
     *
     * @tparam Targs the argument template type pack to be validated against the format string
     * @param fmt the snprintf compliant format string
     * @return jau::cfmt::Result instance for further inspection
     * @see @ref jau_cfmt_header
     */
    template <typename... Targs>
    consteval_cxx20 Result checkR2(std::string_view format) noexcept {
        impl::CheckResult ctx(impl::NullOutput(), format);
        if constexpr( 0 < sizeof...(Targs) ) {
            ((impl::CheckParser::checkOne<Targs>(ctx)), ...);
        }
        impl::CheckParser::checkOne<impl::no_type_t>(ctx);
        return ctx;
    }

    /**@}*/

}  // namespace jau::cfmt

namespace jau {
    /**
     * Safely returns a (potentially truncated) string according to `snprintf()` formatting rules
     * and variable number of arguments following the `fmt` argument.
     *
     * jau::cfmt::format() is utilize to validate `format` against given arguments at *runtime*.
     *
     * Resulting string is truncated to `min(maxLen, formatLen)`,
     * with `formatLen` being the given formatted string length of output w/o limitation.
     *
     * See @ref jau_cfmt_header for details
     *
     * @param maxLen maximum resulting string length including
     * @param fmt the snprintf compliant format string
     * @param args arguments matching the format string
     */
    template<typename... Args>
    inline __attribute__((always_inline))
    std::string format_string_n(const std::size_t maxLen, std::string_view fmt, const Args &...args) noexcept {
        return jau::cfmt::format(maxLen, fmt, args...);
    }

    /**
     * Safely returns a (non-truncated) string according to `snprintf()` formatting rules
     * and variable number of arguments following the `fmt` argument.
     *
     * jau::cfmt::formatR() is utilize to validate `format` against given arguments at *runtime*.
     *
     * Resulting string size matches formated output w/o limitation
     * and its capacity is left unchanged.
     *
     * Use `std::string::shrink_to_fit()` on the returned string,
     * if you desire efficiency for longer lifecycles.
     *
     * See @ref jau_cfmt_header for details
     *
     * @param strLenHint initially used string length w/o EOS
     * @param fmt the snprintf compliant format string
     * @param args arguments matching the format string
     */
    template <typename... Args>
    inline __attribute__((always_inline))
    std::string format_string_h(const std::size_t strLenHint, std::string_view fmt, const Args &...args) noexcept {
        std::string str;
        jau::cfmt::formatR(strLenHint, str, fmt, args...);
        return str;
    }

    /**
     * Safely returns a (potentially truncated) string according to `snprintf()` formatting rules
     * and variable number of arguments following the `fmt` argument.
     *
     * jau::cfmt::formatR() is utilize to validate `format` against given arguments at *runtime*.
     *
     * Resulting string is truncated to `min(maxLen, formatLen)`,
     * with `formatLen` being the given formatted string length of output w/o limitation
     * and its capacity is left unchanged.
     *
     * Use `std::string::shrink_to_fit()` on the returned string,
     * if you desire efficiency for longer lifecycles (assuming `maxLen` hasn't been reached).
     *
     * See @ref jau_cfmt_header for details
     *
     * @param strLenHint initially used string length w/o EOS
     * @param maxLen maximum resulting string length including EOS
     * @param fmt the snprintf compliant format string
     * @param args arguments matching the format string
     */
    template <typename... Args>
    inline __attribute__((always_inline))
    std::string format_string_hn(const std::size_t strLenHint, const std::size_t maxLen, std::string_view fmt, const Args &...args) noexcept {
        std::string str;
        jau::cfmt::formatR(strLenHint, str, maxLen, fmt, args...);
        return str;
    }

    /**
     * Safely returns a (non-truncated) string according to `snprintf()` formatting rules
     * using a reserved string length of jau::cfmt::default_string_capacity and
     * variable number of arguments following the `fmt` argument.
     *
     * jau::cfmt::formatR() is utilize to validate `format` against given arguments at *runtime*.
     *
     * Resulting string size matches formated output w/o limitation
     * and its capacity is left unchanged.
     *
     * Use `std::string::shrink_to_fit()` on the returned string,
     * if you desire efficiency for longer lifecycles.
     *
     * See @ref jau_cfmt_header for details
     *
     * @param fmt the snprintf compliant format string
     * @param args arguments matching the format string
     */
    template <typename... Args>
    inline __attribute__((always_inline))
    std::string format_string(std::string_view fmt, const Args &...args) noexcept {
        return jau::format_string_h(jau::cfmt::default_string_capacity, fmt, args...);
    }

    /**@}*/

} // namespace jau

/** \addtogroup StringUtils
 *
 *  @{
 */

#if 0
// TODO: gcc created multiple instances in shared-lib
//       However, `constexpr` can't be explicitly instantiated.
//
// Explicit instantiation declaration of template function
extern template void jau::cfmt::impl::FormatParser::parseOneImpl<jau::cfmt::impl::no_type_t>(
    typename jau::cfmt::impl::FormatParser::Result&,
    const jau::cfmt::impl::no_type_t&);
#endif

/**
 * Macro, safely returns a (non-truncated) string according to `snprintf()` formatting rules
 * using a reserved string length of jau::cfmt::default_string_capacity and
 * variable number of arguments following the `fmt` argument.
 *
 * This macro also produces compile time validation using a `static_assert`
 * against jau::cfmt::check2.
 *
 * jau::cfmt::formatR() is utilize to validate `format` against given arguments at *runtime*.
 *
 * Resulting string size matches formated output w/o limitation
 * and its capacity is left unchanged.
 *
 * Use `std::string::shrink_to_fit()` on the returned string,
 * if you desire efficiency for longer lifecycles.
 *
 * See @ref jau_cfmt_header for details
 *
 * @param format `printf()` compliant format string
 * @param args arguments matching the format string
 */
#define jau_format_string(fmt, ...) \
    jau::format_string((fmt) __VA_OPT__(,) __VA_ARGS__);  \
    static_assert(0 <= jau::cfmt::check2< JAU_FOR_EACH1_LIST(JAU_DECLTYPE_VALUE, __VA_ARGS__) >(fmt)); // compile time validation!

/**
 * Macro, safely returns a (non-truncated) string according to `snprintf()` formatting rules
 * and variable number of arguments following the `fmt` argument.
 *
 * This macro also produces compile time validation using a `static_assert`
 * against jau::cfmt::check2.
 *
 * jau::cfmt::formatR() is utilize to validate `format` against given arguments at *runtime*.
 *
 * Resulting string size matches formated output w/o limitation
 * and its capacity is left unchanged.
 *
 * Use `std::string::shrink_to_fit()` on the returned string,
 * if you desire efficiency for longer lifecycles.
 *
 * See @ref jau_cfmt_header for details
 *
 * @param strLenHint initially used string length w/o EOS
 * @param format `printf()` compliant format string
 * @param args arguments matching the format string
 */
#define jau_format_string_h(strLenHint, fmt, ...) \
    jau::format_string((strLenHint), (fmt) __VA_OPT__(,) __VA_ARGS__);  \
    static_assert(0 <= jau::cfmt::check2< JAU_FOR_EACH1_LIST(JAU_DECLTYPE_VALUE, __VA_ARGS__) >(fmt)); // compile time validation!

/**
 * Macro, safely returns a (non-truncated) string according to `snprintf()` formatting rules
 * using a reserved string length of jau::cfmt::default_string_capacity and
 * variable number of arguments following the `fmt` argument.
 *
 * This macro also produces compile time validation using a `static_assert`
 * against jau::cfmt::check2Line.
 *
 * jau::cfmt::formatR() is utilize to validate `format` against given arguments at *runtime*.
 *
 * Resulting string size matches formated output w/o limitation
 * and its capacity is left unchanged.
 *
 * Use `std::string::shrink_to_fit()` on the returned string,
 * if you desire efficiency for longer lifecycles.
 *
 * See @ref jau_cfmt_header for details
 *
 * @param format `printf()` compliant format string
 * @param args arguments matching the format string
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
 * @param args arguments matching the format string
 */
#define jau_format_check(fmt, ...) \
    static_assert(0 <= jau::cfmt::check2< JAU_FOR_EACH1_LIST(JAU_DECLTYPE_VALUE, __VA_ARGS__) >(fmt)); // compile time validation!

/**
 * Macro produces compile time validation using a `static_assert`
 * against jau::cfmt::check2Line.
 *
 * See @ref jau_cfmt_header for details
 *
 * @param format `printf()` compliant format string
 * @param args arguments matching the format string
 */
#define jau_format_checkLine(fmt, ...) \
    static_assert(0 == jau::cfmt::check2Line< JAU_FOR_EACH1_LIST(JAU_DECLTYPE_VALUE, __VA_ARGS__) >(fmt)); // compile time validation!

/**@}*/

/** \example test_stringfmt_format.cpp
 * This C++ unit test validates most of the supported format specifiers against its arguments.
 */

/** \example test_stringfmt_perf.cpp
 * This C++ unit test benchmarks the jau::cfmt implementation against `snprintf` and `std::ostringstream`.
 */

/** \example test_stringfmt_check.cpp
 * This C++ unit test validates specific aspects of the implementation.
 */

#endif  // JAU_STRING_CFMT_HPP_
