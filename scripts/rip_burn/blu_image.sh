#!/bin/bash

function show_usage()
{
  echo "This script needs two parameters, (1) an ISO file to create, and"
  echo "(2) a folder to use as the blu-ray data for the ISO.  For example,"
  echo "  $(basename $0) ~/grunge.iso ~/dvdimages/grungebandpro"
}

iso_name="$1"; shift
folder_name="$1"; shift

if [ -z "$iso_name" -o -z "$folder_name" ]; then
  show_usage
  exit 3
fi

if [ -f "$iso_name" ]; then
  echo -e "The ISO file must not already exist.\n"
  show_usage
  exit 3
fi

if [ ! -d "$folder_name" ]; then
  echo -e "The provided folder name must exist.\n"
  show_usage
  exit 3
fi

genisoimage -r -J -o "$iso_name" "$folder_name"

