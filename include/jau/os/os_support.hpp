/*
 * Copyright (c) 2020 Gothel Software e.K.
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
 *
 */

#ifndef JAU_OS_SUPPORT_HPP_
#define JAU_OS_SUPPORT_HPP_

#include <cstdint>

#include <jau/byte_util.hpp>
#include <jau/int_types.hpp>
#include "jau/cpp_lang_util.hpp"
#include "jau/enum_util.hpp"
#include "jau/cpuid.hpp"

namespace jau::os {

    /** @defgroup OSSup OS Support
     *  OS Support Functionality
     *
     * Available predefined macros denoting the [Operating Systems](https://sourceforge.net/p/predef/wiki/OperatingSystems/)
     * - `__FreeBSD__`      : FreeBSD
     * - `__linux__`        : Linux, w/o Android: `__linux__ && !__ANDROID__`
     * - `__ANDROID__`      : Android, implies `__linux__`
     * - `_WIN32`           : Windows
     * - `_WIN64`           : Windows 64 bit, implies `_WIN32`
     * - `__APPLE__`        : Darwin, i.e. MacOS or iOS
     * - `__ros__`          : Akaros
     * - `__native_client__`: NaCL
     * - `__asmjs__`        : AsmJS
     * - `__EMSCRIPTEN__`   : emscripten for asm.js and WebAssembly
     * - `__Fuchsia__`      : Fuchsia
     *
     * Further infos:
     * - [Unix standards](https://sourceforge.net/p/predef/wiki/Standards/)
     * - [GNU glibc](https://sourceforge.net/p/predef/wiki/Libraries/)
     * - [glibc 1.3.4 Feature Test Macros](https://www.gnu.org/software/libc/manual/html_node/Feature-Test-Macros.html)
     * - [Architectures](https://sourceforge.net/p/predef/wiki/Architectures/)
     *
     *  @{
     */

    namespace impl {
        constexpr uint32_t get_host_os_id() noexcept {
            #if defined(__EMSCRIPTEN__)
                #define JAU_OS_TYPE_UNIX 1
                #define JAU_OS_TYPE_WASM 1
                #if defined(__EMSCRIPTEN_PTHREADS__)
                    #define JAU_OS_HAS_PTHREAD 1
                #else
                    #define JAU_OS_HAS_PTHREAD 0
                #endif
                return 0b00000001000000000000000000000001U; // Emscripten
            #elif defined(__QNXNTO__)
                #define JAU_OS_TYPE_UNIX 1
                #define JAU_OS_TYPE_QNXNTO 1
                #define JAU_OS_HAS_PTHREAD 1
                return 0b00000000000000000001000000000001U; // QnxNTO
            #elif defined(__APPLE__) && defined(__MACH__)
                #define JAU_OS_TYPE_UNIX 1
                #define JAU_OS_TYPE_DARWIN 1
                #define JAU_OS_HAS_PTHREAD 1
                return 0b00000000000000000000100000000001U; // Darwin
            #elif defined(__FreeBSD__)
                #define JAU_OS_TYPE_UNIX 1
                #define JAU_OS_TYPE_FREEBSD 1
                #define JAU_OS_HAS_PTHREAD 1
                return 0b00000000000000000000010000000001U; // FreeBSD
            #elif defined(__ANDROID__)
                #define JAU_OS_TYPE_UNIX 1
                #define JAU_OS_TYPE_ANDROID 1
                #define JAU_OS_HAS_PTHREAD 1
                return 0b00000000000000000000001100000001U; // Android
            #elif defined(__linux__)
                #define JAU_OS_TYPE_UNIX 1
                #define JAU_OS_TYPE_LINUX 1
                #define JAU_OS_HAS_PTHREAD 1
                return 0b00000000000000000000000100000001U; // Linux
            #elif defined(_WIN32)
                #define JAU_OS_TYPE_WINDOWS 1
                #define JAU_OS_HAS_PTHREAD 1
                return 0b00000000000000000000000000000010U; // Windows
            #else
                #define JAU_OS_TYPE_UNIX 1
                #define JAU_OS_HAS_PTHREAD 1
                return 0b00000000000000000000000000000001U; // Unix
            #endif
        }
    }

    using namespace jau::enums;

