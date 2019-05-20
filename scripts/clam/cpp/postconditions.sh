#!/bin/bash
# copies the final products of the current project into the repository.

# promote nothing from the built objects if this is a cleanup.
if [ ! -z "$CLEAN" ]; then
  exit 0
fi

# check whether a failure should prevent promotion from occurring.
if [ -f $FAILURE_FILE ]; then
  echo Postconditions will not promote due to a failure:
  cat $FAILURE_FILE
  . $CLAM_SCRIPTS/exit_make.sh
fi

# make sure that we actually did something during the make.
if [ ! -f "$DIRTY_FILE" -a ! -f "$SUBMAKE_FLAG" ]; then
  # nothing was built, seemingly, so we do no promotion.
  exit
fi

# clean up generated resource files after the build.
for i in *.resources; do rm -f "$i"; done

# causes the shell to quit.
DIE="source $CLAM_SCRIPTS/exit_make.sh"

if [ ! -d $TARGETS_STORE ]; then mkdir -p $TARGETS_STORE; fi

if [ "$TYPE" = "library" ]; then

  # make sure the required directories exist.
  if [ ! -d $STATIC_LIBRARY_DIR ]; then mkdir -p $STATIC_LIBRARY_DIR; fi
  if [ ! -d $DYNAMIC_LIBRARY_DIR ]; then mkdir -p $DYNAMIC_LIBRARY_DIR; fi

elif [[ "$TYPE" == "application" || "$TYPE" == "test" ]]; then

  # sets up the directory for executable programs and copies any found in the
  # this project's final directory.

  # first make sure the executable directory is around.
  if [ ! -d $EXECUTABLE_DIR ]; then mkdir -p $EXECUTABLE_DIR; fi

  if [ -z "$NO_COMPILE" ]; then
    # we ensure that none of the normal products are copied for a non-compiling
    # style of project.

    # copy anything extra over.
    if [ ! -z "$EXTRA_COPIES" ]; then
      echo Copying extra files to $EXECUTABLE_DIR.
      echo [$EXTRA_COPIES]
      cp -f $EXTRA_COPIES $EXECUTABLE_DIR || $DIE
    fi

  fi

elif [ "$TYPE" = "hierarchy" ]; then

  # do nothing.
  echo "Done with $PROJECT";

else
  echo "Unknown type for project [$TYPE]; cancelling postconditions!"
  source $CLAM_SCRIPTS/exit_make.sh
fi

