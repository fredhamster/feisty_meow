#!/bin/bash

##############

# Fred Hamster's Feisty Meow Concerns Ltd. Startup Profile.
#
# This file is useful within .profile or other initialization scripts.
#
# Author: Chris Koeritz

##############

# DEBUG_FEISTY_MEOW: if this variable is non-empty, then it causes the feisty meow
# scripts to print more diagnostic information when they run.  not all
# scripts support this, but the core ones do.

#export DEBUG_FEISTY_MEOW=true

##############

# this script cannot handle figuring out where it lives, so approaches that
# get the THISDIR will fail.  this is a consequence of this always being used
# in bash's 'source' directive, which does not pass the script name as
# argument 0.  instead, we just check for the bad condition of a malconfigured
# script system and try to repair it.

# we start out thinking things are good.
NO_REPAIRS_NEEDED=true

# check if any crucial folder is hosed.  we will torch the existing config
# to the extent we can.
if [ ! -d "$FEISTY_MEOW_APEX" ]; then
  # flag some problems.
  unset NO_REPAIRS_NEEDED
  # wipe out the offending variable(s).
  unset FEISTY_MEOW_SCRIPTS FEISTY_MEOW_APEX
  # clean out any unfortunate wrongness that may exist in our generated areas.
  if [ -d "$FEISTY_MEOW_LOADING_DOCK" ]; then \rm -rf "$FEISTY_MEOW_LOADING_DOCK"; fi
  if [ -d "$FEISTY_MEOW_GENERATED_STORE" ]; then \rm -rf "$FEISTY_MEOW_GENERATED_STORE"; fi
  # also wipe any values from the variables pointing at generated stuff.
  unset FEISTY_MEOW_LOADING_DOCK FEISTY_MEOW_GENERATED_STORE
  echo "

The feisty meow configuration is damaged somehow.  Please change to the
directory where it is stored, e.g.:

  cd /opt/feistymeow.org/feisty_meow

and run this command (the whole unwieldy multiple line chunk inside the bars):


##############
  exec bash -i 3<<EOF 4<&0 <&3
    echo -e '\n\n^^^ errors above here indicate potential problems in .bashrc ^^^';
    export FEISTY_MEOW_APEX=\"\$(pwd)\"; export FEISTY_MEOW_SCRIPTS=\$FEISTY_MEOW_APEX/scripts;
    export FEISTY_MEOW_SHOW_LAUNCH_GREETING=yes;
    /bin/bash \$(pwd)/scripts/core/reconfigure_feisty_meow.sh;
    source \$(pwd)/scripts/core/launch_feisty_meow.sh; exec 3>&- <&4
EOF
##############


This code snippet assumes that the .bashrc file could still need editing to
fix an erroneous FEISTY_MEOW_APEX variable, so we skip it above when bash
runs.  Check \$HOME/.bashrc to see if a change there will fix the problem.

"
else
  # apex is good, so let's make the scripts good too.
  if [ -z "$FEISTY_MEOW_SCRIPTS" -o ! -d "$FEISTY_MEOW_SCRIPTS" ]; then
    export FEISTY_MEOW_SCRIPTS="$FEISTY_MEOW_APEX/scripts"
  fi
  # check again to test our belief system...
  if [ ! -d "$FEISTY_MEOW_SCRIPTS" ]; then
    unset NO_REPAIRS_NEEDED
    echo -e "The feisty meow scripts cannot be found under the current top:\n  FEISTY_MEOW_APEX=$FEISTY_MEOW_APEX"
  fi
fi

