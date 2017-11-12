#!/bin/bash

# stops the vnc server and restarts it.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

sep
echo stopping vncserver...
sudo service vncserver stop
sep
echo after stopping vncserver
psa vnc.*Xauth.*geom
sep
echo starting vncserver...
sudo service vncserver start
sep
echo after starting vncserver
psa vnc.*Xauth.*geom
sep
