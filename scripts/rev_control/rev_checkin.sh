#!/bin/bash
# checks in the updated files in a set of folders checked out from subversion
# or git.  this can take a directory as parameter, but will default to the
# current working directory.  all the directories under the passed directory
# will be examined.

dir="$1"; shift
if [ -z "$dir" ]; then
  dir=.
fi

source "$FEISTY_MEOW_SCRIPTS/rev_control/version_control.sh"

pushd "$dir" &>/dev/null
tempfile=$(generate_rev_ctrl_filelist)
popd &>/dev/null

perform_revctrl_action_on_file "$tempfile" do_checkin

