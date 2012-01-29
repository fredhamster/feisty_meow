#!/bin/bash
#
# bootstrap_shells:
#
# This script creates the directory for auto-generated scripts and gets
# the current user's account ready to use YETI.
#
# Note: this does not yet ensure that the YETI profile is executed on
# shell startup.  that can be added manually by editing your .bashrc file.
# read the examples/bashrc_user file for more details.

ORIGINATING_FOLDER="$( \cd "$(\dirname "$0")" && \pwd )"
YETI_CORE_SCRIPTS_DIR="$(echo "$ORIGINATING_FOLDER" | tr '\\\\' '/' )"
THIS_TOOL_NAME="$(basename "$0")"

## check this scripts directory; do we find this script there?
#pushd "/" &>/dev/null  # jump to root so relative paths have to fail.
#if [ ! -f "$YETI_CORE_SCRIPTS_DIR/$THIS_TOOL_NAME" ]; then
#  echo "This script must be run using its full pathname.  This enables the script to"
#  echo "locate the proper folders.  Please try again with the full path, e.g.:"
#  echo "    bash /home/fred/codeplex/$THIS_TOOL_NAME"
#  exit 1
#fi
#popd &>/dev/null

# set up the feisty_meow dir.
pushd "$YETI_CORE_SCRIPTS_DIR/../.." &>/dev/null
export FEISTY_MEOW_DIR="$(pwd)"
popd &>/dev/null
#echo feisty_meow is $FEISTY_MEOW_DIR

export FEISTY_MEOW_SCRIPTS="$FEISTY_MEOW_DIR/scripts"

# FEISTY_MEOW_GENERATED is where the generated files feisty_meow uses are located.
export FEISTY_MEOW_GENERATED="$HOME/.zz_auto_gen"
if [ ! -z "$WINDIR" -o ! -z "$windir" ]; then
  # assume they are using windoze.
  export FEISTY_MEOW_GENERATED="$TMP/zz_auto_gen"
fi
if [ ! -d "$FEISTY_MEOW_GENERATED" ]; then
  mkdir "$FEISTY_MEOW_GENERATED"
fi

GENERATED_FEISTY_MEOW_VARIABLES="$FEISTY_MEOW_GENERATED/feisty_meow_variables.sh"
echo >"$GENERATED_FEISTY_MEOW_VARIABLES"
for i in FEISTY_MEOW_DIR FEISTY_MEOW_SCRIPTS FEISTY_MEOW_GENERATED; do
  echo "export $i=${!i}" >>"$GENERATED_FEISTY_MEOW_VARIABLES"
done

# create our common aliases.
perl "$FEISTY_MEOW_SCRIPTS/core/generate_aliases.pl"

echo ==========================================
echo Established this set of variables to describe how to find feisty_meow assets:
cat "$GENERATED_FEISTY_MEOW_VARIABLES"
echo ==========================================

