#!/bin/bash

# musical_wand: distributes music from our primary source to all hosts that are listed
# as being redundant copies for the music.

# this script is designed to be run on the music host with the super alpha main source of
# music plugged in as an external drive.  that being said, it will still work as long as
# the music host has its local copy intact; the local copy of the primary is always what
# is synched onto the other archive hosts.  in that sense, the musical host is itself a
# musix archive, but it is treated "special".

source "$FEISTY_MEOW_SCRIPTS/core/functions.sh"

#hmmm: add the goodness around these actions like the "nice" updater so we catch all errors.

# this host is where all the music is supposed to come from.
MUSICAL_HOST=curie

#hmmm: this script is currently limited to run ON the music host.  it could easily do the backwards thing instead, and copy FROM music host.

# the list of hosts we know of that are holding onto duplicate copies of the musix archive.
#old list MUSIX_ARCHIVE_SITE_LIST=(surya banshee wildmutt euphrosyne)
MUSIX_ARCHIVE_SITE_LIST=(euphrosyne)
#hmmm: list was contracted a lot, since we don't want to step on the updates done by syncthing.  euphrosyne is still our reference copy for what the archive states "should" be.


if [[ ! ( $(hostname) =~ .*${MUSICAL_HOST}.* ) ]]; then
  echo "This script is only designed to run on $MUSICAL_HOST with the"
  echo "primary fred music source (external) disc plugged in."
  exit 1
fi

# synch our local copy on the music host with the primary music drive, source of all goodness.
function get_music_from_alpha_site()
{
  sep
  echo "getting musix and basement from fred music prime device"
  rsync -av /media/fred/fredmusicprime/musix/* /z/musix/
  rsync -av /media/fred/fredmusicprime/basement/* /z/basement/
  sep
  echo
}

# updates the music on a remote host to our current local copy on the music host.
function update_musix_pile()
{
  sep
  local host="$1"; shift
  echo "$host: synching musix and basement"
  rsync -avz /z/musix/* ${host}:/z/musix/ 
  rsync -avz /z/basement/* ${host}:/z/basement/ 
  sep
  echo
}

##############

# make sure the local machine, our first-stop musix host, is in good shape.
get_music_from_alpha_site

# run through the steps of updating all our machines.
for i in ${MUSIX_ARCHIVE_SITE_LIST[@]}; do
  update_musix_pile $i
done


