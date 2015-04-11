#!/bin/bash

# takes names given to it and replaces any spaces or other gnarly characters with underscores.

#hmmm: this starts to look like a useful function that the bracket fixer could also use.

if [ $# -lt 1 ]; then
  echo "This script requires one or more file names whose names should be fixed."
  echo "Any spaces or single-quote characters will be stripped out in a useful way."
  exit 1
fi

while [ $# -gt 0 ]; do
  file="$1"; shift
  newname="$(echo "$file" | tr -s ' ' '_' | tr -d "\$@#%}{)(][\\\~',:?><\"" | sed -e 's/\([0-9]\)_\./\1./g' | sed -e 's/_-_/-/' )"
  if [ "$file" != "$newname" ]; then
    # we've effected a name change, so let's actually do it.
    echo "'$file' => '$newname'"
    mv "$file" "$newname"
  fi
done

