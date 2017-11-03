#!/bin/bash

# resolves a tree conflict by accepting the "working" version,
# which effectively makes your current change the accepted one.

filename="$1"; shift

svn resolve --accept=working "$filename"


