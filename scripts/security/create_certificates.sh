
filename="$1"; shift
if [ -z "$filename" ]; then
  echo This script needs a base portion for the certificate filename to create.
  echo For example:
  echo   $0 DrakeKey
  echo would create DrakeKey.pem and DrakeKey.pfx.
  exit 1
fi

# create PEM file.
openssl req -x509 -nodes -days 3650 \
  -newkey rsa:1024 -keyout ${filename}.pem -out ${filename}.pem

# export PFX file.
openssl pkcs12 -export -out ${filename}.pfx -in ${filename}.pem -name "Drake Container Certificate"

# export the PFX to a certificate file.  this can be given to other folks.
openssl x509 -inform pem -in ${filename}.pem -outform der -out ${filename}.cer



