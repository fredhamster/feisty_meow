#!/usr/bin/env bash
num=1
while true; do
  import -silent -window root $HOME/bg_image_$num.jpg
  num=$(expr $num + 1)
  sleep 7
done
