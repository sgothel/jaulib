#function(make_native_jar _jarfile _workdir _manifestfile _module _nativelib)
    set(nativejartag.package "jau.nativetag")
    set(nativejartag.prefix "jau/nativetag")

    file(WRITE ${_workdir}/gen/${nativejartag.prefix}/${_module}/${os_and_arch_slash}/TAG.java "package ${nativejartag.package}.${_module}.${os_and_arch_dot}; public final class TAG { }")

    get_filename_component(_nativelib_base ${_nativelib} NAME)
    #file(COPY ${_nativelib} DESTINATION ${_workdir}/natives/${OS_AND_ARCH} FOLLOW_SYMLINK_CHAIN)
    file(COPY ${_nativelib} DESTINATION ${_workdir}/natives/${OS_AND_ARCH})

    execute_process(
        COMMAND ${JAVAC} ${CMAKE_JAVA_COMPILE_FLAGS} 
                         ${_workdir}/gen/${nativejartag.prefix}/${_module}/${os_and_arch_slash}/TAG.java
                         -d ${_workdir}
        COMMAND ${JAR} -v
                       --create --file ${_jarfile}
                       --manifest ${_manifestfile}
                       -C ${_workdir} ${nativejartag.prefix}/${_module}/${os_and_arch_slash}/TAG.class
                       -C ${_workdir} ${_workdir}/natives/${OS_AND_ARCH}/${_nativelib_base}
                   )
#endfunction()
