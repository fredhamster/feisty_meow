#!/bin/bash

# Thanks to Kevin Wika for this shell.

trap '' HUP
trap '' INT
trap '' QUIT
trap '' STOP
trap '' EXIT
trap '' TSTP

PASSWORDFILE=$TMP/lockup_password_file
LOGFILE=$TMP/lockup.log

read password <$PASSWORDFILE

echo "$(date_stringer): terminal locked" >>$LOGFILE

attempt=""

stty -echo
while [[ $attempt != $password ]]
  do
    if [ ! -z "$attempt" ]; then
      echo "$(date_stringer): $attempt" >>$LOGFILE
    fi
    clear
    echo -n "password: "
    read attempt
  done
stty echo

echo -e "$(date_stringer): successful login\n" >>$LOGFILE

clear
echo "Your Password has been Accepted."
