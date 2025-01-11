#!/usr/bin/env bash
# this script connects to a remote machine and records the IP address of the
# local machine there.  this enables the machine's owner to connect back to
# the system even if the IP address floats around (changes).

server="$1"; shift
remote_user="$1"; shift
local_user="$1"; shift

if [ -z "$server" -o -z "$remote_user" -o -z "$local_user" ]; then
  echo "This script will record the IP address for 'this' host into a file on a"
  echo "remote computer (that is running ssh server).  To perform this feat, the"
  echo "following parameters are required:"
  echo "  $(basename $0) {server} {remote-user} {local-user}"
  echo "Note that this script must be run as root, but it uses the local user's"
  echo "capability to connect to the remote system without a password (given the"
  echo "user's possession of an ssh certificate on the remote host).  The remote"
  echo "user, in other words, must have an entry in the ssh authorized_keys that"
  echo "allows the local user to connect."
  exit 1
fi

ip_file="$(hostname | sed -e "s/\..*$//")_ip_address.txt"

# go over to the server and write a file recording our IP address.
# note that the local_user here is expected to have a certificate that
# gets us access to the server.  this needs to be on the local machine
# to enable ssh to run without a login prompt.
sudo -u $local_user ssh $remote_user@$server <<eof
  if [ ! -d gen ]; then mkdir gen; fi
  cd gen
  echo "\$SSH_CLIENT" | awk '{ print \$1; }' >$ip_file
eof

