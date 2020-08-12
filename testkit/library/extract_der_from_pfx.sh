
pfx_file="$1"; shift
if [ -z "$pfx_file" -o ! -f "$pfx_file" ]; then
  echo "This script requires the full path to a PFX file which will have its"
  echo "certificate extracted into a DER formatted file ending in '.cer'."
  exit 1
fi

intermed_file="$TMP/$(basename "$pfx_file" .pfx).pem"
final_file="$(dirname "$pfx_file")/$(basename "$pfx_file" .pfx).cer"

echo input PFX file is $pfx_file
echo generating intermediate PEM file...
openssl pkcs12 -in "$pfx_file" -out "$intermed_file" -nodes -nokeys
if [ $? -ne 0 ]; then echo "previous step failed!"; exit 1; fi
echo generated intermediate PEM file $intermed_file
echo generating final file in DER format...
openssl x509 -outform der -in "$intermed_file" -out "$final_file"
if [ $? -ne 0 ]; then echo "previous step failed!"; exit 1; fi
echo final DER file successfully saved as $final_file

