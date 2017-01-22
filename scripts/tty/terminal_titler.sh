#!/bin/bash

source $FEISTY_MEOW_SCRIPTS/core/functions.sh

# just saying this is an array...
declare -a PRIOR_TERMINAL_TITLES
# set the stack position if not already set.
if [ -z "$PTT_STACK_INDEX" ]; then
  # this variable records where we will push new items on the stack.
  PTT_STACK_INDEX=0
fi

# adds an entry into the stack of terminal titles.
function push_ptt_stack()
{
  PRIOR_TERMINAL_TITLES[$PTT_STACK_INDEX]="$*"
echo now list has:
echo ${PRIOR_TERMINAL_TITLES[@]}
  ((PTT_STACK_INDEX++))
echo stack index incremented and now at $PTT_STACK_INDEX
}

function pop_ptt_stack()
{
  if [ $PTT_STACK_INDEX -le 0 ]; then
    echo nothing to pop from prior terminal titles stack.
  else
    ((PTT_STACK_INDEX--))
echo stack index decremented and now at $PTT_STACK_INDEX
    CURRENT_TERM_TITLE="${PRIOR_TERMINAL_TITLES[$PTT_STACK_INDEX]}"
  fi
}

# returns okay (0) if the stack is empty, or non-zero if not empty.
function ptt_stack_empty()
{
  test $PTT_STACK_INDEX -le 0
}

# puts a specific textual label on the terminal title bar.
# this doesn't consider any previous titles; it just labels the terminal.
# the title may not be visible in some window managers.
function set_terminal_title()
{
  title="$*"
  # if we weren't given a title, then use just the hostname.  this is the degraded functionality.
  if [ -z "${title}" ]; then
    title="$(hostname)"
  fi
  echo -n -e "\033]0;${title}\007"
}

# reads the current terminal title, if possible, and saves it to our stack of titles.
function save_terminal_title()
{
  # save the former terminal title if we're running in X with xterm.
  which xprop &>/dev/null
  if [ $? -eq 0 ]; then
    if [[ "$TERM" =~ .*"xterm".* ]]; then
      local prior_title="$(xprop -id $WINDOWID | perl -nle 'print $1 if /^WM_NAME.+= \"(.*)\"$/')"
echo "saving prior terminal title as '$prior_title'"
      push_ptt_stack "$prior_title"
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
  if ptt_stack_empty; then
echo prior titles were empty, so doing nothing.
  else
    pop_ptt_stack
echo "restoring prior terminal title of '$CURRENT_TERM_TITLE'"
    set_terminal_title "$CURRENT_TERM_TITLE"
  fi
}

# sets a title for the terminal with the hostname and other details.
function label_terminal_with_info()
{
  # we only label the terminal anew if there's no saved title.
#  if [ -z "$PRIOR_TERMINAL_TITLE" ]; then
  if ptt_stack_empty; then
    pruned_host=$(echo $HOSTNAME | sed -e 's/^\([^\.]*\)\..*$/\1/')
    date_string=$(date +"%Y %b %e @ %T")
    user=$USER
    if [ -z "$user" ]; then
      # try snagging the windoze name.
      user=$USERNAME
    fi
    new_title="-- $user@$pruned_host -- [$date_string]"
    set_terminal_title "$new_title"
  else
    # restore the former title.
    restore_terminal_title
  fi
}


