#!/bin/bash
# processes cgi request and passes it on to the real script.

show_name="$(echo $QUERY_STRING | sed -e "s/^[^=][^=]*=\(.*\)$/\1/" | sed -e "s/\+/ /g")"
vids="$DOCUMENT_ROOT/Info/Quartz/video/video_tapes.csv"

randof="$(mktemp "$TMP/zz_randocgi.XXXXXX")"

sh show_stripper.sh "$vids" "$show_name" &>"$randof"

if [ -s "$randof" ]; then

  echo "Content-type: text/plain"
  echo ""
  echo ""
  cat "$randof"

else

  echo "Content-type: text/html"
  echo ""
  echo ""
  cat /home/www-data/feisty_meow/database/pictures/no_matches.html

fi

rm "$randof"

