#!/bin/bash

# fixes the permissions of files in the local games directory so that
# everyone can run or read them if the owner could.

sudo find /home/games -type f -perm /100 -exec chmod a+x {} ';'
sudo find /home/games -type f -perm /400 -exec chmod a+r {} ';'

sudo find /home/games -type d -exec chmod a+rx {} ';'

