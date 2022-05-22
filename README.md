# Jau Support Library (C++, Java, ...)

[Original document location](https://jausoft.com/cgit/jaulib.git/about/).

## Git Repository
This project's canonical repositories is hosted on [Gothel Software](https://jausoft.com/cgit/jaulib.git/).

## Goals
This project aims to provide general C++ and Java collections, algorithms and utilities - as well as basic concepts to support a Java JNI binding.

This project was extracted from [Direct-BT](https://jausoft.com/cgit/direct_bt.git/about/) to enable general use and enforce better encapsulation.

## API Documentation
Up to date API documentation can be found:

* [C++ API Doc](https://jausoft.com/projects/jaulib/build/documentation/cpp/html/index.html) with [modules](https://jausoft.com/projects/jaulib/build/documentation/cpp/html/modules.html):
  * [Basic Algorithms](https://jausoft.com/projects/jaulib/build/documentation/cpp/html/group__Algorithms.html)
  * [Byte Utilities](https://jausoft.com/projects/jaulib/build/documentation/cpp/html/group__ByteUtils.html)
  * [C++ Language Utilities](https://jausoft.com/projects/jaulib/build/documentation/cpp/html/group__CppLang.html)
  * [Concurrency](https://jausoft.com/projects/jaulib/build/documentation/cpp/html/group__Concurrency.html)
  * [Data Structures](https://jausoft.com/projects/jaulib/build/documentation/cpp/html/group__DataStructs.html)
  * [File Utilities](https://jausoft.com/projects/jaulib/build/documentation/cpp/html/group__FileUtils.html)
  * [Float types and arithmetic](https://jausoft.com/projects/jaulib/build/documentation/cpp/html/group__Floats.html)
  * [Fraction Arithmetic and Time](https://jausoft.com/projects/jaulib/build/documentation/cpp/html/group__Fractions.html)
  * [Function Pointer](https://jausoft.com/projects/jaulib/build/documentation/cpp/html/group__FunctionPtr.html)
  * [Integer types and arithmetic](https://jausoft.com/projects/jaulib/build/documentation/cpp/html/group__Integer.html)
  * [Java VM Utilities](https://jausoft.com/projects/jaulib/build/documentation/cpp/html/group__JavaVM.html)
  * [Network Utilities](https://jausoft.com/projects/jaulib/build/documentation/cpp/html/group__NetUtils.html)
  * [String Utilities](https://jausoft.com/projects/jaulib/build/documentation/cpp/html/group__StringUtils.html)
  * [System and OS Utilities](https://jausoft.com/projects/jaulib/build/documentation/cpp/html/group__SysUtils.html)

* [Java API Doc](https://jausoft.com/projects/jaulib/build/documentation/java/html/index.html).


## Examples
See *Direct-BT* [C++ API Doc](https://jausoft.com/projects/direct_bt/build/documentation/cpp/html/examples.html).

## Supported Platforms
C++17 and better.

## Building Binaries
It is advised to include this library into your main project, e.g. as a git-submodule.

Then add *jaulib/include/* to your C++ include-path and also add the C++ source files
under *jaulib/src/* into your build recipe.

The produced Java libraries are fully functional.

This library's build recipe are functional though, 
but currently only intended to support unit testing and to produce a Doxygen API doc.

The project requires CMake 3.13+ for building and a Java JDK >= 11.

Installing build dependencies on Debian (10 or 11):
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.sh}
apt install git
apt install build-essential g++ gcc libc-dev libpthread-stubs0-dev 
apt install libunwind8 libunwind-dev
apt install openjdk-11-jdk openjdk-11-jre junit4
apt install cmake cmake-extras extra-cmake-modules
apt install doxygen graphviz
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

For a generic build use:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.sh}
CPU_COUNT=`getconf _NPROCESSORS_ONLN`
git clone https://jausoft.com/cgit/jaulib.git
cd jaulib
mkdir build
cd build
cmake -DBUILDJAVA=ON -DBUILDEXAMPLES=ON -DBUILD_TESTING=ON ..
make -j $CPU_COUNT install test doc
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The install target of the last command will create the include/ and lib/ directories with a copy of
the headers and library objects respectively in your build location. Note that
doing an out-of-source build may cause issues when rebuilding later on.

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

Disable stripping native lib even in non debug build:
~~~~~~~~~~~~~
-DUSE_STRIP=OFF
~~~~~~~~~~~~~

Disable using `libunwind` (default: enabled for all but `arm32`, `armhf`)
~~~~~~~~~~~~~
-DUSE_LIBUNWIND=OFF
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

Using clang instead of gcc:
~~~~~~~~~~~~~
-DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++
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

You may use [our pi-gen branch](https://jausoft.com/cgit/pi-gen.git/about/) to produce 
a Raspi-arm64, Raspi-armhf or PC-amd64 target image.

## Changes

**1.0.0**

* First stable release (TODO)

**0.10.1** (WIP)

* Add `file_util.hpp`: File Utilities for platform agnostic C++ handling of file stats and directory traversal, etc.
* `fraction_timespec`: Add notion of 'struct timespec' year 2038 issue, `to_timespec()` conversion, `to_iso8601_string()`

**0.10.0**

* Add `jau::latch::count_up()` to `jau::latch`, allowing to dynamically add *events* to required to complete
* Fix `jau::call_on_release`: Skip calling `release_func` if resource has been marked orderly released, avoid use after free.
* Add `root_environment::is_terminating()`
* Robustness of JNI
  - Use `std::shared_ptr<T>` instead of a `naked pointer` for sane lifcycle, see new `shared_ptr_ref<T>`.
  - JNIGlobalRef: Make all operations atomic and fix copy ctor JNIGlobalRef/JavaGlobalObj
  - JavaGlobalObj::dtor needs to acquire JNIGlobalRef lock for atomic `mNotifyDeleted` deleter call (C++ -> java)
  - Remove unused JavaAnon/JavaUplink/JNIGlobalRef's `clear(), having leak potential
  - Have derivations use `override` for `virtual` superclass, marking a requirement
* Move all of `jni_mem` into namespace jau

**0.9.3**

* Group all functionality in modules: Algorithms, ByteUtils, CppLang, Concurrency, DataStructs, Floats, Fractions, FunctionPtr, Integrals, JavaJVM, NetUtils, StringUtils
* Add getMonotonicTime() & getWallClockTime(), returning fraction_timespec
* Add sleep_for(), sleep_until() and wait_for() & wait_until() using fraction, fraction_timespec, as well as choice over clock type.
* Introduce new types: fraction, fraction_timespec; its constants & literals as well adoption in latch, ringbuffer, service_runner and simple_timer.
* int_math.hpp: Add Integer overflow aware arithmetic, use Integer Overflow Builtins if available (GCC + Clang)
* Refine sign(), invert_sign(), abs() and digits10() template funcs: Better type and signed/unsigned variant for invert_sign() and abs()
* Add stdint literals in namespace `jau::int_literals`, e.g. `3_i64` for `(int64_t)3` for all stdint signed and unsigned 8-64 bit wide types
* Always add libatomic (will be required for new fraction)

**0.8.6**

* Enhance `service_runner` and fix `simple_timer`, i.e. end waiting if stopped
* Disable `libunwind` on `armhf`/`arm32` for stability, see `USE_LIBUNWIND` cmake variable above
* OpenJDK 17 compatible w/o warnings, use if available via build scripts.

**0.8.0**

* Enhance `service_runner`: fix using `service_shutdown_timeout_ms()`, add `join()`, `remove service_end_post_notify` (leak), 
* Add `simple_timer`: A simple timer for timeout and interval applications, using one dedicated `service_runner` thread per instance.
* Fix EUI48[Sub] ctor with byte-ptr: Don't swap if given `byte_order == endian::native`, we store in native order
* `uint[128,192,256]_t`: Add ctor with given byte array in native byte order, useful for const initialization
* SIGSEGV workaround in `jau::get_backtrace(..)` of libunwind [1.3 - 1.6.2]'s `unw_step(..)` using g++ 10.2.1 aarch64: Disable Optimization

**0.7.12**

* git version info: Added post-tag: `VERSION_COMMITS` and `VERSION_SHA1_SHORT`. `VERSION_LONG` reflects post-tag and dirty. Added mapping to project version var-names.

**0.7.11**

* Add Java/C++ hexStringBytes(..); Fix Java's bytesHexString(..) path !lsbFirst; Add unit tests
* Fixes for clang++ 11.0.1-2
* Added `jau::service_runner`, a reusable dedicated thread performing custom user services.
* Fix jau::ringbuffer (conditional variable lock)
* Fix jau::latch (conditional variable lock) and `test_latch01.cpp`

**0.7.3**

* Add `to_string(const ordered_atomic<>&)`, allowing to skip manual '.load()' using `jau::to_string()` etc.
* Add jau::latch
* ringbuffer: Remove locking mutex before `notify_all`, avoid re-block of notified wait() threads.

**0.7.2**

* Add helper_jni: checkAndGetObject(..)
* Refine API doc and add build-doc.sh helper script etc

**0.7.0**

* ringbuffer: Add notion of operating threading mode for more efficancy:
  - One producer-thread and one consumer-thread (default)
  - Multiple producer-threads and multiple consumer-threads
  See getMultiPCEnabled() and setMultiPCEnabled()
* ringbuffer API change: Drop whole `NullValue` *angle*, simplifying.
  - Drop `Value_type [get|peek]*()`, use `bool [get|peek]*(Value_type&)` instead
  - Drop `NullValue_type` template type param and `NullValue_type` ctor argument.
  - Simplifies and unifies single and multi get and put, as well as testing (motivation).
  - Adding multi-threading tests w/ value checks in `test_lfringbuffer_a.hpp` for `test_lfringbuffer[01-04]`
* ringbuffer: Drop `use_memset` non-type template param, 
  simply use `use_memcpy` having same semantics of *TriviallyCopyable*.
* darray: Refine API doc for `use_memmove` using *TriviallyCopyable* and *Trivial destructor* ...

**0.6.1**

* TOctets: Add convenient *memmove*, *memset* and *bzero* methods; Ensure all memory ops are either *std::* or global namespace *::*

**0.6.0**

* Bump minor version due to API change (`darray`, `cow_darray` and `ringbuffer`)
* Add `darray` and `cow_darray` initializer list construction using move-semantics, instead of copy-semantics (`std::initializer_list`).
* Fix `cow_darray::push_back( InputIt first, InputIt last )`: On storage growth path, `push_back` must happen on new storage.
* Revised Non-Type Template Parameter (NTTP) of `darray`, `cow_darray` and `ringbuffer`
  - Dropped `use_realloc`, as it is fully deducted
  - Renamed `sec_mem` to `use_secmem`
  - Enable type traits in user `Value_type` to deduce `use_memmove` and `use_secmem`
  - Ringbuffer also uses darray's NTTP `use_memmove` and `use_secmem`
* Ringbuffer uses placement-new for better efficacy (dropped C++ array default init)
* Use explicit `constexpr if`: `darray`, `ringbuffer`, 
  endian conversion `[le|be|cpu]_to_[le|be|cpu]()` and `bit_cast()`.
* POctet: Add explicit copy-ctor with given capacity and add TROOctets default ctor
* EUI48[Sub] C++/Java and POctets: Better API doc re byte order

**0.5.0**

* Bump minor version due to API change
* EUI48Sub: Add required endian conversion for byte stream ctor (C++ and Java)
* EUI48[Sub]: Add endian awareness, also fixes indexOf() semantics (C++ and Java)
* Octets: Enhance API doc
* Octets/ringbuffer: Use std method names for sizes
* Octets: Expose endian awareness, pass either endian::little or endian::big at ctor
* darray: Support immutable type, i.e. 'const Type'

**0.4.7**

* Add uuid_t::equivalent(..) method fo relaxed comparison on different uuid_t types
* Added uuid_t::getTypeSizeString()
* Fix jau::get_backtrace(..) and adjusted ERR_PRINT and ABORT

**0.4.3**

* Moved `EUI48`, `EUI48Sub` (C++/Java) and `uuid_t`, `Octets` (C++) from `Direct-BT` for general use.

**0.4.1**

* ringbuffer: Added block access, fast-path for integral types, `bool get*(Value_type& v)`, ..
* ringbuffer: Special `NullValue_type` template type and value (ctor) handling 
* ringbuffer: Add 'Size_type waitForElements(..)' and fix 'Size_type waitForFreeSlots(..)'
* ringbuffer: `get()` and `getBlocking()` takes size of dest buffer and minimum count to wait for, return received elements.
* ringbuffer: syncWrite and syncRead member semantic cleanup, 
* Added `uint64_t getWallClockSeconds()`

**0.3.0**

* Consolidate conversion to hex and decimal string: `to_hexstring(Type)` and `to_decstring(Type)`, dropped `<type>[Hex|Dec]String(..)`
* All get/set functions are exposed as overloaded templates [get|put]_value(..), so is to_hex_string(..).
* All byte-order conversion and get/set functions are of 'constexpr' now.
* Add constexpr 'enum class endian', 'pointer_cast()' and 'bit_cast()'.
* Offering JaulibSetup.cmake for parent projects for aligned setup.
* Java: Working JNI-Lib bootstraping w/ fat-jar. Passing all unit tests.
* Java: Generating jar and source-zip packages (normal + fat); Enable full junit testing.
* Finalized jau::cow_darray and its iterator API. All write API entries were added to the mutable iterator.
* Introduce jau::callocator for jau::darray to support realloc instead of alloc, copy and dtor
* Passed gcc-10, gcc-11, clang-9 and clang-11
* Passed detailed cow_iterator test.
* jau::cow_vector is marked deprecated, as only jau::cow_darray can give certain guarantees using jau::darray
* jau::cow_vector is aligned to jau::cow_darray and also uses jau::cow_ro_iterator and jau::cow_rw_iterator
* Added jau::darray and jau::cow_darray (using jau::cow_ro_iterator and jau::cow_rw_iterator)
* Test: Using imported Catch2 (v3-devel) for C++ unit testing
* Reduced ringbuffer to single implementation, adding move-operations
* Added basic_types.hpp: 'to_decimal_string(..)' implementing <type>DecString() inlines.
* Use nsize_t and snsize_t where appropriate for smaller footprint
* Have ringbuffer's Size_type parameterized
* Bugfixes and added cow_array

**0.2.1**

* Added cow_vector and sc_atomic_critical
* Passed GCC all warnings, compile clean
* Passed GCC sanitizer runtime checks
* Passed valgrind's memcheck, helgrind and drd validating no memory leak nor data race or deadlock using dbt_scanner10
* Added native de-mangled backtrace support using *libunwind* and and *abi::__cxa_demangle*

**0.1.0**

* Extraction from Direct-BT

