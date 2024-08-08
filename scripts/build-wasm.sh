#! /bin/bash

sdir=`dirname $(readlink -f $0)`
rootdir=`dirname $sdir`
bname=`basename $0 .sh`

. $rootdir/scripts/setup-emscripten.sh

logfile=$rootdir/$bname-wasm.log
rm -f $logfile

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

    dist_dir="dist/emscripten-wasm"
    build_dir="build/emscripten-wasm"
    echo dist_dir $dist_dir
    echo build_dir $build_dir

    if [ -x /usr/bin/time ] ; then
        time_cmd="time"
        echo "time command available: ${time_cmd}"
    else 
        time_cmd=""
        echo "time command not available"
    fi

    cd $rootdir
    rm -rf $dist_dir
    mkdir -p $dist_dir
    rm -rf $build_dir
    mkdir -p $build_dir
    cd $build_dir
    # CLANG_ARGS="-DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++"
    # CLANG_ARGS="-DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++ -DCMAKE_CXX_CLANG_TIDY=/usr/bin/clang-tidy;-p;$rootdir/$build_dir"

    # emcmake cmake --trace-expand $CLANG_ARGS -DCMAKE_INSTALL_PREFIX=$rootdir/$dist_dir -DBUILD_TESTING=ON ../..
    # emcmake cmake $CLANG_ARGS -DCMAKE_INSTALL_PREFIX=$rootdir/$dist_dir ../..
    # emcmake cmake $CLANG_ARGS -DCMAKE_INSTALL_PREFIX=$rootdir/$dist_dir -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON ../..
    emcmake cmake $CLANG_ARGS -DCMAKE_INSTALL_PREFIX=$rootdir/$dist_dir -DBUILD_TESTING=ON ../..
    # emcmake cmake $CLANG_ARGS -DCMAKE_INSTALL_PREFIX=$rootdir/$dist_dir -DDEBUG=ON ../..

    # emcmake cmake $CLANG_ARGS -DCMAKE_INSTALL_PREFIX=$rootdir/$dist_dir -DUSE_STRIP=OFF ../..
    # emcmake cmake $CLANG_ARGS -DCMAKE_INSTALL_PREFIX=$rootdir/$dist_dir -DGPROF=ON ../..
    # emcmake cmake $CLANG_ARGS -DCMAKE_INSTALL_PREFIX=$rootdir/$dist_dir -DPERF_ANALYSIS=ON ../..
    # emcmake cmake $CLANG_ARGS -DCMAKE_INSTALL_PREFIX=$rootdir/$dist_dir -DDEBUG=ON -DINSTRUMENTATION=ON ../..
    # emcmake cmake $CLANG_ARGS -DCMAKE_INSTALL_PREFIX=$rootdir/$dist_dir -DDEBUG=ON -DINSTRUMENTATION_UNDEFINED=ON ../..
    # emcmake cmake $CLANG_ARGS -DCMAKE_INSTALL_PREFIX=$rootdir/$dist_dir -DDEBUG=ON -DINSTRUMENTATION_THREAD=ON ..

    ${time_cmd} emmake make -j $CPU_COUNT install
    if [ $? -eq 0 ] ; then
        echo "BUILD SUCCESS $bname wasm"
        cd $rootdir
        return 0
    else
        echo "BUILD FAILURE $bname wasm"
        cd $rootdir
        return 1
    fi
}

buildit 2>&1 | tee $logfile
