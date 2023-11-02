#! /usr/bin/env python3
"""

Takes a certificate from the awful, often used form of a single line of text and turns
it into a real PEM file.
This assumes the input file has one single line of text that needs to be formatted
appropriately.

author: chris koeritz

"""

import os
import sys

####

def split_lines(unsplit_line: str) -> None:
    """
    Takes text as input and breaks it up at the prescribed 64 character boundaries
    used by PEM files.
    """

    size = 64
    splits = [(unsplit_line[i : i + size]) for i in range(0, len(unsplit_line), size)]

    #print("splits:", splits)
    for split_line in splits:
        if len(split_line) > 0:
            print(split_line)

####

def main() -> None:
    """Main function"""

    # make sure they gave us a filename.
    args = len(sys.argv)
    if args < 2:
        print("\
This script needs a filename to operate on.  The file is expected to contain\n\
one line of certificate data, which this script will reformat into a standard\n\
PEM file format.  The PEM file will be output on the console.")
        exit(1)
 
    filename = sys.argv[1]

    # make sure the filename is valid.
    if not os.path.isfile(filename):
        print("The filename provided does not seem to be a readable file:", filename)
        exit(1)

    file = open(filename, "r")

    cert_line = file.readline()
    cert_line = cert_line.strip('\r\n')

    #ugh, no extra noise needed.
    #print()
    #print("below is the properly formatted output sourced from:", filename)
    #print()

    print("-----BEGIN CERTIFICATE-----")
    split_lines(cert_line)
    print("-----END CERTIFICATE-----")
 
####

if __name__ == "__main__":
    main()


