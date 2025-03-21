#!/usr/bin/env bash
# this script reports files that are not checked in yet in a set of folders.
# it works with subversion only, since git handles new files well whereas
# subversion ignores them until you tell it about them.  this script can take
# a directory as a parameter, but will default to the current directory.
# all the directories under the passed directory will be examined.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"
source "$FEISTY_MEOW_SCRIPTS/rev_control/version_control.sh"

save_terminal_title

##############

dir="$1"; shift
if [ -z "$dir" ]; then
  dir=.
fi

pushd "$dir" &>/dev/null
exit_on_error "changing directory to: $dir"
tempfile="$(generate_rev_ctrl_filelist)"
exit_on_error "generating revision control file list"
popd &>/dev/null

perform_revctrl_action_on_file "$tempfile" do_revctrl_report_new
exit_on_error "running revision control report"

#rm "$tempfile"

restore_terminal_title

