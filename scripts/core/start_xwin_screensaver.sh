#!/usr/bin/env bash

# sets up a screen lock with the xsecurelock program, and assumes
# that it will use the xscreensaver utility for the screen saving.

source "$FEISTY_MEOW_SCRIPTS/core/functions.sh"
source "$FEISTY_MEOW_SCRIPTS/processes/process_manager.sh"
source "$FEISTY_MEOW_SCRIPTS/tty/terminal_titler.sh"

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

# fix the xsecurelock file for the xscreensaver; paths haven't been updated to latest.
XSECURELOCK_XSCREENSAVER='/usr/libexec/xsecurelock/saver_xscreensaver'
grep -q '/usr/lib/xscreensaver' $XSECURELOCK_XSCREENSAVER
#hmmm: also, check that we're using the right path for xscreensaver!  what if old system, where in old place?
if [ $? -eq 0 ]; then
  if [ ! -z "$DEBUG_FEISTY_MEOW" ]; then
    echo "decided that we needed to modify the saver_xscreensaver file for xsecurelock"
  fi
  file="$XSECURELOCK_XSCREENSAVER"
  pattern=': ${XSECURELOCK_XSCREENSAVER_PATH:=/usr/lib/xscreensaver}'
  replacement=': ${XSECURELOCK_XSCREENSAVER_PATH:=/usr/libexec/xscreensaver}'
  # code borrowed from replace_pattern_in_file, but we need sudo here.
  sudo sed -i -e "s%$pattern%$replacement%g" "$file"
  exit_on_error "editing the xsecurelock saver_xscreensaver file"
  if [ ! -z "$DEBUG_FEISTY_MEOW" ]; then
    echo "successfully modified the saver_xscreensaver file for xsecurelock"
  fi
fi

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
continue_on_error "starting up xsecurelock as the screensaver"

if [ ! -z "$DEBUG_FEISTY_MEOW" ]; then
  echo xsecurelock has been started.
fi

