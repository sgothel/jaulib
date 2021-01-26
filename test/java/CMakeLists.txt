# java/CMakeLists.txt

find_jar(JUNIT_JAR
  NAMES junit4 junit
  PATHS "/usr/share/java")

set(jaulib_test_jar_file ${CMAKE_CURRENT_BINARY_DIR}/jaulib-test.jar CACHE FILEPATH "jaulib test jar file" FORCE)

file(GLOB_RECURSE TEST_JAVA_SOURCES "*.java")

file(GLOB_RECURSE TEST_JAVA_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "Test*.java")

add_jar(jaulib_test
        ${TEST_JAVA_SOURCES}
        jau/test/info.txt
        jau/test/net/data/AssetURLConnectionTest.txt
        jau/test/net/data/RelativeData.txt
        jau/test/net/data2/RelativeData2.txt
        jau-test/info.txt
        INCLUDE_JARS ${jaulib_fat_jar_file} ${JUNIT_JAR}
        MANIFEST ${CMAKE_CURRENT_BINARY_DIR}/manifest.txt
        OUTPUT_NAME jaulib-test
)
add_dependencies(jaulib_test jaulib_fat_jar)
install (FILES ${jaulib_test_jar_file} DESTINATION ${CMAKE_INSTALL_LIBDIR}/../lib/java)

# set(ENABLE_jaulib_fat_test_jar_file ON)

#
# jaulib_fat_test_jar_file only demos how to merge jars into one fat jar with cmake
#
if(ENABLE_jaulib_fat_test_jar_file)
    set(jaulib_fat_test_jar_file ${CMAKE_CURRENT_BINARY_DIR}/jaulib-fat-test.jar CACHE FILEPATH "jaulib fat test jar file" FORCE)

    add_custom_command (OUTPUT ${jaulib_fat_test_jar_file}
                        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                        COMMAND ${CMAKE_COMMAND}
                                -D _dstdir:PATH='${CMAKE_CURRENT_BINARY_DIR}/temp'
                                -D _archives:FILEPATH="${jaulib_fat_jar_file} ${jaulib_test_jar_file}"
                                -P ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/modules/ExtractArchiveList.cmake
                        COMMAND ${JAR}
                           --create --file ${jaulib_fat_test_jar_file}
                           --manifest ${CMAKE_CURRENT_BINARY_DIR}/manifest-fat.txt
                           -C ${CMAKE_CURRENT_BINARY_DIR}/temp/ jau
                           -C ${CMAKE_CURRENT_BINARY_DIR}/temp/ org
                           -C ${CMAKE_CURRENT_BINARY_DIR}/temp/ natives
                           -C ${CMAKE_CURRENT_BINARY_DIR}/temp/ jau-test
                        DEPENDS jaulib_fat_jar jaulib_test
                        COMMENT "producing jaulib-fat-test.jar"
                        VERBATIM
                       )
    add_custom_target (jaulib_fat_test ALL DEPENDS ${jaulib_fat_test_jar_file})
    add_dependencies(jaulib_fat_test jaulib_fat_jar jaulib_test)
    install (FILES ${jaulib_fat_test_jar_file} DESTINATION ${CMAKE_INSTALL_LIBDIR}/../lib/java)
endif(ENABLE_jaulib_fat_test_jar_file)


string( REPLACE ".java" "" TEST_JAVA_FILES2 "${TEST_JAVA_FILES}" )
string( REPLACE "/" "." BASENAMES_IDIOMATIC_EXAMPLES "${TEST_JAVA_FILES2}" )
set( TARGETS_IDIOMATIC_EXAMPLES ${BASENAMES_IDIOMATIC_EXAMPLES} )

foreach(name ${TARGETS_IDIOMATIC_EXAMPLES})
    add_test (NAME ${name} COMMAND ${JAVA_RUNTIME} 
                           -cp ${JUNIT_JAR}:${jaulib_fat_jar_file}:${jaulib_test_jar_file}
                           org.junit.runner.JUnitCore ${name})
endforeach()

