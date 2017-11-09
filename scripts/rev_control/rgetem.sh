#!/bin/bash
# a simple script for updating a set of folders from subversion or git.
# this can take a directory as parameter, but will default to the current
# working directory.  all the directories under the passed directory will
# be examined.

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
popd &>/dev/null

perform_revctrl_action_on_file "$tempfile" do_update
test_or_die "running revision control update"

