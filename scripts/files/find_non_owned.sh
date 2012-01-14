#!/bin/bash
if test $# = 0; then
  echo $(basename $0): needs at least one directory name as a parameter.
  \exit 1;
fi;
export outfile="$(mktemp "$TMP/zz_findertmp.XXXXXX")"
echo "These files are not self-owned:" >$outfile
for i; do
  find $i ! -user $USER >>$outfile
done
cat $outfile
  # apparently this utility is supposed to echo to standard out.
rm $outfile

