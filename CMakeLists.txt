cmake_minimum_required (VERSION 3.13.0 FATAL_ERROR)
project (jaulib LANGUAGES C CXX)

if(${CMAKE_VERSION} VERSION_LESS "3.18.0")
    message(STATUS "CMake ${CMAKE_VERSION}, please consider to switch to CMake 3.18.0 or later")
else()
    message(STATUS "CMake ${CMAKE_VERSION} >= 3.18.0, good.")
endif()

FIND_PACKAGE (Threads REQUIRED)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

include(JaulibSetup.cmake)
JaulibSetup()

# this is the library version, derived from git tag and revision
set (jaulib_VERSION_MAJOR      ${VERSION_MAJOR})
set (jaulib_VERSION_MINOR      ${VERSION_MINOR})
set (jaulib_VERSION_PATCH      ${VERSION_PATCH})
set (jaulib_VERSION_COMMITS    ${VERSION_COMMITS})
set (jaulib_VERSION_SHA1       ${VERSION_SHA1})
set (jaulib_VERSION_SHA1_SHORT ${VERSION_SHA1_SHORT})
set (jaulib_GIT_DIRTY          ${GIT_WORKDIR_DIRTY})
set (jaulib_VERSION_LONG       ${VERSION_LONG})
set (jaulib_VERSION_SHORT      ${VERSION_SHORT})
set (jaulib_VERSION_API        ${VERSION_API})

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/version.cpp.in
                ${CMAKE_CURRENT_BINARY_DIR}/src/version.cpp)

set (CMAKE_SWIG_FLAGS "")

set (jaulib_DOCS_DIR documentation)

find_path (SYSTEM_USR_DIR "stdlib.h")
include_directories (${SYSTEM_USR_DIR})

add_subdirectory (src)

message(STATUS "${PROJECT_NAME} TEST_WITH_SUDO = ${TEST_WITH_SUDO} (pre-set)")
if(NOT DEFINED TEST_WITH_SUDO)
    set(TEST_WITH_SUDO OFF)
    message(STATUS "${PROJECT_NAME} TEST_WITH_SUDO ${TEST_WITH_SUDO} (default)")
else()
    message(STATUS "${PROJECT_NAME} TEST_WITH_SUDO ${TEST_WITH_SUDO} (user)")
endif()

IF(BUILDJAVA)
    configure_file (${CMAKE_CURRENT_SOURCE_DIR}/java_base/manifest.txt.in ${CMAKE_CURRENT_BINARY_DIR}/java_base/manifest.txt)
    configure_file (${CMAKE_CURRENT_SOURCE_DIR}/java_jni/manifest.txt.in ${CMAKE_CURRENT_BINARY_DIR}/java_jni/manifest.txt)
    configure_file (${CMAKE_CURRENT_SOURCE_DIR}/java_net/manifest.txt.in ${CMAKE_CURRENT_BINARY_DIR}/java_net/manifest.txt)
    configure_file (${CMAKE_CURRENT_SOURCE_DIR}/java_pkg/manifest.txt.in ${CMAKE_CURRENT_BINARY_DIR}/java_pkg/manifest.txt)
    configure_file (${CMAKE_CURRENT_SOURCE_DIR}/java_fat/manifest.txt.in ${CMAKE_CURRENT_BINARY_DIR}/java_fat/manifest.txt)
    configure_file (${CMAKE_CURRENT_SOURCE_DIR}/java_fat/manifest-fat.txt.in ${CMAKE_CURRENT_BINARY_DIR}/java_fat/manifest-fat.txt)
    add_subdirectory (java_base)
    add_subdirectory (java_jni)
    add_subdirectory (java_net)
    add_subdirectory (java_pkg)
    add_subdirectory (java_fat)
    # add_subdirectory (examples/java)
ENDIF(BUILDJAVA)

# add a target to generate API documentation with Doxygen
find_package (Doxygen)
if (DOXYGEN_FOUND)
  configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.cpp.out @ONLY)
  if (BUILDJAVA)
    configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.java.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.java.out @ONLY)
    add_custom_command (OUTPUT ${jaulib_DOCS_DIR}/cpp ${jaulib_DOCS_DIR}/java
      COMMAND ${CMAKE_COMMAND} -E make_directory ${jaulib_DOCS_DIR}
      COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.cpp.out
      COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.java.out
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/ ${CMAKE_CURRENT_SOURCE_DIR}/java_base/ ${CMAKE_CURRENT_SOURCE_DIR}/java_jni/ ${CMAKE_CURRENT_SOURCE_DIR}/java_net/ ${CMAKE_CURRENT_SOURCE_DIR}/java_pkg/ ${CMAKE_CURRENT_SOURCE_DIR}/test/
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
    add_custom_target(doc_jau
      DEPENDS ${jaulib_DOCS_DIR}/cpp ${jaulib_DOCS_DIR}/java)
  else ()
    add_custom_command (OUTPUT ${jaulib_DOCS_DIR}/cpp
      COMMAND ${CMAKE_COMMAND} -E make_directory ${jaulib_DOCS_DIR}
      COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.cpp.out
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/ ${CMAKE_CURRENT_SOURCE_DIR}/test/
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
    add_custom_target(doc_jau
      DEPENDS ${jaulib_DOCS_DIR}/cpp)
  endif ()
endif (DOXYGEN_FOUND)

if (BUILD_TESTING)
  enable_testing ()
  add_subdirectory (test)
  if (BUILDJAVA)
      configure_file (${CMAKE_CURRENT_SOURCE_DIR}/test/java/manifest.txt.in ${CMAKE_CURRENT_BINARY_DIR}/test/java/manifest.txt)
      configure_file (${CMAKE_CURRENT_SOURCE_DIR}/test/java/manifest-fat.txt.in ${CMAKE_CURRENT_BINARY_DIR}/test/java/manifest-fat.txt)
      add_subdirectory (test/java)
  endif(BUILDJAVA)
endif(BUILD_TESTING)

