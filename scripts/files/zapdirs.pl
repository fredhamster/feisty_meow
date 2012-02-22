#!/usr/bin/perl

###############################################################################
#                                                                             #
#  Name   : zapdirs                                                           #
#  Author : Chris Koeritz                                                     #
#  Rights : Copyright (C) 1996-$now by Author                                 #
#                                                                             #
#  Purpose:                                                                   #
#                                                                             #
#    Removes a list of directories that are expected to be empty.  This       #
#  cleans out any filenames that are considered unimportant first.            #
#                                                                             #
###############################################################################
#  This program is free software; you can redistribute it and/or modify it    #
#  under the terms of the GNU General Public License as published by the Free #
#  Software Foundation; either version 2 of the License or (at your option)   #
#  any later version.  See: "http://www.gruntose.com/Info/GNU/GPL.html" for a #
#  version of the License.  Please send any updates to "fred@gruntose.com".   #
###############################################################################

require "zap_the_dir.pl";

$DEV_NULL = "&>/dev/null"
#hmmm: move this to a useful location in a perl library.
if ($OS eq "UNIX") {
  $FIND_ENDING = "';'";
} elsif ( ($OS eq "DOS") || ($OS eq "Windows_95")
    || ($OS eq "Windows_98") || ($OS eq "Windows_NT") ) {
  $FIND_ENDING = "';'";
} else {
  die "The Operating System variable (OS) is not set.\n";
}

local(@to_zap) = ();  # the array to zap out.

if ($#ARGV < 0) {
  # no parms; give a default list.
  @to_zap = (".");
} else {
  @to_zap = &glob_list(@ARGV);
  if ($#to_zap < 0) {
    local($plural) = "";
    if ($#ARGV > 0) { $plural = "s"; }
    print "The directory name$plural \"@ARGV\" cannot be found.\n";
    exit 0;
  }
}

#print "zap list is:\n@to_zap\n";

&recursively_zap_dirs(@to_zap);

exit 0;

