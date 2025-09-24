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

#include <algorithm>
#include <cassert>
#include <cinttypes>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <regex>
#include <string>
#include <utility>

#include <jau/basic_types.hpp>
#include <jau/byte_util.hpp>
#include <jau/cpp_lang_util.hpp>
#include <jau/debug.hpp>
#include <jau/float_math.hpp>
#include <jau/functional.hpp>
#include <jau/int_math.hpp>
#include <jau/int_types.hpp>
#include <jau/math/math_error.hpp>
#include <jau/secmem.hpp>
#include <jau/string_util.hpp>

using namespace jau;

void jau::zero_bytes_sec(void *s, size_t n) noexcept __attrdef_no_optimize__ {
    // asm asm-qualifiers ( AssemblerTemplate : OutputOperands [ : InputOperands [ : Clobbers ] ] )
    asm volatile("" : "+r,m"(s), "+r,m"(n) : : "memory");  // a nop asm, usually guaranteeing synchronized order and non-optimization
    ::explicit_bzero(s, n);
    // ::bzero(s, n);
    // ::memset(s, 0, n);
}

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
    return fraction_timespec((int64_t)t.tv_sec, (int64_t)t.tv_nsec);
}

fraction_timespec jau::getWallClockTime() noexcept {
    struct timespec t{ .tv_sec = 0, .tv_nsec = 0 };
    ::clock_gettime(CLOCK_REALTIME, &t);
    return fraction_timespec((int64_t)t.tv_sec, (int64_t)t.tv_nsec);
}

uint64_t jau::getCurrentMilliseconds() noexcept {
    constexpr uint64_t ms_per_sec =         1'000UL;
    constexpr uint64_t ns_per_ms  =     1'000'000UL;
    struct timespec t { .tv_sec=0, .tv_nsec=0 };
    ::clock_gettime(CLOCK_MONOTONIC, &t);
    return static_cast<uint64_t>(t.tv_sec) * ms_per_sec +
           static_cast<uint64_t>(t.tv_nsec) / ns_per_ms;
}

uint64_t jau::getWallClockSeconds() noexcept {
    struct timespec t{ .tv_sec = 0, .tv_nsec = 0 };
    ::clock_gettime(CLOCK_REALTIME, &t);
    return static_cast<uint64_t>(t.tv_sec);
}

std::string fraction_timespec::toString() const noexcept {
    return std::to_string(tv_sec) + "s + " + std::to_string(tv_nsec) + "ns";
}

std::string fraction_timespec::toISO8601String(bool space_separator, bool muteTime) const noexcept {
    std::time_t t0 = static_cast<std::time_t>(tv_sec);
    struct std::tm tm_0;
    if ( nullptr == ::gmtime_r(&t0, &tm_0) ) {
        if ( muteTime ) {
            if ( space_separator ) {
                return "1970-01-01";
            } else {
                return "1970-01-01Z";
            }
        } else {
            if ( space_separator ) {
                return "1970-01-01 00:00:00";
            } else {
                return "1970-01-01T00:00:00Z";
            }
        }
    } else {
        // 2022-05-28T23:23:50Z 20+1
        //
        // 1655994850s + 228978909ns
        // 2022-06-23T14:34:10.228978909Z 30+1
        char b[30 + 1];
        size_t p;
        if ( muteTime || (0 == tm_0.tm_hour && 0 == tm_0.tm_min && 0 == tm_0.tm_sec && 0 == tv_nsec) ) {
            p = ::strftime(b, sizeof(b), "%Y-%m-%d", &tm_0);
        } else {
            if ( space_separator ) {
                p = ::strftime(b, sizeof(b), "%Y-%m-%d %H:%M:%S", &tm_0);
            } else {
                p = ::strftime(b, sizeof(b), "%Y-%m-%dT%H:%M:%S", &tm_0);
            }
        }
        if ( 0 < p && p < sizeof(b) - 1 ) {
            size_t q = 0;
            const size_t remaining = sizeof(b) - p;
            if ( !muteTime && 0 < tv_nsec ) {
                q = ::snprintf(b + p, remaining, ".%09" PRIi64, tv_nsec);
            }
            if ( !space_separator ) {
                ::snprintf(b + p + q, remaining - q, "Z");
            }
        }
        return std::string(b);
    }
}

