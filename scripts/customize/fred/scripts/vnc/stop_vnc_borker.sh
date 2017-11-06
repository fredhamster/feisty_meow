#!/bin/bash

# stops the vnc server and restarts it.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

sep
sudo service vncserver stop
sep
echo after stopping vncserver
echo
psa vnc.*Xauth.*geom
sep
