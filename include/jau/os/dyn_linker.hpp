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
#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "jau/debug.hpp"
#include "jau/string_util.hpp"
#include "jau/environment.hpp"
#include "jau/os/os_support.hpp"

namespace jau::os {

    /** \addtogroup OSSup
     *
     *  @{
     */

    /** Low level secure dynamic linker access. */
    class DynamicLinker {
      public:
        /** library handle */
        typedef intptr_t libhandle_t;
        /** symbol handle within a library */
        typedef intptr_t symhandle_t;

      protected:
        constexpr static const bool DEBUG_LOOKUP = false;

        //
        // Implemented per platform as hidden detail
        //

        virtual libhandle_t openLibraryGlobalImpl(const std::string& pathname) noexcept = 0;
        virtual libhandle_t openLibraryLocalImpl(const std::string& pathname) noexcept = 0;
        virtual const char* lookupLibraryPathnameImpl(libhandle_t libraryHandle, const std::string& symbolName) noexcept = 0;
        virtual symhandle_t lookupSymbolGlobalImpl(const std::string& symbolName) noexcept = 0;
        virtual symhandle_t lookupSymbolLocalImpl(libhandle_t handle, const std::string& symbolName) noexcept = 0;
        virtual void closeLibraryImpl(libhandle_t handle) noexcept = 0;
        virtual std::string getLastErrorImpl() noexcept = 0;

      private:
        class LibRef {
          private:
            std::string m_name;
            ssize_t m_count;
          public:
            LibRef(const std::string& name) noexcept
            : m_name(name), m_count(1) {}

            constexpr_cxx20 LibRef(const LibRef& o) noexcept = default;
            constexpr_cxx20 LibRef(LibRef&& o) noexcept = default;
            LibRef& operator=(const LibRef&) noexcept = default;
            LibRef& operator=(LibRef&&) noexcept = default;

            ssize_t incrRefCount() { return ++m_count; }
            ssize_t decrRefCount() { return --m_count; }
            ssize_t count() { return m_count; }

            const std::string& name() { return m_name; }

            std::string toString() { return "LibRef["+m_name+", count "+std::to_string(m_count)+"]"; }
        };
        typedef std::shared_ptr<LibRef> LibRef_ref;

        typedef std::unordered_map<libhandle_t, LibRef_ref> LibRefMap_t;
        typedef typename LibRefMap_t::iterator LibRefIter_t;

        std::mutex m_mtx_libref;
        LibRefMap_t m_handleToNameMap;

        static DynamicLinker* create();

      protected:
        DynamicLinker() noexcept = default;

        LibRef_ref incrLibRefCount(const libhandle_t handle, const std::string& libName) noexcept {
            std::unique_lock<std::mutex> lock(m_mtx_libref);
            LibRef_ref libRef = nullptr;
            LibRefIter_t iter = m_handleToNameMap.find(handle);
            if( m_handleToNameMap.end() == iter ) {
                std::pair<LibRefIter_t,bool> res = m_handleToNameMap.insert( { handle, std::make_shared<LibRef>(libName) } );
                libRef = res.first->second;
                if( !res.second ) {
                    libRef->incrRefCount();
                }
            } else {
                libRef = iter->second;
                libRef->incrRefCount();
            }
            DBG_PRINT("DynamicLinkerImpl.incrLibRefCount %s -> %s, libs loaded %zu",
                    jau::to_hexstring(handle).c_str(), libRef->toString().c_str(), m_handleToNameMap.size());
            return libRef;
        }

        LibRef_ref decrLibRefCount(const libhandle_t handle) noexcept {
            std::unique_lock<std::mutex> lock(m_mtx_libref);
            LibRef_ref libRef = nullptr;
            LibRefIter_t iter = m_handleToNameMap.find(handle);
            if( m_handleToNameMap.end() != iter ) {
                libRef = iter->second;
                if( 0 == libRef->decrRefCount() ) {
                    m_handleToNameMap.erase(iter);
                }
                DBG_PRINT("DynamicLinkerImpl.decrLibRefCount %s -> %s, libs loaded %zu",
                        jau::to_hexstring(handle).c_str(), libRef->toString().c_str(), m_handleToNameMap.size());
            } else {
                DBG_PRINT("DynamicLinkerImpl.decrLibRefCount %s -> null, libs loaded %zu",
                        jau::to_hexstring(handle).c_str(), m_handleToNameMap.size());
            }
            return libRef;
        }

      public:
        virtual ~DynamicLinker() noexcept = default;

