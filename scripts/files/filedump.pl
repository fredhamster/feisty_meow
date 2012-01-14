#!/usr/bin/perl

###############################################################################
#                                                                             #
#  Name   : filedump                                                          #
#  Author : Chris Koeritz                                                     #
#  Rights : Copyright (C) 1996-$now by Author                                 #
#                                                                             #
#  Purpose:                                                                   #
#                                                                             #
#    Smashes multiple text files together into one large text file.           #
#                                                                             #
###############################################################################
#  This program is free software; you can redistribute it and/or modify it    #
#  under the terms of the GNU General Public License as published by the Free #
#  Software Foundation; either version 2 of the License or (at your option)   #
#  any later version.  See: "http://www.gruntose.com/Info/GNU/GPL.html" for a #
#  version of the License.  Please send any updates to "fred@gruntose.com".   #
###############################################################################

require "filename_helper.pl";

if ($#ARGV < 0) {
  &instructions;
  exit 0;
}

# iterate over the list of files and dump them to standard output.

foreach $filename (&glob_list(@ARGV)) {
  &do_dump($filename);
}

exit 0;

############################################################################

sub instructions {
  print "

filedump: this program needs a set of filenames to print out.  they will
          be dumped to the standard output.

";
}

############################################################################

# do_dump: prints the contents of the first parameter's file out to stdout.

sub do_dump {

  ($to_dump) = @_;

#  print "dumpfile=$to_dump\n";

  $header = "

%2
%1
%2

";

  $header_copy = $header;
  $shorter_name = $to_dump;
  $shorter_name =~ s/^\.\///;
  $shorter_name =~ s/\.txt$//;
  $shorter_name =~ s/_/ /g;

  $dashed_line = $shorter_name;
  $dashed_line =~ s/./-/g;

  $header_copy =~ s/%1/$shorter_name/;
  $header_copy =~ s/%2/$dashed_line/g;

##print $header_copy;
##print $to_dump;

  open(TO_DUMP, "<$to_dump");

  print $header_copy;

  local($just_started) = 1;  # check for and remove blank lines at top of file.

  while (<TO_DUMP>) {
    local($line) = $_;
#local($i);
#for ($i = 0; $i < length($line); $i++) {
#$curr_char = substr($line, $i, 1);
#print "[$curr_char]";
#}
#print "\n";
    local($curr_char) = 0;
#print "start len=" . length($line) . "\n";
    do {
#print "chopping\n";
      chop $line;  # remove end of line char(s).
      $curr_char = substr($line, length($line) - 1, 1);
        # get new last char in string.
    } while ( ($curr_char eq "\r") || ($curr_char eq "\n") );
    local($do_print) = 1;
    if ($just_started) {
      if (length($line) == 0) {
        #continue;  # skip the first blank lines.
        $do_print = 0;  #no continue statement??
      } else {
        $just_started = 0;
          # a non-blank line has been seen.  now we reset our flag so we stop
          # checking for blank lines.
      }
#print "do print = $do_print\n";
    }
    if ($do_print) { print $line . "\n"; }
  }
}

