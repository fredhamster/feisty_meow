#!/bin/bash
#
# bootstrap_shells:
#
# This script creates the directory for auto-generated scripts and gets
# the current user's account ready to use the feisty meow scripts.
#
# Note: this does not yet ensure that the profile is executed on shell
# startup.  that can be added manually by editing your .bashrc file.
# read the examples/feisty_meow_startup/bashrc_user file for more details.

ORIGINATING_FOLDER="$( \cd "$(\dirname "$0")" && /bin/pwd )"
CORE_SCRIPTS_DIR="$(echo "$ORIGINATING_FOLDER" | tr '\\\\' '/' )"
THIS_TOOL_NAME="$(basename "$0")"

# set up the feisty_meow dir.
pushd "$CORE_SCRIPTS_DIR/../.." &>/dev/null
source "$CORE_SCRIPTS_DIR/functions.sh"

#echo originating folder is $ORIGINATING_FOLDER
export FEISTY_MEOW_DIR="$(/bin/pwd)"
#echo feisty now is FEISTY_MEOW_DIR=$FEISTY_MEOW_DIR

# repetitive bit stolen from variables.  should make a file out of this somehow.
IS_DOS=$(uname | grep -i ming)
if [ -z "$IS_DOS" ]; then IS_DOS=$(uname | grep -i cygwin); fi
# now if we're stuck in DOS, then fix the feisty meow variable name.
if [ ! -z "$IS_DOS" ]; then
  FEISTY_MEOW_DIR="$(cmd /c chdir | tr A-Z a-z | sed -e 's/\\/\//g')"
echo feisty meow dos is: $FEISTY_MEOW_DIR
  FEISTY_MEOW_DIR="$(dos_to_unix_path "$FEISTY_MEOW_DIR")"
echo new feisty meow fixed dir is: $FEISTY_MEOW_DIR
fi

popd &>/dev/null

export FEISTY_MEOW_SCRIPTS="$FEISTY_MEOW_DIR/scripts"

# FEISTY_MEOW_LOADING_DOCK is where the generated files feisty_meow uses are located.
export FEISTY_MEOW_LOADING_DOCK="$HOME/.zz_feisty_loading"
if [ ! -d "$FEISTY_MEOW_LOADING_DOCK" ]; then
  mkdir -p "$FEISTY_MEOW_LOADING_DOCK"
fi
# make toast out of generated files right away, but leave any custom scripts.
find "$FEISTY_MEOW_LOADING_DOCK" -maxdepth 1 -type f -exec rm -f "{}" ';' &>/dev/null
if [ ! -d "$FEISTY_MEOW_LOADING_DOCK/custom" ]; then
  mkdir "$FEISTY_MEOW_LOADING_DOCK/custom"
fi

# just a variable we use in here to refer to the generated variables file.
GENERATED_FEISTY_MEOW_VARIABLES="$FEISTY_MEOW_LOADING_DOCK/fmc_variables.sh"
# create the alias file as totally blank.
echo -n >"$GENERATED_FEISTY_MEOW_VARIABLES"
for i in FEISTY_MEOW_DIR FEISTY_MEOW_SCRIPTS FEISTY_MEOW_LOADING_DOCK; do
  echo "export $i=${!i}" >>"$GENERATED_FEISTY_MEOW_VARIABLES"
done

# load our variables so we can run our perl scripts successfully.
source "$FEISTY_MEOW_SCRIPTS/core/variables.sh"

# create our common aliases.
perl "$FEISTY_MEOW_SCRIPTS/core/generate_aliases.pl"

if [ ! -z "$SHELL_DEBUG" ]; then
  echo established these variables for feisty_meow assets:
  echo ==============
  cat "$GENERATED_FEISTY_MEOW_VARIABLES"
  echo ==============
fi

