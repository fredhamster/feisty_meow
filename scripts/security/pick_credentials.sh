#!/bin/bash

# chooses the right certificate to use for logging in via ssh.

#hmmm: not the slightest bit general here currently.
#      what about having a main key variable and a sourceforge key variable?
#      better yet, an array of site patterns and keys for those sites.

keyfile="$HOME/.ssh/id_dsa_fred"

if [ ! -z "$(echo $* | grep -i sourceforge)" ]; then
  keyfile="$HOME/.ssh/id_dsa_sourceforge"
fi

