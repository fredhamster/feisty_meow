for i in $FEISTY_MEOW_APEX/binaries/*.exe ; do dumpbin //headers $i | grep -i 8664 ; done
