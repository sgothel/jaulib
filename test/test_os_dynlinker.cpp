/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2021 Gothel Software e.K.
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
#include <cassert>
#include <cinttypes>
#include <cstring>
#include <memory>
#include <thread>
#include <pthread.h>

#include <jau/test/catch2_ext.hpp>

#include <jau/file_util.hpp>
#include <jau/environment.hpp>
#include <jau/os/dyn_linker.hpp>

TEST_CASE( "Test 01 Global Open dlsym ..", "[dll][os]" ) {
    {
        std::string lib_path_var_name = jau::os::DynamicLinker::getEnvLibPathVarName();
        std::string lib_path_var = jau::environment::getProperty( lib_path_var_name );
        std::vector<std::string> lib_paths = jau::os::DynamicLinker::getSystemEnvLibraryPaths();
        std::cout << "- lib_path_var_name: " << lib_path_var_name << std::endl;
        std::cout << "- lib_path_var     : " << lib_path_var << std::endl;
        std::cout << "- lib_paths: count " << lib_paths.size() << std::endl << "  - path: ";
        std::cout << jau::to_string(lib_paths, "\n  - path: ") << std::endl;
    }
    const std::string libBasename = "jaulib";
    const std::string symbolName = "jaulib_id_entryfunc";
    const std::string libName = jau::os::DynamicLinker::getCanonicalName(libBasename);
    std::cout << "- libBasename: " << libBasename << std::endl;
    std::cout << "- libName: " << libName << std::endl;

    std::string libPath;
    {
        std::cout << "- cwd: " << jau::fs::get_cwd() << std::endl;
        std::cout << "- exe " << executable_path << std::endl << std::endl;

        libPath = jau::fs::dirname(executable_path) + "/../src/"+libName;

        jau::fs::file_stats path_stats(libPath);
        std::cout << "fs: " << path_stats.to_string() << std::endl << std::endl;
        if( !path_stats.exists() ) {
            std::cout << "Warning: library '" << libName << "' doesn't exist at: '" << libPath << "'" << std::endl;
            return;
        }
    }
    jau::os::DynamicLinker& dl = jau::os::DynamicLinker::get();
    jau::os::DynamicLinker::libhandle_t libHandle = dl.openLibraryGlobal(libPath);
    std::cout << "- Path: " << libPath << std::endl;
    std::cout << "- LibHandle: " << jau::to_hexstring(libHandle) << std::endl;
    REQUIRE( nullptr != libHandle );

    jau::os::DynamicLinker::symhandle_t symHandle = dl.lookupSymbol(libHandle, symbolName);
    std::cout << "- Symbol '" << symbolName << "': Handle = " << jau::to_hexstring(symHandle) << std::endl;
    REQUIRE( nullptr != symHandle );

    const char* nativePath = dl.lookupLibraryPathname(libHandle, symbolName);
    if( nullptr == nativePath ) {
        std::cout << "- Native Path: null" << std::endl;
    } else {
        std::cout << "- Native Path: '" << std::string(nativePath) << "'" << std::endl;
    }
    REQUIRE( nullptr != nativePath );

    {
        std::string bname1 = jau::os::DynamicLinker::getBaseName(libPath);
        std::cout << "- Basename (path): " << bname1 << std::endl;
        REQUIRE( libBasename == bname1 );
        if( nullptr != nativePath ) {
            std::string bname2 = jau::os::DynamicLinker::getBaseName(std::string(nativePath));
            std::cout << "- Basename (native-path): " << bname2 << std::endl;
            REQUIRE( libBasename == bname2 );
        }
    }
}
