/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020-2026 Gothel Software e.K.
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

#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <exception>
#include <limits>
#include <string>
#include "jau/type_concepts.hpp"

#include <jau/base_math.hpp>
#include <jau/basic_types.hpp>
#include <jau/cpp_lang_util.hpp>
#include <jau/float_math.hpp>
#include <jau/int_math.hpp>
#include <jau/int_types.hpp>
#include <jau/string_cfmt.hpp>

using namespace jau;

//
// jau::cfmt
//

namespace jau::cfmt {
    JAU_MAKE_BITFIELD_ENUM_STRING_CODE(flags_t, hash, zeropad, left, space, plus, thousands, uppercase);
    JAU_MAKE_ENUM_STRING_CODE(plength_t, hh, h, l, ll, L, j, z, t, any);
    JAU_MAKE_ENUM_STRING_CODE(cspec_t, any, character, string, pointer, signed_int, unsigned_int,
        floating_point, exp_float, alt_float, hex_float);
}

/// Reconstructs format string
std::string jau::cfmt::FormatOpts::toFormat() const {
    std::string s;
    s.reserve(31);
    s.append("%");
    if( is_set(flags, flags_t::hash) ) { s.append("#"); }
    if( is_set(flags, flags_t::thousands) ) { s.append("'"); }
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
#ifdef JAU_CFMT_TRACK_FORMAT_OPTS_LEN
    if( plength_t::none != length_mod && plength_t::any!= length_mod ) {
        s.append(to_string(length_mod));
    }
#endif
    if (conversion == cspec_t::unsigned_int) {
        if( 16 == radix ) {
            s.append( is_set(flags, flags_t::uppercase) ? "X" : "x" );
        } else if( 8 == radix ) {
            s.append("o");
        } else if( 2 == radix ) {
            s.append("b");
        } else {
            s.append("u");
        }
    } else {
        s.append(1, to_fmt_spec(conversion));
    }
    return s;
}

std::string jau::cfmt::FormatOpts::toString() const {
    std::string s = "fmt `";
#ifdef JAU_CFMT_TRACK_FORMAT_OPTS_FMT
    s.append(fmt).append("` -> `");
#endif
    s.append(toFormat())
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
#ifdef JAU_CFMT_TRACK_FORMAT_OPTS_LEN
    s.append(", length `")
     .append(to_string(length_mod));
#endif
    s.append("`, cspec ").append(to_string(conversion))
     .append(", radix ").append(std::to_string(radix));
    return s;
}

std::string jau::cfmt::Result::toString() const noexcept {
    const char c = m_pos < m_fmt.cend() ? *m_pos : '@';
    try {
        std::string s = "args ";
        s.append(std::to_string(m_arg_count))
        .append(", ok ")
        .append(jau::to_string(m_success))
        .append(", line ")
        .append(std::to_string(m_line))
        .append(", pos ")
        .append(std::to_string(index()))
        .append(", char `")
        .append(std::string(1, c))
        .append("`, last[").append(m_opts.toString())
        .append("], fmt `").append(m_fmt)
        .append("`");
        return s;
    } catch (...) {
        fput_exception(stderr, std::current_exception(), E_FILE_LINE);
        return std::string();
    }
}

void jau::cfmt::impl::append_rev(std::string &dest, const size_t dest_maxlen, std::string_view src, bool prec_cut, bool reverse, const FormatOpts &opts) noexcept {
    if (!dest_maxlen) [[unlikely]] {
        return;
    }
    size_t src_len = src.size();
    const char *p = src.data();
    const size_t dest_start_len = dest.size();

    // pre padding
    if (prec_cut && opts.precision_set) {
        src_len = jau::min<size_t>(src_len, opts.precision);
    }
    size_t space_left = 0, space_right = 0, added_len;
    {
        // string optional re-capacity and resize
        const size_t added_maxlen = dest_maxlen - dest_start_len;
        size_t len = jau::min(src_len, added_maxlen); // already cut to precision if applicable
        if (!is_set(opts.flags, flags_t::left) && opts.width_set && opts.width > len) {
            space_left = jau::min(opts.width-len, added_maxlen-len);
            len += space_left;
        }
        // p2: append pad spaces left/right up to given width
        if (opts.width_set && len < opts.width) {
            if (is_set(opts.flags, flags_t::left)) [[unlikely]] {
                space_right = jau::min(opts.width-len, added_maxlen-len);
                len += space_right;
            } else if (!is_set(opts.flags, flags_t::zeropad)) {
                space_left = jau::min(opts.width-len, added_maxlen-len);
                len += space_left;
            }
        }
        if (!jau::reserve_setsize_string(dest, dest_start_len + len)) [[unlikely]] { // cap +EOS, not shrinking!
            return;
        }
        added_len = len;
    }
    char *const d_start = dest.data() + dest_start_len;
    char *const d_left = d_start + space_left;
    char *d = dest.data() + dest_start_len + added_len;
    *d = 0; // EOS (is reserved)

    d -= space_right;
    ::memset(d, ' ', space_right);
    assert(d_left <= d);

    // string
    if (!reverse) [[likely]] {
        const size_t len = d-d_left;
        std::memcpy(d_left, p, len);
        // std::copy(p, p+len, d_left);
        d -= len;
    } else {
        while (d_left<d) {
            *(--d) = *(p++);
        }
    }
    assert(d_left <= d);

    ::memset(d - space_left, ' ', space_left);
}

