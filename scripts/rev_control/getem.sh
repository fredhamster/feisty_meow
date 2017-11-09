#!/bin/bash

# gets any updates for the repository folders present in the REPOSITORY_LIST variable.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"
source "$FEISTY_MEOW_SCRIPTS/rev_control/version_control.sh"

##############

# trickery to ensure we can always update this file, even when the operating system has some
# rude behavior with regard to file locking (ahem, windows...).
# and even more rudeness is that the pwd and $TMP may not always be in the same form,
# which causes endless confusion and badness.  that's why we get the pwd reading for TMP
# first so we can do an orange-to-orange compare.
tmpdir="$(cd $TMP; \pwd)"
if [ "$(\pwd)" != "$tmpdir" ]; then
  if [ ! -z "$DEBUG_FEISTY_MEOW" ]; then
    echo "moving to the TMP directory to avoid file access conflicts..."
  fi
  new_name="$TMP/zz_$(basename $0)"
  \cp -f "$0" "$new_name"
  test_or_die "failed to copy this script up to the TMP directory.  exploit attempted?"
  cd "$TMP"
  chmod a+x "$new_name"
  test_or_die "chmodding of file: $new_name"
  exec "$new_name"
  test_or_die "execing cloned getemscript"
fi

##############

export TMPO_CHK=$TMP/zz_chk.log

rm -f "$TMPO_CHK"
test_or_die "removing file: $TMPO_CHK"

echo "getting repositories at: $(date)"
echo

# perform the checkouts as appropriate per OS.
FULL_LIST="$(dirname $FEISTY_MEOW_APEX) $HOME"
if [ "$OS" == "Windows_NT" ]; then
  FULL_LIST+="c:/ d:/ e:/"
fi
checkout_list $FULL_LIST 2>&1 | tee -a "$TMPO_CHK"
test_or_die "checking out list: $FULL_LIST"

##############

# regenerate the scripts after getting latest version of feisty meow.
regenerate

##############

