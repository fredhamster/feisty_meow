#!/bin/bash

# this script adds a sentinel to guard c++ implementation files (*.cpp) from
# multiple inclusion.  it will only add the guardian markers if the files
# don't already have it, as far as the script can tell.
#
# for a file called blahblah.cpp, the sentinel lines end up looking like:
#
#     #ifndef BLAHBLAH_IMPLEMENTATION_FILE
#     #define BLAHBLAH_IMPLEMENTATION_FILE
#     .......rest of file.......
#     #endif

function add_sentinel {
  implem_file="$1"
  echo adding sentinels: $filename
  # get the basename for the file and strip off the extension.
  base=$(basename $implem_file | sed -e 's/\.[a-zA-Z0-9_][a-zA-Z0-9_]*$//')
  # pick a temporary location for the new version of the file.
  temp_out="$(mktemp "$TMP/zz_buildor_sentinel_$base.XXXXXX")"
  # if the file actually exists, then we bother operating on it.
  if [ -f "$implem_file" ]; then
    # check whether the first few lines have our sentinel lines present.
    already_guarded=$(head -3 "$implem_file" | sed -n -e 's/#ifndef/foundit/p' )
    if [ -z "$already_guarded" ]; then
      # the file isn't already protected in those first few lines, so we'll
      # go ahead and add the guardian markers.
#echo "not guarded, will do: $implem_file"
      base_caps=$(echo $base | tr a-z A-Z)
      # the lines below are the first part of the guard which use the
      # capitalized form of the extensionless file name.
      echo "#ifndef ${base_caps}_IMPLEMENTATION_FILE" >"$temp_out"
      echo "#define ${base_caps}_IMPLEMENTATION_FILE" >>"$temp_out"
      echo "" >>"$temp_out"
      # stuff in the original file now.  don't want to leave that out.
      cat "$implem_file" >>"$temp_out"
      # add the ending sentinel with a helpful comment.
      echo "" >>"$temp_out"
      echo "#endif //${base_caps}_IMPLEMENTATION_FILE" >>"$temp_out"
      echo "" >>"$temp_out"
      # move the temporary file into place as the new version of the cpp file.
      mv "$temp_out" "$implem_file"
    fi
  fi
}

outfile="$(mktemp "$TMP/zz_buildor_sentinel.XXXXXX")"

for filename in $*; do 

  # find all the files that match that directory (or filename).
  find "$filename" -iname "*.cpp" >$outfile

  # iterate on the list we found and add sentinels to every file.
  while true; do
    read line_found
#echo line is $line_found
    if [ $? != 0 ]; then break; fi
    add_sentinel "$line_found"
  done <$outfile
  
done

rm "$outfile"

