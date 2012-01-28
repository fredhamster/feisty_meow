#!/bin/bash

# generates alias files for different operating systems and shell scripts.
#
# This script generates command alias definitions for a variety of different
# shells.  The "common.alias" file is used in all of the generated
# files as a base set of generally useful aliases.  Then the appropriate
# shell's specific aliases are added in; these are named after the shell that
# they support, e.g. "sh_aliases.txt" is used with the sh and bash shells.
# The third component of the file is a set of aliases generated automatically
# from the names of all the shell scripts in SHELLDIR.
#
# Additionally, if a file named "c_common_aliases.txt" is found in the
# $SHELLDIR/custom directory, then it is added in after the other alias files
# and can override any existing aliases or provide additional local aliases.
# This file is not included with the YETIcode scripts, since it is intended
# to be provided by individual users.
#
# Further, if any other shell-specific alias files are found mirrored under
# the custom folder, they are pulled in as overrides for that kind of shell.
# For example, there is a sh_aliases.txt in the scripts directory for sh,
# but if there's also scripts/custom/c_sh_aliases.txt, then that will be
# plugged in as well.

# create our generated shells directory if it's not already.
if [ ! -d $GENERADIR ]; then mkdir $GENERADIR; fi

# test if we can use color in ls...
test_color=$(ls --help 2>&1 | grep -i color)

export COMMON_FILES=$SHELLDIR/core/common_aliases.txt
# if custom aliases files exist, add them to the list.
for i in "$SHELLDIR/custom/*.txt"; do
  COMMON_FILES+=" $i"
done
echo -e "Found alias files:\n$COMMON_FILES"

# write the aliases for sh and bash scripts.

export ALIASES_FILE="$GENERADIR/aliases.sh"
echo "writing $ALIASES_FILE..."

#hmmm: perhaps a good place for a function to create the header,
#      given the appropriate comment code.

echo "##" >$ALIASES_FILE
echo "## generated file: $ALIASES_FILE" >>$ALIASES_FILE
echo "## please do not edit." >>$ALIASES_FILE
echo "##" >>$ALIASES_FILE

if [ ! -z "$test_color" ]; then
  echo "color_add=--color=auto" >>$ALIASES_FILE
else
  echo "color_add=" >>$ALIASES_FILE
fi

# we process the alias file to add the word "alias" to the first of every
# line that's not a comment.
#cat $COMMON_FILES | sed -e 's/^\([^#]\)/alias \1/' >>$ALIASES_FILE 
#nope: we no longer do that.

cat $COMMON_FILES >>$ALIASES_FILE 

#echo "##" >>$ALIASES_FILE
#echo "## now including shell specific additions..." >>$ALIASES_FILE
#echo "##" >>$ALIASES_FILE
## then just dump the sh specific alias stuff into the file.
#cat $SHELLDIR/core/sh_aliases.txt >>$ALIASES_FILE 
# add in customized sh aliases if they exist.
#if [ -f "$SHELLDIR/custom/c_sh_aliases.txt" ]; then
#  cat $SHELLDIR/custom/c_sh_aliases.txt >>$ALIASES_FILE 
#fi


