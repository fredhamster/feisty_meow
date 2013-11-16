#!/bin/bash
# this is a simple shell that can be dropped in ~/.kde/Autostart to show a
# bunch of roaches on the root X window.
# note: you must enable desktop | behavior | general | allow programs in
# desktop window for the fishtank to show up.

xroach -roaches 23 -rc purple -rgc green -speed 0.05 -squish &


