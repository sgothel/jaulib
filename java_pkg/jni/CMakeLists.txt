find_package(JNI REQUIRED)

set (jaulib_base_LIB_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/include
)

include_directories(
    ${JNI_INCLUDE_DIRS}
    ${jaulib_base_LIB_INCLUDE_DIRS}
    ${JNI_HEADER_PATH}
)

set (jaulib_pkg_JNI_SRCS
  ${PROJECT_SOURCE_DIR}/java_pkg/jni/jau/JarUtil.cxx
  ${PROJECT_SOURCE_DIR}/java_pkg/jni/jau/JVM_JNI8.cxx
)

set (CMAKE_SHARED_LINKER_FLAGS "-Wl,--as-needed")

add_library (jaulib_pkg_jni SHARED ${jaulib_pkg_JNI_SRCS})
target_link_libraries(jaulib_pkg_jni ${JNI_LIBRARIES})

if(USE_STRIP)
add_custom_command(TARGET jaulib_pkg_jni POST_BUILD
                   COMMAND ${STRIP} ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}jaulib_pkg_jni${CMAKE_SHARED_LIBRARY_SUFFIX}.${jaulib_VERSION_SHORT}
                   COMMENT "stripping jaulib_pkg_jni"
                   VERBATIM
                   )
endif(USE_STRIP)

set_target_properties(
    jaulib_pkg_jni
    PROPERTIES
    SOVERSION ${jaulib_VERSION_MAJOR}
    VERSION ${jaulib_VERSION_SHORT}
)

install(TARGETS jaulib_pkg_jni LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

add_dependencies(jaulib_pkg_jni jaulib jaulib_base_jar jaulib_jni_jar jaulib_net_jar jaulib_pkg_jar)

