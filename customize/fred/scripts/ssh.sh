#!/bin/bash

# wraps our calling the secure shell and lets us pick our credentials.

source "$FEISTY_MEOW_LOADING_DOCK/custom/scripts/pick_credentials.sh"

if [ -z "$PRIOR_TERMINAL_TITLE" ]; then
  # save the former terminal title if we're running in X with xterm.
  PRIOR_TERMINAL_TITLE=
  which xprop &>/dev/null
  if [ $? -eq 0 ]; then
    if [[ "$TERM" =~ .*"xterm".* ]]; then
      PRIOR_TERMINAL_TITLE="$(xprop -id $WINDOWID | perl -nle 'print $1 if /^WM_NAME.+= \"(.*)\"$/')"
    fi
  fi
fi

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
#-c blowfish-cbc 
else
  \ssh -X -C $*
#-c blowfish-cbc 
fi

if [ $? -eq 0 ]; then
  # we don't want to emit anything extra if this is being driven by git.
  if [ -z "$(echo $* | grep git)" ]; then
    # re-run the terminal labeller after coming back from ssh.
    # we check the exit value because we don't want to update this for a failed connection.
    if [ -z "$PRIOR_TERMINAL_TITLE" ]; then
echo prior title nil new label
      bash $FEISTY_MEOW_SCRIPTS/tty/label_terminal_with_infos.sh
    else
echo "using old prior title of '$PRIOR_TERMINAL_TITLE'"
      bash $FEISTY_MEOW_SCRIPTS/tty/set_term_title.sh "$PRIOR_TERMINAL_TITLE"
      # remove the value for this, since we did our job on it.
      unset PRIOR_TERMINAL_TITLE
    fi
  fi
fi


