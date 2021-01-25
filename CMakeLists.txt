cmake_minimum_required (VERSION 3.13.0 FATAL_ERROR)
project (jaulib LANGUAGES C CXX)

FIND_PACKAGE (Threads REQUIRED)

include(JaulibSetup.cmake)

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/version.cpp.in
                ${CMAKE_CURRENT_BINARY_DIR}/src/version.cpp)

# this is the library version, independant of git revision
set (jaulib_VERSION_MAJOR ${VERSION_MAJOR})
set (jaulib_VERSION_MINOR ${VERSION_MINOR})
set (jaulib_VERSION_PATCH ${VERSION_PATCH})
set (jaulib_VERSION_STRING "${jaulib_VERSION_MAJOR}.${jaulib_VERSION_MINOR}.${jaulib_VERSION_PATCH}")

set (CMAKE_SWIG_FLAGS "")

set (jaulib_DOCS_DIR documentation)

find_path (SYSTEM_USR_DIR "stdlib.h")
include_directories (${SYSTEM_USR_DIR})

add_subdirectory (src)

IF(BUILDJAVA)
    configure_file (${CMAKE_CURRENT_SOURCE_DIR}/java_base/manifest.txt.in ${CMAKE_CURRENT_BINARY_DIR}/java_base/manifest.txt)
    configure_file (${CMAKE_CURRENT_SOURCE_DIR}/java_jni/manifest.txt.in ${CMAKE_CURRENT_BINARY_DIR}/java_jni/manifest.txt)
    configure_file (${CMAKE_CURRENT_SOURCE_DIR}/java_net/manifest.txt.in ${CMAKE_CURRENT_BINARY_DIR}/java_net/manifest.txt)
    configure_file (${CMAKE_CURRENT_SOURCE_DIR}/java_pkg/manifest.txt.in ${CMAKE_CURRENT_BINARY_DIR}/java_pkg/manifest.txt)
    configure_file (${CMAKE_CURRENT_SOURCE_DIR}/java_fat/manifest.txt.in ${CMAKE_CURRENT_BINARY_DIR}/java_fat/manifest.txt)
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
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/api/tinyb/*.hpp ${CMAKE_CURRENT_SOURCE_DIR}/api/direct_bt/*.hpp ${CMAKE_CURRENT_SOURCE_DIR}/java/org/tinyb/*.java ${CMAKE_CURRENT_SOURCE_DIR}/java/tinyb/dbus/*java ${CMAKE_CURRENT_SOURCE_DIR}/java/direct_bt/tinyb/*java
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
    add_custom_target(doc_jau
      DEPENDS ${jaulib_DOCS_DIR}/cpp ${jaulib_DOCS_DIR}/java)
  else ()
    add_custom_command (OUTPUT ${jaulib_DOCS_DIR}/cpp
      COMMAND ${CMAKE_COMMAND} -E make_directory ${jaulib_DOCS_DIR}
      COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.cpp.out
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/
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
      add_subdirectory (test/java)
  endif(BUILDJAVA)
endif(BUILD_TESTING)

