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
  ${PROJECT_SOURCE_DIR}/java_jni/jni/jau/JVM_JNI8.cxx
)

set (CMAKE_SHARED_LINKER_FLAGS "-Wl,--as-needed")

add_library (jaulib_pkg_jni SHARED ${jaulib_pkg_JNI_SRCS})
target_link_libraries(jaulib_pkg_jni ${JNI_LIBRARIES})

set_target_properties(
    jaulib_pkg_jni
    PROPERTIES
    SOVERSION ${jaulib_VERSION_MAJOR}
    VERSION ${jaulib_VERSION_STRING}
)

install(TARGETS jaulib_pkg_jni LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

add_dependencies(jaulib_pkg_jni jaulib jaulib_base_jar jaulib_jni_jar jaulib_net_jar jaulib_pkg_jar)

