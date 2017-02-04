#/bin/bash

# a simple component of unit testing which verifies that the input matches
# the expected input.

# the single parameter to the script is a file that contains the correct answer.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

answer_file="$1"; shift

if [ -z "$answer_file" -o ! -f "$answer_file" ]; then
  echo This script needs a valid file parameter that points at the correct
  echo values for the data stream.
  exit 1
fi

input_save_file="$(mktemp "$TMP/zz_verify_input.XXXXXX")"

while read line; do
  echo $line >>"$input_save_file"
done

diff -q "$input_save_file" "$answer_file"
if [ $? -ne 0 ]; then
  sep 76
  echo "The provided text differs from the correct answer!"
  echo -e "\nAnswer file has:\n=============="
  cat "$answer_file"
  echo -e "==============\nBut the data we saw has:\n=============="
  cat "$input_save_file"
  echo -e "=============="
  sep 76
  false  # set bad exit value.
fi

rm "$input_save_file"

exit $?

