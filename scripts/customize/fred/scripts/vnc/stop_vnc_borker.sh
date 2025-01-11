#!/usr/bin/env bash

# stops the vnc server and restarts it.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

sep
echo stopping vncserver...
sudo service vncserver stop
sep
echo after stopping vncserver
psa vnc.*Xauth.*geom
sep
