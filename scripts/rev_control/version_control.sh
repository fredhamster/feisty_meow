#!/usr/bin/env bash

# these are helper functions for doing localized revision control.
# this script should be sourced into other scripts that use it.

# Author: Chris Koeritz
# Author: Kevin Wentworth

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"
source "$FEISTY_MEOW_SCRIPTS/tty/terminal_titler.sh"

##############

# the maximum depth that the recursive functions will try to go below the starting directory.
export MAX_DEPTH=5

# the name of our "don't check this stuff in" file.
#hmmm: name this better for the variable name, like FEISTY_MEOW_REV_CONTROL_NO_CHECKIN_FILENAME or something.
export NO_CHECKIN=".no-checkin"
#hmmm: move to a global repository of variables perhaps?

# use our splitter tool for lengthy output if it's available.
if [ ! -z "$(whichable splitter)" ]; then
  TO_SPLITTER="$(whichable splitter)"
  # calculate the number of columns in the terminal.
  cols=$(get_maxcols)
#hmmm: can the get_maxcols throw any errors, such that it gives a bad value?
  TO_SPLITTER+=" --maxcol $(($cols - 1))"
else
  TO_SPLITTER=cat
fi

##############

# one unpleasantry to take care of first; cygwin barfs aggressively if the TMP directory
# is a DOS path, but we need it to be a DOS path for our GFFS testing, so that blows.
# to get past this, TMP gets changed below to a hopefully generic and safe place.
if [[ "$TMP" =~ .:.* ]]; then
  log_feisty_meow_event "making weirdo temporary directory for PCDOS-style path."
  export TMP=/tmp/rev_control_$(sanitized_username)
fi
if [ ! -d "$TMP" ]; then
  mkdir -p $TMP
fi
if [ ! -d "$TMP" ]; then
  echo "could not create the temporary directory TMP in: $TMP"
  echo "this script will not work properly without an existing TMP directory."
  echo
#hmmm: odd approach to solving the "sourced scripts shouldn't exit or they take down the
#      original caller too" issue.
  echo "hit ctrl-c to stay in this shell now, otherwise it may exit in 10 seconds..."
  sleep 10
  exit 1
fi

##############

# performs a generalized revision control check-in operation for the directory provided.
# this uses the directory's currently configured repository and branch as the target for storage.
function do_revctrl_checkin()
{
  local directory="$1"; shift
#hmmm: abstract reusable code below that processes the directory name for printing.
  # make a nice echoer since we want to use it inside conditions below.
  local nicedir="$directory"
  if [ -z "$nicedir" -o $nicedir == "."  ]; then
    nicedir="$( \cd . && /bin/pwd )"
echo "calculated nicedir as '$nicedir'"
  fi

  # prepare some reporting variables ahead of time.
  local blatt_report="echo -ne \nchecking in '$nicedir'...  "
  local tell_no_checkin="echo -ne \nskipping check-in due to presence of $NO_CHECKIN sentinel file: $nicedir"

  pushd "$directory" &>/dev/null
  if [ -d "CVS" ]; then
    if test_writable "CVS"; then
      do_revctrl_simple_update "$directory"
      exit_on_error "updating repository; this issue should be fixed before check-in."
      if [ -f "$NO_CHECKIN" ]; then
#        echo -ne "\nskipping check-in due to presence of $NO_CHECKIN sentinel file: $directory"
        $tell_no_checkin
      else
        $blatt_report
        cvs ci .
        exit_on_error "cvs checkin"
      fi
    fi
  elif [ -d ".svn" ]; then
    if test_writable ".svn"; then
      do_revctrl_simple_update "$directory"
      exit_on_error "updating repository; this issue should be fixed before check-in."
      if [ -f "$NO_CHECKIN" ]; then
#        echo -ne "\nskipping check-in due to presence of $NO_CHECKIN sentinel file: $directory"
        $tell_no_checkin
      else
        $blatt_report
        svn ci .
        exit_on_error "svn checkin"
      fi
    fi
  elif [ -d ".git" -o ! -z "$(seek_writable ".git" "up")" ]; then
    # if the simple name exists, use that.  otherwise try to seek upwards for .git folder.
    if [ -d ".git" ]; then
      directory="$( \cd . && /bin/pwd )"
      topdir="$directory/.git"
    else
      topdir="$(seek_writable ".git" "up")"
    fi
#echo "got topdir from seeking of '$topdir'"
#if [ -z "$topdir" ]; then
#echo "hey, topdir is blank!!!! bad news."
#fi
    if [ ! -z "$topdir" ]; then

#      # jump to the directory above the .git directory, to make git happy.
#echo "pushing this dir: $topdir/.."
#      pushd "$topdir/.." &>/dev/null
#newdir="$( \cd . && /bin/pwd )"
#echo "now dir is set to $newdir"

      # take steps to make sure the branch integrity is good and we're up to date against remote repos.
      do_revctrl_careful_update "$topdir/.."

      if [ -f "$NO_CHECKIN" ]; then
        $tell_no_checkin
      else
        $blatt_report

        # put all changed and new files in the commit.  not to everyone's liking.
        git add --all . | $TO_SPLITTER
        promote_pipe_return 0
        exit_on_error "git add all new files"

        # see if there are any changes in the local repository.
        if ! git diff-index --quiet HEAD --; then
          # tell git about all the files and get a check-in comment.
          git commit .
          retval=$?
          continue_on_error "git commit"
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

        # we continue on to the push, even if there were no obvious changes this run, because
        # there could already be committed changes that haven't been pushed yet.

        # upload any changes to the upstream repo so others can see them.
        git push --tags origin "$(my_branch_name)" 2>&1 | grep -v "X11 forwarding request failed" | $TO_SPLITTER
        promote_pipe_return 0
        exit_on_error "git push"

      fi
      # unwind the pushed directory again.
      popd &>/dev/null
    fi
  else
    # nothing there.  it's not an error though.
    log_feisty_meow_event "no repository in $directory"
  fi
  popd &>/dev/null

  return 0
}

