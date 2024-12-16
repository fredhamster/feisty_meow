#!/usr/bin/env bash

# Processes the XSEDE tools config file to turn variables listed in the
# file into exported variables in the environment.
#
# Author: Chris Koeritz

##############

# this processes the single file of input parameters at the test root and
# turns it into a collection of environment variables.  we then load all those
# variables into the current environment.  we will also automatically fill in
# some important variables here that we'll use later.  in some cases, we will
# use the existing value if the variable is already set.
define_and_export_variables()
{
  if [ -z "$TESTKIT_SENTINEL" ]; then echo Please run prepare_tools.sh before testing.; return 3; fi

  # create our output folder so we can store logs and temporaries.
  mkdir -p "$TEST_TEMP" &>/dev/null

  # start writing the environment file.
  echo > $TEST_TEMP/env_file

  # turn each useful line in input file into an environment variable declaration.
  while read line; do
    # match lines that are comments or blank.
    echo "$line" | grep -e '^[#;]' -e '^[ 	]*$' &>/dev/null
    # only export non-useless lines.
    if [ $? != 0 ]; then
      echo "$line" | grep '[a-z0-9A-Z]=(' &>/dev/null
      if [ $? == 0 ]; then
        # it's an array variable so don't try to export it or bash loses it for us.
        echo $line >> $TEST_TEMP/env_file
      else
        echo "export" $line >> $TEST_TEMP/env_file
      fi
    fi
  done < "$TESTKIT_CFG_FILE"

  # now run the environment file to add those settings to our environment.
  chmod +x $TEST_TEMP/env_file
  source $TEST_TEMP/env_file &>/dev/null
}

