#!/bin/bash

##############
# Name   : unpack
# Author : Chris Koeritz
# Rights : Copyright (C) 2012-$now by Feisty Meow Concerns, Ltd.
##############
# This script is free software; you can modify/redistribute it under the terms
# of the GNU General Public License. [ http://www.gnu.org/licenses/gpl.html ]
# Feel free to send updates to: [ fred@gruntose.com ]
##############
#
# An arbitrary format archive unpacker, although really we are mainly supporting
# tar and zip currently, including compressed formats.

unpack_file="$1"; shift
if [ -z "$unpack_file" ]; then
  echo "This script takes one archive name (in .tar.gz, .zip, etc. formats) and"
  echo "unpacks the archive with the appropriate tool."
  exit 1
fi
if [ ! -f "$unpack_file" ]; then
  echo "The file specified for unpacking cannot be located: $unpack_file"
  exit 1
fi
unpack_dir="$1"; shift
if [ -z "$unpack_dir" ]; then
  unpack_dir=$(echo unpacked_$(basename $unpack_file) | sed -e 's/^\([^\.]*\)\..*/\1/')
fi

if [ ! -d "$unpack_dir" ]; then
  mkdir "$unpack_dir"
  if [ $? -ne 0 ]; then
    echo "Could not create the unpacking directory: $unpack_dir"
    exit 1
  fi
fi

# save where we started out.
ORIGINATING_FOLDER="$( \cd "$(\dirname "$0")" && \pwd )"

pushd "$unpack_dir" &>/dev/null

if [ ! -f "$unpack_file" ]; then
  # we're assuming we left it behind in our previous directory.
  unpack_file="$ORIGINATING_FOLDER/$unpack_file"
  if [ ! -f "$unpack_file" ]; then
    echo "Could not find file to unpack after shifting directories.  Sorry."
    echo "Tried to locate it as: $unpack_file"
    exit 1
  fi
fi

if [[ $unpack_file =~ .*\.tar ]]; then
  tar -f $unpack_file
elif [[ $unpack_file =~ .*\.tar\.gz \
    || $unpack_file =~ .*\.tar\.bz2 \
    || $unpack_file =~ .*\.tgz ]]; then
  tar -xf $unpack_file
elif [[ $unpack_file =~ .*\.zip ]]; then
  unzip $unpack_file
fi
save_err=$?

if [ $save_err -ne 0 ]; then
  echo "There was a failure reported while unpacking: $unpack_file"
  echo "into the directory: $unpack_dir"
  popd &>/dev/null
  exit 1
else
  echo "Unpacked file $(basename $unpack_file) into folder: $unpack_dir"
fi


