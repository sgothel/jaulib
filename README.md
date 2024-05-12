# Jau Support Library (C++, Java, ...)

[Original document location](https://jausoft.com/cgit/jaulib.git/about/).

## Git Repository
This project's canonical repositories is hosted on [Gothel Software](https://jausoft.com/cgit/jaulib.git/).

## Goals
This project provides general C++ collections, algorithms and utilities.

`jaulib` was extracted from [Direct-BT](https://jausoft.com/cgit/direct_bt.git/about/) to enable general use and enforce better encapsulation,
now it is utilized in multiple projects ranging from cryptography with [Cipherpack](https://jausoft.com/cgit/cipherpack.git/about/), 
over-the-air (OTA) updates to computer graphics with [Gamp](https://jausoft.com/cgit/gamp.git/about/).

It also provides a basic mechanisms to create a thin Java JNI binding 
as well as some Java JNI bindings for a subset of `jaulib`.

### Status
Build and clang-tidy clean on C++20, passing all unit tests.

See [C++ Minimum Requirements](#cpp_min_req) and [Supported Platforms](#sup_platforms) for details.

## API Documentation
Up to date API documentation can be found:

* [C++ API Doc](https://jausoft.com/projects/jaulib/build/documentation/cpp/html/index.html) with [modules](https://jausoft.com/projects/jaulib/build/documentation/cpp/html/modules.html):
  * [Basic Algorithms](https://jausoft.com/projects/jaulib/build/documentation/cpp/html/group__Algorithms.html)
  * [Byte Utilities](https://jausoft.com/projects/jaulib/build/documentation/cpp/html/group__ByteUtils.html)
  * [C++ Language Utilities](https://jausoft.com/projects/jaulib/build/documentation/cpp/html/group__CppLang.html)
  * [Concurrency](https://jausoft.com/projects/jaulib/build/documentation/cpp/html/group__Concurrency.html)
  * [Codec](https://jausoft.com/projects/jaulib/build/documentation/cpp/html/group__Codec.html)
  * [Data Structures](https://jausoft.com/projects/jaulib/build/documentation/cpp/html/group__DataStructs.html)
  * [File Utilities](https://jausoft.com/projects/jaulib/build/documentation/cpp/html/group__FileUtils.html)
  * [Fraction Arithmetic and Time](https://jausoft.com/projects/jaulib/build/documentation/cpp/html/group__Fractions.html)
  * [Function Wrapper](https://jausoft.com/projects/jaulib/build/documentation/cpp/html/group__FunctionWrap.html)
  * [IO Utilities](https://jausoft.com/projects/jaulib/build/documentation/cpp/html/group__IOUtils.html)
  * [Java VM Utilities](https://jausoft.com/projects/jaulib/build/documentation/cpp/html/group__JavaVM.html)
  * [Math](https://jausoft.com/projects/jaulib/build/documentation/cpp/html/group__Math.html)
    * [Integer types and arithmetic](https://jausoft.com/projects/jaulib/build/documentation/cpp/html/group__Integer.html)
    * [Float types and arithmetic](https://jausoft.com/projects/jaulib/build/documentation/cpp/html/group__Floats.html)
    * [Constant Time (CT) Operations](https://jausoft.com/projects/jaulib/build/documentation/cpp/html/group__ConstantTime.html)
  * [OS Support](https://jausoft.com/projects/jaulib/build/documentation/cpp/html/group__OSSup.html)
  * [Network Utilities](https://jausoft.com/projects/jaulib/build/documentation/cpp/html/group__NetUtils.html)
  * [String Utilities](https://jausoft.com/projects/jaulib/build/documentation/cpp/html/group__StringUtils.html)
  * [System and OS Utilities](https://jausoft.com/projects/jaulib/build/documentation/cpp/html/group__SysUtils.html)

* [Java API Doc](https://jausoft.com/projects/jaulib/build/documentation/java/html/index.html).


## Examples
See *Direct-BT* [C++ API Doc](https://jausoft.com/projects/direct_bt/build/documentation/cpp/html/examples.html).

# Usage
* [Direct-BT](https://jausoft.com/cgit/direct_bt.git/about/)
* [Cipherpack](https://jausoft.com/cgit/cipherpack.git/about/)
* [Gamp](https://jausoft.com/cgit/gamp.git/about/)

<a name="cpp_min_req"></a>

## C++ Minimum Requirements
C++20 is the minimum requirement for releases > 1.2.0.

Release 1.2.0 is the last version supporting C++17, see [Changes](CHANGES.md).

Support for C++23 and C++26 will be added step by step.

### C++ Compiler Support
- C++20, see [C++20 compiler support](https://en.cppreference.com/w/cpp/compiler_support#cpp20)
  - gcc >= 11, recommended >= 12
  - clang >= 13, recommended >= 16

### Rational for C++20 Minimum
- Moving metaprogramming to C++20 concepts and constrains
  - `SFINAE` and its utilization in `type_traits` for C++ metaprogramming are great
  - C++20 constrains add an easier to read and code alternative using the same idea
  - C++20 concepts declare a set of C++20 constrains and can be reused, guarantees of same concept
  - `C++ Named Requirements` are defined as concepts next to `type traits` counterpart
  - Hence moving step by step to C++20 concepts helps with maintainability
- Lack of C++17 `constexpr` completeness in the `STL` (e.g. `std::string`)
- Used compiler `gcc` and `clang` have matured enough for C++20 in 2024

<a name="sup_platforms"></a>

## Supported Platforms
Language requirements
- C++20 or better, see [C++ Minimum Requirements](#cpp_min_req)
- Standard C Libraries
  - [FreeBSD libc](https://www.freebsd.org/)
  - [GNU glibc](https://www.gnu.org/software/libc/)
  - [musl](https://musl.libc.org/)
- Java 11, 17+ (optional)

See [supported platforms](PLATFORMS.md) for details.

## Building Binaries
It is advised to include this library into your main project, e.g. as a git-submodule.

Then add *jaulib/include/* to your C++ include-path and also add the C++ source files
under *jaulib/src/* into your build recipe.

The produced Java libraries are fully functional.

This library's build recipe are functional though, 
but currently only intended to support unit testing and to produce a Doxygen API doc.

### Build Dependencies
- CMake >= 3.21 (2021-07-14)
- C++ compiler
  - gcc >= 11 (C++20), recommended >= 12
  - clang >= 13 (C++20), recommended >= 16
- Optional for `lint` validation
  - clang-tidy >= 16
- Optional for `eclipse` and `vscodium` integration
  - clangd >= 16
  - clang-tools >= 16
  - clang-format >= 16
- Optional
  - libunwind8 >= 1.2.1
  - libcurl4 >= 7.74 (tested, lower may work)
- Optional Java support
  - OpenJDK >= 11
  - junit4 >= 4.12

#### Install on FreeBSD

Installing build dependencies on FreeBSD >= 13:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.sh}
pkg install git
pkg install sudo
pkg install cmake
pkg install libunwind
pkg install doxygen
pkg install squashfs-tools
pkg install bash
ln -s /usr/local/bin/bash /bin/bash
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Install optional Java dependencies:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.sh}
pkg install openjdk17
pkg install openjdk17-jre
pkg install junit
rehash
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

For Java ensure `/etc/fstab` includes:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.sh}
fdesc   /dev/fd         fdescfs         rw      0       0
proc    /proc           procfs          rw      0       0
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

`jau::fs::mount_image()` and `jau::fs::umount()` are currenly not
fully implemented under `FreeBSD`,
hence testing using cmake option `-DTEST_WITH_SUDO=ON` is disabled. <br />

To use URL streaming functionality via the `curl` library in `jau_io_util.hpp` and `jau/io_util.cpp`,
the cmake option `-DUSE_LIBCURL=ON` must be set. <br />
This also requires installation of the following packets:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.sh}
pkg install curl
apt install mini-httpd
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Note: `mini-httpd` is being used for unit testing URL streaming only.

#### Install on Debian or Ubuntu

Installing build dependencies on Debian >= 11 and Ubuntu >= 20.04:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.sh}
apt install git
apt install build-essential g++ gcc libc-dev libpthread-stubs0-dev 
apt install clang-16 clang-tidy-16 clangd-16 clang-tools-16 clang-format-16
apt install libunwind8 libunwind-dev
apt install cmake cmake-extras extra-cmake-modules
apt install doxygen graphviz
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If using the optional clang toolchain, 
perhaps change the clang version-suffix of above clang install line to the appropriate version.

After complete clang installation, you might want to setup the latest version as your default.
For Debian you can use this [clang alternatives setup script](https://jausoft.com/cgit/jaulib.git/tree/scripts/setup_clang_alternatives.sh).

Install optional Java dependencies:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.sh}
apt install openjdk-17-jdk openjdk-17-jre junit4
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To test `jau::fs::mount_image()` and `jau::fs::umount()` under `Linux`
with enabled cmake option `-DTEST_WITH_SUDO=ON`, <br />
the following build dependencies are added
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.sh}
apt install libcap-dev libcap2-bin
apt install squashfs-tools
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To use URL streaming functionality via the `curl` library in `jau_io_util.hpp` and `jau/io_util.cpp`,
the cmake option `-DUSE_LIBCURL=ON` must be set. <br />
This also requires installation of the following packets:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.sh}
apt install libcurl4 libcurl4-gnutls-dev
apt install mini-httpd
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Note: `mini-httpd` is being used for unit testing URL streaming only.

### Build Procedure

#### Build preparations

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.sh}
git clone https://jausoft.com/cgit/jaulib.git
cd jaulib
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

<a name="cmake_presets_optional"></a>

#### CMake Build via Presets
Following debug presets are defined in `CMakePresets.json`
- `debug`
  - default generator
  - default compiler
  - C++20
  - debug enabled
  - java (if available)
  - libunwind (if available)
  - libcurl (if available)
  - testing on
  - testing with sudo off
- `debug-gcc`
  - inherits from `debug`
  - compiler: `gcc`
  - disabled `clang-tidy`
- `debug-clang`
  - inherits from `debug`
  - compiler: `clang`
  - enabled `clang-tidy`
- `release`
  - inherits from `debug`
  - debug disabled
  - testing with sudo on
- `release-gcc`
  - compiler: `gcc`
  - disabled `clang-tidy`
- `release-clang`
  - compiler: `clang`
  - enabled `clang-tidy`

Kick-off the workflow by e.g. using preset `release-gcc` to configure, build, test, install and building documentation.
You may skip `install` and `doc_jau` by dropping it from `--target`.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.sh}
cmake --preset release-gcc
cmake --build --preset release-gcc --parallel
cmake --build --preset release-gcc --target test install doc_jau
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

<a name="cmake_presets_hardcoded"></a>

#### CMake Build via Hardcoded Presets
Besides above `CMakePresets.json` presets, 
`JaulibSetup.cmake` contains hardcoded presets for *undefined variables* if
- `CMAKE_INSTALL_PREFIX` and `CMAKE_CXX_CLANG_TIDY` cmake variables are unset, or 
- `JAU_CMAKE_ENFORCE_PRESETS` cmake- or environment-variable is set to `TRUE` or `ON`

The hardcoded presets resemble `debug-clang` [presets](README.md#cmake_presets_optional).

Kick-off the workflow to configure, build, test, install and building documentation.
You may skip `install` and `doc_jau` by dropping it from `--target`.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.sh}
rm -rf build/default
cmake -B build/default
cmake --build build/default --parallel
cmake --build build/default --target test install doc_jau
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The install target of the last command will create the include/ and lib/ directories with a copy of
the headers and library objects respectively in your dist location.

<a name="cmake_variables"></a>

#### CMake Variables
Our cmake configure has a number of options, *cmake-gui* or *ccmake* can show
you all the options. The interesting ones are detailed below:

`JaulibPreset` cached variables for hardcoded presets are
- `CMAKE_BUILD_TYPE`
- `BUILD_TESTING`
- `CMAKE_C_COMPILER`
- `CMAKE_CXX_COMPILER`
- `CMAKE_CXX_CLANG_TIDY`
- `CMAKE_CXX_STANDARD`

`JaulibSetup` cached variables for regular builds are
- `DEBUG`
- `CMAKE_INSTALL_PREFIX`
- `CMAKE_CXX_STANDARD`
- `USE_LIBCURL`
- `USE_LIBUNWIND`
- `BUILDJAVA`

Changing install path
~~~~~~~~~~~~~
-DCMAKE_INSTALL_PREFIX=/somewhere/dist-jaulib
~~~~~~~~~~~~~

Building debug build:
~~~~~~~~~~~~~
-DDEBUG=ON
~~~~~~~~~~~~~
or
~~~~~~~~~~~~~
-DCMAKE_BUILD_TYPE=Debug
~~~~~~~~~~~~~

Enable/disable unit tests to build
~~~~~~~~~~~~~
-DBUILD_TESTING=ON
~~~~~~~~~~~~~

Add unit tests requiring `sudo` to build (default: disabled).<br />
This option requires `-DBUILD_TESTING=ON` to be effective.<br />
Covered unit test requiring `sudo` are currently 
- `Linux` OS
  - `jau::fs::mount_image()`
  - `jau::fs::umount()`
~~~~~~~~~~~~~
-DTEST_WITH_SUDO=ON
~~~~~~~~~~~~~

Using clang instead of gcc:
~~~~~~~~~~~~~
-DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++
~~~~~~~~~~~~~

Building with clang and clang-tidy `lint` validation
~~~~~~~~~~~~~
-DCMAKE_C_COMPILER=/usr/bin/clang 
-DCMAKE_CXX_COMPILER=/usr/bin/clang++ 
-DCMAKE_CXX_CLANG_TIDY=/usr/bin/clang-tidy;-p;$rootdir/$build_dir
~~~~~~~~~~~~~

Disable stripping native lib even in non debug build:
~~~~~~~~~~~~~
-DUSE_STRIP=OFF
~~~~~~~~~~~~~

Enable using `libcurl` (default: disabled)
~~~~~~~~~~~~~
-DUSE_LIBCURL=ON
~~~~~~~~~~~~~

Enable using `libunwind` (default: disabled)
~~~~~~~~~~~~~
-DUSE_LIBUNWIND=ON
~~~~~~~~~~~~~

Disable using `C++ Runtime Type Information` (*RTTI*) (default: enabled)
~~~~~~~~~~~~~
-DDONT_USE_RTTI=ON
~~~~~~~~~~~~~

Building debug and instrumentation (sanitizer) build:
~~~~~~~~~~~~~
-DDEBUG=ON -DINSTRUMENTATION=ON
~~~~~~~~~~~~~

Cross-compiling on a different system:
~~~~~~~~~~~~~
-DCMAKE_CXX_FLAGS:STRING=-m32 -march=i586
-DCMAKE_C_FLAGS:STRING=-m32 -march=i586
~~~~~~~~~~~~~

To build documentation run: 
~~~~~~~~~~~~~
make doc
~~~~~~~~~~~~~

To build Java bindings:
~~~~~~~~~~~~~
-DBUILDJAVA=ON
~~~~~~~~~~~~~

Override default javac debug arguments `source,lines`:
~~~~~~~~~~~~~
-DJAVAC_DEBUG_ARGS="source,lines,vars"

-DJAVAC_DEBUG_ARGS="none"
~~~~~~~~~~~~~

#### Deprecated Build Scripts
You may also invoke `scripts/build.sh`,
which resolves installed environment variables like `JAVA_HOME` and `JUNIT_CP`
as well as building and distributing using `os_arch` type folders.
- `scripts/setup-machine-arch.sh` .. generic setup for all scripts
- `scripts/build.sh` .. initial build incl. install and unit testing
- `scripts/rebuild.sh` .. rebuild
- `scripts/build-cross.sh` .. [cross-build](#cross-build)
- `scripts/rebuild-cross.sh` .. [cross-build](#cross-build)
- `scripts/test_java.sh` .. invoke a java unit test
- `scripts/test_exe_template.sh` .. invoke the symlink'ed files to invoke native unit tests

### Cross Build
Also provided is a [cross-build script](https://jausoft.com/cgit/jaulib.git/tree/scripts/build-cross.sh)
using chroot into a target system using [QEMU User space emulation](https://qemu-project.gitlab.io/qemu/user/main.html)
and [Linux kernel binfmt_misc](https://wiki.debian.org/QemuUserEmulation)
to run on other architectures than the host.

See [Build Procedure](#build-procedure) for general overview.

You may use [our pi-gen branch](https://jausoft.com/cgit/pi-gen.git/about/) to produce 
a Raspi-arm64, Raspi-armhf or PC-amd64 target image.

## IDE Integration

### Eclipse 
Tested Eclipse 2024-03 (4.31).

IDE integration configuration files are provided for 
- [Eclipse](https://download.eclipse.org/eclipse/downloads/) with extensions
  - [CDT](https://github.com/eclipse-cdt/) or [CDT @ eclipse.org](https://projects.eclipse.org/projects/tools.cdt)
  - [CDT-LSP](https://github.com/eclipse-cdt/cdt-lsp) *recommended*
    - Should work with clang toolchain >= 16
    - Utilizes clangd, clang-tidy and clang-format to support C++20 and above
    - Add to available software site: `https://download.eclipse.org/tools/cdt/releases/cdt-lsp-latest`
    - Install `C/C++ LSP Support` in the `Eclipse CDT LSP Category`
  - `CMake Support`, install `C/C++ CMake Build Support` with ID `org.eclipse.cdt.cmake.feature.group`
    - Usable via via [Hardcoded CMake Presets](README.md#cmake_presets_hardcoded) with `debug-clang`

The [Hardcoded CMake Presets](README.md#cmake_presets_hardcoded) will 
use `build/default` as the default build folder with debug enabled.

Make sure to set the environment variable `CMAKE_BUILD_PARALLEL_LEVEL`
to a suitable high number, best to your CPU core count.
This will enable parallel build with the IDE.

You can import the project to your workspace via `File . Import...` and `Existing Projects into Workspace` menu item.

For Eclipse one might need to adjust some setting in the `.project` and `.cproject` (CDT) 
via Eclipse settings UI, but it should just work out of the box.

Otherwise recreate the Eclipse project by 
- delete `.project` and `.cproject` 
- `File . New . C/C++ Project` and `Empty or Existing CMake Project` while using this project folder.

### VSCodium or VS Code

IDE integration configuration files are provided for 
- [VSCodium](https://vscodium.com/) or [VS Code](https://code.visualstudio.com/) with extensions
  - [vscode-clangd](https://github.com/clangd/vscode-clangd)
  - [twxs.cmake](https://github.com/twxs/vs.language.cmake)
  - [ms-vscode.cmake-tools](https://github.com/microsoft/vscode-cmake-tools)
  - [notskm.clang-tidy](https://github.com/notskm/vscode-clang-tidy)
  - Java Support
    - [redhat.java](https://github.com/redhat-developer/vscode-java#readme)
      - Notable, `.settings/org.eclipse.jdt.core.prefs` describes the `lint` behavior
    - [vscjava.vscode-java-test](https://github.com/Microsoft/vscode-java-test)
    - [vscjava.vscode-java-debug](https://github.com/Microsoft/java-debug)
    - [vscjava.vscode-maven](https://github.com/Microsoft/vscode-maven/)
  - [cschlosser.doxdocgen](https://github.com/cschlosser/doxdocgen)
  - [jerrygoyal.shortcut-menu-bar](https://github.com/GorvGoyl/Shortcut-Menu-Bar-VSCode-Extension)

For VSCodium one might copy the [example root-workspace file](https://jausoft.com/cgit/jaulib.git/tree/.vscode/jaulib.code-workspace_example)
to the parent folder of this project (*note the filename change*) and adjust the `path` to your filesystem.
~~~~~~~~~~~~~
cp .vscode/jaulib.code-workspace_example ../jaulib.code-workspace
vi ../jaulib.code-workspace
~~~~~~~~~~~~~
Then you can open it via `File . Open Workspace from File...` menu item.
- All listed extensions are referenced in this workspace file to be installed via the IDE
- The [local settings.json](.vscode/settings.json) has `clang-tidy` enabled
  - If using `clang-tidy` is too slow, just remove it from the settings file.
  - `clangd` will still contain a good portion of `clang-tidy` checks

## Changes

See [Changes](CHANGES.md).

