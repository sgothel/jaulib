# java/CMakeLists.txt

set(jaulib_base_jar_file ${CMAKE_CURRENT_BINARY_DIR}/jaulib_base.jar CACHE FILEPATH "jaulib base jar file" FORCE)

set(CMAKE_JNI_TARGET TRUE)
file(GLOB_RECURSE JAVA_SOURCES "*.java")

set(CMAKE_JAVA_COMPILE_FLAGS ${jaulib_JAVA_COMPILE_FLAGS})
add_jar(jaulib_base_jar 
        ${JAVA_SOURCES} jau/info.txt
        MANIFEST ${CMAKE_CURRENT_BINARY_DIR}/manifest.txt
        OUTPUT_NAME jaulib_base
        GENERATE_NATIVE_HEADERS jaulib_base_javah
            DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/jaulib_base_jar.dir/jni"
)

# JNI header generation is satisfied by above 'add_jar(.. GENERATE_NATIVE_HEADERS <target> DESTINATION <dir>)',
# note that its GENERATE_NATIVE_HEADERS target is a dummy target, since jni/direct_bt defines target jni_jaulib_base.
# Weakness of not directly checking build dependency of javadirect_bt against generated headers exists,
# however, it is unrealistic to assume that the transient generated JNI header will be edited manually
# within the process.
# Therefor we can use the dummy target javadirect_bt_javah and JNI header generation will only
# occur when java sources have been modified, i.e. the jar file being actually build.
#
#add_custom_command (TARGET jaulib_base_jar
#  POST_BUILD
#  COMMAND ${CMAKE_COMMAND} -E echo "Generating JNI headers.."
#  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/jaulib_base_jar.dir"
#  COMMAND ${JAVAC} -h jni/ ${JAVA_SOURCES}
#)

set(JNI_HEADER_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/jaulib_base_jar.dir/jni")
# install (FILES ${CMAKE_CURRENT_BINARY_DIR}/jaulib_base.jar DESTINATION ${CMAKE_INSTALL_LIBDIR}/../lib/java)

