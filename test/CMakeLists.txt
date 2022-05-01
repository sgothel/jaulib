include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

set (catch2_LIB_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/../include/catch2/catch_amalgamated.cpp
)

add_library (catch2 STATIC ${catch2_LIB_SRCS})

if(CMAKE_COMPILER_IS_GNUCC)
target_compile_options(catch2 PUBLIC "-Wno-error=format-overflow" "-Wno-format-overflow")
endif(CMAKE_COMPILER_IS_GNUCC)

# install(TARGETS catch2 LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

# These examples use the standard separate compilation
file(GLOB_RECURSE SOURCES_IDIOMATIC_EXAMPLES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "test_*.cpp")

string( REPLACE ".cpp" "" BASENAMES_IDIOMATIC_EXAMPLES "${SOURCES_IDIOMATIC_EXAMPLES}" )
set( TARGETS_IDIOMATIC_EXAMPLES ${BASENAMES_IDIOMATIC_EXAMPLES} )

foreach( name ${TARGETS_IDIOMATIC_EXAMPLES} )
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} jaulib catch2)
    add_dependencies(${name} jaulib catch2)
endforeach()

foreach(name ${TARGETS_IDIOMATIC_EXAMPLES})
    add_test (NAME ${name} COMMAND ${name})
endforeach()


