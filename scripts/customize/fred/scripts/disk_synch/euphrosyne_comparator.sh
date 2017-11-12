#!/bin/bash

# runs through all the local archives on euphrosyne to make sure nothing is different
# when compared to the mainline versions on surya.

#hmmm: add a check that this is in fact the right host, euphrosyne.


source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

sep 14
echo "Comparing basement folder..."
compare_dirs /z/basement wildmutt:/z/basement
sep 14

sep 14
echo "Comparing imaginations folder..."
compare_dirs /z/imaginations wildmutt:/z/imaginations
sep 14

sep 14
echo "Comparing musix folder..."
compare_dirs /z/musix wildmutt:/z/musix
sep 14

sep 14
echo "Comparing walrus folder..."
compare_dirs /z/walrus wildmutt:/z/walrus
sep 14
