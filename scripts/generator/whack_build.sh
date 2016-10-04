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

  export CRUDFILE="$(mktemp "$TMP/zz_whack_build.XXXXXX")"
  echo "" &>"$CRUDFILE"

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
    "$FEISTY_MEOW_APEX/generatedJUnitFiles" \
    "$GENERATED_DIR/clam_tmp" \
    "$GENERATED_DIR/logs" \
    "$PRODUCTION_DIR/__build_"*.h \
    "$PRODUCTION_DIR/manifest.txt" \
    "$RUNTIME_DIR/binaries" \
    "$RUNTIME_DIR/install" \
    "$RUNTIME_DIR/waste" \
    "$TEMPORARIES_DIR" \
    "$PRODUCTION_DIR/clam_bin" \
    "$PRODUCTION_DIR/binaries" \
    "$PRODUCTION_DIR/install" \
    "$PRODUCTION_DIR/logs" \
    "$PRODUCTION_DIR/waste" 
# last few mentioning production dir are to clean older code.

#  echo $(date): "  cleaning generated files in source hierarchy..."

  if [ "$clean_src" == "clean" -o "$clean_src" == "CLEAN"  ]; then
    echo $(date): "    ** aggressive cleaning activated..."

    # get rid of the build binaries.
    rm -rf "$CLAM_BINARY_DIR"

    perl "$FEISTY_MEOW_SCRIPTS/files/zapdirs.pl" "$FEISTY_MEOW_APEX" >>"$CRUDFILE"
  fi

  echo $(date): "cleaned [$choprepo]."
  rm -rf "$CRUDFILE"
  return 0
}

##############

# clean all known hierarchies of build products...

whack_single_build_area "$FEISTY_MEOW_APEX"

