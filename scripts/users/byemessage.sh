#!/bin/bash
# byemessage prints out a nechung message as part of logging out.
# the first parameter is the file or device to send the message to.
# if it is blank, then standard output is used.

# figure out where they want to show the message, or pick a default.
TARGET_CONSOLE=$1
if [ -z "$TARGET_CONSOLE" ]; then
  TARGET_CONSOLE=/dev/console
fi

# FORTUNE_TO_SHOW is a massaged form of a fortune cookie output.
export FORTUNE_TO_SHOW="$(mktemp "$TMP/zz_nechung.XXXXXX")"

# see if we can send out a screen blank character.
echo -e '\0xc' >$FORTUNE_TO_SHOW
#####echo >>$FORTUNE_TO_SHOW  # first blank line in the file.
# add lots of blank lines.
for ((i=0; i<100; i++)); do echo >>$FORTUNE_TO_SHOW; done
# drop in a fortune cookie.
$BINDIR/nechung >>$FORTUNE_TO_SHOW
# a couple extra blank lines.
for i in 1 2 3; do echo >>$FORTUNE_TO_SHOW; done
# and a reprinting of a login menu, since that's actually where we're
# intended to leave the machine at.
if [ -f /etc/issue.net ]; then
  cat /etc/issue.net >>$FORTUNE_TO_SHOW
elif [ -f /etc/issue ]; then
  cat /etc/issue >>$FORTUNE_TO_SHOW
fi
echo -ne "$(hostname) login: " >>$FORTUNE_TO_SHOW

# clear the screen, if we can.
clear_console

# we have to do extra processing to send the file out to the console.
tr '\n' '\a' <$FORTUNE_TO_SHOW | sed -r -e 's/\a/\r\n/g' >$TARGET_CONSOLE

# clean up.
/bin/rm $FORTUNE_TO_SHOW

