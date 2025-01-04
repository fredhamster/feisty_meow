#!/usr/bin/env bash

# branchy: lists the branches active on all of the paths provided on the command line.
# if no directory is specified, this defaults to operating on the current directory.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"
source "$FEISTY_MEOW_SCRIPTS/rev_control/version_control.sh"

save_terminal_title

##############

## holding old implem:
#FULL_LIST="$@"
## use the current directory if no paths were provided.
#if [ -z "$FULL_LIST" ]; then
#  FULL_LIST="."
#fi#
#show_active_branch $FULL_LIST
#exit_on_error "showing branches: $FULL_LIST"

dir="$1"; shift
if [ -z "$dir" ]; then
  dir=.
fi

pushd "$dir" &>/dev/null
exit_on_error "changing to directory: $dir"
tempfile="$(generate_rev_ctrl_filelist)"
popd &>/dev/null

perform_revctrl_action_on_file "$tempfile" show_active_branch
exit_on_error "performing revision control action show_active_branch on: $tempfile"

rm "$tempfile"

##############

restore_terminal_title

