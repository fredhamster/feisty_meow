#!/bin/bash

# this script locates the solution that a project file belongs in and decides whether the
# project file contains any bad references to projects that are outside of its solution.

proj_file="$1"; shift

if [ -z "$proj_file" ]; then
  echo This script needs one parameter that is a project file to be verified.
  echo The file will be located in a solution file, and then checked for all project
  echo references being in the same solution file.
  exit 3
fi

##############

# look for the solution that a project belongs to.
function find_solution_membership()
{
  local proj="$1"; shift
  found_solution=
  for i in ${SOLUTIONS[*]}; do
    # secret sauce--don't match on any old reference to the file; we need
    # it to be coming from the real project definition.
    grep -i "$proj\"," "$i" &>/dev/null
    if [ $? -eq 0 ]; then
#echo "$proj found in solution $i"
      found_solution="$i"
      break
    fi
  done
}

function complain_about_project()
{
  filename="$1"; shift
  echo "!!"
  echo "Project $proj_base is in error (at $proj_file)"
  echo "it references project $filename which is external to the solution."
  echo "!!"
  ((errors_seen++))
}

##############

proj_base="$(basename $proj_file)"

CHECKERS="$TMP/checking_refs.txt"

errors_seen=0

#hmmm fix this for big time
export SOLUTIONS=("$BUILD_TOP/libraries/solutions/"*.sln "$BUILD_TOP/products/"*/*.sln)

find_solution_membership "$proj_base"

#echo found sol is $found_solution

if [ -z "$found_solution" ]; then
  echo error: could not find the solution containing $proj_base
  exit 3
fi

# get all the project references from the project file being tested.
#hmmm: fix this path to extract!
bash "$BUILD_TOP/build/tool_source/solution_solvers/extract_projects.sh" "$proj_file" >"$CHECKERS"

# iterate over all references in the project file.
while read line; do
  grep -i "$line" "$found_solution" &>/dev/null
  if [ $? -ne 0 ]; then
    complain_about_project "$line"
  fi
done <"$CHECKERS"

if [ $errors_seen -eq 0 ]; then
  echo "project $proj_base is clean."
else
  echo "ERROR: there were $errors_seen problems in $proj_base; see above logging."
  exit 3
fi


