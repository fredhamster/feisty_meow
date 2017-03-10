#!/bin/bash

source "$FEISTY_MEOW_SCRIPTS/core/functions.sh"
source "$FEISTY_MEOW_SCRIPTS/core/common.alias"

#export PRIOR_TERMINAL_TITLE

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
  
  if [ "${TERM}" != "dumb" -a -z "$PBS_ENVIRONMENT" -a ! -z "$PS1" ]; then
    echo -n -e "\033]0;${title}\007"
  else
    # not running interactively, so just echo the title.
    sep
    echo ${title}
    sep
  fi
}

# user friendly version that saves the title being added.
function set_terminal_title()
{
  apply_title_to_terminal $*
  # an echo or some interaction with terminal seems needed to get terminal to respond
  # so we will see the current title when we query for it.
  echo
  save_terminal_title
}

# reads the current terminal title, if possible, and saves it to our record.
function save_terminal_title()
{
  # save the former terminal title if we're running in X with xterm.
  which xprop &>/dev/null
  if [ $? -eq 0 ]; then
    # make sure we're actually using xterm *and* that we have a window ID.
    if [[ "$TERM" =~ .*"xterm".* && ! -z "$WINDOWID" ]]; then
      local prior_title="$(xprop -id $WINDOWID | perl -nle 'print $1 if /^WM_NAME.+= \"(.*)\"$/')"
      if [ ! -z "$prior_title" ]; then
echo "saving prior terminal title as '$prior_title'"
        export PRIOR_TERMINAL_TITLE="$prior_title"
      else
echo "not saving prior terminal title which was empty"
      fi
    fi
  fi
}

# using our stored terminal title, this replaces the title on the terminal.
function restore_terminal_title()
{
# we don't want to emit anything extra if this is being driven by git.
#hmmm...  this could be a problem?
#  if [ -z "$(echo $* | grep git)" ]; then

  # run the terminal labeller to restore the prior title, if there was one.
  if [ ! -z "$PRIOR_TERMINAL_TITLE" ]; then
echo "restoring prior terminal title of '$PRIOR_TERMINAL_TITLE'"
    apply_title_to_terminal "$PRIOR_TERMINAL_TITLE"
  fi
}

# sets a title for the terminal with the hostname and other details.
function label_terminal_with_info()
{
  # we only label the terminal anew if there's no saved title.
  if [ -z "$PRIOR_TERMINAL_TITLE" ]; then
echo "showing new generated title since prior title was empty"
    pruned_host=$(echo $HOSTNAME | sed -e 's/^\([^\.]*\)\..*$/\1/')
    date_string=$(date +"%Y %b %e @ %T")
    user=$USER
    if [ -z "$user" ]; then
      # try snagging the windoze name.
      user=$USERNAME
    fi
    new_title="-- $user@$pruned_host -- [$date_string]"
    apply_title_to_terminal "$new_title"
#needing this extra echo is awful.  is there a better way?
    echo
    save_terminal_title
  else
    # use the former title; paste it back up there just in case.
echo "showing prior terminal title since there was a prior title!"
echo "using prior terminal title of '$PRIOR_TERMINAL_TITLE'"
    apply_title_to_terminal "$PRIOR_TERMINAL_TITLE"
  fi
}


