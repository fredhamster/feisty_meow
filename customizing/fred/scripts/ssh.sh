#!/bin/bash

# wraps our calling the secure shell and lets us pick our credentials.

source "$FEISTY_MEOW_SCRIPTS/security/pick_credentials.sh"

if [ ! -z "$keyfile" ]; then
  \ssh -i "$keyfile" -X -C -c blowfish-cbc $*
else
  \ssh -X -C -c blowfish-cbc $*
fi

if [ $? -eq 0 ]; then
  # we don't want to emit anything extra if this is being driven by git.
  if [ -z "$(echo $* | grep git)" ]; then
    # re-run the terminal labeller after coming back from ssh.
    # we check the exit value because we don't want to update this for a failed connection.
    bash $FEISTY_MEOW_SCRIPTS/tty/label_terminal_with_infos.sh
  fi
fi