        /** Returns the environment library path variable name, e.g. `LD_LIBRARY_PATH` */
        constexpr_cxx20 static std::string getEnvLibPathVarName() noexcept {
            if constexpr ( jau::os::is_darwin() ) {
                return "DYLD_LIBRARY_PATH";
            } else if constexpr ( jau::os::is_windows() ) {
                return "PATH";
            } else {
                return "LD_LIBRARY_PATH";
            }
        }
        /**
         * Returns a list of system paths, from the {@link #getSystemEnvLibraryPathVarname()} variable.
         */
        static std::vector<std::string> getSystemEnvLibraryPaths() {
            return jau::split_string(jau::environment::getProperty( getEnvLibPathVarName() ), jau::os::getPathSeparator());
        }

        /** Returns the native library prefix, e.g. `lib` */
        constexpr_cxx20 static std::string getDefaultPrefix() noexcept {
            if constexpr ( jau::os::is_windows() ) {
                return "";
            } else {
                return "lib";
            }
        }
        /** Returns the native library suffix including the dot, e.g. `.so` */
        constexpr_cxx20 static std::string getDefaultSuffix() noexcept {
            if constexpr ( jau::os::is_darwin() ) {
                return ".dylib";
            } else if constexpr ( jau::os::is_windows() ) {
                return ".dll";
            } else {
                return ".so";
            }
        }
        /**
         * Returns canonical library name for this system from given library-basename, e.g. `tool' -> `libtool.so`
         * if it is not yet canonical, see isCanonicalName().
         * @param basename the library basename
         * @param checkIsCanonical pass true to first check is basename is already canonical, defaults to true
         * @see isCanonicalName()
         * @see getBaseName()
         * @see jau::fs::basename()
         */
        constexpr_cxx20 static std::string getCanonicalName(const std::string& basename, const bool checkIsCanonical=true) noexcept {
            if( !checkIsCanonical || !isCanonicalName(basename, true) ) {
                return getDefaultPrefix()+basename+getDefaultSuffix();
            } else {
                return basename;
            }
        }

        /**
         * Returns true if the given filename contains the canonical prefix and suffix,
         * otherwise returns false.
         *
         * Validation is performed case insensitive if `caseInsensitive == true`
         *
         * @param filename the filename to test
         * @param isBasename pass true if filename is a basename, otherwise false. Defaults to false. See jau::fs::basename().
         * @param caseInsensitive perform prefix and suffix comparison case-insensitive, defaults to true on Windows otherwise false
         * @see getCanonicalName()
         * @see getBaseName()
         * @see jau::fs::basename()
         */
        static bool isCanonicalName(const std::string& filename, const bool isBasename=false, const bool caseInsensitive=jau::os::is_windows()) noexcept;

        /**
         * Returns the library basename, i.e. the file basename without prefix nor suffix,
         * performed case insensitive if `caseInsensitive == true`
         *
         * @param filename the filename to process
         * @param isBasename pass true if filename is a basename, otherwise false. Defaults to false. See jau::fs::basename().
         * @param caseInsensitive perform prefix and suffix comparison case-insensitive, defaults to true on Windows otherwise false
         * @return basename of filename w/o path nor prefix or suffix, ie. /usr/lib/libDrinkBeer.so -> DrinkBeer on Unix systems
         * @see getCanonicalName()
         * @see isCanonicalName()
         * @see jau::fs::basename()
         */
        static std::string getBaseName(const std::string& filename, const bool isBasename=false, const bool caseInsensitive=jau::os::is_windows()) noexcept;

        /**
         * Returns list of potential absolute library filenames
         * @param libName library basename (implies file-basename) or canonical-library-name as file-basename or absolute-path
         * @param searchSystemPath
         * @param searchSystemPathFirst
         */
        static std::vector<std::string> enumerateLibraryPaths(const std::string& libName,
                                                              bool searchSystemPath=false,
                                                              bool searchSystemPathFirst=false) noexcept;


        /** Returns static singleton instance of DynamicLinker */
        static DynamicLinker& get() noexcept {
            /**
             * Thread safe starting with C++11 6.7:
             *
             * If control enters the declaration concurrently while the variable is being initialized,
             * the concurrent execution shall wait for completion of the initialization.
             *
             * (Magic Statics)
             *
             * Avoiding non-working double checked locking.
             */
            static DynamicLinker* dl = create();
            return *dl;
        }

        /**
         * Opens the named library, allowing system wide access for other <i>users</i>.
         *
         * @param pathname the full pathname for the library to open
         * @return the library handle, maybe 0 if not found.
         */
        libhandle_t openLibraryGlobal(const std::string& pathname) noexcept {
            libhandle_t handle = openLibraryGlobalImpl(pathname);
            if( 0 != handle ) {
                LibRef_ref libRef = incrLibRefCount(handle, pathname);
                DBG_PRINT("DynamicLinkerImpl.openLibraryGlobal \"%s\": %s -> %s",
                        pathname.c_str(), jau::to_hexstring(handle).c_str(), libRef->toString().c_str());
            } else {
                DBG_PRINT("DynamicLinkerImpl.openLibraryGlobal \"%s\" failed, error %s",
                        pathname.c_str(), getLastError().c_str());
            }
            return handle;
        }

