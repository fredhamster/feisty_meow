#!/bin/bash
outfile="$(mktemp "$TMP/region_ports.XXXXXX")"
grep "InternalPort *= *[0-9][0-9]*" "$HOME/opensim/bin/Regions"/* | sed -e "s/.*= *\([0-9]*\).*/\1/" | sort &>$outfile
#echo all ports are:
#cat $outfile
#echo head
#head -1 $outfile
#echo tail
#tail -1 $outfile
echo "opensim ports range from $(head -1 $outfile) to $(tail -1 $outfile)"
rm $outfile

