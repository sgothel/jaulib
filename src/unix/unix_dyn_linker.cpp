/**
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2024 Gothel Software e.K.
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
#if !defined(_WIN32)

    #if defined(__linux__)
     #ifndef _GNU_SOURCE
       #define _GNU_SOURCE
     #endif
    #else
     // NOP
    #endif

    #include <dlfcn.h>
    #include <cinttypes>

    #include <cassert>

    #include "jau/os/dyn_linker.hpp"

    #ifndef RTLD_DEFAULT
        #define LIB_DEFAULT   ((void *) 0)
    #endif

    namespace jau::os::impl {

        using namespace jau::os;

        class UnixDynamicLinker : public DynamicLinker {
          private:
            void* const m_lib_default;
            void* const m_lib_next;

            int const m_flag_lazy;
            int const m_flag_now;
            int const m_flag_local;
            int const m_flag_global;

          protected:
            UnixDynamicLinker(void* const lib_default, void* const lib_next,
                              int const flag_lazy, int const flag_now, int const flag_local, int const flag_global)
            : m_lib_default(lib_default), m_lib_next(lib_next),
              m_flag_lazy(flag_lazy), m_flag_now(flag_now), m_flag_local(flag_local), m_flag_global(flag_global)
            {
                (void)m_lib_next;
                (void)m_flag_now;
            }

            libhandle_t openLibraryGlobalImpl(const std::string& pathname) noexcept override {
                return ::dlopen((char *) pathname.c_str(), m_flag_lazy | m_flag_global);

            }
            libhandle_t openLibraryLocalImpl(const std::string& pathname) noexcept override {
                return ::dlopen((char *) pathname.c_str(), m_flag_lazy | m_flag_local);
            }

            const char* lookupLibraryPathnameImpl(libhandle_t handle, const std::string& symbolName) noexcept override {
                if( nullptr != handle && symbolName.length() > 0 ) {
                    symhandle_t addr = ::dlsym(handle, symbolName.c_str());
                    if( nullptr != addr ) {
                        Dl_info info;
                        if( 0 != ::dladdr(addr, &info) ) {
                            return info.dli_fname;
                        } else {
                            return nullptr;
                        }
                    }
                }
                return nullptr;
            }

            symhandle_t lookupSymbolGlobalImpl(const std::string& symbolName) noexcept override {
                return ::dlsym(m_lib_default, symbolName.c_str());
            }

            symhandle_t lookupSymbolLocalImpl(libhandle_t handle, const std::string& symbolName) noexcept override {
                if( nullptr != handle ) {
                    return ::dlsym(handle, symbolName.c_str());
                } else {
                    return nullptr;
                }
            }

            void closeLibraryImpl(libhandle_t handle) noexcept override {
                if( nullptr != handle ) {
                    ::dlclose(handle);
                }
            }

            std::string getLastErrorImpl() noexcept override {
                const char * res = ::dlerror();
                if( nullptr != res ) {
                    return std::string(res);
                } else {
                    return std::string();
                }
            }
        };

        /**
         * POSIX specialization of UnixDynamicLinkerImpl with POSIX flags and mode values.
         */
        class PosixDynamicLinker : public UnixDynamicLinker {
          private:
            constexpr static void* const LIB_DEFAULT = nullptr;
            inline static void* const LIB_NEXT    = reinterpret_cast<void *>( -1l ); // NOLINT(performance-no-int-to-ptr,-warnings-as-errors)

            constexpr static int const FLAG_LAZY     = 0x00001;
            constexpr static int const FLAG_NOW      = 0x00002;
            constexpr static int const FLAG_LOCAL    = 0x00000;
            constexpr static int const FLAG_GLOBAL   = 0x00100;

          public:
            PosixDynamicLinker() noexcept
            : UnixDynamicLinker(LIB_DEFAULT, LIB_NEXT, FLAG_LAZY, FLAG_NOW, FLAG_LOCAL, FLAG_GLOBAL) {}
        };

        /**
         * Darwin (MacOSX/iOS) specialization of UnixDynamicLinkerImpl with non-POSIX flags and mode values.
         */
        class DarwinDynamicLinker : public UnixDynamicLinker {
          private:
            inline static void* const LIB_DEFAULT = reinterpret_cast<void *>( -2l ); // NOLINT(performance-no-int-to-ptr,-warnings-as-errors)
            inline static void* const LIB_NEXT    = reinterpret_cast<void *>( -1l ); // NOLINT(performance-no-int-to-ptr,-warnings-as-errors)

            constexpr static int const FLAG_LAZY     = 0x00001;
            constexpr static int const FLAG_NOW      = 0x00002;
            constexpr static int const FLAG_LOCAL    = 0x00004;
            constexpr static int const FLAG_GLOBAL   = 0x00008;

          public:
            DarwinDynamicLinker() noexcept
            : UnixDynamicLinker(LIB_DEFAULT, LIB_NEXT, FLAG_LAZY, FLAG_NOW, FLAG_LOCAL, FLAG_GLOBAL) {}
        };

        /**
         * Bionic 32bit (Android) specialization of UnixDynamicLinkerImpl with non-POSIX flags and mode values.
         *
         * Note: Bionic 64bit seems to POSIX compliant
         */
        class Bionic32DynamicLinker : public UnixDynamicLinker {
          private:
            inline static void* const LIB_DEFAULT = reinterpret_cast<void *>( 0xfffffffful ); // NOLINT(performance-no-int-to-ptr,-warnings-as-errors)
            inline static void* const LIB_NEXT    = reinterpret_cast<void *>( 0xfffffffeul ); // NOLINT(performance-no-int-to-ptr,-warnings-as-errors)

            constexpr static int const FLAG_LAZY     = 0x00001;
            constexpr static int const FLAG_NOW      = 0x00000;
            constexpr static int const FLAG_LOCAL    = 0x00000;
            constexpr static int const FLAG_GLOBAL   = 0x00002;
            // constexpr static int const FLAG_NOLOAD   = 0x00004;

          public:
            Bionic32DynamicLinker() noexcept
            : UnixDynamicLinker(LIB_DEFAULT, LIB_NEXT, FLAG_LAZY, FLAG_NOW, FLAG_LOCAL, FLAG_GLOBAL) {}
        };
    } // namespace jau::os::impl

    jau::os::DynamicLinker* jau::os::DynamicLinker::create() {
        if constexpr ( jau::os::is_android() && 32 == jau::cpu::get_arch_psize() ) {
            return new jau::os::impl::Bionic32DynamicLinker();
        } else if constexpr ( jau::os::is_darwin() ) {
            return new jau::os::impl::DarwinDynamicLinker();
        } else {
            return new jau::os::impl::PosixDynamicLinker();
        }
    }

#endif // !_WIN32