        /**
         * Opens the named library, restricting access to this process.
         *
         * @param pathname the full pathname for the library to open
         * @return the library handle, maybe 0 if not found.
         */
        libhandle_t openLibraryLocal(const std::string& pathname) noexcept {
            libhandle_t handle = openLibraryLocalImpl(pathname);
            if( 0 != handle ) {
                LibRef_ref libRef = incrLibRefCount(handle, pathname);
                DBG_PRINT("DynamicLinkerImpl.openLibraryLocal \"%s\": %s -> %s",
                        pathname.c_str(), jau::to_hexstring(handle).c_str(), libRef->toString().c_str());
            } else {
                DBG_PRINT("DynamicLinkerImpl.openLibraryLocal \"%s\" failed, error %s",
                        pathname.c_str(), getLastError().c_str());
            }
            return handle;
        }

        /**
         * @param libraryHandle a library handle previously retrieved via {@link #openLibraryLocal(const std::string_view&, boolean)} or {@link #openLibraryGlobal(const std::string_view&, boolean)}.
         * @param symbolName optional symbol name for an OS which requires the symbol's address to retrieve the path of the containing library
         * @return the library pathname if found and supported by OS or {@code null}.
         */
        const char* lookupLibraryPathname(libhandle_t handle, const std::string& symbolName) noexcept {
            const char* fname = lookupLibraryPathnameImpl(handle, symbolName);
            if(DEBUG_LOOKUP) {
                jau::INFO_PRINT("DynamicLinkerImpl.lookupLibraryPathname(%s, %s) -> '%s'",
                        jau::to_hexstring(handle).c_str(), symbolName.c_str(), nullptr != fname ? fname : "null");
            }
            return fname;
        }

        /**
         * @param symbolName global symbol name to lookup up system wide.
         * @return the symbol handle, maybe nullptr if not found.
         */
        symhandle_t lookupSymbolGlobal(const std::string& symbolName) noexcept {
            intptr_t addr = lookupSymbolGlobalImpl(symbolName);
            if(DEBUG_LOOKUP) {
                jau::INFO_PRINT("DynamicLinkerImpl.lookupSymbolGlobal(%s) -> %s",
                        symbolName.c_str(), jau::to_hexstring(addr).c_str());
            }
            return addr;
        }

        /**
         * @param libraryHandle a library handle previously retrieved via {@link #openLibraryLocal(const std::string_view&, boolean)} or {@link #openLibraryGlobal(const std::string_view&, boolean)}.
         * @param symbolName global symbol name to lookup up system wide.
         * @return the symbol handle, maybe nullptr if not found.
         */
        symhandle_t lookupSymbol(libhandle_t handle, const std::string& symbolName) noexcept {
            intptr_t addr = lookupSymbolLocalImpl(handle, symbolName);
            if(DEBUG_LOOKUP) {
                jau::INFO_PRINT("DynamicLinkerImpl.lookupSymbol(%s, %s) -> %s",
                        jau::to_hexstring(handle).c_str(), symbolName.c_str(), jau::to_hexstring(addr).c_str());
            }
            return addr;
        }

        /**
         * Security checks are implicit by previous call of
         * {@link #openLibraryLocal(const std::string_view&, boolean)} or {@link #openLibraryGlobal(const std::string_view&, boolean)}
         * retrieving the <code>librarHandle</code>.
         *
         * @param libraryHandle a library handle previously retrieved via {@link #openLibraryLocal(const std::string_view&, boolean)} or {@link #openLibraryGlobal(const std::string_view&, boolean)}.
         */
        void closeLibrary(libhandle_t handle) noexcept {
            LibRef_ref libRef = decrLibRefCount( handle ); // null libRef is OK for global lookup
            if( nullptr != libRef ) {
                DBG_PRINT("DynamicLinkerImpl.closeLibrary(%s -> %s)",
                        jau::to_hexstring(handle).c_str(), libRef->toString().c_str());
            } else {
                DBG_PRINT("DynamicLinkerImpl.closeLibrary(%s -> null)", jau::to_hexstring(handle).c_str());
            }
            if( 0 != handle ) {
                closeLibraryImpl(handle);
            }
        }
        /**
         * Returns a string containing the last error.
         * Maybe called for debugging purposed if any method fails.
         * @return error string, maybe null. A null or non-null value has no semantics.
         */
        std::string getLastError() noexcept { return getLastErrorImpl(); }
    };

    /**@}*/

} // namespace jau::os

