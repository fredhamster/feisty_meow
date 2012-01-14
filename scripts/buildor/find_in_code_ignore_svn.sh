#!/bin/bash
seek="$1"; shift

find . -type f | grep -v '.svn' | grep -v '.exe' | xargs grep -li "$seek" 
