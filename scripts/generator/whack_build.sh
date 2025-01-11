#!/usr/bin/env bash

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
  if [ -z "$FEISTY_MEOW_GENERATED_STORE" -o -z "$TEMPORARIES_PILE" ]; then
    echo The build whacking script cannot run because either the FEISTY_MEOW_GENERATED_STORE
    echo variable or the TEMPORARIES_PILE variable have not been set.  This makes
    echo it unsafe to remove anything in the build products.
    exit 1
  fi

  # kerzap.  the cleanups in production directory remove older locations of generated files.
  rm -rf \
    "$FEISTY_MEOW_APEX/generatedJUnitFiles" \
    "$FEISTY_MEOW_GENERATED_STORE/clam_tmp" \
    "$FEISTY_MEOW_GENERATED_STORE/logs" \
    "$PRODUCTION_STORE/__build_"*.h \
    "$PRODUCTION_STORE/manifest.txt" \
    "$RUNTIME_PATH/binaries" \
    "$RUNTIME_PATH/install" \
    "$RUNTIME_PATH/waste" \
    "$TEMPORARIES_PILE" \
    "$PRODUCTION_STORE/clam_bin" \
    "$PRODUCTION_STORE/binaries" \
    "$PRODUCTION_STORE/install" \
    "$PRODUCTION_STORE/logs" \
    "$PRODUCTION_STORE/waste" 
# last few mentioning production dir are to clean older code.

#  echo $(date): "  cleaning generated files in source hierarchy..."

  if [ "$clean_src" == "clean" -o "$clean_src" == "CLEAN"  ]; then
    echo $(date): "    ** aggressive cleaning activated..."

    # get rid of the build binaries.
    rm -rf "$CLAM_BINARIES"

    perl "$FEISTY_MEOW_SCRIPTS/files/zapdirs.pl" "$FEISTY_MEOW_APEX" >>"$CRUDFILE"
  fi

  echo $(date): "cleaned [$choprepo]."
  rm -rf "$CRUDFILE"
  return 0
}

##############

# clean all known hierarchies of build products...

whack_single_build_area "$FEISTY_MEOW_APEX"

