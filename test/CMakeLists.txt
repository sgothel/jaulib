include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

set (catch2_LIB_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/../include/catch2/catch_amalgamated.cpp
)

add_library (catch2 STATIC ${catch2_LIB_SRCS})

target_compile_options(catch2 PUBLIC "-Wno-error=format-overflow")

# install(TARGETS catch2 LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

# These examples use the standard separate compilation
set( SOURCES_IDIOMATIC_EXAMPLES
    test_floatepsilon01.cpp
    test_functiondef01.cpp
    test_intdecstring01.cpp
    test_lfringbuffer01.cpp
    test_lfringbuffer11.cpp
    test_mm_sc_drf_00.cpp
    test_mm_sc_drf_01.cpp
    test_cowvector_perf01.cpp
    test_hashset_perf01.cpp
)

string( REPLACE ".cpp" "" BASENAMES_IDIOMATIC_EXAMPLES "${SOURCES_IDIOMATIC_EXAMPLES}" )
set( TARGETS_IDIOMATIC_EXAMPLES ${BASENAMES_IDIOMATIC_EXAMPLES} )

foreach( name ${TARGETS_IDIOMATIC_EXAMPLES} )
    add_executable(${name} ${name}.cpp)
endforeach()

set(ALL_EXAMPLE_TARGETS
  ${TARGETS_IDIOMATIC_EXAMPLES}
)

foreach(name ${ALL_EXAMPLE_TARGETS})
    target_link_libraries(${name} jaulib catch2)
    add_dependencies(${name} jaulib catch2)
    add_test (NAME ${name} COMMAND ${name})
endforeach()


