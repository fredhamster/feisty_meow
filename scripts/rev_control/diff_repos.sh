#!/usr/bin/env bash

# does differences on a set of folders checked out from subversion or git.
# this can take a directory as parameter, but will default to the current
# working directory.  all the directories under the passed directory will
# be examined.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"
source "$FEISTY_MEOW_SCRIPTS/rev_control/version_control.sh"

save_terminal_title

##############

dir="$1"; shift
if [ -z "$dir" ]; then
  dir=.
fi

pushd "$dir" &>/dev/null
exit_on_error "changing to directory: $dir"
tempfile=$(generate_rev_ctrl_filelist)
popd &>/dev/null

perform_revctrl_action_on_file "$tempfile" do_revctrl_diff
exit_on_error "performing revision control action do_revctrl_diff on: $tempfile"

rm "$tempfile"

##############

restore_terminal_title

