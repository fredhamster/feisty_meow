#!/usr/bin/env bash

# counts up the files in each maildir 'cur' folder to find out how many messages are
# held in each subdirectory.

pushd "$HOME/Maildir" &>/dev/null

folderlist=$TMP/zz_folder_list.txt

find . -name "cur" >$folderlist

while read input_text; do echo -n "$input_text  -- " ; find "$input_text" -type f | wc -l ; done < $folderlist

popd &>/dev/null

# how to sum the totals...
# another command perhaps.
# bash ~/count_maildir.sh | sed -e 's/.* -- \(.*\)$/\1/' | paste -sd+ | bc

