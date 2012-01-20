#!/bin/bash

# gets all of fred's revision control folders out.

source "$SHELLDIR/rev_control/rev_control.sh"

if [ "$(pwd)" != "$TMP" ]; then
#  echo "Moving to the TMP directory to avoid file access conflicts..."
  new_name="$TMP/zz_$(basename $0)"
  cp -f "$0" "$new_name"
  if [ $? -ne 0 ]; then
    echo "failed to copy this script up to the TMP directory.  exploit attempted?"
    exit 1
  fi
  cd "$TMP"
  chmod a+x "$new_name"
  exec "$new_name"
fi

export TMPO_CHK=$TMP/zz_chk.log

# selects the checkout method based on where we are (the host the script runs on).
function do_update()
{
  directory="$1"; shift

  if [ -d "CVS" ]; then
    cvs co -P -kb "$directory"
  elif [ -d ".svn" ]; then
    svn update .
  elif [ -d ".git" ]; then
    git pull
  else
    echo unknown repository for $directory...
  fi
}

function checkout_list {
  list=$*
  for i in $list; do
#this list should be part of the configuration file, not part of script.
    for j in $i/feisty_meow $i/hoople $i/hoople1 $i/hoople2 $i/quartz $i/web $i/yeti $i/xsede/xsede_tests $i/xsede/code/cak0l/trunk ; do
      if [ ! -d $j ]; then
#        echo no directory called $j exists
        continue
      fi

      pushd $j &>/dev/null
      echo -n "retrieving '$j'...  "
      do_update $j
      popd &>/dev/null
    done
  done
}

rm -f "$TMPO_CHK"

# perform the checkouts as appropriate per OS.
if [ "$OS" != "Windows_NT" ]; then
  checkout_list $HOME 2>&1 | tee -a "$TMPO_CHK"
else
  checkout_list c:/ c:/home d:/ d:/home e:/ e:/home f:/ f:/home g:/ g:/home h:/ h:/home i:/ i:/home 2>&1 | tee -a "$TMPO_CHK"
fi

less $TMPO_CHK

# we now regenerate the scripts after getme, to ensure it's done automatically.
bash "$SHELLDIR/core/bootstrap_shells.sh"
perl "$SHELLDIR/core/generate_aliases.pl"
echo
nechung

