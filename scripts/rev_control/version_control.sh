#!/bin/bash

# these are helper functions for doing localized revision control.
# this script should be sourced into other scripts that use it.

# one unpleasantry to take care of first; cygwin barfs aggressively if the TMP directory
# is a DOS path, but we need it to be a DOS path for our XSEDE testing, so that blows.
# to get past this, TMP gets changed below to a hopefully generic and safe place.

if [[ "$TMP" =~ .:.* ]]; then
  echo making weirdo temporary directory for DOS path.
  export TMP=/tmp/rev_control_$USER
fi
if [ ! -d "$TMP" ]; then
  mkdir $TMP
fi
if [ ! -d "$TMP" ]; then
  echo "Could not create the temporary directory TMP in: $TMP"
  echo "This script will not work properly without an existing TMP directory."
fi

this_host=
# gets the machine's hostname and stores it in the variable "this_host".
function get_our_hostname()
{
  if [ "$OS" == "Windows_NT" ]; then
    this_host=$(hostname)
  elif [ ! -z "$(echo $MACHTYPE | grep apple)" ]; then
    this_host=$(hostname)
  elif [ ! -z "$(echo $MACHTYPE | grep suse)" ]; then
    this_host=$(hostname --long)
  else
    this_host=$(hostname)
  fi
  #echo "hostname is $this_host"
}

# this function sets a variable called "home_system" to "true" if the
# machine is considered one of fred's home machines.  if you are not
# fred, you may want to change the machine choices.
export home_system=
function is_home_system()
{
  # load up the name of the host.
  get_our_hostname
  # reset the variable that we'll be setting.
  home_system=
  if [[ $this_host == *.gruntose.blurgh ]]; then
    home_system=true
#temp code
elif [[ $this_host == buildy ]]; then
home_system=true
elif [[ $this_host == simmy ]]; then
home_system=true
#temp code
  fi
}

# we only want to totally personalize this script if the user is right.
function check_user()
{
  if [ "$USER" == "fred" ]; then
    export SVNUSER=fred_t_hamster@
    export EXTRA_PROTOCOL=+ssh
  else
    export SVNUSER=
    export EXTRA_PROTOCOL=
  fi
}

# calculates the right modifier for hostnames / repositories.
modifier=
function compute_modifier()
{
  modifier=
  directory="$1"; shift
  in_or_out="$1"; shift
  check_user
  # some project specific overrides.
  if [[ "$directory" == hoople* ]]; then
    modifier="svn${EXTRA_PROTOCOL}://${SVNUSER}svn.code.sf.net/p/hoople2/svn/"
  fi
  if [[ "$directory" == yeti* ]]; then
    modifier="svn${EXTRA_PROTOCOL}://${SVNUSER}svn.code.sf.net/p/yeti/svn/"
  fi
  # see if we're on one of fred's home machines.
  is_home_system
  # special override to pick local servers when at home.
  if [ "$home_system" == "true" ]; then
    if [ "$in_or_out" == "out" ]; then
      # need the right home machine for modifier when checking out.
#huhhh?      modifier="svn://shaggy/"
      modifier=
    else 
      # no modifier for checkin.
      modifier=
    fi
  fi
}

# selects the method for check-in based on where we are.
function do_checkin()
{
  local directory="$1"; shift
  do_update "$directory"
  if [ $? -ne 0 ]; then
    echo "Repository update failed; this should be fixed before check-in."
    return 1
  fi
  pushd "$directory" &>/dev/null
  retval=0  # normally successful.
  if [ -d "CVS" ]; then
    cvs ci .
    retval=$?
  elif [ -d ".svn" ]; then
    svn ci .
    retval=$?
  elif [ -d ".git" ]; then
    # snag all new files.  not to everyone's liking.
    git add --all .
    retval=$?
    # tell git about all the files and get a check-in comment.
    git commit .
    retval+=$?
    # upload the files to the server so others can see them.
    git push 2>&1 | grep -v "X11 forwarding request failed"
    retval+=$?
  else
    echo no repository in $directory
    retval=1
  fi
  popd &>/dev/null
  return $retval
}

function do_diff
{
  local directory="$1"; shift
  pushd "$directory" &>/dev/null
  retval=0  # normally successful.

  # only update if we see a repository living there.
  if [ -d ".svn" ]; then
    svn diff .
  elif [ -d ".git" ]; then
    git diff 
  elif [ -d "CVS" ]; then
    cvs diff .
  fi

  popd &>/dev/null
  return $retval
}

