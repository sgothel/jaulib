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
#include <cstring>
#include <pthread.h>

#include <jau/test/catch2_ext.hpp>

#include <jau/io/file_util.hpp>
#include <jau/environment.hpp>
#include <jau/os/dyn_linker.hpp>
#include <jau/os/native_lib.hpp>

using namespace jau::enums;

static bool existsPath(const std::string& libPath) noexcept {
    jau::io::fs::file_stats path_stats(libPath);
    return path_stats.exists();
}
static bool existsLibBasename(const std::string& libBasename, const std::string& relDir, std::string& libPath) noexcept {
    const std::string libName = jau::os::DynamicLinker::getCanonicalName(libBasename);
    libPath = jau::io::fs::dirname(executable_path) + "/" + relDir + "/"+libName;
    return existsPath(libPath);
}

// Test 00 Move testlib.* -> orig/, copy orig/testlib.so.1.2.3 to copy/testlib2.so
TEST_CASE( "Test00", "[dll][os]" ) {
    // Paranoia constraints!
    //
    // First move testlib.* into a new sub-folder to NOT have system linker:
    // - find it at cwd
    //
    // Second copy testlib.so into a new file to NOT have system linker:
    // - reuse the already linked native library
    // - the path location of the already linked native library
    //
    // Result:
    // - test_dir/orig/libtest.so* (all files w/ symlinks)
    // - test_dir/copy/libtest2.so  (single file)
    //
    const std::string libBasename = "testlib";
    const std::string libName = jau::os::DynamicLinker::getCanonicalName(libBasename);
    const std::string libPathBuild = jau::io::fs::absolute( jau::io::fs::dirname(executable_path) ) + "/"+libName;
    const std::string libPathOrig = jau::io::fs::absolute( jau::io::fs::dirname(executable_path) ) + "/orig/"+libName;
    if( !existsPath(libPathBuild) ) {
        if( !existsPath(libPathOrig) ) {
            std::cout << "Warning: library '" << libBasename << "' doesn't exist at: build '" << libPathBuild << "', nor at orig '" << libPathOrig << "'" << std::endl;
            return;
        }
        // OK, already moved
    } else {
        // move test_dir/testlib.* to test_dir/orig/testlib.* (overwrite)
        jau::io::fs::file_stats path_stats(libPathBuild);
        const std::string libPath = path_stats.final_target()->path();
        const std::string libDir = jau::io::fs::dirname(libPath);
        const std::string libDirOrig = libDir + "/orig";

        // recreate test_dir/orig
        std::cout << "remove: " << libDirOrig << std::endl;
        jau::io::fs::remove(libDirOrig, jau::io::fs::traverse_options::recursive | jau::io::fs::traverse_options::verbose);
        REQUIRE( true == jau::io::fs::mkdir(libDirOrig, jau::io::fs::fmode_t::def_dir_prot, true) );

        // move all libName* from test_dir to test_dir/orig at depth 1 (flat)
        {
            jau::io::fs::traverse_options topts = jau::io::fs::traverse_options::recursive |
                                              jau::io::fs::traverse_options::dir_check_entry |
                                              jau::io::fs::traverse_options::verbose;
            std::cout << "move: libs in '" << libDir << "' to '" << libDirOrig << "'" << std::endl;
            const jau::io::fs::path_visitor pv = [&](jau::io::fs::traverse_event tevt, const jau::io::fs::file_stats& element_stats, size_t depth) -> bool {
                if( jau::io::fs::is_set(tevt, jau::io::fs::traverse_event::dir_check_entry) && depth > 1 ) {
                    std::cout << "- move: ignore entry depth[" << depth << "]" << element_stats.item().toString() << std::endl;
                    return false;
                }
                if( jau::io::fs::traverse_event::none != ( tevt & ( jau::io::fs::traverse_event::file | jau::io::fs::traverse_event::symlink ) ) ) { // at least one of: file + link
                    std::string bname = element_stats.item().basename();
                    if( bname.starts_with(libName) ) {
                        const std::string p = libDirOrig + "/" + bname;
                        std::cout << "- move: depth[" << depth << "]: '" << element_stats.path() << "' to '" << p << "'" << std::endl;
                        jau::io::fs::rename(element_stats.path(), p);
                    }
                }
                return true;
            };
            REQUIRE( true == jau::io::fs::visit(libDir, topts, pv) );
            jau::io::fs::file_stats path_stats2(libPathOrig);
            std::cout << "post move: " << path_stats2.toString() << std::endl;
            REQUIRE( true == existsPath(libPathOrig) );
        }
    }
    // copy test_dir/orig/testlib.so.1.2.3 to test_dir/copy/testlib2.so
    {
        const jau::io::fs::copy_options copts = jau::io::fs::copy_options::preserve_all |
                                            jau::io::fs::copy_options::overwrite |
                                            jau::io::fs::copy_options::verbose;
        jau::io::fs::file_stats path_stats(libPathOrig);
        const std::string libPathOrigFile = path_stats.final_target()->path();

        const std::string libBasenameCopy = "testlib2";
        const std::string libNameCopy = jau::os::DynamicLinker::getCanonicalName(libBasenameCopy);
        const std::string libDirCopy = jau::io::fs::absolute( jau::io::fs::dirname(executable_path) ) + "/copy";

        // recreate test_dir/copy
        if( existsPath( libDirCopy ) ) {
            std::cout << "remove: " << libDirCopy << std::endl;
            jau::io::fs::remove(libDirCopy, jau::io::fs::traverse_options::recursive | jau::io::fs::traverse_options::verbose);
        }
        REQUIRE( true == jau::io::fs::mkdir(libDirCopy, jau::io::fs::fmode_t::def_dir_prot, true) );

        const std::string libPathCopy = libDirCopy + "/" + libNameCopy;
        REQUIRE( true == jau::io::fs::copy(libPathOrigFile, libPathCopy, copts) );
        REQUIRE( true == existsPath(libPathCopy) );
        std::string libPath2;
        REQUIRE( true == existsLibBasename(libBasenameCopy, "copy", libPath2) );
    }
}

