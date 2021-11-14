#!/bin/bash

# displays every certificate in a PEM file that has a whole certificate chain.
# this is surprisingly annoying to get anything to output, so we codified it.

# by chris koeritz

file="$1"; shift
if [ -z "$file" -o ! -f "$file" ]; then
  echo This script requires a PEM-format file name to show the certificates within.
  exit 1
fi

openssl crl2pkcs7 -nocrl -certfile "$file" | openssl pkcs7 -print_certs -text -noout

