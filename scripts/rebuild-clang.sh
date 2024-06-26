#! /bin/sh

sdir=`dirname $(readlink -f $0)`
rootdir=`dirname $sdir`
bname=`basename $0 .sh`

. $sdir/setup-machine-arch.sh

tripleid="$os_name-$archabi-clang"

logfile=$rootdir/$bname-$tripleid.log
rm -f $logfile

CPU_COUNT=`getconf _NPROCESSORS_ONLN`

# run 'dpkg-reconfigure locales' enable 'en_US.UTF-8'
export LANG=en_US.UTF-8
export LC_MEASUREMENT=en_US.UTF-8

buildit() {
    if [ -z "$JAVA_HOME" -o ! -e "$JAVA_HOME" ] ; then
        echo "WARNING: JAVA_HOME $JAVA_HOME does not exist"
    else
        echo JAVA_HOME $JAVA_HOME
    fi
    echo rootdir $rootdir
    echo logfile $logfile
    echo CPU_COUNT $CPU_COUNT

    dist_dir="dist-$tripleid"
    build_dir="build-$tripleid"
    echo dist_dir $dist_dir
    echo build_dir $build_dir

    if [ -x /usr/bin/time ] ; then
        time_cmd="time"
        echo "time command available: ${time_cmd}"
    else 
        time_cmd=""
        echo "time command not available"
    fi

    cd $rootdir/$build_dir
    ${time_cmd} make -j $CPU_COUNT install
    if [ $? -eq 0 ] ; then
        echo "REBUILD SUCCESS $bname $tripleid"
        cd $rootdir
        return 0
    else
        echo "REBUILD FAILURE $bname $tripleid"
        cd $rootdir
        return 1
    fi
}

buildit 2>&1 | tee $logfile

