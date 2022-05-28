/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020 Gothel Software e.K.
 * Copyright (c) 2020 ZAFENA AB
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

#include <algorithm>

#include <jau/debug.hpp>
#include <jau/basic_types.hpp>

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
    clock_gettime(CLOCK_MONOTONIC, &t);
    return fraction_timespec( (int64_t)t.tv_sec, (int64_t)t.tv_nsec );
}

fraction_timespec jau::getWallClockTime() noexcept {
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    return fraction_timespec( (int64_t)t.tv_sec, (int64_t)t.tv_nsec );
}

uint64_t jau::getCurrentMilliseconds() noexcept {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return static_cast<uint64_t>( t.tv_sec ) * MilliPerOne +
           static_cast<uint64_t>( t.tv_nsec ) / NanoPerMilli;
}

uint64_t jau::getWallClockSeconds() noexcept {
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    return static_cast<uint64_t>( t.tv_sec );
}

std::string fraction_timespec::to_string() const noexcept {
    return std::to_string(tv_sec) + "s + " + std::to_string(tv_nsec) + "ns";
}

std::string fraction_timespec::to_iso8601_string(const bool use_space) const noexcept {
    std::time_t t0 = static_cast<std::time_t>(tv_sec);
    struct std::tm tm_0;
    if( nullptr == ::gmtime_r( &t0, &tm_0 ) ) {
        return use_space ? "1970-01-01 00:00:00" : "1970-01-01T00:00:00Z"; // 20 + 1
    } else {
        char b[20+1];
        strftime(b, sizeof(b), use_space ? "%Y-%m-%d %H:%M:%S" : "%Y-%m-%dT%H:%M:%SZ", &tm_0);
        return std::string(b);
    }
}

