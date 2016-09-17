#!/bin/bash

# copies a set of custom scripts into the proper location for feisty meow
# to merge their functions and aliases with the standard set.


source "$FEISTY_MEOW_SCRIPTS/core/functions.sh"

#  function recustomize()
#{

user="$1"; shift
if [ -z "$user" ]; then
  # use our default example user if there was no name provided.
  user=fred
fi
if [ ! -d "$FEISTY_MEOW_APEX/customizing/$user" ]; then
  echo "The customization folder provided for $user should be:"
  echo "  '$FEISTY_MEOW_APEX/customizing/$user'"
  echo "but that folder does not exist.  Skipping customization."
  return 1
fi
regenerate >/dev/null
pushd "$FEISTY_MEOW_LOADING_DOCK/custom" &>/dev/null
incongruous_files="$(bash "$FEISTY_MEOW_SCRIPTS/files/list_non_dupes.sh" "$FEISTY_MEOW_APEX/customizing/$user" "$FEISTY_MEOW_LOADING_DOCK/custom")"

#echo "the incongruous files list is: $incongruous_files"
# disallow a single character result, since we get "*" as result when nothing exists yet.
if [ ${#incongruous_files} -ge 2 ]; then
  echo "cleaning unknown older overrides..."
  perl "$FEISTY_MEOW_SCRIPTS/files/safedel.pl" $incongruous_files
  echo
fi
popd &>/dev/null
echo "copying custom overrides for $user"
mkdir -p "$FEISTY_MEOW_LOADING_DOCK/custom" 2>/dev/null
perl "$FEISTY_MEOW_SCRIPTS/text/cpdiff.pl" "$FEISTY_MEOW_APEX/customizing/$user" "$FEISTY_MEOW_LOADING_DOCK/custom"
if [ -d "$FEISTY_MEOW_APEX/customizing/$user/scripts" ]; then
  echo "copying custom scripts for $user"
  \cp -R "$FEISTY_MEOW_APEX/customizing/$user/scripts" "$FEISTY_MEOW_LOADING_DOCK/custom/"
fi
echo
regenerate
#}


