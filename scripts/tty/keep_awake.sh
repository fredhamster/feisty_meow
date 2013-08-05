#!/bin/bash
# keep_awake: sends a message to the screen from the background.
#
# This program is really just a way to start the keep_awake process in the
# background instead of needing to start a subshell here.  There was some
# kind of snafu with the ksh environment variable $$ where it would always
# record the previous shell's number and not the current one or something....
#
(bash $FEISTY_MEOW_SCRIPTS/tty/keep_awake_process.sh) &
