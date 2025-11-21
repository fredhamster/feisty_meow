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

xss_running="$(psa xss-lock)"
if [ ! -z "$xss_running" ]; then
  if [ ! -z "$DEBUG_FEISTY_MEOW" ]; then
    echo "The xss-lock application is already running, so a screensaver is already hooked in."
  fi
  exit 0
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
exit_on_error "installing xsecurelock as the screensaver when inactive"

if [ ! -z "$DEBUG_FEISTY_MEOW" ]; then
  echo xsecurelock has been started.
fi

