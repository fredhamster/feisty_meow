#!/bin/bash

# This defines some general, useful functions.

#hmmm: starting to get a bit beefy in here.  perhaps there is a good way to refactor the functions into more specific folders, if they aren't really totally general purpose?

##############

# test whether we've been here before or not.
skip_all=
type function_sentinel &>/dev/null
if [ $? -eq 0 ]; then
  # there was no error, so we can skip the inits.
  if [ ! -z "$DEBUG_FEISTY_MEOW" ]; then
    echo "skipping function definitions, because already defined."
  fi
  skip_all=yes
else
  skip_all=
fi

if [ -z "$skip_all" ]; then

  if [ ! -z "$DEBUG_FEISTY_MEOW" ]; then
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

  ##############

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
    done | sort
    IFS="$HOLDIFS"
  }

  ##############

  # when passed a list of things, this will return the unique items from that list as an echo.
  function uniquify()
  {
    # do the uniquification: split the space separated items into separate lines, then
    # sort the list, then run the uniq tool on the list.  results will be packed back onto
    # one line when invoked like: local fredlist="$(uniquify a b c e d a e f a e d b)"
    echo $* | tr ' ' '\n' | sort | uniq
  }

  # sets the variable in parameter 1 to the value in parameter 2, but only if
  # that variable was undefined.
  function set_var_if_undefined()
  {
    local var_name="$1"; shift
    local var_value="$1"; shift
    if [ -z "${!var_name}" ]; then
      eval export $var_name="$var_value"
    fi
  }

  ##############

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

  ##############

  # echoes the maximum number of columns that the terminal supports.  usually
  # anything you print to the terminal with length less than (but not equal to)
  # maxcols will never wrap.
  function get_maxcols()
  {
    # calculate the number of columsn in the terminal.
    local cols=$(stty size | awk '{print $2}')
    echo $cols
  }

  ##############

  # checks the result of the last command that was run, and if that failed,
  # then this complains and exits from bash.  the function parameters are
  # used as the message to print as a complaint.
  function test_or_die()
  {
    if [ $? -ne 0 ]; then
      echo -e "\n\naction failed: $*\n\nExiting script..."
      error_sound
      exit 1
    fi
  }

  # like test_or_die, but will keep going after complaining.
  function test_or_continue()
  {
    if [ $? -ne 0 ]; then
      echo -e "\n\nerror occurred: $*\n\nContinuing script..."
      error_sound
    fi
  }

  ##############

  # wraps secure shell with some parameters we like, most importantly to enable X forwarding.
  function ssh()
  {
    local args=($*)
    # we remember the old terminal title, then force the TERM variable to a more generic
    # version for the other side (just 'linux'); we don't want the remote side still
    # thinking it's running xterm.
    save_terminal_title
#hmmm: why were we doing this?  it scorches the user's logged in session, leaving it without proper terminal handling.
#    # we save the value of TERM; we don't want to leave the user's terminal
#    # brain dead once we come back from this function.
#    local oldterm="$TERM"
#    export TERM=linux
    /usr/bin/ssh -X -C "${args[@]}"
#    # restore the terminal variable also.
#    TERM="$oldterm"
    restore_terminal_title
  }

  ##############

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
      # gets cygwin's (god awful) ps to show windoze processes also.
      local EXTRA_DOZER_FLAGS="-W"
      # pattern to use for peeling off the process numbers.
      local pid_finder_pattern='s/ *\([0-9][0-9]*\) *.*$/\1/p'

    else
      # flags which clean up the process listing output on unixes.
      # apparently cygwin doesn't count as a type of unix, because their
      # crummy specialized ps command doesn't support normal ps flags.
      local EXTRA_UNIX_FLAGS="-o pid,args"
      # pattern to use for peeling off the process numbers.
      local pid_finder_pattern='s/^[[:space:]]*\([0-9][0-9]*\).*$/\1/p'
    fi

    /bin/ps $EXTRA_DOZER_FLAGS $EXTRA_UNIX_FLAGS $user_flag | tail -n +2 >$PID_DUMP
#echo ====
#echo got all this stuff in the pid dump file:
#cat $PID_DUMP
#echo ====

    # search for the pattern the user wants to find, and just pluck the process
    # ids out of the results.
    local i
    for i in "${patterns[@]}"; do
      PIDS_SOUGHT+=($(cat $PID_DUMP \
        | grep -i "$i" \
        | sed -n -e "$pid_finder_pattern"))
    done
