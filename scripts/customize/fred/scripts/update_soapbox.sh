#!/bin/bash

# updates my little 1 TB "soapbox" style usb drive with items that it should contain.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

function get_source()
{
  folder="$1"; shift
  echo getting latest codes in $folder...
  pushd "$folder"
  if [ $? -ne 0 ]; then
    echo Changing to the folder $folder failed.
    exit 1
  fi
  bash "$FEISTY_MEOW_SCRIPTS/rev_control/rev_checkin.sh"
  if [ $? -ne 0 ]; then
    echo Checking out the latest codes has failed somehow for $folder.
    exit 1
  fi
  popd
}

sep

ls /media/fred/soapboxdrive
if [ $? -ne 0 ]; then
  echo The soapbox drive is not mounted currently, so cannot be updated.
  exit 1
fi

for currdir in basement imaginations musix walrus; do
  sep
  echo "synching $currdir..."
  netcp /z/$currdir/* /media/fred/soapboxdrive/$currdir/
  if [ $? -ne 0 ]; then
    echo "The $currdir sync failed."
    exit 1
  fi
done

sep

echo getting latest fred repositories...
pushd /media/fred/soapboxdrive
get_source extra_brain

sep

echo Updated all portions of the soapbox drive successfully.

