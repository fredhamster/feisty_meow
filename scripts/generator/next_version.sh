#!/bin/bash

# increments the build version number.

mkdir -p "$TEMPORARIES_DIR"

export PARAMETER_FILE
if [ ! -z "$BUILD_PARAMETER_FILE" ]; then
  # define our version of the build parameter file.  this should be set
  # beforehand so we override the default parameter file for clam.
  PARAMETER_FILE="$BUILD_PARAMETER_FILE"
fi
if [ -z "$PARAMETER_FILE" ]; then
  # last ditch attempt to get one that will work.
  PARAMETER_FILE="$REPOSITORY_DIR/build.ini"
fi

chmod u+w "$PARAMETER_FILE"

new_buildini="$(mktemp "$TEMPORARIES_DIR/buildini.XXXXXX")"
# whack the file just in case.
rm -f "$new_buildini"
echo -n "" >"$new_buildini"

# pick a weird separator that we hope never to see.
IFS='~'

found_version=""
skip_line=""

major_string=
minor_string=
revision_string=

while read line_found; do
  if [ $? != 0 ]; then break; fi
#echo line found is $line_found
  if [ ! -z "$skip_line" ]; then
    # we were told to skip this line to fix win32.
    skip_line=""
    continue
  fi

  # these checks don't care about whether we've seen other stuff yet.
  if [ -z "$major_string" ]; then
    if [ ! -z "$(echo $line_found | sed -n -e 's/^ *major *=.*$/yep/p')" ]; then
      major_string=$(echo $line_found | sed -e 's/.*=\(.*\)$/\1/' )
    fi
  fi
  if [ -z "$minor_string" ]; then
    if [ ! -z "$(echo $line_found | sed -n -e 's/^ *minor *=.*$/yep/p')" ]; then
      minor_string=$(echo $line_found | sed -e 's/.*=\(.*\)$/\1/' )
    fi
  fi

  # we process the revision string line specially.
  if [ -z "$found_version" ]; then
	if [ "$line_found" == "#[version]" ]; then
	  # repair our special escape that makes this a valid ini file and
	  # gnu make include file.
	  echo -e "#\\\\\n[version]" >>"$new_buildini"
	  found_version="yes"
      continue
	elif [ "$line_found" == "#" ]; then
	  # retarded win32 case.
	  echo -e "#\\\\\n[version]" >>"$new_buildini"
	  found_version="yes"
	  skip_line="yes"
      continue
	fi
  elif [ -z "$revision_string" ]; then
    if [ ! -z "$(echo $line_found | sed -n -e 's/^ *revision *=.*$/yep/p')" ]; then
      revision_string=$(echo $line_found | sed -e 's/.*=\(.*\)$/\1/' )
#echo second part is $revision_string 
      revision_string=$(expr $revision_string + 1)
      echo "revision=$revision_string" >>"$new_buildini"
      # don't print the line normally also.
      continue
    fi
  fi

  # send the line with no special processing.
  echo "$line_found" >>"$new_buildini"

done <"$PARAMETER_FILE"

# if we created something with contents, let's use it.
if [ -s "$new_buildini" ]; then
  cp "$new_buildini" "$PARAMETER_FILE"
fi

echo "New build version is: $major_string.$minor_string.$revision_string"

# don't leave the temporary version files floating around.
rm -f "$new_buildini"

