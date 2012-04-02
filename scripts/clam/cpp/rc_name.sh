#!/bin/bash
if [ -f version.ini ]; then 
  grep "root *=" version.ini | sed -e "s/root *= *//" -e "s/ *$//"
fi
