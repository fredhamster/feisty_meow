#!/bin/bash

#hmmm: nice to set an interrupt handler for ctrl-c, to catch a break and send it to the cleanup of the crudfile.

# catch whether they want rigorous cleaning or not.
clean_src="$1"; shift

function whack_single_build_area()
{
  local CLEANING_LOCALE="$1"; shift
  if [ ! -d "$CLEANING_LOCALE" ]; then
#    echo "the folder $CLEANING_LOCALE does not exist.  not cleaning."
    return 0
  fi

  local choprepo="$(basename "$(dirname "$CLEANING_LOCALE")" )/$(basename "$CLEANING_LOCALE")"
  echo $(date): "Cleaning up [$choprepo]..."
  if [ -z "$CLEANING_LOCALE" ]; then
    echo "The CLEANING_LOCALE variable is not set!"
    exit 3
  fi

  export NEW_TMP="$(mktemp -d "$CLEANING_LOCALE/TEMPS.XXXXXX")"
  export CRUDFILE="$(mktemp "$NEW_TMP/whack_build.XXXXXX")"
  echo "" &>"$CRUDFILE"

  CLEANING_TOP="$CLEANING_LOCALE/production"

#  echo $(date): "  Cleaning up the build products..."

  # avoid accidentally removing way too much important stuff if our variables have not
  # been previously established.
  local WASTE_DIR="$CLEANING_LOCALE/waste"
  local TEMPORARIES_DIR="$CLEANING_LOCALE/temporaries"

  # kerzap.
  rm -rf \
    "$CLEANING_TOP/binaries" \
    "$CLEANING_TOP/install" \
    "$CLEANING_TOP/logs" \
    "$CLEANING_TOP/objects" \
    "$TEMPORARIES_DIR" \
    "$WASTE_DIR" \
    "$CLEANING_TOP/__build_"*.h \
    "$CLEANING_TOP/manifest.txt" 

#  echo $(date): "  Cleaning generated files in source hierarchy..."

  if [ "$clean_src" == "clean" -o "$clean_src" == "CLEAN"  ]; then
    echo $(date): "    ** Aggressive cleaning activated..."
    perl "$SHELLDIR/files/zapdirs.pl" "$CLEANING_TOP" >>"$CRUDFILE"
  fi

  echo $(date): "Cleaned [$choprepo]."

  rm -rf "$NEW_TMP"

  return 0
}

##############

# clean all known hierarchies of build products...

whack_single_build_area "$REPOSITORY_DIR"



