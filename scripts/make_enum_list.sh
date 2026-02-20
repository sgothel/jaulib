#!/bin/sh

if [ -z "$1" -o -z "$2" ] ; then
    echo "Usage $0 <prefix> <items-per-line> <enum-snippet-file>"
    exit 1
fi

prefix=$1
shift
itemsperline=$1
shift
infile=$1
shift

echo "// prefix ${prefix}, items-per-line ${itemsperline}, infile ${infile}"

grep ${prefix} ${infile} | awk -v ipl=${itemsperline} -d '{ printf "%s, ", $1; if (++onr%ipl == 0) { print ""; } }'

