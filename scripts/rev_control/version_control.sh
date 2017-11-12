#!/bin/bash

# these are helper functions for doing localized revision control.
# this script should be sourced into other scripts that use it.

# Author: Chris Koeritz
# Author: Kevin Wentworth

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"
source "$FEISTY_MEOW_SCRIPTS/tty/terminal_titler.sh"

##############

# the maximum depth that the recursive functions will try to go below the starting directory.
export MAX_DEPTH=5

# use our splitter tool for lengthy output if it's available.
if [ ! -z "$(which splitter)" ]; then
  TO_SPLITTER="$(which splitter)"
else
  TO_SPLITTER=cat
fi

##############

#hmmm: move this to core
# this makes the status of pipe N into the main return value.
function promote_pipe_return()
{
  ( exit ${PIPESTATUS[$1]} )
}

##############

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

  do_update "$directory"
  test_or_die "repository update--this should be fixed before check-in."

  pushd "$directory" &>/dev/null
  if [ -f ".no-checkin" ]; then
    echo "skipping check-in due to presence of .no-checkin sentinel file."
  elif [ -d "CVS" ]; then
    if test_writeable "CVS"; then
      $blatt
      cvs ci .
      test_or_die "cvs checkin"
    fi
  elif [ -d ".svn" ]; then
    if test_writeable ".svn"; then
      $blatt
      svn ci .
      test_or_die "svn checkin"
    fi
  elif [ -d ".git" ]; then
    if test_writeable ".git"; then
      $blatt

      # put all changed and new files in the commit.  not to everyone's liking.
      git add --all . | $TO_SPLITTER
      promote_pipe_return 0
      test_or_die "git add all new files"

      # see if there are any changes in the local repository.
      if ! git diff-index --quiet HEAD --; then
        # tell git about all the files and get a check-in comment.
        git commit .
        test_or_die "git commit"
      fi

      # a new set of steps we have to take to make sure the branch integrity is good.
      do_careful_git_update "$(\pwd)"

      # we continue on to the push, even if there were no changes this time, because
      # there could already be committed changes that haven't been pushed yet.

      # upload any changes to the upstream repo so others can see them.
      git push origin "$(my_branch_name)" 2>&1 | grep -v "X11 forwarding request failed" | $TO_SPLITTER
      promote_pipe_return 0
      test_or_die "git push"

    fi
  else
    # nothing there.  it's not an error though.
    echo no repository in $directory
  fi
  popd &>/dev/null

  restore_terminal_title

  return 0
}

# shows the local changes in a repository.
function do_diff
{
  local directory="$1"; shift

  save_terminal_title

  pushd "$directory" &>/dev/null

  # only update if we see a repository living there.
  if [ -d ".svn" ]; then
    svn diff .
    test_or_die "subversion diff"
  elif [ -d ".git" ]; then
    git diff 
    test_or_die "git diff"
  elif [ -d "CVS" ]; then
    cvs diff .
    test_or_die "cvs diff"
  fi

  popd &>/dev/null

  restore_terminal_title

  return 0
}

