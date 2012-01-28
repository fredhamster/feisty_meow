#!/bin/bash

# generates alias files for different operating systems and shell scripts.
# or really, mainly for bash these days, on linux.  but we also run under
# msys and cygwin to some degree.
#
# The "common.alias" file is used in the generated aliases file as a base
# set of generally useful aliases.  Shorter aliases based on any scripts
# we can find in the feisty meow script hierarchy are added in as well.
#
# If any other alias files are found in the scripts/custom folder, they
# are pulled in as additions and overrides for the basic feisty meow command
# set.

if [ ! -z "$SHELL_DEBUG" ]; then echo rebuiling generated aliases file...; fi

# create our generated shells directory if it's not already.
if [ ! -d $FEISTY_MEOW_GENERATED ]; then mkdir $FEISTY_MEOW_GENERATED; fi

# test if we can use color in ls...
test_color=$(ls --help 2>&1 | grep -i color)

ALIAS_DEFINITION_FILES=("$FEISTY_MEOW_SCRIPTS/core/common.alias")
# if custom aliases files exist, add them to the list.
for i in "$FEISTY_MEOW_SCRIPTS/custom/*.alias"; do
echo adding $i
  ALIAS_DEFINITION_FILES+=("$i")
done
echo "alias files:"
for i in "${ALIAS_DEFINITION_FILES[@]}"; do
  echo "  $(basename $(dirname $i))/$(basename $i)"
done

# write the aliases for sh and bash scripts.

GENERATED_ALIAS_FILE="$FEISTY_MEOW_GENERATED/aliases.sh"
echo "writing $GENERATED_ALIAS_FILE..."

#hmmm: perhaps a good place for a function to create the header,
#      given the appropriate comment code.

echo "##" >$GENERATED_ALIAS_FILE
echo "## generated file: $GENERATED_ALIAS_FILE" >>$GENERATED_ALIAS_FILE
echo "## please do not edit." >>$GENERATED_ALIAS_FILE
echo "##" >>$GENERATED_ALIAS_FILE

if [ ! -z "$test_color" ]; then
  echo "color_add=--color=auto" >>$GENERATED_ALIAS_FILE
else
  echo "color_add=" >>$GENERATED_ALIAS_FILE
fi

# plow in the full set of aliases into the file.
for i in "${ALIAS_DEFINITION_FILES[@]}"; do
  cat $i >>$GENERATED_ALIAS_FILE 
done

if [ ! -z "$SHELL_DEBUG" ]; then echo done rebuiling generated aliases file.; fi

