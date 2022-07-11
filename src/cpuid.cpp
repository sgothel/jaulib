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
#include <cstdint>
#include <cstdlib>

#include <jau/cpuid.hpp>
#include <jau/byte_util.hpp>
#include <jau/debug.hpp>

#ifdef __linux__
    extern "C" {
        #include <sys/auxv.h>
    }
#endif

using namespace jau;
using namespace jau::cpu;

template<typename T>
static void append_bitstr(std::string& out, T mask, T bit, const std::string& bitstr, bool& comma) {
    if( bit == ( mask & bit ) ) {
        if( comma ) { out.append(", "); }
        out.append(bitstr); comma = true;
    }
}
#define APPEND_BITSTR(U,V,M) append_bitstr(out, M, U::V, #V, comma);

cpu_family jau::cpu::get_cpu_family() noexcept {
    #if defined(__i386__)
        return cpu_family::X86_32;
    #elif defined(__x86_64__)
        return cpu_family::x86_64;
    #elif defined(__arm__)
        return cpu_family::arm32;
    #elif defined(__aarch64__)
        return cpu_family::arm64;
    #else
        return cpu_family::UNDEF;
    #endif
}

#define CASE_TO_STRING(U,V) case U::V: return #V;

#define CPUFAMILY_ENUM(X) \
    X(cpu_family,none) \
    X(cpu_family,x86_32) \
    X(cpu_family,x86_64) \
    X(cpu_family,arm32) \
    X(cpu_family,arm64) \
    X(cpu_family,ppc) \
    X(cpu_family,sparc) \
    X(cpu_family,mips) \
    X(cpu_family,pa_risc) \
    X(cpu_family,ia64) \
    X(cpu_family,superh)

std::string jau::cpu::to_string(const cpu_family v) noexcept {
    switch(v) {
    CPUFAMILY_ENUM(CASE_TO_STRING)
        default: ; // fall through intended
    }
    return "none";
}


#define ARM32HWCAP1_ENUM(X,M) \
    X(arm32_hwcap1,swp,M) \
    X(arm32_hwcap1,half,M) \
    X(arm32_hwcap1,thumb,M) \
    X(arm32_hwcap1,bits26,M) \
    X(arm32_hwcap1,fmult,M) \
    X(arm32_hwcap1,fpa,M) \
    X(arm32_hwcap1,vfp,M) \
    X(arm32_hwcap1,edsp,M) \
    X(arm32_hwcap1,java,M) \
    X(arm32_hwcap1,iwmmxt,M) \
    X(arm32_hwcap1,crunch,M) \
    X(arm32_hwcap1,thumbee,M) \
    X(arm32_hwcap1,neon,M) \
    X(arm32_hwcap1,vfp_v3,M) \
    X(arm32_hwcap1,vfp_v3_d16,M) \
    X(arm32_hwcap1,tls,M) \
    X(arm32_hwcap1,vfp_v4,M) \
    X(arm32_hwcap1,idiva,M) \
    X(arm32_hwcap1,idivt,M) \
    X(arm32_hwcap1,vfp_d32,M) \
    X(arm32_hwcap1,lpae,M) \
    X(arm32_hwcap1,evtstrm,M)

std::string jau::cpu::to_string(const arm32_hwcap1 hwcaps) noexcept {
    std::string out;
    bool comma = false;
    ARM32HWCAP1_ENUM(APPEND_BITSTR,hwcaps)
    return out;
}

#define ARM32HWCAP2_ENUM(X,M) \
    X(arm32_hwcap2,aes,M) \
    X(arm32_hwcap2,pmull,M) \
    X(arm32_hwcap2,sha1,M) \
    X(arm32_hwcap2,sha2,M) \
    X(arm32_hwcap2,crc32,M)

std::string jau::cpu::to_string(const arm32_hwcap2 hwcaps) noexcept {
    std::string out;
    bool comma = false;
    ARM32HWCAP2_ENUM(APPEND_BITSTR,hwcaps)
    return out;
}

