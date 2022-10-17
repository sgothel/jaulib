#!/bin/sh

if [ "$1" = "-quiet" ] ; then
    quiet=1
    shift 1
else
    quiet=0
fi

kernel=`uname -s`
os=`uname -o`
machine=`uname -m`

case "$os" in
    "FreeBSD") 
        os_name="freebsd"
    ;;
    "GNU/Linux") 
        os_name="linux"
    ;;
    "Linux") 
        os_name="linux"
    ;;
    *) 
        echo "Unsupported os $os"
        exit 1
    ;;
esac

case "$machine" in
    "arm") 
        cpu="arm"
        cpufamily="arm"
        archabi="armhf"
        case "$os_name" in
            "linux")
                syslibdir="arm-${os_name}-gnueabihf"
            ;;
            *) 
                syslibdir=""
            ;;
        esac
    ;;
    "armv7l")
        cpu="armv7l"
        cpufamily="arm"
        archabi="armhf"
        case "$os_name" in
            "linux")
                syslibdir="arm-${os_name}-gnueabihf"
            ;;
            *) 
                syslibdir=""
            ;;
        esac
    ;;
    "aarch64")
        cpu="aarch64"
        cpufamily="arm"
        archabi="arm64"
        case "$os_name" in
            "linux")
                syslibdir="aarch64-${os_name}-gnu"
            ;;
            *) 
                syslibdir=""
            ;;
        esac
    ;;
    "amd64")
        cpu="x86_64"
        cpufamily="x86"
        archabi="amd64"
        case "$os_name" in
            "linux")
                syslibdir="x86_64-${os_name}-gnu"
            ;;
            *) 
                syslibdir=""
            ;;
        esac
    ;;
    "x86_64")
        cpu="x86_64"
        cpufamily="x86"
        archabi="amd64"
        case "$os_name" in
            "linux")
                syslibdir="x86_64-${os_name}-gnu"
            ;;
            *) 
                syslibdir=""
            ;;
        esac
    ;;
    *) 
        echo "Unsupported machine $machine"
        exit 1
    ;;
esac

if [ $quiet -eq 0 ] ; then
    echo kernel $kernel
    echo os $os
    echo machine $machine
    echo os_name $os_name
    echo cpu $cpu
    echo cpufamily $cpufamily
    echo archabi $archabi
    echo syslibdir $syslibdir
fi

if [ -z "$JAVA_HOME" -o ! -e "$JAVA_HOME" ] ; then
    if [ $quiet -eq 0 ] ; then
        echo "JAVA_HOME: Searching for OpenJDK"
    fi
    if [ -e /usr/lib/jvm/default-jvm-$archabi ] ; then
        export JAVA_HOME=/usr/lib/jvm/default-jvm-$archabi
    elif [ -e /usr/lib/jvm/java-17-openjdk-$archabi ] ; then
        export JAVA_HOME=/usr/lib/jvm/java-17-openjdk-$archabi
    elif [ -e /usr/lib/jvm/java-11-openjdk-$archabi ] ; then
        export JAVA_HOME=/usr/lib/jvm/java-11-openjdk-$archabi
    elif [ -e /usr/lib/jvm/default-jvm ] ; then
        export JAVA_HOME=/usr/lib/jvm/default-jvm
    elif [ -e /usr/lib/jvm/java-17-openjdk ] ; then
        export JAVA_HOME=/usr/lib/jvm/java-17-openjdk
    elif [ -e /usr/lib/jvm/java-11-openjdk ] ; then
        export JAVA_HOME=/usr/lib/jvm/java-11-openjdk
    elif [ -e /usr/local/openjdk17 ] ; then
        export JAVA_HOME=/usr/local/openjdk17
    elif [ -e /usr/local/openjdk11 ] ; then
        export JAVA_HOME=/usr/local/openjdk11
    fi
    if [ $quiet -eq 0 ] ; then
        if [ -z "$JAVA_HOME" -o ! -e "$JAVA_HOME" ] ; then
            echo "JAVA_HOME: Warning: Not Found"
        else
            echo "JAVA_HOME: Found $JAVA_HOME"
        fi
    fi
fi

if [ -z "$JUNIT_CP" -o ! -e "$JUNIT_CP" ] ; then
    if [ $quiet -eq 0 ] ; then
        echo "JUNIT_CP: Searching for junit"
    fi
    for prefix_spath in /usr/share/java /usr/local/share/java/classes ; do
        for junit_jarfile in junit4.jar junit.jar ; do
            if [ -e $prefix_spath/$junit_jarfile ] ; then
                JUNIT_CP=$prefix_spath/$junit_jarfile
                for hamcrest_jarfile in hamcrest-all.jar hamcrest.jar ; do
                    if [ -e $prefix_spath/$hamcrest_jarfile ] ; then
                        JUNIT_CP=$JUNIT_CP:$prefix_spath/$hamcrest_jarfile
                    fi
                done
                break
            fi
        done
    done
    if [ $quiet -eq 0 ] ; then
        if [ -z "$JUNIT_CP" ] ; then
            echo "JUNIT_CP: Warning: Not Found"
        else
            echo "JUNIT_CP: Found $JUNIT_CP"
        fi
    fi
fi


