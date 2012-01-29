#!/bin/bash

# generates alias files for bash.
#
# The "common.alias" file is used in the generated aliases file as a base
# set of generally useful aliases.  We also add aliases for any script files
# (perl, bash, python, etc) that we find in the feisty meow script hierarchy.
# Any *.alias files found in the scripts/custom folder are loaded also.

if [ ! -z "$SHELL_DEBUG" ]; then echo rebuiling generated aliases file...; fi

# create our generated shells directory if it's not already.
if [ ! -d $FEISTY_MEOW_GENERATED ]; then mkdir $FEISTY_MEOW_GENERATED; fi

# test if we can use color in ls...
test_color=$(ls --help 2>&1 | grep -i color)

# the main one is our common alias set.
ALIAS_DEFINITION_FILES=("$FEISTY_MEOW_SCRIPTS/core/common.alias")

# if custom aliases files exist, add them to the list.
for i in "$FEISTY_MEOW_SCRIPTS/custom/*.alias"; do
  ALIAS_DEFINITION_FILES+=("$i")
done
echo "alias files:"
for i in "${ALIAS_DEFINITION_FILES[@]}"; do
  echo "  $(basename $(dirname $i))/$(basename $i)"
done

# write the aliases for sh and bash scripts.

GENERATED_ALIAS_FILE="$FEISTY_MEOW_GENERATED/aliases.sh"
echo "writing generated aliases in $GENERATED_ALIAS_FILE..."

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

