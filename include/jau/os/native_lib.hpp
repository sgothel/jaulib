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

#include <string>

#include "jau/string_util.hpp"

#include "jau/os/func_resolver.hpp"
#include "jau/os/dyn_linker.hpp"

namespace jau::os {

    /** \addtogroup OSSup
     *
     *  @{
     */


    /** Runtime libary dynamic library (RTLD) access. */
    class NativeLibrary : public DynamicLookup {
        private:
            DynamicLinker& m_dynLink;

            DynamicLinker::libhandle_t m_libraryHandle;

            // May as well keep around the path to the library we opened
            std::string m_libraryPath;

            bool m_global;

            // Native library path of the opened native libraryHandle, maybe null
            std::string m_nativeLibraryPath;

            static std::string getNativeLibPath(DynamicLinker& dl, DynamicLinker::libhandle_t libraryHandle,
                                                const std::string& libraryPath, const std::string& symbolName) {
                if( nullptr == libraryHandle ) {
                    return "";
                } else {
                    const char *nlp = dl.lookupLibraryPathname(libraryHandle, symbolName);
                    if( nullptr != nlp ) {
                        return std::string(nlp);
                    } else {
                        return libraryPath;
                    }
                }
            }

            // Private constructor to prevent arbitrary instances from floating around
            NativeLibrary(DynamicLinker& dl, DynamicLinker::libhandle_t libraryHandle, const std::string& libraryPath, bool global, const std::string& symbolName)
            : m_dynLink(dl), m_libraryHandle(libraryHandle), m_libraryPath(libraryPath), m_global(global),
              m_nativeLibraryPath(getNativeLibPath(dl, libraryHandle, libraryPath, symbolName))
            { }

        public:

            DynamicLinker::symhandle_t dynamicLookupFunction(const std::string& funcName) const noexcept override {
                return m_dynLink.lookupSymbol(m_libraryHandle, funcName);
            }

            DynamicLinker::symhandle_t dynamicLookupFunctionGlobal(const std::string& funcName) const noexcept override {
                return m_dynLink.lookupSymbolGlobal(funcName);
            }

            /** Returns the used DynamicLinker reference. */
            DynamicLinker& dynamicLinker() const noexcept { return m_dynLink; }

            /**
             * Returns true if this instance is valid, i.e. native library successfully opened once but not necessarily isOpen() now.
             * @see resolvedLibraryPath()
             */
            bool isValid() const noexcept { return m_nativeLibraryPath.size() > 0; }

            /** Returns true if this instance isValid() and not close()'ed, otherwise false. */
            bool isOpen() const noexcept { return nullptr != m_libraryHandle; }

            /** Returns the native library handle if valid and not closed, otherwise nullptr. */
            DynamicLinker::libhandle_t libraryHandle() const noexcept { return m_libraryHandle; }

            /** Returns the path of the opened native library file. */
            const std::string& libraryPath() const noexcept { return m_libraryPath; }

            /**
             * Returns the resolved native path of the opened native library, might be libraryPath() if not supported by OS.
             *
             * If this native library is not isValid(), method returns an empty string.
             * @see isValid()
             */
            const std::string& resolvedLibraryPath() const noexcept { return m_nativeLibraryPath; }

            /** Closes this native library. Further lookup operations are not allowed after calling this method. */
            void close() noexcept  {
                jau_DBG_PRINT("NativeLibrary.close(): closing %s", toString());
                if ( nullptr != m_libraryHandle ) {
                    const DynamicLinker::libhandle_t handle = m_libraryHandle;
                    m_libraryHandle = nullptr;
                    m_dynLink.closeLibrary(handle);
                    jau_DBG_PRINT("NativeLibrary.close(): Successfully closed %s", toString());
                }
            }

            std::string toString() const noexcept {
                if( isValid() ) {
                    return "NativeLibrary[path[given '" + m_libraryPath + "', native '"+m_nativeLibraryPath+"'], 0x" +
                            jau::toHexString(m_libraryHandle) + ", global " + std::to_string(m_global) + "]";
                } else {
                    return "NativeLibrary[invalid, path[given '" + m_libraryPath + "'], 0x" +
                            jau::toHexString(m_libraryHandle) + ", global " + std::to_string(m_global) + "]";
                }
            }

