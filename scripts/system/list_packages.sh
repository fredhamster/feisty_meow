#!/bin/bash

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

if [ -z "$*" ]; then
  # no parameters, so we do a wildcard style of listing packages.
  SEEK_PIECE=' cat '
else
  # we know the parameters are not empty, so we don't go with our
  # wildcard approach for listing the package names.
  SEEK_PIECE=' grep -i $* '
fi

if debian_like; then
  dpkg -la | eval $SEEK_PIECE
  exit $?
fi

rpm_available="$(whichable rpm)"
if [ ! -z "$rpm_available" ]; then
#is that the right phrase for rpm?  somewhat forgotten.
  rpm -qa | eval $SEEK_PIECE
  exit $?
fi

yum_available="$(whichable yum)"
if [ ! -z "$yum_available" ]; then
  yum list | eval $SEEK_PIECE
  exit $?
fi

echo "Could not deduce what type of OS this is; missing package listing commands."
exit 1
