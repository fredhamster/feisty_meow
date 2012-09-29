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

archive_file="$1"; shift
if [ -z "$archive_file" ]; then
  echo "This script takes one archive name (in .tar.gz, .zip, etc. formats) and"
  echo "unpacks the archive with the appropriate tool."
  exit 1
fi
if [ ! -f "$archive_file" ]; then
  echo "The file specified cannot be located: $archive_file"
  exit 1
fi
unpack_dir="$1"; shift
if [ -z "$unpack_dir" ]; then
  unpack_dir=$(echo arch_$(basename $archive_file) | sed -e 's/^\([^\.]*\)\..*/\1/')
fi

if [ ! -d "$unpack_dir" ]; then
  mkdir "$unpack_dir"
  if [ $? -ne 0 ]; then
    echo "Could not create the unpacking directory: $unpack_dir"
    exit 1
  fi
fi

# save where we started out.
ORIGINATING_FOLDER="$( \pwd )"

pushd "$unpack_dir" &>/dev/null

if [ ! -f "$archive_file" ]; then
  # we're assuming we left it behind in our previous directory.
  archive_file="$ORIGINATING_FOLDER/$archive_file"
  if [ ! -f "$archive_file" ]; then
    echo "Could not find file to unpack after shifting directories.  Sorry."
    echo "Tried to locate it as: $archive_file"
    exit 1
  fi
fi

if [[ $archive_file =~ .*\.tar$ \
    || $archive_file =~ .*\.tar\.gz$ \
    || $archive_file =~ .*\.tar\.bz2$ \
    || $archive_file =~ .*\.iar$ \
    || $archive_file =~ .*\.oar$ \
    || $archive_file =~ .*\.tgz$ \
    || $archive_file =~ .*\.ova$ \
    ]]; then
  tar -xf $archive_file &>/dev/null
elif [[ $archive_file =~ .*\.zip$ \
    || $archive_file =~ .*\.epub$ \
    || $archive_file =~ .*\.odt$ \
    || $archive_file =~ .*\.jar$ \
    || $archive_file =~ .*\.war$ \
    ]]; then
  unzip $archive_file &>/dev/null
fi
save_err=$?

popd &>/dev/null

if [ $save_err -ne 0 ]; then
  echo "There was a failure reported while unpacking: $archive_file"
  echo "into the directory: $unpack_dir"
  exit 1
else
  echo "Unpacked file $(basename $archive_file) into folder: $unpack_dir"
fi


