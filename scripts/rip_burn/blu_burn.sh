#!/bin/bash

# burns a bluray or dvd data disc image onto a physical medium.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

function show_usage()
{
  echo "This script needs two parameters, (1) an ISO file that provides the"
  echo "data for the blu-ray and (2) the device to use for burning, e.g."
  echo "  $(basename $0) ~/grunge.iso /dev/sr1"
}

iso_name="$1"; shift
device_name="$1"; shift

if [ -z "$iso_name" -o -z "$device_name" ]; then
  show_usage
  exit 3
fi

if [ ! -f "$iso_name" ]; then
  echo -e "The ISO file must already exist.\n"
  show_usage
  exit 3
fi

if [ ! -b "$device_name" ]; then
  echo -e "The device name provided must exist and be block-special type.\n"
  show_usage
  exit 3
fi

echo "burning disk from image '$iso_name' on device '$device_name'..."
#echo iso "$iso_name" dev "$device_name"

growisofs -dvd-compat -speed=2 -Z ${device_name}=${iso_name}
#hmmm:
#  1) trying with letting it go default speed.  has been working for us recently.
#  2) lots of failures on bluray burns recently; trying dropping the speed back to 2.
#     2024-01-14

exit_on_error growing ISO FS from image ${iso_name}

echo "success burning '$iso_name' onto device '$device_name' !"

