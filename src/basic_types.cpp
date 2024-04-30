/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020-2024 Gothel Software e.K.
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

#include <cstdint>
#include <cinttypes>
#include <cstring>

#include <ctime>

#include <algorithm>

#include <jau/debug.hpp>
#include <jau/basic_types.hpp>
#include <jau/functional.hpp>
#include <jau/math/math_error.hpp>

using namespace jau;

static constexpr const uint64_t NanoPerMilli =  1000'000UL;
static constexpr const uint64_t MilliPerOne  =     1'000UL;

/**
 * See <http://man7.org/linux/man-pages/man2/clock_gettime.2.html>
 * <p>
 * Regarding avoiding kernel via VDSO,
 * see <http://man7.org/linux/man-pages/man7/vdso.7.html>,
 * clock_gettime seems to be well supported at least on kernel >= 4.4.
 * Only bfin and sh are missing, while ia64 seems to be complicated.
 */
fraction_timespec jau::getMonotonicTime() noexcept {
    struct timespec t;
    ::clock_gettime(CLOCK_MONOTONIC, &t);
    return fraction_timespec( (int64_t)t.tv_sec, (int64_t)t.tv_nsec );
}

fraction_timespec jau::getWallClockTime() noexcept {
    struct timespec t { 0, 0 };
    ::clock_gettime(CLOCK_REALTIME, &t);
    return fraction_timespec( (int64_t)t.tv_sec, (int64_t)t.tv_nsec );
}

uint64_t jau::getCurrentMilliseconds() noexcept {
    struct timespec t { 0, 0 };
    ::clock_gettime(CLOCK_MONOTONIC, &t);
    return static_cast<uint64_t>( t.tv_sec ) * MilliPerOne +
           static_cast<uint64_t>( t.tv_nsec ) / NanoPerMilli;
}

uint64_t jau::getWallClockSeconds() noexcept {
    struct timespec t { 0, 0 };
    ::clock_gettime(CLOCK_REALTIME, &t);
    return static_cast<uint64_t>( t.tv_sec );
}

std::string fraction_timespec::to_string() const noexcept {
    return std::to_string(tv_sec) + "s + " + std::to_string(tv_nsec) + "ns";
}

std::string fraction_timespec::to_iso8601_string() const noexcept {
    std::time_t t0 = static_cast<std::time_t>(tv_sec);
    struct std::tm tm_0;
    if( nullptr == ::gmtime_r( &t0, &tm_0 ) ) {
        return "1970-01-01T00:00:00.0Z"; // 22 + 1
    } else {
        // 2022-05-28T23:23:50Z 20+1
        //
        // 1655994850s + 228978909ns
        // 2022-06-23T14:34:10.228978909Z 30+1
        char b[30+1];
        size_t p = ::strftime(b, sizeof(b), "%Y-%m-%dT%H:%M:%S", &tm_0);
        if( 0 < p && p < sizeof(b)-1 ) {
            const size_t remaining = sizeof(b) - p;
            if( 0 < tv_nsec ) {
                ::snprintf(b+p, remaining, ".%09" PRIi64 "Z", tv_nsec);
            } else {
                ::snprintf(b+p, remaining, "Z");
            }
        }
        return std::string(b);
    }
}

void jau::sleep_until(const fraction_timespec& absolute_time, const bool monotonic) noexcept {
    if( absolute_time <= fraction_tv::zero ) {
        return;
    }
    // typedef struct timespec __gthread_time_t;
    struct timespec ts = absolute_time.to_timespec();

    while ( -1 == ::clock_nanosleep(monotonic ? CLOCK_MONOTONIC : CLOCK_REALTIME,
                                    TIMER_ABSTIME,
                                    &ts, &ts) && EINTR == errno ) { }
}

void jau::sleep_for(const fraction_timespec& relative_time, const bool monotonic) noexcept {
    if( relative_time <= fraction_tv::zero ) {
        return;
    }
    const fraction_timespec now = monotonic ? getMonotonicTime() : getWallClockTime();
    sleep_until( now + relative_time );
}

void jau::sleep_for(const fraction_i64& relative_time, const bool monotonic) noexcept {
    if( relative_time <= fractions_i64::zero ) {
        return;
    }
    bool overflow = false;
    const fraction_timespec atime = ( monotonic ? getMonotonicTime() : getWallClockTime() ) + fraction_timespec(relative_time, &overflow);
    if( overflow ) {
        return;
    } else {
        sleep_until( atime );
    }
}

