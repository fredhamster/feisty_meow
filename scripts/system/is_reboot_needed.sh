#!/usr/bin/env bash

# returns an error value if reboot IS required due to some recent
# updates.  otherwise returns a normal zero exit value indicating
# that things are peachy.

if [ -f /var/run/reboot-required ]; then
  echo 'reboot required'
  exit 1
fi
exit 0

#hmmm: add other OS versions, e.g.
#  redhat/centos: $ needs-restarting -r ; echo $?
#     shows if needs to be rebooted and returns an error if does need reboot
#  
