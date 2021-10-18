#!/bin/bash

# makes a playable dvd movie disc image from a folder.
# the folder has to contain a ripped and decrypted DVD.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

function show_usage()
{
  echo "This script needs two parameters, (1) an ISO file to create, and"
  echo "(2) a folder to use as the dvd/cd data for the ISO.  For example,"
  echo "  $(basename $0) ~/grunge.iso ~/dvdimages/grungebandpro"
  echo "where grungebandpro is presumably a directory with a dvd image."
}

iso_name="$1"; shift
folder_name="$1"; shift

if [ -z "$iso_name" -o -z "$folder_name" ]; then
  show_usage
  exit 3
fi

if [ -f "$iso_name" ]; then
  echo -e "The ISO file must not already exist.\n"
  show_usage
  exit 3
fi

if [ ! -d "$folder_name" ]; then
  echo -e "The provided folder name must exist.\n"
  show_usage
  exit 3
fi

mkisofs -dvd-video -o "$iso_name" "$folder_name"

exit_on_error making ISO filesystem from folder ${folder_name}

