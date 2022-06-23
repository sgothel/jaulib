include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

set (catch2_LIB_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/../include/catch2/catch_amalgamated.cpp
)

add_library (catch2 STATIC ${catch2_LIB_SRCS})

target_compile_options(catch2 PUBLIC "-DCATCH_AMALGAMATED_CUSTOM_MAIN=1")

# install(TARGETS catch2 LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

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

IF(TEST_WITH_SUDO)
    # These examples use the standard separate compilation
    file(GLOB_RECURSE SOURCES_IDIOMATIC_TESTSUDO RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "testsudo_*.cpp")

    string( REPLACE ".cpp" "" BASENAMES_IDIOMATIC_TESTSUDO "${SOURCES_IDIOMATIC_TESTSUDO}" )
    set( TARGETS_IDIOMATIC_TESTSUDO ${BASENAMES_IDIOMATIC_TESTSUDO} )

    foreach( name ${TARGETS_IDIOMATIC_TESTSUDO} )
        add_executable(${name} ${name}.cpp)
        target_link_libraries(${name} jaulib catch2)
        add_dependencies(${name} jaulib catch2)
        #
        # cap_sys_admin is not enough to access /dev/loop*
        #    add_test (NAME ${name} COMMAND sudo -E /sbin/capsh --caps=cap_sys_admin+eip\ cap_setpcap,cap_setuid,cap_setgid+ep
        #                           --keep=1 --user=$ENV{USER} --addamb=cap_sys_admin+eip
        #                           -- -c "ulimit -c unlimited; ./${name}")
        #
        add_test (NAME ${name} COMMAND sudo -E ./${name})
    endforeach()
ENDIF(TEST_WITH_SUDO)
