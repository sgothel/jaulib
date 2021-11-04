#! /bin/sh

sdir=`dirname $(readlink -f $0)`
rootdir=`dirname $sdir`
bname=`basename $0 .sh`
logfile=$bname.log
rm -f $logfile

. $sdir/setup-machine-arch.sh

export JAVA_HOME=/usr/lib/jvm/java-11-openjdk-$archabi
if [ ! -e $JAVA_HOME ] ; then
    echo $JAVA_HOME does not exist
    exit 1
fi

CPU_COUNT=`getconf _NPROCESSORS_ONLN`

# run as root 'dpkg-reconfigure locales' enable 'en_US.UTF-8'
# perhaps run as root 'update-locale LC_MEASUREMENT=en_US.UTF-8 LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8'
export LC_MEASUREMENT=en_US.UTF-8
export LC_ALL=en_US.UTF-8
export LANG=en_US.UTF-8

buildit() {
    echo rootdir $rootdir
    echo logfile $logfile
    echo CPU_COUNT $CPU_COUNT

    cd $rootdir
    rm -rf dist-$archabi
    mkdir -p dist-$archabi/bin
    rm -rf build-$archabi
    mkdir -p build-$archabi
    cd build-$archabi
    # CLANG_ARGS="-DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++"
    cmake $CLANG_ARGS -DCMAKE_INSTALL_PREFIX=$rootdir/dist-$archabi -DBUILDJAVA=ON -DBUILD_TESTING=ON ..
    make -j $CPU_COUNT install test doc_jau
    if [ $? -eq 0 ] ; then
        echo "BUILD SUCCESS $bname $archabi"
        tar caf $rootdir/documentation.tar.xz documentation
        cd $rootdir
        return 0
    else
        echo "BUILD FAILURE $bname $archabi"
        cd $rootdir
        return 1
    fi
}

buildit 2>&1 | tee $logfile
