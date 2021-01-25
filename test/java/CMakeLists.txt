# java/CMakeLists.txt

find_jar(JUNIT_JAR
  NAMES junit4 junit
  PATHS "/usr/share/java")

file(GLOB_RECURSE TEST_JAVA_SOURCES "*.java")

file(GLOB_RECURSE TEST_JAVA_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "Test*.java")

add_jar(jaulib_test
        ${TEST_JAVA_SOURCES}
        jau/info.txt
        org/jau/net/data2/RelativeData2.txt
        org/jau/net/data/AssetURLConnectionTest.txt
        org/jau/net/data/RelativeData.txt
        jau-test/info.txt
        INCLUDE_JARS jaulib_base_jar jaulib_jni_jar jaulib_net_jar jaulib_pkg_jar ${JUNIT_JAR}
        MANIFEST ${CMAKE_CURRENT_BINARY_DIR}/manifest.txt
        OUTPUT_NAME jaulib_test
)
add_dependencies(jaulib_test jaulib_base_jar jaulib_jni_jar jaulib_net_jar jaulib_pkg_jar)

string( REPLACE ".java" "" TEST_JAVA_FILES2 "${TEST_JAVA_FILES}" )
string( REPLACE "/" "." BASENAMES_IDIOMATIC_EXAMPLES "${TEST_JAVA_FILES2}" )
set( TARGETS_IDIOMATIC_EXAMPLES ${BASENAMES_IDIOMATIC_EXAMPLES} )

set(ALL_EXAMPLE_TARGETS
  ${TARGETS_IDIOMATIC_EXAMPLES}
)

find_jar(jaulib_base_jar_file NAMES jaulib_base PATHS "${CMAKE_CURRENT_BINARY_DIR}/../../java_base")
find_jar(jaulib_jni_jar_file NAMES jaulib_jni PATHS "${CMAKE_CURRENT_BINARY_DIR}/../../java_jni")
find_jar(jaulib_net_jar_file NAMES jaulib_net PATHS "${CMAKE_CURRENT_BINARY_DIR}/../../java_net")
find_jar(jaulib_pkg_jar_file NAMES jaulib_pkg PATHS "${CMAKE_CURRENT_BINARY_DIR}/../../java_pkg")

foreach(name ${ALL_EXAMPLE_TARGETS})
#    add_dependencies(${name} jaulib_test ${JUNIT_JAR})
    add_test (NAME ${name} COMMAND ${JAVA_RUNTIME} 
                           -cp ${JUNIT_JAR}:${jaulib_base_jar_file}:${jaulib_jni_jar_file}:${jaulib_net_jar_file}:${jaulib_pkg_jar_file}:jaulib_test.jar
                           org.junit.runner.JUnitCore ${name})
endforeach()