fraction_timespec fraction_timespec::from(int year, unsigned month, unsigned day,
                                          unsigned hour, unsigned minute,
                                          unsigned seconds, uint64_t nano_seconds) noexcept {
    fraction_timespec res;
    if ( !(1 <= month && month <= 12 &&
           1 <= day && day <= 31 &&
           hour <= 23 &&
           minute <= 59 && seconds <= 60) ) {
        return res;  // error
    }
    struct std::tm tm_0;
    ::memset(&tm_0, 0, sizeof(tm_0));
    tm_0.tm_year = year - 1900;                 // years since 1900
    tm_0.tm_mon = static_cast<int>(month) - 1;  // months since Janurary [0-11]
    tm_0.tm_mday = static_cast<int>(day);       // day of the month [1-31]
    tm_0.tm_hour = static_cast<int>(hour);      // hours since midnight [0-23]
    tm_0.tm_min = static_cast<int>(minute);     // minutes after the hour [0-59]
    tm_0.tm_sec = static_cast<int>(seconds);    // seconds after the minute [0-60], including one leap second
    std::time_t t1 = ::timegm(&tm_0);
    res.tv_sec = static_cast<int64_t>(t1);
    res.tv_nsec = static_cast<int64_t>(nano_seconds);
    return res;
}

fraction_timespec fraction_timespec::from(const std::string &datestr, Bool addUTCOffset) noexcept {
    int64_t utcOffsetSec;
    size_t consumedChars;
    fraction_timespec res = from(datestr, utcOffsetSec, consumedChars);
    if ( value(addUTCOffset) ) {
        res.tv_sec += utcOffsetSec;
    }
    (void)consumedChars;
    return res;
}

static std::regex jau_ISO8601_regex() noexcept {
    try {
        // 2024-01-02T12:34:56.789+11:00
        // - g1: year 2024
        // - g2: month 01
        // - g3: day 03
        // - g4: hour 12
        // - optional:
        //   -     T|\s+
        //   - g5: minute 34
        //   - g6: second 56
        //   - g7: second-fraction 789  (optional)
        //   - g8: UTC (Zulu) 'Z'       (optional)
        //   - g9:  TZ +/-            (optional)
        //   - g10: TZ hour 11        (optional)
        //   - g11: TZ minute 00      (optional)
        return
            std::regex( R"(^\s*(-?(?:[1-9]\d*)?\d{1,4})-(1[0-2]|0?[1-9])-(3[01]|0?[1-9]|[12]\d))"
                        //        g1                    g2              g3
                        R"((?:(?:T|\s+)(2[0-3]|1\d|0?\d):([1-5]\d|0?\d):([1-5]\d|0?\d)(?:\.(\d+))?)?)"
                        //                    g4               g5               g6           g7
                        R"((?:(Z)|(?:\s*)([+-])(2[0-3]|1\d|0?\d):?([1-5]\d|0?\d)?)?)"
                        //    g8           g9          g10              g11
                      );

    } catch ( ... ) {
        ERR_PRINT2("Caught unknown exception");
        return std::regex();
    }
}

