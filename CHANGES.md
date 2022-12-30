# Jau Support Library (C++, Java, ...)

[Original main document location](https://jausoft.com/cgit/jaulib.git/about/).

## Changes

**1.0.1**
* C++20 clean
* Fix certain C++17 and C++20 compiler and clang-tidy warnings

**1.0.0**
* First stable release
  - Added and passed clang-tidy diagnostics
    - Fixed multiple issues revealed by review
    - fraction_type divison by zero
    - explicitly abort on heap alloc failure, maintaining noexcept
    - performance and API cleansiness
  - Added IDE vscode (vscodium) multi root-workspace config

**0.18.0**
* Add support for *Alpine Linux* using [musl](https://musl.libc.org/) C library
* API changes:
  - Unique lower-case Un-Mountflags w/o prefix in
      - C++ jau::fs::[u]mountflags_linux: Also use 'enum class'
      - Java jau.fs.linux.[Unm|M]outFlags
* Passed [platforms](PLATFORMS.md) testing:
  - Debian 11
  - Debian 12
      - gcc 12.2.0
      - clang 14.0.6
  - Ubuntu 22.04
  - FreeBSD 13.1
  - Alpine Linux 3.16

**0.17.1**
* functional: Refinements ..
  - Hide `delegate_t` union details using a non-anonymous type.
  - Rename `function<R(A...)>::delegate_t_` to `function<R(A...)>::delegate_type`
  - Add ylambda example using `function<R(A...)>::delegate_type` instead of `auto`
  - Fix link to example

**0.17.0**
* functional: Added [ylambda_target_t](https://jausoft.com/projects/jaulib/build/documentation/cpp/html/classjau_1_1func_1_1ylambda__target__t.html#ylambda_target),
  a [Y combinator](https://en.wikipedia.org/wiki/Fixed-point_combinator#Strict_functional_implementation)
  and [Deducing this](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p0847r7.html) implementation for lambda closures
  usable for recursive algorithms.
* functional: Revised implementation using static polymorphism.
  - Deep copy of target function detail
    - Works with mutable lambda states, i.e. mutating a field (by-copy captured)
  - 14-25% less memory footprint
  - Performance gain on linux-arm64, Raspberry Pi 4 Model B Rev 1.4
      - 19% using plain and capturing lambdas
      - 27% using member functions
  - No significant performance gain on linux-amd64, AMD Ryzen 9 3950X 16-Core
  - See commit f1059912dfd633cf90d14b688b1cc3f225a434f5

**0.16.3**
* functional: Add general lambda support incl. captures, revise API doc, have native target function size accessible

**0.16.2**
* Fix `jau::service_runner::start()`: Case `set_shall_stop()` occurs fast by worker callback -> infinite loop.
* `jau::function<R(A...)>` 
  - add fast path `target_t::delegate_t` invocation
  - add function ctor for all type, incl. non-capturing lambda assignment
  - add performance tests

**0.16.0**
* `jau::FunctionDef<>` -> `jau::function<R(A...)>` 
  - from `FunctionDef<R, A1, A2>` -> `function<R(A1, A2)>`
  - support for void return type
  - header file renamed `function_def.hpp` -> `functional.hpp`
  - C++ naming scheme

**0.15.0**
* Add `jau::codec::base` and `org.jau.util.BaseCodec` for variable integer base coding and fixed binary base 64 coding.
* Complete full `jau::fs::mount()` and `umount()`: 
  - `mount()` for block devices etc (non loop-device image)
  - `umount()` of mount-point
  - MountFlags and UnmountFlags detailed for GNU/Linux
  - Java binding and a manual unit test with TestsudoFileUtils02.
* `ByteInStream_URL` available() and read() await `url_header_sync` for `m_content_size` and blocks at read if available
  - `ByteInStream_Feed::read()` blocks as well with known `m_content_size`, but has no `url_header_sync` mechanism
* Introduce `url_header_sync` object, used for async `read_url_stream()` and hence `ByteInStream_URL`
* ringbuffer: Add write interrupt, close() interrupting r/w ops, check capacity upfront for r/w ops
* `ByteInStream_{SecMemor, File}`, `ByteOutStream_File`: Set `iostate::eofbit` at close(), ensure ending potential loops
* FileStats (java): Expose ctor using DirItem for efficiency
* Add `[[nodiscard]]` to Byte{In,Out}Stream read(), peek(), write() interface and all specializations, refine single byte API.
* Add `ByteOutStream` and  `ByteOutStream_File` POSIX 'fd' implementation
* `io_util`/`byte_stream`: Decouple from Botan, add iostate and new `Byte*Stream*` super `iostate_func` class for `std::basic_ios`'s iostate functionality
* Add `jau::io::read_stream()` with double-buffered reading to ensure last `consumer_fn()` call gets `is_final` set if next buffer has `eof()` w/ zero bytes
* `jau::io::ByteInStream_File`: Replace `std::ifstream` for file-descriptor (fd) POSIX layer to support dirfd and plain fd operations
* `jau::fs::file_stats`: Support new `fmode_t` types sock, blk, chr and fifo, move fd from type -> attribute, support sole `file_stats(fd)` ctor,
* Add `jau::fs::traverse_options::lexicographical_order` as required when computing an order dependent outcome like a hash value
* Add `jau::fs::copy_option::into_existing_dir`: Copy source dir content into an already existing destination directory as if destination directory did not exist
* Add support for FreeBSD
  - Build clean and passes all tests.
  - Motivation: Have one more target system for POSIX and clang validation.
* Add support for (named) file descriptor: `jau::fs::file_stat` and `jau::io::ByteInStream_File`
  - sub-project `pipe dreams` for `cipherpack`.
  - allow stream-processing via pipes (e.g. stdin/stdout)
* Resolve OpenJDK segmentation fault running via qemu binfmt_misc armhf or aarch64 on x86_64 host (cross build and test)
* `copy_options::ignore_symlink_errors`: Also ignore symlinks if not supported by target filesystem if not using `follow_symlinks` (e.g.: vfat target)
* `fraction_timespec::to_iso8601_string()`, `file_stats::to_string()`: Drop `use_space` parameter and print UTC ISO8601 w/ nanoseconds if not zero
* Fix `fraction_timespec::normalize()`
* Add `jau::fs::rename()` (C++) / `FileUtil.rename()` (Java)
* Data-Race-Free (DRF) or transaction safe `jau::fs::visit()`, `copy()` and `remove()`: Use `dirfd` `openat()` etc operations and temp-dir for newly dirs (copy) ..
* Add cpuid.hpp (SysUtils), namespace `jau::cpu:` Query `cpu_family` and arm/aarch64 hwcaps using glibc/Linux 'getauxval()' (todo: x86, x86_64 via cpuid)

**0.14.0**

* Java `org.jau.sys.Clock`: Add `Instant get[Monotonic|Wallclock]Time()` and `wallClockSeconds()` 
  - matching C++ lib using Java `Instant` for C++ `fraction_timespec`
* Add java mapping of `jau::fs::*` operations
  - `get_cwd()`, `basename()`, `dirname()`, `compare()`, `mkdir()`, `touch()`, `get_dir_content()`
  - `visit()` with `path_visitor` -> `PathVisitor` and `traverse_options` -> `TraverserOptions`
  - `copy()` with `copy_options` -> `CopyOptions`
  - `remove()` with `traverse_options` -> `TraverseOptions`
  - `mount_image()` and `umount()`
  - Copied `test_fileutils01.cpp` to `TestFileUtils01.java` and `testsudo_fileutils02.cpp` to `TestsudoFileUtils02.java`.
* Have `jau.pkg.PlatformRuntime` load tool library `jaulib` as well, resolving dependencies for self-testing.
* Add java mapping of `jau::io::ByteInStream` for file, URL and feed for general use
  - `org.jau.io.ByteInStream`, `org.jau.io.ByteInStream_File`, `org.jau.io.ByteInStream_URL`, `org.jau.io.ByteInStream_Feed`
  - Mapped `jau::io::to_ByteInStream()` to `org.jau.io.ByteInStreamUtil.to_ByteInStream()`
  - Mapped `jau::io::uri_tk` to `org.jau.io.UriTk`
  - Added java unit test `TestByteStream01` covering above, same as native `test_bytestream01`
* Add `jau::fs::mount_image()` and `umount()`
  - `mount_image()` is currently only supported on `Linux`
  - Enable testing on `Linux` via `-DTEST_WITH_SUDO=ON` as it requires `root` permissions.
  - Require to be called as user with capabilities `cap_sys_admin,cap_setuid,cap_setgid+eip` or as root via sudo
  - Uses fork() to have child-process seteuid(0) if not already for loop-control and mount only.
* Update Catch2 to v3.0.1, generated 2022-05-17 and adopt build and tests
* Use `libcurl` optional, enable via cmake option `-DUSE_LIBCURL=ON` and remove dependencies if unused.
* `libunwind` default is disabled (must be explicitly enabled via `-DUSE_LIBUNWIND=ON`)
* Fix issues with gcc 9.4 used on Ubuntu 20.04
* `file_util`, namespace `jau::fs`
  - `file_stats` fully maps symbolic links via `link_target()` and exposing stored `link_target_path()`
  - `dir_item` reduces `.` and `..` in given path, drop taking splitted dirname and basename arguments
  - `fmode_{bits->t}`: Add full POSIX protection bit support
  - `file_stats`: Use statx() or { lstat64() and stat64() }, maintain retrieved `field_t`, contain the linked-target as `std::shared_ptr<file_stats>`.
  - Add dirname() and basename()
  - Add `fmode_t` POSIX protection bits to mkdir() and touch()
  - Add `traverse_event` as `path_visitor()` argument and `traverse_options` - both supporting visit(); remove() uses `traverse_options`
  - Add jau::fs::copy() with [`recursive`, `follow_symlinks`, `overwrite`, `preserve_all`, `sync`] `copy_options`, using visit()
* Move JavaVM Group to namespace jau::jni
* Security
  - `ringbuffer<T>:` Add clear() argument bool zeromem=false, allowing to explicitly zero memory if used w/o `sec_mem`
  - `ByteInStream_URL`, `ByteInStream_Feed`: Zero ringbuffer at close()/dtor() to be on par with `ByteInStream_SecMemory` and `ByteInStream_File` + `secure_vector<T>` usage.
  - Add `secure_string` typedef along with `secure_vector` using `::explicit_bzero()` `jau::callocator_sec<T>`
* Add `jau::io::uri_tk` to Group IOUtils

**0.13.0**

* `string_util.hpp`: Add `jau::to_string()` support for `std::string` and `std::string_view` as well as for `std::vector<T>` lists
* Add namespace `jau::io::uri_tk`, limited URI scheme functionality to query whether implementation may handle the protocol.
  - Query *libcurl* supported protocols at runtime
  - Test for local file protocol
  - Test whether protocol in given uri is supported by *libcurl*
* `jau::io::read_url_stream()`, sync and async, return immediately if protocol in given url is not supportet
  - async variant returns `std::unique_ptr<std::thread>`, where a nullptr is used for no support
* `jau::io::ByteInStream_File` recognizes the local file protocol and cuts off `file://` is used.
  - Fix: Recognition of a non-existing path, unaccessbile path or non-file case properly
* `jau::io::ByteInStream_URL` recognizes a non supported protocol via async `jau::io::read_url_stream()`.
* Added convenient `jau::io::std::unique_ptr<ByteInStream> to_ByteInStream()`
  - Returning either a `jau::io::ByteInStream_File`, `jau::io::ByteInStream_URL` or nullptr if `path_or_url` is not supported
* Make Java class `org.jau.ney.Uri` standalone, drop dependencies for easier reusage.

**0.12.0**

* Minor changes
* JNI: Add helper functions
  - `convert_jlist_string_to_vector()` and `convert_vector_string_to_jarraylist()`
  - `getObjectFieldValue()`, `getStringFieldValue()` and `getLongFieldValue()`
* Fix ByteInStream_[URL|Feed]::close(): Only change `m_result` if still NONE, then write SUCCESS (close success, no error)

**0.11.1**

* `interruptReader()` has been exposed as `ByteInStream_Feed::interruptReader()` and called in `ByteInStream_Feed::set_eof()`
* `io::read_url_stream`: Properly handle `content_length` and be responsive on errors
  - Fix `content_length` (-1 == unknown)
  - Add `consume_header` w/ `response_code` for errors >= 400 calls `buffer.interruptReader()`
  - `interruptReader()` calls have been added in synchronous- and asynchronous `read_url_stream()` functions on error
* `ringbuffer`: Add `interruptReader()` if intended to abort writing and to interrupt the reader thread's potentially blocked read-access call

**0.11.0**

* Add `byte_stream.hpp` and `io_util.hpp` from Elevator project for generic use
  - This adds build dependency to `curl` library and test dependency to `mini-httpd`, both not needed if unused.
  - Added unit tests for both, `test_bytestream01` and `test_iostream01`
  - Add IOUtils Module/Group
* `fs::file_stats`: Retrieve symlink data and have `fmode_bits::LINK` additionally set; Add `follow_sym_link_dirs` param to visit() and remove()
* Add `fs::get_cwd()`
* Add `callocator_sec<T>`
* FunctionDef: Complete documentation, refine names

**0.10.1**

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