    /** OS type bits and unique IDs */
    enum class os_type_t : uint32_t {
        /** Unix bit, contained by: linux, android, freebsd, darwin. */
        Unix    = 0b00000000000000000000000000000001U,
        /** Windows bit */
        Windows = 0b00000000000000000000000000000010U,
        /** Linux bit, contained by: android; includes: unix  */
        Linux   = 0b00000000000000000000000100000001U,
        /** Android bit, includes: linux and unix  */
        Android = 0b00000000000000000000001100000001U,
        /** FreeBSD bit, includes: unix  */
        FreeBSD = 0b00000000000000000000010000000001U,
        /** Darwin (Apple OSX and iOS) bit, includes: unix  */
        Darwin  = 0b00000000000000000000100000000001U,
        /** QNX NTO (>= 6) bit, includes: unix  */
        QnxNTO  = 0b00000000000000000001000000000001U,
        /** Generic WebAssembly bit */
        GenWasm = 0b00000001000000000000000000000000U,
        /** WebAssembly with Unix/Posix suport bit (emscripten) */
        Emscripten = 0b00000001000000000000000000000001U,
        /** Identifier for native OS type, one of the above. */
        native      = impl::get_host_os_id()
    };
    JAU_MAKE_BITFIELD_ENUM_STRING(os_type_t, Unix, Windows, Linux, Android, FreeBSD, Darwin, QnxNTO, GenWasm, Emscripten);

    /**
     * Evaluates `true` if the given \ref os_type is defined,
     * i.e. `Unix`, `Windows`, `Linux`, `Android`, ...
     */
    constexpr bool is_defined_os_type(const os_type_t v) noexcept {
        switch(v) {
            case os_type_t::Unix:
                [[fallthrough]];
            case os_type_t::Windows:
                [[fallthrough]];
            case os_type_t::Linux:
                [[fallthrough]];
            case os_type_t::Android:
                [[fallthrough]];
            case os_type_t::FreeBSD:
                [[fallthrough]];
            case os_type_t::Darwin:
                [[fallthrough]];
            case os_type_t::QnxNTO:
                [[fallthrough]];
            case os_type_t::GenWasm:
                return true;
            case os_type_t::Emscripten:
                return true;
            default:
                return false;
        }
    }

    // one static_assert is sufficient for whole compilation unit
    static_assert( is_defined_os_type(os_type_t::native) ); // Enhance os_type to match your platform!

    /** Evaluates `true` if platform os_type::native contains os_type::Unix */
    constexpr bool is_unix() noexcept { return is_set(os_type_t::native, os_type_t::Unix); }

    /** Evaluates `true` if platform os_type::native contains os_type::Windows */
    constexpr bool is_windows() noexcept { return is_set(os_type_t::native, os_type_t::Windows); }

    /** Evaluates `true` if platform os_type::native contains os_type::Linux */
    constexpr bool is_linux() noexcept { return is_set(os_type_t::native, os_type_t::Linux); }

    /** Evaluates `true` if platform os_type::native contains os_type::Android */
    constexpr bool is_android() noexcept { return is_set(os_type_t::native, os_type_t::Android); }

    /** Evaluates `true` if platform os_type::native contains os_type::FreeBSD */
    constexpr bool is_freebsd() noexcept { return is_set(os_type_t::native, os_type_t::FreeBSD); }

    /** Evaluates `true` if platform os_type::native contains os_type::Darwin */
    constexpr bool is_darwin() noexcept { return is_set(os_type_t::native, os_type_t::Darwin); }

    /** Evaluates `true` if platform os_type::native contains os_type::QnxNTO */
    constexpr bool is_qnxnto() noexcept { return is_set(os_type_t::native, os_type_t::QnxNTO); }

    /** Evaluates `true` if platform os_type::native contains os_type::GenWasm */
    constexpr bool is_generic_wasm() noexcept { return is_set(os_type_t::native, os_type_t::GenWasm); }

    /** Evaluates `true` if platform os_type::native contains os_type::Emscripten */
    constexpr bool is_emscripten() noexcept { return is_set(os_type_t::native, os_type_t::Emscripten); }

    /** Evaluates `true` if platform supports posix compatible threading. */
    constexpr bool has_pthread() noexcept {
        #if JAU_OS_HAS_PTHREAD
            return true;
        #else
            return false;
        #endif
    }

    struct RuntimeOSInfo {
        std::string sysname;
        std::string nodename;
        std::string release;
        std::string version;
        std::string machine;
        std::string domainname;
        std::string to_string() noexcept {
            std::string sb = sysname+" "+release+", "+machine;
            if( nodename.length() > 0 ) {
                sb.append(", node ").append(nodename);
            }
            if( domainname.length() > 0 ) {
                sb.append(", domain ").append(domainname);
            }
            sb.append(", ").append(version);
            return sb;
        }
    };
    bool get_rt_os_info(RuntimeOSInfo& info) noexcept;

