#!/bin/bash
#need to support multiple names.
for i; do
#old  find "$1" -type d -depth -print -exec perl $SHELLDIR/renlower.pl {}/* ';'
  find "$i" -depth -exec perl $SHELLDIR/renlower.pl "{}" ';'
done

