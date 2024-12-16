#!/usr/bin/env bash
#
# a simple time update script that traps output from rdate, since we
# get a lot of noise even when a successful update occurs, to whit:
#  "rdate: Could not read data: Success"
#
# author: chris koeritz

#hmmm: the generalized pattern of show output only on error is embodied below.  make it a function.

outfile="$(mktemp /tmp/update_system_time.XXXXXX)"
/usr/bin/rdate -s time.nist.gov &> "$outfile"
retval=$?
if [ $retval -ne 0 ]; then
  echo "Actual error code $retval returned by rdate, with output:"
  cat "$outfile" 
fi
rm -f "$outfile"

