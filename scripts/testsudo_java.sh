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

. $sdir/setup-machine-arch.sh

build_dir=${rootdir}/build-${archabi}
dist_dir=$rootdir/dist-$archabi

if [ ! -e $dist_dir/lib/java/jaulib-test.jar ] ; then
    echo "test exe $dist_dir/lib/java/jaulib-test.jar not existing"
    exit 1
fi

if [ -e /usr/lib/jvm/java-17-openjdk-$archabi ] ; then
    export JAVA_HOME=/usr/lib/jvm/java-17-openjdk-$archabi
elif [ -e /usr/lib/jvm/java-11-openjdk-$archabi ] ; then
    export JAVA_HOME=/usr/lib/jvm/java-11-openjdk-$archabi
fi
if [ ! -e $JAVA_HOME ] ; then
    echo $JAVA_HOME does not exist
    exit 1
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

test_class=jau.test.fs.TestsudoFileUtils02
if [ ! -z "$1" ] ; then
    test_class=$1
    shift 1
fi
test_basename=`echo ${test_class} | sed 's/.*\.//g'`

if [ -z "${logfile}" ] ; then
    logfile=~/${bname}-${test_basename}-${archabi}.log
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

test_classpath=/usr/share/java/junit4.jar:${dist_dir}/lib/java/jaulib.jar:${build_dir}/test/java/jaulib-test.jar
#test_classpath=/usr/share/java/junit4.jar:${dist_dir}/lib/java/jaulib-fat.jar:${build_dir}/test/java/jaulib-test.jar

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

    echo "$EXE_WRAPPER ${JAVA_CMD} ${JAVA_PROPS} -cp ${test_classpath} -Djava.library.path=${rootdir}/dist-${archabi}/lib org.junit.runner.JUnitCore ${test_class} ${*@Q}"

    ulimit -c unlimited
    # $EXE_WRAPPER ${JAVA_CMD} ${JAVA_PROPS} -cp ${test_classpath} -Djava.library.path=${rootdir}/dist-${archabi}/lib org.junit.runner.JUnitCore ${test_class} ${*@Q}
    # $EXE_WRAPPER ${JAVA_CMD} ${JAVA_PROPS} -cp ${test_classpath} org.junit.runner.JUnitCore ${test_class} ${*@Q}

    "/usr/bin/sudo" -E "/sbin/capsh" "--caps=cap_sys_admin,cap_setuid,cap_setgid+eip cap_setpcap+ep" "--keep=1" "--user=${USER}" "--addamb=cap_sys_admin,cap_setuid,cap_setgid+eip" "--" "-c" "ulimit -c unlimited; $EXE_WRAPPER ${JAVA_CMD} ${JAVA_PROPS} -cp ${test_classpath} -Djava.library.path=${rootdir}/dist-${archabi}/lib org.junit.runner.JUnitCore ${test_class} ${*@Q}"

    exit $?
}

do_test "$@" 2>&1 | tee $logfile

