#!/bin/bash

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

TEMPO_FILE="$(mktemp "$TMP/zz_feistypack.XXXXXX")"
  # specify where we keep the file until we're ready to move it.

# shortcut for the lengthy exclude parameter.
# note that this only works on file patterns apparently, like *.hosed,
# instead of working with general patterns (like */code_guide/*).
export XC='--exclude='

parent_dir="$(dirname "$FEISTY_MEOW_APEX")"
base_dir="$(basename "$FEISTY_MEOW_APEX")"

pushd $parent_dir

# archive feisty meow current state, but exclude the file names we never want
# to see in the archive.  the exclude vcs flag takes care of excluding
# revision control system private dirs.  first chunk of excludes is for the
# code guide files; this should wash out the majority of those fat things.
# next line is to exclude archives that shouldn't be in the output file.
tar -h -cz --exclude-vcs -f $TEMPO_FILE $base_dir \
\
  ${XC}*incl.map ${XC}*incl.md5 ${XC}*incl.png \
  ${XC}*8h.html ${XC}*8*source.html \
  ${XC}*8cpp.js ${XC}*8h.js \
\
  ${XC}*.tar.gz ${XC}*.zip \

# note: not currently excluded!  cannot do these with --exclude= flag!
#${XC}*/waste/* ${XC}*/logs/* ${XC}*/binaries/* ${XC}*/kona/bin/*

# now move the newest version into its resting place.  this prepares the
# feisty_meow package for uploading.
mv -v $TEMPO_FILE $WEBBED_SITES/feistymeow.org/releases/feisty_meow_codebase_$(date_stringer).tar.gz

popd


