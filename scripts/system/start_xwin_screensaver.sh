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

# tests whether the package name provided as a parameter is already installed on the host.
function check_installed()
{
  bash $FEISTY_MEOW_SCRIPTS/system/list_packages.sh "$1" &>/dev/null
}

# checks whether the provided package is already present, and if not, installs it.
#hmmm: only works with apt based systems currently!
#hmmm: list_packages is an inexact check!  it will match xscreensaver-data for xscreensaver as pattern!!!
function install_if_missing()
{
  packname="$1"; shift
  if check_installed "$packname"; then
    if [ ! -z "$DEBUG_FEISTY_MEOW" ]; then
      echo "'$packname' is already installed."
    fi
    # nothing to do.
    return 0
  else
    if [ ! -z "$DEBUG_FEISTY_MEOW" ]; then
      echo "'$packname' is not installed; installing now."
    fi
    sudo apt --assume-yes install "$packname"
    exit_on_error "installing '$packname' package on system"
  fi
}

# install any of the packages we need for xsecurelock and xscreensaver.
for to_install in xsecurelock \
    mpv mplayer \
    xss-lock \
    xscreensaver xscreensaver-data xscreensaver-data-extra \
    xscreensaver-gl xscreensaver-gl-extra; do
  install_if_missing $to_install
done

# need to kill xscreensaver if it's running.
killall -9 xscreensaver &>/dev/null

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

# check whether xsecurelock is actually present now.  it should be...
DIMMER="/usr/libexec/xsecurelock/dimmer"
if [ ! -x "$DIMMER" ]; then
  if [ ! -z "$DEBUG_FEISTY_MEOW" ]; then
    echo "Could not find the dimmer app for xsecurelock; assuming xsecurelock is not installed and giving up."
  fi
  exit 1
fi

# see if xss-lock is already running.
xss_running="$(psa xss-lock)"
# clean up CRLF type junk to allow emptiness check.
xss_running=${xss_running//$'\n'/}
xss_running=${xss_running//$'\r'/}

# sentinel to track whether we're already running or not.
skip_startup=

if [ ! -z "$DEBUG_FEISTY_MEOW" ]; then
  echo -e "check for running xss-lock came up with: '$xss_running'"
fi
if [ ! -z "$xss_running" ]; then
  if [ ! -z "$DEBUG_FEISTY_MEOW" ]; then
    echo "The xss-lock application is already running, so a screensaver is already hooked in."
  fi
  skip_startup=true
fi

# only register the screensaver and crank it up if not running already.
if [ -z "$skip_startup" ]; then
  # set the time-out for inactivity.
  xset s 300 5
  exit_on_error "setting the x window inactivity timeout"

  # run the xss-lock program to handle x-window locking.
  start_background_action \
    "xss-lock -n "$DIMMER" -l -- xsecurelock"
  continue_on_error "starting up xsecurelock as the screensaver"
fi

echo "xsecurelock has been started.  to test locking the screen, type:"
echo "   xset s activate"

