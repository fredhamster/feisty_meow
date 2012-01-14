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

# set up the yeti dir.
pushd "$YETI_CORE_SCRIPTS_DIR/../.." &>/dev/null
export YETI_DIR="$(pwd)"
popd &>/dev/null
#echo yeti is $YETI_DIR

export YETI_SCRIPTS="$YETI_DIR/scripts"
export SHELLDIR="$YETI_SCRIPTS"

# GENERADIR is where the generated files yeti uses are located.
export GENERADIR="$HOME/.zz_auto_gen"
if [ ! -z "$WINDIR" -o ! -z "$windir" ]; then
  # assume they are using windoze.
  export GENERADIR="$TMP/zz_auto_gen"
fi
if [ ! -d "$GENERADIR" ]; then
  mkdir "$GENERADIR"
fi

YETIVARS="$GENERADIR/yeti_variables.sh"
echo >"$YETIVARS"
for i in YETI_DIR YETI_SCRIPTS SHELLDIR GENERADIR; do
  echo "export $i=${!i}" >>"$YETIVARS"
done

# create our common aliases.
perl "$YETI_SCRIPTS/core/generate_aliases.pl"

echo -e '\n\n'
echo Established this set of variables to describe how to find yeti assets:
cat "$YETIVARS"
echo -e '\n'

