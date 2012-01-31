#!/bin/bash

# this program is run in the background to show a bye message on the console.

source "$FEISTY_MEOW_SCRIPTS/core/functions.sh"

export LIGHTWEIGHT_INIT=true  # make sure we just get our variables.
source $HOME/yeti/scripts/launch_feisty_meow.sh
export host=$(hostname)

# use this to always print the message.
#  export hostlist=$host

# only print on a few...
export hostlist='chandra ducky equanimity gulliver hamstergate \
  mycroft mrowrt numenor shaggy simmy slowboat velma wolfe \
\
  dervish frylock lagomorph shakezula '
#hmmm: this hostlist is highly dependent on my own favorite host names.
# to commoditize this, we should instead use a config file for the list.

# set our domain name based on what's found in the resolv.conf file, where
# dns info is often found.
DOMAIN_NAME=$(grep -i search </etc/resolv.conf | tail -1 | sed -n -e 's/domain.\(.*\)$/\1/p')
if [ -z "$DOMAIN_NAME" ]; then
  # second try, searching out the search domain as a suitable replacement
  # for the actual domain being specified.
  DOMAIN_NAME=$(grep -i search </etc/resolv.conf | tail -1 | sed -n -e 's/search.\(.*\)$/\1/p')
fi
# oh well, we don't know what the heck the domain is.
if [ -z "$DOMAIN_NAME" ]; then DOMAIN_NAME=unknown_domain; fi

echo domain is $DOMAIN_NAME

for i in $hostlist; do
  if [ "$host" = "$i" -o "$host" = "$i.$DOMAIN_NAME" ]; then
#    echo $(date_stringer)": $host matched." >>$HOME/host_matched_in_byejob.txt
    ( /bin/bash <<end
      sleep 7; bash $FEISTY_MEOW_SCRIPTS/users/byemessage.sh /dev/console
end
    )&
jobs
  fi
done

