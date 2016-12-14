#!/bin/bash

# this script is meant to be run on curie with our super alpha prime source of music plugged in.

#hmmm: add the goodness around these like the nice updater.

if [[ ! ( $(hostname) =~ .*curie.* ) ]]; then
  echo this script is only designed to run on curie with the
  echo fred music prime external disc plugged in.
  exit 1
fi

# synch our local copy on curie with the music drive, source of all goodness.
function get_music_from_alpha_site()
{
  echo "getting musix and basement from fred music prime device"
  rsync -av /media/fred/fredmusicprime/musix/* /z/musix/
  rsync -av /media/fred/fredmusicprime/basement/* /z/basement/
}

# updates the music on a remote host to our current local copy on curie.
function update_musix_pile()
{
  local host="$1"; shift
  echo "$host: synching musix and basement"
  rsync -avz /z/musix/* ${host}:/z/musix/ 
  rsync -avz /z/basement/* ${host}:/z/basement/ 
}

# make sure the local machine, curie, is in good shape.
get_music_from_alpha_site

# run through the steps of updating all our machines.
for i in surya banshee wildmutt euphrosyne; do
  update_musix_pile $i
done


