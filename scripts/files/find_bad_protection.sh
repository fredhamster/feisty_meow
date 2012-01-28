#!/bin/bash

source $FEISTY_MEOW_SCRIPTS/core/date_stringer.sh

bad_file="$HOME/bad_protections.txt"
if [ $# = 0 ]; then dirname=$HOME; export dirname;
else dirname=$1; export dirname; fi

echo "Searching for bad file protections in $dirname..."
echo "This includes any files that are writable by other or that have the"
echo "SetUID or SetGID bits turned on."

echo "Bad file modes and owners report for $(date_stringer)." >$bad_file
echo "" >>$bad_file

export outfile="$(mktemp "$TMP/zz_badprot.XXXXXX")"

echo "These files have bad modes:" >>$bad_file
find "$dirname" -type f -exec ls -AlF {} ';'  >$outfile
cat $outfile | 
  sed -n -e '/^.....w/p
    /^........w/p
    /^..s.../p
    /^.....s/p' |
  grep '^[^l]' >>$bad_file
rm $outfile
echo "" >>$bad_file

echo "These directories have bad modes:" >>$bad_file
find "$dirname" -type d -exec ls -Ald {} ';' >$outfile

#this is same as above block.  make it a function.
cat $outfile | 
  sed -n -e '/^.....w/p
    /^........w/p
    /^..s.../p
    /^.....s/p' |
  grep '^[^l]' >>$bad_file
rm $outfile
#

echo "Searching for Files Not Owned by the User in $HOME...."
echo "" >>$bad_file
bash $FEISTY_MEOW_SCRIPTS/find_non_owned.sh $HOME >>$bad_file

echo "" >>$bad_file
echo "" >>$bad_file

echo $(basename $0) " is finished.  Showing report from $bad_file"

less $bad_file

#optional: rm $bad_file