template<jau::req::unsigned_integer T>
CXX_ALWAYS_INLINE
static void append_integralXX(std::string &dest, const size_t dest_maxlen, T v, const bool negative, const jau::cfmt::FormatOpts &opts, const bool inject_dot) noexcept {
    using namespace jau::cfmt;

    if (!dest_maxlen) [[unlikely]] {
        return;
    }
    const size_t dest_start_len = dest.size();
    const uint32_t added_maxlen = jau::min(uint32_t(dest_maxlen - dest_start_len), std::numeric_limits<uint32_t>::max());

    const uint32_t radix = opts.radix;
    const uint32_t sep_gap = 4;
    uint32_t val_digits; // includes inject_dot, excludes separator
    char buf_[number_max_strlen];
    const char * const buf_end = buf_ + jau::min(added_maxlen, number_max_strlen);
    const char * buf_start;
    if( opts.precision_set && opts.precision == 0 && jau::is_zero(v) ) [[unlikely]] {
        val_digits = 0;
        buf_start = buf_end;
    } else {
        char *d = const_cast<char *>(buf_end);
        if (10 == radix) [[likely]] {
            do {
                *(--d) = char('0' + (v % 10u));
                v /= 10u;
            } while (v && d > buf_);
        } else {
            const char *hex_array = is_set(opts.flags, flags_t::uppercase) ? HexadecimalArrayBig : HexadecimalArrayLow;
            if (16 == radix) [[likely]] {
                constexpr uint32_t shift = 4;
                do {
                    *(--d) = hex_array[v & 0x0fu];
                    v >>= shift;
                } while (v && d > buf_);
            } else {
                const T mask = radix - 1u;
                uint32_t shift;
                switch (radix) {
                    // case 16: shift = 4; break;
                    // case 10: shift = 0; break;
                    case 8:  shift = 3; break;
                    case 2:  shift = 1; break;
                    default: return;
                }
                do {
                    *(--d) = hex_array[v & mask];
                    v >>= shift;
                } while (v && d > buf_);
            }
            if (inject_dot && d > buf_ + 1) [[unlikely]] {
                *(d-1) = *d;
                *d = '.';
                --d;
            }
        }
        buf_start = d;
        val_digits = buf_end - buf_start;
        assert(val_digits <= added_maxlen);
    }
    char separator;
    uint32_t num_len; ///< contains zero-padding, val_digits and separator
    if (is_set(opts.flags, flags_t::thousands)) [[unlikely]] {
        separator = Config::default_thousand_separator;
        num_len = jau::min(added_maxlen, val_digits + ( (val_digits - 1) / sep_gap ));
    } else {
        separator = (char)0;
        num_len = val_digits;
    }

    const uint32_t prec = opts.precision_set ? opts.precision : 0;
    uint32_t width = opts.width_set ? jau::min(added_maxlen, opts.width) : 0;
    uint32_t space_left = 0, space_right = 0;
    uint32_t xtra_left = 0;  ///< contains hash, sign, single space
    size_t dest_len;
    {
        const uint32_t num_len0 = num_len; ///< initial num_len digits + sep_count

        // p1: pad leading zeros
        if (!is_set(opts.flags, flags_t::left)) [[likely]] {
            if (is_set(opts.flags, flags_t::zeropad) && width && (negative || has_any(opts.flags, flags_t::plus | flags_t::space))) [[unlikely]] {
                --width;
            }
            if (num_len < prec) [[unlikely]] {
                num_len += prec - num_len;
            }
            if (is_set(opts.flags, flags_t::zeropad) && num_len < width) [[unlikely]] {
                num_len += width - num_len;
            }
        }
        // p1: handle hash
        if (is_set(opts.flags, flags_t::hash)) {
            if (!opts.precision_set && num_len>num_len0 && ((num_len == prec) || (num_len == width))) {
                --num_len;
                if (num_len>num_len0 && radix != 8) [[likely]] { // 2 || 16, b/c only 2, 8, 16 is possible here. 10 has hash-flag cleared
                    --num_len;
                }
            }
            if (radix != 8) [[likely]] { // 2 || 16, b/c only 2, 8, 16 is possible here. 10 has hash-flag cleared
                ++xtra_left;
            }
            ++xtra_left; // hash zero
        }
        num_len = jau::min(added_maxlen, num_len);

        // p1: sign
        if (negative) [[unlikely]] {
            ++xtra_left; // '-';
        } else if (is_set(opts.flags, flags_t::plus)) [[unlikely]] {
            ++xtra_left; // '+';  // ignore the space if the '+' exists
        }

        // p1: space
        if (!negative && is_set(opts.flags, flags_t::space)) [[unlikely]] {
            ++xtra_left; // ' ';
        }
        xtra_left = jau::min(added_maxlen-num_len, xtra_left);

        // p2: append pad spaces left/right up to given width
        {
            const uint32_t len = xtra_left + num_len;
            if (len < width) {
                if (is_set(opts.flags, flags_t::left)) [[unlikely]] {
                    space_right = width - len;
                } else if (!is_set(opts.flags, flags_t::zeropad)) [[likely]] {
                    space_left = width - len;
                }
            }
        }
        space_left = jau::min(added_maxlen-num_len-xtra_left, space_left);
        space_right = jau::min(added_maxlen-num_len-xtra_left-space_left, space_right);
        dest_len = dest_start_len + jau::min(added_maxlen, space_left + xtra_left + num_len + space_right);
        if (!jau::reserve_setsize_string(dest, dest_len)) [[unlikely]] {
            return;
        }
#if !defined(NDEBUG) && 0
        fprintf(stderr, "XXX.80: separator '%c', opts %s\n", separator, opts.toString().c_str());
        fprintf(stderr, "XXX.80: negative %d, val %u\n", (int)negative, v);
        fprintf(stderr, "XXX.80: num_len %u (digits %u + sep %u + .. ), xleft %u, space[l %u, r %u] -> len %u\n",
                num_len, val_digits, sep_count, xtra_left, space_left, space_right, (space_left + xtra_left + num_len + space_right));
        fprintf(stderr, "XXX.80: total len[old %zu, added %zu, len %zu], number_start %u\n", dest_start_len, dest_len-dest_start_len, dest.size(), xtra_left + space_left);
#endif
    }
    const char *const d_start = dest.data() + dest_start_len;
    const char *const d_start_num = d_start + size_t(space_left + xtra_left);
    char *d = dest.data() + dest_len;
    *d = 0; // EOS (is reserved)
    d -= space_right;
    ::memset(d, ' ', space_right);

    const char *const d_end_num = d;
    assert(d_end_num >= d_start_num);
    assert(d_end_num - d_start_num == num_len);
    {
        if( !separator ) [[likely]] {
            d -= val_digits;
            ::memcpy(d, buf_start, val_digits);

            // zero-padding
            const size_t zlen = d - d_start_num;
            d = const_cast<char *>(d_start_num);
            ::memset(d, '0', zlen);
        } else {
            const char * p = buf_end;
            uint32_t sep_count = (num_len - 1) / sep_gap; // final separator count
            uint32_t digit_cnt = 0;
            while (d > d_start_num) [[likely]] {
                if (sep_count && digit_cnt && 0 == digit_cnt % sep_gap) [[unlikely]] {
                    *(--d) = separator;
                    --sep_count;
                    if (d == d_start_num) [[unlikely]] {
                        break; // done
                    }
                }
                if (digit_cnt >= val_digits) [[unlikely]] {
                    *(--d) = '0'; // zero-padding
                } else {
                    *(--d) = *(--p);
                }
                ++digit_cnt;
            }
            assert(p == buf_start);
        }
    }
    assert(d == d_start_num);
    assert(d >= d_start);

    // p1: handle hash
    if (is_set(opts.flags, flags_t::hash) && d > d_start) {
        size_t len = d_end_num - d;  // total length so far
        if (!opts.precision_set && len && ((len == prec) || (len == width))) {
            ++d;
            --len;
            if (len && radix != 8) [[likely]] { // 2 || 16, b/c only 2, 8, 16 is possible here. 10 has hash-flag cleared
                ++d;
                // --len;
            }
        }
        assert(d > d_start);
        if (radix == 16) [[likely]] {
            *(--d) = is_set(opts.flags, flags_t::uppercase) ? 'X' : 'x';
        } else if (radix == 2) {
            *(--d) = 'b';
        }

        assert(d > d_start);
        *(--d) = '0';
    }
    assert(d >= d_start);

    if (negative) [[unlikely]] {
        assert(d > d_start);
        *(--d) = '-';
    } else if (is_set(opts.flags, flags_t::plus)) [[unlikely]] {
        assert(d > d_start);
        *(--d) = '+';  // ignore the space if the '+' exists
    } else if (is_set(opts.flags, flags_t::space)) [[unlikely]] {
        assert(d > d_start);
        *(--d) = ' ';
    }
    assert(d == d_start + space_left);

    ::memset(d - space_left, ' ', space_left);
}

