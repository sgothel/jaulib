# Jau Support Library (C++, Java, ...)

[Original main document location](https://jausoft.com/cgit/jaulib.git/about/).

## Changes

**1.0.0**

* First stable release (TODO)

**0.13.0**

* `string_util.hpp`: Add `jau::to_string()` support for `std::string` and `std::string_view` as well as for `std::vector<T>` lists
* Add namespace `jau::io::uri`, limited URI scheme functionality to query whether implementation may handle the protocol.
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
