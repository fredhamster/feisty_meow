#!/usr/bin/perl

###############################################################################
#                                                                             #
#  Name   : strip CR                                                          #
#  Author : Chris Koeritz                                                     #
#  Rights : Copyright (C) 1996-$now by Author                                 #
#                                                                             #
#  Purpose:                                                                   #
#                                                                             #
#    Turns DOS format text files into Unix format text files.                 #
#                                                                             #
###############################################################################
# This program is free software; you can redistribute it and/or modify it     #
# under the terms of the GNU General Public License as published by the Free  #
# Software Foundation; either version 2 of the License, or (at your option)   #
# any later version.  See: "http://www.gruntose.com/Info/GNU/GPL.html" for a  #
# version of the License.  Please send any updates to "fred@gruntose.com".    #
###############################################################################

require "filename_helper.pl";
require "importenv.pl";

$new_version = `mktemp "$TMP/zz_strip_cr_tmp.XXXXXX"`;
chop($new_version);

foreach $filename (&glob_list(@ARGV)) {
  # go through each file on the command line.

  open(IN_FILE, "<$filename");

  open(NEW_FILE, ">$new_version");  # open our temporary file for appending.
  binmode(NEW_FILE);

  local($changed_file) = 0;  # record if we made any changes.

  # go through each line in the current file...
  while ($to_strip_cr = <IN_FILE>) {

    $new_line = "";
    for ($i = 0; $i < length($to_strip_cr); $i++) {
      $curr_char = substr($to_strip_cr, $i, 1);
      if ($curr_char =~ /[\r\n]/) {
        if ($curr_char =~ /\r/) {
          # if CR came first, this is a dos style file.
          $changed_file = 1;
        }
        last;
      } else { 
        $new_line .= $curr_char;
      }
    }

    # add on unix EOL, just a line feed.
    $new_line .= "\n";

    print NEW_FILE "$new_line";  # write out the current line.
  }

  close(NEW_FILE); 
  close(IN_FILE); 

  if ($changed_file) {
print "$filename\n";
    open(NEW, "<$new_version");
    open(CURR, ">$filename");
    while (<NEW>) { print CURR; }
    close(NEW);
    close(CURR);
  }

  # clean up our temporaries.
  unlink("$new_version");
}

