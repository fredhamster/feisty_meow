#!/bin/bash
###############################################################################
#                                                                             #
#  Name   : buildor_gen_deps                                                  #
#  Author : Chris Koeritz                                                     #
#  Rights : Copyright (C) 2008-$now by Author                                 #
#                                                                             #
###############################################################################
#  This script is free software; you can redistribute it and/or modify it     #
#  under the terms of the GNU General Public License as published by the Free #
#  Software Foundation; either version 2 of the License or (at your option)   #
#  any later version.  See "http://www.fsf.org/copyleft/gpl.html" for a copy  #
#  of the License online.  Please send any updates to "fred@gruntose.com".    #
###############################################################################

if [ ! -z "$CLEAN" ]; then
  echo "in cleaning mode, will not build dependencies."
  exit 0
fi

# this script finds all of the headers used by a cpp file and outputs a
# list of other cpp files that are probably needed for building it.

  # these semi-global variables used throughout the whole script to accumulate
  # information, rather than trying to juggle positional parameters everywhere.

  # the list of dependencies being accumulated.
  declare -a dependency_accumulator=()

  # a set of files that are known to be bad, since we cannot find them.
  declare -a bad_files=()

  # makes sure we don't keep looking at files even when they're neither
  # bad nor listed as dependencies.
  declare -a boring_files=()

  # this directory is not allowed to participate in the scavenging
  # because it's where the tool was pointed at.  if we allowed files in
  # the app's same directory to be added, that leads to bad dependencies.
  prohibited_directory=""

# set up the separator character so we don't eat tabs or spaces.  this should
# be a character we hope to see pretty much never in a file near the includes.
export IFS=""

# create a variable with the tab in it to avoid weirdness with grep.
TAB_CHAR="$(echo -e -n '\t')"

# reports if a certain dependency has been seen already.
# a zero success value is returned if the file has been seen before,
# and a non-zero failure value for when the file is totally new.
function seen_already {
  if existing_dep "$1"; then return 0; fi  # added it to list already.
  if bad_file "$1"; then return 0; fi  # known to suck.
  if boring_file "$1"; then return 0; fi  # we already saw it.
  return 1  # we had not seen this one, so we return an error.
}

# adds a new dependency at the end of the list.
function add_new_dep {
  # make sure we haven't already processed this.
  local dep="$1"
  if seen_already "$dep"; then
#echo bailing since seen: $dep
 return 1; fi
#echo had not seen before: $dep

#  if existing_dep $dep; then return 1; fi  # added it to list already.
#  if bad_file $dep; then return 1; fi  # known to suck.
#  if boring_file $dep; then return 1; fi  # we already saw it.
##echo new dep: $dep

  dependency_accumulator+=($dep)
  return 0
}

# checks the existing dependencies to see if the first parameter is already
# listed.  if this is the case, zero is returned (meaning success).  if
# the dependency is missing, then -1 is return to indicate an error.
function existing_dep {
#hmmm: below is not very efficient!
  for currite in ${dependency_accumulator[*]}; do
    if [ "$currite" == "$1" ]; then return 0; fi
  done
  return 1
}

# reports whether a file name has already been processed.
function boring_file {

#hmmm: below might not be very efficient!
  for currite in ${boring_files[*]}; do
    if [ "$currite" == "$1" ]; then return 0; fi
  done
  return 1
}

# reports whether a file name has already been found to be missing.
function bad_file {

#hmmm: below also is not very efficient!
  for currite in ${bad_files[*]}; do
    if [ "$currite" == "$1" ]; then return 0; fi
  done
  return 1
}

