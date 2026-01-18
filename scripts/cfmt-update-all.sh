#!/bin/sh

sdir=`dirname $(readlink -f $0)`
rootdir=`dirname $sdir`

${sdir}/rebuild-preset.sh perf-clang && \
${sdir}/rebuild-preset.sh perf-gcc && \

#${sdir}/rebuild-preset.sh release-clang && \
#${sdir}/rebuild-preset.sh release-gcc

if [ $? -ne 0 ] ; then
    return 1
fi

nm_all() {
    builddir=$1
    shift
    tag=$1
    shift
    ${sdir}/cfmt-nm_one.sh ${builddir}/src/libjaulib.so.1.4.1 > symbols/symbols-lib-${tag}.txt
    ${sdir}/cfmt-nm_one.sh ${builddir}/test/test_stringfmt_format > symbols/symbols-test_format-${tag}.txt
    ${sdir}/cfmt-nm_one.sh ${builddir}/test/test_stringfmt_perf > symbols/symbols-perf-${tag}.txt
    ${sdir}/cfmt-nm_one.sh ${builddir}/test/test_stringfmt_perf0 > symbols/symbols-perf0-${tag}.txt
}

nm_all build/perf-gcc gcc
nm_all build/perf-clang clang

${sdir}/cfmt-footprint.sh build/perf-gcc
${sdir}/cfmt-footprint.sh build/perf-clang

#${sdir}/cfmt-footprint.sh build/release-gcc
#${sdir}/cfmt-footprint.sh build/release-clang
