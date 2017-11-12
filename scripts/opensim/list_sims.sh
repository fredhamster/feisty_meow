#!/bin/bash

# we guess at some nice values to use in the list.  if these are too small, we'll
# adjust them before printing anything.
max_name=14
max_port=10
max_location=12

# column headings, and a placeholder to be fixed later.
names=(Region hold)
numbers=(Port hold)
locats=('Grid Coords' hold)

# run through and get the data first...
for region_ini in $HOME/opensim/bin/Regions/*.ini; do
  name="$(grep "\[.*\]" <"$region_ini" | head -n 1 | tr -d \[\] | sed -e 's/[\r\n]//g' )"
  if [ ${#name} -gt $max_name ]; then max_name=${#name}; fi
  names[${#names[*]}]=$name
  port="$(grep "InternalPort" <"$region_ini" | head -n 1 | sed -e 's/InternalPort *= *//' -e 's/[\r\n]//g' )"
  if [ ${#port} -gt $max_port ]; then max_port=${#port}; fi
  numbers[${#numbers[*]}]=$port
  location="$(grep "Location" <"$region_ini" | head -n 1 | sed -e 's/Location *= *//' -e 's/[\r\n]//g' )"
  if [ ${#location} -gt $max_location ]; then max_location=${#location}; fi
  locats[${#locats[*]}]=$location
done

dash=
while [ ${#dash} -lt $max_name ]; do dash+='='; done
names[1]=$dash
dash=
while [ ${#dash} -lt $max_port ]; do dash+='='; done
numbers[1]=$dash
dash=
while [ ${#dash} -lt $max_location ]; do dash+='='; done
locats[1]=$dash

#echo names list is ${names[*]}
#echo ports list is ${numbers[*]}
#echo locations list is ${locats[*]}

# now print the data in a pleasant fashion...
indy=0
for region_ini in header line $HOME/opensim/bin/Regions/*.ini; do
  # indy is a zero-based array index.
  name=${names[$indy]}
  port=${numbers[$indy]}
  location=${locats[$indy]}
  while [ ${#name} -lt $max_name ]; do name+=' '; done
  while [ ${#port} -lt $max_port ]; do port+=' '; done
  while [ ${#location} -lt $max_location ]; do location+=' '; done
  if [ "$region_ini" == "header" ]; then
    region_ini='Region Config File'
  elif [ "$region_ini" == "line" ]; then
    region_ini='===================='
  fi
  echo "$name $port $location $(basename "$region_ini")";
  ((indy++))
done

exit 0

