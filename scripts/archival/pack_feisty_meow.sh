#!/bin/bash

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

# location where we intend to store these packages.
RELEASE_PATH="$WEBBED_SITES/feistymeow.org/releases"

# check that we can see the release path.
if [ ! -d "$RELEASE_PATH" ]; then
  echo "The release path does not exist: $RELEASE_PATH"
  exit 1
fi

TEMPO_FILE="$(mktemp "$TMP/zz_feistypack.XXXXXX")"
  # specify where we keep the file until we're ready to move it.

log_feisty_meow_event "packing feisty meow in temporary file $TEMPO_FILE"

parent_dir="$(dirname "$FEISTY_MEOW_APEX")"
base_dir="$(basename "$FEISTY_MEOW_APEX")"

pushd $parent_dir

# shortcut for the lengthy exclude parameter.
# note that this only works on file patterns apparently, like *.hosed,
# instead of working with general patterns (like */code_guide/*).
export XC='--exclude='

# archive feisty meow current state, but exclude the file names we never want
# to see in the archive.  the exclude vcs flag takes care of excluding
# revision control system private dirs.  first chunk of excludes is for the
# code guide files; this should wash out the majority of those fat things.
# next line is to exclude archives that shouldn't be in the output file.
tar -h -cz --exclude-vcs -f $TEMPO_FILE \
\
  ${XC}*incl.map ${XC}*incl.md5 ${XC}*incl.png \
  ${XC}*8h.html ${XC}*8c.html ${XC}*8cpp.html ${XC}*8*source.html \
  ${XC}class*html ${XC}class*js ${XC}class*members.html \
  ${XC}struct*html ${XC}struct*js ${XC}struct*members.html \
  ${XC}globals*html ${XC}functions*html \
  ${XC}navtree*js ${XC}inherit_graph_* \
  ${XC}namespace*js ${XC}namespace*html \
  ${XC}dir_*html ${XC}dir_*map ${XC}dir_*md5 ${XC}dir_*png ${XC}dir_*js \
  ${XC}*8cpp.js ${XC}*8h.js \
  ${XC}*graph.map ${XC}*graph.md5 ${XC}*graph.png \
\
  ${XC}*.tar.gz ${XC}*.zip \
\
$base_dir 

# note: not currently excluded!  cannot do these with --exclude= flag!
#${XC}*/waste/* ${XC}*/logs/* ${XC}*/binaries/* ${XC}*/kona/bin/*

# now move the newest version into its resting place.  this prepares the
# feisty_meow package for uploading.
mv -v $TEMPO_FILE "$RELEASE_PATH/feisty_meow_codebase_$(date_stringer).tar.gz"

popd


