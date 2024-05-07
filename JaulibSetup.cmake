#
# jaulib cmake build settings, modularized to be optionally included by parent projects
#
# JaulibPreset Cached variables are
# - CMAKE_BUILD_TYPE
# - BUILD_TESTING
# - CMAKE_CXX_COMPILER_ID
# - CMAKE_C_COMPILER
# - CMAKE_CXX_COMPILER
# - CMAKE_CXX_CLANG_TIDY
#
# JaulibSetup Cached variables are
# - DEBUG
# - CMAKE_INSTALL_PREFIX
# - CMAKE_CXX_STANDARD_REQUIRED
# - CMAKE_CXX_STANDARD
# - LIB_INSTALL_DIR
# - USE_LIBCURL
# - USE_LIBUNWIND
# - BUILDJAVA
#

include_guard(GLOBAL)

macro(JaulibPreset)
    # Poor man's IDE integration, hard-coded presets for undefined variables
    message(STATUS "JaulibPreset: Start")
    if( (NOT DEFINED JAU_CMAKE_ENFORCE_PRESETS) AND (DEFINED ENV{JAU_CMAKE_ENFORCE_PRESETS}) )
        set (JAU_CMAKE_ENFORCE_PRESETS $ENV{JAU_CMAKE_ENFORCE_PRESETS})
        message(STATUS "JaulibPreset: JAU_CMAKE_ENFORCE_PRESETS -> ${JAU_CMAKE_ENFORCE_PRESETS} (env)")
    endif()
    if( JAU_CMAKE_ENFORCE_PRESETS OR 
        ( (NOT DEFINED CMAKE_INSTALL_PREFIX) AND (NOT DEFINED CMAKE_CXX_CLANG_TIDY) ) )
        message(STATUS "JaulibPreset: Enforcing hardcoded CMake Presets!")
        if(JAU_CMAKE_ENFORCE_PRESETS)
            message(STATUS "JaulibPreset: ... triggered by CMake variable JAU_CMAKE_ENFORCE_PRESETS ${JAU_CMAKE_ENFORCE_PRESETS}.")
        elseif( (NOT DEFINED CMAKE_INSTALL_PREFIX) AND (NOT DEFINED CMAKE_CXX_CLANG_TIDY) )
            message(STATUS "JaulibPreset: ... triggered by undefined CMAKE_INSTALL_PREFIX && CMAKE_CXX_CLANG_TIDY.")
        endif()
        if (DEFINED ENV{CMAKE_BUILD_PARALLEL_LEVEL})
            message(STATUS "JaulibPreset: Parallel build: CMAKE_BUILD_PARALLEL_LEVEL = $ENV{CMAKE_BUILD_PARALLEL_LEVEL}.")
        else()
            message(STATUS "JaulibPreset: Parallel build: Consider setting environment variable CMAKE_BUILD_PARALLEL_LEVEL.")
        endif()
        #
        # Defaulting presets: clang, clang-tidy, C++20, CMAKE_BUILD_TYPE, +testing
        #
        if( (NOT DEFINED CMAKE_INSTALL_PREFIX) )
            set(JAU_CMAKE_OVERRIDE_INSTALL_PREFIX ON)
            message(STATUS "JaulibPreset: Setting CMAKE_INSTALL_PREFIX earmarked")
        endif()
        if(NOT DEFINED CMAKE_CXX_STANDARD)
            set(CMAKE_CXX_STANDARD 20 CACHE STRING "" FORCE)
            message(STATUS "JaulibPreset: Setting CMAKE_CXX_STANDARD ${CMAKE_CXX_STANDARD}")
        endif()
        if(NOT DEFINED CMAKE_BUILD_TYPE)
            set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "" FORCE)
            message(STATUS "JaulibPreset: Setting CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE}")
        endif()
        if( (NOT DEFINED BUILD_TESTING) )
            set(BUILD_TESTING ON CACHE STRING "" FORCE)
            message(STATUS "JaulibPreset: Setting BUILD_TESTING ${BUILD_TESTING}")
        endif()
        if(NOT DEFINED CMAKE_CXX_COMPILER)
            set(CMAKE_CXX_COMPILER_ID "Clang" CACHE STRING "" FORCE)
            set(CMAKE_C_COMPILER "clang" CACHE STRING "" FORCE)
            set(CMAKE_CXX_COMPILER "clang++" CACHE STRING "" FORCE)
            message(STATUS "JaulibPreset: Setting CMAKE_CXX_COMPILER ${CMAKE_CXX_COMPILER}")
        endif()
        if( (NOT DEFINED CMAKE_CXX_CLANG_TIDY) 
            AND
            ( (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang") 
              OR 
              (${CMAKE_CXX_COMPILER} STREQUAL "clang++") 
            ) 
          ) 
            set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-p;${CMAKE_BINARY_DIR}" CACHE STRING "" FORCE)
            message(STATUS "JaulibPreset: Setting CMAKE_CXX_CLANG_TIDY ${CMAKE_CXX_CLANG_TIDY}")
        endif()

        # find_library will not work pre-project stage, hence try later..
        if(NOT DEFINED USE_LIBCURL)
            set(TRY_LIBCURL ON)
            message(STATUS "JaulibPreset: Setting TRY_LIBCURL ${TRY_LIBCURL}")
        endif()

        # find_library will not work pre-project stage, hence try later..
        if(NOT DEFINED USE_LIBUNWIND)
            set(TRY_LIBUNWIND ON)
            message(STATUS "JaulibPreset: Setting TRY_LIBUNWIND ${TRY_LIBUNWIND}")
        endif()

        # find_package will not work pre-project stage, hence try later..
        if(NOT DEFINED BUILDJAVA)
            set(TRY_JAVA ON)
            message(STATUS "JaulibPreset: Setting TRY_JAVA ${TRY_JAVA}")
        endif()
    endif()
    message(STATUS "JaulibPreset: End")
endmacro()

macro(JaulibSetup)
message(STATUS "JaulibSetup: Start: ${PROJECT_NAME}")

set(ENV{LANG} en_US.UTF-8)
set(ENV{LC_MEASUREMENT} en_US.UTF-8)

# Determine OS_AND_ARCH as library appendix, e.g. 'direct_bt-linux-amd64'
string(TOLOWER ${CMAKE_SYSTEM_NAME} OS_NAME)
string(TOLOWER ${CMAKE_SYSTEM_PROCESSOR} OS_ARCH0)
if(${OS_ARCH0} STREQUAL "arm")
    set(OS_ARCH "armhf")
elseif(${OS_ARCH0} STREQUAL "armv7l")
    set(OS_ARCH "armhf")
elseif(${OS_ARCH0} STREQUAL "aarch64")
    set(OS_ARCH "arm64")
elseif(${OS_ARCH0} STREQUAL "x86_64")
    set(OS_ARCH "amd64")
elseif( ( ${OS_ARCH0} STREQUAL "x86" ) OR ( ${OS_ARCH0} STREQUAL "i386" ) ( ${OS_ARCH0} STREQUAL "i686" ) )
    set(OS_ARCH "i586")
else()
    set(OS_ARCH ${OS_ARCH0})
endif()
set(OS_AND_ARCH ${OS_NAME}-${OS_ARCH})
set(os_and_arch_slash ${OS_NAME}/${OS_ARCH})
set(os_and_arch_dot ${OS_NAME}.${OS_ARCH})

if(DEFINED CMAKE_BUILD_TYPE)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(DEBUG_TMP ON)
    else()
        set(DEBUG_TMP OFF)
    endif()
    message(STATUS "JaulibSetup: Build Type = ${CMAKE_BUILD_TYPE} -> DEBUG ${DEBUG_TMP}")
endif()
if(NOT DEFINED CC_INSTRUMENTATION)
    set(CC_INSTRUMENTATION OFF)
endif()
if(NOT DEFINED CC_INSTRUMENTATION_UNDEFINED)
    set(CC_INSTRUMENTATION_UNDEFINED OFF)
endif()
if(NOT DEFINED CC_INSTRUMENTATION_THREAD)
    set(CC_INSTRUMENTATION_THREAD OFF)
endif()
if(CC_INSTRUMENTATION OR CC_INSTRUMENTATION_UNDEFINED OR CC_INSTRUMENTATION_THREAD)
    set(DEBUG_TMP ON)
    message(STATUS "JaulibSetup: CC_INSTRUMENTATION.. -> DEBUG ${DEBUG_TMP}")
endif()
if(DEBUG_TMP)
    set(DEBUG ON CACHE BOOL "" FORCE)
else()
    set(DEBUG OFF CACHE BOOL "" FORCE)
endif()

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    set(TOOLSET "clang")
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    set(TOOLSET "gcc")
else()
    string(TOLOWER ${CMAKE_CXX_COMPILER_ID} TOOLSET)
endif()

if( (JAU_CMAKE_OVERRIDE_INSTALL_PREFIX) OR (NOT DEFINED CMAKE_INSTALL_PREFIX) )
    if (DEBUG)
        set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/dist/default-debug")
    else()
        set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/dist/default-release")
    endif()
    set(JAU_CMAKE_FIX_INSTALL_PREFIX ON)
    message(STATUS "JaulibSetup: Setting(1) CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}")
endif()
if(JAU_CMAKE_FIX_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}-${OS_AND_ARCH}-${TOOLSET}")
    set(CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}" CACHE PATH "" FORCE)
    message(STATUS "JaulibSetup: Setting(2) CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}")
endif()

message(STATUS "JaulibSetup: OS_NAME ${OS_NAME}")
message(STATUS "JaulibSetup: OS_ARCH ${OS_ARCH} (${CMAKE_SYSTEM_PROCESSOR})")
message(STATUS "JaulibSetup: OS_AND_ARCH ${OS_AND_ARCH}")
message(STATUS "JaulibSetup: CC_INSTRUMENTATION = All ${CC_INSTRUMENTATION}, Undef ${CC_INSTRUMENTATION_UNDEFINED}, Thread ${CC_INSTRUMENTATION_THREAD}")
message(STATUS "JaulibSetup: DEBUG = ${DEBUG}")
message(STATUS "JaulibSetup: Compiler = ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "JaulibSetup: TOOLSET ${TOOLSET}")
message(STATUS "JaulibSetup: CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}")

if(DEFINED CMAKE_CXX_CLANG_TIDY)
    message(STATUS "JaulibSetup: clang-tidy preset: ${CMAKE_CXX_CLANG_TIDY}")
else()
    message(STATUS "JaulibSetup: clang-tidy not used")
endif()

set (CMAKE_CXX_STANDARD_REQUIRED ON CACHE BOOL "" FORCE)
if(DEFINED CMAKE_CXX_STANDARD)
    message(STATUS "JaulibSetup: CMAKE_CXX_STANDARD (preset): ${CMAKE_CXX_STANDARD}, CMAKE_CXX_STANDARD_REQUIRED: ${CMAKE_CXX_STANDARD_REQUIRED}")
else()
    set(CMAKE_CXX_STANDARD 20 CACHE STRING "" FORCE)
    message(STATUS "JaulibSetup: CMAKE_CXX_STANDARD (default): ${CMAKE_CXX_STANDARD}, CMAKE_CXX_STANDARD_REQUIRED: ${CMAKE_CXX_STANDARD_REQUIRED}")
endif()

#
# Post initial setup / var-check
#

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# for all
set (CC_FLAGS_WARNING "-Wall -Wextra -Werror")
set (GCC_FLAGS_WARNING_FORMAT "-Wformat=2 -Wformat-overflow=2 -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat-y2k")
set (GCC_FLAGS_WARNING "-Wall -Wextra -Wshadow -Wtype-limits -Wsign-compare -Wcast-align=strict -Wnull-dereference -Winit-self ${GCC_FLAGS_WARNING_FORMAT} -Werror")
# causes issues in jau::get_int8(..): "-Wnull-dereference"
set (GCC_FLAGS_WARNING_NO_ERROR "-Wno-error=array-bounds -Wno-error=null-dereference -Wno-multichar")

# too pedantic, but nice to check once in a while
# set (DISABLED_CC_FLAGS_WARNING "-Wsign-conversion")

# debug only
set (GCC_FLAGS_STACK "-fstack-protector-strong")
set (GCC_FLAGS_SANITIZE_ALL "-fsanitize-address-use-after-scope -fsanitize=address -fsanitize=pointer-compare -fsanitize=pointer-subtract -fsanitize=undefined -fsanitize=leak -fsanitize-recover=address")
set (GCC_FLAGS_SANITIZE_UNDEFINED "-fsanitize=undefined -fsanitize-recover=address")
set (GCC_FLAGS_SANITIZE_THREAD "-fsanitize-address-use-after-scope -fsanitize=undefined -fsanitize=thread -fsanitize-recover=address")
# -fsanitize=address cannot be combined with -fsanitize=thread
# -fsanitize=pointer-compare -fsanitize=pointer-subtract must be combined with -fsanitize=address
# -fsanitize=thread TSAN's lacks ability to properly handle GCC's atomic macros (like helgrind etc), can't check SC-DRF!

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # shorten __FILE__ string and the like ..
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_FLAGS_WARNING} ${GCC_FLAGS_WARNING_NO_ERROR} -fmacro-prefix-map=${CMAKE_SOURCE_DIR}/=/")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CC_FLAGS_WARNING}")
endif()

