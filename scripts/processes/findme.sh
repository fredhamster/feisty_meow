#!/bin/bash
# finds the current user's processes in the process list.
snuser=$USER
# if the unix variable for the user is not set, try the dos variable.
if [ -z "$snuser" ]; then snuser=$USERNAME; fi
#hmmm: more checks?  what else would we get it from, REPLYTO?

bash "$FEISTY_MEOW_SCRIPTS/users/find_user.sh" $snuser
