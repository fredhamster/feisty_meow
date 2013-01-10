#!/bin/bash
seek="$1"; shift

find . -type f -exec echo "\"{}\"" ';' | grep -v '.svn' | grep -v '.git' | grep -v '.exe' | xargs grep -li "$seek" 
