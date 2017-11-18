#!/bin/bash

# some code i wrote to add to revamp that turned out to be unsuitable.
# but it corrects a problem in cakelampvm v002 release that i find annoying,
# so here it is as its own file.

# clean out some old files that were not checked in in mapsdemo.
echo Doing some git repository maintenance in fred account.
#
# change over to fred folder
pushd /home/fred
test_or_die "changing dir to fred's home; what have you done with fred?"

pushd apps/mapsdemo/avenger5
test_or_die "changing dir to mapsdemo app"

rpuffer . &>/dev/null
if [ $? -ne 0 ]; then
  # it seems our old files are still conflicting this.
  if [ -f config/config_google.php ]; then
    \rm -f config/config_google.php
    test_or_die "removing old config for google"
  fi
  if [ -f config/app.php ]; then
    \rm -f config/app.php
    test_or_die "removing old config for app"
  fi

  git reset --hard HEAD
  test_or_die "resetting git's hard head"

  rpuffer .
#hmmm: use output saver thing when that exists.
  test_or_die "puffing out mapsdemo app after inadequate corrective action was taken"
fi

popd

popd
#...coolness, if we got to here.