    enum class abi_type_t : uint16_t {
        generic =  0x00,
        /** ARM GNU-EABI ARMEL -mfloat-abi=softfp */
        gnu_armel = 0x01,
        /** ARM GNU-EABI ARMHF -mfloat-abi=hard */
        gnu_armhf = 0x02,
        /** ARM EABI AARCH64 (64bit) */
        aarch64   = 0x03,
        /** WASM Generic (32bit) */
        wasm32_gen = 0x20,
        /** WASM Emscripten (32bit) */
        wasm32_ems = 0x21,
        /** WASM Generic (64bit) */
        wasm64_gen = 0x2a,
        /** WASM Emscripten (64bit) */
        wasm64_ems = 0x2b
    };
    constexpr abi_type_t get_abi_type(const jau::cpu::cpu_family_t cpu) noexcept {
        if ( jau::cpu::cpu_family_t::arm64 == cpu ) {
            return abi_type_t::aarch64;
        } else if ( jau::cpu::cpu_family_t::arm32 == cpu ) {
            return abi_type_t::gnu_armhf; // FIXME?
        } else if ( jau::cpu::cpu_family_t::wasm32 == cpu ) {
            #if defined(__EMSCRIPTEN__)
                return abi_type_t::wasm32_ems;
            #else
                return abi_type_t::wasm32_gen;
            #endif
        } else if ( jau::cpu::cpu_family_t::wasm64 == cpu ) {
            #if defined(__EMSCRIPTEN__)
                return abi_type_t::wasm64_ems;
            #else
                return abi_type_t::wasm64_gen;
            #endif
        }
        return abi_type_t::generic;
    }
    inline abi_type_t get_abi_type() noexcept {
        return get_abi_type( jau::cpu::CpuInfo::get().family );
    }
    JAU_MAKE_ENUM_STRING(abi_type_t, generic, gnu_armel, gnu_armhf, aarch64, wasm32_gen, wasm32_ems, wasm64_gen, wasm64_ems);

    /**
     * Returns the common name for the given
     * os_type, jau::cpu::cpu_family, abi_type and endian.
     *
     * An excerpt of supported <code>os.and.arch</code> strings:
     * <ul>
     *   <li>android-armv6</li>
     *   <li>android-aarch64</li>
     *   <li>android-x86</li>
     *   <li>linux-armv6</li>
     *   <li>linux-armv6hf</li>
     *   <li>linux-i586</li>
     *   <li>linux-ppc</li>
     *   <li>linux-mips</li>
     *   <li>linux-mipsel</li>
     *   <li>linux-superh</li>
     *   <li>linux-sparc</li>
     *   <li>linux-aarch64</li>
     *   <li>linux-amd64</li>
     *   <li>linux-ppc64</li>
     *   <li>linux-ppc64le</li>
     *   <li>linux-mips64</li>
     *   <li>linux-ia64</li>
     *   <li>linux-sparcv9</li>
     *   <li>linux-risc2.0</li>
     *   <li>freebsd-i586</li>
     *   <li>freebsd-amd64</li>
     *   <li>darwin-universal</li>
     *   <li>windows-amd64</li>
     *   <li>windows-i586</li>
     * </ul>
     * @return The <i>os.and.arch</i> value.
     */
    std::string get_os_and_arch(const os_type_t os, const jau::cpu::cpu_family_t cpu, const abi_type_t abi, const endian_t e) noexcept;

    /** Returns this hosts's common name, see get_os_and_arch() */
    inline std::string get_os_and_arch() noexcept {
        return get_os_and_arch(os_type_t::native, jau::cpu::CpuInfo::get().family, get_abi_type(), endian_t::native);
    }

    /** Returns the OS's path separator character, e.g. `;` for Windows and `:` for Unix (rest of the world) */
    constexpr char path_separator_char() noexcept {
        if constexpr (jau::os::is_windows()) {
            return ';';
        } else {
            return ':';
        }
    }
    /** Returns the OS's path separator as a string, e.g. `;` for Windows and `:` for Unix (rest of the world) */
    constexpr_cxx20 std::string path_separator() noexcept {
        return std::string(1, path_separator_char());
    }

    /** Returns the OS's path separator character, e.g. `\\` for Windows and `/` for Unix (rest of the world) */
    constexpr char dir_separator_char() noexcept {
        if constexpr (jau::os::is_windows()) {
            return '\\';
        } else {
            return '/';
        }
    }

    /** Returns the OS's path separator as a string, e.g. `\\` for Windows and `/` for Unix (rest of the world) */
    constexpr_cxx20 std::string dir_separator() noexcept {
        return std::string(1, dir_separator_char());
    }

    std::string get_platform_info(std::string& sb) noexcept;
    inline std::string get_platform_info() noexcept {
        std::string sb; get_platform_info(sb); return sb;
    }

    /**@}*/

} // namespace jau::os

#endif /* JAU_OS_SUPPORT_HPP_ */
