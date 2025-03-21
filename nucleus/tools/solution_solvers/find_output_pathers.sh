#!/usr/bin/env bash

# this script locates any project files that mention the output path setting (OutputPath).  
# these are extra suspicious due to the problems caused for our solutions when files have the
# OutputPath specified rather than allowing the parent's settings to be inherited.

# only works when repo dir is at the top of the full builds area.
# we need like a top dir of some sort.
find $FEISTY_MEOW_APEX -iname "*proj" -exec grep -l OutputPath {} ';' >~/outputpath_mentioners.txt