template<jau::req::unsigned_integer T>
CXX_ALWAYS_INLINE
static void append_integralXXRadix10(std::string &dest, const size_t dest_maxlen, T v, const bool negative, const jau::cfmt::FormatOpts &opts) noexcept {
    using namespace jau::cfmt;

    if (!dest_maxlen) [[unlikely]] {
        return;
    }
    const size_t dest_start_len = dest.size();
    const uint32_t added_maxlen = jau::min(uint32_t(dest_maxlen - dest_start_len), std::numeric_limits<uint32_t>::max());

    const uint32_t sep_gap = 3;
    uint32_t val_digits; // includes inject_dot, excludes separator
    char buf_[number_max_strlen];
    const char * const buf_end = buf_ + jau::min(added_maxlen, number_max_strlen);
    const char * buf_start;
    if( opts.precision_set && opts.precision == 0 && jau::is_zero(v) ) [[unlikely]] {
        val_digits = 0;
        buf_start = buf_end;
    } else {
        char *d = const_cast<char *>(buf_end);
        do {
            *(--d) = char('0' + (v % 10u));
            v /= 10u;
        } while (v && d > buf_);
        buf_start = d;
        val_digits = buf_end - buf_start;
        assert(val_digits <= added_maxlen);
    }
    char separator;
    uint32_t num_len; ///< contains zero-padding, val_digits and separator
    if (is_set(opts.flags, flags_t::thousands)) [[unlikely]] {
        separator = Config::default_thousand_separator;
        num_len = jau::min(added_maxlen, val_digits + ( (val_digits - 1) / sep_gap ));
    } else {
        separator = (char)0;
        num_len = val_digits;
    }

    const uint32_t prec = opts.precision_set ? opts.precision : 0;
    uint32_t width = opts.width_set ? jau::min(added_maxlen, opts.width) : 0;
    uint32_t space_left = 0, space_right = 0;
    uint32_t xtra_left = 0;  ///< contains hash, sign, single space
    size_t dest_len;
    {
        // p1: pad leading zeros
        if (!is_set(opts.flags, flags_t::left)) [[likely]] {
            if (is_set(opts.flags, flags_t::zeropad) && width && (negative || has_any(opts.flags, flags_t::plus | flags_t::space))) [[unlikely]] {
                --width;
            }
            if (num_len < prec) [[unlikely]] {
                num_len += prec - num_len;
            }
            if (is_set(opts.flags, flags_t::zeropad) && num_len < width) [[unlikely]] {
                num_len += width - num_len;
            }
        }
        num_len = jau::min(added_maxlen, num_len);

        // p1: sign
        if (negative) [[unlikely]] {
            ++xtra_left; // '-';
        } else if (is_set(opts.flags, flags_t::plus)) [[unlikely]] {
            ++xtra_left; // '+';  // ignore the space if the '+' exists
        }

        // p1: space
        if (!negative && is_set(opts.flags, flags_t::space)) [[unlikely]] {
            ++xtra_left; // ' ';
        }
        xtra_left = jau::min(added_maxlen-num_len, xtra_left);

        // p2: append pad spaces left/right up to given width
        {
            const uint32_t len = xtra_left + num_len;
            if (len < width) {
                if (is_set(opts.flags, flags_t::left)) [[unlikely]] {
                    space_right = width - len;
                } else if (!is_set(opts.flags, flags_t::zeropad)) [[likely]] {
                    space_left = width - len;
                }
            }
        }
        space_left = jau::min(added_maxlen-num_len-xtra_left, space_left);
        space_right = jau::min(added_maxlen-num_len-xtra_left-space_left, space_right);
        dest_len = dest_start_len + jau::min(added_maxlen, space_left + xtra_left + num_len + space_right);
        if (!jau::reserve_setsize_string(dest, dest_len)) [[unlikely]] {
            return;
        }
#if !defined(NDEBUG) && 0
        fprintf(stderr, "XXX.80: separator '%c', opts %s\n", separator, opts.toString().c_str());
        fprintf(stderr, "XXX.80: negative %d, val %u\n", (int)negative, v);
        fprintf(stderr, "XXX.80: num_len %u (digits %u + sep %u + .. ), xleft %u, space[l %u, r %u] -> len %u\n",
                num_len, val_digits, sep_count, xtra_left, space_left, space_right, (space_left + xtra_left + num_len + space_right));
        fprintf(stderr, "XXX.80: total len[old %zu, added %zu, len %zu], number_start %u\n", dest_start_len, dest_len-dest_start_len, dest.size(), xtra_left + space_left);
#endif
    }
    const char *const d_start = dest.data() + dest_start_len;
    const char *const d_start_num = d_start + size_t(space_left + xtra_left);
    char *d = dest.data() + dest_len;
    *d = 0; // EOS (is reserved)
    d -= space_right;
    ::memset(d, ' ', space_right);

    // d_end_num == d
    assert(d >= d_start_num);
    assert(d - d_start_num == num_len);
    {
        if( !separator ) [[likely]] {
            d -= val_digits;
            ::memcpy(d, buf_start, val_digits);

            // zero-padding
            const size_t zlen = d - d_start_num;
            d = const_cast<char *>(d_start_num);
            ::memset(d, '0', zlen);
        } else {
            const char * p = buf_end;
            uint32_t sep_count = (num_len - 1) / sep_gap; // final separator count
            uint32_t digit_cnt = 0;
            while (d > d_start_num) [[likely]] {
                if (sep_count && digit_cnt && 0 == digit_cnt % sep_gap) [[unlikely]] {
                    *(--d) = separator;
                    --sep_count;
                    if (d == d_start_num) [[unlikely]] {
                        break; // done
                    }
                }
                if (digit_cnt >= val_digits) [[unlikely]] {
                    *(--d) = '0'; // zero-padding
                } else {
                    *(--d) = *(--p);
                }
                ++digit_cnt;
            }
            assert(p == buf_start);
        }
    }
    assert(d == d_start_num);
    assert(d >= d_start);

    if (negative) [[unlikely]] {
        assert(d > d_start);
        *(--d) = '-';
    } else if (is_set(opts.flags, flags_t::plus)) [[unlikely]] {
        assert(d > d_start);
        *(--d) = '+';  // ignore the space if the '+' exists
    } else if (is_set(opts.flags, flags_t::space)) [[unlikely]] {
        assert(d > d_start);
        *(--d) = ' ';
    }
    assert(d == d_start + space_left);

    ::memset(d - space_left, ' ', space_left);
}

