# Jau Support Library (C++, Java, ...)

[Original document location](https://jausoft.com/cgit/jaulib.git/about/).

## Git Repository
This project's canonical repositories is hosted on [Gothel Software](https://jausoft.com/cgit/jaulib.git/).

## Goals
This project aims to provide general C++ and Java collections, algorithms and utilities - as well as basic concepts to support a Java JNI binding.

This project was extracted from [Direct-BT](https://jausoft.com/cgit/direct_bt.git/about/) to enable general use and enforce better encapsulation.

### Status
Build and clang-tidy clean on C++17 and C++20, passing all unit tests.

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

## Supported Platforms
Language requirements
- C++17 or C++20
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
- CMake 3.13+ but >= 3.18 is recommended
- C++ compiler
  - gcc >= 8.3.0 (C++17)
  - gcc >= 10.2.1 (C++17 and C++20)
  - clang >= 15 (C++17 and C++20)
- Optional for `lint` validation
  - clang-tidy >= 15
- Optional for `vscodium` integration
  - clangd >= 15
  - clang-tools >= 15
  - clang-format >= 15
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
apt install clang-15 clang-tidy-15 clangd-15 clang-tools-15 clang-format-15
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

For a generic build use:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.sh}
CPU_COUNT=`getconf _NPROCESSORS_ONLN`
git clone https://jausoft.com/cgit/jaulib.git
cd jaulib
mkdir build
cd build
cmake -DBUILDJAVA=ON -DBUILDEXAMPLES=ON -DBUILD_TESTING=ON ..
make -j $CPU_COUNT install
make test
make doc
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The install target of the last command will create the include/ and lib/ directories with a copy of
the headers and library objects respectively in your build location. Note that
doing an out-of-source build may cause issues when rebuilding later on.

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

Our cmake configure has a number of options, *cmake-gui* or *ccmake* can show
you all the options. The interesting ones are detailed below:

Changing install path from /usr/local to /usr
~~~~~~~~~~~~~
-DCMAKE_INSTALL_PREFIX=/usr
~~~~~~~~~~~~~

Building debug build:
~~~~~~~~~~~~~
-DDEBUG=ON
~~~~~~~~~~~~~

Add unit tests to build (default: disabled)
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

Override default javac debug arguments `source,lines`:
~~~~~~~~~~~~~
-DJAVAC_DEBUG_ARGS="source,lines,vars"

-DJAVAC_DEBUG_ARGS="none"
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

To build Java bindings:
~~~~~~~~~~~~~
-DBUILDJAVA=ON
~~~~~~~~~~~~~

To build documentation run: 
~~~~~~~~~~~~~
make doc
~~~~~~~~~~~~~

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
IDE integration configuration files are provided for 
- [Eclipse](https://download.eclipse.org/eclipse/downloads/) with extensions
  - [CDT](https://github.com/eclipse-cdt/) or [CDT @ eclipse.org](https://projects.eclipse.org/projects/tools.cdt)
  - Not used due to lack of subproject include file and symbol resolution:
    - `CMake Support`, install `C/C++ CMake Build Support` with ID `org.eclipse.cdt.cmake.feature.group`

From the project root directory, prepare the `Debug` folder using `cmake`
~~~~~~~~~~~~~
./scripts/eclipse-cmake-prepare.sh
~~~~~~~~~~~~~

The existing project setup is just using `external build` via `make`.

You can import the project to your workspace via `File . Import...` and `Existing Projects into Workspace` menu item.

For Eclipse one might need to adjust some setting in the `.project` and `.cproject` (CDT) 
via Eclipse settings UI, but it should just work out of the box.

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