            /**
             * Opens the given native library, assuming it has the same base name on all platforms.
             *
             * The {@code searchSystemPath} argument changes the behavior to
             * either use the default system path or not at all.
             *
             * Assuming {@code searchSystemPath} is {@code true},
             * the {@code searchSystemPathFirst} argument changes the behavior to first
             * search the default system path rather than searching it last.
             *
             * @param libName library name, with or without prefix and suffix
             * @param searchSystemPath if {@code true} library shall be searched in the system path <i>(default)</i>, otherwise {@code false}.
             * @param searchSystemPathFirst if {@code true} system path shall be searched <i>first</i> <i>(default)</i>, rather than searching it last.
             *                              if {@code searchSystemPath} is {@code false} this argument is ignored.
             * @param global if {@code true} allows system wide access of the loaded library, otherwise access is restricted to the process.
             * @return {@link NativeLibrary} instance, use isValid() to check whether the native library was loaded successful.
             */
            static NativeLibrary open(const std::string& libName,
                                      const bool searchSystemPath,
                                      const bool searchSystemPathFirst,
                                      const bool global) noexcept {
              return open(libName, searchSystemPath, searchSystemPathFirst, global, "");
            }

            /**
             * Opens the given native library, assuming it has the same base name on all platforms.
             *
             * The {@code searchSystemPath} argument changes the behavior to
             * either use the default system path or not at all.
             *
             * Assuming {@code searchSystemPath} is {@code true},
             * the {@code searchSystemPathFirst} argument changes the behavior to first
             * search the default system path rather than searching it last.
             *
             * @param libName library name, with or without prefix and suffix
             * @param searchSystemPath if {@code true} library shall be searched in the system path <i>(default)</i>, otherwise {@code false}.
             * @param searchSystemPathFirst if {@code true} system path shall be searched <i>first</i> <i>(default)</i>, rather than searching it last.
             *                              if {@code searchSystemPath} is {@code false} this argument is ignored.
             * @param loader {@link ClassLoader} to locate the library
             * @param global if {@code true} allows system wide access of the loaded library, otherwise access is restricted to the process.
             * @param symbolName optional symbol name for an OS which requires the symbol's address to retrieve the path of the containing library
             * @return {@link NativeLibrary} instance, use isValid() to check whether the native library was loaded successful.
             * @throws SecurityException if user is not granted access for the named library.
             * @since 2.4.0
             */
            static NativeLibrary open(const std::string& libName,
                                      const bool searchSystemPath,
                                      const bool searchSystemPathFirst,
                                      const bool global, const std::string& symbolName) noexcept {
              std::vector<std::string> paths = DynamicLinker::enumerateLibraryPaths(libName, searchSystemPath, searchSystemPathFirst);

              DynamicLinker& m_dynLink = DynamicLinker::get();

              // Iterate down these and see which one if any we can actually find.
              for (const std::string& path : paths ) {
                  jau_DBG_PRINT("NativeLibrary.open(global %d): Trying to load %s", global, path);
                  DynamicLinker::libhandle_t res;
                  if(global) {
                      res = m_dynLink.openLibraryGlobal(path);
                  } else {
                      res = m_dynLink.openLibraryLocal(path);
                  }
                  if ( nullptr != res ) {
                      NativeLibrary nl(m_dynLink, res, path, global, symbolName);
                      jau_DBG_PRINT("NativeLibrary.open: Opened: %s", nl.toString());
                      return nl;
                  } else if( jau::environment::get().debug ) {
                      jau_DBG_PRINT("NativeLibrary.open: Failed to open '%s', last error %s", path, m_dynLink.getLastError());
                  }
              }
              jau_DBG_PRINT("NativeLibrary.open(global %d): Did not succeed in loading: '%s' within '%s'",
                      global, libName, jau::to_string(paths));
              return NativeLibrary(m_dynLink, nullptr, libName, global, symbolName);
            }
    };

    /**@}*/

} // namespace jau::os

