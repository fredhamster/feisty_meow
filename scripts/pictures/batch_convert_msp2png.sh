#!/usr/bin/env bash

# converts ancient microsoft paint files to PNG format using recoil.
# arose from a question on askubuntu: http://askubuntu.com/questions/881412/how-can-i-convert-really-old-files-in-microsoft-paint-format-msp-to-a-usabl
# requires recoil to be installed: http://recoil.sourceforge.net/

# run two phases since sometimes the ending is capitalized.
for suffix in .msp .MSP; do
  if [ ! -z "$(ls *${suffix} 2>/dev/null)" ]; then
    for i in *${suffix} ; do recoil2png -o $(basename $i ${suffix}).png $i ; done
  fi
done