if [ "$NO_REPAIRS_NEEDED" == "true" ]; then

  # we believe it's safe to run through the rest of this script.

  ##############
  
  # some preconditions we want to establish before loading anything...
  
  # make sure that aliases can be used in non-interactive shells.
  # this causes all aliases that are currently defined for this shell to
  # be inherited by subshells that this shell starts.  this is unusual,
  # but is preferred for my workflow in feisty meow scripts; it saves me
  # time re-adding aliases if i can count on them already being there.
  # this is a problem if you *don't* want the aliases there though.  we can
  # solve that problem by running bash with the "-O expand_aliases" flags to
  # stop the expansion for the next subshell.
  shopt -s expand_aliases
  
  # patch the user variable if we were launched by one of our cron jobs.
  if [ -z "$USER" -a ! -z "$CRONUSER" ]; then
    export USER="$CRONUSER"
  fi

  # use the xauth info if we were given one in the environment.
  # this allows root or other su'd identities to create windows with same
  # display variable.
  if [ ! -z "$DISPLAY" -a ! -z "$IMPORTED_XAUTH" ]; then
    xauth add $DISPLAY . $IMPORTED_XAUTH
  fi
  
  ##############
  
  export ERROR_OCCURRED=
    # there have been no errors to start with, at least.  we will set this
    # to non-empty if something bad happens.
  
  if [ -z "$FEISTY_MEOW_LOADING_DOCK" ]; then
    # FEISTY_MEOW_LOADING_DOCK is where the generated files are located.
    # this is our single entry point we can use without knowing any variables
    # yet in the initialization process.
    export FEISTY_MEOW_LOADING_DOCK="$HOME/.zz_feisty_loading"
  #hmmm: the above is kind of a constant.  that's not so great.
  
    # make sure our main variables are established.
    FEISTY_MEOW_VARIABLES_LOADING_FILE="$FEISTY_MEOW_LOADING_DOCK/fmc_variables.sh"
    if [ ! -f "$FEISTY_MEOW_VARIABLES_LOADING_FILE" ]; then
      echo -e "\
  
  The feisty meow scripts need initialization via the bootstrap process.  For\n\
  example, if the feisty meow folder lives in '$DEFAULT_FEISTYMEOW_ORG_DIR', then this\n\
  command bootstraps feisty meow:\n\
  \n\
    bash $example_dir/feisty_meow/scripts/core/reconfigure_feisty_meow.sh\n\
  \n\
  \n"
      ERROR_OCCURRED=true
    fi
  
    ##############

    if [ -z "$ERROR_OCCURRED" ]; then
      # pull in our generated variables that are the minimal set we need to find
      # the rest of our resources.
      source "$FEISTY_MEOW_VARIABLES_LOADING_FILE"
  
      # Set up the temporary directory.
      source "$FEISTY_MEOW_SCRIPTS/core/create_tempdir.sh"
    fi
  
  fi

  ##############

  if [ -z "$ERROR_OCCURRED" ]; then
    # no error occurred in our tests above, so load the larger body of standard feisty
    # meow variables into the environment.  we actually want this to always run also;
    # it will decide what variables need to be set again.
    source "$FEISTY_MEOW_SCRIPTS/core/variables.sh"

    ##############

    # include helpful functions.  we do this every time rather than making it part
    # of variable initialization, because functions cannot be exported to
    # sub-shells in bash.
    source "$FEISTY_MEOW_SCRIPTS/core/functions.sh"

    # load some helper methods for the terminal which we'll use below.
    source "$FEISTY_MEOW_SCRIPTS/tty/terminal_titler.sh"

    ##############

#hmmm: abstract this to a twiddle shell options method.
    # check hash table before searching path.
    shopt -s checkhash
    # don't check path for sourced files.
    shopt -u sourcepath
    # ignore duplicate lines.
    HISTCONTROL=ignoredups
    # append to the history file.
    shopt -s histappend
    # automatically update window size if needed.
    shopt -s checkwinsize

    ##############

    # make history writes immediate to avoid losing history if bash is zapped.
    echo $PROMPT_COMMAND | grep -q history
    if [ $? -ne 0 ]; then
      # we only change the prompt command if we think it hasn't already been done.
      export PROMPT_COMMAND="history -a;$PROMPT_COMMAND"
    fi

    ##############

    # perform the bulkier parts of the initialization process.

    if [ ! -z "$DEBUG_FEISTY_MEOW" ]; then echo "heavyweight init begins..."; fi

    # set up the aliases for the shell, but only if they are not already set.
    type CORE_ALIASES_LOADED &>/dev/null
    if [ $? -ne 0 ]; then
      if [ ! -z "$DEBUG_FEISTY_MEOW" ]; then
        echo "the aliases were missing, now they are being added..."
      fi
      source "$FEISTY_MEOW_LOADING_DOCK/fmc_core_and_custom_aliases.sh"
    fi

    #echo before the new labelling, terminal titles have:
    #show_terminal_titles

    # a minor tickle of the title of the terminal, unless we already have some history.
    label_terminal_with_info

    if [ ! -z "$DEBUG_FEISTY_MEOW" ]; then echo "heavyweight init is done."; fi

    if [ -z "$ERROR_OCCURRED" ]; then
      # set a sentinel variable to say we loaded the feisty meow environment.
      export FEISTY_MEOW_SCRIPTS_LOADED=true
    fi

  fi  # no error occurred.

  if [ ! -z "$FEISTY_MEOW_SHOW_LAUNCH_GREETING" ]; then
    echo
    echo
    echo "welcome to the feisty meow zone of peace, one of many refuges in the uncountably"
    echo "infinite multiverses that are hypothetically possible."
    echo
    echo
    unset FEISTY_MEOW_SHOW_LAUNCH_GREETING
  fi

  # only run this hello file if the core feisty meow support haven't been loaded already.  this
  # hopefully guarantees we show the info at most once in one shell continuum.
  # this can also be disabled if the NO_HELLO variable has a non-empty value.
  type CORE_VARIABLES_LOADED &>/dev/null
  if [ $? -ne 0 -a -z "$NO_HELLO" ]; then
    # print out a personalized hello file if we find one.
    if [ -f ~/hello.txt ]; then
      echo
      sep 28
      perl $FEISTY_MEOW_SCRIPTS/*/filedump.pl ~/hello.txt
      sep 28
      echo
    fi
    # from now on there should be no extra helloing.
    export NO_HELLO=true
  fi

  # load the last bits we do here.
  source "$FEISTY_MEOW_LOADING_DOCK/fmc_ending_sentinel.sh"

fi # "$NO_REPAIRS_NEEDED" was == "true" 