function do_report_new
{
  local directory="$1"; shift
  pushd "$directory" &>/dev/null
  retval=0  # normally successful.

  # only update if we see a repository living there.
  if [ -d ".svn" ]; then
    # this action so far only makes sense and is needed for svn.
    bash $FEISTY_MEOW_SCRIPTS/rev_control/svnapply.sh \? echo
  fi

  popd &>/dev/null
  return $retval
}

# checks in all the folders in a specified list.
function checkin_list()
{
  local list=$*
  for i in $list; do
    # turn repo list back into an array.
    eval "repository_list=( ${REPOSITORY_LIST[*]} )"
    for j in "${repository_list[@]}"; do
      # add in the directory component.
      j="$i/$j"
      if [ ! -d "$j" ]; then continue; fi
      echo "checking in '$j'..."
      do_checkin $j
    done
  done
}

# takes out the first few carriage returns that are in the input.
function squash_first_few_crs()
{
  i=0
  while read line; do
    i=$((i+1))
    if [ $i -le 3 ]; then
      echo -n "$line  "
    else
      echo $line
    fi
  done
  if [ $i -le 3 ]; then
    # if we're still squashing eols, make sure we don't leave them hanging.
    echo
  fi
}

# selects the checkout method based on where we are (the host the script runs on).
function do_update()
{
  directory="$1"; shift
  # plan on success for now.
  retval=0
  pushd "$directory" &>/dev/null
  if [ -d "CVS" ]; then
    cvs update . | squash_first_few_crs
    retval=${PIPESTATUS[0]}
  elif [ -d ".svn" ]; then
    svn update . | squash_first_few_crs
    retval=${PIPESTATUS[0]}
  elif [ -d ".git" ]; then
    git pull 2>&1 | grep -v "X11 forwarding request failed" | squash_first_few_crs
    retval=${PIPESTATUS[0]}
  else
    # this is not an error necessarily; we'll just pretend they planned this.
    echo no repository in $directory
  fi
  popd &>/dev/null
  return $retval
}

# gets all the updates for a list of folders under revision control.
function checkout_list {
  list=$*
  for i in $list; do
    # turn repo list back into an array.
    eval "repository_list=( ${REPOSITORY_LIST[*]} )"
    for j in "${repository_list[@]}"; do
      # add in the directory for our purposes here.
      j="$i/$j"
      if [ ! -d $j ]; then
        if [ ! -z "$SHELL_DEBUG" ]; then
          echo "No directory called $j exists."
        fi
        continue
      fi

      echo -n "retrieving '$j'...  "
      do_update $j
    done
  done
}

# provides a list of absolute paths of revision control directories
# that are located under the directory passed as the first parameter.
function generate_rev_ctrl_filelist()
{
  local dir="$1"; shift
  pushd "$dir" &>/dev/null
  local dirhere="$(\pwd)"
  local tempfile=$(mktemp /tmp/zz_rev_checkin.XXXXXX)
  echo >$tempfile
  find $dirhere -maxdepth 5 -type d -iname ".svn" -exec echo {}/.. ';' >>$tempfile
  find $dirhere -maxdepth 5 -type d -iname ".git" -exec echo {}/.. ';' >>$tempfile
  # CVS is not well behaved like git and (now) svn, and we seldom use it anymore.
  popd &>/dev/null
  local sortfile=$(mktemp /tmp/zz_rev_checkin_sort.XXXXXX)
  sort <"$tempfile" >"$sortfile"
  \rm "$tempfile"
  echo "$sortfile"
}

# iterates across a list of directories contained in a file (first parameter).
# on each directory name, it performs the action (second parameter) provided.
function perform_action_on_file()
{
  local tempfile="$1"; shift
  local action="$1"; shift

#  dirs=($(cat $tempfile))

  while read -u 3 dirname; do
#  for dirname in "${dirs[@]}"; do
    if [ -z "$dirname" ]; then continue; fi
    pushd "$dirname" &>/dev/null
    echo "[$(pwd)]"
    $action .
    echo "======="
    popd &>/dev/null
  done 3<"$tempfile"

  rm $tempfile
}