#echo ====
#echo pids sought list became:
#echo "${PIDS_SOUGHT[@]}"
#echo ====

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
          ps -W -p $curr | tail -n +2
        done
      else
        # normal OSes can handle a nice simple query.
        ps wu $p
      fi
    fi
  }
  
  ##############

#hmmm: holy crowbars, this is an old one.  do we ever still have any need of it?
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
  
#hmmm: not really doing anything yet; ubuntu seems to have changed from pulseaudio in 17.04?
  # restarts the sound driver.
  function fix_sound_driver() {
    # stop bash complaining about blank function body.
    local nothing=
#if alsa something
#    sudo service alsasound restart
#elif pulse something
#    sudo pulseaudio -k
#    sudo pulseaudio -D
#else
#    something else...?
#fi

  }

  function screen() {
    save_terminal_title
#hmmm: ugly absolute path here.
    /usr/bin/screen $*
    restore_terminal_title
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
      source "$FEISTY_MEOW_SCRIPTS/security/get_x_auth.sh"
  
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
  }
  
  # sudo function wraps the normal sudo by ensuring we replace the terminal
  # label if they're doing an su with the sudo.
  function sudo() {
    save_terminal_title
    /usr/bin/sudo "$@"
    retval=$?
    restore_terminal_title
#    if [ "$first_command" == "su" ]; then
#      # yep, they were doing an su, but they're back now.
#      label_terminal_with_info
#    fi
    return $retval
  }
  
  # trashes the .#blah files that cvs and subversion leave behind when finding conflicts.
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
      echo "bash $FEISTY_MEOW_SCRIPTS/generator/produce_feisty_meow.sh"
      echo
    else
      $wheres_nechung
    fi
  }
  
  # recreates all the generated files that the feisty meow scripts use.
  function regenerate() {
    # do the bootstrapping process again.
    save_terminal_title
    echo "regenerating feisty meow script environment."
    bash $FEISTY_MEOW_SCRIPTS/core/reconfigure_feisty_meow.sh
    echo
    # force a full reload by turning off sentinel variables and methods.
    unset -v CORE_VARIABLES_LOADED FEISTY_MEOW_LOADING_DOCK USER_CUSTOMIZATIONS_LOADED
    unalias CORE_ALIASES_LOADED &>/dev/null
    unset -f function_sentinel 
    # reload feisty meow environment in current shell.
    source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"
    # run nechung oracle to give user a new fortune.
    nechung
    restore_terminal_title
  }

  # copies a set of custom scripts into the proper location for feisty meow
  # to merge their functions and aliases with the standard set.
  function recustomize()
  {
    local custom_user="$1"; shift
    if [ -z "$custom_user" ]; then
      # use our default example user if there was no name provided.
      custom_user=fred
    fi

    save_terminal_title

    if [ ! -d "$FEISTY_MEOW_SCRIPTS/customize/$custom_user" ]; then
      echo "The customization folder provided for $custom_user should be:"
      echo "  '$FEISTY_MEOW_SCRIPTS/customize/$custom_user'"
      echo "but that folder does not exist.  Skipping customization."
      return 1
    fi

    # prevent permission foul-ups.
    chown -R "$(logname):$(logname)" "$FEISTY_MEOW_LOADING_DOCK"/* "$FEISTY_MEOW_GENERATED_STORE"/*

    regenerate >/dev/null
    pushd "$FEISTY_MEOW_LOADING_DOCK/custom" &>/dev/null
    incongruous_files="$(bash "$FEISTY_MEOW_SCRIPTS/files/list_non_dupes.sh" "$FEISTY_MEOW_SCRIPTS/customize/$custom_user" "$FEISTY_MEOW_LOADING_DOCK/custom")"

    local fail_message="\nare the perl dependencies installed?  if you're on ubuntu or debian, try this:\n
    $(grep "apt.*perl" $FEISTY_MEOW_APEX/readme.txt)\n"
    
    #echo "the incongruous files list is: $incongruous_files"
    # disallow a single character result, since we get "*" as result when nothing exists yet.
    if [ ${#incongruous_files} -ge 2 ]; then
      echo "cleaning unknown older overrides..."
      perl "$FEISTY_MEOW_SCRIPTS/files/safedel.pl" $incongruous_files
      test_or_continue "running safedel.  $fail_message" 
      echo
    fi
    popd &>/dev/null
    echo "copying custom overrides for $custom_user"
    mkdir -p "$FEISTY_MEOW_LOADING_DOCK/custom" 2>/dev/null
    perl "$FEISTY_MEOW_SCRIPTS/text/cpdiff.pl" "$FEISTY_MEOW_SCRIPTS/customize/$custom_user" "$FEISTY_MEOW_LOADING_DOCK/custom"
    test_or_continue "running cpdiff.  $fail_message"

    if [ -d "$FEISTY_MEOW_SCRIPTS/customize/$custom_user/scripts" ]; then
      echo "copying custom scripts for $custom_user"
      netcp "$FEISTY_MEOW_SCRIPTS/customize/$custom_user/scripts" "$FEISTY_MEOW_LOADING_DOCK/custom/" &>/dev/null
#hmmm: could save output to show if an error occurs.
    fi
    echo
    regenerate

    # prevent permission foul-ups, again.
    chown -R "$(logname):$(logname)" "$FEISTY_MEOW_LOADING_DOCK" "$FEISTY_MEOW_GENERATED_STORE"

    restore_terminal_title
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

#hmmm: improve this by not adding the link
# if already there, or if the drive is not valid.
  function add_cygwin_drive_mounts() {
    for i in c d e f g h q z ; do
      ln -s /cygdrive/$i $i
    done
  }

  ############################

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

  # similar to replace_pattern_in_file, but also will add the new value
  # when the old one did not already exist in the file.
  function replace_if_exists_or_add()
  {
    local file="$1"; shift
    local phrase="$1"; shift
    local replacement="$1"; shift
    if [ -z "$file" -o ! -f "$file" -o -z "$phrase" -o -z "$replacement" ]; then
      echo "replace_if_exists_or_add: needs a filename, a phrase to replace, and the"
      echo "text to replace that phrase with."
      return 1
    fi
    grep "$phrase" "$file" >/dev/null
    # replace if the phrase is there, otherwise add it.
    if [ $? -eq 0 ]; then
      replace_pattern_in_file "$file" "$phrase" "$replacement"
    else
      # this had better be the complete line.
      echo "$replacement" >>"$file"
    fi
  }

  ############################

  # finds a variable (first parameter) in a particular property file
  # (second parameter).  the expected format for the file is:
  # varX=valueX
  function seek_variable()
  {
    local find_var="$1"; shift
    local file="$1"; shift
    if [ -z "$find_var" -o -z "$file" -o ! -f "$file" ]; then
      echo -e "seek_variable: needs two parameters, firstly a variable name, and\nsecondly a file where the variable's value will be sought." 1>&2
      return 1
    fi
  
    while read line; do
      if [ ${#line} -eq 0 ]; then continue; fi
      # split the line into the variable name and value.
      IFS='=' read -a assignment <<< "$line"
      local var="${assignment[0]}"
      local value="${assignment[1]}"
      if [ "${value:0:1}" == '"' ]; then
        # assume the entry was in quotes and remove them.
        value="${value:1:$((${#value} - 2))}"
      fi
      if [ "$find_var" == "$var" ]; then
        echo "$value"
      fi
    done < "$file"
  }
  
  # finds a variable (first parameter) in a particular XML format file
  # (second parameter).  the expected format for the file is:
  # ... name="varX" value="valueX" ...
  function seek_variable_in_xml()
  {
    local find_var="$1"; shift
    local file="$1"; shift
    if [ -z "$find_var" -o -z "$file" -o ! -f "$file" ]; then
      echo "seek_variable_in_xml: needs two parameters, firstly a variable name, and"
      echo "secondly an XML file where the variable's value will be sought."
      return 1
    fi
  
    while read line; do
      if [ ${#line} -eq 0 ]; then continue; fi
      # process the line to make it more conventional looking.
      line="$(echo "$line" | sed -e 's/.*name="\([^"]*\)" value="\([^"]*\)"/\1=\2/')"
      # split the line into the variable name and value.
      IFS='=' read -a assignment <<< "$line"
      local var="${assignment[0]}"
      local value="${assignment[1]}"
      if [ "${value:0:1}" == '"' ]; then
        # assume the entry was in quotes and remove them.
        value="${value:1:$((${#value} - 2))}"
      fi
      if [ "$find_var" == "$var" ]; then
        echo "$value"
      fi
    done < "$file"
  }
  
  ############################

  # goes to a particular directory passed as parameter 1, and then removes all
  # the parameters after that from that directory.
  function push_whack_pop()
  {
    local dir="$1"; shift
    pushd "$dir" &>/dev/null
    if [ $? -ne 0 ]; then echo failed to enter dir--quitting.; fi
    rm -rf $* &>/dev/null
    if [ $? -ne 0 ]; then echo received a failure code when removing.; fi
    popd &>/dev/null
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

  ##############

#hmmm: this points to an extended functions file being needed; not all of these are core.

  # displays documentation in "md" formatted files.
  function show_md()
  {
    local file="$1"; shift
    pandoc "$file" | lynx -stdin
  }

  ##############

  # just shows a separator line for an 80 column console, or uses the first
  # parameter as the number of columns to expect.
  function separator()
  {
    count=$1; shift
    if [ -z "$count" ]; then
      count=79
    fi
    echo
    local i
    for ((i=0; i < $count - 1; i++)); do
      echo -n "="
    done
    echo
    echo
  }
  # alias for separator.
  function sep()
  {
    separator $*
  }

  ##############

  # count the number of sub-directories in a directory and echo the result.
  function count_directories()
  {
    local appsdir="$1"; shift
    numdirs="$(find "$appsdir" -mindepth 1 -maxdepth 1 -type d | wc -l)"
    echo $numdirs
  }

  # takes a string and capitalizes just the first character.  any capital letters in the remainder of
  # the string are made lower case.  the processed string is returned by an echo.
  function capitalize_first_char()
  {
    local to_dromedary="$1"; shift
    to_dromedary="$(tr '[:lower:]' '[:upper:]' <<< ${to_dromedary:0:1})$(tr '[:upper:]' '[:lower:]' <<< ${to_dromedary:1})"
    echo "$to_dromedary"
  }

  # given a source path and a target path, this will make a symbolic link from
  # the source to the destination, but only if the source actually exists.
  function make_safe_link()
  {
    local src="$1"; shift
    local target="$1"; shift
  
    if [ -d "$src" ]; then
      ln -s "$src" "$target"
      test_or_die "Creating symlink from '$src' to '$target'"
    fi
    echo "Created symlink from '$src' to '$target'."
  }

  # pretty prints the json files provided as parameters.
  function clean_json()
  {
    if [ -z "$*" ]; then return; fi
    local show_list=()
    while true; do
      local file="$1"; shift
      if [ -z "$file" ]; then break; fi
      if [ ! -f "$file" ]; then "echo File '$file' does not exist."; continue; fi
      temp_out="$TMP/$file.view"
      cat "$file" | python -m json.tool > "$temp_out"
      show_list+=($temp_out)
      test_or_continue "pretty printing '$file'"
    done
    filedump "${show_list[@]}"
    rm "${show_list[@]}"
  }

  function json_text()
  {
    # only print our special headers or text fields.
    local CR=$'\r'
    local LF=$'\n'
    clean_json $* |
        grep -i "\"text\":\|^=.*" | 
        sed -e "s/\\\\r/$CR/g" -e "s/\\\\n/\\$LF/g"
  }

  ##############

  # echoes the machine's hostname.  can be used like so:
  #   local my_host=$(get_hostname)
  function get_hostname()
  {
    # there used to be more variation in how to do this, but adopting mingw
    # and cygwin tools really helped out.
    local this_host=unknown
    if [ "$OS" == "Windows_NT" ]; then
      this_host=$(hostname)
    elif [ ! -z "$(echo $MACHTYPE | grep apple)" ]; then
      this_host=$(hostname)
    elif [ ! -z "$(echo $MACHTYPE | grep suse)" ]; then
      this_host=$(hostname --long)
    elif [ -x "$(which hostname 2>/dev/null)" ]; then
      this_host=$(hostname)
    fi
    echo "$this_host"
  }

  # makes sure that the provided "folder" is a directory and is writable.
  function test_writeable()
  {
    local folder="$1"; shift
    if [ ! -d "$folder" -o ! -w "$folder" ]; then return 1; fi
    return 0
  }

  ##############

  # NOTE: no more function definitions are allowed after this point.

  function function_sentinel()
  {
    return 0; 
  }
  
  if [ ! -z "$DEBUG_FEISTY_MEOW" ]; then echo "feisty meow function definitions done."; fi

  ##############

  # test code for set_var_if_undefined.
  run_test=0
  if [ $run_test != 0 ]; then
    echo running tests on set_var_if_undefined.
    flagrant=petunia
    set_var_if_undefined flagrant forknordle
    test_or_die "testing if defined variable would be whacked"
    if [ $flagrant != petunia ]; then
      echo set_var_if_undefined failed to leave the test variable alone
      exit 1
    fi
    unset bobblehead_stomper
    set_var_if_undefined bobblehead_stomper endurance
    if [ $bobblehead_stomper != endurance ]; then
      echo set_var_if_undefined failed to set a variable that was not defined yet
      exit 1
    fi
  fi

fi