#define ARM64HWCAP_ENUM(X,M) \
    X(arm64_hwcap,fp,M) \
    X(arm64_hwcap,asimd,M) \
    X(arm64_hwcap,evtstrm,M) \
    X(arm64_hwcap,aes,M) \
    X(arm64_hwcap,pmull,M) \
    X(arm64_hwcap,sha1,M) \
    X(arm64_hwcap,sha2,M) \
    X(arm64_hwcap,crc32,M) \
    X(arm64_hwcap,atomics,M) \
    X(arm64_hwcap,fphp,M) \
    X(arm64_hwcap,asimdhp,M) \
    X(arm64_hwcap,cpuid,M) \
    X(arm64_hwcap,asimdrdm,M) \
    X(arm64_hwcap,jscvt,M) \
    X(arm64_hwcap,fcma,M) \
    X(arm64_hwcap,lrcpc,M) \
    X(arm64_hwcap,dcpop,M) \
    X(arm64_hwcap,sha3,M) \
    X(arm64_hwcap,sm3,M) \
    X(arm64_hwcap,sm4,M) \
    X(arm64_hwcap,asimddp,M) \
    X(arm64_hwcap,sha512,M) \
    X(arm64_hwcap,sve,M) \
    X(arm64_hwcap,asimdfhm,M) \
    X(arm64_hwcap,dit,M) \
    X(arm64_hwcap,uscat,M) \
    X(arm64_hwcap,ilrcpc,M) \
    X(arm64_hwcap,flagm,M) \
    X(arm64_hwcap,ssbs,M) \
    X(arm64_hwcap,sb,M) \
    X(arm64_hwcap,paca,M) \
    X(arm64_hwcap,pacg,M)

std::string jau::cpu::to_string(const arm64_hwcap hwcaps) noexcept {
    std::string out;
    bool comma = false;
    ARM64HWCAP_ENUM(APPEND_BITSTR,hwcaps)
    return out;
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

bool jau::cpu::get_arm32_hwcap(arm32_hwcap1& hwcap1, arm32_hwcap2& hwcap2) noexcept {
#ifdef __linux__
    if( cpu_family::arm32 == get_cpu_family() ) {
        hwcap1 = (arm32_hwcap1) ::getauxval(number(arm32_hwcap1::at_hwcap_1));
        if( arm32_hwcap1::neon == ( hwcap1 & arm32_hwcap1::neon ) ) {
            hwcap2 = (arm32_hwcap2) ::getauxval(number(arm32_hwcap2::at_hwcap_2));
        } else {
            hwcap2 = arm32_hwcap2::none;
        }
        return true;
    } else {
        hwcap1 = arm32_hwcap1::none;
        hwcap2 = arm32_hwcap2::none;
        return false;
    }
#else
    hwcap1 = arm32_hwcap1::none;
    hwcap2 = arm32_hwcap2::none;
    return false;
#endif
}

bool jau::cpu::get_arm64_hwcap(arm64_hwcap& hwcap) noexcept {
#ifdef __linux__
    if( cpu_family::arm64 == get_cpu_family() ) {
        hwcap = (arm64_hwcap) ::getauxval(number(arm64_hwcap::at_hwcap));
        return true;
    } else {
        hwcap = arm64_hwcap::none;
        return false;
    }
#else
    hwcap = arm64_hwcap::none;
    return false;
#endif
}

void jau::cpu::print_cpu_info(FILE* stream) noexcept {
    cpu_family cpu = get_cpu_family();

    jau::fprintf_td(stream, "cpu info: family '%s', endian '%s', arch-pointer-bits %zu\n",
            to_string(cpu).c_str(), to_string(endian::native).c_str(), get_arch_psize());

    if( cpu_family::arm32 == cpu ) {
        arm32_hwcap1 hwcap1;
        arm32_hwcap2 hwcap2;
        if( get_arm32_hwcap(hwcap1, hwcap2) ) {
            jau::fprintf_td(stream, "cpu info: hwcap1 0x%" PRIx64 ": '%s'\n", number(hwcap1), to_string(hwcap1).c_str());
            if( arm32_hwcap2::none != hwcap2 ) {
                jau::fprintf_td(stream, "cpu info: hwcap2 0x%" PRIx64 ": '%s'\n", number(hwcap2), to_string(hwcap2).c_str());
            }
        }
    } else if( cpu_family::arm64 == cpu ) {
        arm64_hwcap hwcap;
        if( get_arm64_hwcap(hwcap) ) {
            jau::fprintf_td(stream, "cpu info: hwcap 0x%" PRIx64 ": '%s'\n", number(hwcap), to_string(hwcap).c_str());
        }
    }
}
