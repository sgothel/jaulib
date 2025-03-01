cmake_minimum_required (VERSION 3.21.0 FATAL_ERROR)
include(JaulibSetup.cmake)
JaulibPreset()

project (jaulib LANGUAGES C CXX)

message(STATUS "CMake ${CMAKE_VERSION}")

FIND_PACKAGE (Threads REQUIRED)

if(EMSCRIPTEN)
    message(STATUS "Jaulib: EMSCRIPTEN (wasm)")
    # See https://emscripten.org/docs/tools_reference/settings_reference.html
    #
    set(jaulib_EMS_FLAGS "-Wno-unused-command-line-argument")
    set(jaulib_EMS_FLAGS ${${PROJECT_NAME}_EMS_FLAGS} "SHELL:-s STANDALONE_WASM")
    # set(jaulib_EMS_FLAGS ${${PROJECT_NAME}_EMS_FLAGS} "SHELL:-s WASM=1" "SHELL:-s LZ4=1" "SHELL:-s EXPORTED_RUNTIME_METHODS=cwrap")
    # set(jaulib_EMS_FLAGS ${${PROJECT_NAME}_EMS_FLAGS} "SHELL:-s ALLOW_MEMORY_GROWTH=1")
    # set(jaulib_EMS_FLAGS ${jaulib_EMS_FLAGS} "-pthread") # fights w/ ALLOW_MEMORY_GROWTH
    # set(jaulib_EMS_FLAGS ${jaulib_EMS_FLAGS} "SHELL:-s MEMORY64=1") # wasm64 end-to-end: wasm32 object file can't be linked in wasm64 mode
    # set(jaulib_EMS_FLAGS ${jaulib_EMS_FLAGS} "SHELL:-s ASSERTIONS=1")
    # set(jaulib_EMS_FLAGS ${jaulib_EMS_FLAGS} "SHELL:-s STACK_OVERFLOW_CHECK=1") # cheap cockie magic, enables CHECK_NULL_WRITES

    #
    # set(EMS_EXE_LD_FLAGS "SHELL:-s SIDE_MODULE=1")
    set(jaulib_EMS_EXE_LD_FLAGS ${jaulib_EMS_FLAGS})
    set(jaulib_EMS_STATIC_LD_FLAGS )
    message(STATUS "Jaulib: jaulib_EMS_FLAGS = ${jaulib_EMS_FLAGS}")
    message(STATUS "Jaulib: jaulib_EMS_EXE_LD_FLAGS = ${jaulib_EMS_EXE_LD_FLAGS}")
    message(STATUS "Jaulib: jaulib_EMS_STATIC_LD_FLAGS = ${jaulib_EMS_STATIC_LD_FLAGS}")
else()
    message(STATUS "Jaulib: No EMSCRIPTEN (wasm)")
endif()

JaulibSetup()

# Fixup clangd w/ absolute path
set (jaulib_clangd_source_args "-isystem, ${CMAKE_CURRENT_SOURCE_DIR}/include, -isystem, ${CMAKE_CURRENT_SOURCE_DIR}/src, -isystem, ${CMAKE_CURRENT_SOURCE_DIR}/java_jni/jni, -isystem, ${CMAKE_CURRENT_SOURCE_DIR}/java_jni/jni/jau")
set (jaulib_clangd_source_args "${jaulib_clangd_source_args}, -isystem, ${CMAKE_CURRENT_SOURCE_DIR}/include/catch2_jau, -isystem, ${CMAKE_CURRENT_SOURCE_DIR}/test")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/clangd.in
                ${CMAKE_CURRENT_BINARY_DIR}/.clangd)
file(COPY_FILE ${CMAKE_CURRENT_BINARY_DIR}/.clangd ${CMAKE_CURRENT_SOURCE_DIR}/.clangd)

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
  configure_file (${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.cpp.out @ONLY)
  file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/doc/doxygen-awesome.css DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
  if (BUILDJAVA)
    configure_file (${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.java.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.java.out @ONLY)
    file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/doc/doxygen-awesome.css DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
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

