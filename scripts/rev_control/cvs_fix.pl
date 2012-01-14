#!/usr/bin/perl

###############################################################################
#                                                                             #
#  Name   : cvs_fix                                                           #
#  Author : Chris Koeritz                                                     #
#  Rights : Copyright (C) 2001-$now by Author                                 #
#                                                                             #
#  Purpose:                                                                   #
#                                                                             #
#    Flips any win32 slashes into friendly forward slashes before running     #
#  the cvs binary.                                                            #
#                                                                             #
###############################################################################
#  This program is free software; you can redistribute it and/or modify it    #
#  under the terms of the GNU General Public License as published by the Free #
#  Software Foundation; either version 2 of the License or (at your option)   #
#  any later version.  See: "http://www.gruntose.com/Info/GNU/GPL.html" for a #
#  version of the License.  Please send any updates to "fred@gruntose.com".   #
###############################################################################

#require "filename_helper.pl";
#require "importenv.pl";

$new_cmd="cvs";

foreach $i (@ARGV) {
#  printf "$i\n";
  $i =~ s/[\/\\]*$//;
  $i =~ s/\\/\//g;
#  print " --> $i\n";
  if ($i =~ /^co$/) {
    # insert our modifiers here.
    $new_cmd = $new_cmd . " " . $i . " -P -kb ";
  } else {
    # normal addition of next parameter.
    $new_cmd = $new_cmd . " " . $i;
  }
}

#printf "new command is: \"$new_cmd\"\n";
#printf "are the -P and -kb flags in there?\n";
system("$new_cmd");

1;

