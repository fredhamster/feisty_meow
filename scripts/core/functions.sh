#!/usr/bin/env bash

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

  # a slightly different but also handy time and date function.  this is
  # intended for prefixing on log lines, so that each line has the time it
  # occurred as the first element.
  function timestamper() {
    date +"[%Y-%m-%d %H:%M:%S] " | tr -d '/\n/'
  }
  
  # a wrapper for the which command that finds items on the path.  some OSes
  # do not provide which, so we want to not be spewing errors when that
  # happens.
  function whichable()
  {
    local to_find="$1"; shift
    local WHICHER="$(/usr/bin/which which 2>/dev/null)"
#>&2 echo "got whicher as: $WHICHER"
    if [ $? -ne 0 ]; then
      # there is no which command here.  we produce nothing due to this.
      echo
      return 2
    fi
    local sporkenz  # must be defined local here, before call, or we don't get exit value?!
    sporkenz=$($WHICHER "$to_find" 2>/dev/null)
#>&2 echo "broken with this line, but here is exit val: $?"
    local err=$?
#>&2 echo "got whicher as: $WHICHER"
    echo $sporkenz
    return $err
  }

  # makes a directory of the name specified and then tries to change the
  # current directory to that directory.
  function mcd() {
    if [ ! -d "$1" ]; then mkdir -p "$1"; fi
    cd "$1"
  }

  # returns true if the variable is an array.
  function is_array() {
    [[ "$(declare -p $1)" =~ "declare -a" ]]
  }

  # returns true if the name provided is a defined alias.
  function is_alias() {
    alias $1 &>/dev/null
    return $?
  }

  ####

  # makes the status of pipe number N (passed as first parameter) into the
  # main return value (i.e., the value for $?).  this is super handy to avoid
  # repeating the awkward looking code below in multiple places.
  # the numbering starts at zero, for the first item at the head of the pipe.
  function promote_pipe_return()
  {
    ( exit ${PIPESTATUS[$1]} )
  }

  ####

  # sets the main exit value with the pipe status from a smooshed combination
  # of the top N pipe statuses.
  # zero from all becomes zero, but any pipe status being non-zero will yield
  # a non-zero exit status.  note that the exit values involved are not
  # preserved intact--only the fact of a bad, non-zero exit is signalled.
  function combine_pipe_returns()
  {
    local preserved_statuses=( ${PIPESTATUS[@]} )
    # we are told how many pipe statuses to consider...
    local n=$1
    # or if we are not told, then we make up a highest pipe status to consider.
    if [ -z "$n" ]; then n=1; fi
    # we always incorporate the highest level pipe status requested.
    local accumulator=${preserved_statuses[$n]}
    for (( looper = $n - 1; looper >= 0; looper-- )); do
#echo accumulator has $accumulator 
#echo "loop at index [$looper]"
      # math exercise below is to ensure that we won't overflow 255 limit on
      # return values.  we add about half of each return value being considered
      # (accumulator and current pipe return) to make the "real" return value.
      # this of course blows away mathematical comparisons for the return value
      # in the future (unless all but one pipe status is zero).
      # also, below, the addition of 1 below ensures that an error value of
      # 1 cannot turn into a 0 when we do integer math division by 2.
      combined=$(( (accumulator + ${preserved_statuses[$looper]} + 1) / 2 ))
#echo combined calced as $combined
  
      # now push our new smooshed result into the accumulator.  we'll merge it
      # again if we have to keep looping.
      accumulator=$combined
    done
    # signal the exit value.
    ( exit $accumulator )
  }
  
  #hmmm: pretty handy general function, but horrible name here.
  # ploop: sets the highest non-zero exit value possible.
  function ploop() { (exit 255); return $?; }
  
  ##############

  # attempts to find the script file or at least the defition for a command
  # given the command name.  if it's an alias or a 
  # if the alias points directly at a script file, then that will be printed to stdout.
  function locater()
  {
    local command_name="$1"; shift
    # first test checks whether it's a function or not.  we have a special
    # check here, since this actually does generate some text using our
    # alias finding pattern below, but it's gibberish.  better to rule this
    # case out first.
    maybe_found="$(type "$command_name" 2>/dev/null | grep -i "is a function")"
    if [ ! -z "$maybe_found" ]; then
      # we found a function, so just dump out its definition.
      echo "$(type "$command_name")"
      return
    fi
    # now try searching for a simple alias.
    maybe_found="$(type "$command_name" 2>/dev/null | sed -n -r -e "s/^.*is aliased to .[^ ]+ (.*).$/\1/p")"
    if [ -z "$maybe_found" ]; then
#wtf?      maybe_found="$(alias "$command_name" 2>/dev/null | sed -n -r -e "s/^.*is aliased to .[^ ]+ (.*).$/\1/p")"
#wtf2      if [ -z "$maybe_found" ]; then
        # last ditch effort is to just show what "type" outputs.
        maybe_found="$(type "$command_name")"
#wtf3      fi
    fi
    echo "$maybe_found"
  }

  ##############

  function fm_username()
  {
    # see if we can get the user name from the login name.  oddly this sometimes doesn't work.
    local custom_user="$(logname 2>/dev/null)"
    if [ -z "$custom_user" ]; then
      # try the normal unix user variable.
      custom_user="$(sanitized_username)"
    fi
    if [ -z "$custom_user" ]; then
      # try the windows user variable.
      custom_user="$USERNAME"
    fi
    echo "$custom_user"
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
        rm $tmpfile
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
    local cols=$(stty size 2>/dev/null | awk '{print $2}')
    echo $cols
  }

  ##############

  # checks the result of the last command that was run, and if that failed,
  # then this complains and exits from bash.  the function parameters are
  # used as the message to print as a complaint.
  function exit_on_error()
  {
    if [ $? -ne 0 ]; then
      echo -e "\n\nan important action failed and this script will stop:\n\n$*\n\n*** Exiting script..."
      error_sound
      exit 1
    fi
  }

  # like exit_on_error, but will keep going after complaining.
  function continue_on_error()
  {
    if [ $? -ne 0 ]; then
      echo -e "\n\na problem occurred, but we can continue:\n\n$*\n\n=> Continuing script..."
      error_sound
    fi
  }

  ##############

  # accepts any number of arguments and outputs them to the feisty meow event log.
  function log_feisty_meow_event()
  {
    echo -e "$(timestamper)-- $(sanitized_username)@$(hostname): $*" >> "$FEISTY_MEOW_EVENT_LOG"
  }

  ##############

  # wraps secure shell with some parameters we like, most importantly to enable X forwarding.
  function ssh()
  {
    local args=($@)
    save_terminal_title  # remember the current terminal title.
    /usr/bin/ssh -C "${args[@]}"
#hmmm: removed -Y flag because considered dangerous to trust remote hosts to not abuse our X session.
    restore_terminal_title
  }

  # this version of ssh preserves the use of the -Y flag for when X forwarding is needed.
  function yssh()
  {
    local args=($@)
    save_terminal_title  # remember the current terminal title.
    /usr/bin/ssh -Y "${args[@]}"
    restore_terminal_title
  }

  ##############

  # locates a process given a search pattern to match in the process list.
  #
  # + the -u flag specifies a user name, e.g. "-u joe", which causes only
  #   the processes of that user "joe" to be considered.
  #
  # + the -x flag specifies a pattern to exclude from the list, e.g. "-x pszap.sh"
  #   would ignore any processes that mention the phrase "pszap.sh".
  function psfind() {
    local user_flag="-e"
      # default user flag is for all users.
    local excluder="ScrengeflebbitsAPhraseWeNeverExpecttomatchanythingYO298238"
      # for our default, pick an exclusion string we would never match.

    local found_flag=1
    while [ $found_flag -eq 1 ]; do
      # reset our sentinel now that we're safely in our loop.
      found_flag=0

      # save the first argument, since we're going to shift the args.
      local arg1="$1"
      if [ "$arg1" == "-u" ]; then
        # handle the user flag.
        user_flag="-u $2" 
#echo "found a -u parm and user=$2" 
        found_flag=1  # signal that we found one.
        # skip these two arguments, since we've consumed them.
        shift
        shift
      elif [ "$arg1" == "-x" ]; then
        # handle the exclusion flag.
        excluder="$2" 
#echo "found a -x parm and excluder=$excluder" 
        found_flag=1  # signal that we found one.
        # skip these two arguments, since we've consumed them.
        shift
        shift
      fi
    done

    # now that we've yanked any flags out, we can pull the rest of the
    # arguments in as patterns to seek in the process list.
    local -a patterns=("${@}")
#echo ====
#echo patterns list is: "${patterns[@]}"
#echo ====

    local PID_DUMP="$(mktemp "$TMP/zz_pidlist.XXXXXX")"
    local -a PIDS_SOUGHT

    if [ "$OS" == "Windows_NT" ]; then
      # gets cygwin's (god awful) ps to show windoze processes also.
      local EXTRA_DOZER_FLAGS="-W"
      # pattern to use for peeling off the process numbers.
#      local pid_finder_cmd="awk -- '{ print \$4; }'"
      local field_number=4
    else
      # flags which clean up the process listing output on unixes.
      # apparently cygwin doesn't count as a type of unix, because their
      # crummy specialized ps command doesn't support normal ps flags.
      local EXTRA_UNIX_FLAGS="-o pid,args"
      # pattern to use for peeling off the process numbers.
#      local pid_finder_cmd="sed -n -e \\'s/^[[:space:]]*\([0-9][0-9]*\).*$/\\\\1/p\\'"
#echo pidfinder: $pid_finder_cmd
#      local pid_finder_cmd="awk -- '{ print \$1; }'"
      local field_number=1
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
#echo "pattern curr is '$i'"
      PIDS_SOUGHT+=($(cat $PID_DUMP \
        | grep -i "$i" \
        | grep -v "$excluder" \
        | awk -- "{ print \$${field_number}; }" ))
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
#hmmm: currently not working right for windows cygwin.  we're getting proper
#      winpids out of the list now, but not able to use them in ps?
#      should i be keeping the weirdo pid that we were getting in column 1 and
#      use that, except when talking to taskkill?
#      need further research.
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

#    if [ ! -z "$SERIOUS_SLASH_TREATMENT" ]; then
#      # unless this flag is set, in which case we force dos slashes.
#      echo "$1" | sed -e "s?^$HOME?$DOSSYHOME?g" | sed -e 's/\\/\//g' | sed -e 's/\/cygdrive//' | sed -e 's/\/\([a-zA-Z]\)\/\(.*\)/\1:\/\2/' | sed -e 's/\//\\/g'
#    else
      echo "$1" | sed -e "s?^$HOME?$DOSSYHOME?g" | sed -e 's/\\/\//g' | sed -e 's/\/cygdrive//' | sed -e 's/\/\([a-zA-Z]\)\/\(.*\)/\1:\/\2/'
#    fi
  }
  
