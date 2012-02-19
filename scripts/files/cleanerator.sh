#!/bin/bash

cleaning_dirs=($*)

more_authoritative=( ~/feisty_meow ~/scavenging_inova/inovasoft )
###not yet ~/scavenging_inova/lightlink 

if [ -z "$TMP" ]; then TMP=/tmp; fi

# we generate the full directory list for the authoritative places into here.
zesty_authoritative_dirs=$(mktemp $TMP/cleanerator_authority.XXXXXX)
# we also make a list of all the directories under our cleaning locations here.
perky_scrubbing_dirs=$(mktemp $TMP/cleanerator_scrubbinz.XXXXXX)

# dump out the directories found in the authoritative areas.
for i in "${more_authoritative[@]}"; do
  real_dir="$(\cd "$i" && \pwd)"
  find "$real_dir" -type d | grep -v "\.svn" | grep -v "\.git" >>"$zesty_authoritative_dirs"
done

# now go to all of our places to clean, find all directories under them,
# and add them to our big list of places to scrub.
for i in "${cleaning_dirs[@]}"; do
  real_dir="$(\cd "$i" && \pwd)"
  find "$real_dir" -depth -type d | grep -v "\.svn" | grep -v "\.git" >>"$perky_scrubbing_dirs"
done

# load all the names from our files and do a massive N x M size loop.
while read i; do
#  echo "cleaning: $i"
  pushd "$i" &>/dev/null
#pwd
  while read j; do 
#    echo "against authority: $j"
    # try to clean it out of source control first.
    bash ~/feisty_meow/scripts/rev_control/svn_rm_dupes.sh "$j" "$i"
    # if that doesn't work, just whack it.
    bash ~/feisty_meow/scripts/files/whack_dupes.sh "$j" "$i"
  done <"$zesty_authoritative_dirs"
  popd &>/dev/null
done <"$perky_scrubbing_dirs"

# clean up afterwards.
rm -f "$zesty_authoritative_dirs"
rm -f "$perky_scrubbing_dirs"

