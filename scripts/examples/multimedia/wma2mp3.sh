#!/bin/bash

# wma to mp3 script by mtron
# from http://ubuntuforums.org/showthread.php?t=37793

# have found that soundconverter package on ubuntu works on more types
# and is a bit more polished, but mtron's script kicks ass anyhow, if all
# you need is wma -> mp3 conversions.
# --fred hamster

zenity --info \
        --text="this script converts all wma files in the current folder
to mp3s and puts them in the folder output 

all lame command line options can be set in the next step. 

usage:
	lame -m s: for stereo mp3 output
	lame -m s V 3-4-5: for stereo mp3 output with VBR"

# Dialog box to choose output quality
FORMAT=$(zenity --list --title="Choose mp3 output quality" --radiolist --column="Check" --column="Quality (editable)" --editable "" "lame -m s" "" "lame -m s -V 3" "" "lame -m s -V 4" "" "lame -m s -V 5")

if [ $FORMAT -eq ""]; then    
zenity --error --text="mp3 output quality not defined or no wma file found

usage:
	lame -m s: for stereo mp3 output
	lame -m s V 3-4-5: for stereo mp3 output with VBR 
 
type: lame --longhelp 
for all command line options "
exit 1
fi

mkdir -p output
cp *.wma output
cd output

# How many files to make the progress bar
PROGRESS=0
NUMBER_OF_FILES=$(find -iname "*.wma")
let "INCREMENT=100/$NUMBER_OF_FILES"

#remove spaces
(for i in *.wma; do mv "$i" $(echo $i | tr ' ' '_'); done

#remove uppercase
for i in *.[Ww][Mm][Aa]; do mv "$i" $(echo $i | tr '[A-Z]' '[a-z]'); done

#Rip with Mplayer / encode with LAME
for i in *.wma ; do 
echo "$PROGRESS";
echo "# Re-Coding $i";
mplayer -vo null -vc dummy -af resample=44100 -ao pcm:waveheader $i && $FORMAT audiodump.wav -o $i;
let "PROGRESS+=$INCREMENT"
done

#convert file names
for i in *.wma; do mv "$i" "$(basename "$i" .wma).mp3"; 
done

rm audiodump.wav
let "PROGRESS+=$INCREMENT"
) | zenity  --progress --title "$Recoding...encoding..." --percentage=0


