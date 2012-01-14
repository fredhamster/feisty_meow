#!/bin/bash

# this script checks all the known solution files to ensure that no project file is
# listed more than once in the whole set.

export errors_seen=0

# where we'll send our generated files.
export OUT_DIR="$TMP"

export CORE_PROJ="$OUT_DIR/core_projects.txt"
export SHARED_PROJ="$OUT_DIR/shared_projects.txt"
export MIDDLEWARE_PROJ="$OUT_DIR/middleware_projects.txt"

#hmmm: fix references!
# extract all the project names from the solution files.
bash $BUILD_TOP/build/tool_source/solution_solvers/extract_projects.sh "$BUILD_TOP/libraries/solutions/lightlink_core.sln" >"$CORE_PROJ"
bash $BUILD_TOP/build/tool_source/solution_solvers/extract_projects.sh "$BUILD_TOP/libraries/solutions/lightlink_shared.sln" >"$SHARED_PROJ"
bash $BUILD_TOP/build/tool_source/solution_solvers/extract_projects.sh "$BUILD_TOP/products/Middleware/middleware.sln" >"$MIDDLEWARE_PROJ"

export TEMP_COMPARE_FILE="$OUT_DIR/commonlines.txt"

# outer loop is all but the most dependent project.
for i in "$CORE_PROJ" "$SHARED_PROJ"; do
  # inner loop is all but the most depended on project.
  for j in "$SHARED_PROJ" "$MIDDLEWARE_PROJ"; do
    if [ "$i" == "$j" ]; then continue; fi
#echo comparing $i and $j
    comm -1 -2 "$i" "$j" >"$TEMP_COMPARE_FILE"
    if [ -s "$TEMP_COMPARE_FILE" ]; then
      echo "ERROR: the two files $(basename $i) and $(basename $j) share common projects."
      ((errors_seen++))
    fi
  done
done

if [ $errors_seen -gt 0 ]; then
  echo "ERROR: There were errors detected during the checks!"
  exit 3
else
  echo "No problems were seen in any checks."
fi

