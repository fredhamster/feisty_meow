#!/bin/bash

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

if debian_like; then
  dpkg -la | grep $*
  exit $?
fi

rpm_available="$(which rpm)"
if [ ! -z "$rpm_available" ]; then
#is that the right phrase for rpm?  somewhat forgotten.
  rpm -qa | grep $*
  exit $?
fi

yum_available="$(which yum)"
if [ ! -z "$yum_available" ]; then
  yum list | grep $*
  exit $?
fi

echo "Could not deduce what type of OS this is; missing package listing commands."
exit 1
