#!/bin/bash

THIS_FOLDER="$( \cd "$(\dirname "$0")" && /bin/pwd )"
pushd $THIS_FOLDER/modem_driver/*
sudo apt-get remove dgcmodem
dpkg -i dgcmodem_1.13_i386.deb
popd


