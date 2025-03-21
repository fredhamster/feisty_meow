#!/usr/bin/env bash
#need to support multiple names.
for i; do
#old  find "$1" -type d -depth -print -exec perl $FEISTY_MEOW_SCRIPTS/renlower.pl {}/* ';'
  find "$i" -depth -exec perl $FEISTY_MEOW_SCRIPTS/files/renlower.pl "{}" ';'
done

