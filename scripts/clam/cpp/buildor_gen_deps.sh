#!/usr/bin/env bash

####
#  Name   : buildor_gen_deps
#  Author : Chris Koeritz
#  Rights : Copyright (C) 2008-$now by Author
#  Purpose:
#    this script finds all of the headers used by a cpp file and outputs a
#    list of other cpp files that are probably needed for building it.
####
#  This script is free software; you can redistribute it and/or modify it
#  under the terms of the GNU General Public License as published by the Free
#  Software Foundation; either version 2 of the License or (at your option)
#  any later version.  See "http://www.fsf.org/copyleft/gpl.html" for a copy
#  of the License online.  Please send any updates to "fred@gruntose.com".
####

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

# define some variables to avoid going nuts seeing 0 and 1 everywhere.
RET_OKAY=0
RET_FAIL=1

# prints out a message with a time-stamp prefixed to it.
function log_it()
{
  echo -e "$(date_stringer): $@"
}

if [ ! -z "$CLEAN" ]; then
  log_it "in cleaning mode, will not build dependencies."
  exit $RET_OKAY
fi

# uncomment to enable debugging noises.
#DEBUG_BUILDOR_GEN_DEPS=yo

# these semi-global variables used throughout the whole script to accumulate
# information, rather than trying to juggle positional parameters everywhere.

# the list of dependencies being accumulated.
declare -A dependency_accumulator

# a set of files that are known to be bad, since we cannot find them.
declare -A bad_files

# makes sure we don't keep looking at files even when they're neither
# bad nor listed as dependencies.
declare -A boring_files

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
  if existing_dep "$1"; then return $RET_OKAY; fi  # added it to list already.
  if bad_file "$1"; then return $RET_OKAY; fi  # known to suck.
  if boring_file "$1"; then return $RET_OKAY; fi  # we already saw it.
  return $RET_FAIL  # we had not seen this one, so we return an error.
}

# adds a new dependency at the end of the list.
function add_new_dep {
  # make sure we haven't already processed this.
  local dep="$1"; shift
  if seen_already "$dep"; then
    if [ ! -z "$DEBUG_BUILDOR_GEN_DEPS" ]; then
      log_it "bailing since seen: $dep"
    fi
    return $RET_FAIL
  fi
  if [ ! -z "$DEBUG_BUILDOR_GEN_DEPS" ]; then
    log_it "had not seen before: $dep"
  fi

  if [ ! -z "$DEBUG_BUILDOR_GEN_DEPS" ]; then
    log_it "new dependency: $dep"
  fi

  dependency_accumulator[$dep]=yo
  return $RET_OKAY
}

# checks the existing dependencies to see if the first parameter is already
# listed.  if this is the case, zero is returned (meaning success).  if
# the dependency is missing, then -1 is return to indicate an error.
function existing_dep {
  if [ ! -z "${dependency_accumulator[$1]}" ]; then
    return $RET_OKAY
  fi
  return $RET_FAIL
}

# reports whether a file name has already been processed.
function boring_file {
  if [ ! -z "${boring_files[$1]}" ]; then
    return $RET_OKAY
  fi
  return $RET_FAIL
}

# reports whether a file name has already been found to be missing.
function bad_file {
  if [ ! -z "${bad_files[$1]}" ]; then
    return $RET_OKAY
  fi
  return $RET_FAIL
}

