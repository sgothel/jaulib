# java/CMakeLists.txt

#                    COMMAND ${CMAKE_COMMAND} 
#                            -DOS_AND_ARCH=${OS_AND_ARCH}
#                            -Dos_and_arch_slash=${os_and_arch_slash}
#                            -Dos_and_arch_dot=${os_and_arch_dot}
#                            -D_jarfile=${CMAKE_CURRENT_BINARY_DIR}/jaulib_jni-natives-${OS_AND_ARCH}.jar
#                            -D_workdir=${CMAKE_CURRENT_BINARY_DIR}
#                            -D_module=jni
#                            -P ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/modules/MakeNativeJar.cmake

set(jaulib_fat_jar_file ${CMAKE_CURRENT_BINARY_DIR}/jaulib_fat.jar CACHE FILEPATH "jaulib fat jar file" FORCE)

add_custom_command (OUTPUT ${jaulib_fat_jar_file}
                    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                    COMMAND ${CMAKE_COMMAND}
                            -D_srcdir=${CMAKE_CURRENT_BINARY_DIR}/../java_jni/jni
                            -D_dstdir=${CMAKE_CURRENT_BINARY_DIR}/natives/${OS_AND_ARCH}
                            -D_srcbasename=libjaulib_jni_jni.so.${jaulib_VERSION_STRING}
                            -D_dstbasename=libjaulib_jni_jni.so
                            -P ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/modules/FileCopyS2D.cmake
                    COMMAND ${CMAKE_COMMAND}
                            -D_srcdir=${CMAKE_CURRENT_BINARY_DIR}/../java_pkg/jni
                            -D_dstdir=${CMAKE_CURRENT_BINARY_DIR}/natives/${OS_AND_ARCH}
                            -D_srcbasename=libjaulib_pkg_jni.so.${jaulib_VERSION_STRING}
                            -D_dstbasename=libjaulib_pkg_jni.so
                            -P ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/modules/FileCopyS2D.cmake
                    COMMAND ${JAR}
                       --create --file ${jaulib_fat_jar_file}
                       --manifest ${CMAKE_CURRENT_BINARY_DIR}/manifest.txt
                       -C ${CMAKE_CURRENT_BINARY_DIR}/../java_base/${CMAKE_FILES_DIRECTORY}/jaulib_base_jar.dir/ jau
                       -C ${CMAKE_CURRENT_BINARY_DIR}/../java_base/${CMAKE_FILES_DIRECTORY}/jaulib_base_jar.dir/ org
                       -C ${CMAKE_CURRENT_BINARY_DIR}/../java_jni/${CMAKE_FILES_DIRECTORY}/jaulib_jni_jar.dir/ jau
                       -C ${CMAKE_CURRENT_BINARY_DIR}/../java_jni/${CMAKE_FILES_DIRECTORY}/jaulib_jni_jar.dir/ org
                       -C ${CMAKE_CURRENT_BINARY_DIR}/../java_net/${CMAKE_FILES_DIRECTORY}/jaulib_net_jar.dir/ org
                       -C ${CMAKE_CURRENT_BINARY_DIR}/../java_pkg/${CMAKE_FILES_DIRECTORY}/jaulib_pkg_jar.dir/ jau
                       -C ${CMAKE_CURRENT_BINARY_DIR}/../java_pkg/${CMAKE_FILES_DIRECTORY}/jaulib_pkg_jar.dir/ org
                       -C ${CMAKE_CURRENT_BINARY_DIR}/ natives
                    DEPENDS jaulib_base_jar jaulib_jni_jar jaulib_net_jar jaulib_pkg_jar jaulib_jni_jni jaulib_pkg_jni
                   )

add_custom_target (jaulib_fat_jar ALL 
                   DEPENDS ${jaulib_fat_jar_file}
                  )

add_dependencies(jaulib_fat_jar jaulib_base_jar jaulib_jni_jar jaulib_net_jar jaulib_pkg_jar jaulib_jni_jni jaulib_pkg_jni)

install (FILES ${jaulib_fat_jar_file} DESTINATION ${CMAKE_INSTALL_LIBDIR}/../lib/java)


