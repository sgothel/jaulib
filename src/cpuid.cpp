/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2022 Gothel Software e.K.
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

#include <cstring>
#include <string>
#include <cstdlib>
#include <thread>

#include <jau/cpuid.hpp>
#include <jau/byte_util.hpp>
#include <jau/debug.hpp>
#include <jau/os/os_support.hpp>

#if defined(_WIN32)
    #include <windows.h>
#else /* assume POSIX sysconf() availability */
    #include <unistd.h>
    #ifdef __linux__
        extern "C" {
            #include <sys/auxv.h>
        }
    #endif
#endif

using namespace jau;
using namespace jau::cpu;

static bool get_cache_line_size(size_t& l1_share_max, size_t& l1_apart_min) noexcept {
    #ifdef __cpp_lib_hardware_interference_size
        l1_share_max = std::hardware_constructive_interference_size;
        l1_apart_min = std::hardware_destructive_interference_size;
        return true;
    #else
        l1_share_max = 0;
        l1_apart_min = 0;
        return false;
    #endif
}

static size_t get_page_size() noexcept {
    #if defined(_WIN32)
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        return (jlong) si.dwPageSize;
    #elif defined(JAU_OS_TYPE_UNIX) && defined(_SC_PAGESIZE)
        return sysconf(_SC_PAGESIZE);
    #else
        return 0;
    #endif
}
static size_t get_concurrent_thread_count() noexcept {
    return std::thread::hardware_concurrency();
}

static size_t get_sys_online_core_count() noexcept {
    #if defined(_WIN32)
        SYSTEM_INFO sysinfo;
        GetSystemInfo(&sysinfo);
        return static_cast<size_t>( sysinfo.dwNumberOfProcessors );
    #elif defined(JAU_OS_TYPE_UNIX) && defined(_SC_NPROCESSORS_ONLN)
        return static_cast<size_t>( sysconf(_SC_NPROCESSORS_ONLN) );
    #elif defined(JAU_OS_TYPE_UNIX) && defined(HW_NCPU)
        int mib[] { CTL_HW, HW_NCPU }; // set the mib for hw.ncpu
        int numCPU;
        size_t len = sizeof(numCPU);
        if( sysctl(mib, 2, &numCPU, &len, NULL, 0) || 0 >= numCPU ) {
            return 1; // minimum ;-)
        } else {
            return static_cast<size_t>(numCPU);
        }
    #else
        return 1; // minimum ;-)
    #endif
}

static size_t get_sys_max_core_count() noexcept {
    #if defined(_WIN32)
        SYSTEM_INFO sysinfo;
        GetSystemInfo(&sysinfo);
        return static_cast<size_t>( sysinfo.dwNumberOfProcessors ); // FIXME?
    #elif defined(JAU_OS_TYPE_UNIX) && defined(_SC_NPROCESSORS_CONF)
        return static_cast<size_t>( sysconf(_SC_NPROCESSORS_CONF) );
    #elif defined(JAU_OS_TYPE_UNIX) && defined(HW_NCPU)
        int mib[] { CTL_HW, HW_AVAILCPU }; // set the mib for hw.availcpu FIXME?
        int numCPU;
        size_t len = sizeof(numCPU);
        if( sysctl(mib, 2, &numCPU, &len, NULL, 0) || 0 >= numCPU ) {
            return 1; // minimum ;-)
        } else {
            return static_cast<size_t>(numCPU);
        }
    #else
        return 1; // minimum ;-)
    #endif
}

template<typename T>
static void append_bitstr(std::string& out, T mask, T bit, const std::string& bitstr, bool& comma) {
    if( bit == ( mask & bit ) ) {
        if( comma ) { out.append(", "); }
        out.append(bitstr); comma = true;
    }
}

/** Returns cpu_family derived from [Architectures](https://sourceforge.net/p/predef/wiki/Architectures/) predefined compiler macros. Consider using singleton CpuInfo. */
static cpu_family_t get_cpu_family() noexcept {
    #if defined(__EMSCRIPTEN__)
        if( 32 == pointer_bit_size() ) {
            return cpu_family_t::wasm32;
        } else if( 64 == pointer_bit_size() ) {
            return cpu_family_t::wasm64;
        } else {
            return cpu_family_t::wasm64; // FIXME?
        }
    #elif defined(__aarch64__)
        static_assert( 64 == pointer_bit_size() );
        return cpu_family_t::arm64;
    #elif defined(__arm__)
        static_assert( 32 == pointer_bit_size() );
        return cpu_family_t::arm32;
    #elif defined(__x86_64__)
        static_assert( 64 == pointer_bit_size() );
        return cpu_family_t::x86_64;
    #elif defined(__ia64__)
        static_assert( 64 == pointer_bit_size() );
        return cpu_family_t::ia64;
    #elif defined(__i386__)
        static_assert( 32 == pointer_bit_size() );
        return cpu_family_t::X86_32;
    #elif defined(__powerpc__)
        #if defined(__LP64__)
            static_assert( 64 == pointer_bit_size() );
            return cpu_family_t::ppc64;
        #else
            static_assert( 32 == pointer_bit_size() );
            return cpu_family_t::ppc32;
        #endif
    #elif defined(__sparc__)
        #if defined(__LP64__)
            static_assert( 64 == pointer_bit_size() );
            return cpu_family_t::sparc64;
        #else
            static_assert( 32 == pointer_bit_size() );
            return cpu_family_t::sparc32;
        #endif
    #elif defined(__mips__)
        #if defined(__LP64__)
            static_assert( 64 == pointer_bit_size() );
            return cpu_family_t::mips64;
        #else
            static_assert( 32 == pointer_bit_size() );
            return cpu_family_t::mips32;
        #endif
    #elif defined(__sh__)
        #if defined(__LP64__)
            static_assert( 64 == pointer_bit_size() );
            return cpu_family_t::superh64;
        #else
            static_assert( 32 == pointer_bit_size() );
            return cpu_family_t::superh32;
        #endif
    #else
        return cpu_family_t::UNDEF;
    #endif
}

