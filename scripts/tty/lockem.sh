#!/bin/bash

# Can be used to lock up a terminal screen until the user enters the correct
# password.  Pretty sturdy.  Store your password in the file "PASSWORD_FILE",
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

PASSWORD_FILE=$TMP/lockem.password
LOG_FILE=$TMP/session-lockem.log

if [ ! -f "$PASSWORD_FILE" ]; then
  sep
  echo "no password file is configured.  please put your unlock password in:"
  echo "$PASSWORD_FILE"
  sep
  exit 1
fi

chmod 700 "$PASSWORD_FILE"

read password <"$PASSWORD_FILE"

echo "$(date_stringer): +++ terminal locked" >>"$LOG_FILE"

attempt=""

stty -echo
while [[ $attempt != $password ]]; do
  if [ ! -z "$attempt" ]; then
    echo "$(date_stringer): login attempt with wrong password at $(date)" >>$LOG_FILE
  fi
  clear
  nechung
  echo -ne "\npassword: "
  read attempt
done
stty echo

echo "$(date_stringer): successful login" >>$LOG_FILE
echo "$(date_stringer): --- terminal unlocked" >>$LOG_FILE

clear
echo "hi $USER, your password has been accepted.  enjoy your computer."
echo


