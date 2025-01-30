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

#include <jau/byte_util.hpp>
#include <jau/enum_util.hpp>
#include <string>

namespace jau::cpu {

    using namespace jau::enums;

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
    constexpr size_t pointer_bit_size() noexcept { return sizeof(void*) * 8; }

    enum class cpu_family_t : uint16_t {
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
        ppc32 = 20,
        /** Power PC 32bit */
        ppc64 = 21,

        /** SPARC 32bit */
        sparc32 = 30,
        /** SPARC 32bit */
        sparc64 = 31,

        /** Mips 32bit */
        mips32 = 40,
        /** Mips 64bit */
        mips64 = 41,

        /** Hitachi SuperH 32bit */
        superh32 = 50,
        /** Hitachi SuperH 64bit */
        superh64 = 51,

        /** WebAssembly 32-bit */
        wasm32 = 60,
        /** WebAssembly 64-bit */
        wasm64 = 61

    };
    JAU_MAKE_ENUM_STRING(cpu_family_t, arm32, arm64, x86_32, x86_64, ia64, ppc32, ppc64, sparc32, sparc64, mips32, mips64, superh32, superh64, wasm32, wasm64);

    enum class arm32_hwcap1_t : uint64_t {
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
    JAU_MAKE_BITFIELD_ENUM_STRING(arm32_hwcap1_t, swp, half, thumb, bits26, fmult, fpa, vfp, edsp, java, iwmmxt,
                                                crunch, thumbee, neon, vfp_v3, vfp_v3_d16, tls, vfp_v4, idiva,
                                                idivt, vfp_d32, lpae, evtstrm);

    enum class arm32_hwcap2_t : uint64_t {
        none  = 0,
        aes   = (1 << 0),
        pmull = (1 << 1),
        sha1  = (1 << 2),
        sha2  = (1 << 3),
        crc32 = (1 << 4),

        at_hwcap_2 = 26
    };
    JAU_MAKE_BITFIELD_ENUM_STRING(arm32_hwcap2_t, aes, pmull, sha1, sha2, crc32);

    enum class arm64_hwcap_t : uint64_t {
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
    JAU_MAKE_BITFIELD_ENUM_STRING(arm64_hwcap_t, fp, asimd, evtstrm, aes, pmull, sha1, sha2, crc32, atomics, fphp,
                                               asimdhp, cpuid, asimdrdm, jscvt, fcma, lrcpc, dcpop, sha3, sm3, sm4,
                                               asimddp, sha512, sve, asimdfhm, dit, uscat, ilrcpc, flagm, ssbs, sb, paca, pacg);

    /** Singleton CpuInfo caching all jau::cpu information */
    class CpuInfo {
        public:
            /** See pointer_bit_size() */
            size_t pointer_bits;
            /** Size of a page in bytes or zero if not available */
            size_t page_size;
            /** True if successfully queried l1_share_max and l1_apart_min. */
            bool has_l1_minmax;
            /** Maximum size of contiguous memory to promote true sharing if has_l1_minmax, or zero */
            size_t l1_share_max;
            /** Minimum offset between two objects to avoid false sharing if has_l1_minmax, or zero */
            size_t l1_apart_min;
            /** Number of available concurrent threads (cores) or zero if information is not available, using C++11 std::thread::hardware_concurrency() */
            size_t concurrent_threads;
            /** Number of available/online cores from system call */
            size_t sys_online_cores;
            /** Number of installed/configured cores from system call */
            size_t sys_max_cores;
            /** cpu_family_t derived from [Architectures](https://sourceforge.net/p/predef/wiki/Architectures/) predefined compiler macros. */
            cpu_family_t family;
            jau::endian_t byte_order;
            /** True if successfully queried arm32_hwcap1 and arm32_hwcap1 on cpu_family_t::arm32. */
            bool has_arm32_hwcap;
            /** arm32_hwcap1_t info if available, i.e. has_arm32_hwcap */
            arm32_hwcap1_t arm32_hwcap1;
            /** arm32_hwcap2_t info if available, i.e. has_arm32_hwcap */
            arm32_hwcap2_t arm32_hwcap2;
            /** True if successfully queried arm64_hwcap on cpu_family_t::arm64. */
            bool has_arm64_hwcap;
            /** arm64_hwcap_t info if available, i.e. has_arm64_hwcap */
            arm64_hwcap_t arm64_hwcap;

        private:
            CpuInfo() noexcept;

        public:
            CpuInfo(const CpuInfo&) = delete;
            void operator=(const CpuInfo&) = delete;

            /** Returns reference to const singleton instance */
            static inline const CpuInfo& get() noexcept {
                static CpuInfo ci;
                return ci;
            }

            /** Returns maximum number of available/online cores, i.e. max(sys_online_cores, concurrent_threads). */
            inline size_t online_core_count() const noexcept {
                return std::max(sys_online_cores, concurrent_threads);
            }

            std::string toString(std::string& sb, bool details_only=false) const noexcept;
            std::string toString() const noexcept {
                std::string sb; toString(sb); return sb;
            }
    };

    inline std::string get_cpu_info(std::string& sb) noexcept {
        return CpuInfo::get().toString(sb);
    }
    inline std::string get_cpu_info() noexcept {
        std::string sb; CpuInfo::get().toString(sb); return sb;
    }

    /**@}*/

}  // namespace jau::cpu

#endif /* JAU_CPUID_HPP_ */

