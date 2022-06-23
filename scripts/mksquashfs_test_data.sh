#! /bin/sh

sdir=`dirname $(readlink -f $0)`
rootdir=`dirname $sdir`

cd $rootdir
/usr/bin/mksquashfs test_data test_data.sqfs -comp lzo
