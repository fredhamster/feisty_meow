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
  echo $(date): "cleaning up [$choprepo]..."
  if [ -z "$CLEANING_LOCALE" ]; then
    echo "The CLEANING_LOCALE variable is not set!"
    exit 3
  fi

#old  export NEW_TMP="$(mktemp -d "$CLEANING_LOCALE/TEMPS.XXXXXX")"
  export CRUDFILE="$(mktemp "$TMP/zz_whack_build.XXXXXX")"
  echo "" &>"$CRUDFILE"

#  CLEANING_TOP="$CLEANING_LOCALE/production"

#  echo $(date): "  cleaning up the build products..."

  # avoid accidentally removing important stuff if our variables have not been previously
  # established.
  if [ -z "$GENERATED_DIR" -o -z "$TEMPORARIES_DIR" ]; then
    echo The build whacking script cannot run because either the GENERATED_DIR
    echo variable or the TEMPORARIES_DIR variable have not been set.  This makes
    echo it unsafe to remove anything in the build products.
    exit 1
  fi

  # kerzap.  the cleanups in production directory remove older locations of generated files.
  rm -rf \
    "$FEISTY_MEOW_DIR/generatedJUnitFiles" \
    "$TEMPORARIES_DIR" \
    "$GENERATED_DIR" \
    "$CLEANING_TOP/__build_"*.h \
    "$CLEANING_TOP/manifest.txt" \
    "$PRODUCTION_DIR/clam_bin" \
    "$PRODUCTION_DIR/binaries" \
    "$PRODUCTION_DIR/install" \
    "$PRODUCTION_DIR/logs" \
    "$PRODUCTION_DIR/waste" 

#  echo $(date): "  cleaning generated files in source hierarchy..."

  if [ "$clean_src" == "clean" -o "$clean_src" == "CLEAN"  ]; then
    echo $(date): "    ** aggressive cleaning activated..."
    perl "$FEISTY_MEOW_SCRIPTS/files/zapdirs.pl" "$FEISTY_MEOW_DIR" >>"$CRUDFILE"
  fi

  echo $(date): "cleaned [$choprepo]."
  rm -rf "$CRUDFILE"
  return 0
}

##############

# clean all known hierarchies of build products...

whack_single_build_area "$FEISTY_MEOW_DIR"

