include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

set (catch2_LIB_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/../include/catch2/catch_amalgamated.cpp
)

add_library (catch2 STATIC ${catch2_LIB_SRCS})

target_compile_options(catch2 PUBLIC "-DCATCH_AMALGAMATED_CUSTOM_MAIN=1")

# install(TARGETS catch2 LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

set(ownership_fix_tag ${CMAKE_CURRENT_SOURCE_DIR}/../test_data/ownership_fix_tag CACHE FILEPATH "test_data ownership fix" FORCE)
add_custom_command (OUTPUT ${ownership_fix_tag}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../test_data
    # Ubuntu `/usr/bin/ln`, Debian `/bin/ln`
    # Ubuntu `/usr/bin/chgrp`, Debian `/bin/chgrp`
    COMMAND chgrp cdrom file_01.txt
    COMMAND chgrp cdrom dir_01/dir_01_02
    COMMENT "changed group ownership of 2 test_data entries to cdrom"
    VERBATIM
)
add_custom_target (ownership_fix ALL DEPENDS ${ownership_fix_tag})

# These examples use the standard separate compilation
file(GLOB_RECURSE SOURCES_IDIOMATIC_TEST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "test_*.cpp")

string( REPLACE ".cpp" "" BASENAMES_IDIOMATIC_TEST "${SOURCES_IDIOMATIC_TEST}" )
set( TARGETS_IDIOMATIC_TEST ${BASENAMES_IDIOMATIC_TEST} )

foreach( name ${TARGETS_IDIOMATIC_TEST} )
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} jaulib catch2)
    add_dependencies(${name} jaulib catch2)
    add_test (NAME ${name} COMMAND ${name})
endforeach()

IF( ( TEST_WITH_SUDO ) AND ( "${OS_NAME}" STREQUAL "linux" ) ) 
    set(test_data_sqfs_file ${CMAKE_CURRENT_BINARY_DIR}/test_data.sqfs CACHE FILEPATH "test_data sqfs image file" FORCE)

    add_custom_command (OUTPUT ${test_data_sqfs_file}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..
        COMMAND /usr/bin/mksquashfs test_data ${CMAKE_CURRENT_BINARY_DIR}/test_data.sqfs -comp lzo
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
        target_link_libraries(${name} jaulib catch2 cap)
        add_dependencies(${name} jaulib catch2 test_data_sqfs)
        add_test (NAME ${name} COMMAND sudo -E /sbin/capsh --caps=cap_sys_admin,cap_setuid,cap_setgid+eip\ cap_setpcap+ep
                                   --keep=1 --user=$ENV{USER} --addamb=cap_sys_admin,cap_setuid,cap_setgid+eip
                                   -- -c "ulimit -c unlimited; ./${name}")
        #
        # add_test (NAME ${name} COMMAND sudo -E ./${name})
    endforeach()
ELSE()
    message(STATUS "testsudo_* disabled: TEST_WITH_SUDO ${TEST_WITH_SUDO}, OS_NAME ${OS_NAME}")
ENDIF()