# shows the local changes in a repository.
function do_revctrl_diff
{
  local directory="$1"; shift

  pushd "$directory" &>/dev/null

  # only update if we see a repository living there.
  if [ -d ".svn" ]; then
    svn diff .
    exit_on_error "subversion diff"
  elif [ -d ".git" -o ! -z "$(seek_writable ".git" "up")" ]; then
    git --no-pager diff 
    exit_on_error "git diff"
  elif [ -d "CVS" ]; then
    cvs diff .
    exit_on_error "cvs diff"
  fi

  popd &>/dev/null

  return 0
}

# reports any files that are not already known to the upstream repository.
function do_revctrl_report_new
{
  local directory="$1"; shift

  pushd "$directory" &>/dev/null

  # only update if we see a repository living there.
  if [ -f "$NO_CHECKIN" ]; then
    echo -ne "\nskipping reporting due to presence of $NO_CHECKIN sentinel file: $directory"
  elif [ -d ".svn" ]; then
    # this action so far only makes sense and is needed for svn.
    bash $FEISTY_MEOW_SCRIPTS/rev_control/svnapply.sh \? echo
    exit_on_error "svn diff"
  elif [ -d ".git" -o ! -z "$(seek_writable ".git" "up")" ]; then
    git status -u
    exit_on_error "git status -u"
  fi

  popd &>/dev/null

  return 0
}

# checks in all the folders in the specified list.
function checkin_list()
{
  # make the list of directories unique.
  local list="$(uniquify $*)"

  # turn repo list back into an array.
  eval "repository_list=( ${REPOSITORY_LIST[*]} )"

  local outer inner

  for outer in "${repository_list[@]}"; do
    # check the repository first, since it might be an absolute path.
    if [[ $outer =~ /.* ]]; then
      # yep, this path is absolute.  just handle it directly.
      if [ ! -d "$outer" ]; then continue; fi
      do_revctrl_checkin "$outer"
      exit_on_error "running check-in (absolute) on path: $outer"
    else
      for inner in $list; do
        # add in the directory component to see if we can find the folder.
        local path="$inner/$outer"
        if [ ! -d "$path" ]; then continue; fi
        do_revctrl_checkin "$path"
        exit_on_error "running check-in (relative) on path: $path"
      done
    fi
  done
}

#hmmm: below functions are git specific and should be named that way.

function all_branch_names()
{
  echo "$(git branch -vv | cut -d ' ' -f2)"
}

#this had a -> in it at one point for not matching, didn't it?
# this reports the upstream branch for the current repo.
##function parent_branch_name()
##{
  ##echo "$(git branch -vv | grep \* | cut -d ' ' -f2)"
##}

# a helpful method that reports the git branch for the current directory's
# git repository.
function my_branch_name()
{
  echo "$(git branch -vv | grep '^\*' | cut -d ' ' -f2)"
}

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

  local local_branch=$(git rev-parse HEAD)
  local remote_branch=$(git rev-parse "$branch")
  local merge_base=$(git merge-base HEAD "$branch")

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

