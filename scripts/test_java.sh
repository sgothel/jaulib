#!/bin/sh

# Arguments:
#   class-name        unit test class name
#

sdir=`dirname $(readlink -f $0)`
rootdir=`dirname $sdir`
bname=`basename $0 .sh`

. $sdir/setup-machine-arch.sh

dist_dir=$rootdir/dist-$archabi

if [ ! -e $dist_dir/lib/java/jaulib-test.jar ] ; then
    echo "test exe $dist_dir/lib/java/jaulib-test.jar not existing"
    exit 1
fi

if [ ! -z "$1" ] ; then
    logbasename=$1
else
    logbasename=$bname.$archabi
fi

logfile=$rootdir/doc/test/$logbasename.0.log
rm -f $logfile

ulimit -c unlimited

# run as root 'dpkg-reconfigure locales' enable 'en_US.UTF-8'
# perhaps run as root 'update-locale LC_MEASUREMENT=en_US.UTF-8 LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8'
export LC_MEASUREMENT=en_US.UTF-8
export LC_ALL=en_US.UTF-8
export LANG=en_US.UTF-8

# export EXE_WRAPPER="nice -20"

runit() {
    echo COMMANDLINE $0 $*
    echo EXE_WRAPPER $EXE_WRAPPER
    echo logbasename $logbasename
    echo logfile $logfile

    echo $EXE_WRAPPER java -cp /usr/share/java/junit4.jar:$dist_dir/lib/java/jaulib-test.jar $*

    $EXE_WRAPPER java -cp /usr/share/java/junit4.jar:$dist_dir/lib/java/jaulib-test.jar $*
}

runit $* 2>&1 | tee $logfile