template<jau::req::unsigned_integer T>
CXX_ALWAYS_INLINE
static void append_integralXXRadixXX(std::string &dest, const size_t dest_maxlen, T v, const bool negative, const jau::cfmt::FormatOpts &opts, const bool inject_dot) noexcept {
    using namespace jau::cfmt;

    if (!dest_maxlen) [[unlikely]] {
        return;
    }
    const size_t dest_start_len = dest.size();
    const uint32_t added_maxlen = jau::min(uint32_t(dest_maxlen - dest_start_len), std::numeric_limits<uint32_t>::max());

    const uint32_t radix = opts.radix;
    const uint32_t sep_gap = 4;
    uint32_t val_digits; // includes inject_dot, excludes separator
    char buf_[number_max_strlen];
    const char * const buf_end = buf_ + jau::min(added_maxlen, number_max_strlen);
    const char * buf_start;
    if( opts.precision_set && opts.precision == 0 && jau::is_zero(v) ) [[unlikely]] {
        val_digits = 0;
        buf_start = buf_end;
    } else {
        char *d = const_cast<char *>(buf_end);
        const char *hex_array = is_set(opts.flags, flags_t::uppercase) ? HexadecimalArrayBig : HexadecimalArrayLow;
        if (16 == radix) [[likely]] {
            constexpr uint32_t shift = 4;
            do {
                *(--d) = hex_array[v & 0x0fu];
                v >>= shift;
            } while (v && d > buf_);
        } else {
            const T mask = radix - 1u;
            uint32_t shift;
            switch (radix) {
                // case 16: shift = 4; break;
                // case 10: shift = 0; break;
                case 8:  shift = 3; break;
                case 2:  shift = 1; break;
                default: return;
            }
            do {
                *(--d) = hex_array[v & mask];
                v >>= shift;
            } while (v && d > buf_);
        }
        if (inject_dot && d > buf_ + 1) [[unlikely]] {
            *(d-1) = *d;
            *d = '.';
            --d;
        }
        buf_start = d;
        val_digits = buf_end - buf_start;
        assert(val_digits <= added_maxlen);
    }
    char separator;
    uint32_t num_len; ///< contains zero-padding, val_digits and separator
    if (is_set(opts.flags, flags_t::thousands)) [[unlikely]] {
        separator = Config::default_thousand_separator;
        num_len = jau::min(added_maxlen, val_digits + ( (val_digits - 1) / sep_gap ));
    } else {
        separator = (char)0;
        num_len = val_digits;
    }

    const uint32_t prec = opts.precision_set ? opts.precision : 0;
    uint32_t width = opts.width_set ? jau::min(added_maxlen, opts.width) : 0;
    uint32_t space_left = 0, space_right = 0;
    uint32_t xtra_left = 0;  ///< contains hash, sign, single space
    size_t dest_len;
    {
        const uint32_t num_len0 = num_len; ///< initial num_len digits + sep_count

        // p1: pad leading zeros
        if (!is_set(opts.flags, flags_t::left)) [[likely]] {
            if (is_set(opts.flags, flags_t::zeropad) && width && (negative || has_any(opts.flags, flags_t::plus | flags_t::space))) [[unlikely]] {
                --width;
            }
            if (num_len < prec) [[unlikely]] {
                num_len += prec - num_len;
            }
            if (is_set(opts.flags, flags_t::zeropad) && num_len < width) [[unlikely]] {
                num_len += width - num_len;
            }
        }
        // p1: handle hash
        if (is_set(opts.flags, flags_t::hash)) {
            if (!opts.precision_set && num_len>num_len0 && ((num_len == prec) || (num_len == width))) {
                --num_len;
                if (num_len>num_len0 && radix != 8) [[likely]] { // 2 || 16, b/c only 2, 8, 16 is possible here. 10 has hash-flag cleared
                    --num_len;
                }
            }
            if (radix != 8) [[likely]] { // 2 || 16, b/c only 2, 8, 16 is possible here. 10 has hash-flag cleared
                ++xtra_left;
            }
            ++xtra_left; // hash zero
        }
        num_len = jau::min(added_maxlen, num_len);

        // p1: sign
        if (negative) [[unlikely]] {
            ++xtra_left; // '-';
        } else if (is_set(opts.flags, flags_t::plus)) [[unlikely]] {
            ++xtra_left; // '+';  // ignore the space if the '+' exists
        }

        // p1: space
        if (!negative && is_set(opts.flags, flags_t::space)) [[unlikely]] {
            ++xtra_left; // ' ';
        }
        xtra_left = jau::min(added_maxlen-num_len, xtra_left);

        // p2: append pad spaces left/right up to given width
        {
            const uint32_t len = xtra_left + num_len;
            if (len < width) {
                if (is_set(opts.flags, flags_t::left)) [[unlikely]] {
                    space_right = width - len;
                } else if (!is_set(opts.flags, flags_t::zeropad)) [[likely]] {
                    space_left = width - len;
                }
            }
        }
        space_left = jau::min(added_maxlen-num_len-xtra_left, space_left);
        space_right = jau::min(added_maxlen-num_len-xtra_left-space_left, space_right);
        dest_len = dest_start_len + jau::min(added_maxlen, space_left + xtra_left + num_len + space_right);
        if (!jau::reserve_setsize_string(dest, dest_len)) [[unlikely]] {
            return;
        }
#if !defined(NDEBUG) && 0
        fprintf(stderr, "XXX.80: separator '%c', opts %s\n", separator, opts.toString().c_str());
        fprintf(stderr, "XXX.80: negative %d, val %u\n", (int)negative, v);
        fprintf(stderr, "XXX.80: num_len %u (digits %u + sep %u + .. ), xleft %u, space[l %u, r %u] -> len %u\n",
                num_len, val_digits, sep_count, xtra_left, space_left, space_right, (space_left + xtra_left + num_len + space_right));
        fprintf(stderr, "XXX.80: total len[old %zu, added %zu, len %zu], number_start %u\n", dest_start_len, dest_len-dest_start_len, dest.size(), xtra_left + space_left);
#endif
    }
    const char *const d_start = dest.data() + dest_start_len;
    const char *const d_start_num = d_start + size_t(space_left + xtra_left);
    char *d = dest.data() + dest_len;
    *d = 0; // EOS (is reserved)
    d -= space_right;
    ::memset(d, ' ', space_right);

    const char *const d_end_num = d;
    assert(d_end_num >= d_start_num);
    assert(d_end_num - d_start_num == num_len);
    {
        if( !separator ) [[likely]] {
            d -= val_digits;
            ::memcpy(d, buf_start, val_digits);

            // zero-padding
            const size_t zlen = d - d_start_num;
            d = const_cast<char *>(d_start_num);
            ::memset(d, '0', zlen);
        } else {
            const char * p = buf_end;
            uint32_t sep_count = (num_len - 1) / sep_gap; // final separator count
            uint32_t digit_cnt = 0;
            while (d > d_start_num) [[likely]] {
                if (sep_count && digit_cnt && 0 == digit_cnt % sep_gap) [[unlikely]] {
                    *(--d) = separator;
                    --sep_count;
                    if (d == d_start_num) [[unlikely]] {
                        break; // done
                    }
                }
                if (digit_cnt >= val_digits) [[unlikely]] {
                    *(--d) = '0'; // zero-padding
                } else {
                    *(--d) = *(--p);
                }
                ++digit_cnt;
            }
            assert(p == buf_start);
        }
    }
    assert(d == d_start_num);
    assert(d >= d_start);

    // p1: handle hash
    if (is_set(opts.flags, flags_t::hash) && d > d_start) {
        size_t len = d_end_num - d;  // total length so far
        if (!opts.precision_set && len && ((len == prec) || (len == width))) {
            ++d;
            --len;
            if (len && radix != 8) [[likely]] { // 2 || 16, b/c only 2, 8, 16 is possible here. 10 has hash-flag cleared
                ++d;
                // --len;
            }
        }
        assert(d > d_start);
        if (radix == 16) [[likely]] {
            *(--d) = is_set(opts.flags, flags_t::uppercase) ? 'X' : 'x';
        } else if (radix == 2) {
            *(--d) = 'b';
        }

        assert(d > d_start);
        *(--d) = '0';
    }
    assert(d >= d_start);

    if (negative) [[unlikely]] {
        assert(d > d_start);
        *(--d) = '-';
    } else if (is_set(opts.flags, flags_t::plus)) [[unlikely]] {
        assert(d > d_start);
        *(--d) = '+';  // ignore the space if the '+' exists
    } else if (is_set(opts.flags, flags_t::space)) [[unlikely]] {
        assert(d > d_start);
        *(--d) = ' ';
    }
    assert(d == d_start + space_left);

    ::memset(d - space_left, ' ', space_left);
}

