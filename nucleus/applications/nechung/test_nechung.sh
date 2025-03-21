#!/usr/bin/env bash

# prints out the time it takes to run the nechung oracle a thousand times.
# the number of seconds in the result should be equivalent to the number of
# milliseconds that nechung takes to run and produce a fortune, on average.

export i=0

mdate
while [ $i -le 1000 ]; do
#  echo $i
  nechung >/dev/null
  let i++
done
mdate
