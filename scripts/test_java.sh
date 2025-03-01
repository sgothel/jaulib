#!/bin/bash

# export jau_debug=true
# export jau_verbose=true

#
# JAVA_PROPS="-Djau.debug=true -Djau.verbose=true"
#

script_args="$@"
sdir=`dirname $(readlink -f $0)`
rootdir=`dirname $sdir`
bname=`basename $0 .sh`

. $sdir/setup-machine-arch.sh "-quiet"

#dist_dir=$rootdir/"dist-$os_name-$archabi-gcc"
#build_dir=$rootdir/"build-$os_name-$archabi-gcc"
if [ -z "$1" -o ! -e "$2" ] ; then
    echo "ERROR: Using $0 <dist_dir> <build_dir>"
    exit 1
fi
dist_dir=$(readlink -f "$1")
shift
build_dir=$(readlink -f "$1")
shift
echo dist_dir $dist_dir
echo build_dir $build_dir

if [ ! -e $dist_dir/lib/java/jaulib-test.jar ] ; then
    echo "test exe $dist_dir/lib/java/jaulib-test.jar not existing"
    exit 1
fi

if [ -z "$JAVA_HOME" -o ! -e "$JAVA_HOME" ] ; then
    echo "ERROR: JAVA_HOME $JAVA_HOME does not exist"
    exit 1
else
    echo JAVA_HOME $JAVA_HOME
fi
if [ -z "$JUNIT_CP" ] ; then
    echo "ERROR: JUNIT_CP $JUNIT_CP does not exist"
    exit 1
else
    echo JUNIT_CP $JUNIT_CP
fi
JAVA_EXE=${JAVA_HOME}/bin/java
# JAVA_EXE=`readlink -f $(which java)`
# JAVA_CMD="${JAVA_EXE} -Xcheck:jni -verbose:jni"
JAVA_CMD="${JAVA_EXE}"

if [ "$1" = "-log" ] ; then
    logfile=$2
    shift 2
else
    logfile=
fi

test_class=jau.test.io.TestByteStream01
if [ ! -z "$1" ] ; then
    test_class=$1
    shift 1
fi
test_basename=`echo ${test_class} | sed 's/.*\.//g'`

if [ -z "${logfile}" ] ; then
    mkdir -p $rootdir/doc/test
    logfile=$rootdir/doc/test/${bname}-${test_basename}-${os_name}-${archabi}.log
fi
rm -f $logfile
logbasename=`basename ${logfile} .log`

ulimit -c unlimited

# run as root 'dpkg-reconfigure locales' enable 'en_US.UTF-8'
# perhaps run as root 'update-locale LC_MEASUREMENT=en_US.UTF-8 LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8'
export LC_MEASUREMENT=en_US.UTF-8
export LC_ALL=en_US.UTF-8
export LANG=en_US.UTF-8

# export JAVA_PROPS="-Xint"
# export EXE_WRAPPER="nice -20"
# export JAVA_PROPS="-Djau.debug=true -Djau.verbose=true"

test_classpath=$JUNIT_CP:${dist_dir}/lib/java/jaulib.jar:${build_dir}/test/java/jaulib-test.jar
#test_classpath=$JUNIT_CP:${dist_dir}/lib/java/jaulib-fat.jar:${build_dir}/test/java/jaulib-test.jar

do_test() {
    echo "script invocation: $0 ${script_args}"
    echo EXE_WRAPPER $EXE_WRAPPER
    echo jau_debug $jau_debug
    echo jau_verbose $jau_verbose
    echo logbasename $logbasename
    echo logfile $logfile
    echo test_class ${test_class}

    test_dir="${build_dir}/test/java/"
    echo "cd ${test_dir}"
    cd ${test_dir}
    pwd

    echo "$EXE_WRAPPER ${JAVA_CMD} ${JAVA_PROPS} -cp ${test_classpath} -Djava.library.path=${dist_dir}/lib org.junit.runner.JUnitCore ${test_class} ${*@Q}"

    ulimit -c unlimited
    $EXE_WRAPPER ${JAVA_CMD} ${JAVA_PROPS} -cp ${test_classpath} -Djava.library.path=${dist_dir}/lib org.junit.runner.JUnitCore ${test_class} ${*@Q}
    # $EXE_WRAPPER ${JAVA_CMD} ${JAVA_PROPS} -cp ${test_classpath} org.junit.runner.JUnitCore ${test_class} ${*@Q}
    exit $?
}

do_test "$@" 2>&1 | tee $logfile

