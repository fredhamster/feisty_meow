#!/usr/bin/env bash

# shows fdisk report for "real" devices.

for device in \
    $(grep "sd.*[0-9]" /proc/partitions | awk '{ print "/dev/" $4 }') \
; do
  echo
  echo
  echo ==== device $device ====
  sudo fdisk -l $device
done