#  # switches from an X:/ form to a /cygdrive/X/path form.  this is only useful
#  # for the cygwin environment currently.
#  function dos_to_unix_path() {
#    # we always remove dos slashes in favor of forward slashes.
##old:    echo "$1" | sed -e 's/\\/\//g' | sed -e 's/\([a-zA-Z]\):\/\(.*\)/\/\1\/\2/'
#         echo "$1" | sed -e 's/\\/\//g' | sed -e 's/\([a-zA-Z]\):\/\(.*\)/\/cygdrive\/\1\/\2/'
#  }

  # returns a successful value (0) if this system is debian or ubuntu.
  function debian_like() {
    # decide if we think this is debian or ubuntu or a variant.
    DEBIAN_LIKE=$( \
      if [ \
        ! -z "$(grep -i debian /etc/issue)" -o \
        ! -z "$(grep -i ubuntu /etc/issue)" -o \
        ! -z "$(grep -i 'Pop._OS' /etc/issue)" \
      ]; then echo 1; else echo 0; fi)
    if [ $DEBIAN_LIKE -eq 1 ]; then
      # success; this is debianish.
      return 0
    else
      # this seems like some other OS.
      return 1
    fi
  }
  
  # this function wraps the normal sudo by ensuring we replace the terminal
  # label before we launch what they're passing to sudo.  we also preserve
  # specific variables that enable the main user's ssh credentials to still
  # be relied on for ssh forwarding, even if the '-i' flag is passed to cause
  # a fresh shell (which normally doesn't get the launching user's environment
  # variables).
  function sudo() {
    save_terminal_title

    # hoist our X authorization info in case environment is passed along;
    # this can allow root to use our display to show X.org windows.
    if [ -z "$IMPORTED_XAUTH" -a ! -z "$DISPLAY" ]; then
      export IMPORTED_XAUTH="$(xauth list $DISPLAY | head -n 1 | awk '{print $3}')"
      local REMOVE_IMP_XAUTH=true
    fi

    # launch sudo with just the variables we want to reach the other side.
    local varmods=
    varmods+="OLD_HOME=$HOME "
    if [ ! -z "$IMPORTED_XAUTH" ]; then varmods+="IMPORTED_XAUTH=$IMPORTED_XAUTH "; fi
    if [ ! -z "$SSH_AUTH_SOCK" ]; then varmods+="SSH_AUTH_SOCK=$SSH_AUTH_SOCK "; fi
    /usr/bin/sudo $varmods "$@"
    retval=$?

    # take the xauth info away again if it wasn't set already.
    if [ ! -z "$REMOVE_IMP_XAUTH" ]; then
      unset IMPORTED_XAUTH
    fi
    restore_terminal_title
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
    local wheres_nechung=$(whichable nechung)
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
    echo "$(timestamper)regenerating feisty meow script environment."
    bash $FEISTY_MEOW_SCRIPTS/core/reconfigure_feisty_meow.sh
    echo
    # force a full reload by turning off sentinel variables and methods.
    unset -v CORE_VARIABLES_LOADED FEISTY_MEOW_LOADING_DOCK USER_CUSTOMIZATIONS_LOADED \
        BUILD_VARS_LOADED
    unalias CORE_ALIASES_LOADED &>/dev/null
    unset -f function_sentinel 

    # reuse the original path if we can.
    if [ ! -z "$FEISTY_MEOW_ORIGINAL_PATH" ]; then
      export PATH="$FEISTY_MEOW_ORIGINAL_PATH"
    fi

    # reload feisty meow environment in current shell.
    log_feisty_meow_event "reloading the feisty meow scripts for $(sanitized_username) in current shell."
    source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"
    # run nechung oracle to give user a new fortune.
    nechung
    restore_terminal_title
  }

  # merges a set of custom scripts into the feisty meow environment.  can be
  # passed a name to use as the custom scripts source folder (found on path
  # $FEISTY_MEOW_SCRIPTS/customize/{name}), or it will try to guess the name
  # by using the login name.
  function recustomize()
  {
    local custom_user="$1"; shift
    if [ -z "$custom_user" ]; then
      # default to login name if there was no name provided.
      custom_user="$(fm_username)"
        # we do intend to use the login name here to get the login name and to ignore
        # if the user has sudo root access; we don't want to provide a custom
        # profile for root.
    fi
    # chop off any email address style formatting to leave just the name.
    custom_user="$(echo "$custom_user" | cut -f1 -d'@')"

    save_terminal_title

    if [ ! -d "$FEISTY_MEOW_SCRIPTS/customize/$custom_user" ]; then
      echo -e "the customization folder for '$custom_user' is missing:

    $FEISTY_MEOW_SCRIPTS/customize/$custom_user

we will skip recustomization, but these other customizations are available:
"
      # a little tr and sed magic to fix the carriage returns into commas.
      local line="$(find $FEISTY_MEOW_SCRIPTS/customize -mindepth 1 -maxdepth 1 -type d -exec basename {} ';' | tr '\n' '&' | sed 's/&/, /g' | sed -e 's/, $//')"
        # make the line feeds and carriage returns manageable with tr.
        # convert the ampersand, our weird replacement for EOL, with a comma + space in sed.
        # last touch with sed removes the last comma.
      echo "    $line"
      return 1
    fi

    # recreate the feisty meow loading dock.
    regenerate >/dev/null

    # jump into the loading dock and make our custom link.
    pushd "$FEISTY_MEOW_LOADING_DOCK" &>/dev/null
    if [ -h custom ]; then
      # there's an existing link, so remove it.
      rm custom
    fi
    # make sure we cleaned up the area before we re-link.
    if [ -h custom -o -d custom -o -f custom ]; then
      echo "
Due to an over-abundance of caution, we are not going to remove an unexpected
'custom' object found in the file system.  This object is located in the
feisty meow loading dock here: $(pwd)
And here is a description of the rogue 'custom' object:
"
      ls -al custom
      echo "
If you are pretty sure that this is just a remnant of an older approach in
feisty meow, where we copied the custom directory rather than linking it
(and it most likely is just such a bit of cruft of that nature), then please
remove that old remnant 'custom' item, for example by saying:
  /bin/rm -rf \"custom\" ; popd
Sorry for the interruption, but we want to make sure this removal wasn't
automatic if there is even a small amount of doubt about the issue."
      return 1
    fi

    # create the custom folder as a link to the customizations.
    ln -s "$FEISTY_MEOW_SCRIPTS/customize/$custom_user" custom

    popd &>/dev/null

    # now take into account all the customizations by regenerating the feisty meow environment.
    regenerate

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

  function add_cygwin_drive_mounts() {
    for i in c d e f g h q z ; do
#hmmm: improve this by not adding the link if already there, or if the drive is not valid.
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

  # just shows a separator line for the current console size, or uses the first
  # parameter as the number of columns to expect.  if a second parameter is provided,
  # then that is used as the separator character(s).
  function separator()
  {
    count=$1; shift
    if [ -z "$count" ]; then
      count=$(($COLUMNS - 1))
    fi

    # snag remaining paramters into the characters to show.
    characters="${@}"
    if [ -z "$characters" ]; then
      characters="="
    fi

#hmmm: works, but has flaw of disallowing spaces within the characters variable.
#    local garptemp="$(printf '%*s' "$count")"
#    local emission="${garptemp// /${characters}}"

    local garptemp="$(dd if=/dev/zero bs="$count" count=1 2>/dev/null | tr '\0' 'Q')"
    local emission="${garptemp//Q/${characters}}"

    echo "$emission"
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
    local subbydir="$1"; shift
    numdirs="$(find "$subbydir" -mindepth 1 -maxdepth 1 -type d | wc -l)"
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
      exit_on_error "Creating symlink from '$src' to '$target'"
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
      continue_on_error "pretty printing '$file'"
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
    elif [ -x "$(whichable hostname)" ]; then
      this_host=$(hostname)
    fi
    echo "$this_host"
  }

  # makes sure that the provided "folder" is a directory and is writable.
  function test_writable()
  {
    local folder="$1"; shift
    if [ ! -d "$folder" -o ! -w "$folder" ]; then return 1; fi
    return 0
  }

  # seek_writable:
  # examines the provided "folder" name to test whether it is a directory
  # and is writable.  zero (success) is returned if the folder is found.
  # if the folder is not found, and the second parameter passed is "up",
  # then the folder is sought recursively in higher directory levels.
  # if the directory is found anywhere below the root of the filesystem,
  # then zero (success) is returned.
  # in either case of the directory being found, the successful path location
  # is emitted to stdout.  (note that this may emit a relative path, if the
  # "folder" is a relative path.)
  # if the directory is not found by any means, then a non-zero failure
  # code is returned.
  # if the "folder" is a slash-less string, then it is treated as a simple
  # directory name to be sought in the current directory or above.  but if the
  # folder has a full path with slashes, then the most basenamey directory
  # component is considered the directory to locate.
  function seek_writable()
  {
#hmmm: ever any use to search downwards?  sure there is. ==> currently not supported, but should be.
    local folder="$1"; shift
    local recurse_up="$1"; shift

    # handle a folder with no path elements by jamming current dir into it.
    if [[ $string != *"/"* ]]; then
      local curdir="$( \cd "$(\dirname ".")" && /bin/pwd )"
#echo "curdir is '$curdir'"
      folder="${curdir}/${folder}"
#echo "folder is now '$folder'"
    fi

    # default for us is to not do any directory recursion...
    local loop_up=""

    # magical conversion to lower case in bash.
    recurse_up="${recurse_up,,}"

    # check if they actually wanted recursion.
    if [ "$recurse_up" == "up" ]; then
      # yes, they do want to loop upwards if the relevant revision control
      # folder is not present right here.
      loop_up=yup
    fi

#hmmm: recursion bit below maybe has some useful reusable code, should be its own func,

    # pessimistic default assumes that we will not find it...
    # (here, zero means false, so this is not a bash return value.)
    local directory_present=0

    local mod_folder="$folder"
#echo "mod folder before loop is '$mod_folder'"

    while [ ! -z "$mod_folder" ]; do

      # check for existence and writeability of the folder.
      if [ ! -d "$mod_folder" -o ! -w "$mod_folder" ]; then
        # evidence suggests the desired folder really isn't here at this location.
        directory_present=0
#echo "mod folder does not exist at this level."
      else
        directory_present=1
#echo "mod folder DOES exist at this level."
      fi

      # check if we should be looping at all; if we are not going recursive,
      # then it's time to produce an exit value.
      if [ -z "$loop_up" -o $directory_present -eq 1 ]; then
#echo "exiting now since not looping or found dir"
        # let them know where we found the file.
        echo "$mod_folder"
        # invert the sense of the directory presence to provide a bash return value.
        return $(( ! directory_present ))
      fi

      local base="$(basename "$mod_folder")"
      local parent="$(dirname "$mod_folder")"
#echo parent is $parent
      local parents_parent="$(dirname "$parent")"
#echo parents_parent is $parents_parent
      if [ "$parent" == "$parents_parent" ]; then
        # have to bail now, since we've reached the top of the filesystem.
        break
      fi

      # reconstruct the path without the current unsuccessful parent directory.
      # basically, if mom says no, ask grandma.
      mod_folder="${parents_parent}/${base}"
#echo "mod folder after reconstruction is '$mod_folder'"
    done

    # if we got out of the loop to here, then this is a lack of success.
    return 1
  }

  ##############

  # given a filename and a string to seek and a number of lines, then this
  # function will remove the first occurrence of a line in the file that
  # matches the string, and it will also axe the next N lines as specified.
  function create_chomped_copy_of_file()
  {
    local filename="$1"; shift
    local seeker="$1"; shift
    local numlines=$1; shift

#echo into create_chomped_copy...
#var filename seeker numlines 

    # make a backup first, oy.
    \cp -f "$filename" "/tmp/$(basename ${filename}).bkup-${RANDOM}" 
    exit_on_error "backing up file: $filename"

    # make a temp file to write to before we move file into place in bind.
    local new_version="/tmp/$(basename ${filename}).bkup-${RANDOM}" 
    rm -f "$new_version"
    exit_on_error "cleaning out new version of file from: $new_version"

    local line
    local skip_count=0
    local found_any=
    while read line; do
      # don't bother looking at the lines if we're already in skip mode.
      if [[ $skip_count == 0 ]]; then
        # find the string they're seeking.
        if [[ ! "$line" =~ .*${seeker}.* ]]; then
          # no match.
          echo "$line" >> "$new_version"
        else
          # a match!  start skipping.  we will delete this line and the next N lines.
          ((skip_count++))
#echo first skip count is now $skip_count
          found_any=yes
        fi
      else
        # we're already skipping.  let's keep going until we hit the limit.
        ((skip_count++))
#echo ongoing skip count is now $skip_count
        if (( $skip_count > $numlines )); then
          echo "Done skipping, and back to writing output file."
          skip_count=0
        fi
      fi
    done < "$filename"

#echo file we created looks like this:
#cat "$new_version"

    if [ ! -z "$found_any" ]; then
      # put the file back into place under the original name.
      \mv "$new_version" "$filename"
      exit_on_error "moving the new version into place in: $filename"
    else
      # cannot always be considered an error, but we can at least gripe.
      echo "Did not find any matches for seeker '$seeker' in file: $filename"
    fi
  }

  ##############

  # space 'em all: fixes naming for all of the files of the appropriate types
  # in the directories specified.  we skip any file with a dot in front, to
  # respect their hidden nature.  currently the set of files we'll rename is
  # very boutique; it's in this function, and just happens to be the types of
  # files we work with a lot.
  function spacemall() {
    local -a dirs=("${@}")
    if [ ${#dirs[@]} -eq 0 ]; then
      dirs=(.)
    fi

    local charnfile="$(mktemp $TMP/zz_charn.XXXXXX)"
#hmmm: any way to do the below more nicely or reusably?
#hmmm: yes!  a variable with a list of files that are considered TEXT_FILE_EXTENSIONS or something like that.
#hmmm: yes continued!  also a variable for BINARY_FILE_EXTENSIONS to avoid those, where we need to in other scripts.
#hmmm: wait, we actually have a mix here, since this is a renaming function and not a searching function; get it straight!
#hmmm: would the composition of those two types of extensions cover all the files i want to rename?  they have to be "important".
    find "${dirs[@]}" -follow -maxdepth 1 -mindepth 1 -type f -and -not -iname ".[a-zA-Z0-9]*" | \
        grep -i \
"csv\|doc\|docx\|eml\|html\|ics\|jpeg\|jpg\|m4a\|mov\|mp3\|mp4\|odp\|ods\|odt\|pdf\|png\|ppt\|pptx\|rtf\|txt\|vsd\|vsdx\|wav\|webp\|xls\|xlsx\|xml\|zip" | \
        sed -e 's/^/"/' | sed -e 's/$/"/' | \
        xargs bash "$FEISTY_MEOW_SCRIPTS/files/spacem.sh"
    # drop the temp file now that we're done.
    rm "$charnfile"
  }

  ##############

  # tty relevant functions...

  # keep_awake: sends a message to the screen from the background.
  function keep_awake()
  {
    # just starts the keep_awake process in the background.
    bash $FEISTY_MEOW_SCRIPTS/tty/keep_awake_process.sh &
      # this should leave the job running as %1 or a higher number if there
      # are pre-existing background jobs.
  }

  ##############

  # site avenger functions...

  function switchto()
  {
    THISDIR="$FEISTY_MEOW_SCRIPTS/site_avenger"
    source "$FEISTY_MEOW_SCRIPTS/site_avenger/shared_site_mgr.sh"
    switch_to "$1"
  }

  ##############

  # you have hit the borderline functional zone...

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

  # ...and here's the end of the borderline functional zone.

  ##############

  # NOTE: no more function definitions are allowed after this point.

  function function_sentinel()
  {
    return 0; 
  }
  
  if [ ! -z "$DEBUG_FEISTY_MEOW" ]; then echo "feisty meow function definitions done."; fi

  ##############

  # tests for our functions defined above.

  # change this to '1' to run the tests.
  run_tests=0

  if [ $run_tests != 0 ]; then
    echo running tests on set_var_if_undefined.
    flagrant=petunia
    set_var_if_undefined flagrant forknordle
    exit_on_error "testing if defined variable would be whacked"
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

#ploop
#echo exit value after ploop is $?
#ploop | ploop | ploop | ploop
#echo pipes after plooping: 0=${PIPESTATUS[0]} 1=${PIPESTATUS[1]} 2=${PIPESTATUS[2]} 3=${PIPESTATUS[3]}

    function test_combine_pipe_returns()
    {
      sep 14 '-'
      CALL=(combine_pipe_returns 1)
      duckit | arghmore
      ${CALL[@]}
      retval=$?
      echo "call '${CALL[@]}' => $retval"
  
      sep 14 '-'
      CALL=(combine_pipe_returns 0)
      duckit | arghmore
      ${CALL[@]}
      retval=$?
      echo "call '${CALL[@]}' => $retval"
  
      sep 14 '-'
      CALL=(combine_pipe_returns 2)
      duckit | arghmore | grubblez
      ${CALL[@]}
      retval=$?
      echo "call '${CALL[@]}' => $retval"
    
      sep 14 '-'
      CALL=(combine_pipe_returns 3)
      ploop | ploop | ploop | ploop
      ${CALL[@]}
      retval=$?
      echo "call '${CALL[@]}' => $retval"
    
      sep 14 '-'
      CALL=(combine_pipe_returns 3)
      flarkas=$(ploop | ploop | ploop | ploop)
      ${CALL[@]}
      retval=$?
      echo "embedded quoted call '${CALL[@]}' => $retval"
      echo "and the flarkas string got: '$flarkas'"
    
      sep 14 '-'
    }
    
    # now run the fancy pipe tests.  hope they do not leak.
    echo running tests on combine_pipe_returns.
    test_combine_pipe_returns

# more tests go here...
  fi
fi