message(STATUS "JaulibSetup: USE_STRIP = ${USE_STRIP} (pre-set)")

if(DEBUG)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -ggdb -DDEBUG -fno-omit-frame-pointer ${GCC_FLAGS_STACK} -no-pie")
        set(CMAKE_SHARED_LINKER_FLAGS  "${CMAKE_SHARED_LINKER_FLAGS} -no-pie")
        set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -no-pie")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -DDEBUG")
    endif()
    if(CC_INSTRUMENTATION)
        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_FLAGS_SANITIZE_ALL}")
        endif()
    elseif(CC_INSTRUMENTATION_UNDEFINED)
        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_FLAGS_SANITIZE_UNDEFINED}")
        endif()
    elseif(CC_INSTRUMENTATION_THREAD)
        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_FLAGS_SANITIZE_THREAD}")
        endif()
    endif(CC_INSTRUMENTATION)
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG")
    if(GPROF)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -ggdb -pg")
    elseif(PERF_ANALYSIS)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -ggdb")
    else()
        find_program(STRIP strip)
        if (STRIP STREQUAL "STRIP-NOTFOUND")
            set(USE_STRIP OFF)
            message(STATUS "JaulibSetup: USE_STRIP:=false, strip not found")
        elseif(NOT DEFINED USE_STRIP)
            set(USE_STRIP ON)
            message(STATUS "JaulibSetup: USE_STRIP:=true, !DEBUG and not set")
        endif()
    endif()
