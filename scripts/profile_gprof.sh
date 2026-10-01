#!/bin/sh

sdir=`dirname $(readlink -f $0)`
rootdir=`dirname $sdir`
bname=`basename $0 .sh`

username=$USER

testname=test_stringfmt_profiling_int
testargs="--test 13 --loops 10000000"

testexe=`readlink -f build/perf-gcc/test/${testname}`

mkdir -p profiling

$sdir/rebuild-preset.sh perf-gcc && \
echo "Profiling ${testexe} ${testargs} in sub-dir profiling" && \
cd profiling && \
/usr/bin/gp-collect-app -o ./${testname}.1.er -a on -p high -S on ${testexe} ${testargs}
echo "Do: GDK_SCALE=2 gprofng display gui"
