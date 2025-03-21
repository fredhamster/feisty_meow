#!/usr/bin/env bash

# act_on_tree: performs a command on a hierarchy of directories.
#
# a handy way to run a command across a set of folders.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

#echo command line in act_on_tree is: $*

if [ -z "$APP_NAME" ]; then
  APP_NAME="$(basename $0 .sh)"
fi

function print_instructions_and_exit()
{
  echo "
$APP_NAME [-d directory] [-f subfolder] action1 [action2...]

This script runs an action command on each of the folders that live under the
current directory (going one level down from this directory, not recursively).
The single action command to run is built from the pieces action1, action2,
and so on that are provided on the command line.

For example, this command:
  $APP_NAME git branch
will show the branch information on each project under the current directory.

You can specify an alternate directory to use with the '-d' flag, e.g.:
  $APP_NAME -d ~/turnip_codes/ ant clean build

You can provide a sub-folder name with -f that must exist and which the script
changes the directory to before the command is run.  This supports hierarchies
where the action must take place below the children of the -d directory.
  $APP_NAME -f avenger5 rgetem

The flags and their parameters must precede the action1... arguments.

"
  exit 1
}

changes=true

while [ $changes == true ]; do
  changes=nope

  # check if they gave us a special directory flag.
  if [ "$1" == "-d" ]; then
    shift  # toss the -d.
    # grab the directory they want to actually use.
    seekdir="$1"; shift
    # check for more flags.
    changes=true
  fi

  # check if they gave us a subfolder name flag.
  if [ "$1" == "-f" ]; then
    shift  # toss the -f.
    # get their preferred subfolder.
    subfoldername="$1"; shift
    # check for more flags.
    changes=true
  fi
done

# check that there are some remaining parms for the action pieces.
if [ -z "$*" ]; then
  print_instructions_and_exit
fi

# plug in our defaults.
if [ -z "$seekdir" ]; then
  seekdir="."
fi
if [ -z "$subfoldername" ]; then
  subfoldername="."
fi

sep 28

pushd $seekdir &>/dev/null

for i in *; do
  if [ -d "$i" -a -d "$i/$subfoldername" ]; then
    pushd "$i/$subfoldername" &>/dev/null
    echo "[in '$i' running action: $*]"
    $* 
    sep 28
    popd &>/dev/null
  fi
done

popd &>/dev/null

