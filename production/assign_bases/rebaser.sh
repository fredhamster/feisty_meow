#!/bin/sh
bash $PRODUCTION_DIR/assign_bases/whack_odd_dlls.sh
"$VS80COMNTOOLS/Bin"/rebase.exe -b 0x68000000 -d $DYNAMIC_LIBRARY_DIR/*.dll