#if defined(__linux__) && defined(__GLIBC__)
    // Hack for glibc/pthread library w/o pthread_cond_clockwait,
    // i.e. on g++8, arm32, Debian 10.
    // Here we have to use pthread_cond_timedwait(), ignoring the clock type.
    //
    // __attribute__((weak)) tested w/ g++8.3, g++10 and clang-11
    //
    typedef __clockid_t os_clockid_t;
    extern int pthread_cond_clockwait (pthread_cond_t *__restrict __cond,
                       pthread_mutex_t *__restrict __mutex,
                       os_clockid_t __clock_id,
                       const struct timespec *__restrict __abstime)
         __nonnull ((1, 2, 4)) __attribute__((weak));

    static bool __jau__has_pthread_cond_clockwait() noexcept {
        const bool r = nullptr != pthread_cond_clockwait;
        ::fprintf(stderr, "INFO: jau::has_pthread_cond_clockwait: %d\n", r);
        return r;
    }
    static bool jau_has_pthread_cond_clockwait() noexcept {
        static bool r = __jau__has_pthread_cond_clockwait();
        return r;
    }
#else
    typedef int32_t os_clockid_t;
    static int pthread_cond_clockwait (pthread_cond_t * __cond,
                       pthread_mutex_t * __mutex,
                       os_clockid_t __clock_id,
                       const struct timespec * __abstime) {
        (void)__cond;
        (void)__mutex;
        (void)__clock_id;
        (void)__abstime;
        return -1;
    }

    static bool jau_has_pthread_cond_clockwait() noexcept {
        return false;
    }
#endif

std::cv_status jau::wait_until(std::condition_variable& cv, std::unique_lock<std::mutex>& lock, const fraction_timespec& absolute_time, const bool monotonic) noexcept {
    if( absolute_time <= fraction_tv::zero ) {
        return std::cv_status::no_timeout;
    }
    // typedef struct timespec __gthread_time_t;
    struct timespec ts = absolute_time.to_timespec();

    if( jau_has_pthread_cond_clockwait() ) {
        pthread_cond_clockwait(cv.native_handle(), lock.mutex()->native_handle(),
                               monotonic ? CLOCK_MONOTONIC : CLOCK_REALTIME, &ts);
    } else {
        ::pthread_cond_timedwait(cv.native_handle(), lock.mutex()->native_handle(), &ts);
    }

    const fraction_timespec now = monotonic ? getMonotonicTime() : getWallClockTime();
    return now < absolute_time ? std::cv_status::no_timeout : std::cv_status::timeout;
}

std::cv_status jau::wait_for(std::condition_variable& cv, std::unique_lock<std::mutex>& lock, const fraction_timespec& relative_time, const bool monotonic) noexcept {
    if( relative_time <= fraction_tv::zero ) {
        return std::cv_status::no_timeout;
    }
    const fraction_timespec now = monotonic ? getMonotonicTime() : getWallClockTime();
    return wait_until(cv, lock, now + relative_time, monotonic);
}

std::cv_status jau::wait_for(std::condition_variable& cv, std::unique_lock<std::mutex>& lock, const fraction_i64& relative_time, const bool monotonic) noexcept {
    if( relative_time <= fractions_i64::zero ) {
        return std::cv_status::no_timeout;
    }
    bool overflow = false;
    const fraction_timespec atime = ( monotonic ? getMonotonicTime() : getWallClockTime() ) + fraction_timespec(relative_time, &overflow);
    if( overflow ) {
        return std::cv_status::timeout;
    } else {
        return wait_until(cv, lock, atime, monotonic);
    }
}


jau::ExceptionBase::ExceptionBase(std::string type, std::string const& m, const char* file, int line) noexcept
: msg_( std::move( type.append(" @ ").append(file).append(":").append(std::to_string(line)).append(": ").append(m) ) ),
  backtrace_( jau::get_backtrace(true /* skip_anon_frames */) )
{
    what_ = msg_;
    what_.append("\nNative backtrace:\n");
    what_.append(backtrace_);
}

std::string jau::get_string(const uint8_t *buffer, nsize_t const buffer_len, nsize_t const max_len) noexcept {
    const nsize_t cstr_max_len = std::min(buffer_len, max_len);
    const size_t cstr_len = ::strnlen(reinterpret_cast<const char*>(buffer), cstr_max_len); // if cstr_len == cstr_max_len then no EOS
    return std::string(reinterpret_cast<const char*>(buffer), cstr_len);
}

void jau::trimInPlace(std::string &s) noexcept {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch);
    }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

std::string jau::trim(const std::string &_s) noexcept {
    std::string s(_s);
    trimInPlace(s);
    return s;
}

