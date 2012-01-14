#!/bin/bash

# This defines some general, useful functions.

if [ ! -z "$SHELL_DEBUG" ]; then
  echo function definitions begin...
fi

# applies a chown and a chgrp to the files specified, but the user name must
# have a private group of the same name for this to work.
function chowngrp {
  chown $*
# $2 $3 $4 $5 $6 $7 $8 $9
  chgrp $*
# $2 $3 $4 $5 $6 $7 $8 $9
}

# makes a directory of the name specified and then tries to change the
# current directory to that directory.
function mcd {
  if [ ! -d "$1" ]; then mkdir "$1"; fi
  cd "$1"
}

# locates a process given a search pattern to match in the process list.
function psfind {
  PID_DUMP="$(mktemp "$TMP/zz_pidlist.XXXXXX")"
  appropriate_pattern='s/^[-a-zA-Z_0-9][-a-zA-Z_0-9]*  *\([0-9][0-9]*\).*$/\1/p'
    # pattern to use for peeling off the process numbers.
  extra_flags=
    # flags to pass to ps if any special ones are needed.
  if [ "$OS" = "Windows_NT" ]; then
    # on win32, there is some weirdness to support msys.
    appropriate_pattern='s/^[ 	]*\([0-9][0-9]*\).*$/\1/p'
    extra_flags=-W
  fi
  /bin/ps $extra_flags wuax >$PID_DUMP
  # remove the first line of the file, search for the pattern the
  # user wants to find, and just pluck the process ids out of the
  # results.
  PIDS_SOUGHT=$(cat $PID_DUMP \
    | sed -e '1d' \
    | grep -i "$1" \
    | sed -n -e "$appropriate_pattern")
  if [ ! -z "$PIDS_SOUGHT" ]; then echo "$PIDS_SOUGHT"; fi
  /bin/rm $PID_DUMP
}

# finds all processes matching the pattern specified and shows their full
# process listing (whereas psfind just lists process ids).
function psa {
  p=$(psfind "$1")
  if [ ! -z "$p" ]; then
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
      # cases besides darwin OS (for macs).
      extra_flags=
      if [ "$OS" = "Windows_NT" ]; then
        # special case for windows.
        extra_flags=-W
        ps | head -1
        for curr in $p; do
          ps $extra_flags | grep "^ *$curr" 
        done
      else
        # normal OSes can handle a nice simple query.
        ps wu $p
      fi
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
function ps2pcl2lpr {
  for $i in $*; do
    gs -sDEVICE=pcl3 -sOutputFile=- -sPAPERSIZE=letter "$i" | lpr -l 
  done
}

function fix_alsa {
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

if [ ! -z "$SHELL_DEBUG" ]; then echo function definitions end....; fi

