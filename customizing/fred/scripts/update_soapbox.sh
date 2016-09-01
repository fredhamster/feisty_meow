#!/bin/bash

# updates my little 1 TB "soapbox" style usb drive with items that it should contain.

function liney()
{
  echo
  echo ==============
  echo
}

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

liney

liney

ls /media/fred/soapboxdrive
if [ $? -ne 0 ]; then
  echo The soapbox drive is not mounted currently, so cannot be updated.
  exit 1
fi

liney

echo synching walrus...
rsync -av /z/walrus/* /media/fred/soapboxdrive/walrus/
if [ $? -ne 0 ]; then
  echo The walrus sync failed.
  exit 1
fi

liney

echo synching musix...
rsync -av /z/musix/* /media/fred/soapboxdrive/musix/
if [ $? -ne 0 ]; then
  echo The musix sync failed.
  exit 1
fi

liney

echo getting latest fred codes...
pushd /media/fred/soapboxdrive
get_source extra_brain

liney

echo getting latest gffs codes...
get_source gffs
popd

liney

echo Updated all portions of the soapbox drive successfully.