#if 0
#if defined(__i386__) || defined(__x86_64__)

#if defined(_MSC_VER)
  #include <intrin.h>
#elif defined(__INTEL__)
  #include <ia32intrin.h>
#elif defined(__GNUC__) || defined(__clang__)
  #include <cpuid.h>
#endif

static void invoke_cpuid(uint32_t type, uint32_t out[4]) {
#if defined(_MSC_VER) || defined(__INTEL__)
   __cpuid((int*)out, type);
#elif defined(__GNUC__) || defined(__clang__)
   __get_cpuid(type, out, out+1, out+2, out+3);
#else
   #warning "No x86 cpuid supported for this compiler"
   ::bzero(out, sizeof(uint32_t)*4);
#endif
}

static void invoke_cpuid_sublevel(uint32_t type, uint32_t level, uint32_t out[4]) {
#if defined(_MSC_VER)
   __cpuidex((int*)out, type, level);
#elif defined(__GNUC__) || defined(__clang__)
   __cpuid_count(type, level, out[0], out[1], out[2], out[3]);
#else
   #warning "No x86 cpuid sublevel supported for this compiler"
   ::bzero(out, sizeof(uint32_t)*4);
#endif
}

#endif // #if defined(__i386__) || defined(__x86_64__)
#endif // disabled

static bool get_arm32_hwcap(arm32_hwcap1_t& hwcap1, arm32_hwcap2_t& hwcap2) noexcept {
#ifdef __linux__
    if( cpu_family_t::arm32 == get_cpu_family() ) {
        hwcap1 = (arm32_hwcap1_t) ::getauxval(number(arm32_hwcap1_t::at_hwcap_1));
        if( arm32_hwcap1_t::neon == ( hwcap1 & arm32_hwcap1_t::neon ) ) {
            hwcap2 = (arm32_hwcap2_t) ::getauxval(number(arm32_hwcap2_t::at_hwcap_2));
        } else {
            hwcap2 = arm32_hwcap2_t::none;
        }
        return true;
    } else {
        hwcap1 = arm32_hwcap1_t::none;
        hwcap2 = arm32_hwcap2_t::none;
        return false;
    }
#else
    hwcap1 = arm32_hwcap1_t::none;
    hwcap2 = arm32_hwcap2_t::none;
    return false;
#endif
}

static bool get_arm64_hwcap(arm64_hwcap_t& hwcap) noexcept {
#ifdef __linux__
    if( cpu_family_t::arm64 == get_cpu_family() ) {
        hwcap = (arm64_hwcap_t) ::getauxval(number(arm64_hwcap_t::at_hwcap));
        return true;
    } else {
        hwcap = arm64_hwcap_t::none;
        return false;
    }
#else
    hwcap = arm64_hwcap_t::none;
    return false;
#endif
}

jau::cpu::CpuInfo::CpuInfo() noexcept
: pointer_bits(pointer_bit_size()),
  page_size(get_page_size()),
  has_l1_minmax(false), l1_share_max(0), l1_apart_min(0),
  concurrent_threads(get_concurrent_thread_count()),
  sys_online_cores(get_sys_online_core_count()),
  sys_max_cores(get_sys_max_core_count()),
  family(get_cpu_family()), byte_order(jau::endian_t::native),
  has_arm32_hwcap(cpu_family_t::arm32 == family ? get_arm32_hwcap(arm32_hwcap1, arm32_hwcap2) : false),
  has_arm64_hwcap(cpu_family_t::arm64 == family ? get_arm64_hwcap(arm64_hwcap) : false)
{
    has_l1_minmax = get_cache_line_size(l1_share_max, l1_apart_min);
}

std::string jau::cpu::CpuInfo::toString(std::string& sb, bool details_only) const noexcept {
    if( !details_only ) {
        sb.append(to_string(family)).append(" (");
    }
    sb.append( jau::format_string("%s endian, %zu bits, %zu cores, page-sz %zu",
        to_string(byte_order).c_str(), pointer_bits, online_core_count(), page_size) );
    if( has_l1_minmax ) {
        sb.append( jau::format_string(", l1[shared-max %zu, apart-min %zu]", l1_share_max, l1_apart_min) );
    }
    if( has_arm32_hwcap ) {
        if( arm32_hwcap1_t::none != arm32_hwcap1 ) {
            sb.append( jau::format_string(", hwcap1 0x%" PRIx64 ": '%s'", number(arm32_hwcap1), to_string(arm32_hwcap1).c_str()) );
        }
        if( arm32_hwcap2_t::none != arm32_hwcap2 ) {
            sb.append( jau::format_string(", hwcap2 0x%" PRIx64 ": '%s'", number(arm32_hwcap2), to_string(arm32_hwcap2).c_str()) );
        }
    } else if( has_arm64_hwcap ) {
        if( arm64_hwcap_t::none != arm64_hwcap ) {
            sb.append( jau::format_string(", hwcap 0x%" PRIx64 ": '%s'", number(arm64_hwcap), to_string(arm64_hwcap).c_str()) );
        }
    }
    if( !details_only ) {
        sb.append(")");
    }
    return sb;
}

