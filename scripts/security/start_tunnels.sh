#!/bin/bash
# this script makes a tunnel for SMTP traffic and others.  a remote ssh server
# is required.  this is especially useful for routing around firewalls using
# a web proxy like squid.  when used for SMTP, it ensures that none of the
# text is seen on whatever network one is on before it's sent from the remote
# server.

#hmmm:  none of the user info below will work for others: parameterize it.

#hmmm: maybe we need a base function that takes all the disparate values,
#      and this script could call it with known feisty meow settings.

##############

# check for parameters on the command line.
launch_it="$1"; shift

##############

LAUNCHING_TUNNEL=0
if [ "$launch_it" == "go" ]; then
  LAUNCHING_TUNNEL=1
fi

##############

# these variables are configurable from plug-ins.
#hmmm: what?

soundfile=$FEISTY_MEOW_DIR/database/sounds/woouoo.wav
if [ ! -z "$1" ]; then
  soundfile=$1
fi

##############

# provides a list of properly formatted tunnels for ssh to create.  if this list
# is empty, then we do nothing.
TUNNEL_LIST=()

# set this to the hostname that will be providing the tunnel.  this is
# usually a remote system.
TUNNEL_USER_PLUS_HOST=""

# set this to your key file, plus the -i flag, such as: 
#   SECURITY_KEY="-i $HOME/.ssh/id_rsa" 
TUNNEL_SECURITY_KEY=""

# this variable should be set to the name for the tunnel.  one can then
# open the tunnel screen with: screen -r -S "name"
TUNNEL_SCREEN_NAME="tunnely"

# a comment for when we make the connection
TUNNEL_COMMENT="Connecting tunnel to destination..."

##############

#hmmm:move to fred configs!
TUNNEL_LIST+=(-L 14008:localhost:25)
TUNNEL_USER_PLUS_HOST="fred@serene.feistymeow.org"
TUNNEL_SECURITY_KEY="-i $HOME/.ssh/id_dsa_fred" 
TUNNEL_COMMENT="Connecting sendmail to serenely zooty."
TUNNEL_SCREEN_NAME="zooty"

##############

# how often to play sounds when reconnecting.
NOISE_PERIOD=180

# when we last played a sound.
LAST_SOUND_TIME=0

play_sound_periodically()
{
  CURRENT_TIME=$(date +"%s")
  if (( $CURRENT_TIME - $LAST_SOUND_TIME >= $NOISE_PERIOD )); then
    echo playing sound now.
    bash $FEISTY_MEOW_SCRIPTS/multimedia/sound_play.sh $soundfile &>/dev/null </dev/null &
#hmmm: parameterize this for the sound to be played.  doofus.
    LAST_SOUND_TIME=$CURRENT_TIME
  fi
}

##############

function main_tunnely_loop()
{
  while true; do
    echo "$TUNNEL_COMMENT"
    ssh -2 -N -v ${TUNNEL_LIST[*]} "$TUNNEL_SECURITY_KEY" "$TUNNEL_USER_PLUS_HOST"
    echo "Got dumped from tunnels; re-establishing connection."
    play_sound_periodically
    echo "Note: if you're being asked for a password, you haven't set up an RSA key yet."
    sleep 1
  done
}

# notes...

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

if [ $LAUNCHING_TUNNEL -eq 1 ]; then
  # this version is already ready to tunnel already, so let's tunnel.
  main_tunnely_loop
  # loop does not exit on its own.
else
  # this version re-launches the script but tells it to start the tunnel.
  screen -L -S "$TUNNEL_SCREEN_NAME" -d -m bash $0 go
fi


