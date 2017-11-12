#!/bin/bash
#
# reconfigure_feisty_meow:
#
# This script creates the directory for auto-generated scripts and gets
# the current user's account ready to use the feisty meow scripts.
#
# Note: this does not yet ensure that the profile is executed on shell
# startup.  that can be added manually by editing your .bashrc file.
# read the files infobase/feisty_inits for more details.

ORIGINATING_FOLDER="$( \cd "$(\dirname "$0")" && /bin/pwd )"
CORE_SCRIPTS_DIR="$(echo "$ORIGINATING_FOLDER" | tr '\\\\' '/' )"
THIS_TOOL_NAME="$(basename "$0")"

# set up the feisty_meow dir.
pushd "$CORE_SCRIPTS_DIR/../.." &>/dev/null
source "$CORE_SCRIPTS_DIR/functions.sh"

#echo originating folder is $ORIGINATING_FOLDER
export FEISTY_MEOW_APEX="$(/bin/pwd)"
#echo feisty now is FEISTY_MEOW_APEX=$FEISTY_MEOW_APEX

# make the variables script run again.
unset CORE_VARIABLES_LOADED

# repetitive bit stolen from variables.  should make a file out of this somehow.
IS_DOS=$(uname | grep -i ming)
if [ -z "$IS_DOS" ]; then IS_DOS=$(uname | grep -i cygwin); fi
# now if we're stuck in DOS, then fix the feisty meow variable name.
if [ ! -z "$IS_DOS" ]; then
  FEISTY_MEOW_APEX="$(cmd /c chdir | tr A-Z a-z | sed -e 's/\\/\//g')"
echo feisty meow dos is: $FEISTY_MEOW_APEX
  FEISTY_MEOW_APEX="$(dos_to_unix_path "$FEISTY_MEOW_APEX")"
echo new feisty meow fixed dir is: $FEISTY_MEOW_APEX
fi

popd &>/dev/null

export FEISTY_MEOW_SCRIPTS="$FEISTY_MEOW_APEX/scripts"

# FEISTY_MEOW_LOADING_DOCK is where the generated files feisty_meow uses are located.
export FEISTY_MEOW_LOADING_DOCK="$HOME/.zz_feisty_loading"
if [ ! -d "$FEISTY_MEOW_LOADING_DOCK" ]; then
  mkdir -p "$FEISTY_MEOW_LOADING_DOCK"
fi
# need to add some paths explicitly until we've bootstrapped ourselves.
#hmmm: this is tasty reusable code...
export PERLLIB=$(echo $PERLLIB | sed -e "s?\([:]*\)\([^:]*feisty_meow[^:]*\)\([:]*\)??g")

PERLLIB=$PERLLIB:"$FEISTY_MEOW_SCRIPTS/core":"$FEISTY_MEOW_SCRIPTS/files":"$FEISTY_MEOW_SCRIPTS/generator"
PERL5LIB=$PERLLIB
#echo PERLLIB after modification is $PERLLIB
# make toast out of generated files right away, but leave any custom scripts.
find "$FEISTY_MEOW_LOADING_DOCK" -maxdepth 1 -type f -exec perl "$FEISTY_MEOW_SCRIPTS/files/safedel.pl" "{}" ';'
# &>/dev/null
if [ ! -d "$FEISTY_MEOW_LOADING_DOCK/custom" ]; then
  mkdir "$FEISTY_MEOW_LOADING_DOCK/custom"
fi

# just a variable we use in here to refer to the generated variables file.
FEISTY_MEOW_VARIABLES_LOADING_FILE="$FEISTY_MEOW_LOADING_DOCK/fmc_variables.sh"
# create the alias file as totally blank.
echo -n >"$FEISTY_MEOW_VARIABLES_LOADING_FILE"
for i in FEISTY_MEOW_APEX FEISTY_MEOW_SCRIPTS FEISTY_MEOW_LOADING_DOCK; do
  echo "export $i=${!i}" >>"$FEISTY_MEOW_VARIABLES_LOADING_FILE"
done

# load our variables so we can run our perl scripts successfully.
source "$FEISTY_MEOW_SCRIPTS/core/variables.sh"

# create our common aliases.  we need to refresh the PERLLIB since the shell
# seems to keep accumulating the value in a weird way.
PERLLIB=$PERLLIB PERL5LIB=$PERLLIB perl "$FEISTY_MEOW_SCRIPTS/core/generate_aliases.pl"
#echo after the generate aliases, perllib is $PERLLIB

if [ ! -z "$DEBUG_FEISTY_MEOW" ]; then
  echo this is the variable config file for feisty_meow assets:
  echo ==============
  cat "$FEISTY_MEOW_VARIABLES_LOADING_FILE"
  echo ==============
fi

