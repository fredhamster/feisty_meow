#!/bin/bash
# prepares the project for compilation by creating the appropriate directories.

# make sure the top-level repository exists.
if [ ! -d $REPOSITORY_DIR ]; then mkdir $REPOSITORY_DIR; fi
# make sure our temp directory is there.
if [ ! -d $CLAM_TMP ]; then mkdir $CLAM_TMP; fi
# make sure the generated files have a home.
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
if [ ! -d $TESTS_DIR ]; then mkdir $TESTS_DIR; fi
#
if [ ! -d $EXECUTABLE_DIR ]; then mkdir $EXECUTABLE_DIR; fi
#
if [ ! -d $DYNAMIC_LIBRARY_DIR ]; then mkdir $DYNAMIC_LIBRARY_DIR; fi
#
if [ ! -d $STATIC_LIBRARY_DIR ]; then mkdir $STATIC_LIBRARY_DIR; fi

# set versions on any extras that were specified in the makefile.
if [ ! -z "$EXTRA_VERSIONS" ]; then
  for i in $EXTRA_VERSIONS; do $CLAM_BIN/version_stamper$EXE_END $i $PARAMETER_FILE; done
fi

# create all the directories that objects will go into.
###for i in $OUTPUT_DIRECTORY_LIST; do
###  if [ ! -d "$OUTPUT_PATH/$i" ]; then mkdir "$OUTPUT_PATH/$i"; fi
###done

# for firmware compilations set the compiler to the correct processor platform
if [ "$COMPILER" = "DIAB" ]; then 
  $COMPILER_CONTROL $COMPILER_CONTROL_FLAGS
fi

