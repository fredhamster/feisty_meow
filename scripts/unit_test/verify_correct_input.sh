#/bin/bash

# a simple component of unit testing which verifies that the input matches the output.

# the single parameter to the script is a file that contains the correct answer.

source "$FEISTY_MEOW_SCRIPTS/core/functions.sh"

answer_file="$1"; shift

if [ -z "$answer_file" -o ! -f "$answer_file" ]; then
  echo This script needs a parameter which is a valid file filled with the
  echo correct version of the input.
  exit 1
fi

input_save_file="$(mktemp "$TMP/zz_verify_input.XXXXXX")"

while read line; do
  echo $line >>"$input_save_file"
done

diff -q "$input_save_file" "$answer_file"
if [ $? -ne 0 ]; then
  sep 76
  echo "The provided input differs from the correct answer!"
  echo -e "\nAnswer file has:\n=============="
  cat "$answer_file"
  echo -e "==============\nBut the input data has:\n=============="
  cat "$input_save_file"
  echo -e "=============="
  sep 76
  false  # set bad exit value.
fi

exit $?

