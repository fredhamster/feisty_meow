#!/bin/bash
# this script makes a tunnel for SMTP traffic and others.  a remote ssh server
# is required.  this is especially useful for routing around firewalls using
# a web proxy like squid.  when used for SMTP, it ensures that none of the
# text is seen on whatever network one is on before it's sent from the remote
# server.
#
# it accepts a single parameter so far, which will be used as the name of a
# sound file to play.

#hmmm:  none of the user info below will work for others: parameterize it.

#ssh_host=khandroma.cs.virginia.edu
ssh_host=mason.cs.virginia.edu

soundfile=$FEISTY_MEOW_APEX/infobase/sounds/woouoo.wav
if [ ! -z "$1" ]; then
  soundfile=$1
fi

while true; do
  echo Connecting uva web sites via a machine on site: $ssh_host
  ssh -i $HOME/.ssh/id_dsa_fred -2 -N -v -D localhost:14420 fred@$ssh_host
  bash $FEISTY_MEOW_SCRIPTS/multimedia/sound_play.sh $soundfile
  echo "Got dumped from tunnels; re-establishing connection."
  echo "Note: if you're being asked for a password, you haven't set up an RSA key yet."
  sleep 14
done

#-L 8028:localhost:3128 

#-L 8043:localhost:443 

# ports sometimes used:
#     25 is the sendmail tunnel.
#   3128 is the squid proxy server.
#    443 is the https version of squid.

# ssh flags in use sometimes:
#   -f   go into the background once connected.
#   -2   enforce ssh version 2.
#   -N   don't execute any command; just forward data between the ports.
#   -L   (port:host:hostport) connect the local machine's "port" to the
#        remote port "hostport" on the "host" specified.  the local "port"
#        becomes an alias for the remote port.  note that the connection
#        being made to host and hostport is from the perspective of the ssh
#        server, not the local host.