# reports any files that are not already known to the upstream repository.
function do_report_new
{
  local directory="$1"; shift

  save_terminal_title

  pushd "$directory" &>/dev/null

  # only update if we see a repository living there.
  if [ -f ".no-checkin" ]; then
    echo "skipping reporting due to presence of .no-checkin sentinel file."
  elif [ -d ".svn" ]; then
    # this action so far only makes sense and is needed for svn.
    bash $FEISTY_MEOW_SCRIPTS/rev_control/svnapply.sh \? echo
    test_or_die "svn diff"
  elif [ -d ".git" ]; then
    git status -u
    test_or_die "git status -u"
  fi

  popd &>/dev/null

  restore_terminal_title

  return 0
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
      test_or_die "running check-in (absolute) on path: $outer"
      sep 28
    else
      for inner in $list; do
        # add in the directory component to see if we can find the folder.
        local path="$inner/$outer"
        if [ ! -d "$path" ]; then continue; fi
        do_checkin $path
        test_or_die "running check-in (relative) on path: $path"
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

#hmmm: the below are git specific and should be named that way.

function all_branch_names()
{
  echo "$(git branch -vv | cut -d ' ' -f2)"
}

# a helpful method that reports the git branch for the current directory's
# git repository.
function my_branch_name()
{
  echo "$(git branch -vv | grep '\*' | cut -d ' ' -f2)"
}

#this had a -> in it at one point for not matching, didn't it?
# this reports the upstream branch for the current repo.
##function parent_branch_name()
##{
  ##echo "$(git branch -vv | grep \* | cut -d ' ' -f2)"
##}

# reports the status of the branch by echoing one of these values:
#   okay: up to date and everything is good.
#   needs_pull: this branch needs to be pulled from origins.
#   needs_push: there are unsaved changes on this branch to push to remote store.
#   diverged: the branches diverged and are going to need a merge.
# reference: https://stackoverflow.com/questions/3258243/check-if-pull-needed-in-git
function check_branch_state()
{
  local branch="$1"; shift

  local to_return=120  # unknown issue.

  local local_branch=$(git rev-parse @)
  local remote_branch=$(git rev-parse "$branch")
  local merge_base=$(git merge-base @ "$branch")

  if [ "$local_branch" == "$remote_branch" ]; then
    echo "okay"
  elif [ "$local_branch" == "$merge_base" ]; then
    echo "needs_pull"
  elif [ "$remote_branch" == "$merge_base" ]; then
    echo "needs_push"
  else
    echo "diverged"
  fi

  return $to_return
}

# the git update process just gets more and more complex when you bring in
# branches, so we've moved this here to avoid having a ton of code in the
# other methods.
function do_careful_git_update()
{
  local directory="$1"; shift
  pushd "$directory" &>/dev/null
  test_or_die "changing to directory: $directory"

  if [ ! -d ".git" ]; then
    # we ignore if they're jumping into a non-useful folder, but also tell them.
    echo "Directory is not a git repository: $directory"
    return 0
  fi

  # first update all our remote branches to their current state from the repos.
  git remote update | $TO_SPLITTER
  promote_pipe_return 0
  test_or_die "git remote update"

  local this_branch="$(my_branch_name)"
#appears to be useless; reports no changes when we need to know about remote changes that do exist:
#hmmm: trying it out again now that things are better elsewhere.  let's see what it says.
  state=$(check_branch_state "$this_branch")
  echo "=> branch '$this_branch' state is: $state"

  # this code is now doing what i have to do when i repair the repo.  and it seems to be good so far.
  local branch_list=$(all_branch_names)
  local bran
  for bran in $branch_list; do
#    echo "synchronizing remote branch: $bran"
    git checkout "$bran" | $TO_SPLITTER
    promote_pipe_return 0
    test_or_die "git switching checkout to remote branch: $bran"

    state=$(check_branch_state "$bran")
    echo "=> branch '$bran' state is: $state"

    remote_branch_info=$(git ls-remote --heads origin $bran 2>/dev/null)
    if [ ! -z "$remote_branch_info" ]; then
      # we are pretty sure the remote branch does exist.
      git pull --no-ff origin "$bran" | $TO_SPLITTER
      promote_pipe_return 0
    fi
    test_or_die "git pull of remote branch: $bran"
  done
  # now switch back to our branch.
  git checkout "$this_branch" | $TO_SPLITTER
  promote_pipe_return 0
  test_or_die "git checking out our current branch: $this_branch"

  # now pull down any changes in our own origin in the repo, to stay in synch
  # with any changes from others.
  git pull --no-ff --all | $TO_SPLITTER
  promote_pipe_return 0
  test_or_die "git pulling all upstream"

  popd &>/dev/null
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

  pushd "$directory" &>/dev/null
  if [ -d "CVS" ]; then
    if test_writeable "CVS"; then
      $blatt
      cvs update . | $TO_SPLITTER
      promote_pipe_return 0
      test_or_die "cvs update"
    fi
  elif [ -d ".svn" ]; then
    if test_writeable ".svn"; then
      $blatt
      svn update . | $TO_SPLITTER
      promote_pipe_return 0
      test_or_die "svn update"
    fi
  elif [ -d ".git" ]; then
    if test_writeable ".git"; then
      $blatt
      git pull --no-ff 2>&1 | grep -v "X11 forwarding request failed" | $TO_SPLITTER
      promote_pipe_return 0
      test_or_die "git pull of origin without fast forwards"
    fi
  else
    # this is not an error necessarily; we'll just pretend they planned this.
    echo no repository in $directory
  fi
  popd &>/dev/null

  restore_terminal_title

  return 0
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
  echo "$sortfile"
  \rm "$tempfile" "$sortfile"
}

# iterates across a list of directories contained in a file (first parameter).
# on each directory name, it performs the action (second parameter) provided.
function perform_revctrl_action_on_file()
{
  local tempfile="$1"; shift
  local action="$1"; shift

  save_terminal_title

  local did_anything=

  while read -u 3 dirname; do
    if [ -z "$dirname" ]; then
      # we often have blank lines in the input file for some reason.
      continue
    fi
    did_anything=yes
    pushd "$dirname" &>/dev/null
    echo "[$(pwd)]"
    # pass the current directory plus the remaining parameters from function invocation.
    $action . 
    test_or_die "performing action $action on: $(pwd)"
    sep 28
    popd &>/dev/null
  done 3<"$tempfile"

  if [ -z "$did_anything" ]; then
    echo "There was nothing to do the action '$action' on."
  fi

  restore_terminal_title

  rm $tempfile
}

