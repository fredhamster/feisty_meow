#!/bin/bash

# This defines some general, useful functions.

# test whether we've been here before or not.
skip_all=
function_sentinel &>/dev/null
if [ $? -eq 0 ]; then
  # there was no error, so we can skip the inits.
  if [ ! -z "$SHELL_DEBUG" ]; then
    echo "skipping function definitions, because already defined."
  fi
  skip_all=yes
fi

if [ -z "$skip_all" ]; then

  if [ ! -z "$SHELL_DEBUG" ]; then
    echo "feisty meow function definitions beginning now..."
  fi

  # a handy little method that can be used for date strings.  it was getting
  # really tiresome how many different ways the script did the date formatting.
  function date_stringer() {
    local sep="$1"; shift
    if [ -z "$sep" ]; then sep='_'; fi
    date +"%Y$sep%m$sep%d$sep%H%M$sep%S" | tr -d '/\n/'
  }
  
  # makes a directory of the name specified and then tries to change the
  # current directory to that directory.
  function mcd() {
    if [ ! -d "$1" ]; then mkdir -p "$1"; fi
    cd "$1"
  }

  function is_array() {
    [[ "$(declare -p $1)" =~ "declare -a" ]]
  }

  function is_alias() {
    alias $1 &>/dev/null
    return $?
  }

  # displays the value of a variable in bash friendly format.
  function var() {
    HOLDIFS="$IFS"
    IFS=""
    while true; do
      local varname="$1"; shift
      if [ -z "$varname" ]; then
        break
      fi

      if is_alias "$varname"; then
#echo found $varname is alias
        local tmpfile="$(mktemp $TMP/aliasout.XXXXXX)"
        alias $varname | sed -e 's/.*=//' >$tmpfile
        echo "alias $varname=$(cat $tmpfile)"
        \rm $tmpfile
      elif [ -z "${!varname}" ]; then
        echo "$varname undefined"
      else
        if is_array "$varname"; then
#echo found $varname is array var 
          local temparray
          eval temparray="(\${$varname[@]})"
          echo "$varname=(${temparray[@]})"
#hmmm: would be nice to print above with elements enclosed in quotes, so that we can properly
# see ones that have spaces in them.
        else
#echo found $varname is simple
          echo "$varname=${!varname}"
        fi
      fi
    done
    IFS="$HOLDIFS"
  }

  function success_sound()
  {
    if [ ! -z "$CLAM_FINISH_SOUND" ]; then
      bash $FEISTY_MEOW_SCRIPTS/multimedia/sound_play.sh "$CLAM_FINISH_SOUND"
    fi
  }

  function error_sound()
  {
    if [ ! -z "$CLAM_ERROR_SOUND" ]; then
      bash $FEISTY_MEOW_SCRIPTS/multimedia/sound_play.sh "$CLAM_ERROR_SOUND"
    fi
  }

  # checks the result of the last command that was run, and if that failed,
  # then this complains and exits from bash.  the function parameters are
  # used as the message to print as a complaint.
  function check_result()
  {
    if [ $? -ne 0 ]; then
      echo -e "failed on: $*"
      error_sound
      exit 1
    fi
  }

  # locates a process given a search pattern to match in the process list.
  # supports a single command line flag style parameter of "-u USERNAME";
  # if the -u flag is found, a username is expected afterwards, and only the
  # processes of that user are considered.
  function psfind() {
    local -a patterns=("${@}")
#echo ====
#echo patterns list is: "${patterns[@]}"
#echo ====

    local user_flag
    if [ "${patterns[0]}" == "-u" ]; then
      user_flag="-u ${patterns[1]}" 
#echo "found a -u parm and user=${patterns[1]}" 
      # void the two elements with that user flag so we don't use them as patterns.
      unset patterns[0] patterns[1]=
    else
      # select all users.
      user_flag="-e"
    fi

    local PID_DUMP="$(mktemp "$TMP/zz_pidlist.XXXXXX")"
    local -a PIDS_SOUGHT
    if [ "$OS" == "Windows_NT" ]; then

#hmmm: windows isn't implementing the user flag yet!
#try collapsing back to the ps implementation from cygwin?
# that would simplify things a lot, if we can get it to print the right output.

      # windows case has some odd gyrations to get the user list.
      if [ ! -d c:/tmp ]; then
        mkdir c:/tmp
      fi
      # windows7 magical mystery tour lets us create a file c:\\tmp_pids.txt, but then it's not
      # really there in the root of drive c: when we look for it later.  hoping to fix that
      # problem by using a subdir, which also might be magical thinking from windows perspective.
      tmppid=c:\\tmp\\pids.txt
      # we have abandoned all hope of relying on ps on windows.  instead we use wmic to get full
      # command lines for processes.
      wmic /locale:ms_409 PROCESS get processid,commandline </dev/null >"$tmppid"
      local flag='/c'
      if [ ! -z "$(uname -a | grep "^MING" )" ]; then
        flag='//c'
      fi
      # we 'type' the file to get rid of the unicode result from wmic.
      # needs to be a windows format filename for 'type' to work.
      cmd $flag type "$tmppid" >$PID_DUMP
      \rm "$tmppid"
      local pid_finder_pattern='s/^.*[[:space:]][[:space:]]*\([0-9][0-9]*\) *\$/\1/p'
      local i
      for i in "${patterns[@]}"; do
        PIDS_SOUGHT+=($(cat $PID_DUMP \
          | grep -i "$i" \
          | sed -n -e "$pid_finder_pattern"))
      done
    else
      /bin/ps $user_flag -o pid,args >$PID_DUMP
#echo ====
#echo got all this stuff in the pid dump file:
#cat $PID_DUMP
#echo ====
      # pattern to use for peeling off the process numbers.
      local pid_finder_pattern='s/^[[:space:]]*\([0-9][0-9]*\).*$/\1/p'
      # remove the first line of the file, search for the pattern the
      # user wants to find, and just pluck the process ids out of the
      # results.
      local i
      for i in "${patterns[@]}"; do
#echo pattern is $i
#echo phase 1: $(cat $PID_DUMP | sed -e '1d' )
#echo phase 2: $(cat $PID_DUMP | sed -e '1d' | grep -i "$i" )
        PIDS_SOUGHT+=($(cat $PID_DUMP \
          | sed -e '1d' \
          | grep -i "$i" \
          | sed -n -e "$pid_finder_pattern"))
      done
#echo ====
#echo pids sought list became:
#echo "${PIDS_SOUGHT[@]}"
#echo ====
    fi
    if [ ${#PIDS_SOUGHT[*]} -ne 0 ]; then
      local PIDS_SOUGHT2=$(printf -- '%s\n' ${PIDS_SOUGHT[@]} | sort | uniq)
      PIDS_SOUGHT=()
      PIDS_SOUGHT=${PIDS_SOUGHT2[*]}
      echo ${PIDS_SOUGHT[*]}
    fi
    /bin/rm $PID_DUMP
  }
  
  # finds all processes matching the pattern specified and shows their full
  # process listing (whereas psfind just lists process ids).
  function psa() {
    if [ -z "$1" ]; then
      echo "psa finds processes by pattern, but there was no pattern on the command line."
      return 1
    fi
    local -a patterns=("${@}")
    p=$(psfind "${patterns[@]}")
    if [ -z "$p" ]; then
      # no matches.
      return 0
    fi

    if [ "${patterns[0]}" == "-u" ]; then
      # void the two elements with that user flag so we don't use them as patterns.
      unset patterns[0] patterns[1]=
    fi

    echo ""
    echo "Processes matching ${patterns[@]}..."
    echo ""
    if [ -n "$IS_DARWIN" ]; then
      unset fuzil_sentinel
      for i in $p; do
        # only print the header the first time.
        if [ -z "$fuzil_sentinel" ]; then
          ps $i -w -u
        else
          ps $i -w -u | sed -e '1d'
        fi
        fuzil_sentinel=true
      done
    else 
      # cases besides mac os x's darwin.
      if [ "$OS" == "Windows_NT" ]; then
        # special case for windows.
        ps | head -1
        for curr in $p; do
          ps -W | grep "$curr" 
        done
      else
        # normal OSes can handle a nice simple query.
        ps wu $p
      fi
    fi
  }
  
  # an unfortunately similarly named function to the above 'ps' as in process
  # methods, but this 'ps' stands for postscript.  this takes a postscript file
  # and converts it into pcl3 printer language and then ships it to the printer.
  # this mostly makes sense for an environment where one's default printer is
  # pcl.  if the input postscript causes ghostscript to bomb out, there has been
  # some good success running ps2ps on the input file and using the cleaned
  # postscript file for printing.
  function ps2pcl2lpr() {
    for $i in $*; do
      gs -sDEVICE=pcl3 -sOutputFile=- -sPAPERSIZE=letter "$i" | lpr -l 
    done
  }
  
  function fix_alsa() {
    sudo /etc/init.d/alsasound restart
  }
  
  # switches from a /X/path form to an X:/ form.  this also processes cygwin paths.
  function unix_to_dos_path() {
    # we usually remove dos slashes in favor of forward slashes.
    local DOSSYHOME
    if [[ ! "$OS" =~ ^[Ww][iI][nN] ]]; then
      # fake this value for non-windows (non-cygwin) platforms.
      DOSSYHOME="$HOME"
    else
      # for cygwin, we must replace the /home/X path with an absolute one, since cygwin
      # insists on the /home form instead of /c/cygwin/home being possible.  this is
      # super frustrating and nightmarish.
      DOSSYHOME="$(cygpath -am "$HOME")"
    fi

    if [ ! -z "$SERIOUS_SLASH_TREATMENT" ]; then
      # unless this flag is set, in which case we force dos slashes.
      echo "$1" | sed -e "s?^$HOME?$DOSSYHOME?g" | sed -e 's/\\/\//g' | sed -e 's/\/cygdrive//' | sed -e 's/\/\([a-zA-Z]\)\/\(.*\)/\1:\/\2/' | sed -e 's/\//\\/g'
    else
      echo "$1" | sed -e "s?^$HOME?$DOSSYHOME?g" | sed -e 's/\\/\//g' | sed -e 's/\/cygdrive//' | sed -e 's/\/\([a-zA-Z]\)\/\(.*\)/\1:\/\2/'
    fi
  }
  
  # switches from an X:/ form to a /cygdrive/X/path form.  this is only useful
  # for the cygwin environment currently.
  function dos_to_unix_path() {
    # we always remove dos slashes in favor of forward slashes.
#old:    echo "$1" | sed -e 's/\\/\//g' | sed -e 's/\([a-zA-Z]\):\/\(.*\)/\/\1\/\2/'
         echo "$1" | sed -e 's/\\/\//g' | sed -e 's/\([a-zA-Z]\):\/\(.*\)/\/cygdrive\/\1\/\2/'
  }

  # returns a successful value (0) if this system is debian or ubuntu.
  function debian_like() {
    # decide if we think this is debian or ubuntu or a variant.
    DEBIAN_LIKE=$(if [ ! -z "$(grep -i debian /etc/issue)" \
        -o ! -z "$(grep -i ubuntu /etc/issue)" ]; then echo 1; else echo 0; fi)
    if [ $DEBIAN_LIKE -eq 1 ]; then
      # success; this is debianish.
      return 0
    else
      # this seems like some other OS.
      return 1
    fi
  }
  
  # su function: makes su perform a login.
  # for some OSes, this transfers the X authority information to the new login.
  function su() {
    if debian_like; then
      # debian currently requires the full version which imports X authority
      # information for su.
  
      # get the x authority info for our current user.
      source $FEISTY_MEOW_SCRIPTS/x_win/get_x_auth.sh
  
      if [ -z "$X_auth_info" ]; then
        # if there's no authentication info to pass along, we just do a normal su.
        /bin/su -l $*
      else
        # under X, we update the new login's authority info with the previous
        # user's info.
        (unset XAUTHORITY; /bin/su -l $* -c "$X_auth_info ; export DISPLAY=$DISPLAY ; bash")
      fi
    else
      # non-debian supposedly doesn't need the extra overhead any more.
      # or at least suse doesn't, which is the other one we've tested on.
      /bin/su -l $*
    fi
  
    # relabel the console after returning.
    bash $FEISTY_MEOW_SCRIPTS/tty/label_terminal_with_infos.sh
  }
  
  # sudo function wraps the normal sudo by ensuring we replace the terminal
  # label if they're doing an su with the sudo.
  function sudo() {
    local first_command="$1"
    /usr/bin/sudo "$@"
    if [ "$first_command" == "su" ]; then
      # yep, they were doing an su, but they're back now.
      bash $FEISTY_MEOW_SCRIPTS/tty/label_terminal_with_infos.sh
    fi
  }
  
  # trashes the .#blah files that cvs and svn leave behind when finding conflicts.
  # this kind of assumes you've already checked them for any salient facts.
  function clean_cvs_junk() {
    for i in $*; do
      find $i -follow -type f -iname ".#*" -exec perl $FEISTY_MEOW_SCRIPTS/files/safedel.pl {} ";" 
    done
  }

  # overlay for nechung binary so that we can complain less grossly about it when it's missing.
  function nechung() {
    local wheres_nechung=$(which nechung 2>/dev/null)
    if [ -z "$wheres_nechung" ]; then
      echo "The nechung oracle program cannot be found.  You may want to consider"
      echo "rebuilding the feisty meow applications with this command:"
      echo "bash $FEISTY_MEOW_SCRIPTS/generator/bootstrap_build.sh"
    else
      $wheres_nechung
    fi
  }
  
  # recreates all the generated files that the feisty meow scripts use.
  function regenerate() {
    # do the bootstrapping process again.
    echo "regenerating feisty meow script environment."
    bash $FEISTY_MEOW_SCRIPTS/core/bootstrap_shells.sh
    echo
    # force a full reload by turning off sentinel variable and alias.
    # the nethack one is used by fred's customizations.
    # interesting note perhaps: found that the NETHACKOPTIONS variable was
    # not being unset correctly when preceded by an alias.  split them up
    # like they are now due to that bug.
    unset -v CORE_ALIASES_LOADED FEISTY_MEOW_GENERATED NECHUNG NETHACKOPTIONS 
    unset -f function_sentinel 
    # reload feisty meow environment in current shell.
    source $FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh
    # run nechung oracle to give user a new fortune.
    nechung
  }

  # generates a random password where the first parameter is the number of characters
  # in the password (default 20) and the second parameter specifies whether to use
  # special characters (1) or not (0).
  # found function at http://legroom.net/2010/05/06/bash-random-password-generator
  function random_password()
  {
    [ "$2" == "0" ] && CHAR="[:alnum:]" || CHAR="[:graph:]"
    cat /dev/urandom | tr -cd "$CHAR" | head -c ${1:-32}
    echo
  }

  # a wrapper for the which command that finds items on the path.  some OSes
  # do not provide which, so we want to not be spewing errors when that
  # happens.
  function whichable()
  {
    to_find="$1"; shift
    which which &>/dev/null
    if [ $? -ne 0 ]; then
      # there is no which command here.  we produce nothing due to this.
      echo
    fi
    echo $(which $to_find)
  }

  # copies a set of custom scripts into the proper location for feisty meow
  # to merge their functions and aliases with the standard set.
  function recustomize()
  {
    user="$1"; shift
    if [ -z "$user" ]; then
      # use our default example user if there was no name provided.
      user=fred
    fi
    if [ ! -d "$FEISTY_MEOW_DIR/customizing/$user" ]; then
      echo "The customization folder provided for $user should be:"
      echo "  '$FEISTY_MEOW_DIR/customizing/$user'"
      echo "but that folder does not exist.  Skipping customization."
      return 1
    fi
    regenerate >/dev/null
    pushd "$FEISTY_MEOW_GENERATED/custom" &>/dev/null
    local incongruous_files="$(bash "$FEISTY_MEOW_SCRIPTS/files/list_non_dupes.sh" "$FEISTY_MEOW_DIR/customizing/$user" "$FEISTY_MEOW_GENERATED/custom")"
    if [ ${#incongruous_files} -ge 1 ]; then
      echo "cleaning unknown older overrides..."
      perl "$FEISTY_MEOW_SCRIPTS/files/safedel.pl" $incongruous_files
      echo
    fi
    popd &>/dev/null
    echo "copying custom overrides for $user"
    mkdir "$FEISTY_MEOW_GENERATED/custom" 2>/dev/null
    perl "$FEISTY_MEOW_SCRIPTS/text/cpdiff.pl" "$FEISTY_MEOW_DIR/customizing/$user" "$FEISTY_MEOW_GENERATED/custom"
    if [ -d "$FEISTY_MEOW_DIR/customizing/$user/scripts" ]; then
      echo "copying custom scripts for $user"
      \cp -R "$FEISTY_MEOW_DIR/customizing/$user/scripts" "$FEISTY_MEOW_GENERATED/custom/"
    fi
    echo
    regenerate
  }

#uhhh, this does what now?
  function add_cygwin_drive_mounts() {
    for i in c d e f g h q z ; do
      ln -s /cygdrive/$i $i
    done
  }

  # takes a file to modify, and then it will replace any occurrences of the
  # pattern provided as the second parameter with the text in the third
  # parameter.
  function replace_pattern_in_file()
  {
    local file="$1"; shift
    local pattern="$1"; shift
    local replacement="$1"; shift
    if [ -z "$file" -o -z "$pattern" -o -z "$replacement" ]; then
      echo "replace_pattern_in_file: needs a filename, a pattern to replace, and the"
      echo "text to replace that pattern with."
      return 1
    fi
    sed -i -e "s%$pattern%$replacement%g" "$file"
  }

  function spacem()
  {
    while [ $# -gt 0 ]; do
      arg="$1"; shift
      if [ ! -f "$arg" -a ! -d "$arg" ]; then
        echo "failure to find a file or directory named '$arg'."
        continue
      fi

      # first we will capture the output of the character replacement operation for reporting.
      # this is done first since some filenames can't be properly renamed in perl (e.g. if they
      # have pipe characters apparently).
      intermediate_name="$(bash "$FEISTY_MEOW_SCRIPTS/files/replace_spaces_with_underscores.sh" "$arg")"
      local saw_intermediate_result=0
      if [ -z "$intermediate_name" ]; then
        # make sure we report something, if there are no further name changes.
        intermediate_name="'$arg'"
      else 
        # now zap the first part of the name off (since original name isn't needed).
        intermediate_name="$(echo $intermediate_name | sed -e 's/.*=> //')"
        saw_intermediate_result=1
      fi

      # first we rename the file to be lower case.
      actual_file="$(echo $intermediate_name | sed -e "s/'\([^']*\)'/\1/")"
      final_name="$(perl $FEISTY_MEOW_SCRIPTS/files/renlower.pl "$actual_file")"
      local saw_final_result=0
      if [ -z "$final_name" ]; then
        final_name="$intermediate_name"
      else
        final_name="$(echo $final_name | sed -e 's/.*=> //')"
        saw_final_result=1
      fi
#echo intermed=$saw_intermediate_result 
#echo final=$saw_final_result 

      if [[ $saw_intermediate_result != 0 || $saw_final_result != 0 ]]; then
        # printout the combined operation results.
        echo "'$arg' => $final_name"
      fi
    done
  }

  ##############

# new breed of definer functions goes here.  still in progress.

  # defines an alias and remembers that this is a new or modified definition.
  # if the feisty meow codebase is unloaded, then so are all the aliases that
  # were defined.
  function define_yeti_alias()
  {
# if alias exists already, save old value for restore,
# otherwise save null value for restore,
# have to handle unaliasing if there was no prior value of one
# we newly defined.
# add alias name to a list of feisty defined aliases.

#hmmm: first implem, just do the alias and get that working...
alias "${@}"


return 0
  }

  # defines a variable within the feisty meow environment and remembers that
  # this is a new or modified definition.  if the feisty meow codebase is
  # unloaded, then so are all the variables that were defined.
  # this function always exports the variables it defines.
#  function define_yeti_variable()
#  {
## if variable exists already, save old value for restore,
## otherwise save null value for restore,
## have to handle unsetting if there was no prior value of one
## we newly defined.
## add variable name to a list of feisty defined variables.
#
##hmmm: first implem just sets it up and exports the variable.
##  i.e., this method always exports.
#export "${@}" 
#
#
#return 0
#  }

  ##############

  function function_sentinel() { return 0; }
  
  if [ ! -z "$SHELL_DEBUG" ]; then echo "feisty meow function definitions done."; fi
  
fi

