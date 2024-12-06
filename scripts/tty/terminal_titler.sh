#!/bin/bash

source "$FEISTY_MEOW_SCRIPTS/core/functions.sh"
source "$FEISTY_MEOW_SCRIPTS/core/common.alias"

# uncomment this to get extra noisy debugging.
#export DEBUG_TERM_TITLE=true

##############

# puts a specific textual label on the terminal title bar.
# this doesn't consider any previous titles; it just labels the terminal.
# the title may not be visible in some window managers.
function apply_title_to_terminal()
{
  title="$*"
  # if we weren't given a title, then use just the hostname.  this is the degraded functionality.
  if [ -z "${title}" ]; then
    title="$(hostname)"
  fi
  
  # first check if we're in an interactive shell, according to bash voodoo.
  if [[ $- == *i* ]]; then
    if [ "${TERM}" != "dumb" \
        -a -z "$PBS_ENVIRONMENT" \
        -a ! -z "$PS1" \
        -a "${TERM}" != "linux" \
      ]; then
      echo -n -e "\033]0;${title}\007"
###  else
###    # not running interactively, so just echo the title.
#hmmm: oh hell no!  what were you thinking?  so non-interactive scripts get all sorts of terminal label goo injected into them?
#      what a freaking nightmare.  of course this is not okay to do!!!
###    sep
###    echo "${title}"
###    sep
    fi
  fi
}

##############

# records the current terminal title, pushing it down on the stack of titles,
# possibly prior to a new one being used.
function save_terminal_title()
{
  local title="$*"

  if [ -z "$title" ]; then
    if [ ! -z "$DEBUG_TERM_TITLE" ]; then
      echo "terminal_titler: empty title: pushing current title again"
    fi
    peek_title_stack
    title="$LAST_TITLE"
    if [ -z "$title" ]; then
      log_feisty_meow_event "terminal_titler: there was no saved title, so we're ignoring the save attempt."
      return 1
    fi
  fi

#hmmm: need a validation step here to remove bad chars that conflict with our title compression scheme.

  # only slap a comma after the existing value if it wasn't empty.
  if [ -z "$TERMINAL_TITLE_STACK" ]; then
    export TERMINAL_TITLE_STACK="\"$title\""
  else
    export TERMINAL_TITLE_STACK="$TERMINAL_TITLE_STACK,\"$title\""
  fi

  if [ ! -z "$DEBUG_TERM_TITLE" ]; then
    echo "terminal_titler: new terminal title stack is:"
    echo "$TERMINAL_TITLE_STACK"
  fi
}

# takes a terminal title off the stack and sets the LAST_TITLE variable.
function pop_title_stack()
{
  # whack our output variable, just in case.
  unset LAST_TITLE
  # get our stack top.
  peek_title_stack
  # trim the popped item out of the stack.
  if [ ! -z "$TERMINAL_TITLE_STACK" ]; then
    TERMINAL_TITLE_STACK="$(echo $TERMINAL_TITLE_STACK | sed -n -e 's/\(.*\),[^,]*$/\1/p')"
  fi
}

# like pop, but does not change the stack, effectively handing you the most
# recently set title.
function peek_title_stack()
{
  # whack our output variable, just in case.
  unset LAST_TITLE

  if [ ! -z "$TERMINAL_TITLE_STACK" ]; then
    LAST_TITLE="$(echo $TERMINAL_TITLE_STACK | sed -n -e 's/.*","\([^,]*\)"$/\1/p')"
    if [ -z "$LAST_TITLE" ]; then
      LAST_TITLE="$(echo $TERMINAL_TITLE_STACK | sed -n -e 's/"//gp')"
    fi
  fi
}

# using our stored terminal title, this replaces the title on the terminal.
function restore_terminal_title()
{
  # run the terminal labeller to restore the prior title, if there was one.
  pop_title_stack

  if [ ! -z "$LAST_TITLE" ]; then
    if [ ! -z "$DEBUG_TERM_TITLE" ]; then
      echo "terminal_titler: restoring prior terminal title of '$LAST_TITLE'"
      echo "terminal_titler: while new title stack is: $TERMINAL_TITLE_STACK"
    fi
    apply_title_to_terminal "$LAST_TITLE"
  fi
}

# sets a title for the terminal with the hostname and other details.
function label_terminal_with_info()
{
  # we only label the terminal anew if there's no saved title.
  if [ -z "$TERMINAL_TITLE_STACK" ]; then
    if [ ! -z "$DEBUG_TERM_TITLE" ]; then
      echo "terminal_titler: showing new generated title since prior title was empty"
    fi
    pruned_host=$(echo $HOSTNAME | sed -e 's/^\([^\.]*\)\..*$/\1/')
    date_string=$(date +"%Y %b %e @ %T")
    user="$(fm_username)"
    if [ -z "$user" ]; then
      # try snagging the windoze name.
      user=$USERNAME
    fi
    new_title="-- $user@$pruned_host -- [$date_string]"
    apply_title_to_terminal "$new_title"
    save_terminal_title "$new_title"
  else
    # use the former title; paste it back up there just in case.
    peek_title_stack
    apply_title_to_terminal "$LAST_TITLE"
  fi
}

##############

# user friendly version sets the terminal title and saves the title being added.
function set_terminal_title()
{
  apply_title_to_terminal $*
  save_terminal_title $*
}

##############


