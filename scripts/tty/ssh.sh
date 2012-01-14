#!/bin/bash

# a wrapper for the secure shell.
# we want to fix any terminal titles that the foreign shells give us, and
# this script is our chance to do so.

\ssh -i $HOME/.ssh/id_dsa_fred -X $*

# re-run our terminal labeller.
bash $SHELLDIR/tty/label_terminal_with_infos.sh


