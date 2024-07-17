# java/CMakeLists.txt

set(jaulib_pkg_jar_file ${CMAKE_CURRENT_BINARY_DIR}/jaulib_pkg.jar CACHE FILEPATH "jaulib pkg jar file" FORCE)

set(CMAKE_JNI_TARGET TRUE)
file(GLOB_RECURSE JAVA_SOURCES "*.java")
set(CMAKE_JAVA_COMPILE_FLAGS ${jaulib_JAVA_COMPILE_FLAGS})
add_jar(jaulib_pkg_jar 
        ${JAVA_SOURCES}
        INCLUDE_JARS jaulib_base_jar jaulib_jni_jar jaulib_net_jar
        MANIFEST ${CMAKE_CURRENT_BINARY_DIR}/manifest.txt
        OUTPUT_NAME jaulib_pkg
        GENERATE_NATIVE_HEADERS jaulib_pkg_javah
            DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/jaulib_pkg_jar.dir/jni"
)
add_dependencies(jaulib_pkg_jar jaulib_base_jar jaulib_jni_jar jaulib_net_jar)

set(JNI_HEADER_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/jaulib_pkg_jar.dir/jni")
# install (FILES ${CMAKE_CURRENT_BINARY_DIR}/jaulib_pkg.jar DESTINATION ${CMAKE_INSTALL_LIBDIR}/../lib/java)

add_subdirectory (jni)

