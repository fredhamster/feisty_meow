#!/bin/bash
XTFONT=$(xmenu -geometry +800+50 -heading "Font" \
"LucidaTypeWriter 12" "LucidaTypeWriter 12 Bold" \
"LucidaTypeWriter 14" "LucidaTypeWriter 14 Bold" \
"LucidaTypeWriter 18" "LucidaTypeWriter 18 Bold" \
"Courier 12" "Courier 12 Bold" \
"Courier 14" "Courier 14 Bold" \
"Courier 18" "Courier 18 Bold" \
"Gallant 19" "PC Font 14" \
"Screen 12" "Screen 12 Bold" \
"Screen 14" "Screen 14 Bold" \
"Screen 16" "Screen 16 Bold" \
"Fixed 10" \
"Fixed 12"  "Fixed 12 Bold" \
"Fixed 14" "Fixed 14 Bold" \
"Fixed 17" \
"Clean 12" "Clean 12 Bold" \
"Clean 14" "Clean 14 Bold" \
"Clean 16" "Clean 16 Bold" \
"Cancel") 

case $XTFONT in
   'Clean 12')
   xtattr -font '-*-clean-medium-r-*-*-*-120-*-*-*-*-*-*'
   ;;

   'Clean 12 Bold')
   xtattr -font '-*-clean-bold-r-*-*-*-120-*-*-*-*-*-*'
   ;;

   'Clean 14')
   xtattr -font '-*-clean-medium-r-*-*-*-140-*-*-*-*-*-*'
   ;;

   'Clean 14 Bold')
   xtattr -font '-*-clean-bold-r-*-*-*-140-*-*-*-*-*-*'
   ;;

   'Clean 16')
   xtattr -font '-*-clean-medium-r-*-*-*-160-*-*-*-*-*-*'
   ;;

   'Clean 16 Bold')
   xtattr -font '-*-clean-bold-r-*-*-*-160-*-*-*-*-*-*'
   ;;

   'Fixed 10')
   xtattr -font '-*-fixed-medium-*-*-*-*-100-*-*-*-*-*-*'
   ;;

   'Fixed 12')
   xtattr -font '-*-fixed-medium-*-*-*-*-120-*-*-*-*-*-*'
   ;;

   'Fixed 12 Bold')
   xtattr -font '-*-fixed-bold-*-*-*-*-120-*-*-*-*-*-*'
   ;;

   'Fixed 14')
   xtattr -font '-*-fixed-medium-*-*-*-*-140-*-*-*-*-*-*'
   ;;

   'Fixed 14 Bold')
   xtattr -font '-*-fixed-bold-*-*-*-*-140-*-*-*-*-*-*'
   ;;

   'Fixed 17')
   xtattr -font '-*-fixed-medium-*-*-*-*-170-*-*-*-*-*-*'
   ;;

   'Screen 12')
   xtattr -font '-*-screen-medium-*-*-*-*-120-*-*-m-*-*-*' 
   ;;

   'Screen 12 Bold')
   xtattr -font '-*-screen-bold-*-*-*-*-120-*-*-m-*-*-*' 
   ;;

   'Screen 14')
   xtattr -font '-*-screen-medium-*-*-*-*-140-*-*-m-*-*-*' 
   ;;

   'Screen 14 Bold')
   xtattr -font '-*-screen-bold-*-*-*-*-140-*-*-m-*-*-*' 
   ;;

   'Screen 16')
   xtattr -font '-*-screen-medium-*-*-*-*-160-*-*-m-*-*-*' 
   ;;

   'Screen 16 Bold')
   xtattr -font '-*-screen-bold-*-*-*-*-160-*-*-m-*-*-*' 
   ;;

   'LucidaTypeWriter 12')
   xtattr -font '-*-lucidatypewriter-medium-*-*-*-*-120-*-*-*-*-*-*'
   ;;

   'LucidaTypeWriter 14')
   xtattr -font '-*-lucidatypewriter-medium-*-*-*-*-140-*-*-*-*-*-*'
   ;;

   'LucidaTypeWriter 18')
   xtattr -font '-*-lucidatypewriter-medium-*-*-*-*-180-*-*-*-*-*-*'
   ;;

   'LucidaTypeWriter 12 Bold')
   xtattr -font '-*-lucidatypewriter-bold-*-*-*-*-120-*-*-*-*-*-*'
   ;;

   'LucidaTypeWriter 14 Bold')
   xtattr -font '-*-lucidatypewriter-bold-*-*-*-*-140-*-*-*-*-*-*'
   ;;

   'LucidaTypeWriter 18 Bold')
   xtattr -font '-*-lucidatypewriter-bold-*-*-*-*-180-*-*-*-*-*-*'
   ;;

   'Courier 12')
   xtattr -font '-*-courier-medium-r-*-*-*-120-*-*-*-*-*-*'
   ;;
   
   'Courier 14')
   xtattr -font '-*-courier-medium-r-*-*-*-140-*-*-*-*-*-*'
   ;;
   
   'Courier 18')
   xtattr -font '-*-courier-medium-r-*-*-*-180-*-*-*-*-*-*'
   ;;

   'Courier 12 Bold')
   xtattr -font '-*-courier-bold-r-*-*-*-120-*-*-*-*-*-*'
   ;;
   
   'Courier 14 Bold')
   xtattr -font '-*-courier-bold-r-*-*-*-140-*-*-*-*-*-*'
   ;;
   
   'Courier 18 Bold')
   xtattr -font '-*-courier-bold-r-*-*-*-180-*-*-*-*-*-*'
   ;;

   'Gallant 19')
   xtattr -font '-*-gallant-*-*-*-*-*-190-*-*-m-*-*-*'
   ;;
   
   'PC Font 14')
   xtattr -font '-*-pcfont-*-*-*-*-*-140-*-*-m-*-*-*'
   ;;

   'Cancel')
   ;;
   
esac
