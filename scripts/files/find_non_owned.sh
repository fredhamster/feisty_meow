#!/usr/bin/env bash
if test $# = 0; then
  echo $(basename $0): needs at least one directory name as a parameter.
#hmmm: set the first parm to . instead.
  \exit 1;
fi;
export outfile="$(mktemp "$TMP/zz_findertmp.XXXXXX")"
# check for files not owned by the user.
echo "These files are not self-owned by $USER:" >$outfile
for i; do
  find $i ! -user $USER >>$outfile
done
# check for files not in same group as the user.
GROUP="$(groups | awk '{print $1}')"
  # assumption above that the first group is the 'primary' one.
echo "These files are not owned by primary group of $GROUP:" >>$outfile
for i; do
  find $i ! -group $GROUP >>$outfile
done

less $outfile

rm $outfile

