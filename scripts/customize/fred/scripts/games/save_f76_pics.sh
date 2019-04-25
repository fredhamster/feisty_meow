#!/bin/bash

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

if [[ "$hostname" ~= clemems* ]]; then
  character=clemens_pc
fi
if [[ "$hostname" ~= orpheus* ]]; then
  character=spoonburg_pc
fi

netcp ~/linx/wind_goods/My\ Games/Fallout\ 76/Photos/8f99c06443f04f6f8270604369bb78eb/*[0-9].png /z/walrus/media/pictures/metaverse/fallout_76/${character}/

