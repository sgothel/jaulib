# java/CMakeLists.txt

set(jaulib_net_jar_file ${CMAKE_CURRENT_BINARY_DIR}/jaulib_net.jar CACHE FILEPATH "jaulib net jar file" FORCE)

set(CMAKE_JNI_TARGET TRUE)
file(GLOB_RECURSE JAVA_SOURCES "*.java")
set(CMAKE_JAVA_COMPILE_FLAGS ${jaulib_JAVA_COMPILE_FLAGS})
add_jar(jaulib_net_jar 
        ${JAVA_SOURCES}
        INCLUDE_JARS jaulib_base_jar jaulib_jni_jar
        MANIFEST ${CMAKE_CURRENT_BINARY_DIR}/manifest.txt
        OUTPUT_NAME jaulib_net
        GENERATE_NATIVE_HEADERS jaulib_net_javah
            DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/jaulib_net_jar.dir/jni"
)
add_dependencies(jaulib_net_jar jaulib_base_jar jaulib_jni_jar)
#add_dependencies(jaulib_pkg_jar jaulib_base_jar jaulib_jni_jar jaulib_net_jar)

set(JNI_HEADER_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/jaulib_net_jar.dir/jni")
# install (FILES ${CMAKE_CURRENT_BINARY_DIR}/jaulib_net.jar DESTINATION ${CMAKE_INSTALL_LIBDIR}/../lib/java)

