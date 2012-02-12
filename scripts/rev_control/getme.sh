#!/bin/bash

# gets any updates for the repository folders present in the REPOSITORY_LIST variable.

source "$FEISTY_MEOW_SCRIPTS/rev_control/rev_control.sh"

# trickery to ensure we can always update this file, even when the operating system has some
# rude behavior with regard to file locking (ahem, windows...).
if [ "$(pwd)" != "$TMP" ]; then
  if [ ! -z "$SHELL_DEBUG" ]; then
    echo "Moving to the TMP directory to avoid file access conflicts..."
  fi
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

# selects the checkout method based on where we are (the host the script runs on).
function do_update()
{
  directory="$1"; shift

  if [ -d "CVS" ]; then
    cvs update .
  elif [ -d ".svn" ]; then
    svn update .
  elif [ -d ".git" ]; then
    git pull
  else
    echo unknown repository for $directory...
  fi
}

# gets all the updates for a list of folders under revision control.
function checkout_list {
  list=$*
  for i in $list; do
    # turn repo list back into an array.
    eval "repository_list=( ${REPOSITORY_LIST[*]} )"
    for j in "${repository_list[@]}"; do
      # add in the directory for our purposes here.
      j="$i/$j"
      if [ ! -d $j ]; then
        if [ ! -z "$SHELL_DEBUG" ]; then
          echo "No directory called $j exists."
        fi
        continue
      fi

      pushd $j &>/dev/null
      echo -n "retrieving '$j'...  "
      do_update $j
      popd &>/dev/null
    done
  done
}

##############

export TMPO_CHK=$TMP/zz_chk.log

rm -f "$TMPO_CHK"

# perform the checkouts as appropriate per OS.
if [ "$OS" != "Windows_NT" ]; then
  checkout_list $HOME 2>&1 | tee -a "$TMPO_CHK"
else
  checkout_list c:/ c:/home d:/ d:/home e:/ e:/home f:/ f:/home g:/ g:/home h:/ h:/home i:/ i:/home 2>&1 | tee -a "$TMPO_CHK"
fi

less $TMPO_CHK

##############

# we now regenerate the scripts after getme, to ensure it's done automatically.
bash "$FEISTY_MEOW_SCRIPTS/core/bootstrap_shells.sh"
perl "$FEISTY_MEOW_SCRIPTS/core/generate_aliases.pl"
echo
nechung

##############

