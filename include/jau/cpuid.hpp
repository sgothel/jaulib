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

#ifndef JAU_CPUID_HPP_
#define JAU_CPUID_HPP_

#include <string>

namespace jau::cpu {

    /** \addtogroup SysUtils
     *
     *  @{
     */

    /**
     * Returns the compile time pointer architecture size in bits.
     * e.g. 64-bit for LP64 and 32-bit for LP32.
     *
     * Implementations uses `sizeof(void*)`, i.e. the address bus size,
     * the common denominator across all LP64, ILP64 and LLP64 for 64-bit.
     */
    constexpr size_t get_arch_psize() noexcept { return sizeof(void*) * 8; }

    enum class cpu_family : uint16_t {
        /** Undefined */
        none = 0,

        /** ARM 32bit */
        arm32 = 1,
        /** ARM 64bit */
        arm64 = 2,

        /** AMD/Intel 32-bit */
        x86_32 = 10,
        /** AMD/Intel 64-bit */
        x86_64 = 11,
        /** Itanium */
        ia64 = 12,

        /** Power PC 32bit */
        ppc_32 = 20,
        /** Power PC 32bit */
        ppc_64 = 21,

        /** SPARC 32bit */
        sparc_32 = 30,
        /** SPARC 32bit */
        sparc_64 = 31,

        /** Mips 32bit */
        mips_32 = 40,
        /** Mips 64bit */
        mips_64 = 41,

        /** Hitachi SuperH 32bit */
        superh_32 = 50,
        /** Hitachi SuperH 64bit */
        superh_64 = 51,

        /** WebAssembly 32-bit */
        wasm_32 = 60,
        /** WebAssembly 64-bit */
        wasm_64 = 61

    };
    constexpr uint16_t number(const cpu_family rhs) noexcept {
        return static_cast<uint16_t>(rhs);
    }
    constexpr cpu_family operator ~(const cpu_family rhs) noexcept {
        return static_cast<cpu_family> ( ~number(rhs) );
    }
    constexpr cpu_family operator ^(const cpu_family lhs, const cpu_family rhs) noexcept {
        return static_cast<cpu_family> ( number(lhs) ^ number(rhs) );
    }
    constexpr cpu_family operator |(const cpu_family lhs, const cpu_family rhs) noexcept {
        return static_cast<cpu_family> ( number(lhs) | number(rhs) );
    }
    constexpr cpu_family operator &(const cpu_family lhs, const cpu_family rhs) noexcept {
        return static_cast<cpu_family> ( number(lhs) & number(rhs) );
    }
    constexpr cpu_family& operator |=(cpu_family& lhs, const cpu_family rhs) noexcept {
        lhs = static_cast<cpu_family> ( number(lhs) | number(rhs) );
        return lhs;
    }
    constexpr cpu_family& operator &=(cpu_family& lhs, const cpu_family rhs) noexcept {
        lhs = static_cast<cpu_family> ( number(lhs) & number(rhs) );
        return lhs;
    }
    constexpr cpu_family& operator ^=(cpu_family& lhs, const cpu_family rhs) noexcept {
        lhs = static_cast<cpu_family> ( number(lhs) ^ number(rhs) );
        return lhs;
    }
    constexpr bool operator ==(const cpu_family lhs, const cpu_family rhs) noexcept {
        return number(lhs) == number(rhs);
    }
    constexpr bool operator !=(const cpu_family lhs, const cpu_family rhs) noexcept {
        return !( lhs == rhs );
    }
    constexpr bool is_set(const cpu_family mask, const cpu_family bit) noexcept {
        return bit == ( mask & bit );
    }
    /** Returns cpu_family derived from [Architectures](https://sourceforge.net/p/predef/wiki/Architectures/) predefined compiler macros. */
    constexpr cpu_family get_cpu_family() noexcept {
        #if defined(__EMSCRIPTEN__)
            static_assert( 32 == get_arch_psize() );
            return cpu_family::wasm_32;
        #elif defined(__aarch64__)
            static_assert( 64 == get_arch_psize() );
            return cpu_family::arm64;
        #elif defined(__arm__)
            static_assert( 32 == get_arch_psize() );
            return cpu_family::arm32;
        #elif defined(__x86_64__)
            static_assert( 64 == get_arch_psize() );
            return cpu_family::x86_64;
        #elif defined(__ia64__)
            static_assert( 64 == get_arch_psize() );
            return cpu_family::ia64;
        #elif defined(__i386__)
            static_assert( 32 == get_arch_psize() );
            return cpu_family::X86_32;
        #elif defined(__powerpc__)
            #if defined(__LP64__)
                static_assert( 64 == get_arch_psize() );
                return cpu_family::ppc_64;
            #else
                static_assert( 32 == get_arch_psize() );
                return cpu_family::ppc_32;
            #endif
        #elif defined(__sparc__)
            #if defined(__LP64__)
                static_assert( 64 == get_arch_psize() );
                return cpu_family::sparc_64;
            #else
                static_assert( 32 == get_arch_psize() );
                return cpu_family::sparc_32;
            #endif
        #elif defined(__mips__)
            #if defined(__LP64__)
                static_assert( 64 == get_arch_psize() );
                return cpu_family::mips_64;
            #else
                static_assert( 32 == get_arch_psize() );
                return cpu_family::mips_32;
            #endif
        #elif defined(__sh__)
            #if defined(__LP64__)
                static_assert( 64 == get_arch_psize() );
                return cpu_family::superh_64;
            #else
                static_assert( 32 == get_arch_psize() );
                return cpu_family::superh_32;
            #endif
        #else
            return cpu_family::UNDEF;
        #endif
    }

