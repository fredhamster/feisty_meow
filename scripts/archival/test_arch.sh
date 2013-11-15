#!/bin/bash

##############
# Name   : test_arch
# Author : Chris Koeritz
# Rights : Copyright (C) 2012-$now by Feisty Meow Concerns, Ltd.
##############
# This script is free software; you can modify/redistribute it under the terms
# of the GNU General Public License. [ http://www.gnu.org/licenses/gpl.html ]
# Feel free to send updates to: [ fred@gruntose.com ]
##############
#
# An arbitrary format archive tester, although really we are mainly supporting
# tar and zip currently, including compressed formats.

archive_file="$1"; shift
if [ -z "$archive_file" ]; then
  echo "This script takes one archive name (in .tar.gz, .zip, etc. formats) and"
  echo "tests the archive with the appropriate tool."
  exit 1
fi
if [ ! -f "$archive_file" ]; then
  echo "The file specified cannot be located: $archive_file"
  exit 1
fi

# save where we started out.
ORIGINATING_FOLDER="$( \pwd )"

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
    || $archive_file =~ .*\.snarf$ \
    ]]; then
  tar -tf $archive_file &>/dev/null
elif [[ $archive_file =~ .*\.zip$ \
    || $archive_file =~ .*\.epub$ \
    || $archive_file =~ .*\.odt$ \
    || $archive_file =~ .*\.jar$ \
    || $archive_file =~ .*\.war$ \
    ]]; then
  unzip -t $archive_file &>/dev/null
else
  # we don't know this as an archive.
  exit 0
fi

if [ $? -ne 0 ]; then
  echo "** failure while testing: $archive_file"
  exit 1
else
  echo "good: $archive_file"
fi

