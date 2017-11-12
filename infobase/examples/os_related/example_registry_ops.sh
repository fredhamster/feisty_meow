#!/bin/sh
# an example of using the reg.exe tool on windows to find some things in
# the registry.
# this happens to look in the registry for PuTTY keys for a set of named
# displays.

declare ip_array=(zorba-1 zorba-2 zorba-3 zorba-4 zorba-5)

for i in ${ip_array[*]}; do 
  target=${i}
  echo "display is $target"
  reg query 'HKCU\Software\SimonTatham\PuTTY\SshHostKeys' /v "rsa2@22:$target"
done


