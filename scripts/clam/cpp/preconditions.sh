#!/bin/bash
# prepares the project for compilation by creating the appropriate directories.

# make sure our temp directory is there.
if [ ! -d $CLAM_TMP ]; then mkdir $CLAM_TMP; fi

# if the clean flag is defined, then we need to quit.  we should not be
# creating directories or doing other tasks for a build that is really
# a cleanup session.
if [ ! -z "$CLEAN" ]; then
  exit 0
fi

# make sure generated files have a home.
if [ ! -d $TARGETS_DIR ]; then mkdir $TARGETS_DIR; fi
# create the generated files storage area.
if [ ! -d $OUTPUT_ROOT ]; then mkdir $OUTPUT_ROOT; fi
# create the top level object directory if it doesn't exist.
if [ ! -d $BASE_OUTPUT_PATH ]; then mkdir $BASE_OUTPUT_PATH; fi
# create the project level object directory if it is non-existent.
if [ ! -d $OUTPUT_PATH ]; then mkdir $OUTPUT_PATH; fi
# create a directory to hold any debugging files, if necessary.
if [ ! -d $PDB_DIR ]; then mkdir $PDB_DIR; fi
#
####if [ ! -d $TESTS_DIR ]; then mkdir $TESTS_DIR; fi
#
if [ ! -d $EXECUTABLE_DIR ]; then mkdir $EXECUTABLE_DIR; fi
#
if [ ! -d $DYNAMIC_LIBRARY_DIR ]; then mkdir $DYNAMIC_LIBRARY_DIR; fi
#
if [ ! -d $STATIC_LIBRARY_DIR ]; then mkdir $STATIC_LIBRARY_DIR; fi

# set versions on any extras that were specified in the makefile.
if [ ! -z "$EXTRA_VERSIONS" ]; then
  for i in $EXTRA_VERSIONS; do
    $CLAM_BIN/version_stamper$EXE_END $i $PARAMETER_FILE
  done
fi

# we whack any zero length objects found, since those are generally artifacts
# of an aborted compilation.
$FIND "$OBJECT_DIR" -type f -size 0 -exec rm -f {} ';'

# we also clean out the specific targets that we intend to build.  we don't
# want phony old versions sitting around confusing us.
if [ ! -z "$NO_COMPILE" ]; then
#  echo "we're removing these now: $ACTUAL_TARGETS"
  rm -f $ACTUAL_TARGETS
fi

if [ ! -z "$TEST_MAKEFILE" ]; then

# this will only work if the LOCAL_LIBS_USED and SOURCE are exported

#??, STATIC_LIBRARY_DIR

  #echo "compiler=$COMPILER"
  #echo "OP_SYSTEM=$OP_SYSTEM"

  #echo "checking for all local libs: $LOCAL_LIBS_USED"
  failed_check=
  for i in $LOCAL_LIBS_USED; do
  #echo curr lib is $STATIC_LIBRARY_DIR/$i$LIB_ENDING
    if [ ! -f "$STATIC_LIBRARY_DIR/$i$LIB_ENDING" ]; then
      echo "Missing a local library: $i$LIB_ENDING"
      failed_check=true
    fi
  done
  if [ "$failed_check" != "" ]; then
    exit 1  # failure.  
  fi
  
  #echo "checking for all source files: $SOURCE"
  failed_check=
  for i in $SOURCE; do
  #echo curr src file is $i
    if [ ! -f "$i" ]; then
      echo "Missing a source file: $i"
      failed_check=true
    fi
  done
  if [ "$failed_check" != "" ]; then
    exit 1  # failure.  
  fi
fi


