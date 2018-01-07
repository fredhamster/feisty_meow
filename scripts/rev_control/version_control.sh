#!/bin/bash

# these are helper functions for doing localized revision control.
# this script should be sourced into other scripts that use it.

# Author: Chris Koeritz
# Author: Kevin Wentworth

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"
source "$FEISTY_MEOW_SCRIPTS/tty/terminal_titler.sh"

##############

# check git version to see if we can use autostash.
# this appears to be an ubuntu issue, where xenial did not provide it even though the
# feature appeared in git 2.6 and xenial claims it has git version 2.7.4.  eventually,
# this version test can go away.
gitvertest="$(git version | sed -e 's/git version [0-9]\.//' | sed -e 's/\.0$//' )"
#hmmm: temp below!
echo gitvertest is $gitvertest
if (( $gitvertest >= 11 )); then
  # auto-stash is not available until 2.6 for git, but ubuntu is misreporting or using a
  # differing version number somehow.  we are sure autostash was missing on ubuntu xenial
  # with git 2.7.4 and it's definitely present in zesty with git at 2.11.
  PULL_ADDITION='--rebase --autostash'
fi

##############

# the maximum depth that the recursive functions will try to go below the starting directory.
export MAX_DEPTH=5

# use our splitter tool for lengthy output if it's available.
if [ ! -z "$(which splitter 2>/dev/null)" ]; then
  TO_SPLITTER="$(which splitter)"
  # calculate the number of columsn in the terminal.
  cols=$(get_maxcols)
  TO_SPLITTER+=" --maxcol $(($cols - 1))"
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
  local blatt="echo -n checking in '$nicedir'...  "

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
#hmmm: begins to look like, you guessed it, a reusable bit that all commit actions could enjoy.
        git commit .
        retval=$?
        test_or_continue "git commit"
        if [ $retval -ne 0 ]; then
          echo -e -n "Commit failed or was aborted:\nShould we continue with other check-ins? [y/N] "
          local line
          read line
          if [[ "${line:0:1}" != "y" ]]; then
            echo "Stopping check-in process due to missing commit and user request."
            exit 1
          fi
        fi
      fi

      # a new set of steps we have to take to make sure the branch integrity is good.
      do_careful_git_update "$(\pwd)"

      # we continue on to the push, even if there were no changes this time, because
      # there could already be committed changes that haven't been pushed yet.

      # upload any changes to the upstream repo so others can see them.
      git push --tags origin "$(my_branch_name)" 2>&1 | grep -v "X11 forwarding request failed" | $TO_SPLITTER
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

# checks in all the folders in the specified list.
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
      do_checkin "$outer"
      test_or_die "running check-in (absolute) on path: $outer"
      sep 28
    else
      for inner in $list; do
        # add in the directory component to see if we can find the folder.
        local path="$inner/$outer"
        if [ ! -d "$path" ]; then continue; fi
        do_checkin "$path"
        test_or_die "running check-in (relative) on path: $path"
        sep 28
      done
    fi
  done

  restore_terminal_title
}

# does a careful git update on all the folders in the specified list.
function puff_out_list()
{
  # make the list of directories unique.
  local list="$(uniquify $*)"

  save_terminal_title

  # turn repo list back into an array.
  eval "repository_list=( ${REPOSITORY_LIST[*]} )"

  local outer inner

#hmmm: once again, seeing some reusable code in this loop...
  for outer in "${repository_list[@]}"; do
    # check the repository first, since it might be an absolute path.
    if [[ $outer =~ /.* ]]; then
      # yep, this path is absolute.  just handle it directly.
      if [ ! -d "$outer" ]; then continue; fi
      do_careful_git_update "$outer"
      test_or_die "running puff-out (absolute) on path: $outer"
      sep 28
    else
      for inner in $list; do
        # add in the directory component to see if we can find the folder.
        local path="$inner/$outer"
        if [ ! -d "$path" ]; then continue; fi
        do_careful_git_update "$path"
        test_or_die "running puff-out (relative) on path: $path"
        sep 28
      done
    fi
  done

  restore_terminal_title
}

#hmmm: to go below.
### takes out the first few carriage returns that are in the input.
##function squash_first_few_crs()
##{
  ##i=0
  ##while read input_text; do
    ##i=$((i+1))
    ##if [ $i -le 5 ]; then
      ##echo -n "$input_text  "
    ##else
      ##echo $input_text
    ##fi
  ##done
  ##if [ $i -le 3 ]; then
    ### if we're still squashing eols, make sure we don't leave them hanging.
    ##echo
  ##fi
##}

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

  if [ -z "$branch" ]; then
    echo "No branch was passed to check branch state."
    return 1
  fi

  local to_return=120  # unknown issue.

  local local_branch=$(git rev-parse @)
  local remote_branch=$(git rev-parse "$branch")
  local merge_base=$(git merge-base @ "$branch")

  local to_echo=
  if [ "$local_branch" == "$remote_branch" ]; then
    to_echo="okay"
  elif [ "$local_branch" == "$merge_base" ]; then
    to_echo="needs_pull"
  elif [ "$remote_branch" == "$merge_base" ]; then
    to_echo="needs_push"
  else
    to_echo="diverged"
  fi

  echo -n "$to_echo"

  return $to_return
}

# only shows the branch state if it's not okay.
# note that this is not the same as a conditional branch (ha ha).
function show_branch_conditionally()
{
  local this_branch="$1"; shift

  local state=$(check_branch_state "$this_branch")
  if [ "$state" != "okay" ]; then
    echo "=> branch '$this_branch' state is not clean: $state"
  fi
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

  local this_branch="$(my_branch_name)"

  show_branch_conditionally "$this_branch"

  # first update all our remote branches to their current state from the repos.
  git remote update | $TO_SPLITTER
  promote_pipe_return 0
  test_or_die "git remote update"

  show_branch_conditionally "$this_branch"

  # this code is now doing what i have to do when i repair the repo.  and it seems to be good so far.
  # note that we allow the local branch to be merged with its remote counterpart; otherwise we would
  # miss changes that happened elsewhere which should be seen in our local copy.
  local branch_list=$(all_branch_names)
  local bran
  for bran in $branch_list; do
#    echo "synchronizing remote branch: $bran"
    git checkout "$bran" | $TO_SPLITTER
    promote_pipe_return 0
    test_or_die "git switching checkout to remote branch: $bran"

    show_branch_conditionally "$this_branch"

    remote_branch_info=$(git ls-remote --heads origin $bran 2>/dev/null)
    if [ ! -z "$remote_branch_info" ]; then
      # we are pretty sure the remote branch does exist.
      git pull $PULL_ADDITION origin "$bran" | $TO_SPLITTER
# we may want to choose to do fast forward, to avoid crazy multiple merge histories
# without any changes in them.  --no-ff
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
  git pull $PULL_ADDITION --all | $TO_SPLITTER
#is the above really important when we did this branch already in the loop?
#it does an --all, but is that effective or different?  should we be doing that in above loop?
# --no-ff   
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
      git pull $PULL_ADDITION 2>&1 | grep -v "X11 forwarding request failed" | $TO_SPLITTER
#ordinary pulls should be allowed to do fast forward: --no-ff 
      promote_pipe_return 0
      test_or_die "git pull of origin"
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
  \rm "$tempfile"
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

  rm "$tempfile"
}

