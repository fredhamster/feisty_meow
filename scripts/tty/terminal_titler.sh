#!/bin/bash

source "$FEISTY_MEOW_SCRIPTS/core/functions.sh"
source "$FEISTY_MEOW_SCRIPTS/core/common.alias"

# uncomment this to get extra noisy debugging.
#export DEBUG_TERM_TITLE=true

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
  
  if [ "${TERM}" != "dumb" -a -z "$PBS_ENVIRONMENT" -a \
        ! -z "$PS1" -a "${TERM}" != "linux" ]; then
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

#tricky attempts to get it to be available when we ask for it in get_terminal_title
  sync
#  echo -n

#  # we're enforcing a new title from here on.
#  unset PRIOR_TERMINAL_TITLE
  save_terminal_title
}

# echoes back the current title on the terminal window, if we can acquire it.
function get_terminal_title()
{
#hmmm: totally failing since gnome doesn't provide the info we need like it should; WINDOWID is empty.
#hmmm: need to revise this somehow to work around that, but nothing is quite right so far.
#hmmm: had to disable the xwininfo approach because it's super messy (console noise) and still isn't right (just like xdotool approach wasn't right).

  # this is an important value now; it is checked for in save_terminal_title.
  local term_title_found="unknown"
  # save the former terminal title if we're running in X with xterm.

  # are we even running a terminal?
#hmmm: abstract out that condition below to check against a list of names.
  if [[ "$TERM" =~ .*"xterm".* ]]; then
    # check that we have a window ID.
    if [[ ! -z "$WINDOWID" ]]; then
      # the window id exists in the variable; can we get its info?
      if [[ ! -z "$(which xprop)" ]]; then
        # sweet, we have all the info we need to get this done.
        term_title_found="$(xprop -id $WINDOWID | perl -nle 'print $1 if /^WM_NAME.+= \"(.*)\"$/')"
      fi
    else
      # gnome-terminal doesn't set WINDOWID currently; we can try to work around this.
      false
#hmmm: so far, none of these approaches are any good.
#      # not good solution; gets wrong titles.  plus uses xdotool which is not installed by default in ubuntu.
#      if [[ ! -z "$(which xdotool)" ]]; then
#        term_title_found="$(xprop -id $(xdotool getactivewindow) | perl -nle 'print $1 if /^WM_NAME.+= \"(.*)\"$/')"
#      fi
#      # this solution also fails by getting the wrong window title if this one isn't focussed.
#      if [[ ! -z "$(which xwininfo)" ]]; then
#        term_title_found=$(xwininfo -id $(xprop -root | awk '/NET_ACTIVE_WINDOW/ { print $5; exit }') | awk -F\" '/xwininfo:/ { print $2; exit }')
#      fi
    fi
  fi
  echo -n "$term_title_found"
}

# reads the current terminal title, if possible, and saves it to our record.
function save_terminal_title()
{
  local title="$(get_terminal_title)"
  if [ "$title" != "unknown" ]; then
    # there was a title, so save it.
    if [ ! -z "$DEBUG_TERM_TITLE" ]; then
      echo "saving prior terminal title as '$title'"
    fi
    export PRIOR_TERMINAL_TITLE="$title"
  else
    # the terminal had no title, or we couldn't access it, or there's no terminal.
    if [ ! -z "$DEBUG_TERM_TITLE" ]; then
      echo "not saving prior terminal title which was empty"
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
    if [ ! -z "$DEBUG_TERM_TITLE" ]; then
      echo "restoring prior terminal title of '$PRIOR_TERMINAL_TITLE'"
    fi
    apply_title_to_terminal "$PRIOR_TERMINAL_TITLE"
  fi
}

# sets a title for the terminal with the hostname and other details.
function label_terminal_with_info()
{
  # we only label the terminal anew if there's no saved title.
  if [ -z "$PRIOR_TERMINAL_TITLE" ]; then
    if [ ! -z "$DEBUG_TERM_TITLE" ]; then
      echo "showing new generated title since prior title was empty"
    fi
    pruned_host=$(echo $HOSTNAME | sed -e 's/^\([^\.]*\)\..*$/\1/')
    date_string=$(date +"%Y %b %e @ %T")
    user="$(logname)"
    if [ -z "$user" ]; then
      # try snagging the windoze name.
      user=$USERNAME
    fi
    new_title="-- $user@$pruned_host -- [$date_string]"
    apply_title_to_terminal "$new_title"
    save_terminal_title
  else
    # use the former title; paste it back up there just in case.
    if [ ! -z "$DEBUG_TERM_TITLE" ]; then
      echo "showing prior terminal title since there was a prior title!"
      echo "using prior terminal title of '$PRIOR_TERMINAL_TITLE'"
    fi
    apply_title_to_terminal "$PRIOR_TERMINAL_TITLE"
  fi
}


