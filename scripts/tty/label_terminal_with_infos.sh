#!/bin/bash

# this script makes a new title for the terminal window that matches the
# hostname and some other details.

pruned_host=$(echo $HOSTNAME | sed -e 's/^\([^\.]*\)\..*$/\1/')
date_string=$(date +"%Y %b %e @ %T")

user=$USER
if [ -z "$user" ]; then
  # try snagging the windoze name.
  user=$USERNAME
fi

new_title="-- $user@$pruned_host -- [$date_string]"

bash "$FEISTY_MEOW_SCRIPTS/tty/set_term_title.sh" "$new_title"
