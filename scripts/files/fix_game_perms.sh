#!/bin/bash

# fixes the permissions of files in the local games directory so that
# everyone can run or read them if the owner could.

find /home/games -type f -perm /100 -exec chmod a+x {} ';'
find /home/games -type f -perm /400 -exec chmod a+r {} ';'

find /home/games -type d -exec chmod a+rx {} ';'

