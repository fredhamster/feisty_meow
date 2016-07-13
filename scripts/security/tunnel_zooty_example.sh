#!/bin/bash
# this script makes a tunnel for fred's SMTP traffic.

#hmmm: move this script to fred customized stuff

TUNNEL_USER_PLUS_HOST="fred@serene.feistymeow.org"
TUNNEL_LINK="-L 14008:localhost:25"
TUNNEL_SCREEN_NAME="zooty"
TUNNEL_SECURITY_KEY="-i $HOME/.ssh/id_dsa_fred" 

##############

bash $FEISTY_MEOW_SCRIPTS/security/screened_tunneler.sh "$TUNNEL_USER_PLUS_HOST" "$TUNNEL_LINK" "$TUNNEL_SCREEN_NAME" "$TUNNEL_SECURITY_KEY"


