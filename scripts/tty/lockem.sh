#!/bin/bash

# Can be used to lock up a terminal screen until the user enters the correct
# password.  Pretty sturdy.  Store your password in the file "PASSWORDFILE",
# configured below.
#
# Thanks to Kevin Wika for this shell.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

trap '' HUP
trap '' INT
trap '' QUIT
trap '' STOP
trap '' EXIT
trap '' TSTP

PASSWORDFILE=$TMP/lockup_password_file
LOGFILE=$TMP/session-lockup.log

if [ ! -f "$PASSWORDFILE" ]; then
  sep
  echo "no password file is configured.  please put your unlock password in:"
  echo "$PASSWORDFILE"
  sep
  exit 1
fi

read password <$PASSWORDFILE

echo "$(date_stringer): +++ terminal locked" >>$LOGFILE

attempt=""

stty -echo
while [[ $attempt != $password ]]; do
  if [ ! -z "$attempt" ]; then
    echo "$(date_stringer): login attempt with wrong password at $(date)" >>$LOGFILE
  fi
  clear
  nechung
  echo -ne "\npassword: "
  read attempt
done
stty echo

echo "$(date_stringer): successful login" >>$LOGFILE
echo "$(date_stringer): --- terminal unlocked" >>$LOGFILE

clear
echo "hi $USER, your password has been accepted.  enjoy your computer."
echo


