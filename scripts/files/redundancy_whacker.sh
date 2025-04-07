#!/usr/bin/env bash

# grab the arguments.
SOURCE_DIR="$1"; shift
WHACKING_DAY_DIR="$1"; shift

function print_instructions()
{
  echo "
$(basename $0): This script needs two directory names as arguments.
The first argument should be a source directory where the 'good' files are
located.  The files in this first source folder will not be touched.
The second argument should be a target directory where some of the same files
that are in the source directory can be found.  We don't want these target
files if they're already in the source, so they will be deleted if the user
okays this action.
"
}

# validate the arguments.
if [ -z "$SOURCE_DIR" -o ! -d "$SOURCE_DIR" ]; then
  print_instructions
  echo "the source directory (arg 1) was not provided or does not exist."
  exit 1
fi
if [ -z "$WHACKING_DAY_DIR" -o ! -d "$WHACKING_DAY_DIR" ]; then
  print_instructions
  echo "the whacking target directory (arg2) was not provided or does not exist."
  exit 1
fi
# verify that we are not smashing the same exact directory on itself.
check_source="$(cd $SOURCE_DIR; pwd)"
check_target="$(cd $WHACKING_DAY_DIR; pwd)"
if [ "$check_source" == "$check_target" ]; then
  print_instructions
  echo "the two directories evaluate to the same location!  this would be bad."
  exit 1
fi

# make a temporary file for our filename processing.
TMPFILE="$(mktemp /tmp/compwhacker.XXXXXX)"

# find the list of things we want to whack.
declare -A potential_whackees
find "$WHACKING_DAY_DIR" -follow -type f  > "$TMPFILE"
while read filename; do
  basepiece="${filename##*/}"
  potential_whackees[$basepiece]="$filename"
done < "$TMPFILE"

# find all the source images that we have in imaginations.
declare -A source_files
find "$SOURCE_DIR" -follow -type f  > "$TMPFILE"
while read filename; do
  basepiece="${filename##*/}"
  source_files[$basepiece]="$filename"
done < "$TMPFILE"

# clean up now that we're done with the temporary file.
rm "$TMPFILE"

# now iterate through all the whackable files and see if we have a match on the source side.
declare -A legit_whackables
for maybe_whack in "${!potential_whackees[@]}"; do
  source_location="${source_files[$maybe_whack]}"
#echo "found in source: $source_location"
  if [ ! -z "$source_location" ]; then
    legit_whackables[$maybe_whack]="${potential_whackees[$maybe_whack]}"
#echo "added a legit whackable at: ${legit_whackables[$maybe_whack]}"
  fi
done

if [ ${#legit_whackables[@]} -eq 0 ]; then
  echo "
There were no whackable files found for:
  source='$SOURCE_DIR'
  whacking target='$WHACKING_DAY_DIR'"
  exit 0
fi

echo "
We have found a set of files to clean up:
"
for iter in "${!legit_whackables[@]}"; do
  echo "[$iter] => ${legit_whackables[$iter]}"
done

echo "
Do you want to proceed on cleaning up the above list of files from
the whackable location of '$WHACKING_DAY_DIR' (yes/no)?
"
read line

# just get first character.
firstchar="${line::1}"
# make it lower case also.
firstchar="${firstchar,,}"


if [ "$firstchar" == "y" ]; then
  echo Proceeding with cleanup...
  for zapper in "${legit_whackables[@]}"; do
    rm "$zapper"
  done
else
  echo "Skipping the cleanup due to your answer of '$line'"
fi


