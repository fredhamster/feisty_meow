#!/usr/bin/env bash

# finds all the resource ids in resource headers (only those named the
# canonical resource.h name) and discovers any duplicates.  the duplicates
# are shown with their symbolic names and file locations.

TEMP_RESOURCE_HEADERS=/tmp/resrc_headers_$USER.txt

sp='[ 	]'  # space and tab.

# find all the resource headers so we can look at their contents.
find "$BUILD_TOP" -type f -iname "resource.h" | \
  grep -vi 3rdparty | \
  grep -vi admin_items | \
  grep -v app >"$TEMP_RESOURCE_HEADERS"
#hmmm: above ignores *anything* with app in the name.
#  grep -v app_src >"$TEMP_RESOURCE_HEADERS"

FULLDEFS=/tmp/full_definition_list_$USER.txt
# clean up prior versions.
rm -f "$FULLDEFS"

# iterate through all the resource headers we found.
while read line; do
#echo "file=$line"
  # find any lines that define a resource id.  remove any that are part of
  # visual studio's tracking system for next id to assign (_APS_NEXT crud).
  chop_line="$(echo $line | sed -e 's/[\\\/]/+/g')"
  grep "^$sp*#define$sp*[_A-Za-z0-9][_A-Za-z0-9]*$sp*[0-9][0-9]*$sp*$" <"$line" | \
  grep -v "_APS_NEXT" | \
  grep -v "_APS_3D" | \
  grep -v "$sp*\/\/.*" | \
  sed -e "s/^$sp*#define$sp*\([_A-Za-z0-9][_A-Za-z0-9]*\)$sp*\([0-9][0-9]*\)$sp*$/\1=\2#$chop_line/" >>"$FULLDEFS"
done <"$TEMP_RESOURCE_HEADERS"

# our accumulated lists of names and ids (in order per list).
declare -a resource_names=()
declare -a resource_ids=()
declare -a resource_files=()

# iterate through the definitions list and compile the set of known ids.
while read line; do
  name=$(echo $line | sed -e 's/\([^=]*\)=[^#]*#.*/\1/')
  id=$(echo $line | sed -e 's/[^=]*=\([^#]*\)#.*/\1/')
  file=$(echo $line | sed -e 's/[^=]*=[^#]*#\(.*\)/\1/')

#echo got name $name
#echo got id $id
#echo got file $file
  next_index=${#resource_names[*]}
#echo next ind is $next_index
  resource_names[${next_index}]=$name
  resource_ids[${next_index}]=$id
  resource_files[${next_index}]=$id

done <"$FULLDEFS"

echo done reading all definitions.

JUST_IDS=/tmp/ids_list_$USER.txt
rm -f "$JUST_IDS"

i=0
while [[ i -le $next_index ]]; do
  echo ${resource_ids[$i]} >>"$JUST_IDS"
  ((i++))
done

echo done accumulating list of integer ids.

id_size=$(wc "$JUST_IDS")

JUST_IDS_TEMP=/tmp/ids_list_temp_$USER.txt

sort "$JUST_IDS" | uniq >"$JUST_IDS_TEMP"
id_temp_size=$(wc "$JUST_IDS_TEMP")
if [ "$id_size" == "$id_temp_size" ]; then
  echo "Your IDs are all unique!  Ending analysis."
  exit 0
fi

echo "Your ids are *NOT* all unique; the repeated ones are:"
sort "$JUST_IDS" | uniq -d >"$JUST_IDS_TEMP"

while read line; do
  id="$line"
  echo "=== identifier $id ==="
  grep "=$line#" "$FULLDEFS" | sed -e 's/\+/\//g' | sed -e 's/#/\
/'
done <"$JUST_IDS_TEMP"


