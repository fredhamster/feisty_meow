#!/bin/bash

source $FEISTY_MEOW_SCRIPTS/core/functions.sh

# just saying this is an array...
#declare -a PRIOR_TERMINAL_TITLES

# set the stack position if not already set.
if [ -z "$PTT_STACK_INDEX" ]; then
  # this variable records where we will push new items on the stack.
  PTT_STACK_INDEX=0
fi

# returns okay (0) if the stack is empty, or non-zero if not empty.
function ptt_stack_empty()
{
  if [ -z "$PTT_STACK_INDEX" ]; then
    # fix the index value.
    PTT_STACK_INDEX=0
    true
  else
    test $PTT_STACK_INDEX -le 0
  fi
}

# a debugging function that should never have been necessary.
# a little bit furious the restore is failing during regenerate right now.
function show_terminal_titles()
{
  sep 14
  echo "[terminal title list now has...]"
  local i=${#PRIOR_TERMINAL_TITLES[@]}
  if ptt_stack_empty; then
    echo the terminal title list is empty.
  else
    while ((i--)); do
      echo "ent #$i: '${PRIOR_TERMINAL_TITLES[$i]}'"
    done
  fi
  sep 14
}

# adds an entry into the stack of terminal titles.
function push_ptt_stack()
{
  PRIOR_TERMINAL_TITLES[$PTT_STACK_INDEX]="$*"
  ((PTT_STACK_INDEX++))
#echo stack index incremented and now at $PTT_STACK_INDEX
#show_terminal_titles
}

function pop_ptt_stack()
{
  if [ $PTT_STACK_INDEX -le 0 ]; then
    echo nothing to pop from prior terminal titles stack.
  else
    ((PTT_STACK_INDEX--))
#echo stack index decremented and now at $PTT_STACK_INDEX
    CURRENT_TERM_TITLE="${PRIOR_TERMINAL_TITLES[$PTT_STACK_INDEX]}"
#echo "got the last title as '$CURRENT_TERM_TITLE'"
#show_terminal_titles
  fi
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
    # make sure we're actually using xterm *and* that we have a window ID.
    if [[ "$TERM" =~ .*"xterm".* && ! -z "$WINDOWID" ]]; then
      local prior_title="$(xprop -id $WINDOWID | perl -nle 'print $1 if /^WM_NAME.+= \"(.*)\"$/')"
      if [ ! -z "$prior_title" ]; then
#echo "saving prior terminal title as '$prior_title'"
        push_ptt_stack "$prior_title"
#      else
#echo "not saving prior terminal title which was empty"
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
  if ! ptt_stack_empty; then
    pop_ptt_stack
#echo "restoring prior terminal title of '$CURRENT_TERM_TITLE'"
    set_terminal_title "$CURRENT_TERM_TITLE"
  fi
}

# sets a title for the terminal with the hostname and other details.
function label_terminal_with_info()
{
  # we only label the terminal anew if there's no saved title.
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
    # use the former title; paste it back up there just in case.
#echo "showing prior terminal title since there was a prior title!"
    pop_ptt_stack
#echo "using prior terminal title of '$CURRENT_TERM_TITLE'"
    set_terminal_title "$CURRENT_TERM_TITLE"
    push_ptt_stack "$CURRENT_TERM_TITLE"
  fi
}


