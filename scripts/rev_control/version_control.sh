#!/bin/bash

# these are helper functions for doing localized revision control.
# this script should be sourced into other scripts that use it.

# Author: Chris Koeritz
# Author: Kevin Wentworth

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"
source "$FEISTY_MEOW_SCRIPTS/tty/terminal_titler.sh"

#hmmm: we need to dump all the outputs in this script into splitter

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

# classic implementation, but only works with one master branch.
# fixes will be forthcoming from development branch.

      # snag all new files.  not to everyone's liking.
      git add --all .
      test_or_die "git add all new files"

      # see if there are any changes in the local repository.
      if ! git diff-index --quiet HEAD --; then
        # tell git about all the files and get a check-in comment.
        git commit .
        test_or_die "git commit"
      fi

#      # upload the files to the server so others can see them.
#      git push 2>&1 | grep -v "X11 forwarding request failed"
#      if [ ${PIPESTATUS[0]} -ne 0 ]; then false; fi
#      test_or_die "git push"

      # catch if the diff-index failed somehow.
      test_or_die "git diff-index"

      # we continue on to the push, even if there were no changes this time, because
      # there could already be committed changes that haven't been pushed yet.

      local myself="$(my_branch_name)"
#      local parent="$(parent_branch_name)"

      # upload any changes to the upstream repo so others can see them.

#      if [ "$myself" != "$parent" ]; then
#        git push origin "$(myself)" 2>&1 | grep -v "X11 forwarding request failed" | $TO_SPLITTER
#        test_or_die "git push to origin: $myself"
#      else
#        # this branch is the same as the parent, so just push.

        git push 2>&1 | grep -v "X11 forwarding request failed" | $TO_SPLITTER
        test_or_die "git push"
#      fi

    fi
  else
    # nothing there.  it's not an error though.
    echo no repository in $directory
  fi
  popd &>/dev/null

  restore_terminal_title

  true;
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

  true;
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

  true
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

# a helpful method that reports the git branch for the current directory's
# git repository.
function my_branch_name()
{
  echo "$(git branch | grep '\*' | cut -d ' ' -f2)"
}

#this had a -> in it at one point for not matching, didn't it?
# this reports the upstream branch for the current repo.
##function parent_branch_name()
##{
  ##echo "$(git branch -vv | grep \* | cut -d ' ' -f2)"
##}

# this exits with 0 for success (normal bash behavior) when up to date.  if the branch is not up to date,
# then these values are returned:
#DOCUMENT THE VALUES
# reference: https://stackoverflow.com/questions/3258243/check-if-pull-needed-in-git
function check_branch_state()
{
  local branch="$1"; shift

  local to_return=120  # unknown issue.

sep

  LOCAL=$(git rev-parse @)
  REMOTE=$(git rev-parse "$branch")
  BASE=$(git merge-base @ "$branch")
var branch LOCAL REMOTE BASE

  if [ "$LOCAL" == "$REMOTE" ]; then
    echo "Up-to-date"
    to_return=0
  elif [ "$LOCAL" == "$BASE" ]; then
    echo "Need to pull"
    to_return=1
  elif [ "$REMOTE" == "$BASE" ]; then
    echo "Need to push"
    to_return=2
  else
    echo "Diverged"
    to_return=3
  fi

sep

  return $to_return
}

# the git update process just gets more and more complex when you bring in
# branches, so we've moved this here to avoid having a ton of code in the
# do_checkin method.
function careful_git_update()
{

echo A
  this_branch="$(my_branch_name)"

#we want my branch here, don't we?  not like parent or anything?
  check_branch_state "$this_branch"
  state=$?
  test_or_continue "branch state check"
  echo the branch state is $state
#need to instead do something here if fails.

echo B

  # first update all our remote branches to their current state from the repos.
  git remote update
  test_or_die "git remote update"

echo C
  # now pull down any changes in our own origin in the repo, to stay in synch
  # with any changes from others.
  git pull --no-ff --all
  test_or_die "git pulling all upstream"

echo D
#the above are just not enough.  now doing what i have to do to repair things.
branch_list=$(git branch |grep -v '^\*')


echo E

echo The rest of pull is not being done yet.
return 1



# below has older shards of partial knowledge.

#      reslog=$(git log HEAD..origin/master --oneline)
#      if [[ "${reslog}" != "" ]] ; then
#        git merge origin/master

#      # from very helpful page:
#      # https://stackoverflow.com/questions/10312521/how-to-fetch-all-git-branches
#      for remote in $( git branch -r | grep -v -- '->' ); do
#        git branch --track ${remote#origin/} $remote 2>/dev/null
##hmmm: ignoring errors from these, since they are continual.
##hmmm: if we could find a way to not try to track with a local branch when there's already one present, that would be swell.  it's probably simple.
#      done
#
##hmmm: well, one time it failed without the fetch.  i hope that's because the fetch is actually needed and not because the whole approach is fubar.
#      git fetch --all 2>&1 | grep -v "X11 forwarding request failed" | $TO_SPLITTER
#      test_or_die "git fetch"
#
#      git pull --all 2>&1 | grep -v "X11 forwarding request failed" | $TO_SPLITTER
#      test_or_die "git pull"

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
      test_or_die "cvs update"
    fi
  elif [ -d ".svn" ]; then
    if test_writeable ".svn"; then
      $blatt
      svn update . | $TO_SPLITTER
      test_or_die "svn update"
    fi
  elif [ -d ".git" ]; then
    if test_writeable ".git"; then
      $blatt

# classic implementation, but only works with one master branch.
# fixes will be forthcoming from development branch.

#      git pull 2>&1 | grep -v "X11 forwarding request failed" | $TO_SPLITTER
#      if [ ${PIPESTATUS[0]} -ne 0 ]; then false; fi
#      test_or_die "git pull"

#any parms needed?
      careful_git_update 

    fi
  else
    # this is not an error necessarily; we'll just pretend they planned this.
    echo no repository in $directory
  fi
  popd &>/dev/null

  restore_terminal_title

  true
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

