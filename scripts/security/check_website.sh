#!/usr/bin/env bash

nmap -O -sS -F -P0 -T Aggressive $1

echo "require that all ports show as closed or filtered except for 80, 443, and, possibly, 25 and 22, and that the OS is not ms-win32."

