#!/bin/bash
# checks in the updated files in a set of folders checked out from subversion
# or git.  this can take a directory as parameter, but will default to the
# current working directory.  all the directories under the passed directory
# will be examined.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"
source "$FEISTY_MEOW_SCRIPTS/rev_control/version_control.sh"

##############

dir="$1"; shift
if [ -z "$dir" ]; then
  dir=.
fi

pushd "$dir" &>/dev/null
test_or_die "changing to directory: $dir"
tempfile=$(generate_rev_ctrl_filelist)
test_or_die "generating revision control file list"
popd &>/dev/null

perform_revctrl_action_on_file "$tempfile" do_checkin
test_or_die "doing a check-in on: $tempfile"

