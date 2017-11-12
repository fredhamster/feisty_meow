#!/bin/bash
# This script finds the X window system authorization info that's already
# been established for this display.  The information can then be used to
# set the authorization for a new su session, which keeps X secure as
# well as allowing the user to run X programs on the original display.

# make sure we have some information to return in the first place.
if [ ! -z "$DISPLAY" ]; then 
  disp_search=$(echo $DISPLAY | sed -e 's/.*:\([0-9]*\).*/:\1/')
#echo disp search is $disp_search
  temp_auth=$(xauth list | grep -i $HOSTNAME | grep $disp_search)
#echo temp auth is $temp_auth
  temp_auth2=$(echo $temp_auth | sed -e "s/$HOSTNAME/; xauth add $HOSTNAME/g" )
#echo temp auth2 is $temp_auth2
  export X_auth_info="echo setting X permissions $temp_auth2"
#echo $X_auth_info

fi

