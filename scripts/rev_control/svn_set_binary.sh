#!/bin/bash
# clean out flags that hinder files being binary.
for i in $*; do
  echo "svn binary -> $i"
  svn pd svn:executable "$i" >/dev/null 2>&1
  svn pd svn:eol-style "$i" >/dev/null 2>&1
  svn pd svn:keywords "$i" >/dev/null 2>&1
done

