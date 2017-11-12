#!/bin/bash

# a simple unit test component that takes three parameters: the test to run, the
# input file to give that test, and the expected correct output file from the test.
# the script will complain if there is an error in the test output.  otherwise it
# says nothing, but the script's return value can also be checked.

to_run="$1"; shift
input_file="$1"; shift
answer_file="$1"; shift

eval "$to_run" < "$input_file" | bash "$FEISTY_MEOW_SCRIPTS/testing/verify_correct_input.sh" "$answer_file"