void jau::sleep_until(const fraction_timespec& absolute_time, const bool monotonic) noexcept {
    if( absolute_time <= fraction_tv::zero ) {
        return;
    }
    // typedef struct timespec __gthread_time_t;
    __gthread_time_t ts = absolute_time.to_timespec();

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

// Hack for glibc/pthread library w/o pthread_cond_clockwait,
// i.e. on g++8, arm32, Debian 10.
// Here we have to use pthread_cond_timedwait(), ignoring the clock type.
//
// __attribute__((weak)) tested w/ g++8.3, g++10 and clang-11
//
extern int pthread_cond_clockwait (pthread_cond_t *__restrict __cond,
                   pthread_mutex_t *__restrict __mutex,
                   __clockid_t __clock_id,
                   const struct timespec *__restrict __abstime)
     __nonnull ((1, 2, 4)) __attribute__((weak));

static bool __jau__has_pthread_cond_clockwait() noexcept {
    const bool r = nullptr != pthread_cond_clockwait;
    fprintf(stderr, "INFO: jau::has_pthread_cond_clockwait: %d\n", r);
    return r;
}
static bool jau_has_pthread_cond_clockwait() noexcept {
    static bool r = __jau__has_pthread_cond_clockwait();
    return r;
}

std::cv_status jau::wait_until(std::condition_variable& cv, std::unique_lock<std::mutex>& lock, const fraction_timespec& absolute_time, const bool monotonic) noexcept {
    if( absolute_time <= fraction_tv::zero ) {
        return std::cv_status::no_timeout;
    }
    // typedef struct timespec __gthread_time_t;
    __gthread_time_t ts = absolute_time.to_timespec();

    if( jau_has_pthread_cond_clockwait() ) {
        pthread_cond_clockwait(cv.native_handle(), lock.mutex()->native_handle(),
                               monotonic ? CLOCK_MONOTONIC : CLOCK_REALTIME, &ts);
    } else {
        pthread_cond_timedwait(cv.native_handle(), lock.mutex()->native_handle(), &ts);
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


jau::ExceptionBase::ExceptionBase(std::string const type, std::string const m, const char* file, int line) noexcept
: msg_(std::string(type).append(" @ ").append(file).append(":").append(std::to_string(line)).append(": ").append(m)),
  backtrace_( jau::get_backtrace(true /* skip_anon_frames */) )
{
    what_ = msg_;
    what_.append("\nNative backtrace:\n");
    what_.append(backtrace_);
}

std::string jau::get_string(const uint8_t *buffer, nsize_t const buffer_len, nsize_t const max_len) noexcept {
    const nsize_t cstr_len = std::min(buffer_len, max_len);
    char cstr[max_len+1]; // EOS
    memcpy(cstr, buffer, cstr_len);
    cstr[cstr_len] = 0; // EOS
    return std::string(cstr);
}

void jau::trimInPlace(std::string &s) noexcept {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch);
    }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

std::string jau::trimCopy(const std::string &_s) noexcept {
    std::string s(_s);
    trimInPlace(s);
    return s;
}

uint128_t jau::merge_uint128(uint16_t const uuid16, uint128_t const & base_uuid, nsize_t const uuid16_le_octet_index)
{
    if( uuid16_le_octet_index > 14 ) {
        std::string msg("uuid16_le_octet_index ");
        msg.append(std::to_string(uuid16_le_octet_index));
        msg.append(", not within [0..14]");
        throw IllegalArgumentException(msg, E_FILE_LINE);
    }
    uint128_t dest = base_uuid;

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
    static_assert(isLittleOrBigEndian()); // one static_assert is sufficient for whole compilation unit
    nsize_t offset;
    if( isBigEndian() ) {
        offset = 15 - 1 - uuid16_le_octet_index;
    } else {
        offset = uuid16_le_octet_index;
    }
    // uint16_t * destu16 = (uint16_t*)(dest.data + offset);
    // *destu16 += uuid16;
    reinterpret_cast<packed_t<uint16_t>*>( dest.data + offset )->store += uuid16;
    return dest;
}

uint128_t jau::merge_uint128(uint32_t const uuid32, uint128_t const & base_uuid, nsize_t const uuid32_le_octet_index)
{
    if( uuid32_le_octet_index > 12 ) {
        std::string msg("uuid32_le_octet_index ");
        msg.append(std::to_string(uuid32_le_octet_index));
        msg.append(", not within [0..12]");
        throw IllegalArgumentException(msg, E_FILE_LINE);
    }
    uint128_t dest = base_uuid;

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
    static_assert(isLittleOrBigEndian()); // one static_assert is sufficient for whole compilation unit
    nsize_t offset;;
    if( isBigEndian() ) {
        offset = 15 - 3 - uuid32_le_octet_index;
    } else {
        offset = uuid32_le_octet_index;
    }
    // uint32_t * destu32 = (uint32_t*)(dest.data + offset);
    // *destu32 += uuid32;
    reinterpret_cast<packed_t<uint32_t>*>( dest.data + offset )->store += uuid32;
    return dest;
}


static snsize_t hexCharByte_(const char c)
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

nsize_t jau::hexStringBytes(std::vector<uint8_t>& out, const std::string& hexstr, const bool lsbFirst, const bool checkLeading0x) noexcept {
    ssize_t offset;
    if( checkLeading0x && hexstr.size() >= 2 && hexstr[0] == '0' && hexstr[1] == 'x' ) {
        offset = 2;
    } else {
        offset = 0;
    }
    const ssize_t size = ( hexstr.size() - offset ) / 2;
    out.clear();
    out.reserve(size);
    if( lsbFirst ) {
        for (ssize_t i = 0; i < size; i++) {
            const nsize_t idx = i * 2;
            const snsize_t h = hexCharByte_( hexstr[ offset + idx ] );
            const snsize_t l = hexCharByte_( hexstr[ offset + idx + 1 ] );
            if( 0 <= h && 0 <= l ) {
                out.push_back( static_cast<uint8_t>( (h << 4) + l ) );
            } else {
                // invalid char
                return out.size();
            }
        }
    } else {
        for(ssize_t idx = (size-1)*2; idx >= 0; idx-=2) {
            const snsize_t h = hexCharByte_( hexstr[ offset + idx ] );
            const snsize_t l = hexCharByte_( hexstr[ offset + idx + 1 ] );
            if( 0 <= h && 0 <= l ) {
                out.push_back( static_cast<uint8_t>( (h << 4) + l ) );
            } else {
                // invalid char
                return out.size();
            }
        }
    }
    return out.size();
}


static const char* HEX_ARRAY_LOW = "0123456789abcdef";
static const char* HEX_ARRAY_BIG = "0123456789ABCDEF";

std::string jau::bytesHexString(const uint8_t * bytes, const nsize_t offset, const nsize_t length,
                                const bool lsbFirst, const bool lowerCase) noexcept
{
    const char* hex_array = lowerCase ? HEX_ARRAY_LOW : HEX_ARRAY_BIG;
    std::string str;

    if( nullptr == bytes ) {
        return "null";
    }
    if( 0 == length ) {
        return "nil";
    }
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

std::string jau::to_string(const endian& v) noexcept {
    switch(v) {
        case endian::little:  return "little";
        case endian::big:  return "big";
        case endian::pdp:  return "pdb";
        case endian::honeywell: return "honeywell";
        case endian::undefined: return "undefined";
    }
    return "unlisted";
}

static bool to_integer(long long & result, const char * str, size_t str_len, const char limiter, const char *limiter_pos) {
    static constexpr const bool _debug = false;
    char *endptr = NULL;
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

bool jau::to_fraction_i64(fraction_i64& result, const std::string & value, const fraction_i64& min_allowed, const fraction_i64& max_allowed) noexcept {
    static constexpr const bool _debug = false;
    const char * str = const_cast<const char*>(value.c_str());
    const size_t str_len = value.length();
    const char *divptr = NULL;

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
    result = std::move(temp);
    return true;
}