template<jau::req::unsigned_integer T>
CXX_ALWAYS_INLINE
static void append_integralXXSimple(std::string &dest, const size_t dest_maxlen, T v, const bool negative, const jau::cfmt::FormatOpts &opts) noexcept {
    using namespace jau::cfmt;

    if (!dest_maxlen) [[unlikely]] {
        return;
    }
    const size_t dest_start_len = dest.size();
    const uint32_t added_maxlen = jau::min(uint32_t(dest_maxlen - dest_start_len), std::numeric_limits<uint32_t>::max());

    const uint32_t radix = opts.radix;
    const char separator = is_set(opts.flags, flags_t::thousands) ? Config::default_thousand_separator : (char)0;

    // const uint32_t val_digits = opts.precision_set && opts.precision == 0 && jau::is_zero(v) ? 0 : jau::digits(v, radix);
    uint32_t val_digits; // includes separator count
    constexpr uint32_t buf_len = number_max_strlen + number_max_strlen/2; // added separator
    char buf_[buf_len];
    const char * const buf_end = buf_ + jau::min(added_maxlen, buf_len);
    const char * buf_start;
    size_t dest_len;
    {
        char *d = const_cast<char *>(buf_end);
        if (10 == radix) [[likely]] {
            if (!separator) [[likely]] {
                do {
                    *(--d) = char('0' + (v % 10u));
                    v /= 10u;
                } while (v && d > buf_);
            } else {
                constexpr uint32_t sep_gap = 3;
                uint32_t digit_cnt = 0;
                do {
                    if (0 < digit_cnt && 0 == digit_cnt % sep_gap) [[unlikely]] {
                        *(--d) = separator;
                    }
                    assert(d > buf_);

                    *(--d) = char('0' + (v % 10u));
                    v /= 10u;
                    ++digit_cnt;
                } while (v && d > buf_);
            }
        } else {
            const char *hex_array = is_set(opts.flags, flags_t::uppercase) ? HexadecimalArrayBig : HexadecimalArrayLow;
            if (16 == radix) [[likely]] {
                constexpr uint32_t shift = 4;
                if (!separator) [[likely]] {
                    do {
                        *(--d) = hex_array[v & 0x0fu];
                        v >>= shift;
                    } while (v && d > buf_);
                } else {
                    constexpr uint32_t sep_gap = 4;
                    uint32_t digit_cnt = 0;
                    do {
                        if (0 < digit_cnt && 0 == digit_cnt % sep_gap) [[unlikely]] {
                            *(--d) = separator;
                        }
                        assert(d > buf_);

                        *(--d) = hex_array[v & 0x0fu];
                        v >>= shift;
                        ++digit_cnt;
                    } while (v && d > buf_);
                }
            } else {
                const T mask = radix - 1;
                uint32_t shift;
                switch (radix) {
                    // case 16: shift = 4; break;
                    // case 10: shift = 0; break;
                    case 8:  shift = 3; break;
                    case 2:  shift = 1; break;
                    default: return;
                }
                if (!separator) [[likely]] {
                    do {
                        *(--d) = hex_array[v & mask];
                        v >>= shift;
                    } while (v && d > buf_);
                } else {
                    constexpr uint32_t sep_gap = 4;
                    uint32_t digit_cnt = 0;
                    do {
                        if (0 < digit_cnt && 0 == digit_cnt % sep_gap) [[unlikely]] {
                            *(--d) = separator;
                        }
                        assert(d > buf_);

                        *(--d) = hex_array[v & mask];
                        v >>= shift;
                        ++digit_cnt;
                    } while (v && d > buf_);
                }
            }
        }
        buf_start = d;
        val_digits = buf_end - buf_start;
        assert(val_digits <= added_maxlen);
    }
    uint32_t xtra_left = 0;  ///< contains hash, sign and single space
    {
        // p1: handle hash
        if (is_set(opts.flags, flags_t::hash)) [[unlikely]] {
            if (radix != 8) [[likely]] { // 2 || 16, b/c only 2, 8, 16 is possible here. 10 has hash-flag cleared
                ++xtra_left;
            }
            ++xtra_left; // hash zero
        }

        // p1: sign
        if (negative) [[unlikely]] {
            ++xtra_left; // '-';
        } else if (is_set(opts.flags, flags_t::plus)) [[unlikely]] {
            ++xtra_left; // '+';  // ignore the space if the '+' exists
        }

        // p1: space
        if (!negative && is_set(opts.flags, flags_t::space)) [[unlikely]] {
            ++xtra_left; // ' ';
        }
        xtra_left = jau::min(added_maxlen-val_digits, xtra_left);

        dest_len = dest_start_len + jau::min(added_maxlen, xtra_left + val_digits);
        if (!jau::reserve_setsize_string(dest, dest_len)) [[unlikely]] {
            return;
        }

#if !defined(NDEBUG) && 0
        const uint32_t sep_gap = 10 == radix ? 3 : 4;
        const uint32_t sep_count = val_digits > 0 && separator ? (val_digits - 1) / sep_gap : 0;
        fprintf(stderr, "XXX.81: seperator '%c', opts %s\n", separator, opts.toString().c_str());
        fprintf(stderr, "XXX.81: negative %d, val %u\n", (int)negative, v);
        fprintf(stderr, "XXX.81: idx[digits %u, sep %u, xleft %u\n",
                val_digits, sep_count, xtra_left);
        fprintf(stderr, "XXX.81: total len[old %zu, added %zu, len %zu], number_start %u\n", dest_start_len, dest_len-dest_start_len, dest.size(), xtra_left);
#endif
    }
    const char *const d_start = dest.data() + dest_start_len;
    [[maybe_unused]] const char *const d_start_num = d_start + xtra_left;
    char *d = dest.data() + dest_len;
    *d = 0; // EOS (is reserved)

    // d_end_num == d
    assert(d >= d_start_num);
    assert(d - d_start_num == val_digits);
    {
        d -= val_digits;
        ::memcpy(d, buf_start, val_digits);
    }
    assert(d == d_start_num);
    assert(d >= d_start);

    // p1: handle hash
    if (is_set(opts.flags, flags_t::hash) && d > d_start) [[unlikely]] {
        assert(d > d_start);
        if (radix == 16) [[likely]] {
            *(--d) = is_set(opts.flags, flags_t::uppercase) ? 'X' : 'x';
        } else if (radix == 2) {
            *(--d) = 'b';
        }

        assert(d > d_start);
        *(--d) = '0';
    }
    assert(d >= d_start);

    if (negative) [[unlikely]] {
        assert(d > d_start);
        *(--d) = '-';
    } else if (is_set(opts.flags, flags_t::plus)) [[unlikely]] {
        assert(d > d_start);
        *(--d) = '+';  // ignore the space if the '+' exists
    } else if (is_set(opts.flags, flags_t::space)) [[unlikely]] {
        assert(d > d_start);
        *(--d) = ' ';
    }
    assert(d == d_start);  // string space fully written
}

