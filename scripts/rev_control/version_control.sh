#!/bin/bash

# these are helper functions for doing localized revision control.
# this script should be sourced into other scripts that use it.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"
source "$FEISTY_MEOW_SCRIPTS/tty/terminal_titler.sh"

##############

# the maximum depth that the recursive functions will try to go below the starting directory.
export MAX_DEPTH=5

# one unpleasantry to take care of first; cygwin barfs aggressively if the TMP directory
# is a DOS path, but we need it to be a DOS path for our GFFS testing, so that blows.
# to get past this, TMP gets changed below to a hopefully generic and safe place.
if [[ "$TMP" =~ .:.* ]]; then
  echo "making weirdo temporary directory for PCDOS-style path."
  export TMP=/tmp/rev_control_$USER
fi
if [ ! -d "$TMP" ]; then
  mkdir -p $TMP
fi
if [ ! -d "$TMP" ]; then
  echo "could not create the temporary directory TMP in: $TMP"
  echo "this script will not work properly without an existing TMP directory."
fi
#hmmm: re-address the above code, since it doesn't make a lot of sense to me right now...


##############

# checks the directory provided into the revision control system repository it belongs to.
function do_checkin()
{
  local directory="$1"; shift

  save_terminal_title

  # make a nice echoer since we want to use it inside conditions below.
  local nicedir="$directory"
  if [ $nicedir == "." ]; then
    nicedir=$(\pwd)
  fi
  local blatt="echo checking in '$nicedir'..."

  local retval=0  # normally successful.

  do_update "$directory"
  retval=$?
  test_or_die "repository update failed; this should be fixed before check-in."

  pushd "$directory" &>/dev/null
  if [ -f ".no-checkin" ]; then
    echo "skipping check-in due to presence of .no-checkin sentinel file."
  elif [ -d "CVS" ]; then
    if test_writeable "CVS"; then
      $blatt
      cvs ci .
      retval=$?
    fi
  elif [ -d ".svn" ]; then
    if test_writeable ".svn"; then
      $blatt
      svn ci .
      retval=$?
    fi
  elif [ -d ".git" ]; then
    if test_writeable ".git"; then
      $blatt
      # snag all new files.  not to everyone's liking.
      git add --all .
      retval=$?

      # see if there are any changes in the local repository.
      if ! git diff-index --quiet HEAD --; then
        # tell git about all the files and get a check-in comment.
        git commit .
        retval+=$?
      fi
      # catch if the diff-index failed somehow.
      retval+=$?

#push the changes to where?  locally?
      git push 2>&1 | grep -v "X11 forwarding request failed" | squash_first_few_crs
      retval+=${PIPESTATUS[0]}

      # upload any changes to the upstream repo so others can see them.
      if [ "$(git_branch_name)" != "master" ]; then
        git push origin "$(git_branch_name)" 2>&1 | grep -v "X11 forwarding request failed" | squash_first_few_crs
        retval+=${PIPESTATUS[0]}
      fi

    fi
  else
    # nothing there.  it's not an error though.
    echo no repository in $directory
    retval=0
  fi
  popd &>/dev/null

  restore_terminal_title

  return $retval
}

# shows the local changes in a repository.
function do_diff
{
  local directory="$1"; shift

  save_terminal_title

  pushd "$directory" &>/dev/null
  local retval=0  # normally successful.

  # only update if we see a repository living there.
  if [ -d ".svn" ]; then
    svn diff .
    retval+=$?
  elif [ -d ".git" ]; then
    git diff 
    retval+=$?
  elif [ -d "CVS" ]; then
    cvs diff .
    retval+=$?
  fi

  popd &>/dev/null

  restore_terminal_title

  return $retval
}

# reports any files that are not already known to the upstream repository.
function do_report_new
{
  local directory="$1"; shift

  save_terminal_title

  pushd "$directory" &>/dev/null
  local retval=0  # normally successful.

  # only update if we see a repository living there.
  if [ -f ".no-checkin" ]; then
    echo "skipping reporting due to presence of .no-checkin sentinel file."
  elif [ -d ".svn" ]; then
    # this action so far only makes sense and is needed for svn.
    bash $FEISTY_MEOW_SCRIPTS/rev_control/svnapply.sh \? echo
    retval=$?
  elif [ -d ".git" ]; then
    git status -u
    retval=$?
  fi

  popd &>/dev/null

  restore_terminal_title

  return $retval
}

# checks in all the folders in a specified list.
function checkin_list()
{
  # make the list of directories unique.
  local list="$(uniquify $*)"

  save_terminal_title

  # turn repo list back into an array.
  eval "repository_list=( ${REPOSITORY_LIST[*]} )"

  local outer inner

  for outer in "${repository_list[@]}"; do
    # check the repository first, since it might be an absolute path.
    if [[ $outer =~ /.* ]]; then
      # yep, this path is absolute.  just handle it directly.
      if [ ! -d "$outer" ]; then continue; fi
      do_checkin $outer
      test_or_die "running check-in on: $outer"
      sep 28
    else
      for inner in $list; do
        # add in the directory component to see if we can find the folder.
        local path="$inner/$outer"
        if [ ! -d "$path" ]; then continue; fi
        do_checkin $path
        test_or_die "running check-in on: $path"
        sep 28
      done
    fi
  done

  restore_terminal_title
}