std::vector<std::string> jau::split_string(const std::string& str, const std::string& separator) noexcept {
    std::vector<std::string> res;
    size_t p0 = 0;
    while( p0 != std::string::npos && p0 < str.size() ) {
        size_t p1 = str.find(separator, p0);
        res.push_back(str.substr(p0, p1)); // incl. npos
        if( p1 != std::string::npos ) {
            p1 += separator.length();
        }
        p0 = p1;
    }
    return res;
}

std::string& jau::toLowerInPlace(std::string& s) noexcept {
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c){ return std::tolower(c); });
    return s;
}
std::string jau::toLower(const std::string& s) noexcept {
    std::string t(s); toLowerInPlace(t); return t;
}

// one static_assert is sufficient for whole compilation unit
static_assert( is_defined_endian(endian::native) );
static_assert( is_little_or_big_endian() );

uint128dp_t jau::merge_uint128(uint16_t const uuid16, uint128dp_t const & base_uuid, nsize_t const uuid16_le_octet_index)
{
    if( uuid16_le_octet_index > 14 ) {
        std::string msg("uuid16_le_octet_index ");
        msg.append(std::to_string(uuid16_le_octet_index));
        msg.append(", not within [0..14]");
        throw IllegalArgumentException(msg, E_FILE_LINE);
    }
    uint128dp_t dest = base_uuid;

    // base_uuid: 00000000-0000-1000-8000-00805F9B34FB
    //    uuid16: DCBA
    // uuid16_le_octet_index: 12
    //    result: 0000DCBA-0000-1000-8000-00805F9B34FB
    //
    // LE: low-mem - FB349B5F8000-0080-0010-0000-ABCD0000 - high-mem
    //                                           ^ index 12
    // LE: uuid16 -> value.data[12+13]
    //
    // BE: low-mem - 0000DCBA-0000-1000-8000-00805F9B34FB - high-mem
    //                   ^ index 2
    // BE: uuid16 -> value.data[2+3]
    //
    nsize_t offset;
    if( is_big_endian() ) {
        offset = 15 - 1 - uuid16_le_octet_index;
    } else {
        offset = uuid16_le_octet_index;
    }
    // uint16_t * destu16 = (uint16_t*)(dest.data + offset);
    // *destu16 += uuid16;
    reinterpret_cast<packed_t<uint16_t>*>( dest.data + offset )->store += uuid16;
    return dest;
}

uint128dp_t jau::merge_uint128(uint32_t const uuid32, uint128dp_t const & base_uuid, nsize_t const uuid32_le_octet_index)
{
    if( uuid32_le_octet_index > 12 ) {
        std::string msg("uuid32_le_octet_index ");
        msg.append(std::to_string(uuid32_le_octet_index));
        msg.append(", not within [0..12]");
        throw IllegalArgumentException(msg, E_FILE_LINE);
    }
    uint128dp_t dest = base_uuid;

    // base_uuid: 00000000-0000-1000-8000-00805F9B34FB
    //    uuid32: 87654321
    // uuid32_le_octet_index: 12
    //    result: 87654321-0000-1000-8000-00805F9B34FB
    //
    // LE: low-mem - FB349B5F8000-0080-0010-0000-12345678 - high-mem
    //                                           ^ index 12
    // LE: uuid32 -> value.data[12..15]
    //
    // BE: low-mem - 87654321-0000-1000-8000-00805F9B34FB - high-mem
    //               ^ index 0
    // BE: uuid32 -> value.data[0..3]
    //
    nsize_t offset;;
    if( is_big_endian() ) {
        offset = 15 - 3 - uuid32_le_octet_index;
    } else {
        offset = uuid32_le_octet_index;
    }
    // uint32_t * destu32 = (uint32_t*)(dest.data + offset);
    // *destu32 += uuid32;
    reinterpret_cast<packed_t<uint32_t>*>( dest.data + offset )->store += uuid32;
    return dest;
}

std::string jau::vformat_string(const char* format, va_list ap) noexcept {
    size_t nchars;
    std::string str;
    {
        const size_t bsz = 1024; // including EOS
        str.reserve(bsz);  // incl. EOS
        str.resize(bsz-1); // excl. EOS

        nchars = vsnprintf(&str[0], bsz, format, ap);
        if( nchars < bsz ) {
            str.resize(nchars);
            str.shrink_to_fit();
            return str;
        }
    }
    {
        const size_t bsz = std::min<size_t>(nchars+1, str.max_size()+1); // limit incl. EOS
        str.reserve(bsz);  // incl. EOS
        str.resize(bsz-1); // excl. EOS
        nchars = vsnprintf(&str[0], bsz, format, ap);
        str.resize(nchars);
        return str;
    }
}

