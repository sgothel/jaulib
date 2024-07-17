include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
  ${CMAKE_CURRENT_SOURCE_DIR}/../include/catch2_jau
)

set (catch2_LIB_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/../include/catch2/catch_amalgamated.cpp
)

add_library (catch2 STATIC ${catch2_LIB_SRCS})
target_compile_options(catch2 PRIVATE -DCATCH_AMALGAMATED_CUSTOM_MAIN=1 -Wno-error=unused-but-set-variable -Wno-unused-but-set-variable)
set_target_properties(catch2 PROPERTIES CXX_CLANG_TIDY "")
# install(TARGETS catch2 LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

#
# testlib is produced for test_os_dynlink
# and has the proper SONAME to be found by dlopen w/ system path
#
set (testlib_VERSION_MAJOR "1")
set (testlib_VERSION_SHORT "${VERSION_MAJOR}.0.2")
set (testlib_LIB_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/debug.cpp
)
if (EMSCRIPTEN)
  add_library (testlib STATIC ${testlib_LIB_SRCS})
  target_link_options(testlib PUBLIC "${jaulib_STATIC_LINKER_FLAGS}")
else()
  add_library (testlib SHARED ${testlib_LIB_SRCS})
  target_link_options(testlib PUBLIC "${jaulib_SHARED_LINKER_FLAGS}")
endif()

target_compile_options(testlib PUBLIC "${jaulib_CXX_FLAGS}")

target_link_libraries (
  testlib
)
set_target_properties(
   testlib
   PROPERTIES
   SOVERSION ${testlib_VERSION_MAJOR}
   VERSION ${testlib_VERSION_SHORT}
)

set(ownership_fix_tag ${CMAKE_CURRENT_SOURCE_DIR}/../test_data/ownership_fix_tag CACHE FILEPATH "test_data ownership fix" FORCE)
add_custom_command (OUTPUT ${ownership_fix_tag}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../test_data
    # Ubuntu `/usr/bin/ln`, Debian `/bin/ln`
    # FreeBSD,Ubuntu `/usr/bin/chgrp`, Debian `/bin/chgrp`
    # Ubuntu, Debian group 24: cdrom
    # FreeBSD, Ubuntu, Debian group 44: video
    # Alpine/Linux group 27: video
    COMMAND chgrp -f video file_01.txt
    COMMAND chgrp -f video dir_01/dir_01_02
    COMMENT "changed group ownership of 2 test_data entries to video(??)"
    VERBATIM
)
add_custom_target (ownership_fix ALL DEPENDS ${ownership_fix_tag})

# These examples use the standard separate compilation
file(GLOB_RECURSE SOURCES_IDIOMATIC_TEST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "test_*.cpp")

string( REPLACE ".cpp" "" BASENAMES_IDIOMATIC_TEST "${SOURCES_IDIOMATIC_TEST}" )
set( TARGETS_IDIOMATIC_TEST ${BASENAMES_IDIOMATIC_TEST} )

foreach( name ${TARGETS_IDIOMATIC_TEST} )
    add_executable(${name} ${name}.cpp)
    target_compile_options(${name} PUBLIC "${jaulib_CXX_FLAGS}")
    target_link_options(${name} PUBLIC "${jaulib_EXE_LINKER_FLAGS}")

    target_link_libraries(${name} jaulib catch2)
    add_dependencies(${name} jaulib catch2)
    add_test (NAME ${name} COMMAND ${name})
    if(DEFINED CMAKE_CXX_CLANG_TIDY)
        set_target_properties(${name} PROPERTIES CXX_CLANG_TIDY "${CMAKE_CXX_CLANG_TIDY};--checks;-bugprone-chained-comparison")
    endif()
endforeach()

IF( ( TEST_WITH_SUDO ) AND ( "${OS_NAME}" STREQUAL "linux" ) ) 
    set(test_data_sqfs_file ${CMAKE_CURRENT_BINARY_DIR}/test_data.sqfs CACHE FILEPATH "test_data sqfs image file" FORCE)

    add_custom_command (OUTPUT ${test_data_sqfs_file}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..
        COMMAND mksquashfs test_data ${CMAKE_CURRENT_BINARY_DIR}/test_data.sqfs -comp lzo
        COMMENT "producing test_data sqfs image file"
        VERBATIM
    )
    add_custom_target (test_data_sqfs ALL DEPENDS ${test_data_sqfs_file})
    message(STATUS "test_data.sqfs to be produced in ${CMAKE_CURRENT_BINARY_DIR}/test_data.sqfs")

    # These examples use the standard separate compilation
    file(GLOB_RECURSE SOURCES_IDIOMATIC_TESTSUDO RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "testsudo_*.cpp")

    string( REPLACE ".cpp" "" BASENAMES_IDIOMATIC_TESTSUDO "${SOURCES_IDIOMATIC_TESTSUDO}" )
    set( TARGETS_IDIOMATIC_TESTSUDO ${BASENAMES_IDIOMATIC_TESTSUDO} )

    foreach( name ${TARGETS_IDIOMATIC_TESTSUDO} )
        add_executable(${name} ${name}.cpp)
        target_compile_options(${name} PUBLIC "${jaulib_CXX_FLAGS}")
        target_link_options(${name} PUBLIC "${jaulib_EXE_LINKER_FLAGS}")

        target_link_libraries(${name} jaulib catch2 cap)
        add_dependencies(${name} jaulib catch2 test_data_sqfs)
        add_test (NAME ${name} COMMAND sudo -E /sbin/capsh --caps=cap_sys_admin,cap_setuid,cap_setgid+eip\ cap_setpcap+ep
                                   --keep=1 --user=$ENV{USER} --addamb=cap_sys_admin,cap_setuid,cap_setgid+eip
                                   -- -c "ulimit -c unlimited; ./${name}")
        if(DEFINED CMAKE_CXX_CLANG_TIDY)
            set_target_properties(${name} PROPERTIES CXX_CLANG_TIDY "${CMAKE_CXX_CLANG_TIDY};--checks;-bugprone-chained-comparison")
        endif()
        #
        # add_test (NAME ${name} COMMAND sudo -E ./${name})
    endforeach()
ELSE()
    message(STATUS "testsudo_* disabled: TEST_WITH_SUDO ${TEST_WITH_SUDO}, OS_NAME ${OS_NAME}")
ENDIF()