    std::string to_string(const cpu_family v) noexcept;

    enum class arm32_hwcap1 : uint64_t {
        none         = 0,
        swp          = (1 << 0),
        half         = (1 << 1),
        thumb        = (1 << 2),
        bits26       = (1 << 3),
        fmult        = (1 << 4),
        fpa          = (1 << 5),
        vfp          = (1 << 6),
        edsp         = (1 << 7),
        java         = (1 << 8),
        iwmmxt       = (1 << 9),
        crunch       = (1 << 10),
        thumbee      = (1 << 11),
        neon         = (1 << 12),
        vfp_v3       = (1 << 13),
        vfp_v3_d16   = (1 << 14),
        tls          = (1 << 15),
        vfp_v4       = (1 << 16),
        idiva        = (1 << 17),
        idivt        = (1 << 18),
        vfp_d32      = (1 << 19),
        lpae         = (1 << 20),
        evtstrm      = (1 << 21),

        at_hwcap_1   = 16
    };
    constexpr uint64_t number(const arm32_hwcap1 rhs) noexcept {
        return static_cast<uint64_t>(rhs);
    }
    constexpr arm32_hwcap1 operator ~(const arm32_hwcap1 rhs) noexcept {
        return static_cast<arm32_hwcap1> ( ~number(rhs) );
    }
    constexpr arm32_hwcap1 operator ^(const arm32_hwcap1 lhs, const arm32_hwcap1 rhs) noexcept {
        return static_cast<arm32_hwcap1> ( number(lhs) ^ number(rhs) );
    }
    constexpr arm32_hwcap1 operator |(const arm32_hwcap1 lhs, const arm32_hwcap1 rhs) noexcept {
        return static_cast<arm32_hwcap1> ( number(lhs) | number(rhs) );
    }
    constexpr arm32_hwcap1 operator &(const arm32_hwcap1 lhs, const arm32_hwcap1 rhs) noexcept {
        return static_cast<arm32_hwcap1> ( number(lhs) & number(rhs) );
    }
    constexpr arm32_hwcap1& operator |=(arm32_hwcap1& lhs, const arm32_hwcap1 rhs) noexcept {
        lhs = static_cast<arm32_hwcap1> ( number(lhs) | number(rhs) );
        return lhs;
    }
    constexpr arm32_hwcap1& operator &=(arm32_hwcap1& lhs, const arm32_hwcap1 rhs) noexcept {
        lhs = static_cast<arm32_hwcap1> ( number(lhs) & number(rhs) );
        return lhs;
    }
    constexpr arm32_hwcap1& operator ^=(arm32_hwcap1& lhs, const arm32_hwcap1 rhs) noexcept {
        lhs = static_cast<arm32_hwcap1> ( number(lhs) ^ number(rhs) );
        return lhs;
    }
    constexpr bool operator ==(const arm32_hwcap1 lhs, const arm32_hwcap1 rhs) noexcept {
        return number(lhs) == number(rhs);
    }
    constexpr bool operator !=(const arm32_hwcap1 lhs, const arm32_hwcap1 rhs) noexcept {
        return !( lhs == rhs );
    }
    constexpr bool is_set(const arm32_hwcap1 mask, const arm32_hwcap1 bit) noexcept {
        return bit == ( mask & bit );
    }
    std::string to_string(const arm32_hwcap1 hwcaps) noexcept;

