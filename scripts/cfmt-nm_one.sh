#!/bin/sh

objfile=$1
shift

if [ -z "${objfile}" ] ; then
    exit 1;
fi

nm_opts="-C --print-size --size-sort --radix=d"

# builddir=release-gcc
# nm ${nm_opts} ${builddir}/src/libjaulib.so.1.4.1 | grep "jau::cfmt"

nm ${nm_opts} ${objfile} | grep "jau::cfmt"
#nm ${nm_opts} ${objfile} | grep "jau::"

