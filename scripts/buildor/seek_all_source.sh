#!/usr/bin/env bash
# this script traverses the directories passed as its parameters and locates
# any files that we consider to possibly be source code.
# a new variable called "SOURCES_FOUND_LIST" will be declared and will point at
# the list of files.  this file should be deleted when the caller is done.
# due to the variable creation requirement, this script should only be included
# in other bash scripts (via 'source' or '.').

declare find_src_parms=$*
  # grab the parameters passed to us.
if [ -z "${find_src_parms[0]}" ]; then
  # if there were no parameters, we will just do the current directory.
  find_src_parms[0]="."
fi

#echo "full find_src_parms=${find_src_parms[@]}"

if [ -z "$SOURCES_FOUND_LIST" ]; then
  export SOURCES_FOUND_LIST="$(mktemp "$TMP/zz_temp_find_srcs.XXXXXX")"
fi
# clean up the file to start with.
\rm $SOURCES_FOUND_LIST &>/dev/null

for i in $find_src_parms; do
#echo current dir is $i
  find "$i" -type f -a \( \
    -iname "*.c" \
    -o -iname "*.cfg" \
    -o -iname "*.conf" \
    -o -iname "*.cpp" \
    -o -iname "*.css" \
    -o -iname "*.csv" \
    -o -iname "*.def" \
    -o -iname "*.fn2" \
    -o -iname "*.h" \
    -o -iname "*.html" \
    -o -iname "*.ini" \
    -o -iname "*.inl" \
    -o -iname "*.java" \
    -o -iname "*.js" \
    -o -iname "*.lst" \
    -o -iname "*.mak" \
    -o -iname "makefile*" \
    -o -iname "*.man" \
    -o -iname "*.pl" \
    -o -iname "*.r[hc]" \
    -o -iname "*.rc2" \
    -o -iname "readme*" \
    -o -iname "*.sh" \
    -o -iname "*.sln" \
    -o -iname "*.txt" \
    -o -iname "*.vcf" \
    -o -iname "*.xpm" \
    -o -iname "*.xml" \
    \) \
  | grep -v ".*\.svn.*" >>$SOURCES_FOUND_LIST
done

