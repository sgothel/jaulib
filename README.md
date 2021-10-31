# Jau Support Library (C++, Java, ...)

[Original document location](https://jausoft.com/cgit/jaulib.git/about/).

## Git Repository
This project's canonical repositories is hosted on [Gothel Software](https://jausoft.com/cgit/jaulib.git/).

## Goals
This project aims to provide general C++ and Java collections, algorithms and utilities - as well as basic concepts to support a Java JNI binding.

This project was extracted from [Direct-BT](https://jausoft.com/cgit/direct_bt.git/about/) to enable general use and enforce better encapsulation.

## API Documentation
Up to date API documentation can be found:

* [C++ API Doc](https://jausoft.com/projects/jaulib/build/documentation/cpp/html/index.html).

* [Java API Doc](https://jausoft.com/projects/jaulib/build/documentation/java/html/index.html).


## Examples
See *Direct-BT* [C++ API Doc](https://jausoft.com/projects/direct_bt/build/documentation/cpp/html/index.html).

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


## Changes

**1.0.0**

* First stable release (TODO)

**0.7.0**

* To be completed ...
* ringbuffer API change: Drop whole `NullValue` *angle*, simplifying.
  - Drop `Value_type [get|peek]*()`, use `bool [get|peek]*(Value_type&)` instead
  - Drop `NullValue_type` template type param and `NullValue_type` ctor argument.
  - Simplifies and unifies single and multi get and put, as well as testing (motivation).
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