# checks whether an item is already contained in a list.  the first parameter
# is taken as the item that one wants to add.  the second through n-th
# parameters are taken as the candidate list.  if the item is present, then
# zero is returned to indicate success.  otherwise a non-zero return value
# indicates that the item was not yet present.
function already_listed {
  to_find=$1
  shift
  while (( $# > 0 )); do
    # return that we found it if the current item matches.
    if [ "$to_find" == "$1" ]; then return 0; fi
    shift  # toss next one out.
  done
  # failed to match it.
  return 1
}

# finds the index of a particular element in the remainder of a list.
# the variable __finders_indy will be set to -1 for no match, or it will be the
# index of the element if the item was found.
__finders_indy=-1
function find_in_array {
  local to_find=$1
#echo find_in_array needs: $to_find
  shift
#echo restargs finder: $*
  local indy=0
  while (( $# > 0 )); do
    # return that we found it if the current item matches.
#echo "find_in_array posn $indy has $1"
    if [ "$to_find" == "$1" ]; then
#echo "FOUND $to_find at $indy"
       __finders_indy=$indy
       return 0
    fi
    shift  # toss next one out.
    indy=$(expr $indy + 1)
#echo "find_in_array indy now $indy "
  done
  _finders_indy=-1
  # failed to match it.
  return 1
}

############################################################################
#
# this variable gets stored into when resolve_filename runs.
declare -a resolve_target_array=()
#
# this variable is used internally by resolve_filename.  it should not need
# to be reset between runs on different files because the source hierarchy
# is not supposed to be getting files deleted or added while the deps are
# being geneated.
declare -a resolve_matches_src=()
declare -a resolve_matches_dest=()
#
# tries to find a filename in the library hierarchy.
function resolve_filename {
  local code_file=$1
#echo resolving: $code_file
  if [ -f "$code_file" ]; then
    # that was pretty easy.
    resolve_target_array=($code_file)
    return 0
  fi
#echo "MUST seek: $code_file"

  local dir=$(dirname "$code_file")
  local base=$(basename "$code_file")
  local src_key="$dir/$base"
#echo "src_key: $src_key"

  # see if we can find that element in the previously resolved items.
  if find_in_array "$src_key" ${resolve_matches_src[*]}; then
    local found_indy=$__finders_indy
    resolve_target_array=(${resolve_matches_dest[$found_indy]})
#echo "FOUND \"$src_key\" AT ${resolve_matches_dest[$found_indy]}"
    return 0
  fi

  # reset our global list.
  resolve_target_array=()
#echo "HAVING TO FIND: $dir and $base"
  if [ -z "$dir" ]; then
    resolve_target_array=($(find "$BUILD_TOP" -iname "$base"))
  else
    resolve_target_array=($(find "$BUILD_TOP" -iname "$base" | grep "$dir.$base"))
  fi
#echo resolved to: ${resolve_target_array[*]}
#echo size of resolve array=${#resolve_target_array[*]}
  if [ ${#resolve_target_array[*]} -eq 1 ]; then
#echo ADDING a match: $src_key ${resolve_target_array[0]}
    # for unique matches, we will store the correspondence so we can look
    # it up very quickly later.
    resolve_matches_src+=($src_key)
    resolve_matches_dest+=(${resolve_target_array[0]})
  fi
}
#
############################################################################

# main function that recurses on files and their dependencies.
# this takes a list of file names to examine.  each one will have its
# dependencies crawled.  we attempt to recurse on as few items as possible
# by making sure we haven't already seen files or decided they're bad.
function recurse_on_deps {
  # snag arguments into a list of dependencies to crawl.
  local -a active_deps=($*)

  # pull off the first dependency so we can get all of its includes.
  local first_element="${active_deps[0]}"
  active_deps=(${active_deps[*]:1})

  # make the best guess we can at the real path.
  resolve_filename $first_element
  local to_examine="${resolve_target_array[0]}"

  # we didn't already have a failure (due to it being a bad file already
  # or other problems).  and once we execute the below code to grab the
  # file's dependencies, it really is boring and we never want to see it
  # again.
  boring_files+=($to_examine)

local dirtmp=$(dirname "$to_examine")
local basetmp=$(basename "$to_examine")
echo "dependent on: $(basename "$dirtmp")/$basetmp"
#hmmm: gather the dependencies listed in debugging line above into a
#      list that will be printed out at the end.

  ##########################################################################

  local current_includes="$(mktemp $TEMPORARIES_PILE/zz_buildor_deps4-$base.XXXXXX)"
  rm -f "$current_includes"

  local partial_file="$(mktemp $TEMPORARIES_PILE/zz_buildor_deps5-$base.XXXXXX)"
  rm -f "$partial_file"

  # find all the includes in this file and save to the temp file.
  while read -r spoon; do
    has_guard="$(echo "$spoon" \
        | sed -n -e 's/#ifdef __BUILD_STATIC_APPLICATION__/yep/p')" 
    if [ ! -z "$has_guard" ]; then
      # quit reading when we've seen the start of one of our guards.
      break
    fi
    # if we are okay with the line, save it to the temp file.
    echo "$spoon"
  done <"$to_examine" >"$partial_file"

  grep "^[ $TAB_CHAR]*#include.*" <"$partial_file" >>"$current_includes"

  rm "$partial_file"

#echo "grabbing includes from: $to_examine"

#hmmm: could separate the find deps on this file stuff below.

  local fp_dir=$(dirname "$to_examine")
#echo fp_dir is: $fp_dir

  # iterate across the dependencies we saw and add them to our list if
  # we haven't already.
  while read -r line_found; do
    local chew_toy=$(echo $line_found | sed -e 's/^[ \t]*#include *<\(.*\)>.*$/\1/')
    # we want to add the file to the active list before we forgot about it.
#echo A: chew_toy=$chew_toy

    # check whether the dependency looks like one of our style of includes.
    # if it doesn't have a slash in it, then we need to give it the same
    # directory as the file we're working on.
    local slash_present=$(echo $chew_toy | sed -n -e 's/.*[\\\/].*/yep/p')

    # the replacement above to get rid of #include failed.  try something
    # simpler.
    if [ ! -z "$(echo $chew_toy | sed -n -e 's/#include/crud/p')" ]; then
      # try again with a simpler pattern.
      chew_toy=$(echo $line_found | sed -e 's/^[ \t]*#include *[">]\(.*\)[">].*$/\1/') 
#echo B: chew_toy=$chew_toy

      # if it still has an #include or if it's not really a file, we can't
      # use it for anything.
      if [ ! -z "$(echo $chew_toy | sed -n -e 's/#include/crud/p')" ]; then
        echo "** bad include: $chew_toy"
        continue
      fi

      # we are pretty sure that this file has no path components in it.
      # we will add the surrounding directory if possible.
      if [ -z "$slash_present" ]; then
        if [ -z "$fp_dir" ]; then
          # well, now we have no recourse, since we don't know where to
          # say this file comes from.
          echo "** unknown directory: $chew_toy"
        else
          # cool, we can rely on the existing directory.
          chew_toy="$fp_dir/$chew_toy"
#echo patched dir: $chew_toy
        fi
      fi
    fi

    if bad_file $chew_toy; then
#echo C: skipping because on bad list: $chew_toy
      continue
    fi

###  # if we've seen it before, we bail.
###  if seen_already "$to_examine"; then 
###echo bailing since seen before: $to_examine
###return 0;
### fi

  # now remember that we've seen this file.  we only remember it if
    # make sure we can see this file already, or we will need to seek it out.
    if [ ! -f "$chew_toy" ]; then
      # not an obvious filename yet.  try resolving it.
      resolve_filename $chew_toy
      declare -a found_odd=(${resolve_target_array[*]})
#echo found-list-is: ${found_odd[*]}
      local odd_len=${#found_odd[*]}
#echo odd len is $odd_len
      if [ $odd_len -eq 0 ]; then
        # whoops.  we couldn't find it.  probably a system header, so toss it.
#echo "** ignoring: $chew_toy"
        bad_files+=($chew_toy)
        chew_toy=""
      elif [ $odd_len -eq 1 ]; then
        # there's exactly one match, which is very good.
        chew_toy="${found_odd[0]}"
#echo C: chew_toy=$chew_toy
      else
        # this is really wrong.  there are multiple files with the same name?
        # that kind of things makes debugger tools angry or stupid.
        echo "** non-unique name: $chew_toy"
        bad_files+=($chew_toy)
        chew_toy=""
      fi
    fi

    if [ ! -z "$chew_toy" -a ! -f "$chew_toy" ]; then
      echo "** failed to compute a real path for: $chew_toy"
      bad_files+=($chew_toy)
      chew_toy=""
      continue
    fi

    # now if we got something out of our patterns, add it as a file to
    # investigate.
    if [ ! -z "$chew_toy" ]; then
      # add the dependency we found.
      if add_new_dep "$chew_toy"; then
        # if that worked, it's not existing or bad so we want to keep it.
        if ! already_listed "$chew_toy" ${active_deps[*]}; then
          # track the file for its own merits also (to squeeze more includes).
          active_deps+=($chew_toy)
        fi
      fi
    fi

    # now compute the path as if it was the implementation file (x.cpp)
    # instead of being a header.  does that file exist?  if so, we'd like
    # its dependencies also.
    local cpp_toy=$(echo $chew_toy | sed -e 's/^\([^\.]*\)\.h$/\1.cpp/')

    # there's no point in adding it if the name didn't change.
    if [ "$cpp_toy" != "$chew_toy" ]; then
      resolve_filename $cpp_toy
#hmmm: what if too many matches occur?
      found_it="${resolve_target_array[0]}"

      # if the dependency actually exists, then we'll add it to our list.
      if [ ! -z "$found_it" ]; then
        if add_new_dep "$found_it"; then
          # that was a new dependency, so we'll continue examining it.
          if ! already_listed "$found_it" ${active_deps[*]}; then
            active_deps+=($found_it)
          fi
        fi
      fi
    fi
  done <"$current_includes"

  rm -f "$current_includes"

  # keep going on the list after our modifications.
  if [ ${#active_deps[*]} -ne 0 ]; then recurse_on_deps ${active_deps[*]}; fi
  return 0
}

# this takes the dependency list and adds it to our current file.
function write_new_version {
  local code_file=$1

  local opening_guard_line="\n#ifdef __BUILD_STATIC_APPLICATION__\n  // static dependencies found by buildor_gen_deps.sh:"
  local closing_guard_line="#endif // __BUILD_STATIC_APPLICATION__\n"

#echo "would write deps to: $code_file"
#echo ${dependency_accumulator[*]}

  local replacement_file="$(mktemp $TEMPORARIES_PILE/zz_buildor_deps3.XXXXXX)"

  # blanks is a list of blank lines that we save up in between actual content.
  # if we don't hold onto them, we can have the effect of "walking" the static
  # section down the file as progressively more blanks get added.  we ensure
  # that only one is between the last code line and the guarded static chunk.
  declare -a blanks=()
  # read in our existing file.
  while read -r orig_line; do
#echo "read: '$orig_line'"
    # if it's the beginning of our static app section, stop reading.
    if [ ! -z "$(echo $orig_line \
        | sed -n -e 's/#ifdef __BUILD_STATIC_APPLICATION__/yep/p')" ]; then
      break
    fi
    if [ -z "$orig_line" ]; then
      # add another blank line to our list and don't print any of them yet.
      blanks+=($'\n')
    else
      # this line is not a blank; send any pending blanks to the file first.
      if [ ${#blanks[*]} -ne 0 ]; then
        echo -n ${blanks[*]} >>"$replacement_file"
      fi
      echo "$orig_line" >>"$replacement_file"
      # reset our list of blank lines, since we just added them.
      blanks=()
    fi
  done <"$code_file"

  echo -e "$opening_guard_line" >>"$replacement_file"

  # now accumulate just the dependencies for a bit.
  local pending_deps="$(mktemp $TEMPORARIES_PILE/zz_buildor_deps2.XXXXXX)"
  rm -f "$pending_deps"

  # iterate across all the dependencies we found.
  for line_please in ${dependency_accumulator[*]}; do
    
    # throw out any items that are in the same directory we started in.
    if [ "$prohibited_directory" == "$(dirname $line_please)" ]; then
#echo "skipping prohibited: $line_please"
      continue
    fi

    # strip the line down to just the filename and single directory component.
    local chewed_line=$(echo $line_please | sed -e 's/.*[\\\/]\(.*\)[\\\/]\(.*\)$/\1\/\2/')

    if [ ! -z "$(echo $chewed_line | sed -n -e 's/\.h$/yow/p')" ]; then
#echo skipping header file: $chewed_line
      continue
    fi

    local new_include="  #include <$chewed_line>"
    echo "$new_include" >>"$pending_deps"
#echo adding "$new_include" 
  done

  sort "$pending_deps" >>"$replacement_file"
  rm -f "$pending_deps"

  echo -e "$closing_guard_line" >>"$replacement_file"

#echo "about to move replacement, diffs:"
#diff "$replacement_file" "$code_file"
#echo "--------------"
#echo full file:
#cat "$replacement_file"
#echo "--------------"

  mv "$replacement_file" "$code_file"
}

function find_dependencies {
  local code_file=$1

  # initialize our globals.
  dependency_accumulator=()
  boring_files=()

  # start recursing with the first dependency being the file itself.
  recurse_on_deps $code_file

  # create the new version of the file.
  write_new_version "$code_file"
}

# main script starts here.

for curr_parm in $*; do 

  echo "----------------------------------------------------------------------------"
  echo ""

  # resets the bad list in between sessions.
  bad_files=() 
#echo bad_files initial: ${bad_files[*]} 

  if [ -f "$curr_parm" ]; then
    echo "scanning file: $curr_parm"
    # get absolute path of the containing directory.
    prohibited_directory="$(pwd "$curr_parm")"
    # fix our filename to be absolute.
    temp_absolute="$prohibited_directory/$(basename "$curr_parm")"
    curr_parm="$temp_absolute"
#echo "curr_parm: $curr_parm"
    find_dependencies "$curr_parm"
  elif [ -d "$curr_parm" ]; then
    echo "scanning folder: $curr_parm"
    # get absolute path of the containing directory.
    prohibited_directory="$(pwd $curr_parm)"
    # set the directory to that absolute path.
    curr_parm="$prohibited_directory"
#echo "curr_parm: $curr_parm"
    outfile="$(mktemp $TEMPORARIES_PILE/zz_buildor_deps1.XXXXXX)"
    find "$curr_parm" -iname "*.cpp" >"$outfile"
    while read -r line_found; do
      if [ $? != 0 ]; then break; fi
#echo "looking at file: $line_found"
      find_dependencies "$line_found"
    done <"$outfile"
    rm -f "$outfile"
  else
    echo "parameter is not a file or directory: $curr_parm"
  fi

  echo "ignored: " ${bad_files[*]}

  echo ""
  echo ""

done


