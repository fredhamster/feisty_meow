#!/usr/bin/env bash

# version_utils:
#
# Some utilities for dealing with version stamps.

function print_instructions {
  echo -e "\
This script processes version numbers in an ini file formatted file.  It\n\
currently only supports one set of version numbers in the file, in the form:\n\
  major=0\n\
  minor=420\n\
  revision=2323\n\
  build=0\n\
The first parameter to the script is a command.  The available commands\n\
consists of:\n\
  show\n\
The second parameter is the version file to process.\n\
All parameters are required."

  exit 5
}

command=$1
if [ -z "$command" ]; then print_instructions; fi
version_file=$2
if [ -z "$version_file" ]; then print_instructions; fi

#echo got a command $command and version file $version_file

if [ "$command" = "show" ]; then
  # show the version in the version file.
  if [ ! -f "$version_file" ]; then
    echo "The version file \"$version_file\" doesn't seem to exist."
    exit 8
  fi
  major=$(grep 'major=' "$version_file" | sed -e 's/.*major=\([! #]*\)/\1/')
  minor=$(grep 'minor=' "$version_file" | sed -e 's/.*minor=\([! #]*\)/\1/')
  revision=$(grep 'revision=' "$version_file" | sed -e 's/.*revision=\([! #]*\)/\1/')
  build=$(grep 'build=' "$version_file" | sed -e 's/.*build=\([! #]*\)/\1/')
#echo major=$major minor=$minor rev=$revision build=$build

  echo -n $major.$minor.$revision.$build
fi


