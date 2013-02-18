#!/bin/bash

# just tests whether a linux box can handle 64-bit operating systems.

longmode_check="$(grep " lm " /proc/cpuinfo |head -n 1)"

if [ -z "$longmode_check" ]; then
  echo "CPU does not seem to support 64 bit long mode."
else
  echo "CPU can support 64 bit long mode."
fi

