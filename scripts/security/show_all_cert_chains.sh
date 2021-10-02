#!/bin/bash

file="$1"; shift
if [ -z "$file" -o ! -f "$file" ]; then
  echo This script requires a PEM-format file name to show the certificates within.
  exit 1
fi

openssl crl2pkcs7 -nocrl -certfile "$file" | openssl pkcs7 -print_certs -text -noout

