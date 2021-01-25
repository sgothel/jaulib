find_package(JNI REQUIRED)

set (jaulib_base_LIB_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/include
)

include_directories(
    ${JNI_INCLUDE_DIRS}
    ${jaulib_base_LIB_INCLUDE_DIRS}
    ${JNI_HEADER_PATH}
)

set (jaulib_jni_JNI_SRCS
  ${PROJECT_SOURCE_DIR}/java_jni/jni/jni_mem.cxx
  ${PROJECT_SOURCE_DIR}/java_jni/jni/helper_jni.cxx
  ${PROJECT_SOURCE_DIR}/java_jni/jni/jau/JVM_JNI8.cxx
  ${PROJECT_SOURCE_DIR}/java_jni/jni/jau/MachineDataInfoRuntime.cxx
  ${PROJECT_SOURCE_DIR}/java_jni/jni/jau/Clock.cxx
)

if(WIN32)
    set (jaulib_jni_JNI_SRCS ${jaulib_jni_JNI_SRCS} ${PROJECT_SOURCE_DIR}/java_jni/jni/jau/WindowsDynamicLinkerImpl_JNI.cxx)
else()
    set (jaulib_jni_JNI_SRCS ${jaulib_jni_JNI_SRCS} ${PROJECT_SOURCE_DIR}/java_jni/jni/jau/UnixDynamicLinkerImpl_JNI.cxx)
endif()

set (CMAKE_SHARED_LINKER_FLAGS "-Wl,--as-needed")

add_library (jaulib_jni_jni SHARED ${jaulib_jni_JNI_SRCS})
target_link_libraries(jaulib_jni_jni ${JNI_LIBRARIES} jaulib)

set_target_properties(
    jaulib_jni_jni
    PROPERTIES
    SOVERSION ${jaulib_VERSION_MAJOR}
    VERSION ${jaulib_VERSION_STRING}
)

install(TARGETS jaulib_jni_jni LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

add_dependencies(jaulib_jni_jni jaulib jaulib_base_jar jaulib_jni_jar)