endif(DEBUG)

message(STATUS "JaulibSetup: USE_STRIP = ${USE_STRIP} (final)")

if(DONT_USE_RTTI)
    message(STATUS "JaulibSetup: RTTI disabled")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fno-rtti")
    #set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} -fno-rtti")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fno-rtti")
else()
    message(STATUS "JaulibSetup: RTTI enabled")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frtti")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -frtti")
    #set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} -frtti")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -frtti")
endif(DONT_USE_RTTI)

if(${OS_NAME} STREQUAL "freebsd")
    set (SYS_INCLUDE_DIRS
      /usr/include
      /usr/local/include
    )
    set(CMAKE_SYSTEM_PREFIX_PATH "/usr;/usr/local")
else()
    set (SYS_INCLUDE_DIRS
      /usr/include
    )
endif()

if(${OS_NAME} STREQUAL "linux")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(CMAKE_CXX_STANDARD_LIBRARIES  "${CMAKE_CXX_STANDARD_LIBRARIES} -latomic")
    else()
        set(CMAKE_CXX_STANDARD_LIBRARIES  "${CMAKE_CXX_STANDARD_LIBRARIES} -latomic")
    endif()
endif()

set (LIB_INSTALL_DIR "lib${LIB_SUFFIX}" CACHE PATH "Installation path for libraries")