# shows the branch currently active in the repository.
function show_active_branch()
{
#hmmm: if no args, assume current dir!?

  for directory in "$@"; do
    if [ -z "$directory" -o $directory == "."  ]; then
      directory="$( \cd . && /bin/pwd )"
#echo "calculated directory as '$directory'"
    fi

    echo -n -e "$(basename $directory) => branch "
    pushd "$directory" &>/dev/null

#hmmm: if git...
    git rev-parse --abbrev-ref HEAD
#hmmm: else OTHERS!!!

    echo

    popd &>/dev/null
  done
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
function do_revctrl_careful_update()
{
  local directory="$1"; shift
  pushd "$directory" &>/dev/null
  exit_on_error "changing to directory: $directory"

  if [ -d ".git" -o ! -z "$(seek_writable ".git" "up")" ]; then
    # not a git project, so just boil this down to a getem action.
    popd &>/dev/null
    log_feisty_meow_event "skipping careful part and doing simple update on non-git repository: $directory"
    do_revctrl_simple_update "$directory"
    return $?
  fi

#hmmm: another piece of reusable code, to process the directory for printing.
  # make a nice echoer since we want to use it inside conditions below.
  local nicedir="$directory"
  if [ -z "$nicedir" -o "$nicedir" == "." ]; then
    nicedir=$(\pwd)
  fi
  local blatt_report="echo -e \ncarefully retrieving '$nicedir'..."
  $blatt_report

echo "about to do git checkin magic, and current dir is '$(\pwd)'"
echo "this is what i see in this directory..."
ls -al

  local this_branch="$(my_branch_name)"

  show_branch_conditionally "$this_branch"

  # first update all our remote branches to their current state from the repos.
  git remote update | $TO_SPLITTER
  promote_pipe_return 0
  exit_on_error "git remote update"

  show_branch_conditionally "$this_branch"

  # this code is now doing what i have to do when i repair the repo.  and it seems to be good so far.
  # note that we allow the local branch to be merged with its remote counterpart; otherwise we would
  # miss changes that happened elsewhere which should be seen in our local copy.
  local branch_list=$(all_branch_names)
  local bran
  for bran in $branch_list; do
    log_feisty_meow_event "synchronizing remote branch: $bran"
    git checkout "$bran" | $TO_SPLITTER
    promote_pipe_return 0
    exit_on_error "git switching checkout to remote branch: $bran"

    show_branch_conditionally "$this_branch"

    remote_branch_info=$(git ls-remote --heads origin $bran 2>/dev/null)
    if [ ! -z "$remote_branch_info" ]; then
      # we are pretty sure the remote branch does exist.
      git pull --tags origin "$bran" | $TO_SPLITTER
      promote_pipe_return 0
    fi
    exit_on_error "git pull of remote branch: $bran"
  done
  # now switch back to our branch.
  git checkout "$this_branch" | $TO_SPLITTER
  promote_pipe_return 0
  exit_on_error "git checking out our current branch: $this_branch"

  # now pull down any changes in our own origin in the repo, to stay in synch
  # with any changes from others.
  git fetch --tags --all | $TO_SPLITTER
#is the above really important when we did this branch already in the loop?
#it does an --all, but is that effective or different?  should we be doing that in above loop?
  promote_pipe_return 0
  exit_on_error "git pulling all upstream"

  popd &>/dev/null
}

# gets the latest versions of the assets from the upstream repository.
function do_revctrl_simple_update()
{
  directory="$1"; shift

#hmmm: another piece of reusable code, to process the directory for printing.
  # make a nice echoer since we want to use it inside conditions below.
  local nicedir="$directory"
  if [ -z "$nicedir" -o "$nicedir" == "." ]; then
    nicedir=$(\pwd)
  fi
  local blatt_report="echo -e \nretrieving '$nicedir'..."

  pushd "$directory" &>/dev/null
  if [ -d "CVS" ]; then
    if test_writable "CVS"; then
      $blatt_report
      cvs update . | $TO_SPLITTER
      promote_pipe_return 0
      exit_on_error "cvs update"
    fi
  elif [ -d ".svn" ]; then
    if test_writable ".svn"; then
      $blatt_report
      svn update . | $TO_SPLITTER
      promote_pipe_return 0
      exit_on_error "svn update"
    fi
  elif [ -d ".git" -o ! -z "$(seek_writable ".git" "up")" ]; then
    if test_writable ".git"; then
      $blatt_report
      git pull --tags 2>&1 | grep -v "X11 forwarding request failed" | $TO_SPLITTER
      promote_pipe_return 0
      exit_on_error "git pull of origin"
    fi
  else
    # this is not an error necessarily; we'll just pretend they planned this.
    log_feisty_meow_event "no repository in $directory"
  fi
  popd &>/dev/null

  return 0
}

# gets all the updates for a list of folders under revision control.
function checkout_list()
{
  local list="$(uniquify $*)"

  # turn repo list back into an array.
  eval "repository_list=( ${REPOSITORY_LIST[*]} )"

  local outer inner

  for outer in "${repository_list[@]}"; do
    # check the repository first, since it might be an absolute path.
    if [[ $outer =~ /.* ]]; then
      # yep, this path is absolute.  just handle it directly.
      if [ ! -d "$outer" ]; then continue; fi
      do_revctrl_simple_update $outer
      exit_on_error "running update on: $path"
    else
      for inner in $list; do
        # add in the directory component to see if we can find the folder.
        local path="$inner/$outer"
        if [ ! -d "$path" ]; then continue; fi
        do_revctrl_simple_update $path
        exit_on_error "running update on: $path"
      done
    fi
  done
}

# does a careful update on all the folders in the specified list;
# it looks in the REPOSITORY_LIST for those names and updates them.
# this is just like checkout_list, but it's for the puffing up action
# we need to do on git.
function puff_out_list()
{
  # make the list of directories unique.
  local list="$(uniquify $*)"

  # turn repo list back into an array.
  eval "repository_list=( ${REPOSITORY_LIST[*]} )"

  local outer inner

#hmmm: once again, seeing some reusable code in this loop...
  for outer in "${repository_list[@]}"; do
    # check the repository first, since it might be an absolute path.
    if [[ $outer =~ /.* ]]; then
      # yep, this path is absolute.  just handle it directly.
      if [ ! -d "$outer" ]; then continue; fi
      do_revctrl_careful_update "$outer"
      exit_on_error "running puff-out (absolute) on path: $outer"
    else
      for inner in $list; do
        # add in the directory component to see if we can find the folder.
        local path="$inner/$outer"
        if [ ! -d "$path" ]; then continue; fi
        do_revctrl_careful_update "$path"
        exit_on_error "running puff-out (relative) on path: $path"
      done
    fi
  done
}

# provides a list of absolute paths of revision control directories
# that are located under the directory passed as the first parameter.
# if this does not result in any directories being found, then a recursive
# upwards search is done for git repos, which wants the .git directory.
function generate_rev_ctrl_filelist()
{
  local dir="$1"; shift
  pushd "$dir" &>/dev/null
  local dirhere="$( \cd "$(\dirname "$dir")" && /bin/pwd )"
  local tempfile=$(mktemp /tmp/zz_checkins.XXXXXX)
  echo -n >$tempfile
  local additional_filter
  find $dirhere -follow -maxdepth $MAX_DEPTH -type d -iname ".svn" -exec echo {}/.. ';' >>$tempfile 2>/dev/null

#hmmm: how to get the report of things ABOVE here, which we need.
#  can we do an exec using the seek writable?

  find $dirhere -follow -maxdepth $MAX_DEPTH -type d -iname ".git" -exec echo {}/.. ';' >>$tempfile 2>/dev/null

  # CVS is not well behaved like git and (now) svn, and we seldom use it anymore.
  popd &>/dev/null

  # see if they've warned us not to try checking in within vendor hierarchies.
  if [ ! -z "NO_CHECKIN_VENDOR" ]; then
    sed -i -e '/.*\/vendor\/.*/d' "$tempfile"
  fi

  # check if we got any matches.  if this is empty, we'll try our last ditch approach
  # of searching above here for .git directories.
  if [ ! -s "$tempfile" ]; then
    seek_writable ".git" "up" >>$tempfile 2>/dev/null
  fi

  local sortfile=$(mktemp /tmp/zz_checkin_sort.XXXXXX)
  sort <"$tempfile" >"$sortfile"
  echo "$sortfile"
  rm "$tempfile"
}

# iterates across a list of directories contained in a file (first parameter).
# on each directory name, it performs the action (second parameter) provided.
function perform_revctrl_action_on_file()
{
  local tempfile="$1"; shift
  local action="$1"; shift

  local did_anything=

  while read -u 3 dirname; do
    if [ -z "$dirname" ]; then
      # we often have blank lines in the input file for some reason.
      continue
    fi
    did_anything=yes
    pushd "$dirname" &>/dev/null
    echo -n "[$(pwd)]  "
    # pass the current directory plus the remaining parameters from function invocation.
#echo "about to get active with: '$action .'"
    $action . 
    local retval=$?
    if [ $retval -ne 0 ]; then
      rm "$tempfile"
      (exit $retval)  # re-assert the return value as our exit value.
      exit_on_error "performing action $action on: $(pwd)"
    fi
    popd &>/dev/null
  done 3<"$tempfile"

  if [ -z "$did_anything" ]; then
    echo "There was nothing to do the action '$action' on."
  fi

  rm "$tempfile"
}

