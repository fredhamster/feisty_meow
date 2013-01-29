
file="$1"; shift

if [ -z "$file" ]; then
  echo "This script requires one filename that is a differ output file."
  echo "All of the files with differences mentioned in the differ file will be"
  echo "extracted and printed out."
  exit 1
fi

grep "seen for [a-zA-Z0-9]*\.java" "$file" | sed -e "s/Differences seen for //" -e "s/:$//"