std::string jau::format_string(const char* format, ...) noexcept {
    va_list args;
    va_start (args, format);
    std::string str = vformat_string(format, args);
    va_end (args);
    return str;
}

static snsize_t hexCharByte_(const uint8_t c)
{
  if('0' <= c && c <= '9') {
      return c - '0';
  }
  if('A' <= c && c <= 'F') {
      return c - 'A' + 10;
  }
  if('a' <= c && c <= 'f') {
      return c - 'a' + 10;
  }
  return -1;
}

size_t jau::hexStringBytes(std::vector<uint8_t>& out, const std::string& hexstr, const bool lsbFirst, const bool checkLeading0x) noexcept {
    return jau::hexStringBytes(out, cast_char_ptr_to_uint8(hexstr.data()), hexstr.size(), lsbFirst, checkLeading0x);
}
size_t jau::hexStringBytes(std::vector<uint8_t>& out, const uint8_t hexstr[], const size_t hexstr_len, const bool lsbFirst, const bool checkLeading0x) noexcept {
    size_t offset;
    if( checkLeading0x && hexstr_len >= 2 && hexstr[0] == '0' && hexstr[1] == 'x' ) {
        offset = 2;
    } else {
        offset = 0;
    }
    size_t lsb, msb;
    if( lsbFirst ) {
        lsb = 1; msb = 0;
    } else {
        lsb = 0; msb = 1;
    }
    const size_t hexlen_in = hexstr_len - offset;
    const size_t bsize = hexlen_in / 2;
    out.clear();
    out.reserve(bsize);

    size_t i = 0;
    if( 0 < hexlen_in % 2 ) {
        // no leading '0', digest a single digit
        const size_t idx = ( lsb*i + msb*(bsize-1-i) ) * 2;
        const snsize_t l = hexCharByte_( hexstr[ offset + idx + 1 ] );
        if( 0 <= l ) {
            out.push_back( static_cast<uint8_t>( l ) );
        } else {
            // invalid char
            return out.size();
        }
        ++i;
    }
    for (; i < bsize; ++i) {
        const size_t idx = ( lsb*i + msb*(bsize-1-i) ) * 2;
        const snsize_t h = hexCharByte_( hexstr[ offset + idx ] );
        const snsize_t l = hexCharByte_( hexstr[ offset + idx + 1 ] );
        if( 0 <= h && 0 <= l ) {
            out.push_back( static_cast<uint8_t>( (h << 4) + l ) );
        } else {
            // invalid char
            return out.size();
        }
    }
    return out.size();
}


static const char* HEX_ARRAY_LOW = "0123456789abcdef";
static const char* HEX_ARRAY_BIG = "0123456789ABCDEF";

std::string jau::bytesHexString(const void* data, const nsize_t offset, const nsize_t length,
                                const bool lsbFirst, const bool lowerCase) noexcept
{
    const char* hex_array = lowerCase ? HEX_ARRAY_LOW : HEX_ARRAY_BIG;
    std::string str;

    if( nullptr == data ) {
        return "null";
    }
    if( 0 == length ) {
        return "nil";
    }
    const uint8_t * const bytes = static_cast<const uint8_t*>(data);
    if( lsbFirst ) {
        // LSB left -> MSB right, no leading `0x`
        str.reserve(length * 2 +1);
        for (nsize_t j = 0; j < length; j++) {
            const int v = bytes[offset+j] & 0xFF;
            str.push_back(hex_array[v >> 4]);
            str.push_back(hex_array[v & 0x0F]);
        }
    } else {
        // MSB left -> LSB right, with leading `0x`
        str.reserve(2 + length * 2 +1);
        str.push_back('0');
        str.push_back('x');
        nsize_t j = length;
        do {
            j--;
            const int v = bytes[offset+j] & 0xFF;
            str.push_back(hex_array[v >> 4]);
            str.push_back(hex_array[v & 0x0F]);
        } while( j != 0);
    }
    return str;
}

std::string& jau::byteHexString(std::string& dest, const uint8_t value, const bool lowerCase) noexcept
{
    const char* hex_array = lowerCase ? HEX_ARRAY_LOW : HEX_ARRAY_BIG;

    if( 2 > dest.capacity() - dest.size() ) { // Until C++20, then reserve is ignored if capacity > reserve
        dest.reserve(dest.size()+2);
    }
    const int v = value & 0xFF;
    dest.push_back(hex_array[v >> 4]);
    dest.push_back(hex_array[v & 0x0F]);
    return dest;
}

