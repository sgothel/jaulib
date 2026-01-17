#!/bin/sh

sdir=`dirname $(readlink -f $0)`
rootdir=`dirname $sdir`

${sdir}/cfmt-nm_one.sh release-gcc/src/libjaulib.so.1.4.1 > symbols-lib10a.txt
${sdir}/cfmt-nm_one.sh release-gcc/test/test_stringfmt_format > symbols-test10a.txt
${sdir}/cfmt-nm_one.sh release-clang/test/test_stringfmt_format > symbols-test10a-clang.txt
${sdir}/cfmt-nm_one.sh release-clang/src/libjaulib.so.1.4.1 > symbols-lib10a-clang.txt
${sdir}/cfmt-footprint.sh release-gcc
${sdir}/cfmt-footprint.sh release-clang/
