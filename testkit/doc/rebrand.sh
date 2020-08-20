#!/bin/bash

# makes a new copy of the testkit reference with branding info installed.

echo "Rebranding the TestKit reference for: $TESTKIT_BRANDING"

# pull in our testkit config.
source ../prepare_tools.sh ../prepare_tools.sh 
source $TESTKIT_ROOT/library/process_configuration.sh
define_and_export_variables

# just a search and replace.  the source doc had better still have "$BRANDING" tags in it.
sed -e "s/\$BRANDING/$TESTKIT_BRANDING/g" \
  < testkit_reference-source.html \
  > testkit_reference.html


