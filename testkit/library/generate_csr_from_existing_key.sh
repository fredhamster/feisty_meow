#!/bin/bash

privkey="$1"; shift
subject="$1"; shift
csrfile="$1"; shift

function print_instructions()
{
  echo -e "\n\
This script creates a new CSR (certificate signing request) file for you from\n\
an existing private key.  Getting a new certificate using this CSR ensures\n\
that previously signed resources can still be considered properly signed, even\n\
after the original certificate has expired, by using the new certificate for\n\
validation.  After the new CSR file is generated, it must be sent to the\n\
certificate authority and they can generate a new certificate for you.\n\
\n\
The script takes three parameters.  The first is the file in which the\n\
private key is stored in PEM format.  The second parameter is the subject\n\
to use in the certificate (who the certificate is issued to).  The third\n\
parameter is the output file for the generated CSR (certificate signing\n\
request).\n\
\n\
For example:\n\
  $(basename $0) my-private.key \"Julius Orange\" orange-new-cert-request.csr\n\
"
}

if [ -z "$privkey" -o -z "$subject" -o -z "$csrfile" -o ! -f "$privkey" ]; then
  print_instructions
  echo -e "\nThere was a missing parameter or the private key file did not exist."
  exit 1
fi

openssl req -new -key "$privkey" -nodes -subj "$subject" -out "$csrfile"