############################################################################
#
# this variable gets stored into when resolve_filename runs.
declare -A resolve_target_array
#
# this variable is used internally by resolve_filename.  it should not need
# to be reset between runs on different files because the source hierarchy
# is not supposed to be getting files deleted or added while the deps are
# being geneated.
declare -A resolve_matches
#
# tries to find a filename in the library hierarchy.
function resolve_filename() {
  # wipe out prior global contents from resolved paths.
  unset resolve_target_array
  declare -gA resolve_target_array

  local code_file=$1
  if [ ! -z "$DEBUG_BUILDOR_GEN_DEPS" ]; then
    log_it "resolving: $code_file"
  fi
  if [ -f "$code_file" ]; then
    # that was pretty easy.
    resolve_target_array[$code_file]=zuh
    return $RET_OKAY
  fi
  if [ ! -z "$DEBUG_BUILDOR_GEN_DEPS" ]; then
    log_it "MUST seek: $code_file"
  fi

  local dir=$(dirname "$code_file")
  if [ "$dir" == "." ]; then
    # we plan to behave as if there was no directory for this case.
    # we can't include a simple dot in our searches below.
    unset dir
  fi
  local base=$(basename "$code_file")
  local src_key="$dir/$base"
  if [ -z "$dir" ]; then
    # with no directory, or current directory, this just has to be keyed by the basename.
    src_key="$base"
  fi
  if [ ! -z "$DEBUG_BUILDOR_GEN_DEPS" ]; then
    log_it "src_key: $src_key"
  fi

  # see if we can find that element in the previously resolved items.
  if [ ! -z "${resolve_matches[$src_key]}" ]; then
    local flounder="${resolve_matches[$src_key]}"
    resolve_target_array[$flounder]=blup
    if [ ! -z "$DEBUG_BUILDOR_GEN_DEPS" ]; then
      log_it "FOUND \"$src_key\" AT $flounder"
    fi
    return $RET_OKAY
  fi

  if [ ! -z "$DEBUG_BUILDOR_GEN_DEPS" ]; then
    log_it "HAVING TO FIND: dir='$dir' and base='$base'"
  fi
  if [ -z "$dir" ]; then
    local -a init_list="$(find "$BUILD_TOP" -iname "$base" | sed -e "s%\\(.*\\)%[\1]=lop %")"
#log_it "no dir case, init list is: ${init_list}"
    if [ ${#init_list} -gt 0 ]; then
      eval resolve_target_array=(${init_list})
#log_it "after no dir case addition, resolve targets are: " ${!resolve_target_array[@]}
    fi
  else
    # a little tricky here, since we can't emit the associative array init line too early, or it will match for the grep.
    local -a init_list="$(find "$BUILD_TOP" -iname "$base" | grep "$dir.$base" | sed -e "s%\\(.*\\)%[\1]=oof %")"
#log_it "with dir case, init list is: ${init_list}"
    if [ ${#init_list} -gt 0 ]; then
      eval resolve_target_array=(${init_list})
#log_it "after dir case addition, resolve targets are: " ${!resolve_target_array[@]}
    fi
  fi
  if [ ! -z "$DEBUG_BUILDOR_GEN_DEPS" ]; then
    log_it "resolved to: ${!resolve_target_array[@]}"
    log_it "size of resolve array=${#resolve_target_array[@]}"
  fi
  if [ ${#resolve_target_array[@]} -eq 1 ]; then 
    local -a indies=( ${!resolve_target_array[@]} )
    local first_item=${indies[0]}
    if [ ! -z "$DEBUG_BUILDOR_GEN_DEPS" ]; then
      log_it ADDING a match: [$src_key]=$first_item
    fi
    # for unique matches, we will store the correspondence so we can find it very quickly later.
    resolve_matches[$src_key]="$first_item"
    return $RET_OKAY
  fi
  return $RET_FAIL
}
#
############################################################################

# main function that iterates on files and their dependencies.  this takes
# the variable name of a list of file names to examine.  each one will have
# its dependencies crawled.  we attempt to recurse on as few items as possible
# by making sure we haven't already seen files or decided they're bad.
function recurse_on_deps()
{
  # the name of the list of dependencies to crawl is passed to us, initially with just
  # one item.  it's important to realize that we're getting the external variable's name
  # and then accessing it locally via the alias "active_deps" below.  as we find new items
  # for the list, we add them to it.  when an item has been totally processed, it's removed
  # from the list.
  local -n active_deps="$1"; shift

  while [ ${#active_deps[@]} -ne 0 ]; do
    # pull off the first dependency so we can get all of its includes.
    local -a indies=( ${!active_deps[@]} )
    local first_element=${indies[0]}
    # chop the element we're working on out of the active list.
    unset active_deps[$first_element]
    # invoke our workhorse method on the item.
    chew_on_one_dependency ${!active_deps} "$first_element"
  done
  return 0
}

# processes one file to locate all of its dependencies.
# the external global list will be updated as this runs.
function chew_on_one_dependency()
{
  local -n active_deps="$1"; shift
  local first_element="$1"; shift

  # make the best guess we can at the real path.
  if ! resolve_filename $first_element; then
    log_it "-- FAILED to resolve the filename for '$first_element'"
    return $RET_FAIL
  fi
  local -a indies=( ${!resolve_target_array[@]} )
  local to_examine=${indies[0]}
  if [ ! -z "$DEBUG_BUILDOR_GEN_DEPS" ]; then
    log_it "resolved path to_examine is '$to_examine'"
  fi

  # we didn't already see a failure (due to it being a bad file already
  # or other problems).  and once we execute the below code to grab the
  # file's dependencies, the file really will be boring and we never want
  # to see it again in our processing.
  boring_files[$to_examine]=yawn

  local dirtmp=$(dirname "$to_examine")
  local basetmp=$(basename "$to_examine")

  log_it "++ dependent on: $(basename "$dirtmp")/$basetmp"
#hmmm: do a better, nicer output--gather the dependencies listed in debugging
#      line above into a list that will be printed out at the end.

  ##########################################################################

  local current_includes="$(mktemp $TEMPORARIES_PILE/zz_buildor_deps_includes_${basetmp}.XXXXXX)"
  \rm -f "$current_includes"

  local partial_file="$(mktemp $TEMPORARIES_PILE/zz_buildor_deps_filepart_${basetmp}.XXXXXX)"
  \rm -f "$partial_file"

  if [ ! -z "$DEBUG_BUILDOR_GEN_DEPS" ]; then
    log_it "\n\n========\nprocessing file '$to_examine'\n========"
  fi

  # find all the includes in this file and save to the temp file.
  while read -r spoon; do
    if [[ $spoon == *"#ifdef __BUILD_STATIC_APPLICATION__"* ]]; then
      # quit reading when we've seen the start of one of our guards.
      break
    fi
    # if we are okay with the line, save it to the temp file.
    echo "$spoon"
  done <"$to_examine" >"$partial_file"

  grep "^[ $TAB_CHAR]*#include.*" <"$partial_file" >>"$current_includes"

  \rm "$partial_file"

  if [ ! -z "$DEBUG_BUILDOR_GEN_DEPS" ]; then
    log_it "grabbing includes from: $to_examine"
  fi

#hmmm: could separate the find deps on this file stuff below.

  local fp_dir=$(dirname "$to_examine")

  # iterate across the dependencies we saw and add them to our list if we haven't already.
  while read -r line_found; do
    # process the line to see if we can get a simple filename out of the include.
    # we are only trying for system-searched files for this one, with angle brackets.
    local chew_toy="${line_found#*\#include *<}"
    chew_toy="${chew_toy/>*}"
    if [ ! -z "$DEBUG_BUILDOR_GEN_DEPS" ]; then
      log_it A: chew_toy=$chew_toy
    fi

    # check whether the dependency looks like one of our style of includes.
    # if it doesn't have a slash in it, then we need to give it the same
    # directory as the file we're working on.
    local slash_present="${chew_toy/[^\/]*/}"

    if [[ $chew_toy == *"#include"* ]]; then
      # the replacement above to get rid of #include failed.  try something
      # more inclusive, so we match double quote includes also.
      chew_toy="${chew_toy#*\#include *[\"<]}"
      chew_toy="${chew_toy/[\">]*}"
      if [ ! -z "$DEBUG_BUILDOR_GEN_DEPS" ]; then
        log_it B: chew_toy=$chew_toy
      fi

      # if it still has an #include or if it's not really a file, we can't
      # use it for anything.
      if [[ $chew_toy == *"#include"* ]]; then
        log_it "-- BAD include: $chew_toy"
        continue
      fi

      if [ -z "$slash_present" ]; then
        # we are pretty sure that this file has no path components in it.
        # we will add the surrounding directory if possible.
        if [ -z "$fp_dir" ]; then
          # well, now we have no recourse, since we don't know where to
          # say this file comes from.
          log_it "-- UNKNOWN directory: $chew_toy"
        else
          # cool, we can rely on the existing directory.
          chew_toy="$fp_dir/$chew_toy"
          if [ ! -z "$DEBUG_BUILDOR_GEN_DEPS" ]; then
            log_it "patched dir: $chew_toy"
          fi
        fi
      fi
    fi

    if bad_file $chew_toy; then
      if [ ! -z "$DEBUG_BUILDOR_GEN_DEPS" ]; then
        log_it "C: skipping because on bad list: $chew_toy"
      fi
      continue
    fi

    # now remember that we've seen this file.  we only remember it if
    # make sure we can see this file already, or we will need to seek it out.
    if [ ! -f "$chew_toy" ]; then
      # not an obvious filename yet.  try resolving it.
      resolve_filename $chew_toy
      declare -a found_odd=(${!resolve_target_array[@]})
      local odd_len=${#found_odd[*]}
      if [ $odd_len -eq 0 ]; then
        # whoops.  we couldn't find it.  probably a system header, so toss it.
        log_it "-- ignoring missing file: $chew_toy"
        bad_files[$chew_toy]=yup
        chew_toy=""
      elif [ $odd_len -eq 1 ]; then
        # there's exactly one match, which is very good.
        chew_toy="${found_odd[0]}"
        if [ ! -z "$DEBUG_BUILDOR_GEN_DEPS" ]; then
          log_it "C: chew_toy=$chew_toy"
        fi
      else
        # this is really wrong.  there are multiple files with the same name?
        # that kind of things makes debugger tools angry or stupid.
        log_it "-- NON-UNIQUE name: $chew_toy"
        bad_files[$chew_toy]=urf
        chew_toy=""
      fi
    fi

    if [ ! -z "$chew_toy" -a ! -f "$chew_toy" ]; then
      log_it "-- FAILED to compute a real path for: $chew_toy"
      bad_files[$chew_toy]=meh
      chew_toy=""
      continue
    fi

    # now if we got something out of our patterns, add it as a file to investigate.
    # it is reasonable for the chew toy to be empty here, given the work we do above,
    # hence the emptiness check.
    if [ ! -z "$chew_toy" ]; then
      # add the dependency we found.
      if add_new_dep "$chew_toy"; then
        # if that worked, it's not existing or bad so we want to keep it.
#        if ! already_listed "$chew_toy" ${active_deps[*]}; then
        if [ -z "${active_deps[$chew_toy]}" ]; then
          # track the file for its own merits also (to squeeze more includes).
          active_deps[$chew_toy]=fiz
        fi
      fi

      # now compute the path as if it was the implementation file (x.cpp)
      # instead of being a header.  does that file exist?  if so, we'd like
      # its dependencies also.
      local cpp_toy="${chew_toy%.h}.cpp"  # sweet and fast using just bash variable expansion.
      if [ ! -z "$DEBUG_BUILDOR_GEN_DEPS" ]; then
        log_it "cpp_toy is '$cpp_toy' as derived from chew_toy '$chew_toy'"
      fi

      # there's no point in adding it if the name didn't change.
      if [ "$cpp_toy" != "$chew_toy" ]; then
        resolve_filename $cpp_toy
        local -a indies=( ${!resolve_target_array[@]} )
        local found_it="${indies[0]}"
#hmmm: what if too many matches occur?

        # if the dependency actually exists, then we'll add it to our list.
        if [ ! -z "$found_it" ]; then
          if add_new_dep "$found_it"; then
            # that was a new dependency, so we'll continue examining it.
#            if ! already_listed "$found_it" ${active_deps[*]}; then
            if [ -z "${active_deps[$found_it]}" ]; then
#              active_deps+=($found_it)
              active_deps[$found_it]=pop
            fi
          fi
        fi
      fi
    fi
  done <"$current_includes"

  \rm -f "$current_includes"

#  # keep going on the list after our modifications.
#  if [ ${#active_deps[@]} -ne 0 ]; then
#    recurse_on_deps ${active_deps[@]}
#  fi

  return $RET_OKAY
}

# this takes the dependency list and adds it to our current file.
function write_new_version {
  local code_file=$1

  local opening_guard_line="\n#ifdef __BUILD_STATIC_APPLICATION__\n  // static dependencies found by buildor_gen_deps.sh:"
  local closing_guard_line="#endif // __BUILD_STATIC_APPLICATION__\n"

#  if [ ! -z "$DEBUG_BUILDOR_GEN_DEPS" ]; then
    log_it "++ writing new dependencies to: $code_file"
#  fi

  local base="$(basename "$code_file")"
  local replacement_file="$(mktemp $TEMPORARIES_PILE/zz_buildor_deps_replacement_${base}.XXXXXX)"

  # blanks is a list of blank lines that we save up in between actual content.
  # if we don't hold onto them, we can have the effect of "walking" the static
  # section down the file as progressively more blanks get added.  we ensure
  # that only one is between the last code line and the guarded static chunk.
  declare -a blanks=()
  # read in our existing file.
  while read -r orig_line; do
    # if it's the beginning of our static app section, stop reading.
    if [[ $orig_line == *"#ifdef __BUILD_STATIC_APPLICATION__"* ]]; then
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
  local pending_deps="$(mktemp $TEMPORARIES_PILE/zz_buildor_deps_pendingdeps_${base}.XXXXXX)"
  \rm -f "$pending_deps"

  # iterate across all the dependencies we found.
  for line_please in "${!dependency_accumulator[@]}"; do
    # throw out any items that are in the same directory we started in.
    local the_dir="$(dirname $line_please)"
    local the_base="$(basename $line_please)"
    if [ "$prohibited_directory" == "$the_dir" ]; then
      if [ ! -z "$DEBUG_BUILDOR_GEN_DEPS" ]; then
        log_it "skipping prohibited: $line_please"
      fi
      continue
    fi

    # strip the line down to just the filename and single directory component.
#    local chewed_line=$(echo $line_please | sed -e 's/.*[\\\/]\(.*\)[\\\/]\(.*\)$/\1\/\2/')
    local chewed_line="$(echo "$(basename "$the_dir")/$the_base" )"
#log_it chewed_line became: $chewed_line

    # see if this matches the header file ending; we don't want to add those to the cpp code.
    if [[ "$chewed_line" =~ ^.*\.h$ ]]; then
      if [ ! -z "$DEBUG_BUILDOR_GEN_DEPS" ]; then
        log_it "skipping header file: $chewed_line"
      fi
      continue
    fi

    # this one seems like a good code file to add, so chuck it in there.
    local new_include="  #include <$chewed_line>"
    if [ ! -z "$DEBUG_BUILDOR_GEN_DEPS" ]; then
      log_it "adding '$new_include'"
    fi
    echo "$new_include" >>"$pending_deps"
  done

  # check that our dependencies file is not empty still.
  if [ ! -s "$pending_deps" ]; then
    log_it "
We encountered a problem during the generation of dependencies.
The temporary output file:
  '${pending_deps}'
was still empty after the dependency generation process.  This is a failure
to find any dependencies and would result in writing an empty list into the
file (possibly clobbering a perfectly fine existing list of generated
dependencies).  So, we're bailing now.  Please resolve the issue in either
the current code file:
  '${code_file}'
or within this script itself:
  '$0'
"
    exit $RET_FAIL
  fi

  sort "$pending_deps" >>"$replacement_file"
  exit_on_error "sorting pending deps into the replacement file"
  \rm -f "$pending_deps"

  echo -e "$closing_guard_line" >>"$replacement_file"

  if [ ! -z "$DEBUG_BUILDOR_GEN_DEPS" ]; then
    log_it "about to replace file.  here are the diffs:"
    log_it "--------------"
    diff "$replacement_file" "$code_file"
    log_it "--------------"
  fi

  \mv "$replacement_file" "$code_file"
  exit_on_error "replacing the original file with updated dependency version"
}

function find_dependencies {
  local code_file=$1

  # initialize our globals.
  unset dependency_accumulator
  declare -gA dependency_accumulator
  unset boring_files
  declare -gA boring_files

  # start recursing with the first dependency being the file itself.
  if [ ! -z "$DEBUG_BUILDOR_GEN_DEPS" ]; then
    log_it "\n\n========\nstarting recursion on dependencies...\n========"
  fi

  # set up our indirectly referenced dictionary of dependencies.
  unset global_active_dependencies
  declare -gA global_active_dependencies
  global_active_dependencies[$code_file]=go

  recurse_on_deps global_active_dependencies

  # create the new version of the file.
  if [ ! -z "$DEBUG_BUILDOR_GEN_DEPS" ]; then
    log_it "\n\n========\nwriting new version of code file...\n========"
  fi
  write_new_version "$code_file"
}

# main script starts here.

for curr_parm in $*; do 

  echo "----------------------------------------------------------------------------"
  echo ""

  # resets the bad list in between sessions.
  unset bad_files
  declare -gA bad_files

  if [ -f "$curr_parm" ]; then
    log_it "scanning file: $curr_parm"
    prohibited_directory="$(dirname "$curr_parm")"
    # get the absolute path of the containing directory with our freaky pwd trick.
    prohibited_directory="$( \cd "$prohibited_directory" && \pwd )"
    if [ ! -z "$DEBUG_BUILDOR_GEN_DEPS" ]; then
      log_it "for file, containing dir absolute is now: $prohibited_directory"
    fi
    # fix our filename to be absolute.
    temp_absolute="$prohibited_directory/$(basename "$curr_parm")"
    curr_parm="$temp_absolute"
    if [ ! -z "$DEBUG_BUILDOR_GEN_DEPS" ]; then
      log_it "curr_parm file: $curr_parm"
    fi
    find_dependencies "$curr_parm"
  elif [ -d "$curr_parm" ]; then
    log_it "scanning folder: $curr_parm"
    prohibited_directory="$(dirname "$curr_parm")"
    # get absolute path of the containing directory.
    prohibited_directory="$( \cd "$prohibited_directory" && \pwd )"
    if [ ! -z "$DEBUG_BUILDOR_GEN_DEPS" ]; then
      log_it "for dir, containing dir absolute is now: $prohibited_directory"
    fi
    # set the directory to that absolute path.
    curr_parm="$prohibited_directory"
    if [ ! -z "$DEBUG_BUILDOR_GEN_DEPS" ]; then
      log_it "curr_parm dir: $curr_parm"
    fi
    local base="$(basename "$curr_parm")"
    outfile="$(mktemp $TEMPORARIES_PILE/zz_buildor_deps_outfile_${base}.XXXXXX)"
    find "$curr_parm" -iname "*.cpp" >"$outfile"
    while read -r line_found; do
      if [ $? != 0 ]; then break; fi
      if [ ! -z "$DEBUG_BUILDOR_GEN_DEPS" ]; then
        log_it "looking at file: $line_found"
      fi
      find_dependencies "$line_found"
    done <"$outfile"
    \rm -f "$outfile"
  else
    log_it "-- parameter is not a file or directory: $curr_parm"
  fi

  log_it "++ ignored these files: " ${!bad_files[@]}

  echo ""
  echo ""

done

