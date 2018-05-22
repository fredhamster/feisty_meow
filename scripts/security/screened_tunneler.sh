#!/bin/bash
# this script manages one ssh tunnel inside a named 'screen' session.

##############

# this section scavenges and documents the command line parameters...

# set this to the user and hostname that serve the tunnel on the remote
# side.  for example: geoffrey@chaucertales.com
TUNNEL_USER_PLUS_HOST="$1"; shift

# a tunnel command for ssh that gives us a link between here and there.
# this should be in the form:
#   "-L ${sourcePort}:${tunnelHost}:${destinationPort}"
# such as this example that connects a local port 12000 to the remote port
# of 25 in order to create a tunnel for smtp traffic:
#   "-L 12000:localhost:25"
# other ssh commands can be used here if they are properly formatted.
TUNNEL_LINK="$1"; shift

# this variable should be set to the name for the tunnel.  one can then
# open the tunnel screen with: screen -r -S "name"
TUNNEL_SCREEN_NAME="$1"; shift

# set this to your key file, plus the -i flag, such as: 
#   SECURITY_KEY="-i $HOME/.ssh/id_rsa" 
# if you do not have a special one set up or the default is fine, then just
# pass a blank parameter (e.g. "").
TUNNEL_SECURITY_KEY="$1"; shift

# if this is set, it means we're through the script the second time, inside
# a screen session, and we need to actually do the work now.
LAUNCH_IT="$1"; shift

##############

function print_instructions()
{
  echo -e "\
$(basename $0): This script requires at least three parameters and can\n\
take up to five.  The parameters are (1) tunnel user at hostname, (2) ssh tunnel\n\
link command, (3) tunnel screen name, (4) tunnel security key, (5) the launch\n\
command 'go'.  An example command is shown below, but many more details are\n\
explained inside this script:\n\
  $(basename $0) "geoffrey@chaucertales.com" "-L 12000:localhost:25" \\\n\
"tunz" "-i mykey.pem"\n\
The fifth flag is really only needed internally, but often the other four\n\
parameters are specified."
}

##############

# make sure the required parameters are provided.
if [ -z "$TUNNEL_USER_PLUS_HOST" -o -z "$TUNNEL_LINK" -o -z "$TUNNEL_SCREEN_NAME" ]; then
  print_instructions
  exit 1
fi

##############

# translate command line parameters if desired.
LAUNCHING_TUNNEL=0
if [ "$LAUNCH_IT" == "go" ]; then
  LAUNCHING_TUNNEL=1
fi

##############

#hmmm: these variables should be configurable from plug-ins someday.

TUNNEL_ALERT_SOUND=$FEISTY_MEOW_APEX/infobase/sounds/woouoo.wav
if [ ! -z "$1" ]; then
  TUNNEL_ALERT_SOUND=$1
fi

# how often to play sounds when reconnecting.
NOISE_PERIOD=180

# when we last played a sound.
LAST_SOUND_TIME=0

##############

play_sound_periodically()
{
  CURRENT_TIME=$(date +"%s")
  if (( $CURRENT_TIME - $LAST_SOUND_TIME >= $NOISE_PERIOD )); then
    echo playing sound now.
    bash $FEISTY_MEOW_SCRIPTS/multimedia/sound_play.sh $TUNNEL_ALERT_SOUND &>/dev/null </dev/null &
    LAST_SOUND_TIME=$CURRENT_TIME
  fi
}

##############

function main_tunnely_loop()
{
  while true; do
    echo "Connecting tunnel to destination..."
    ssh -2 -N -v "$TUNNEL_LINK" "$TUNNEL_SECURITY_KEY" "$TUNNEL_USER_PLUS_HOST"
    echo "Got dumped from tunnels; re-establishing connection."
    play_sound_periodically
    echo "Note: if you're being asked for a password, then you haven't provided\nan RSA key that works yet."
    sleep 1
  done
}

if [ $LAUNCHING_TUNNEL -eq 1 ]; then
  # this version is already ready to tunnel already, so let's tunnel.
  main_tunnely_loop
  # loop does not exit on its own.
else
  # this version re-launches the script but tells it to start the tunnel.
  existingScreens="$(screen -ls | grep "$TUNNEL_SCREEN_NAME")"
  if [ ! -z "$existingScreens" ]; then
    echo "This script is already running a screen for: $TUNNEL_SCREEN_NAME"
    echo "Connect to that and zap it first before we try to start a new one,"
    echo "e.g.: screen -r -S \"$TUNNEL_SCREEN_NAME\""
    exit 1
  fi
  screen -L -S "$TUNNEL_SCREEN_NAME" -d -m bash $0 "$TUNNEL_USER_PLUS_HOST" "$TUNNEL_LINK" "$TUNNEL_SCREEN_NAME" "$TUNNEL_SECURITY_KEY" go
fi

