#!/bin/bash

function print_instructions()
{
  echo -e "\n$(basename $0 .sh):\n"

  # calculate the number of columsn in the terminal.
  local cols=$(get_maxcols)

  echo -e 'this script takes two parameters, a "here" folder and a "there" folder, almost as if it were a copy command.  but instead, this removes any file from under the "here" location if it cannot be found in the "there" location.  so the "there" location is considered a more definitive template of what should be in "here", such that we strip out what "there" does not have.\n\n
the most" useful way to use this script is for a "here" hierarchy that is a copy of an older version of another "there" hierarchy.  the "there" hierarchy may have changed a lot, including new files, changed files, and deleted files.  it is a simple operation to copy everything from "there" into "here" (such as by using the command [ cp -R "$there"/* "$here" ] ) , but it is a lot harder to determine what stuff in "here" is out of date and should be removed.  that is where this script comes in; it can be run to flush out any older things in "here", rather than requiring the user to manually find all those files.  ' | splitter --maxcol $(($cols - 1))
  echo
  echo "Example Usage:"
  echo 
  echo "$(basename $0 .sh) backup_copy original_folder"
  echo
  echo " ==> Deletes any files in backup_copy that are not found in original_folder." 
  echo 
}

if [ $# -lt 2 ]; then
  print_instructions
  echo "There were not enough parameters provided to the script."
  exit 1
fi

here="$1"; shift
there="$1"; shift

if [ ! -d "$there" -o ! -d "$here" ]; then
  print_instructions
  echo "One of the directories specified does not exist."
  exit 1
fi

here_temp_file="$(mktemp "$TMP/remover_list.XXXXXX")"
there_temp_file="$(mktemp "$TMP/remover_list.XXXXXX")"

# find all the files in both hierarchies.
pushd "$here" &>/dev/null
find "." -type f | sort >"$here_temp_file"
popd &>/dev/null
pushd "$there" &>/dev/null
find "." -type f | sort >"$there_temp_file"
popd &>/dev/null

whack_list="$(mktemp "$TMP/remover_list.XXXXXX")"

comm -23 "$here_temp_file" "$there_temp_file" >"$whack_list"

while read input_text; do
  if [ -z "$input_text" ]; then break; fi
  herename="$here/$input_text"
  rm -v "$herename"
done <"$whack_list"

# clean up.
rm "$whack_list" "$here_temp_file" "$there_temp_file"

