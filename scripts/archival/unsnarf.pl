#!/usr/bin/perl

###############################################################################
#                                                                             #
#  Name   : unsnarf                                                           #
#  Author : Chris Koeritz                                                     #
#  Rights : Copyright (C) 1996-$now by Author                                 #
#                                                                             #
#  Purpose:                                                                   #
#                                                                             #
#    Unpacks a snarf file and updates the archive number for the file type.   #
#                                                                             #
###############################################################################
#  This program is free software; you can redistribute it and/or modify it    #
#  under the terms of the GNU General Public License as published by the Free #
#  Software Foundation; either version 2 of the License or (at your option)   #
#  any later version.  See: "http://www.gruntose.com/Info/GNU/GPL.html" for a #
#  version of the License.  Please send any updates to "fred@gruntose.com".   #
###############################################################################

require "shared_snarfer.pl";

# check that we received a parameter.
if ($#ARGV < 0) {
  &print_instructions;
  exit 1;
}

local($filename) = $ARGV[0];

&restore_archive($filename);

exit 0;

sub print_instructions {
  print "this program needs a filename as a parameter.  the file will be\n";
  print "unsnarfed into an appropriate subdirectory and the backup number\n";
  print "will be updated.\n";
  exit 1;
}


