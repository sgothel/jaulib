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

grep ${prefix} ${infile} | awk -v prefix=${prefix} -v ipl=${itemsperline} -d 'BEGIN { prelen=length(prefix); } { printf "%s, %s, ", $1, substr($1, prelen+1); if (++onr%ipl == 0) { print ""; } }'

