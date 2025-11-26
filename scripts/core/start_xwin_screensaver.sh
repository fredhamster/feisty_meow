#!/usr/bin/env bash

# sets up a screen lock with the xsecurelock program, and assumes
# that it will use the xscreensaver utility for the screen saving.

source "$FEISTY_MEOW_SCRIPTS/core/functions.sh"
source "$FEISTY_MEOW_SCRIPTS/processes/process_manager.sh"

if ! test_for_xwin; then
  if [ ! -z "$DEBUG_FEISTY_MEOW" ]; then
    echo "X windows is not running; will not start up xsecurelock."
  fi
  exit 1
fi

# see if xss-lock is already running.
xss_running="$(psa xss-lock)"
# clean up CRLF type junk to allow emptiness check.
xss_running=${xss_running//$'\n'/}
xss_running=${xss_running//$'\r'/}

if [ ! -z "$DEBUG_FEISTY_MEOW" ]; then
  echo -e "check for running xss-lock came up with: '$xss_running'"
fi
if [ ! -z "$xss_running" ]; then
  if [ ! -z "$DEBUG_FEISTY_MEOW" ]; then
    echo "The xss-lock application is already running, so a screensaver is already hooked in."
  fi
  exit 0
fi

#steps still to take:
#  install xsecurelock and such if not present yet.
#  install xscreensaver and packages if not present yet.
#  use our feisty meow version of sudo grabbing thingy.
#sudo apt install xsecurelock mpv xscreensaver-data xscreensaver-data-extra xscreensaver-gl xscreensaver-gl-extra xscreensaver xss-lock

# need to kill xscreensaver if it's running.
#  e.g. killall -9 xscreensaver

# need to fix the xsecurelock file for the xscreensaver...
#  => we have a function that does this kind of replacement editing!
# in file:
# /usr/libexec/xsecurelock/saver_xscreensaver
# modify this line:
# : ${XSECURELOCK_XSCREENSAVER_PATH:=/usr/lib/xscreensaver}
# to become this line:
# : ${XSECURELOCK_XSCREENSAVER_PATH:=/usr/share/xscreensaver}

DIMMER="/usr/libexec/xsecurelock/dimmer"
if [ ! -x "$DIMMER" ]; then
  if [ ! -z "$DEBUG_FEISTY_MEOW" ]; then
    echo "Could not find the dimmer app for xsecurelock; assuming xsecurelock is not installed and giving up."
  fi
  exit 1
fi

xset s 300 5
exit_on_error "setting the x window inactivity timeout"

start_background_action \
  "xss-lock -n "$DIMMER" -l -- xsecurelock"
exit_on_error "installing xsecurelock as the screensaver"

if [ ! -z "$DEBUG_FEISTY_MEOW" ]; then
  echo xsecurelock has been started.
fi

