#!/bin/bash

# This defines some general, useful functions.

# test whether we've been here before or not.
skip_all=
function_sentinel &>/dev/null
if [ $? -eq 0 ]; then
  # there was no error, so we can skip the inits.
  if [ ! -z "$SHELL_DEBUG" ]; then
    echo skipping functions.sh because already defined.
  fi
  skip_all=yes
fi

if [ -z "$skip_all" ]; then
  if [ ! -z "$SHELL_DEBUG" ]; then
    echo function definitions begin...
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
  
  # locates a process given a search pattern to match in the process list.
  function psfind() {
    local PID_DUMP="$(mktemp "$TMP/zz_pidlist.XXXXXX")"
    local PIDS_SOUGHT=()
    local patterns=($*)
    if [ "$OS" == "Windows_NT" ]; then
      # needs to be a windows format filename for 'type' to work.
      if [ ! -d c:/tmp ]; then
        mkdir c:/tmp
      fi
      # windows7 magical mystery tour lets us create a file c:\\tmp_pids.txt, but then it's not really there
      # in the root of drive c: when we look for it later.  hoping to fix that problem by using a subdir, which
      # also might be magical thinking from windows perspective.
      tmppid=c:\\tmp\\pids.txt
      # we have abandoned all hope of relying on ps on windows.  instead
      # we use wmic to get full command lines for processes.
      # this does not exist on windows home edition.  we are hosed if that's
      # what they insist on testing on.
      wmic /locale:ms_409 PROCESS get processid,commandline </dev/null >"$tmppid"
      local flag='/c'
      if [ ! -z "$(uname -a | grep "^MING" )" ]; then
        flag='//c'
      fi
      # we 'type' the file to get rid of the unicode result from wmic.
      cmd $flag type "$tmppid" >$PID_DUMP
      \rm "$tmppid"
      local CR=''  # embedded carriage return.
      local appropriate_pattern="s/^.*  *\([0-9][0-9]*\)[ $CR]*\$/\1/p"
      for i in "${patterns[@]}"; do
        PIDS_SOUGHT+=$(cat $PID_DUMP \
          | grep -i "$i" \
          | sed -n -e "$appropriate_pattern")
        if [ ${#PIDS_SOUGHT[*]} -ne 0 ]; then
          # we want to bail as soon as we get matches, because on the same
          # platform, the same set of patterns should work to find all
          # occurrences of the genesis java.
          break;
        fi
      done
    else
      /bin/ps $extra_flags wuax >$PID_DUMP
      # pattern to use for peeling off the process numbers.
      local appropriate_pattern='s/^[-a-zA-Z_0-9][-a-zA-Z_0-9]*  *\([0-9][0-9]*\).*$/\1/p'
      # remove the first line of the file, search for the pattern the
      # user wants to find, and just pluck the process ids out of the
      # results.
      for i in "${patterns[@]}"; do
        PIDS_SOUGHT=$(cat $PID_DUMP \
          | sed -e '1d' \
          | grep -i "$i" \
          | sed -n -e "$appropriate_pattern")
        if [ ${#PIDS_SOUGHT[*]} -ne 0 ]; then
          # we want to bail as soon as we get matches, because on the same
          # platform, the same set of patterns should work to find all
          # occurrences of the genesis java.
          break;
        fi
      done
    fi
    if [ ! -z "$PIDS_SOUGHT" ]; then echo "$PIDS_SOUGHT"; fi
    /bin/rm $PID_DUMP
  }
  
  # finds all processes matching the pattern specified and shows their full
  # process listing (whereas psfind just lists process ids).
  function psa() {
    p=$(psfind "$1")
    if [ -z "$p" ]; then
      echo "psa finds processes by pattern, but there was no pattern on the command line."
      return 1
    fi
    echo ""
    echo "Processes containing \"$1\"..."
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
      extra_flags=
      if [ "$OS" = "Windows_NT" ]; then
        # special case for windows.
        extra_flags=-W
        ps | head -1
        for curr in $p; do
          ps $extra_flags | grep "$curr" 
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
  
  # switches from a /X/path form to an X:/ form.
  function msys_to_dos_path() {
    # we always remove dos slashes in favor of forward slashes.
    echo "$1" | sed -e 's/\\/\//g' | sed -e 's/\/\([a-zA-Z]\)\/\(.*\)/\1:\/\2/'
  }
  
  # switches from an X:/ form to an /X/path form.
  function dos_to_msys_path() {
    # we always remove dos slashes in favor of forward slashes.
    echo "$1" | sed -e 's/\\/\//g' | sed -e 's/\([a-zA-Z]\):\/\(.*\)/\/\1\/\2/'
  }
  
  # su function: makes su perform a login.
  # for some OSes, this transfers the X authority information to the new login.
  function su() {
    # decide if we think this is debian or ubuntu or a variant.
    DEBIAN_LIKE=$(if [ ! -z "$(grep -i debian /etc/issue)" \
        -o ! -z "$(grep -i ubuntu /etc/issue)" ]; then echo 1; else echo 0; fi)
  
    if [ $DEBIAN_LIKE -eq 1 ]; then
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
    /usr/bin/sudo $*
    if [ "$first_command" == "su" ]; then
      # yep, they were doing an su, but they're back now.
      bash $FEISTY_MEOW_SCRIPTS/tty/label_terminal_with_infos.sh
    fi
  }
  
  # buntar is a long needed uncompressing macro that feeds into tar -x.
  # it takes a list of bz2 file names and extracts their contents into
  # sequentially numbered directories.
  function buntar() {
    index=1
    for i in $*; do
      mkdir buntar_$index
      pushd buntar_$index &>/dev/null
      file=$i
      # if the filename has no directory component, we will assume it used to
      # be above our unzipping directory here.
      if [ "$(basename $file)" = $file ]; then
        file=../$file
      fi
      bunzip2 -d -c $file | tar -xf -
      popd &>/dev/null
      index=$(expr $index + 1)
    done
  }
  
  # trashes the .#blah files that cvs and svn leave behind when finding conflicts.
  # this kind of assumes you've already checked them for any salient facts.
  function clean_cvs_junk() {
    for i in $*; do
      find $i -follow -type f -iname ".#*" -exec perl $FEISTY_MEOW_SCRIPTS/files/safedel.pl {} ";" 
    done
  }
  
  # recreates all the generated files that the feisty meow scripts use.
  function regenerate() {
    bash $FEISTY_MEOW_SCRIPTS/core/bootstrap_shells.sh
    echo
    local wheres_nechung=$(which nechung 2>/dev/null)
    if [ -z "$wheres_nechung" ]; then
      echo "The nechung oracle program cannot be found.  You may want to consider"
      echo "rebuilding the feisty meow applications with this command:"
      echo "   bash $FEISTY_MEOW_DIR/scripts/generator/bootstrap_build.sh"
    else
      nechung
    fi
  }

  function function_sentinel() { return 0; }
  
  if [ ! -z "$SHELL_DEBUG" ]; then echo function definitions end....; fi
  
fi