message(STATUS "JaulibSetup: CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS}")
message(STATUS "JaulibSetup: CMAKE_SHARED_LINKER_FLAGS = ${CMAKE_SHARED_LINKER_FLAGS}")
message(STATUS "JaulibSetup: CMAKE_EXE_LINKER_FLAGS = ${CMAKE_EXE_LINKER_FLAGS}")
message(STATUS "JaulibSetup: CMAKE_CXX_STANDARD_LIBRARIES = ${CMAKE_CXX_STANDARD_LIBRARIES}")
message(STATUS "JaulibSetup: LIB_INSTALL_DIR = ${LIB_INSTALL_DIR}")

# Set CMAKE_INSTALL_XXXDIR (XXX {BIN LIB ..} if not defined
# (was: CMAKE_LIB_INSTALL_DIR)
include(GNUInstallDirs)

# Appends the cmake/modules path to MAKE_MODULE_PATH variable.
set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

if(TRY_ALL)
    set(TRY_JAVA ON)
    set(TRY_LIBUNWIND ON)
    set(TRY_LIBCURL ON)
    message(STATUS "JaulibSetup: TRY_ALL -> TRY_JAVA, TRY_LIBUNWIND, TRY_LIBCURL")
endif(TRY_ALL)

message(STATUS "JaulibSetup: USE_LIBCURL = ${USE_LIBCURL} (pre-set)")
if ((NOT DEFINED USE_LIBCURL) AND TRY_LIBCURL)
    find_library(LIBCURL_LIBNAME "curl")
    if(NOT LIBCURL_LIBNAME)
        set(LIBCURL_LIBNAME "")
        set(USE_LIBCURL OFF CACHE BOOL "" FORCE)
        message(STATUS "JaulibSetup: TRY_LIBCURL: CURL disabled, not found")
    else()
        set(USE_LIBCURL ON CACHE BOOL "" FORCE)
        message(STATUS "JaulibSetup: TRY_LIBCURL: CURL found, USE_LIBCURL ${USE_LIBCURL}")
    endif()
