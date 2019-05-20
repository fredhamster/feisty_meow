#!/bin/bash

# locates read only files in the directories specified.
# this is a much simpler and more efficient command than some i've seen
# floating about; it gets the find command to do all the heavy lifting
# and doesn't try to build impossibly large command lines.

find "${@}" -follow -type f -perm -u+r -a ! -perm -u+w 

