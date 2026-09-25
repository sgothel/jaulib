#!/bin/sh

sdir=`dirname $(readlink -f $0)`
rootdir=`dirname $sdir`
bname=`basename $0 .sh`

username=$USER

$sdir/rebuild-preset.sh perf-gcc && \
sudo perf record --call-graph fp,64 build/perf-gcc/test/test_stringfmt_profiling_int --test 10 --loops 10000000
sudo chown ${username}:${username} perf.data
echo "Do: perf report"
echo "Do: perf report --no-children"