endif()
if(NOT DEFINED USE_LIBCURL)
    set(USE_LIBCURL OFF CACHE BOOL "" FORCE)
    message(STATUS "JaulibSetup: USE_LIBCURL ${USE_LIBCURL} (default)")
else()
    message(STATUS "JaulibSetup: USE_LIBCURL ${USE_LIBCURL} (user)")
endif()
if(USE_LIBCURL)
    find_library(LIBCURL_LIBNAME "curl" REQUIRED)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_LIBCURL=1")
else()
    set(LIBCURL_LIBNAME "")
endif()
message(STATUS "JaulibSetup: USE_LIBCURL ${USE_LIBCURL} -> libname ${LIBCURL_LIBNAME}")

message(STATUS "JaulibSetup: USE_LIBUNWIND = ${USE_LIBUNWIND} (pre-set)")
if ((NOT DEFINED USE_LIBUNWIND) AND TRY_LIBUNWIND)
    find_library(LIBUNWIND_LIBNAME "unwind")
    if(NOT LIBUNWIND_LIBNAME)
        set(LIBUNWIND_LIBNAME "")
        set(USE_LIBUNWIND OFF CACHE BOOL "" FORCE)
        message(STATUS "JaulibSetup: TRY_LIBUNWIND: UNWIND disabled, not found")
    else()
        set(USE_LIBUNWIND ON CACHE BOOL "" FORCE)
        message(STATUS "JaulibSetup: TRY_LIBUNWIND: UNWIND found, USE_LIBUNWIND ${USE_LIBUNWIND}")
    endif()
endif()
if(NOT DEFINED USE_LIBUNWIND)
    set(USE_LIBUNWIND OFF CACHE BOOL "" FORCE)
    message(STATUS "JaulibSetup: USE_LIBUNWIND ${USE_LIBUNWIND} (default)")
else()
    message(STATUS "JaulibSetup: USE_LIBUNWIND ${USE_LIBUNWIND} (user)")
endif()
if(USE_LIBUNWIND)
    find_library(LIBUNWIND_LIBNAME "unwind" REQUIRED)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_LIBUNWIND=1")
else()
    set(LIBUNWIND_LIBNAME "")
endif()
message(STATUS "JaulibSetup: USE_LIBUNWIND ${USE_LIBUNWIND} -> libname ${LIBUNWIND_LIBNAME}")

# Make a version file containing the current version from git.
include (GetGitRevisionDescription)

