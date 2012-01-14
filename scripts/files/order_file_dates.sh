#!/bin/bash
for i in $(ls -1 | sort ) ; do 
  touch $i
  sleep 2
done

