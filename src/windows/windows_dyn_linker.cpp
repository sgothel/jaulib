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
#if defined(_WIN32)
    #include <cstdlib>

    #include <cassert>

    #include "jau/os/dyn_linker.hpp"

    #include <windows.h>
    /* This typedef is apparently needed for compilers before VC8,
       and for the embedded ARM compilers we're using */
    #if !defined(__MINGW64__) && ( (_MSC_VER < 1400) || defined(UNDER_CE) )
        typedef int intptr_t;
    #endif
    /* GetProcAddress doesn't exist in A/W variants under desktop Windows */
    #ifndef UNDER_CE
    #define GetProcAddressA GetProcAddress
    #endif

    namespace jau::os::impl {

        using namespace jau::os;

        class WindowsDynamicLinker : public DynamicLinker {
          private:
              constexpr static const int symbolArgAlignment=4;  // 4 byte alignment of each argument
              constexpr static const int symbolMaxArguments=12; // experience ..

          protected:
            libhandle_t openLibraryGlobalImpl(const std::string& pathname) noexcept override {
                HANDLE res = LoadLibraryW((LPCWSTR) pathname.c_str());
                return reinterpret_cast<libhandle_t>( res ); // NOLINT(performance-no-int-to-ptr,-warnings-as-errors)

            }
            libhandle_t openLibraryLocalImpl(const std::string& pathname) noexcept override {
                // How does that work under Windows ?
                // Don't know .. so it's an alias to global, for the time being
                HANDLE res = LoadLibraryW((LPCWSTR) pathname.c_str());
                return reinterpret_cast<libhandle_t>( res ); // NOLINT(performance-no-int-to-ptr,-warnings-as-errors)
            }

            const char* lookupLibraryPathnameImpl(libhandle_t handle, const std::string& symbolName) noexcept override {
                // symbolName is not required
                return 0 != handle ? GetModuleFileNameA( reinterpret_cast<HANDLE>(handle) ) : nullptr; // NOLINT(performance-no-int-to-ptr,-warnings-as-errors)
            }

            symhandle_t lookupSymbolLocalImpl(libhandle_t handle, const std::string& symbolName) noexcept override {
                std::string altSymbolName = symbolName;
                symhandle_t addr = GetProcAddressA(reinterpret_cast<HANDLE>(handle), altSymbolName.c_str()); // NOLINT(performance-no-int-to-ptr,-warnings-as-errors)
                if( 0 == addr ) {
                    // __stdcall hack: try some @nn decorations,
                    //                 the leading '_' must not be added (same with cdecl)
                    for(int arg=0; 0==addr && arg<=symbolMaxArguments; arg++) {
                        altSymbolName = symbolName+"@"+std::to_string(arg*symbolArgAlignment);
                        addr = GetProcAddressA(reinterpret_cast<HANDLE>(handle), altSymbolName.c_str()); // NOLINT(performance-no-int-to-ptr,-warnings-as-errors)
                    }
                }
                return addr;
            }

            symhandle_t lookupSymbolGlobalImpl(const std::string& symbolName) noexcept override {
                if(DEBUG_LOOKUP) {
                    jau_WARN_PRINT("lookupSymbolGlobal: Not supported on Windows");
                }
                // allow DynamicLibraryBundle to continue w/ local libs
                return 0;
            }

            void closeLibraryImpl(libhandle_t handle) noexcept override {
                FreeLibrary( reinterpret_cast<HANDLE>(handle) ); // NOLINT(performance-no-int-to-ptr,-warnings-as-errors)
            }

            std::string getLastErrorImpl() noexcept override {
                const int err = GetLastError();
                return "Last error: "+jau::to_hexstring(err)+" ("+std::to_string(err)+")";
            }
        };

    } // namespace jau::os::impl

    jau::os::DynamicLinker* jau::os::DynamicLinker::create() {
        return new jau::os::impl::WindowsDynamicLinker();
    }

#endif // _WIN32
