#!/bin/bash

# processes cgi request and passes it on to the real script.

vids="$DOCUMENT_ROOT/Info/Quartz/video/video_tapes.csv"

echo "Content-type: text/plain"
echo ""
echo ""
bash movie_stripper.sh "$vids" "$movie_name"

