#!/usr/bin/env bash

host="$1"; shift
port="$1"; shift
if [ -z "$port" ]; then
  port=443
fi

if [ -z "$host" ]; then
  echo "This test takes at least a hostname parameter for testing, and will also"
  echo "accept an optional port parameter, e.g."
  echo "  $(basename $0) garvey.edu 17001"
  exit 1
fi

echo
echo "about to try connecting; if this fails to stay connected, then you are not"
echo "vulnerable to POODLE SSLv3 attack.  if it does connect, and you see the"
echo "protocol SSLv3 listed, then the server at $host:$port"
echo "is vulnerable to POODLE!"
echo
openssl s_client -ssl3 -host "$host" -port $port

exit 0


#could improve this by starting openssl connect in background 
# and awaiting its exit.  if it doesn't exit in like 3 seconds,
# then it probably connected.  at that point, print the error
# message about vulnerability found, and show where the output
# file from connect can be found for inspection.
