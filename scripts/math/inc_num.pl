#!/usr/bin/perl

###############################################################################
#                                                                             #
#  Name   : inc_num                                                           #
#  Author : Chris Koeritz                                                     #
#  Rights : Copyright (C) 1996-$now by Author                                 #
#                                                                             #
#  Purpose:                                                                   #
#                                                                             #
#    Operates on a number stored in a text file.  the number can be retrieved #
#  for whatever purpose or it can be incremented.                             #
#                                                                             #
###############################################################################
#  This program is free software; you can redistribute it and/or modify it    #
#  under the terms of the GNU General Public License as published by the Free #
#  Software Foundation; either version 2 of the License or (at your option)   #
#  any later version.  See: "http://www.gruntose.com/Info/GNU/GPL.html" for a #
#  version of the License.  Please send any updates to "fred@gruntose.com".   #
###############################################################################

# A number is retrieved from the passed in filename and returned.
sub get_number {
  local($number_file) = @_;
#print "num file is $number_file\n";
  open(NUMBERING, "$number_file");
  local($number) = int(<NUMBERING>);
  if ($number <= 0) { $number = 1; }
  if ($number < 10) { $number = '0'.$number; }
  if ($number < 100) { $number = '0'.$number; }
  if ($number < 1000) { $number = '0'.$number; }
  close(NUMBERING);
  return $number;
}

# the number in the passed filename is increased.
sub next_number {
  local($number_file) = @_;
  local($number) = &get_number($number_file);
  if ($number < 0) { $number = '0000'; }
  $number++;
  if ($number > 9999) { $number = '0000'; }
  open(NUMBERING, "> $number_file");
#print "number is now $number\n";
  print NUMBERING "$number\n";
  close(NUMBERING);
}

# stores the number specified into the file specified.
sub store_number {
  local($number, $number_file) = @_;
#print "storing $number into $number_file\n";
  open(NUMBERING, "> $number_file");
  print NUMBERING "$number\n";
  close(NUMBERING);
}

1;

