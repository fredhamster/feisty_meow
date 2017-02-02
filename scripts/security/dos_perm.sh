#!/bin/bash
# dos_perm: whacks a directory with the most open set of permissions
# available to the dos/windoze attrib command.

folder="$1"; shift

if [ -z "$folder" ]; then
  echo "This program requires a folder to set the permissions on.  That folder and all"
  echo "files in it will be opened up to as full a permission level as DOS's attrib"
  echo "command is capable of."
  exit 3
fi

folder="${folder}/"  # add a slash to ensure there's at least one after drive letters.

dos_folder=$(echo $folder | sed -e 's/\/\([a-zA-Z]\)\/\(.*\)/\1:\/\2/' | sed -e 's/\//\\\\/g')
#echo dos folder is $dos_folder

attrib -r -s -h //s //d "$dos_folder\\*"

