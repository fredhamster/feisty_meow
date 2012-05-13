#!/bin/bash

# goes through a list of pictures, first showing the picture to you,
# and then nagging you for each one if you want to delete it or not.
# it's intended for culling a bunch of vacation pictures.  it does a
# safe delete on the file in case there's a mistake, so if feisty meow
# scripts are set up properly, these will be in your del-keeper folder.

# go through all the files that were passed in, if any.
for i in "$@"; do
  file="$i"
  echo "showing file: $file"
  # display the file with eog.
  eog "$file"
  # now ask the big question: to whack or not to whack?
  echo "whack this file? (y/N)"
  read line
  # a few things mean yes here.
  if [ "$line" == "y" -o "$line" == "Y" -o "$line" == "yes" ]; then
    echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
    echo "$(date): deleting $file."
    perl $FEISTY_MEOW_SCRIPTS/files/safedel.pl "$file"
    echo "$(date): done deleting $file."
    echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
  else
    echo not deleting.
  fi
done