static void test01DynamikLinkerAbs(const std::string& libBasename, const std::string& relDir) {
    const std::string symbolName = "jaulib_id_entryfunc";
    const std::string libName = jau::os::DynamicLinker::getCanonicalName(libBasename);
    std::cout << "- libBasename: " << libBasename << std::endl;
    std::cout << "- libName: " << libName << std::endl;

    std::string libPath;
    {
        std::cout << "- cwd: " << jau::io::fs::get_cwd() << std::endl;
        std::cout << "- exe " << executable_path << std::endl << std::endl;
        if( !existsLibBasename(libBasename, relDir, libPath) ) {
            std::cout << "Warning: library '" << libName << "' doesn't exist at: '" << libPath << "'" << std::endl;
            return;
        }
    }
    jau::os::DynamicLinker& dl = jau::os::DynamicLinker::get();
    jau::os::DynamicLinker::libhandle_t libHandle = dl.openLibraryLocal(libPath);
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

// Test 01 Local Open dlsym etc
TEST_CASE( "Test01", "[dll][os]" ) {
    {
        std::string lib_path_var_name = jau::os::DynamicLinker::getEnvLibPathVarName();
        std::string lib_path_var = jau::environment::getProperty( lib_path_var_name );
        std::vector<std::string> lib_paths = jau::os::DynamicLinker::getSystemEnvLibraryPaths();
        std::cout << "- lib_path_var_name: " << lib_path_var_name << std::endl;
        std::cout << "- lib_path_var     : " << lib_path_var << std::endl;
        std::cout << "- lib_paths: count : " << lib_paths.size() << std::endl << "  - path: ";
        std::cout << jau::to_string(lib_paths, "\n  - path: ") << std::endl;
    }
    test01DynamikLinkerAbs("testlib", "orig");
    test01DynamikLinkerAbs("testlib2", "copy");
}

static void test10NativeLibrary(const std::string& libBasename, const std::string& libDirRel) {
    const std::string symbolName = "jaulib_id_entryfunc";
    const std::string libName = jau::os::DynamicLinker::getCanonicalName(libBasename);
    std::cout << "- libBasename: " << libBasename << std::endl;
    std::cout << "- libName: " << libName << std::endl;
    const std::string exe_path_abs = jau::io::fs::absolute(executable_path);
    const std::string exe_dir = jau::io::fs::dirname(exe_path_abs);
    const std::string cwd = jau::io::fs::get_cwd();
    const std::string libPathRel = libDirRel + "/" + libName;
    std::string libDirAbs, libPathAbs;
    {
        std::cout << "- cwd: " << cwd << std::endl;
        std::cout << "- exe-rel " << executable_path << std::endl;
        std::cout << "- exe-abs " << exe_path_abs << std::endl << std::endl;
        {
            jau::io::fs::file_stats fs(exe_dir + "/" + libDirRel);
            REQUIRE( true == fs.exists() );
            libDirAbs = fs.final_target()->path();
        }
        libPathAbs = libDirAbs + "/" + libName;

        if( !existsPath(libPathAbs) ) {
            std::cout << "Warning: library '" << libName << "' doesn't exist at: '" << libPathAbs << "'" << std::endl;
            return;
        }
    }
    std::string lib_path_var_name = jau::os::DynamicLinker::getEnvLibPathVarName();
    std::string lib_path_var0 = jau::environment::getProperty( lib_path_var_name );
    {
        std::cout << "Sys-Path: '" << lib_path_var_name << "': Original" << std::endl;
        std::vector<std::string> lib_paths1 = jau::os::DynamicLinker::getSystemEnvLibraryPaths();
        std::cout << "- lib_path_var  (1) : " << lib_path_var0 << std::endl;
        std::cout << "- lib_paths: count  : " << lib_paths1.size() << std::endl << "  - path: '";
        std::cout << jau::to_string(lib_paths1, "'\n  - path: '") << "'" << std::endl;
    }

    // 1: Test with libPathAbs, no search
    {
        const std::string cwd2 = jau::io::fs::get_cwd();
        const bool searchSystemPath = false;
        const bool searchSystemPathFirst = false;
        const bool global = false;
        std::cout << "Check-1: Absolute Path: '" << libPathAbs << "', cwd2 '" << cwd2 << "'" << std::endl;
        jau::os::NativeLibrary nl = jau::os::NativeLibrary::open(libPathAbs, searchSystemPath, searchSystemPathFirst, global, symbolName);
        std::cout << "Check-1: " << nl.toString() << std::endl;
        REQUIRE( true == nl.isValid() );
        REQUIRE( true == nl.isOpen() );
        nl.close();
        REQUIRE( false == nl.isOpen() );
        REQUIRE( true == nl.isValid() );
    }

    // 2: Test with libPathRel, cd into test_exe path and search from cwd (no sys)
    {
        REQUIRE( true == jau::io::fs::chdir(exe_dir) );
        {
            const std::string cwd2 = jau::io::fs::get_cwd();
            const bool searchSystemPath = false;
            const bool searchSystemPathFirst = false;
            const bool global = false;
            std::cout << "Check-2: Relative Path to cwd: libPathRel '" << libPathRel << "', cwd2 '" << cwd2 << "'" << std::endl;
            jau::os::NativeLibrary nl = jau::os::NativeLibrary::open(libPathRel, searchSystemPath, searchSystemPathFirst, global, symbolName);
            std::cout << "Check-2: " << nl.toString() << std::endl;
            REQUIRE( true == nl.isValid() );
            REQUIRE( true == nl.isOpen() );
            nl.close();
            REQUIRE( false == nl.isOpen() );
            REQUIRE( true == nl.isValid() );
        }
        REQUIRE( true == jau::io::fs::chdir(cwd) );
    }

    // Add libDirAbs to orig sys-path
    {
        std::cout << "Sys-Path: '" << lib_path_var_name << "': Variant 1: With libDirAbs" << std::endl;
        std::string lib_path_var2;
        if( lib_path_var0.size() > 0 ) {
            lib_path_var2 = lib_path_var0+jau::os::path_separator()+libDirAbs;
        } else {
            lib_path_var2 = libDirAbs;
        }
        ::setenv(lib_path_var_name.c_str(), lib_path_var2.c_str(), 1 /* overwrite */);
        std::cout << "- lib_path_var set 2: " << lib_path_var2 << std::endl;

        std::string lib_path_var = jau::environment::getProperty( lib_path_var_name );
        std::vector<std::string> lib_paths = jau::os::DynamicLinker::getSystemEnvLibraryPaths();
        std::cout << "- lib_path_var get 2: " << lib_path_var << std::endl;
        std::cout << "- lib_paths: count  : " << lib_paths.size() << std::endl << "  - path: '";
        std::cout << jau::to_string(lib_paths, "'\n  - path: '") << "'" << std::endl;
    }

    // 10+11: Test with libBasename + libName, search from sys-path (incl. libDirAbs)
    {
        const bool searchSystemPath = true;
        const bool searchSystemPathFirst = true;
        const bool global = false;
        {
            const std::string cwd2 = jau::io::fs::get_cwd();
            std::cout << "Check-10: libBasename in sys: '" << libBasename << "', cwd2 '" << cwd2 << "'" << std::endl;
            jau::os::NativeLibrary nl = jau::os::NativeLibrary::open(libBasename, searchSystemPath, searchSystemPathFirst, global, symbolName);
            std::cout << "Check-10: " << nl.toString() << std::endl;
            REQUIRE( true == nl.isValid() );
            REQUIRE( true == nl.isOpen() );
            nl.close();
            REQUIRE( false == nl.isOpen() );
            REQUIRE( true == nl.isValid() );
        }
        {
            const std::string cwd2 = jau::io::fs::get_cwd();
            std::cout << "Check-11: libName in sys: '" << libName << "', cwd2 '" << cwd2 << "'" << std::endl;
            jau::os::NativeLibrary nl = jau::os::NativeLibrary::open(libName, searchSystemPath, searchSystemPathFirst, global, symbolName);
            std::cout << "Check-11: " << nl.toString() << std::endl;
            REQUIRE( true == nl.isValid() );
            REQUIRE( true == nl.isOpen() );
            nl.close();
            REQUIRE( false == nl.isOpen() );
            REQUIRE( true == nl.isValid() );
        }
    }

    // Add test_exe path to orig sys-path
    {
        std::cout << "Sys-Path: '" << lib_path_var_name << "': Variant 2: With test_exe path" << std::endl;
        std::string lib_path_var2;
        if( lib_path_var0.size() > 0 ) {
            lib_path_var2 = lib_path_var0+jau::os::path_separator()+exe_dir;
        } else {
            lib_path_var2 = exe_dir;
        }
        ::setenv(lib_path_var_name.c_str(), lib_path_var2.c_str(), 1 /* overwrite */);
        std::cout << "- lib_path_var set 3: " << lib_path_var2 << std::endl;

        std::string lib_path_var = jau::environment::getProperty( lib_path_var_name );
        std::vector<std::string> lib_paths = jau::os::DynamicLinker::getSystemEnvLibraryPaths();
        std::cout << "- lib_path_var get 3: " << lib_path_var << std::endl;
        std::cout << "- lib_paths: count  : " << lib_paths.size() << std::endl << "  - path: '";
        std::cout << jau::to_string(lib_paths, "'\n  - path: '") << "'" << std::endl;
    }

#if 1
    // 12: Test with libPathRel, search from sys-path (incl. text_exe path)
    {
        const bool searchSystemPath = true;
        const bool searchSystemPathFirst = true;
        const bool global = true;
        std::cout << "Check-12: Relative Path to cwd: " << libPathRel << std::endl;
        jau::os::NativeLibrary nl = jau::os::NativeLibrary::open(libPathRel, searchSystemPath, searchSystemPathFirst, global, symbolName);
        std::cout << "Check-12: " << nl.toString() << std::endl;
        REQUIRE( true == nl.isValid() );
        REQUIRE( true == nl.isOpen() );
        nl.close();
        REQUIRE( false == nl.isOpen() );
        REQUIRE( true == nl.isValid() );
    }
#endif
}

// Test 10 NativeLibrary Find / Open dlsym etc using orig filenames
TEST_CASE( "Test10", "[orig][NativeLibrary][dll][os]" ) {
    test10NativeLibrary("testlib", "orig");
}

// Test 10 NativeLibrary Find / Open dlsym etc using a copy w/ changed filename
TEST_CASE( "Test11", "[copy][NativeLibrary][dll][os]" ) {
    test10NativeLibrary("testlib2", "copy");
}
