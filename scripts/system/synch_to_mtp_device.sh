#!/bin/bash

sourcedir="$1"; shift
targetdir="$1"; shift

# where we will look for mtp devices.
mtp_base_path="/run/user/$UID/gvfs/mtp\:host\="

if [ -z "$sourcedir" -o -z "$targetdir" ]; then
  echo "This script needs source and target directory names that can be synched"
  echo "between the computer's file system and a USB drive mounted with the mtp"
  echo "protocol.  The folder on the USB drive should include the entire path except"
  echo "for the device mount location.  For example:"
  echo "    $(basename $0) ebooks \"/Internal\ Storage/My\ Files/ebooks\""
  exit 1
fi

# the mtp part will flux.  if there is more than one device mounted, this will hose up.
#checking for more than one device there:
mtpdevices=("$mtp_base_path"*)
if [ ${#mtpdevices[@]} -ne 1 ]; then
  echo "There is more than one MTP device mounted.  This script requires exactly one"
  echo "MTP device mounted at a time.  Sorry."
  exit 1
elif [ ! -d "${#mtpdevices[@]}" ]; then
  echo "The MTP device does not seem to be mounted currently.  The path did not"
  echo "expand properly."   
  exit 1
fi

rsync -rv --exclude *.git --exclude *.svn "$sourcedir" "${mtpdevices[0]}/$targetdir"

