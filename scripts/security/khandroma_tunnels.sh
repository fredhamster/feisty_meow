#!/bin/bash
# tunnels to the khandroma machine for vnc access to the win7 box and for the jenkins
# server.

soundfile=$FEISTY_MEOW_DIR/database/sounds/my_mind_is_going.wav
if [ ! -z "$1" ]; then
  soundfile=$1
fi

while true; do
  echo Connecting jenkins and vms at khandroma.
  ssh -i $HOME/.ssh/id_dsa_fred -2 -N -v -L "*:5908:localhost:5908" -L "*:4040:localhost:8080" fred@khandroma.cs.virginia.edu &>~/.tmp/zz_tunnel_khandro_jenkins_vnc.log &
  bash $FEISTY_MEOW_SCRIPTS/multimedia/sound_play.sh $soundfile &>/dev/null </dev/null &
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


