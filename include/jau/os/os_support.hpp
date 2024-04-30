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

#include "jau/cpp_lang_util.hpp"
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
                return 0b00000001000000000000000000000000U; // WebAsm
            #elif defined(__QNXNTO__)
                return 0b00000000000000000001000000000001U; // QnxNTO
            #elif defined(__APPLE__) && defined(__MACH__)
                return 0b00000000000000000000100000000001U; // Darwin
            #elif defined(__FreeBSD__)
                return 0b00000000000000000000010000000001U; // FreeBSD
            #elif defined(__ANDROID__)
                return 0b00000000000000000000001100000001U; // Android
            #elif defined(__linux__)
                return 0b00000000000000000000000100000001U; // Linux
            #elif defined(_WIN32)
                return 0b00000000000000000000000000000010U; // Windows
            #else
                return 0b00000000000000000000000000000001U; // Unix
            #endif
        }
    }

    /** OS type bits and unique IDs */
    enum class os_type : uint32_t {
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
        /** WebAssembly bit */
        WebAsm  = 0b00000001000000000000000000000000U,
        /** Identifier for native OS type, one of the above. */
        native      = impl::get_host_os_id()
    };
    constexpr uint32_t number(const os_type rhs) noexcept {
        return static_cast<uint32_t>(rhs);
    }
    constexpr os_type operator~(const os_type rhs) noexcept {
        return static_cast<os_type>(~number(rhs));
    }
    constexpr os_type operator^(const os_type lhs, const os_type rhs) noexcept {
        return static_cast<os_type>(number(lhs) ^ number(rhs));
    }
    constexpr os_type operator|(const os_type lhs, const os_type rhs) noexcept {
        return static_cast<os_type>(number(lhs) | number(rhs));
    }
    constexpr os_type operator&(const os_type lhs, const os_type rhs) noexcept {
        return static_cast<os_type>(number(lhs) & number(rhs));
    }
    constexpr os_type& operator|=(os_type& lhs, const os_type rhs) noexcept {
        lhs = static_cast<os_type>(number(lhs) | number(rhs));
        return lhs;
    }
    constexpr os_type& operator&=(os_type& lhs, const os_type rhs) noexcept {
        lhs = static_cast<os_type>(number(lhs) & number(rhs));
        return lhs;
    }
    constexpr os_type& operator^=(os_type& lhs, const os_type rhs) noexcept {
        lhs = static_cast<os_type>(number(lhs) ^ number(rhs));
        return lhs;
    }
    constexpr bool operator==(const os_type lhs, const os_type rhs) noexcept {
        return number(lhs) == number(rhs);
    }
    constexpr bool operator!=(const os_type lhs, const os_type rhs) noexcept {
        return !(lhs == rhs);
    }
    constexpr bool is_set(const os_type mask, const os_type bits) noexcept {
        return bits == (mask & bits);
    }
    /**
     * Return the string representation of os_type
     * @param mask the os_type to convert
     * @return the string representation.
     */
    std::string to_string(const os_type mask) noexcept;

    /**
     * Evaluates `true` if the given \ref os_type is defined,
     * i.e. `Unix`, `Windows`, `Linux`, `Android`, ...
     */
    constexpr bool is_defined_os_type(const os_type v) noexcept {
        switch(v) {
            case os_type::Unix:
                [[fallthrough]];
            case os_type::Windows:
                [[fallthrough]];
            case os_type::Linux:
                [[fallthrough]];
            case os_type::Android:
                [[fallthrough]];
            case os_type::FreeBSD:
                [[fallthrough]];
            case os_type::Darwin:
                [[fallthrough]];
            case os_type::QnxNTO:
                [[fallthrough]];
            case os_type::WebAsm:
                return true;
            default:
                return false;
        }
    }

    // one static_assert is sufficient for whole compilation unit
    static_assert( is_defined_os_type(os_type::native) ); // Enhance os_type to match your platform!

    /** Evaluates `true` if platform os_type::native contains os_type::Unix */
    constexpr bool is_unix() noexcept { return is_set(os_type::native, os_type::Unix); }

    /** Evaluates `true` if platform os_type::native contains os_type::Windows */
    constexpr bool is_windows() noexcept { return is_set(os_type::native, os_type::Windows); }

    /** Evaluates `true` if platform os_type::native contains os_type::Linux */
    constexpr bool is_linux() noexcept { return is_set(os_type::native, os_type::Linux); }

    /** Evaluates `true` if platform os_type::native contains os_type::Android */
    constexpr bool is_android() noexcept { return is_set(os_type::native, os_type::Android); }

    /** Evaluates `true` if platform os_type::native contains os_type::FreeBSD */
    constexpr bool is_freebsd() noexcept { return is_set(os_type::native, os_type::FreeBSD); }

    /** Evaluates `true` if platform os_type::native contains os_type::Darwin */
    constexpr bool is_darwin() noexcept { return is_set(os_type::native, os_type::Darwin); }

    /** Evaluates `true` if platform os_type::native contains os_type::QnxNTO */
    constexpr bool is_qnxnto() noexcept { return is_set(os_type::native, os_type::QnxNTO); }

    /** Evaluates `true` if platform os_type::native contains os_type::WebAsm */
    constexpr bool is_wasm() noexcept { return is_set(os_type::native, os_type::WebAsm); }

    struct rt_os_info {
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
    bool get_rt_os_info(rt_os_info& info) noexcept;

    enum class abi_type : uint16_t {
        generic_abi =  0x00,
        /** ARM GNU-EABI ARMEL -mfloat-abi=softfp */
        eabi_gnu_armel = 0x01,
        /** ARM GNU-EABI ARMHF -mfloat-abi=hard */
        eabi_gnu_armhf = 0x02,
        /** ARM EABI AARCH64 (64bit) */
        eabi_aarch64   = 0x03,
        /** WASM Undefined  */
        wasm_abi_undef = 0x20,
        /** WASM Emscripten  */
        wasm_abi_emscripten = 0x21
    };
    constexpr abi_type get_abi_type(const jau::cpu::cpu_family cpu) noexcept {
        if ( jau::cpu::cpu_family::arm64 == cpu ) {
            return abi_type::eabi_aarch64;
        } else if ( jau::cpu::cpu_family::arm32 == cpu ) {
            return abi_type::eabi_gnu_armhf; // FIXME?
        } else if ( jau::cpu::cpu_family::wasm == cpu ) {
            #if defined(__EMSCRIPTEN__)
                return abi_type::wasm_abi_emscripten;
            #else
                return abi_type::wasm_abi_undef;
            #endif
        }
        return abi_type::generic_abi;
    }
    constexpr abi_type get_abi_type() noexcept {
        return get_abi_type( jau::cpu::get_cpu_family() );
    }
    std::string to_string(const abi_type abi) noexcept;

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
    std::string get_os_and_arch(const os_type os, const jau::cpu::cpu_family cpu, const abi_type abi, const endian e) noexcept;

    /** Returns this hosts's common name, see get_os_and_arch() */
    inline std::string get_os_and_arch() noexcept {
        return get_os_and_arch(os_type::native, jau::cpu::get_cpu_family(), get_abi_type(), endian::native);
    }

    /** Returns the OS's path separator character, e.g. `;` for Windows and `:` for Unix (rest of the world) */
    inline char getPathSeparatorChar() noexcept {
        return jau::os::is_windows() ? ';' : ':';
    }
    /** Returns the OS's path separator as a string, e.g. `;` for Windows and `:` for Unix (rest of the world) */
    inline std::string getPathSeparator() noexcept {
        return std::string(1, getPathSeparatorChar());
    }

    /** Returns the OS's path separator character, e.g. `\\` for Windows and `/` for Unix (rest of the world) */
    inline char getDirSeparatorChar() noexcept {
        return jau::os::is_windows() ? '\\' : '/';
    }
    /** Returns the OS's path separator as a string, e.g. `\\` for Windows and `/` for Unix (rest of the world) */
    inline std::string getDirSeparator() noexcept {
        return std::string(1, getDirSeparatorChar());
    }

    std::string get_platform_info(std::string& sb) noexcept;
    inline std::string get_platform_info() noexcept {
        std::string sb; get_platform_info(sb); return sb;
    }

    /**@}*/

} // namespace jau::os

#endif /* JAU_OS_SUPPORT_HPP_ */
