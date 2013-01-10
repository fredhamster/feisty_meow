#!/bin/bash
seek="$1"; shift

find . -type f -exec echo "\"{}\"" ';' | grep -v '.svn' | grep -v '.git' | grep -v '.exe' | grep -v '.obj' | grep -v '.class' | grep -v '.dll' | grep -v '.lib' | xargs grep -li "$seek" 