std::string jau::to_string(const endian v) noexcept {
    switch(v) {
        case endian::little:  return "little";
        case endian::big:  return "big";
        case endian::pdp:  return "pdb";
        case endian::honeywell: return "honeywell";
        case endian::undefined: return "undefined";
    }
    return "undef";
}

std::string jau::to_string(const lb_endian v) noexcept {
    switch(v) {
        case lb_endian::little:  return "little";
        case lb_endian::big:  return "big";
    }
    return "undef";
}

std::string jau::to_string(const jau::func::target_type v) noexcept {
    switch(v) {
        case jau::func::target_type::null:  return "null";
        case jau::func::target_type::member:  return "member";
        case jau::func::target_type::free:  return "free";
        case jau::func::target_type::lambda: return "lambda";
        case jau::func::target_type::ylambda: return "ylambda";
        case jau::func::target_type::capval: return "capval";
        case jau::func::target_type::capref: return "capref";
        case jau::func::target_type::std: return "std";
    }
    return "undef";
}

bool jau::to_integer(long long & result, const char * str, size_t str_len, const char limiter, const char *limiter_pos) {
    static constexpr const bool _debug = false;
    char *endptr = nullptr;
    if( nullptr == limiter_pos ) {
        limiter_pos = str + str_len;
    }
    errno = 0;
    const long long num = std::strtoll(str, &endptr, 10);
    if( 0 != errno ) {
        // value under- or overflow occured
        if constexpr ( _debug ) {
            INFO_PRINT("Value under- or overflow occurred, value %lld in: '%s', errno %d %s", num, str, errno, strerror(errno));
        }
        return false;
    }
    if( nullptr == endptr || endptr == str ) {
        // no digits consumed
        if constexpr ( _debug ) {
            INFO_PRINT("Value no digits consumed @ idx %d, %p == start, in: '%s'", endptr-str, endptr, str);
        }
        return false;
    }
    if( endptr < limiter_pos ) {
        while( endptr < limiter_pos && ::isspace(*endptr) ) { // only accept whitespace
            ++endptr;
        }
    }
    if( *endptr != limiter || endptr != limiter_pos ) {
        // numerator value not completely valid
        if constexpr ( _debug ) {
            INFO_PRINT("Value end not '%c' @ idx %d, %p != %p, in: %p '%s' len %zd", limiter, endptr-str, endptr, limiter_pos, str, str, str_len);
        }
        return false;
    }
    result = num;
    return true;
}

bool jau::to_integer(long long & result, const std::string& str, const char limiter, const char *limiter_pos) {
    return to_integer(result, str.c_str(), str.size(), limiter, limiter_pos);
}

bool jau::to_fraction_i64(fraction_i64& result, const std::string & value, const fraction_i64& min_allowed, const fraction_i64& max_allowed) noexcept {
    static constexpr const bool _debug = false;
    const char * str = const_cast<const char*>(value.c_str());
    const size_t str_len = value.length();
    const char *divptr = nullptr;

    divptr = std::strstr(str, "/");
    if( nullptr == divptr ) {
        if constexpr ( _debug ) {
            INFO_PRINT("Missing '/' in: '%s'", str);
        }
        return false;
    }

    long long num;
    if( !to_integer(num, str, str_len, '/', divptr) ) {
        return false;
    }

    long long denom; // 0x7ffc7090d904 != 0x7ffc7090d907 " 10 / 1000000 "
    if( !to_integer(denom, divptr+1, str_len-(divptr-str)-1, '\0', str + str_len) ) {
        return false;
    }

    fraction_i64 temp((int64_t)num, (uint64_t)denom);
    if( ! ( min_allowed <= temp && temp <= max_allowed ) ) {
        // invalid user value range
        if constexpr ( _debug ) {
            INFO_PRINT("Numerator out of range, not %s <= %s <= %s, in: '%s'", min_allowed.to_string().c_str(), temp.to_string().c_str(), max_allowed.to_string().c_str(), str);
        }
        return false;
    }
    result = temp;
    return true;
}

std::string jau::math::to_string(const jau::math::math_error_t v) noexcept {
    switch(v) {
        case jau::math::math_error_t::invalid: return "invalid";
        case jau::math::math_error_t::div_by_zero: return "div_by_zero";
        case jau::math::math_error_t::overflow: return "overflow";
        case jau::math::math_error_t::underflow: return "underflow";
        case jau::math::math_error_t::inexact: return "inexact";
    }
    return "unknown";
}
