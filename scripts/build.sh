#! /bin/sh

sdir=`dirname $(readlink -f $0)`
rootdir=`dirname $sdir`
bname=`basename $0 .sh`

. $sdir/setup-machine-arch.sh

logfile=$rootdir/$bname-$os_name-$archabi.log
rm -f $logfile

CPU_COUNT=`getconf _NPROCESSORS_ONLN`

# run as root 'dpkg-reconfigure locales' enable 'en_US.UTF-8'
# perhaps run as root 'update-locale LC_MEASUREMENT=en_US.UTF-8 LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8'
export LC_MEASUREMENT=en_US.UTF-8
export LC_ALL=en_US.UTF-8
export LANG=en_US.UTF-8

if ! type time &> /dev/null ; then 
    time_cmd=""
else 
    time_cmd="time"
fi

buildit() {
    if [ -z "$JAVA_HOME" -o ! -e "$JAVA_HOME" ] ; then
        echo "WARNING: JAVA_HOME $JAVA_HOME does not exist"
    else
        echo JAVA_HOME $JAVA_HOME
    fi
    echo rootdir $rootdir
    echo logfile $logfile
    echo CPU_COUNT $CPU_COUNT

    dist_dir="dist-$os_name-$archabi"
    build_dir="build-$os_name-$archabi"
    echo dist_dir $dist_dir
    echo build_dir $build_dir

    cd $rootdir
    rm -rf $dist_dir
    mkdir -p $dist_dir
    rm -rf $build_dir
    mkdir -p $build_dir
    cd $build_dir
    # CLANG_ARGS="-DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++"
    # CLANG_ARGS="-DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++ -DCMAKE_CXX_CLANG_TIDY=/usr/bin/clang-tidy;-p;$rootdir/$build_dir"

    # cmake $CLANG_ARGS -DCMAKE_INSTALL_PREFIX=$rootdir/$dist_dir -DBUILDJAVA=ON -DBUILD_TESTING=ON -DUSE_LIBUNWIND=ON ..
    # cmake $CLANG_ARGS -DCMAKE_INSTALL_PREFIX=$rootdir/$dist_dir -DBUILDJAVA=ON -DBUILD_TESTING=ON -DUSE_LIBUNWIND=ON -DUSE_LIBCURL=ON ..

    cmake $CLANG_ARGS -DCMAKE_INSTALL_PREFIX=$rootdir/$dist_dir -DBUILDJAVA=ON -DBUILD_TESTING=ON -DUSE_LIBUNWIND=ON -DUSE_LIBCURL=ON -DTEST_WITH_SUDO=ON ..
    # cmake $CLANG_ARGS -DCMAKE_INSTALL_PREFIX=$rootdir/$dist_dir -DBUILDJAVA=ON -DBUILD_TESTING=ON -DUSE_LIBUNWIND=ON -DUSE_LIBCURL=ON -DTEST_WITH_SUDO=ON -DDONT_USE_RTTI=ON ..
    # cmake $CLANG_ARGS -DCMAKE_INSTALL_PREFIX=$rootdir/$dist_dir -DBUILDJAVA=ON -DBUILD_TESTING=ON -DUSE_LIBUNWIND=ON -DUSE_LIBCURL=ON -DTEST_WITH_SUDO=ON -DDEBUG=ON ..

    # cmake $CLANG_ARGS -DCMAKE_INSTALL_PREFIX=$rootdir/$dist_dir -DBUILDJAVA=ON -DBUILD_TESTING=ON ..
    # cmake $CLANG_ARGS -DCMAKE_INSTALL_PREFIX=$rootdir/$dist_dir -DBUILDJAVA=ON -DBUILD_TESTING=ON -DDEBUG=ON ..

    # cmake $CLANG_ARGS -DCMAKE_INSTALL_PREFIX=$rootdir/$dist_dir -DBUILDJAVA=ON -DBUILD_TESTING=ON -DUSE_STRIP=OFF ..
    # cmake $CLANG_ARGS -DCMAKE_INSTALL_PREFIX=$rootdir/$dist_dir -DBUILDJAVA=ON -DBUILD_TESTING=ON -DUSE_STRIP=ON -DJAVAC_DEBUG_ARGS="none" ..
    # cmake $CLANG_ARGS -DCMAKE_INSTALL_PREFIX=$rootdir/$dist_dir -DBUILDJAVA=ON -DBUILD_TESTING=ON -DGPROF=ON ..
    # cmake $CLANG_ARGS -DCMAKE_INSTALL_PREFIX=$rootdir/$dist_dir -DBUILDJAVA=ON -DBUILD_TESTING=ON -DPERF_ANALYSIS=ON ..
    # cmake $CLANG_ARGS -DCMAKE_INSTALL_PREFIX=$rootdir/$dist_dir -DBUILDJAVA=ON -DBUILD_TESTING=ON -DDEBUG=ON -DINSTRUMENTATION=ON ..
    # cmake $CLANG_ARGS -DCMAKE_INSTALL_PREFIX=$rootdir/$dist_dir -DBUILDJAVA=ON -DBUILD_TESTING=ON -DDEBUG=ON -DINSTRUMENTATION_UNDEFINED=ON ..
    # cmake $CLANG_ARGS -DCMAKE_INSTALL_PREFIX=$rootdir/$dist_dir -DBUILDJAVA=ON -DBUILD_TESTING=ON -DDEBUG=ON -DINSTRUMENTATION_THREAD=ON ..
    ${time_cmd} make -j $CPU_COUNT install
    if [ $? -eq 0 ] ; then
        echo "BUILD SUCCESS $bname $os_name $archabi"
        ${time_cmd} make test
        if [ $? -eq 0 ] ; then
            echo "TEST SUCCESS $bname $os_name $archabi"
            cd $rootdir
            return 0
        else
            echo "TEST FAILURE $bname $os_name $archabi"
            cd $rootdir
            return 1
        fi
    else
        echo "BUILD FAILURE $bname $os_name $archabi"
        cd $rootdir
        return 1
    fi
}

buildit 2>&1 | tee $logfile
