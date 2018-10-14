#!/bin/bash

# this script rebuilds the bookmarks files.  it requires the variables:
#     WEBBED_SITES: points at the root of the web hierarchy.

export GRUNTOSE_DIR=$WEBBED_SITES/gruntose.com

rootname=$HOME/generated
suffix=.html
norm_add=_marks
js_add=_js_tree_marks
moz_add=_moz_bookmarks

newmarx=${rootname}_links.csv 
genlinx=$rootname$norm_add$suffix
genlinx_js=$rootname$js_add$suffix
genlinx_moz=$rootname$moz_add$suffix

if [ -f $genlinx ]; then rm $genlinx; fi
if [ -f $newmarx ]; then rm $newmarx; fi
if [ -f $genlinx_js ]; then rm $genlinx_js; fi
if [ -f $genlinx_moz ]; then rm $genlinx_moz; fi

$RUNTIME_PATH/binaries/marks_sorter -i $GRUNTOSE_DIR/Info/Twain/links_db.csv -o $newmarx
if [ $? != 0 ]; then
  echo error during sorting of the bookmarks.
  exit 1
fi

$RUNTIME_PATH/binaries/marks_maker -i $GRUNTOSE_DIR/Info/Twain/links_db.csv -t $GRUNTOSE_DIR/Info/Twain/marks_template.html -o $genlinx -s human
if [ $? != 0 ]; then
  echo error during creation of the normal web page of bookmarks.
  exit 1
fi

$RUNTIME_PATH/binaries/marks_maker -i $GRUNTOSE_DIR/Info/Twain/links_db.csv -t $GRUNTOSE_DIR/Info/Twain/marks_template.html -o $genlinx_moz -s mozilla
if [ $? != 0 ]; then
  echo error during creation of the mozilla format page of bookmarks.
  exit 1
fi

$RUNTIME_PATH/binaries/js_marks_maker -i $GRUNTOSE_DIR/Info/Twain/links_db.csv -t $GRUNTOSE_DIR/Info/Twain/js_template.html -o $genlinx_js
if [ $? != 0 ]; then
  echo error during creation of the javascript bookmark page.
  exit 1
fi

\mv -f $genlinx $genlinx_moz $genlinx_js $GRUNTOSE_DIR/Info/Twain
\mv -f $newmarx $HOME

