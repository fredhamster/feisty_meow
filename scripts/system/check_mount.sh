#!/bin/bash
if [ -z "$1" ]; then
  echo This program needs a mount point as a parameter.  If the mount point is not
  echo already mounted, then the program will attempt to mount it.
  exit 2
fi

if [ -z "$(mount | grep "$1")" ]; then
  mount "$1"
fi


