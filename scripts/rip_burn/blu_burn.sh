#!/bin/bash

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

echo iso "$iso_name" dev "$device_name"

growisofs -speed=2 -dvd-compat -Z ${device_name}=${iso_name}

