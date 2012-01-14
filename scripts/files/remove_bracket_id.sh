

file=$1

if [ -z "$file" ]; then
  echo "need a parameter that's a file to remove the id from the name."
  exit 3
fi

newname="$(echo "$file" | sed -e 's/\([^[ ]*\) \[[a-z0-9A-Z-]*\]/\1/')"

mv "$file" "$newname"

