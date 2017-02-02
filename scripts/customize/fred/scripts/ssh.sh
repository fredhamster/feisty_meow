#!/bin/bash

# wraps calling the secure shell to let us pick our appropriate credentials.

source "$FEISTY_MEOW_SCRIPTS/tty/terminal_titler.sh"

#hmmm: is this still used???
#  it seems redundant with the ssh config file that says which creds to use.
source "$FEISTY_MEOW_LOADING_DOCK/custom/scripts/pick_credentials.sh"

# remember the old title.
save_terminal_title

# force the TERM variable to a more generic version for other side.
# we don't want the remote side still thinking it's running xterm.
export TERM=linux

#hmmm: it would be good to set an interrupt handler here and
#      trap ctrl-c, since otherwise we are getting exited from and losing a chance
#      to reset the terminal title.  this actually happens a lot, since some X11
#      or other background process is left running and the ssh never actually quits,
#      forcing one to hit ctrl-c.

if [ ! -z "$keyfile" ]; then
  \ssh -i "$keyfile" -X -C $*
else
  \ssh -X -C $*
fi

restore_terminal_title