    enum class arm32_hwcap2 : uint64_t {
        none  = 0,
        aes   = (1 << 0),
        pmull = (1 << 1),
        sha1  = (1 << 2),
        sha2  = (1 << 3),
        crc32 = (1 << 4),

        at_hwcap_2 = 26
    };
    constexpr uint64_t number(const arm32_hwcap2 rhs) noexcept {
        return static_cast<uint64_t>(rhs);
    }
    constexpr arm32_hwcap2 operator ~(const arm32_hwcap2 rhs) noexcept {
        return static_cast<arm32_hwcap2> ( ~number(rhs) );
    }
    constexpr arm32_hwcap2 operator ^(const arm32_hwcap2 lhs, const arm32_hwcap2 rhs) noexcept {
        return static_cast<arm32_hwcap2> ( number(lhs) ^ number(rhs) );
    }
    constexpr arm32_hwcap2 operator |(const arm32_hwcap2 lhs, const arm32_hwcap2 rhs) noexcept {
        return static_cast<arm32_hwcap2> ( number(lhs) | number(rhs) );
    }
    constexpr arm32_hwcap2 operator &(const arm32_hwcap2 lhs, const arm32_hwcap2 rhs) noexcept {
        return static_cast<arm32_hwcap2> ( number(lhs) & number(rhs) );
    }
    constexpr arm32_hwcap2& operator |=(arm32_hwcap2& lhs, const arm32_hwcap2 rhs) noexcept {
        lhs = static_cast<arm32_hwcap2> ( number(lhs) | number(rhs) );
        return lhs;
    }
    constexpr arm32_hwcap2& operator &=(arm32_hwcap2& lhs, const arm32_hwcap2 rhs) noexcept {
        lhs = static_cast<arm32_hwcap2> ( number(lhs) & number(rhs) );
        return lhs;
    }
    constexpr arm32_hwcap2& operator ^=(arm32_hwcap2& lhs, const arm32_hwcap2 rhs) noexcept {
        lhs = static_cast<arm32_hwcap2> ( number(lhs) ^ number(rhs) );
        return lhs;
    }
    constexpr bool operator ==(const arm32_hwcap2 lhs, const arm32_hwcap2 rhs) noexcept {
        return number(lhs) == number(rhs);
    }
    constexpr bool operator !=(const arm32_hwcap2 lhs, const arm32_hwcap2 rhs) noexcept {
        return !( lhs == rhs );
    }
    constexpr bool is_set(const arm32_hwcap2 mask, const arm32_hwcap2 bit) noexcept {
        return bit == ( mask & bit );
    }
    std::string to_string(const arm32_hwcap2 hwcaps) noexcept;

    bool get_arm32_hwcap(arm32_hwcap1& hwcap1, arm32_hwcap2& hwcap2) noexcept;