fraction_timespec fraction_timespec::from(const std::string &datestr, int64_t &utcOffsetSec, size_t &consumedChars) noexcept {
    static std::regex pattern = jau_ISO8601_regex();

    consumedChars = 0;
    utcOffsetSec = 0;

    std::smatch match;
    try {
        if ( std::regex_search(datestr, match, pattern) ) {
            consumedChars = match.length();
            constexpr bool DBG_OUT = false;
            if constexpr ( DBG_OUT ) {
                std::cout << "XXX: " << datestr << std::endl;
                std::cout << "XXX: match pos " << match.position() << ", len " << match.length() << ", sz " << match.size() << std::endl;
                for ( size_t i = 0; i < match.size(); ++i ) {
                    const std::string &ms = match[i];
                    std::cout << "- [" << i << "]: '" << ms << "', len " << ms.length() << std::endl;
                }
            }
            int y = 0;
            int M = 0, d = 0;
            int h = 0, m = 0, s = 0;
            uint64_t ns = 0;
            int offset_sign = 1, offset_h = 0, offset_m = 0;

            if ( match.size() > 1 && match[1].length() > 0 ) {
                y = std::stoi(match[1]);
                if ( match.size() > 2 && match[2].length() > 0 ) {
                    M = std::stoi(match[2]);
                    if ( match.size() > 3 && match[3].length() > 0 ) {
                        d = std::stoi(match[3]);
                        if ( match.size() > 4 && match[4].length() > 0 ) {
                            h = std::stoi(match[4]);
                            if ( match.size() > 5 && match[5].length() > 0 ) {
                                m = std::stoi(match[5]);
                                if ( match.size() > 6 && match[6].length() > 0 ) {
                                    s = std::stoi(match[6]);
                                    if ( match.size() > 7 && match[7].length() > 0 ) {
                                        ns = std::stoul(match[7]);
                                    }
                                    if ( match.size() > 8 && match[8].length() > 0 ) {
                                        // UTC Zulu
                                    } else if ( match.size() > 10 && match[9].length() > 0 && match[10].length() > 0 ) {
                                        if ( match[9] == "-" ) {
                                            offset_sign = -1;
                                        }
                                        offset_h = std::stoi(match[10]);
                                        if ( match.size() > 11 && match[11].length() > 0 ) {
                                            offset_m = std::stoi(match[11]);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            if ( 0 < ns ) {
                const size_t ns_digits = jau::digits10(ns, 1, true);
                if ( 9 < ns_digits ) {
                    ns = 0;  // error in remainder
                }
                ns = ns * static_cast<uint64_t>(std::pow(10, 9 - ns_digits));
            }
            // add the timezone offset if desired, rendering result non-UTC
            utcOffsetSec = offset_sign * ((offset_h * 60_i64 * 60_i64) + (offset_m * 60_i64));
            return fraction_timespec::from(y, M, d, h, m, s, ns);
        }
    } catch ( ... ) {
        ERR_PRINT2("Caught unknown exception parsing %s", datestr.c_str());
    }
    return fraction_timespec();  // error
}

bool jau::milli_sleep(uint64_t td_ms, const bool ignore_irq) noexcept {
    constexpr uint64_t ms_per_sec =         1'000UL;
    constexpr uint64_t ns_per_ms  =     1'000'000UL;
    constexpr uint64_t ns_per_sec = 1'000'000'000UL;;
    const int64_t td_ns_0 = static_cast<int64_t>( (td_ms * ns_per_ms) % ns_per_sec );
    struct timespec ts;
    ts.tv_sec = static_cast<decltype(ts.tv_sec)>(td_ms / ms_per_sec);  // signed 32- or 64-bit integer
    ts.tv_nsec = td_ns_0;
    int res;
    do {
        res = ::clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, &ts);
    } while ( ignore_irq && EINTR == res );
    return 0 == res;
}
bool jau::sleep(const fraction_timespec &relative_time, const bool ignore_irq) noexcept {
    struct timespec ts = relative_time.to_timespec();
    int res;
    do {
        res = ::clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, &ts);
    } while ( ignore_irq && EINTR == res );
    return 0 == res;
}

bool jau::sleep_until(const fraction_timespec &absolute_time, const bool monotonic, const bool ignore_irq) noexcept {
    if ( absolute_time <= fraction_tv::zero ) {
        return false;
    }
    // typedef struct timespec __gthread_time_t;
    struct timespec ts = absolute_time.to_timespec();
    int res;
    do {
        res = ::clock_nanosleep(monotonic ? CLOCK_MONOTONIC : CLOCK_REALTIME,
                                TIMER_ABSTIME, &ts, &ts);
    } while ( ignore_irq && EINTR == res );
    return 0 == res;
}

bool jau::sleep_for(const fraction_timespec &relative_time, const bool monotonic, const bool ignore_irq) noexcept {
    if ( relative_time <= fraction_tv::zero ) {
        return false;
    }
    const fraction_timespec now = monotonic ? getMonotonicTime() : getWallClockTime();
    return sleep_until(now + relative_time, monotonic, ignore_irq);
}

bool jau::sleep_for(const fraction_i64 &relative_time, const bool monotonic, const bool ignore_irq) noexcept {
    if ( relative_time <= fractions_i64::zero ) {
        return false;
    }
    bool overflow = false;
    const fraction_timespec atime = (monotonic ? getMonotonicTime() : getWallClockTime()) + fraction_timespec(relative_time, &overflow);
    if ( overflow ) {
        return false;
    } else {
        return sleep_until(atime, monotonic, ignore_irq);
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

std::cv_status jau::wait_until(std::condition_variable &cv, std::unique_lock<std::mutex> &lock, const fraction_timespec &absolute_time, const bool monotonic) noexcept {
    if ( absolute_time <= fraction_tv::zero ) {
        return std::cv_status::no_timeout;
    }
    // typedef struct timespec __gthread_time_t;
    struct timespec ts = absolute_time.to_timespec();

    if ( jau_has_pthread_cond_clockwait() ) {
        pthread_cond_clockwait(cv.native_handle(), lock.mutex()->native_handle(),
                               monotonic ? CLOCK_MONOTONIC : CLOCK_REALTIME, &ts);
    } else {
        ::pthread_cond_timedwait(cv.native_handle(), lock.mutex()->native_handle(), &ts);
    }

    const fraction_timespec now = monotonic ? getMonotonicTime() : getWallClockTime();
    return now < absolute_time ? std::cv_status::no_timeout : std::cv_status::timeout;
}

std::cv_status jau::wait_for(std::condition_variable &cv, std::unique_lock<std::mutex> &lock, const fraction_timespec &relative_time, const bool monotonic) noexcept {
    if ( relative_time <= fraction_tv::zero ) {
        return std::cv_status::no_timeout;
    }
    const fraction_timespec now = monotonic ? getMonotonicTime() : getWallClockTime();
    return wait_until(cv, lock, now + relative_time, monotonic);
}

std::cv_status jau::wait_for(std::condition_variable &cv, std::unique_lock<std::mutex> &lock, const fraction_i64 &relative_time, const bool monotonic) noexcept {
    if ( relative_time <= fractions_i64::zero ) {
        return std::cv_status::no_timeout;
    }
    bool overflow = false;
    const fraction_timespec atime = (monotonic ? getMonotonicTime() : getWallClockTime()) + fraction_timespec(relative_time, &overflow);
    if ( overflow ) {
        return std::cv_status::timeout;
    } else {
        return wait_until(cv, lock, atime, monotonic);
    }
}

std::string jau::threadName(const std::thread::id id) noexcept {
    #if 1
        return "Thread 0x"+jau::toHexString( std::hash<std::thread::id>{}(id) );
    #else
        std::stringstream ss;
        ss.setf(std::ios_base::hex | std::ios_base::showbase);
        ss << "Thread " << id;
        return ss.str();
    #endif
}

jau::ExceptionBase::ExceptionBase(std::string &&type, std::string const& m, const char* file, int line) noexcept // NOLINT(modernize-pass-by-value)
: msg_( std::move(type) ),
  backtrace_( jau::get_backtrace(true /* skip_anon_frames */) )
{
    msg_.append(" @ ").append(file).append(":").append(std::to_string(line)).append(": ").append(m);
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
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) { // NOLINT(modernize-use-ranges)
        return !std::isspace(ch);
    }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) { // NOLINT(modernize-use-ranges)
        return !std::isspace(ch);
    }).base(), s.end());
}

std::string jau::trim(const std::string &_s) noexcept {
    std::string s(_s);
    trimInPlace(s);
    return s;
}

std::vector<std::string> jau::split_string(const std::string &str, const std::string &separator) noexcept {
    std::vector<std::string> res;
    size_t p0 = 0;
    while ( p0 != std::string::npos && p0 < str.size() ) {
        size_t p1 = str.find(separator, p0);
        res.push_back(str.substr(p0, p1));  // incl. npos
        if ( p1 != std::string::npos ) {
            p1 += separator.length();
        }
        p0 = p1;
    }
    return res;
}

std::string &jau::toLowerInPlace(std::string &s) noexcept {
    std::transform(s.begin(), s.end(), s.begin(),  // NOLINT(modernize-use-ranges)
                   [](unsigned char c) { return std::tolower(c); });
    return s;
}
std::string jau::toLower(const std::string& s) noexcept {
    std::string t(s); toLowerInPlace(t); return t;
}

// one static_assert is sufficient for whole compilation unit
static_assert(is_defined_endian(endian_t::native));
static_assert(is_little_or_big_endian());

uint128dp_t jau::merge_uint128(uint16_t const uuid16, uint128dp_t const &base_uuid, nsize_t const uuid16_le_octet_index) {
    if ( uuid16_le_octet_index > 14 ) {
        std::string msg("uuid16_le_octet_index ");
        msg.append(std::to_string(uuid16_le_octet_index));
        msg.append(", not within [0..14]");
        throw IllegalArgumentError(msg, E_FILE_LINE);
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
    if ( is_big_endian() ) {
        offset = 15 - 1 - uuid16_le_octet_index;
    } else {
        offset = uuid16_le_octet_index;
    }
    // uint16_t * destu16 = (uint16_t*)(dest.data + offset);
    // *destu16 += uuid16;
    reinterpret_cast<packed_t<uint16_t> *>(dest.data + offset)->store += uuid16;
    return dest;
}

uint128dp_t jau::merge_uint128(uint32_t const uuid32, uint128dp_t const &base_uuid, nsize_t const uuid32_le_octet_index) {
    if ( uuid32_le_octet_index > 12 ) {
        std::string msg("uuid32_le_octet_index ");
        msg.append(std::to_string(uuid32_le_octet_index));
        msg.append(", not within [0..12]");
        throw IllegalArgumentError(msg, E_FILE_LINE);
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
    nsize_t offset;
    if ( is_big_endian() ) {
        offset = 15 - 3 - uuid32_le_octet_index;
    } else {
        offset = uuid32_le_octet_index;
    }
    // uint32_t * destu32 = (uint32_t*)(dest.data + offset);
    // *destu32 += uuid32;
    reinterpret_cast<packed_t<uint32_t> *>(dest.data + offset)->store += uuid32;
    return dest;
}

static snsize_t hexCharByte_(const uint8_t c) {
    if ( '0' <= c && c <= '9' ) {
        return c - '0';
    }
    if ( 'A' <= c && c <= 'F' ) {
        return c - 'A' + 10;
    }
    if ( 'a' <= c && c <= 'f' ) {
        return c - 'a' + 10;
    }
    return -1;
}

SizeBoolPair jau::fromHexString(std::vector<uint8_t> &out, const uint8_t hexstr[], const size_t hexstr_len, const bool lsbFirst, const Bool checkPrefix) noexcept {
    size_t offset;
    if ( *checkPrefix && hexstr_len >= 2 && hexstr[0] == '0' && hexstr[1] == 'x' ) {
        offset = 2;
    } else {
        offset = 0;
    }
    size_t lsb, msb;
    if ( lsbFirst ) {
        lsb = 1;
        msb = 0;
    } else {
        lsb = 0;
        msb = 1;
    }
    const size_t hexlen_in = hexstr_len - offset;
    const size_t bsize = hexlen_in / 2;
    out.clear();
    out.reserve(bsize + hexlen_in % 2);

    // Odd nibbles:
    // - 0xf[12] = 0x0f12 = { 0x12, 0x0f } - msb, 1st single low-nibble is most significant
    // -   [12]f = 0xf012 = { 0x12, 0xf0 } - lsb, last single high-nibble is most significant
    //
    const bool has_single_nibble = 0 < hexlen_in % 2;
    uint8_t high_msb_nibble = 0;
    if ( !lsbFirst && has_single_nibble ) {
        // consume single MSB nibble
        const size_t idx = 0;
        assert(hexstr_len - 1 >= offset + idx);
        const snsize_t l = hexCharByte_(hexstr[offset + idx]);
        if ( 0 <= l ) {
            high_msb_nibble = static_cast<uint8_t>(l);
        } else {
            // invalid char
            return { .s = offset, .b = false };
        }
        ++offset;
    }
    for ( size_t i = 0; i < bsize; ++i ) {
        const size_t idx = (lsb * i + msb * (bsize - 1 - i)) * 2;
        assert(hexstr_len - 1 >= offset + idx + 1);
        const snsize_t h = hexCharByte_(hexstr[offset + idx]);
        const snsize_t l = hexCharByte_(hexstr[offset + idx + 1]);
        if ( 0 <= h && 0 <= l ) {
            out.push_back(static_cast<uint8_t>((h << 4) + l));
        } else if ( 0 > h ) {
            // invalid 1st char
            return { .s = offset + 2 * i, .b = false };
        } else {
            // invalid 2nd char
            return { .s = offset + 2 * i + 1, .b = false };
        }
    }
    offset += bsize * 2;
    if ( has_single_nibble ) {
        if ( lsbFirst ) {
            assert(hexstr_len - 1 == offset);
            const snsize_t h = hexCharByte_(hexstr[offset]);
            if ( 0 <= h ) {
                out.push_back(static_cast<uint8_t>((h << 4) + 0));
            } else {
                // invalid char
                return { .s = offset, .b = false };
            }
            ++offset;
        } else {
            out.push_back(high_msb_nibble);
        }
    }
    assert(hexlen_in / 2 + hexlen_in % 2 == out.size());
    assert(offset == hexstr_len);
    return { .s = offset, .b = true };
}

UInt64SizeBoolTuple jau::fromHexString(std::string_view const hexstr, const bool lsbFirst, const Bool checkPrefix) noexcept {
    std::vector<uint8_t> out;
    auto [consumed, complete] = fromHexString(out, hexstr, lsbFirst, checkPrefix);
    if constexpr ( jau::is_little_endian() ) {
        while ( out.size() < sizeof(uint64_t) ) {
            out.push_back(0);
        }
    } else {
        while ( out.size() < sizeof(uint64_t) ) {
            out.insert(out.cbegin(), 0);
        }
    }
    uint64_t result = jau::le_to_cpu(*pointer_cast<const uint64_t *>(out.data()));
    return { .v = result, .s = consumed, .b = complete };
}

static snsize_t bitCharByte_(const uint8_t c) {
    if ( '0' <= c && c <= '1' ) {
        return c - '0';
    }
    return -1;
}

SizeBoolPair jau::fromBitString(std::vector<uint8_t> &out, const uint8_t bitstr[], const size_t bitstr_len, const bool lsbFirst, const Bool checkPrefix) noexcept {
    size_t offset;
    if ( *checkPrefix && bitstr_len >= 2 && bitstr[0] == '0' && bitstr[1] == 'b' ) {
        offset = 2;
    } else {
        offset = 0;
    }
    size_t lsb, msb;
    if ( lsbFirst ) {
        lsb = 1;
        msb = 0;
    } else {
        lsb = 0;
        msb = 1;
    }
    const size_t bitlen_in = bitstr_len - offset;
    const size_t bsize = bitlen_in / 8;
    const size_t bnibbles = bitlen_in % 8;
    out.clear();
    out.reserve(bsize + (bnibbles > 0 ? 1 : 0));

    // Nibbles (incomplete octets):
    // - 0b11[00000001] = 0x0301 = { 0x01, 0x03 } - msb, 1st single low-nibble is most significant
    // - 0b[01000000]11 = 0xC040 = { 0x40, 0xC0 } - lsb, last single high-nibble is most significant
    //   - 11 -> 11000000 -> C0
    //
    uint8_t cached_nibble8 = 0;
    if ( !lsbFirst && 0 < bnibbles ) {
        // consume single MSB nibble
        assert(bitstr_len - 1 >= offset + bnibbles - 1);
        for ( size_t i = 0; i < bnibbles; ++i ) {
            const snsize_t l = bitCharByte_(bitstr[offset + i]);
            if ( 0 <= l ) {
                cached_nibble8 |= static_cast<uint8_t>(l) << (bnibbles - 1 - i);
            } else {
                // invalid char
                return { .s = offset + i, .b = false };
            }
        }
        offset += bnibbles;
    }
    for ( size_t i = 0; i < bsize; ++i ) {
        uint8_t b = 0;
        const size_t idx = (lsb * i + msb * (bsize - 1 - i)) * 8;
        assert(bitstr_len - 1 >= offset + idx + 8 - 1);
        for ( size_t j = 0; j < 8; ++j ) {
            const snsize_t l = bitCharByte_(bitstr[offset + idx + j]);
            if ( 0 <= l ) {
                b |= static_cast<uint8_t>(l) << (8 - 1 - j);
            } else {
                // invalid char
                return { .s = offset + 8 * i + j, .b = false };
            }
        }
        out.push_back(b);
    }
    offset += bsize * 8;
    if ( 0 < bnibbles ) {
        if ( lsbFirst ) {
            assert(bitstr_len - 1 >= offset + bnibbles - 1);
            for ( size_t i = 0; i < bnibbles; ++i ) {
                const snsize_t l = bitCharByte_(bitstr[offset + i]);
                if ( 0 <= l ) {
                    cached_nibble8 |= static_cast<uint8_t>(l) << (8 - 1 - i);
                } else {
                    // invalid char
                    return { .s = offset + i, .b = false };
                }
            }
            offset += bnibbles;
        }
        out.push_back(cached_nibble8);
    }
    assert(bitlen_in / 8 + (bitlen_in % 8 ? 1 : 0) == out.size());
    assert(offset == bitstr_len);
    return { .s = offset, .b = true };
}

UInt64SizeBoolTuple jau::fromBitString(std::string_view const bitstr, const bool lsbFirst, const Bool checkPrefix) noexcept {
    std::vector<uint8_t> out;
    auto [consumed, complete] = fromBitString(out, bitstr, lsbFirst, checkPrefix);
    if constexpr ( jau::is_little_endian() ) {
        while ( out.size() < sizeof(uint64_t) ) {
            out.push_back(0);
        }
    } else {
        while ( out.size() < sizeof(uint64_t) ) {
            out.insert(out.cbegin(), 0);
        }
    }
    uint64_t result = jau::le_to_cpu(*pointer_cast<const uint64_t *>(out.data()));
    return { .v = result, .s = consumed, .b = complete };
}

static constexpr const char *HEX_ARRAY_BIG = "0123456789ABCDEF";

std::string jau::toHexString(const void *data, const nsize_t length,
                             const bool lsbFirst, const bool lowerCase, const bool skipPrefix) noexcept {
    const char *hex_array = lowerCase ? HexadecimalArray : HEX_ARRAY_BIG;
    std::string str;

    if ( nullptr == data ) {
        return "null";
    }
    if ( 0 == length ) {
        return "nil";
    }
    const uint8_t *const bytes = static_cast<const uint8_t *>(data);
    if ( lsbFirst ) {
        // LSB left -> MSB right, no leading `0x`
        // TODO: skip tail all-zeros?
        str.reserve(length * 2 + 1);
        for ( nsize_t i = 0; i < length; i++ ) {
            const int v = bytes[i] & 0xFF;
            str.push_back(hex_array[v >> 4]);
            str.push_back(hex_array[v & 0x0F]);
        }
    } else {
        // MSB left -> LSB right, with leading `0x`
        if ( skipPrefix ) {
            str.reserve(length * 2 + 1);
        } else {
            str.reserve(length * 2 + 1 + 2);
            str.push_back('0');
            str.push_back('x');
        }
        bool skip_leading_zeros = true;
        nsize_t i = length;
        do {
            i--;
            const int v = bytes[i] & 0xFF;
            if ( 0 != v || !skip_leading_zeros || i == 0 ) {
                str.push_back(hex_array[v >> 4]);
                str.push_back(hex_array[v & 0x0F]);
                skip_leading_zeros = false;
            }
        } while ( i != 0 );
    }
    return str;
}

std::string &jau::appendToHexString(std::string &dest, const uint8_t value, const bool lowerCase) noexcept {
    const char *hex_array = lowerCase ? HexadecimalArray : HEX_ARRAY_BIG;

    if ( 2 > dest.capacity() - dest.size() ) {  // Until C++20, then reserve is ignored if capacity > reserve
        dest.reserve(dest.size() + 2);
    }
    const int v = value & 0xFF;
    dest.push_back(hex_array[v >> 4]);
    dest.push_back(hex_array[v & 0x0F]);
    return dest;
}

std::string jau::toBitString(const void *data, const nsize_t length,
                                const bool lsbFirst, const bool skipPrefix) noexcept {
    std::string str;

    if ( nullptr == data ) {
        return "null";
    }
    if ( 0 == length ) {
        return "nil";
    }
    const uint8_t *const bytes = static_cast<const uint8_t *>(data);
    if ( lsbFirst ) {
        // LSB left -> MSB right, no leading `0x`
        // TODO: skip tail all-zeros?
        str.reserve(length * 8 + 1);
        for ( nsize_t i = 0; i < length; i++ ) {
            const nsize_t v = bytes[i] & 0xFF;
            for ( nsize_t j = 0; j < 8; ++j ) {
                str.push_back(char('0' + (v >> (8 - 1 - j) & 1)));
            }
        }
    } else {
        // MSB left -> LSB right, with leading `0b`
        if ( skipPrefix ) {
            str.reserve(length * 8 + 1);
        } else {
            str.reserve(length * 8 + 1 + 2);
            str.push_back('0');
            str.push_back('b');
        }
        bool skip_leading_zeros = true;
        nsize_t i = length;
        do {
            i--;
            const int v = bytes[i] & 0xFF;
            if ( 0 != v || !skip_leading_zeros || i == 0 ) {
                for ( nsize_t j = 0; j < 8; ++j ) {
                    str.push_back(char('0' + (v >> (8 - 1 - j) & 1)));
                }
                skip_leading_zeros = false;
            }
        } while ( i != 0 );
    }
    return str;
}
std::string jau::to_string(const endian_t v) noexcept {
    switch(v) {
        case endian_t::little:  return "little";
        case endian_t::big:  return "big";
        case endian_t::pdp:  return "pdb";
        case endian_t::honeywell: return "honeywell";
        case endian_t::undefined: return "undefined";
    }
    return "undef";
}

std::string jau::to_string(const lb_endian_t v) noexcept {
    switch(v) {
        case lb_endian_t::little:  return "little";
        case lb_endian_t::big:  return "big";
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

Int64SizeBoolTuple jau::to_integer(const char *str, size_t str_len, const jau::nsize_t radix, const char limiter, const char *limiter_pos) {
    int64_t result = 0;
    size_t consumed = 0;
    bool complete = false;
    static constexpr const bool _debug = false;
    char *endptr = nullptr;
    if ( nullptr == limiter_pos ) {
        limiter_pos = str + str_len;
    }
    errno = 0;
    const long long num = std::strtoll(str, &endptr, int(radix));
    if ( 0 != errno ) {
        // value under- or overflow occured
        if constexpr ( _debug ) {
            INFO_PRINT("Value under- or overflow occurred, value %lld in: '%s', errno %d %s", num, str, errno, strerror(errno));
        }
        return { .v = result, .s = consumed, .b = complete };
    }
    if ( nullptr == endptr || endptr == str ) {
        // no digits consumed
        if constexpr ( _debug ) {
            INFO_PRINT("Value no digits consumed @ idx %d, %p == start, in: '%s'", endptr - str, endptr, str);
        }
        return { .v = result, .s = consumed, .b = complete };
    }
    result = static_cast<int64_t>(num);
    if ( endptr < limiter_pos ) {
        while ( endptr < limiter_pos && ::isspace(*endptr) ) {  // only accept whitespace
            ++endptr;
        }
    }
    consumed = endptr - str;
    if ( *endptr == limiter || endptr == limiter_pos ) {
        complete = true;
    } else {
        // numerator value not completely valid
        if constexpr ( _debug ) {
            INFO_PRINT("Value end not '%c' @ idx %d, %p != %p, in: %p '%s' len %zd", limiter, endptr - str, endptr, limiter_pos, str, str, str_len);
        }
    }
    return { .v = result, .s = consumed, .b = complete };
}

FracI64SizeBoolTuple jau::to_fraction_i64(const std::string &value, const fraction_i64 &min_allowed, const fraction_i64 &max_allowed) noexcept {
    static constexpr const bool _debug = false;
    fraction_i64 result;
    size_t consumed = 0;
    const char *str = const_cast<const char *>(value.c_str());
    const size_t str_len = value.length();
    const char *divptr = nullptr;

    divptr = std::strstr(str, "/");
    if ( nullptr == divptr ) {
        if constexpr ( _debug ) {
            INFO_PRINT("Missing '/' in: '%s'", str);
        }
        return { .v = result, .s = consumed, .b = false };
    }

    auto [num, num_consumed, num_ok] = to_integer(str, str_len, 10, '/', divptr);
    consumed = num_consumed;
    if ( !num_ok ) {
        return { .v = result, .s = consumed, .b = false };
    }
    consumed = divptr + 1 - str;  // complete up until '/'

    // int64_t denom;  // 0x7ffc7090d904 != 0x7ffc7090d907 " 10 / 1000000 "
    auto [denom, denom_consumed, denom_ok] = to_integer(divptr + 1, str_len - (divptr - str) - 1, 10, '\0', str + str_len);
    consumed += denom_consumed;

    fraction_i64 temp(num, (uint64_t)denom);
    if ( !(min_allowed <= temp && temp <= max_allowed) ) {
        // invalid user value range
        if constexpr ( _debug ) {
            INFO_PRINT("Numerator out of range, not %s <= %s <= %s, in: '%s'", min_allowed.toString().c_str(), temp.toString().c_str(), max_allowed.toString().c_str(), str);
        }
        return { .v = result, .s = consumed, .b = false };
    }
    result = temp;
    return { .v = result, .s = consumed, .b = denom_ok };
}

std::string jau::math::to_string(const jau::math::math_error_t v) noexcept {
    switch(v) {
        case jau::math::math_error_t::none: return "none";
        case jau::math::math_error_t::invalid: return "invalid";
        case jau::math::math_error_t::div_by_zero: return "div_by_zero";
        case jau::math::math_error_t::overflow: return "overflow";
        case jau::math::math_error_t::underflow: return "underflow";
        case jau::math::math_error_t::inexact: return "inexact";
        case jau::math::math_error_t::undefined: return "undefined";
    }
    return "undef";
}

std::string jau::type_info::toString() const noexcept {
    using namespace jau::enums;

    std::string r("TypeInfo[addr ");
    r.append(jau::toHexString(this))
     .append(", hash ").append(jau::toHexString(hash_code()))
     .append(", `").append(name())
     .append("`, ident").append(to_string(m_idflags));
    r.append("]]");
    return r;
}
