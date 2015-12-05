#!/bin/bash
# tunnels to the khandroma machine for vnc access to the win7 box and for the jenkins
# server.

soundfile=$FEISTY_MEOW_APEX/infobase/sounds/my_mind_is_going.wav
if [ ! -z "$1" ]; then
  soundfile=$1
fi

while true; do
  echo Connecting jenkins and vms at khandroma.
  ssh -i $HOME/.ssh/id_dsa_fred -2 -N -v -L "*:5905:localhost:5905" -L "*:5902:localhost:5902" -L "*:5909:localhost:5909" -L "*:5908:localhost:5908" -L "*:4040:localhost:8080" fred@khandroma.cs.virginia.edu
  bash $FEISTY_MEOW_SCRIPTS/multimedia/sound_play.sh $soundfile &>/dev/null </dev/null &
  echo "Got dumped from tunnels; re-establishing connection."
  echo "Note: if you're being asked for a password, you haven't set up an RSA key yet."
  sleep 14
done