    enum class arm64_hwcap : uint64_t {
        none     = 0,
        fp       = (1 << 0),
        asimd    = (1 << 1),
        evtstrm  = (1 << 2),
        aes      = (1 << 3),
        pmull    = (1 << 4),
        sha1     = (1 << 5),
        sha2     = (1 << 6),
        crc32    = (1 << 7),
        atomics  = (1 << 8),
        fphp     = (1 << 9),
        asimdhp  = (1 << 10),
        cpuid    = (1 << 11),
        asimdrdm = (1 << 12),
        jscvt    = (1 << 13),
        fcma     = (1 << 14),
        lrcpc    = (1 << 15),
        dcpop    = (1 << 16),
        sha3     = (1 << 17),
        sm3      = (1 << 18),
        sm4      = (1 << 19),
        asimddp  = (1 << 20),
        sha512   = (1 << 21),
        sve      = (1 << 22),
        asimdfhm = (1 << 23),
        dit      = (1 << 24),
        uscat    = (1 << 25),
        ilrcpc   = (1 << 26),
        flagm    = (1 << 27),
        ssbs     = (1 << 28),
        sb       = (1 << 29),
        paca     = (1 << 30),
        pacg     = (1UL << 31),

        at_hwcap = 16
    };
    constexpr uint64_t number(const arm64_hwcap rhs) noexcept {
        return static_cast<uint64_t>(rhs);
    }
    constexpr arm64_hwcap operator ~(const arm64_hwcap rhs) noexcept {
        return static_cast<arm64_hwcap> ( ~number(rhs) );
    }
    constexpr arm64_hwcap operator ^(const arm64_hwcap lhs, const arm64_hwcap rhs) noexcept {
        return static_cast<arm64_hwcap> ( number(lhs) ^ number(rhs) );
    }
    constexpr arm64_hwcap operator |(const arm64_hwcap lhs, const arm64_hwcap rhs) noexcept {
        return static_cast<arm64_hwcap> ( number(lhs) | number(rhs) );
    }
    constexpr arm64_hwcap operator &(const arm64_hwcap lhs, const arm64_hwcap rhs) noexcept {
        return static_cast<arm64_hwcap> ( number(lhs) & number(rhs) );
    }
    constexpr arm64_hwcap& operator |=(arm64_hwcap& lhs, const arm64_hwcap rhs) noexcept {
        lhs = static_cast<arm64_hwcap> ( number(lhs) | number(rhs) );
        return lhs;
    }
    constexpr arm64_hwcap& operator &=(arm64_hwcap& lhs, const arm64_hwcap rhs) noexcept {
        lhs = static_cast<arm64_hwcap> ( number(lhs) & number(rhs) );
        return lhs;
    }
    constexpr arm64_hwcap& operator ^=(arm64_hwcap& lhs, const arm64_hwcap rhs) noexcept {
        lhs = static_cast<arm64_hwcap> ( number(lhs) ^ number(rhs) );
        return lhs;
    }
    constexpr bool operator ==(const arm64_hwcap lhs, const arm64_hwcap rhs) noexcept {
        return number(lhs) == number(rhs);
    }
    constexpr bool operator !=(const arm64_hwcap lhs, const arm64_hwcap rhs) noexcept {
        return !( lhs == rhs );
    }
    constexpr bool is_set(const arm64_hwcap mask, const arm64_hwcap bit) noexcept {
        return bit == ( mask & bit );
    }
    std::string to_string(const arm64_hwcap hwcaps) noexcept;

    bool get_arm64_hwcap(arm64_hwcap& hwcap) noexcept;

    std::string get_cpu_info(const std::string& line_prefix, std::string& sb) noexcept;
    inline std::string get_cpu_info() noexcept {
        std::string sb; get_cpu_info("cpu info: ", sb); return sb;
    }

    /**@}*/

}  // namespace jau::cpu

#endif /* JAU_CPUID_HPP_ */

