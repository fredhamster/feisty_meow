#!/usr/bin/env bash

source "$FEISTY_MEOW_SCRIPTS/core/functions.sh"

if [ -z "$*" ]; then
  echo This script requires a package name to find in the installed list of packages.
  exit 1
fi

if debian_like; then
  apt -qq list "$1" 2>/dev/null | grep -q '[installed]'
ret=$?
echo ret from seeking actual is $ret
exit $ret

  exit $?
fi

rpm_available="$(whichable rpm)"
if [ ! -z "$rpm_available" ]; then
  rpm -q --queryformat %{NAME} -p "$1"
#hmmm: this is probably noisy, but we need an rpm system to test on.
  exit $?
fi

# yum should never be available if rpm was not!
#yum_available="$(whichable yum)"
#if [ ! -z "$yum_available" ]; then
#  yum list | eval $SEEK_PIECE
#  exit $?
#fi

echo "Could not deduce what type of OS this is; missing package listing commands."
exit 1

