for i in $FEISTY_MEOW_DIR/binaries/*.exe ; do dumpbin //headers $i | grep -i 8664 ; done
