#!/bin/bash

# keeps trying to mount the cd on a mac mini to overcome
# a new bug in itunes seen as of osx leopard upgrade.

mountpoint=/Volumes/mounty_cd

echo "$(date_stringer): starting cd mounter..."

while true; do
  if [ ! -d $mountpoint ]; then
    mkdir $mountpoint
  fi

  found_device=$(mount|grep -i cddafs)
  if [ -z "$found_device" ]; then
    echo "$(date_stringer): cd not mounted--pausing before attempt..."
    sleep 7
    echo "$(date_stringer): cd not mounted--now trying to mount..."
    mount -t cddafs /dev/disk1 $mountpoint
  else
    echo "$(date_stringer): cd already mounted--ignoring it."
  fi
  sleep 7
done