void jau::cfmt::impl::append_integral32Simple(std::string &dest, const size_t dest_maxlen, uint32_t v, const bool negative, const jau::cfmt::FormatOpts &opts) noexcept {
    append_integralXXSimple<uint32_t>(dest, dest_maxlen, v, negative, opts);
}
void jau::cfmt::impl::append_integral32Radix10(std::string &dest, const size_t dest_maxlen, uint32_t v, const bool negative, const jau::cfmt::FormatOpts &opts) noexcept {
    append_integralXXRadix10<uint32_t>(dest, dest_maxlen, v, negative, opts);
}
void jau::cfmt::impl::append_integral32RadixXX(std::string &dest, const size_t dest_maxlen, uint32_t v, const bool negative, const jau::cfmt::FormatOpts &opts, const bool inject_dot) noexcept {
    append_integralXXRadixXX<uint32_t>(dest, dest_maxlen, v, negative, opts, inject_dot);
}

void jau::cfmt::impl::append_integral64Simple(std::string &dest, const size_t dest_maxlen, uint64_t v, const bool negative, const jau::cfmt::FormatOpts &opts) noexcept {
    append_integralXXSimple<uint64_t>(dest, dest_maxlen, v, negative, opts);
}
void jau::cfmt::impl::append_integral64Radix10(std::string &dest, const size_t dest_maxlen, uint64_t v, const bool negative, const jau::cfmt::FormatOpts &opts) noexcept {
    append_integralXXRadix10<uint64_t>(dest, dest_maxlen, v, negative, opts);
}
void jau::cfmt::impl::append_integral64RadixXX(std::string &dest, const size_t dest_maxlen, uint64_t v, const bool negative, const jau::cfmt::FormatOpts &opts, const bool inject_dot) noexcept {
    append_integralXXRadixXX<uint64_t>(dest, dest_maxlen, v, negative, opts, inject_dot);
}

