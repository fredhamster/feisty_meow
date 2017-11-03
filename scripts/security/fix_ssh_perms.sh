#!/bin/bash

# resets the permissions on the .ssh directory to be safe.
# otherwise ssh may ignore keys and such from there.

chmod 700 $HOME/.ssh 
chmod 600 $HOME/.ssh/*


