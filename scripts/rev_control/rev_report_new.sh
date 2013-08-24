#!/bin/bash
# this script reports files that are not checked in yet in a set of folders.
# it works with subversion only, since git handles new files well whereas
# subversion ignores them until you tell it about them.  this script can take
# a directory as a parameter, but will default to the current directory.
# all the directories under the passed directory will be examined.

dir="$1"; shift
if [ -z "$dir" ]; then
  dir=.
fi

source "$FEISTY_MEOW_SCRIPTS/rev_control/version_control.sh"

tempfile=$(generate_rev_ctrl_filelist)

perform_action_on_file "$tempfile" do_report_new

