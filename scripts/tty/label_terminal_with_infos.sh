#!/bin/bash

# this script makes a new title for the terminal window that matches the
# hostname and some other details.

source $FEISTY_MEOW_SCRIPTS/core/functions.sh

# we only label the terminal anew if there's no saved title.

#hmmm: how about putting that title back in place?
# we currently do this manually in places, like ssh, which is dumb.


if [ -z "$PRIOR_TERMINAL_TITLE" ]; then
  pruned_host=$(echo $HOSTNAME | sed -e 's/^\([^\.]*\)\..*$/\1/')
  date_string=$(date +"%Y %b %e @ %T")

  user=$USER
  if [ -z "$user" ]; then
    # try snagging the windoze name.
    user=$USERNAME
  fi
  
  new_title="-- $user@$pruned_host -- [$date_string]"
  
  bash "$FEISTY_MEOW_SCRIPTS/tty/set_term_title.sh" "$new_title"
fi
