# java/CMakeLists.txt

find_jar(JUNIT_JAR
  NAMES junit4 junit
  PATHS "/usr/share/java")

set(jaulib_test_jar_file ${CMAKE_CURRENT_BINARY_DIR}/jaulib-test.jar CACHE FILEPATH "jaulib test jar file" FORCE)

file(GLOB_RECURSE TEST_JAVA_SOURCES "*.java")

file(GLOB_RECURSE TEST_JAVA_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "Test*.java")

add_jar(jaulib_test
        ${TEST_JAVA_SOURCES}
        jau/info.txt
        org/jau/net/data2/RelativeData2.txt
        org/jau/net/data/AssetURLConnectionTest.txt
        org/jau/net/data/RelativeData.txt
        jau-test/info.txt
        INCLUDE_JARS ${jaulib_fat_jar_file} ${JUNIT_JAR}
        MANIFEST ${CMAKE_CURRENT_BINARY_DIR}/manifest.txt
        OUTPUT_NAME jaulib-test
)
add_dependencies(jaulib_test jaulib_fat_jar)

install (FILES ${jaulib_test_jar_file} DESTINATION ${CMAKE_INSTALL_LIBDIR}/../lib/java)

string( REPLACE ".java" "" TEST_JAVA_FILES2 "${TEST_JAVA_FILES}" )
string( REPLACE "/" "." BASENAMES_IDIOMATIC_EXAMPLES "${TEST_JAVA_FILES2}" )
set( TARGETS_IDIOMATIC_EXAMPLES ${BASENAMES_IDIOMATIC_EXAMPLES} )

set(ALL_EXAMPLE_TARGETS
  ${TARGETS_IDIOMATIC_EXAMPLES}
)

foreach(name ${ALL_EXAMPLE_TARGETS})
    add_test (NAME ${name} COMMAND ${JAVA_RUNTIME} 
                           -cp ${JUNIT_JAR}:${jaulib_fat_jar_file}:${jaulib_test_jar_file}
                           org.junit.runner.JUnitCore ${name})
endforeach()



