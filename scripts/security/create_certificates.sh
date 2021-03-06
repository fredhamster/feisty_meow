
filename="$1"; shift
cert_alias="$1"; shift
if [ -z "$filename" -o -z "$cert_alias" ]; then
  echo "This script needs a base portion for the certificate filename to create"
  echo "and the alias (or short name) for the certificate."
  echo For example:
  echo -e "\t$(basename $0 .sh) DrakeKey \"DrakeContainer\""
  echo would create DrakeKey.pem and DrakeKey.pfx with a cert alias of DrakeContainer.
  exit 1
fi

# create PEM file.
openssl req -x509 -nodes -days 3650 \
  -newkey rsa:1024 -keyout ${filename}.pem -out ${filename}.pem

# export the PEM to a PFX file.
openssl pkcs12 -export -out ${filename}.pfx -in ${filename}.pem -name "$cert_alias"

# export the PEM to a DER certificate file.
openssl x509 -inform pem -in ${filename}.pem -outform der -out ${filename}.cer



