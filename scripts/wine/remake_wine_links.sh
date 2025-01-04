#!/usr/bin/env bash

# fixes the links that wine makes to our home folder under linux.  that's a really bad
# practice that exposes all one's private files to the wine subsystem.  dumb.
# instead, this replaces those links to a local folder with things that the wine applications
# can't pooch up too much.  must be used within a wine user directory.  for example,
# i fix my directory '~/.PlayOnLinux/wineprefix/Steam/drive_c/users/fred' with this.

if [ ! -L "My Documents" -o ! -L "Desktop" ]; then
  echo "This script is meant to be used in a user directory under wine."
  echo "It will re-hook the links for the desktop and documents to a local folder"
  echo "called '~/linx/wine_goods'"
  exit 1
fi

rm "Desktop" "My Documents" "My Pictures" "My Videos" "My Music"

if [ ! -d ~/linx/wine_goods ]; then
  mkdir ~/linx/wine_goods
fi
if [ ! -d ~/linx/wine_goods/desktop ]; then
  mkdir ~/linx/wine_goods/desktop
fi
if [ ! -d ~/linx/wine_goods/otherlinks ]; then
  mkdir ~/linx/wine_goods/otherlinks
fi

ln -s ~/linx/wine_goods "My Documents"
ln -s ~/linx/wine_goods/desktop "Desktop"
ln -s ~/linx/wine_goods/otherlinks "My Pictures"
ln -s ~/linx/wine_goods/otherlinks "My Videos"
ln -s ~/linx/wine_goods/otherlinks "My Music"


