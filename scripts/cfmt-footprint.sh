#!/bin/sh

builddir=$1

nm_opts="-C --print-size --size-sort --radix=d"

check() {
    builddir=$1
    echo "Symbols ${builddir}: libjaulib"
    nm ${nm_opts} ${builddir}/src/libjaulib.so.1.4.1 | grep "jau::cfmt" | wc

    echo Symbols "${builddir}: test_stringfmt_format"
    nm ${nm_opts} ${builddir}/test/test_stringfmt_format | grep "jau::cfmt" | wc
    echo
    echo "--------------------------------------------------"
    echo

    echo "Size ${builddir}: Kilobytes libjaulib"
    du -hsk --apparent-size ${builddir}/test/test_stringfmt_format

    echo "Size ${builddir}: Kilobytes test_stringfmt_format"
    du -hsk --apparent-size ${builddir}/src/libjaulib.so.1.4.1
    echo
    echo "--------------------------------------------------"
    echo
}

if [ -z "${builddir}" ] ; then
    check release-gcc-master-00
    check release-gcc
else
    check ${builddir}
fi
