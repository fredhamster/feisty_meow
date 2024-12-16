#!/usr/bin/env bash

# looks for process names for a particular user.  if no user is specified, then this
# assumes we should look for the current user's processes.

user="$1"
if [ -z "$user" ]; then
  user="$USER"
fi

ps wuax | grep $user | awk '{ print $11; }'
