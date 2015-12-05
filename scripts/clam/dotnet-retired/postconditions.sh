#!/bin/bash
# copies the final products of the current project into the repository.

# check whether a failure should prevent promotion from occurring.
if [ -f $FAILURE_FILE ]; then
  echo Postconditions will not promote due to a failure:
  cat $FAILURE_FILE
  . $CLAM_DIR/exit_make.sh
fi

# make sure that we actually did something during the make.
if [ ! -f "$DIRTY_FILE" -a ! -f "$SUBMAKE_FLAG" ]; then
  # nothing was built, seemingly, so we do no promotion.
  exit
fi

# toss the flag files so we don't see them again.
rm -f "$DIRTY_FILE" "$SUBMAKE_FLAG"

# clean up generated resource files after the build.
for i in *.resources; do rm -f "$i"; done

# these variables define the locations for final products.  all of them
# descend from the root of the repository.
ROOT=$TARGETS_DIR
LIB_DIR=$ROOT/lib
DLL_DIR=$ROOT/dll
EXE_DIR=$ROOT/exe
TEST_ROOT=$ROOT/tests
TEST_DIR=$TEST_ROOT/$PROJECT

# causes the shell to quit.
DIE=". $CLAM_DIR/exit_make.sh"

if [ "$TYPE" = "library" ]; then

  # make sure the required directories exist.
  if [ ! -d $ROOT ]; then mkdir -p $ROOT; fi
  if [ ! -d $LIB_DIR ]; then mkdir $LIB_DIR; fi
  if [ ! -d $DLL_DIR ]; then mkdir $DLL_DIR; fi

  if [ -z "$NO_COMPILE" ]; then
    # we ensure that none of the normal products are copied for a non-compiling
    # style of project.

    # copy the import libraries for any DLLs.
    if [ ! -z "`$FIND "$DYNAMIC_LIBRARY_DIR" -iname "*.lib"`" ]; then
      echo Moving import libraries to $LIB_DIR.
      mv "$DYNAMIC_LIBRARY_DIR"/*.lib $LIB_DIR
    fi

  fi

elif [ "$TYPE" = "application" ]; then

  # sets up the directory for executable programs and copies any found in the
  # this project's final directory.

  # first make sure the executable directory is around.
  if [ ! -d $EXE_DIR ]; then mkdir $EXE_DIR; fi

  if [ -z "$NO_COMPILE" ]; then
    # we ensure that none of the normal products are copied for a non-compiling
    # style of project.

    # copy anything extra over.
    if [ ! -z "$EXTRA_COPIES" ]; then
      echo Copying extra files to $EXE_DIR.
      echo [$EXTRA_COPIES]
      cp -f $EXTRA_COPIES $EXE_DIR || $DIE
    fi

  fi

elif [ "$TYPE" = "test" ]; then

  # sets up a directory for test programs based on the project name and copies
  # the generated programs into there.

  # first make sure the test program root directory is around.
  if [ ! -d $TEST_ROOT ]; then mkdir $TEST_ROOT; fi

  # create the target directory if it doesn't exist.
  if [ ! -d $TEST_DIR ]; then mkdir $TEST_DIR; fi

  if [ -z "$NO_COMPILE" ]; then
    # we ensure that none of the normal products are copied for a non-compiling
    # style of project.

    # make the files writable.  this is required for some tests' data files,
    # which come in from the build and could be read-only.
    chmod 777 $TEST_DIR/* $TEST_DIR/*/* $TEST_DIR/*/*/* >/dev/null 2>&1

    # copy anything extra over.
    if [ ! -z "$EXTRA_COPIES" ]; then
      echo Copying extra files to $TEST_DIR.
      echo [$EXTRA_COPIES]
      cp -f $EXTRA_COPIES $TEST_DIR || $DIE
    fi

  fi

else
  echo "Unknown type for project [$TYPE]; cancelling postconditions!"
  . $CLAM_DIR/exit_make.sh
fi

