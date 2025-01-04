#!/usr/bin/env bash

# this "puffs out" the repositories that it finds.  what this means is that
# any git repositories found will have all of their remote state updated (by
# pulling all remote repos).  this ensures that any upstream changes get
# merged into the local branch.
# it's better to puff out your code regularly rather than waiting for a huge
# merge snarl later.  note that if you check in the code frequently with the
# feisty meow scripts, that will also take care of puffing out the code.

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
tempfile="$(generate_rev_ctrl_filelist)"
exit_on_error "generating revision control file list"
popd &>/dev/null

perform_revctrl_action_on_file "$tempfile" do_revctrl_careful_update
exit_on_error "puffing out repository at: $tempfile"

rm "$tempfile"

restore_terminal_title

