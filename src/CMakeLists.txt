set (jaulib_LIB_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

include_directories(
  ${jaulib_LIB_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
)

set (jaulib_LIB_SRCS
  dfa_utf8_decode.cpp
  environment.cpp
  debug.cpp
  basic_types.cpp
# autogenerated files
  ${CMAKE_CURRENT_BINARY_DIR}/version.cpp
)

add_library (jaulib SHARED ${jaulib_LIB_SRCS})
target_link_libraries (
  jaulib
  unwind
  ${CMAKE_THREAD_LIBS_INIT}
)

if(USE_STRIP)
add_custom_command(TARGET jaulib POST_BUILD
                   COMMAND ${STRIP} ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}jaulib${CMAKE_SHARED_LIBRARY_SUFFIX}.${jaulib_VERSION_STRING}
                   COMMENT "stripping jaulib"
                   VERBATIM
                   )
endif(USE_STRIP)

set_target_properties(
   jaulib
   PROPERTIES
   SOVERSION ${jaulib_VERSION_MAJOR}
   VERSION ${jaulib_VERSION_STRING}
)
install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../include/jau/ DESTINATION include/jau)

install(TARGETS jaulib LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

