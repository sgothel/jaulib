/*
 * Author: Sven Gothel <sgothel@jausoft.com>
 * Copyright (c) 2020-2024 Gothel Software e.K.
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

#include <cstdint>
#include <cinttypes>
#include <cstring>

#include <ctime>

#include <algorithm>

#include <jau/debug.hpp>
#include <jau/file_util.hpp>
#include <jau/os/dyn_linker.hpp>
#include <jau/os/os_support.hpp>
#include <jau/os/user_info.hpp>

#if !defined(_WIN32)
    #include <sys/utsname.h>
#endif

using namespace jau;

std::string jau::os::to_string(const os_type v) noexcept {
    switch(v) {
        case os_type::Unix: return "Unix";
        case os_type::Windows: return "Windows";
        case os_type::Linux: return "Linux";
        case os_type::Android: return "Android";
        case os_type::FreeBSD: return "FreeBSD";
        case os_type::Darwin: return "Darwin";
        case os_type::QnxNTO: return "QNX-NTO";
        case os_type::WebAsm: return "WebAsm";
        case os_type::UnixWasm: return "UnixWasm";
    }
    return "undef";
}

bool jau::os::get_rt_os_info(rt_os_info& info) noexcept {
    #if !defined(_WIN32)
        struct utsname uinfo;
        if( 0 == ::uname(&uinfo) ) {
            info.sysname = std::string(uinfo.sysname);
            info.nodename = std::string(uinfo.nodename);
            info.release = std::string(uinfo.release);
            info.version = std::string(uinfo.version);
            info.machine = std::string(uinfo.machine);
            #if defined(_GNU_SOURCE)
                info.domainname = std::string(uinfo.domainname);
            #endif
            return true;
        }
    #endif
    return false;
}

std::string jau::os::to_string(const abi_type v) noexcept {
    switch(v) {
        case abi_type::generic_abi: return "generic_abi";
        case abi_type::eabi_gnu_armel: return "gnu_armel_abi";
        case abi_type::eabi_gnu_armhf: return "gnu_armhf_abi";
        case abi_type::eabi_aarch64: return "aarch64_abi";
        case abi_type::wasm32_abi_undef: return "wasm32_undef_abi";
        case abi_type::wasm32_abi_emscripten: return "wasm32_emscripten_abi";
        case abi_type::wasm64_abi_undef: return "wasm64_undef_abi";
        case abi_type::wasm64_abi_emscripten: return "wasm64_emscripten_abi";
    }
    return "undef";
}

std::string jau::os::get_os_and_arch(const os_type os, const jau::cpu::cpu_family cpu, const abi_type abi, const endian e) noexcept {
    std::string os_;
    std::string _and_arch_tmp, _and_arch_final;

    switch( cpu ) {
        case jau::cpu::cpu_family::arm32:
            if( abi_type::eabi_gnu_armhf == abi ) {
                _and_arch_tmp = "armv6hf";
            } else {
                _and_arch_tmp = "armv6";
            }
            break;
        case jau::cpu::cpu_family::x86_32:
            _and_arch_tmp = "i586";
            break;
        case jau::cpu::cpu_family::ppc_32:
            _and_arch_tmp = "ppc";
            break;
        case jau::cpu::cpu_family::mips_32:
            _and_arch_tmp = jau::is_little_endian(e) ? "mipsel" : "mips";
            break;
        case jau::cpu::cpu_family::sparc_32:
            _and_arch_tmp = "sparc";
            break;
        case jau::cpu::cpu_family::superh_32:
            _and_arch_tmp = "superh";
            break;

        case jau::cpu::cpu_family::arm64:
            _and_arch_tmp = "aarch64";
            break;
        case jau::cpu::cpu_family::x86_64:
            _and_arch_tmp = "amd64";
            break;
        case jau::cpu::cpu_family::ppc_64:
            _and_arch_tmp = jau::is_little_endian(e) ? "ppc64le" : "ppc64";
            break;
        case jau::cpu::cpu_family::mips_64:
            _and_arch_tmp = "mips64";
            break;
        case jau::cpu::cpu_family::ia64:
            _and_arch_tmp = "ia64";
            break;
        case jau::cpu::cpu_family::sparc_64:
            _and_arch_tmp = "sparcv9";
            break;
        case jau::cpu::cpu_family::superh_64:
            _and_arch_tmp = "superh64";
            break;
        case jau::cpu::cpu_family::wasm_32:
            _and_arch_tmp = "wasm32";
            break;
        case jau::cpu::cpu_family::wasm_64:
            _and_arch_tmp = "wasm64";
            break;
        default:
            _and_arch_tmp = "undef_arch";
            break;
    }

    switch( os ) {
        case os_type::Android:
          os_ = "android";
          _and_arch_final = _and_arch_tmp;
          break;
        case os_type::Darwin:
          os_ = "darwin";
          _and_arch_final = "universal";
          break;
        case os_type::Windows:
          os_ = "windows";
          _and_arch_final = _and_arch_tmp;
          break;
        case os_type::Linux:
          os_ = "linux";
          _and_arch_final = _and_arch_tmp;
          break;
        case os_type::FreeBSD:
          os_ = "freebsd";
          _and_arch_final = _and_arch_tmp;
          break;
        case os_type::QnxNTO:
          os_ = "qnxnto";
          _and_arch_final = _and_arch_tmp;
          break;
        case os_type::WebAsm:
          os_ = "webasm";
          _and_arch_final = _and_arch_tmp;
          break;
        case os_type::UnixWasm:
          os_ = "unixwasm";
          _and_arch_final = _and_arch_tmp;
          break;
        default:
          os_ = "undef_os";
          _and_arch_final = _and_arch_tmp;
          break;
    }
    return os_ + "-" + _and_arch_final;
}

std::string jau::os::get_platform_info(std::string& sb) noexcept {
    const jau::os::os_type os = jau::os::os_type::native;
    const jau::cpu::cpu_family cpu = jau::cpu::get_cpu_family();
    const jau::os::abi_type abi = jau::os::get_abi_type();
    const jau::endian byte_order = jau::endian::native;
    jau::os::rt_os_info rti;
    bool rti_ok = jau::os::get_rt_os_info(rti);

    size_t cores = 0;
    sb.append( jau::format_string("Platform: %s %s, %s (%s, %s endian, %zu bits), %zu cores, %s\n",
            jau::os::to_string(os).c_str(),
            ( rti_ok ? rti.release.c_str() : ""),
            jau::cpu::to_string(cpu).c_str(),
            jau::os::to_string(abi).c_str(),
            jau::to_string(byte_order).c_str(),
            jau::cpu::get_arch_psize(),
            cores,
            jau::os::get_os_and_arch(os, cpu, abi, byte_order).c_str()) );
    jau::cpu::get_cpu_info("- cpu_info: ", sb);

    if( rti_ok ) {
        sb.append("- runtime: ").append(rti.to_string()).append("\n");
    }
    return sb;
}

static
std::pair<std::string, bool>
DynamicLinker_processCanonicalNameImpl(bool strip, const std::string& filename, const bool isBasename,
                                       const bool caseInsensitive=jau::os::is_windows()) noexcept
{
  const std::string prefix = jau::os::DynamicLinker::getDefaultPrefix();
  const std::string suffix = jau::os::DynamicLinker::getDefaultSuffix();
  const std::string libBaseName = isBasename ? filename : jau::fs::basename(filename);
  const std::string libBaseNameLC = caseInsensitive ? jau::toLower(libBaseName) : libBaseName;
  const size_t pre_idx = libBaseNameLC.find(prefix);
  if( 0 == pre_idx ) { // starts-with
      const size_t sfx_idx = libBaseNameLC.rfind(suffix);
      if( std::string::npos != sfx_idx ) {
          // Check to see if everything after it is a Unix version number
          bool ok = true;
          for (size_t i = sfx_idx + suffix.length(); i < libBaseName.size(); i++) {
              const char c = libBaseName[i];
              if (!(c == '.' || (c >= '0' && c <= '9'))) {
                  ok = false;
                  break;
              }
          }
          if (ok) {
              std::string res;
              if( strip ) {
                  const size_t sfx_len = libBaseName.size() - sfx_idx;
                  res = libBaseName.substr(prefix.size(), libBaseName.size() - prefix.size() - sfx_len);
              }
              return std::make_pair(res, true);
          }
      }
  }
  return std::make_pair(std::string(), false);
}

std::string jau::os::DynamicLinker::getBaseName(const std::string& filename, const bool isBasename, const bool caseInsensitive) noexcept {
  return DynamicLinker_processCanonicalNameImpl(true, filename, isBasename, caseInsensitive).first;
}

bool jau::os::DynamicLinker::isCanonicalName(const std::string& filename, const bool isBasename, const bool caseInsensitive) noexcept {
  return DynamicLinker_processCanonicalNameImpl(false, filename, isBasename, caseInsensitive).second;
}

static std::vector<std::string> DynamicLinker_buildNames(const std::string& libName) noexcept {
  std::vector<std::string> res;

  const std::string libBaseName = jau::fs::basename(libName);
  if( jau::os::DynamicLinker::isCanonicalName(libBaseName, true) ) {
      // basename is canonical, so use the original with leading path
      res.push_back(libName);
      return res;
  }

  res.push_back( jau::os::DynamicLinker::getCanonicalName(libBaseName, false) );
  if ( jau::os::is_darwin() ) {
      // Plain library-base-name in Framework folder
      res.push_back(libBaseName);
  }
  return res;
}

static void DynamicLinker_addBasenames(const std::string& cause, const std::vector<std::string>& baseNames, std::vector<std::string>& paths) noexcept {
  for (const std::string& baseName : baseNames) {
      DBG_PRINT("NativeLibrary.enumerateLibraryPaths: %s: '%s'", cause.c_str(), baseName.c_str());
      paths.push_back(baseName);
  }
}
static void DynamicLinker_addAbsPaths(const std::string& cause, const std::string& abs_path, const std::vector<std::string>& baseNames, std::vector<std::string>& paths) noexcept {
  for (const std::string& baseName : baseNames) {
      std::string p(abs_path); p.append("/").append(baseName);
      DBG_PRINT("NativeLibrary.enumerateLibraryPaths: %s: '%s', from path '%s'", cause.c_str(), p.c_str(), abs_path.c_str());
      paths.push_back(p);
  }
}
static void DynamicLinker_addSysPaths(const std::string& cause, const std::vector<std::string>& baseNames, std::vector<std::string>& paths) noexcept {
    // First add just the library names to use the OS's search algorithm
    DynamicLinker_addBasenames(cause, baseNames, paths);

    // Second add full path for each sys-folder to overcome SONAME mismatch (OS's search algorithm)
    std::vector<std::string> lib_paths = jau::os::DynamicLinker::getSystemEnvLibraryPaths();
    for(const std::string& p : lib_paths) {
        DynamicLinker_addAbsPaths(cause, p, baseNames, paths);
    }
}
#if 0
static void DynamicLinker_addRelPaths(const std::string& cause, const std::string& path, const std::vector<std::string>& baseNames, std::vector<std::string>& paths) noexcept {
  std::string abs_path;
  jau::fs::file_stats path_stats(path);
  if( path_stats.exists() ) {
      DynamicLinker_addAbsPaths(cause, path_stats.path(), baseNames, paths);
  }
}
#endif

std::vector<std::string> jau::os::DynamicLinker::enumerateLibraryPaths(const std::string& libName,
                                                                       bool searchSystemPath,
                                                                       bool searchSystemPathFirst) noexcept {
    DBG_PRINT("DynamicLibrary.enumerateLibraryPaths: libName '%s'", libName.c_str());
    std::vector<std::string> paths;
    if ( 0 == libName.size() ) {
        return paths;
    }

    // Allow user's full path specification to override our building of paths
    if ( jau::fs::isAbsolute(libName) ) {
        paths.push_back(libName);
        DBG_PRINT("NativeLibrary.enumerateLibraryPaths: done, absolute path found '%s'", libName.c_str());
        return paths;
    }

    std::vector<std::string> baseNames = DynamicLinker_buildNames(libName);
    DBG_PRINT("NativeLibrary.enumerateLibraryPaths: baseNames: %s", jau::to_string(baseNames).c_str());

    if( searchSystemPath && searchSystemPathFirst ) {
        DynamicLinker_addSysPaths("add.ssp_1st", baseNames, paths);

        // Add probable Mac OS X-specific paths
        if ( jau::os::is_darwin() ) {
            // Add historical location
            DynamicLinker_addAbsPaths("add.ssp_1st_macos_old", "/Library/Frameworks/" + libName + ".framework", baseNames, paths);
            // Add current location
            DynamicLinker_addAbsPaths("add.ssp_1st_macos_cur", "/System/Library/Frameworks/" + libName + ".framework", baseNames, paths);
        }
    }

    // Add current working directory
    {
        std::string cwd = jau::fs::get_cwd();
        DynamicLinker_addAbsPaths("add.cwd", cwd, baseNames, paths);

        // Add current working directory + natives/os-arch/ + library names (for unpacked archives, if exists)
        const std::string cwd_bin = cwd+"/natives/"+jau::os::get_os_and_arch();
        jau::fs::file_stats fstats(cwd_bin);
        if( fstats.exists() ) {
            DynamicLinker_addAbsPaths("add.cwd.natives.os_arch", cwd_bin, baseNames, paths);
        }
    }

    // Add user directory
    {
        jau::os::UserInfo user;
        if( user.isValid() ) {
            DynamicLinker_addAbsPaths("add.home.std", user.homedir(), baseNames, paths);

            // Add current home + bin/os-arch/ + library names (if exists)
            const std::string home_bin = user.homedir()+"/bin/"+jau::os::get_os_and_arch();
            jau::fs::file_stats fstats(home_bin);
            if( fstats.exists() ) {
                DynamicLinker_addAbsPaths("add.home.bin.os_arch", home_bin, baseNames, paths);
            }
        }
    }

    if( searchSystemPath && !searchSystemPathFirst ) {
        DynamicLinker_addSysPaths("add.ssp_lst", baseNames, paths);

        // Add probable Mac OS X-specific paths
        if ( jau::os::is_darwin() ) {
            // Add historical location
            DynamicLinker_addAbsPaths("add.ssp_lst_macos_old", "/Library/Frameworks/" + libName + ".framework", baseNames, paths);
            // Add current location
            DynamicLinker_addAbsPaths("add.ssp_lst_macos_cur", "/System/Library/Frameworks/" + libName + ".framework", baseNames, paths);
        }
    }
    DBG_PRINT("NativeLibrary.enumerateLibraryPaths: done: %s", jau::to_string(paths).c_str());
    return paths;
}

