# java/CMakeLists.txt

#                    COMMAND ${CMAKE_COMMAND} 
#                            -DOS_AND_ARCH=${OS_AND_ARCH}
#                            -Dos_and_arch_slash=${os_and_arch_slash}
#                            -Dos_and_arch_dot=${os_and_arch_dot}
#                            -D_jarfile=${CMAKE_CURRENT_BINARY_DIR}/jaulib_jni-natives-${OS_AND_ARCH}.jar
#                            -D_workdir=${CMAKE_CURRENT_BINARY_DIR}
#                            -D_module=jni
#                            -P ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/modules/MakeNativeJar.cmake

set(jaulib_jar_file ${CMAKE_CURRENT_BINARY_DIR}/jaulib.jar CACHE FILEPATH "jaulib jar file" FORCE)
set(jaulib_fat_jar_file ${CMAKE_CURRENT_BINARY_DIR}/jaulib-fat.jar CACHE FILEPATH "jaulib fat jar file" FORCE)
set(jaulib_java_src_file ${CMAKE_CURRENT_BINARY_DIR}/jaulib-java-src.zip CACHE FILEPATH "jaulib java source file" FORCE)

add_custom_command (OUTPUT ${jaulib_jar_file} ${jaulib_fat_jar_file} ${jaulib_java_src_file}
                    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                    COMMAND ${CMAKE_COMMAND}
                            -D_srcdir=${CMAKE_CURRENT_BINARY_DIR}/../java_jni/jni
                            -D_dstdir=${CMAKE_CURRENT_BINARY_DIR}/natives/${OS_AND_ARCH}
                            -D_srcbasename=${CMAKE_SHARED_LIBRARY_PREFIX}jaulib_jni_jni${CMAKE_SHARED_LIBRARY_SUFFIX}.${jaulib_VERSION_STRING}
                            -D_dstbasename=${CMAKE_SHARED_LIBRARY_PREFIX}jaulib_jni_jni${CMAKE_SHARED_LIBRARY_SUFFIX}
                            -P ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/modules/FileCopyS2D.cmake
                    COMMAND ${CMAKE_COMMAND}
                            -D_srcdir=${CMAKE_CURRENT_BINARY_DIR}/../java_pkg/jni
                            -D_dstdir=${CMAKE_CURRENT_BINARY_DIR}/natives/${OS_AND_ARCH}
                            -D_srcbasename=${CMAKE_SHARED_LIBRARY_PREFIX}jaulib_pkg_jni${CMAKE_SHARED_LIBRARY_SUFFIX}.${jaulib_VERSION_STRING}
                            -D_dstbasename=${CMAKE_SHARED_LIBRARY_PREFIX}jaulib_pkg_jni${CMAKE_SHARED_LIBRARY_SUFFIX}
                            -P ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/modules/FileCopyS2D.cmake
                    COMMAND ${JAR}
                       --create --file ${jaulib_jar_file}
                       --manifest ${CMAKE_CURRENT_BINARY_DIR}/manifest.txt
                       -C ${CMAKE_CURRENT_BINARY_DIR}/../java_base/${CMAKE_FILES_DIRECTORY}/jaulib_base_jar.dir/ jau
                       -C ${CMAKE_CURRENT_BINARY_DIR}/../java_base/${CMAKE_FILES_DIRECTORY}/jaulib_base_jar.dir/ org
                       -C ${CMAKE_CURRENT_BINARY_DIR}/../java_jni/${CMAKE_FILES_DIRECTORY}/jaulib_jni_jar.dir/ jau
                       -C ${CMAKE_CURRENT_BINARY_DIR}/../java_jni/${CMAKE_FILES_DIRECTORY}/jaulib_jni_jar.dir/ org
                       -C ${CMAKE_CURRENT_BINARY_DIR}/../java_net/${CMAKE_FILES_DIRECTORY}/jaulib_net_jar.dir/ org
                       -C ${CMAKE_CURRENT_BINARY_DIR}/../java_pkg/${CMAKE_FILES_DIRECTORY}/jaulib_pkg_jar.dir/ jau
                       -C ${CMAKE_CURRENT_BINARY_DIR}/../java_pkg/${CMAKE_FILES_DIRECTORY}/jaulib_pkg_jar.dir/ org
                    COMMAND ${JAR}
                       --create --file ${jaulib_fat_jar_file}
                       --manifest ${CMAKE_CURRENT_BINARY_DIR}/manifest-fat.txt
                       -C ${CMAKE_CURRENT_BINARY_DIR}/../java_base/${CMAKE_FILES_DIRECTORY}/jaulib_base_jar.dir/ jau
                       -C ${CMAKE_CURRENT_BINARY_DIR}/../java_base/${CMAKE_FILES_DIRECTORY}/jaulib_base_jar.dir/ org
                       -C ${CMAKE_CURRENT_BINARY_DIR}/../java_jni/${CMAKE_FILES_DIRECTORY}/jaulib_jni_jar.dir/ jau
                       -C ${CMAKE_CURRENT_BINARY_DIR}/../java_jni/${CMAKE_FILES_DIRECTORY}/jaulib_jni_jar.dir/ org
                       -C ${CMAKE_CURRENT_BINARY_DIR}/../java_net/${CMAKE_FILES_DIRECTORY}/jaulib_net_jar.dir/ org
                       -C ${CMAKE_CURRENT_BINARY_DIR}/../java_pkg/${CMAKE_FILES_DIRECTORY}/jaulib_pkg_jar.dir/ jau
                       -C ${CMAKE_CURRENT_BINARY_DIR}/../java_pkg/${CMAKE_FILES_DIRECTORY}/jaulib_pkg_jar.dir/ org
                       -C ${CMAKE_CURRENT_BINARY_DIR}/ natives
                    COMMAND ${JAR}
                       --create --file ${jaulib_java_src_file}
                       --no-manifest
                       -C ${CMAKE_CURRENT_SOURCE_DIR}/../java_base jau
                       -C ${CMAKE_CURRENT_SOURCE_DIR}/../java_base org
                       -C ${CMAKE_CURRENT_SOURCE_DIR}/../java_jni jau
                       -C ${CMAKE_CURRENT_SOURCE_DIR}/../java_jni org
                       -C ${CMAKE_CURRENT_SOURCE_DIR}/../java_net org
                       -C ${CMAKE_CURRENT_SOURCE_DIR}/../java_pkg jau
                       -C ${CMAKE_CURRENT_SOURCE_DIR}/../java_pkg org
                    DEPENDS jaulib_base_jar jaulib_jni_jar jaulib_net_jar jaulib_pkg_jar jaulib_jni_jni jaulib_pkg_jni
                    COMMENT "producing jaulib [fat] jar files and its java source zip"
                    VERBATIM
                   )

add_custom_target (jaulib_fat_jar ALL DEPENDS ${jaulib_jar_file} ${jaulib_fat_jar_file} ${jaulib_java_src_file})

add_dependencies(jaulib_fat_jar jaulib_base_jar jaulib_jni_jar jaulib_net_jar jaulib_pkg_jar jaulib_jni_jni jaulib_pkg_jni)

install (FILES ${jaulib_jar_file} ${jaulib_fat_jar_file} ${jaulib_java_src_file} DESTINATION ${CMAKE_INSTALL_LIBDIR}/../lib/java)