bool jau::cfmt::impl::is_float_validF64(std::string &dest, const size_t dest_maxlen, const double value, const FormatOpts &opts) noexcept {
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


void jau::cfmt::impl::append_floatF64(std::string &dest, const size_t dest_maxlen, const double ivalue, const FormatOpts &opts) noexcept {
    using namespace jau::float_literals;

    if (!dest_maxlen) {
        return;
    }
    if (!is_float_validF64(dest, dest_maxlen, ivalue, opts)) {
        return;
    }
    // FIXME `long double`
    typedef double float_type;  // enforce 64bit only double type (see below)
    // typedef jau::float_bytes_t<std::max(sizeof(ifloat_type), sizeof(double))> float_type;

    char buf_[number_max_strlen];
    char *d = buf_;
    const char *const d_start = d;
    const char *const d_end = d + number_max_strlen;
    float_type diff = 0;

    // powers of 10
    static constexpr const float_type pow10[] = { 1e0_f64, 1e1_f64, 1e2_f64, 1e3_f64, 1e4_f64, 1e5_f64, 1e6_f64, 1e7_f64, 1e8_f64, 1e9_f64,
                                                  1e10_f64, 1e11_f64, 1e12_f64, 1e13_f64, 1e14_f64 };
    static constexpr const size_t prec_max = sizeof(pow10) / sizeof(float_type) - 1;  // 14

    // test for negative
    const bool negative = ivalue < 0;
    float_type value = float_type(negative ? -ivalue : ivalue);

    // test for very large values
    // standard printf behavior is to print EVERY whole number digit -- which could be 100s of characters overflowing your buffers == bad
    if ((value > max_append_float) || (value < -max_append_float)) {
        append_efloatF64(dest, dest_maxlen, ivalue, opts);
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
    float_type tmp = (value - float_type(whole)) * pow10[prec];
    uint64_t frac = (uint64_t)tmp;
    diff = tmp - (float_type)frac;

    if (diff > 0.5) {
        ++frac;
        // handle rollover, e.g. case 0.99 with prec 1 is 1.0
        if (float_type(frac) >= pow10[prec]) {
            frac = 0;
            ++whole;
        }
    } else if (diff < 0.5) {
    } else if ((frac == 0) || (frac & 1)) {
        // if halfway, round up if odd OR if last digit is 0
        ++frac;
    }

#if !defined(NDEBUG) && 0
    fprintf(stderr, "FFF.10: val %f, positive %d, len %zu/%zu, prec %zu/%zu, width %zu: whole %u, frac %u, double_t %s\n",
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
        if ((negative || has_any(opts.flags, flags_t::plus | flags_t::space)) && width) {
            width--;
        }
        while ((d < d_start + width) && d < d_end) {
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

    append_rev(dest, dest_maxlen, std::string_view(d_start, d - d_start), false /*prec*/, true /*rev**/, opts);
}

void jau::cfmt::impl::append_efloatF64(std::string &dest, const size_t dest_maxlen, const double ivalue, const FormatOpts &iopts) noexcept {
    using namespace jau::float_literals;

    if (!dest_maxlen) {
        return;
    }
    if (!is_float_validF64(dest, dest_maxlen, ivalue, iopts)) {
        return;
    }
    // FIXME `long double`
    typedef double float_type;  // enforce 64bit only double type (see below)

    // determine the sign
    const bool negative = ivalue < 0;
    float_type value = float_type(negative ? -ivalue : ivalue);

    // default precision
    uint32_t prec = iopts.precision_set ? iopts.precision : default_float_precision;

    // determine the decimal exponent
    // based on the algorithm by David Gay (https://www.ampl.com/netlib/fp/dtoa.c)
    // NOTE: 64bit double type specific
    union {
        uint64_t U;
        double F;
    } conv;

    conv.F = value;
    int32_t expval;
    {
        int32_t exp2 = int32_t((conv.U >> 52U) & 0x07FFU) - 1023;       // effectively log2
        conv.U = (conv.U & ((1_u64 << 52U) - 1U)) | (1023_u64 << 52U);  // drop the exponent so conv.F is now in [1,2)
        // now approximate log10 from the log2 integer part and an expansion of ln around 1.5
        expval = int32_t(0.1760912590558 + exp2 * 0.301029995663981 + (conv.F - 1.5) * 0.289529654602168);
        // now we want to compute 10^expval but we want to be sure it won't overflow
        exp2 = int32_t(expval * 3.321928094887362 + 0.5);  // NOLINT(bugprone-incorrect-roundings)
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
            if (int32_t(prec) > expval) { // NOLINT(modernize-use-integer-sign-comparison): false positive
                prec = uint32_t(int32_t(prec) - expval - 1);
            } else {
                prec = 0;
            }
            fopts.precision_set = true;  // make sure _ftoa respects precision
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
        if (iopts.precision_set) {
            fopts.precision_set = true;
        }
        if (fopts.precision_set) {
            fopts.precision = prec;
        } else {
            fopts.precision = 0;
        }
        fopts.width_set = true;
        fopts.width = fwidth;
        append_floatF64(dest, dest_maxlen, negative ? -value : value, fopts);
    }

    // output the exponent part
    if (minwidth) {
        // output the exponential symbol
        {
            const size_t idx = dest.size();
            if (idx + 1 > dest_maxlen ||
                !jau::reserve_append_string(dest, jau::min(idx + 1 + number_max_strlen, dest_maxlen) + 1, 1) // cap +EOS, not shrinking!
               )
            {
                return;
            }
            *(dest.data() + idx) = is_set(iopts.flags, flags_t::uppercase) ? 'E' : 'e';
        }
        // output the exponent value
        fopts.conversion = cspec_t::unsigned_int;
        fopts.radix = 10;
        fopts.flags = flags_t::zeropad | flags_t::plus;
        fopts.precision_set = false;
        fopts.precision = 0;
        fopts.width_set = true;
        fopts.width = minwidth - 1;
#if !defined(NDEBUG) && 0
        fprintf(stderr, "EEE.31: v %f (exp %d), dest '%s' (len %zu), fopts %s\n",
                value, expval, dest.c_str(), dest.size(), fopts.toString().c_str());
#endif
        append_integral64Radix10(dest, dest_maxlen, uint64_t(jau::abs(expval)), expval < 0, fopts);

        // might need to right-pad spaces
        if (is_set(iopts.flags, flags_t::left)) {
            const size_t idx = dest.size();
            if (idx - start_idx < width) {
                // const size_t with_space_right = idx + ( width - (idx - start_idx) );
                const size_t with_space_right = width + start_idx;
                if (with_space_right > dest_maxlen ||
                    !jau::reserve_append_string(dest, with_space_right + 1, with_space_right - idx) // cap +EOS, not shrinking!
                   )
                {
                    return;
                }
            }
        }
    }
#if !defined(NDEBUG) && 0
    fprintf(stderr, "EEE.88: expval %d, dest '%s' (len %zu, cap %zu)\n", expval, dest.c_str(), dest.size(), dest.capacity());
#endif
}

void jau::cfmt::impl::append_afloatF64(std::string &dest, const size_t dest_maxlen, const double ivalue, const size_t ivalue_size, const FormatOpts &iopts) noexcept {
    using namespace jau::float_literals;

    if (!dest_maxlen) {
        return;
    }
    if (!is_float_validF64(dest, dest_maxlen, ivalue, iopts)) {
        return;
    }
    typedef double float_type;  // enforce 64bit only double type (spec)
    const unsigned significand_shift = sizeof(float_type) > ivalue_size ? 8 * (sizeof(float_type) - ivalue_size) - 4 : 0;

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
    fprintf(stderr, "AAA.10: v %f, frac %x, expval %d, dest '%s' (len %zu), iopts %s\n",
            ivalue, significand, expval, dest.c_str(), dest.size(), iopts.toString().c_str());
#endif

    const size_t start_idx = dest.size();
    // output the floating part
    {
        fopts.conversion = cspec_t::signed_int;
        fopts.radix = 16;
        fopts.flags = iopts.flags | flags_t::hash;

        if (iopts.precision_set) {
            fopts.precision_set = true;
        }
        if (fopts.precision_set) {
            fopts.precision = prec;
        } else {
            fopts.precision = 0;
        }
        fopts.width_set = true;
        fopts.width = fwidth;
#if !defined(NDEBUG) && 0
        fprintf(stderr, "AAA.31: v %f, frac %x, expval %d, dest '%s' (len %zu), fopts %s\n",
                ivalue, significand, expval, dest.c_str(), dest.size(), fopts.toString().c_str());
#endif
        append_integral64RadixXX(dest, dest_maxlen, significand, false, fopts, true);
    }

    // output the exponent part
    if (minwidth) {
        // output the exponential symbol
        {
            const size_t idx = dest.size();
            if (idx + 1 > dest_maxlen ||
                !jau::reserve_append_string(dest, jau::min(idx + 1 + number_max_strlen, dest_maxlen) + 1, 1)  // cap +EOS, not shrinking!
               )
            {
                return;
            }
            *(dest.data() + idx) = is_set(iopts.flags, flags_t::uppercase) ? 'P' : 'p';
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
        fprintf(stderr, "AAA.32: v %f, frac %x, expval %d, dest '%s' (len %zu), fopts %s\n",
                ivalue, significand, expval, dest.c_str(), dest.size(), fopts.toString().c_str());
#endif
        append_integral64Radix10(dest, dest_maxlen, uint64_t(jau::abs(expval)), expval < 0, fopts);

        // might need to right-pad spaces
        if (is_set(iopts.flags, flags_t::left)) {
            const size_t idx = dest.size();
            if (idx - start_idx < width) {
                // const size_t with_space_right = idx + ( width - (idx - start_idx) );
                const size_t with_space_right = width + start_idx;
                if (with_space_right > dest_maxlen ||
                    !jau::reserve_append_string(dest, with_space_right + 1, with_space_right - idx) // cap +EOS, not shrinking!
                   )
                {
                    return;
                }
            }
        }
    }
#if !defined(NDEBUG) && 0
    fprintf(stderr, "AAA.88: expval %d, dest '%s' (len %zu, cap %zu)\n", expval, dest.c_str(), dest.size(), dest.capacity());
#endif
}

void jau::cfmt::impl::StringOutput::appendError(size_t argIdx, int line, const std::string_view tag) noexcept {
    jau::append_string(m_s, "<E#");
    jau::appendDecString(m_s, argIdx, 0);
    jau::append_string(m_s, "@");
    jau::appendDecString(m_s, line, 0);
    jau::append_string(m_s, ":");
    jau::append_string(m_s, tag);
    jau::append_string(m_s, ">");
}

// Explicit instantiation definition of template function
template class jau::cfmt::impl::Parser<jau::cfmt::impl::StringOutput>;
template void jau::cfmt::impl::Parser<jau::cfmt::impl::StringOutput>::parseOneImpl<bool>(jau::cfmt::impl::FResult<jau::cfmt::impl::StringOutput>&, bool const&);
template void jau::cfmt::impl::Parser<jau::cfmt::impl::StringOutput>::parseOneImpl<jau::cfmt::unsigned_integral_promotion64>(jau::cfmt::impl::FResult<jau::cfmt::impl::StringOutput>&, jau::cfmt::unsigned_integral_promotion64 const&);
template void jau::cfmt::impl::Parser<jau::cfmt::impl::StringOutput>::parseOneImpl<jau::cfmt::unsigned_integral_promotion32>(jau::cfmt::impl::FResult<jau::cfmt::impl::StringOutput>&, jau::cfmt::unsigned_integral_promotion32 const&);
template void jau::cfmt::impl::Parser<jau::cfmt::impl::StringOutput>::parseOneImpl<double>(jau::cfmt::impl::FResult<jau::cfmt::impl::StringOutput>&, double const&);
template void jau::cfmt::impl::Parser<jau::cfmt::impl::StringOutput>::parseOneImpl<const void * const>(jau::cfmt::impl::FResult<jau::cfmt::impl::StringOutput>&, void const* const&);
template void jau::cfmt::impl::Parser<jau::cfmt::impl::StringOutput>::parseOneImpl<const char * const>(jau::cfmt::impl::FResult<jau::cfmt::impl::StringOutput>&, char const* const&);
template void jau::cfmt::impl::Parser<jau::cfmt::impl::StringOutput>::parseOneImpl<std::string_view>(jau::cfmt::impl::FResult<jau::cfmt::impl::StringOutput>&, std::string_view const&);
template void jau::cfmt::impl::Parser<jau::cfmt::impl::StringOutput>::parseOneImpl<jau::cfmt::impl::no_type_t>(jau::cfmt::impl::FResult<jau::cfmt::impl::StringOutput>&, jau::cfmt::impl::no_type_t const&);
template void jau::cfmt::impl::Parser<jau::cfmt::impl::StringOutput>::parseOne<jau::cfmt::impl::no_type_t>(jau::cfmt::impl::FResult<jau::cfmt::impl::StringOutput>&, jau::cfmt::impl::no_type_t const&);
template void jau::cfmt::impl::Parser<jau::cfmt::impl::StringOutput>::parseOne<bool>(jau::cfmt::impl::FResult<jau::cfmt::impl::StringOutput>&, bool const&);
template void jau::cfmt::impl::Parser<jau::cfmt::impl::StringOutput>::parseOne<char>(jau::cfmt::impl::FResult<jau::cfmt::impl::StringOutput>&, char const&);
template void jau::cfmt::impl::Parser<jau::cfmt::impl::StringOutput>::parseOne<short>(jau::cfmt::impl::FResult<jau::cfmt::impl::StringOutput>&, short const&);
template void jau::cfmt::impl::Parser<jau::cfmt::impl::StringOutput>::parseOne<int>(jau::cfmt::impl::FResult<jau::cfmt::impl::StringOutput>&, int const&);
template void jau::cfmt::impl::Parser<jau::cfmt::impl::StringOutput>::parseOne<long>(jau::cfmt::impl::FResult<jau::cfmt::impl::StringOutput>&, long const&);
template void jau::cfmt::impl::Parser<jau::cfmt::impl::StringOutput>::parseOne<unsigned char>(jau::cfmt::impl::FResult<jau::cfmt::impl::StringOutput>&, unsigned char const&);
template void jau::cfmt::impl::Parser<jau::cfmt::impl::StringOutput>::parseOne<unsigned short>(jau::cfmt::impl::FResult<jau::cfmt::impl::StringOutput>&, unsigned short const&);
template void jau::cfmt::impl::Parser<jau::cfmt::impl::StringOutput>::parseOne<unsigned int>(jau::cfmt::impl::FResult<jau::cfmt::impl::StringOutput>&, unsigned int const&);
template void jau::cfmt::impl::Parser<jau::cfmt::impl::StringOutput>::parseOne<unsigned long>(jau::cfmt::impl::FResult<jau::cfmt::impl::StringOutput>&, unsigned long const&);
template void jau::cfmt::impl::Parser<jau::cfmt::impl::StringOutput>::parseOne<double>(jau::cfmt::impl::FResult<jau::cfmt::impl::StringOutput>&, double const&);
template void jau::cfmt::impl::Parser<jau::cfmt::impl::StringOutput>::parseOne<float>(jau::cfmt::impl::FResult<jau::cfmt::impl::StringOutput>&, float const&);
template void jau::cfmt::impl::Parser<jau::cfmt::impl::StringOutput>::parseOne<char const*>(jau::cfmt::impl::FResult<jau::cfmt::impl::StringOutput>&, char const* const&);
template void jau::cfmt::impl::Parser<jau::cfmt::impl::StringOutput>::parseOne<char*>(jau::cfmt::impl::FResult<jau::cfmt::impl::StringOutput>&, char* const&);
template void jau::cfmt::impl::Parser<jau::cfmt::impl::StringOutput>::parseOne<std::string_view>(jau::cfmt::impl::FResult<jau::cfmt::impl::StringOutput>&, std::string_view const&);
template class jau::cfmt::impl::FResult<jau::cfmt::impl::StringOutput>;
