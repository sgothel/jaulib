#!/bin/sh

sdir=`dirname $(readlink -f $0)`
rootdir=`dirname $sdir`
bname=`basename $0 .sh`

username=$USER

testname=test_stringfmt_profiling_int
#testargs="--test 13 --loops 10000000"
testargs="--test 64 --loops 10000000"
#testargs="--test 88 --loops 5000000"
#testargs="--test 0 --loops 4000000"

testexe=`readlink -f build/perf-gcc/test/${testname}`

mkdir -p profiling

$sdir/rebuild-preset.sh perf-gcc && \
echo "Profiling ${testexe} ${testargs} in sub-dir profiling" && \
cd profiling && \
sudo perf record -P --freq=max --call-graph fp,64 ${testexe} ${testargs}
sudo chown ${username}:${username} perf.data
echo "Do: perf report"
echo "Do: perf report --no-children"
