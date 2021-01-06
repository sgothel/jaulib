#!/bin/sh

# Arguments:
#   --perf_analysis   special performance analysis using 3rd party tools
#   -v normal         dummy for full benchmarking
#   <none>            auto_run, no benchmarking
#

sdir=`dirname $(readlink -f $0)`
rootdir=`dirname $sdir`
bname=`basename $0 .sh`

. $sdir/setup-machine-arch.sh

build_dir=$rootdir/build-$archabi

if [ ! -e $build_dir/test/$bname ] ; then
    echo "test exe $build_dir/test/$bname not existing"
    exit 1
fi

if [ "$1" = "-log" ] ; then
    logbasename=$2
    shift 2
else
    logbasename=$bname.$archabi
fi

logfile=$rootdir/doc/test/$logbasename.0.log
rm -f $logfile

valgrindlogfile=$rootdir/doc/test/$logbasename.valgrind.0.log
rm -f $valgrindlogfile

callgrindoutfile=$rootdir/doc/test/$logbasename.callgrind.0.out
rm -f $callgrindoutfile

ulimit -c unlimited

# run 'dpkg-reconfigure locales' enable 'en_US.UTF-8'
export LANG=en_US.UTF-8
export LC_MEASUREMENT=en_US.UTF-8

# export EXE_WRAPPER="valgrind --tool=memcheck --leak-check=full --show-reachable=yes --error-limit=no --default-suppressions=yes --suppressions=$sdir/valgrind.supp --gen-suppressions=all -s --log-file=$valgrindlogfile"
# export EXE_WRAPPER="valgrind --tool=helgrind --track-lockorders=yes  --ignore-thread-creation=yes --default-suppressions=yes --suppressions=$sdir/valgrind.supp --gen-suppressions=all -s --log-file=$valgrindlogfile"
# export EXE_WRAPPER="valgrind --tool=drd --segment-merging=no --ignore-thread-creation=yes --trace-barrier=no --trace-cond=no --trace-fork-join=no --trace-mutex=no --trace-rwlock=no --trace-semaphore=no --default-suppressions=yes --suppressions=$sdir/valgrind.supp --gen-suppressions=all -s --log-file=$valgrindlogfile"
#export EXE_WRAPPER="valgrind --tool=callgrind --instr-atstart=yes --collect-atstart=yes --collect-systime=nsec --combine-dumps=yes --separate-threads=no --callgrind-out-file=$callgrindoutfile --log-file=$valgrindlogfile"
# export EXE_WRAPPER="nice -20 valgrind --tool=callgrind --read-inline-info=yes --instr-atstart=yes --collect-atstart=yes --collect-systime=nsec --combine-dumps=yes --separate-threads=no --callgrind-out-file=$callgrindoutfile --log-file=$valgrindlogfile"
export EXE_WRAPPER="nice -20"

runit() {
    echo COMMANDLINE $0 $*
    echo EXE_WRAPPER $EXE_WRAPPER
    echo logbasename $logbasename
    echo logfile $logfile
    echo valgrindlogfile $valgrindlogfile
    echo callgrindoutfile $callgrindoutfile

    echo $EXE_WRAPPER $build_dir/test/$bname $*

    $EXE_WRAPPER $build_dir/test/$bname $*
}

runit $* 2>&1 | tee $logfile

