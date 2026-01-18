#!/bin/sh

sdir=`dirname $(readlink -f $0)`
rootdir=`dirname $sdir`

${sdir}/cfmt-nm_one.sh build/perf-gcc/src/libjaulib.so.1.4.1 > symbols-lib10a.txt
${sdir}/cfmt-nm_one.sh build/perf-gcc/test/test_stringfmt_format > symbols-test10a.txt
${sdir}/cfmt-nm_one.sh build/perf-clang/test/test_stringfmt_format > symbols-test10a-clang.txt
${sdir}/cfmt-nm_one.sh build/perf-clang/src/libjaulib.so.1.4.1 > symbols-lib10a-clang.txt
${sdir}/cfmt-footprint.sh build/perf-gcc
${sdir}/cfmt-footprint.sh build/perf-clang
${sdir}/cfmt-footprint.sh build/release-gcc
${sdir}/cfmt-footprint.sh build/release-clang