git_describe (VERSION "--tags")
get_git_head_revision(GIT_REFSPEC VERSION_SHA1 ALLOW_LOOKING_ABOVE_CMAKE_SOURCE_DIR)
git_local_changes(GIT_WORKDIR_DIRTY)
message(STATUS "JaulibSetup: ${PROJECT_NAME} git_describe ${VERSION}")
message(STATUS "JaulibSetup: ${PROJECT_NAME} get_git_head_revision ${GIT_REFSPEC}")
message(STATUS "JaulibSetup: ${PROJECT_NAME} get_git_head_revision ${VERSION_SHA1}")
message(STATUS "JaulibSetup: ${PROJECT_NAME} git_local_changes ${GIT_WORKDIR_DIRTY}")

if ("x_${VERSION}" STREQUAL "x_GIT-NOTFOUND" OR "x_${VERSION}" STREQUAL "x_HEAD-HASH-NOTFOUND" OR "x_${VERSION}" STREQUAL "x_-128-NOTFOUND")
  message (WARNING "JaulibSetup:  - Install git to compile for production!")
  set (VERSION "v1.0.0-dirty")
endif ()

message (STATUS "JaulibSetup: ${PROJECT_NAME} version ${VERSION}")

#parse the version information into pieces.
string (REGEX REPLACE "^v([0-9]+)\\..*" "\\1" VERSION_MAJOR "${VERSION}")
string (REGEX REPLACE "^v[0-9]+\\.([0-9]+).*" "\\1" VERSION_MINOR "${VERSION}")
string (REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" VERSION_PATCH "${VERSION}")
string (REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.[0-9]+\\-([0-9]+).*" "\\1" VERSION_COMMITS "${VERSION}")
string (REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.[0-9]+-[0-9]+\\-(.*)" "\\1" VERSION_SHA1_SHORT "${VERSION}")

if ("${VERSION_COMMITS}" MATCHES "^v.*")
  set (VERSION_COMMITS "")
endif()
if ("${VERSION_SHA1_SHORT}" MATCHES "^v.*")
  set (VERSION_SHA1_SHORT "")
endif()
if ("${GIT_WORKDIR_DIRTY}" STREQUAL "CLEAN")
    set (VERSION_GIT_DIRTY_SUFFIX "")
else()
    set (VERSION_GIT_DIRTY_SUFFIX "-dirty")
endif()
if ("${VERSION_COMMITS}" STREQUAL "")
  set (VERSION_LONG "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}${VERSION_GIT_DIRTY_SUFFIX}")
else ()
  set (VERSION_LONG "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}-${VERSION_COMMITS}-${VERSION_SHA1_SHORT}${VERSION_GIT_DIRTY_SUFFIX}")
endif()
set (VERSION_SHORT "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
set (VERSION_API "${VERSION_MAJOR}.${VERSION_MINOR}")

if ("${GIT_WORKDIR_DIRTY}" STREQUAL "CLEAN")
    message (STATUS "JaulibSetup: ${PROJECT_NAME} git repo is CLEAN")
else()
    message (STATUS "JaulibSetup: ${PROJECT_NAME} git repo is DIRTY")
endif()
message (STATUS "JaulibSetup: ${PROJECT_NAME} version major ${VERSION_MAJOR}, minor ${VERSION_MINOR}, patch ${VERSION_PATCH}, post version commits ${VERSION_COMMITS}, ssha1 ${VERSION_SHA1_SHORT}, sha1 ${VERSION_SHA1}")
message (STATUS "JaulibSetup: ${PROJECT_NAME} VERSION_LONG  ${VERSION_LONG}")
message (STATUS "JaulibSetup: ${PROJECT_NAME} VERSION_SHORT ${VERSION_SHORT}")
message (STATUS "JaulibSetup: ${PROJECT_NAME} VERSION_API   ${VERSION_API}")

string(TIMESTAMP BUILD_TSTAMP "%Y-%m-%d %H:%M:%S")

if ((NOT DEFINED BUILDJAVA) AND TRY_JAVA)
    find_package(Java 11)
    find_package(JNI)
    include(UseJava)
    if(Java_FOUND) 
        message (STATUS "JaulibSetup: TRY_JAVA Java: ${Java_VERSION} or '${Java_VERSION_STRING}'")
    else (Java_FOUND)
        message (STATUS "JaulibSetup: TRY_JAVA Java not found")
    endif (Java_FOUND)
    if (JNI_FOUND)
        message (STATUS "JaulibSetup: TRY_JAVA JNI_INCLUDE_DIRS=${JNI_INCLUDE_DIRS}")
        message (STATUS "JaulibSetup: TRY_JAVA JNI_LIBRARIES=${JNI_LIBRARIES}")
    else (JNI_FOUND)
        message (STATUS "JaulibSetup: TRY_JAVA JNI not found")
    endif (JNI_FOUND)
    if(Java_FOUND AND JNI_FOUND) 
        set(BUILDJAVA ON CACHE BOOL "" FORCE)
    else()
        set(BUILDJAVA OFF CACHE BOOL "" FORCE)
    endif()
    message (STATUS "JaulibSetup: TRY_JAVA BUILDJAVA ${BUILDJAVA}")
endif()
IF(BUILDJAVA)
    message (STATUS "JaulibSetup: JAVA_HOME (var,pre ) = ${JAVA_HOME}")
    message (STATUS "JaulibSetup: JAVA_HOME (env,pre ) = $ENV{JAVA_HOME}")

    if (NOT DEFINED JAVA_HOME)
        if (DEFINED ENV{JAVA_HOME})
            set (JAVA_HOME $ENV{JAVA_HOME})
        endif()
    endif ()
    message (STATUS "JaulibSetup: JAVA_HOME (var,post) = ${JAVA_HOME}")

    find_package(Java 11 REQUIRED)
    find_package(JNI REQUIRED)
    include(UseJava)

    if(Java_FOUND) 
        message (STATUS "JaulibSetup: Java: ${Java_VERSION} or '${Java_VERSION_STRING}'")
    else (Java_FOUND)
        message (STATUS "JaulibSetup: Error: Java not found")
    endif (Java_FOUND)

    if (JNI_FOUND)
        message (STATUS "JaulibSetup: JNI_INCLUDE_DIRS=${JNI_INCLUDE_DIRS}")
        message (STATUS "JaulibSetup: JNI_LIBRARIES=${JNI_LIBRARIES}")
    else (JNI_FOUND)
        message (STATUS "JaulibSetup: Error: JNI not found")
    endif (JNI_FOUND)

    set (JAVAC ${Java_JAVAC_EXECUTABLE})
    set (JAR ${Java_JAR_EXECUTABLE})

    message (STATUS "JaulibSetup: JAVAC = ${JAVAC}")
    message (STATUS "JaulibSetup: JAR = ${JAR}")

    set(CMAKE_JAVA_COMPILE_FLAGS ${CMAKE_JAVA_COMPILE_FLAGS} -source 11 -target 11 -encoding UTF-8)
    if(DEBUG)
        if(DEFINED JAVAC_DEBUG_ARGS)
            set(CMAKE_JAVA_COMPILE_FLAGS ${CMAKE_JAVA_COMPILE_FLAGS} -g:${JAVAC_DEBUG_ARGS})
        else()
            set(CMAKE_JAVA_COMPILE_FLAGS ${CMAKE_JAVA_COMPILE_FLAGS} -g:source,lines)
        endif()
    else()
        # Adding source,lines (default javac debug setting) adds ~13% or 30k.
        # jaulib_fat.jar: No-Debug: 237458 bytes, Def-Debug: 267221 bytes (source, lines)
        if(DEFINED JAVAC_DEBUG_ARGS)
            set(CMAKE_JAVA_COMPILE_FLAGS ${CMAKE_JAVA_COMPILE_FLAGS} -g:${JAVAC_DEBUG_ARGS})
        else()
            set(CMAKE_JAVA_COMPILE_FLAGS ${CMAKE_JAVA_COMPILE_FLAGS} -g:source,lines)
            # set(CMAKE_JAVA_COMPILE_FLAGS ${CMAKE_JAVA_COMPILE_FLAGS} -g:none)
        endif()
    endif(DEBUG)
    message(STATUS "JaulibSetup: JAVAC_FLAGS = ${CMAKE_JAVA_COMPILE_FLAGS}")
ENDIF(BUILDJAVA)

message(STATUS "JaulibSetup: End: ${PROJECT_NAME}")
endmacro()