# takes out the first few carriage returns that are in the input.
function squash_first_few_crs()
{
  i=0
  while read input_text; do
    i=$((i+1))
    if [ $i -le 5 ]; then
      echo -n "$input_text  "
    else
      echo $input_text
    fi
  done
  if [ $i -le 3 ]; then
    # if we're still squashing eols, make sure we don't leave them hanging.
    echo
  fi
}

# a helpful method that reports the git branch for the current directory's
# git repository.
function git_branch_name()
{
  echo "$(git branch | grep \* | cut -d ' ' -f2-)"
}

# gets the latest versions of the assets from the upstream repository.
function do_update()
{
  directory="$1"; shift

  save_terminal_title

  # make a nice echoer since we want to use it inside conditions below.
  local nicedir="$directory"
  if [ $nicedir == "." ]; then
    nicedir=$(\pwd)
  fi
  local blatt="echo retrieving '$nicedir'..."

  local retval=0  # plan on success for now.
  pushd "$directory" &>/dev/null
  if [ -d "CVS" ]; then
    if test_writeable "CVS"; then
      $blatt
      cvs update . | squash_first_few_crs
      retval=${PIPESTATUS[0]}
    fi
  elif [ -d ".svn" ]; then
    if test_writeable ".svn"; then
      $blatt
      svn update . | squash_first_few_crs
      retval=${PIPESTATUS[0]}
    fi
  elif [ -d ".git" ]; then
    if test_writeable ".git"; then
      $blatt
      retval=0

      if [ "$(git_branch_name)" != "master" ]; then
        git pull origin master 2>&1 | grep -v "X11 forwarding request failed" | squash_first_few_crs
        retval+=${PIPESTATUS[0]}
      fi

      git pull 2>&1 | grep -v "X11 forwarding request failed" | squash_first_few_crs
      retval+=${PIPESTATUS[0]}
    fi
  else
    # this is not an error necessarily; we'll just pretend they planned this.
    echo no repository in $directory
  fi
  popd &>/dev/null

  restore_terminal_title

  return $retval
}

# gets all the updates for a list of folders under revision control.
function checkout_list()
{
  local list="$(uniquify $*)"

  save_terminal_title

  # turn repo list back into an array.
  eval "repository_list=( ${REPOSITORY_LIST[*]} )"

  local outer inner

  for outer in "${repository_list[@]}"; do
    # check the repository first, since it might be an absolute path.
    if [[ $outer =~ /.* ]]; then
      # yep, this path is absolute.  just handle it directly.
      if [ ! -d "$outer" ]; then continue; fi
      do_update $outer
      test_or_die "running update on: $path"
      sep 28
    else
      for inner in $list; do
        # add in the directory component to see if we can find the folder.
        local path="$inner/$outer"
        if [ ! -d "$path" ]; then continue; fi
        do_update $path
        test_or_die "running update on: $path"
        sep 28
      done
    fi
  done

  restore_terminal_title
}

# provides a list of absolute paths of revision control directories
# that are located under the directory passed as the first parameter.
function generate_rev_ctrl_filelist()
{
  local dir="$1"; shift
  pushd "$dir" &>/dev/null
  local dirhere="$( \cd "$(\dirname "$dir")" && /bin/pwd )"
  local tempfile=$(mktemp /tmp/zz_checkins.XXXXXX)
  echo >$tempfile
  local additional_filter
  find $dirhere -follow -maxdepth $MAX_DEPTH -type d -iname ".svn" -exec echo {}/.. ';' >>$tempfile 2>/dev/null
  find $dirhere -follow -maxdepth $MAX_DEPTH -type d -iname ".git" -exec echo {}/.. ';' >>$tempfile 2>/dev/null
  # CVS is not well behaved like git and (now) svn, and we seldom use it anymore.
  popd &>/dev/null

  # see if they've warned us not to try checking in within vendor hierarchies.
  if [ ! -z "NO_CHECKIN_VENDOR" ]; then
    sed -i -e '/.*\/vendor\/.*/d' "$tempfile"
  fi

  local sortfile=$(mktemp /tmp/zz_checkin_sort.XXXXXX)
  sort <"$tempfile" >"$sortfile"
  \rm "$tempfile"
  echo "$sortfile"
}

# iterates across a list of directories contained in a file (first parameter).
# on each directory name, it performs the action (second parameter) provided.
function perform_revctrl_action_on_file()
{
  local tempfile="$1"; shift
  local action="$1"; shift

  save_terminal_title

  while read -u 3 dirname; do
    if [ -z "$dirname" ]; then continue; fi
    pushd "$dirname" &>/dev/null
    echo "[$(pwd)]"
    $action .
    test_or_die "performing action $action on: $(pwd)"
    sep 28
    popd &>/dev/null
  done 3<"$tempfile"

  restore_terminal_title

  rm $tempfile
}


