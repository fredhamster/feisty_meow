#!/usr/bin/perl

###############################################################################
#                                                                             #
#  Name   : renlower                                                          #
#  Author : Chris Koeritz                                                     #
#  Rights : Copyright (C) 1996-$now by Author                                 #
#                                                                             #
#  Purpose:                                                                   #
#                                                                             #
#    This program renames all of the files in a specified directory to have   #
#  completely lower case names.                                               #
#                                                                             #
###############################################################################
#  This program is free software; you can redistribute it and/or modify it    #
#  under the terms of the GNU General Public License as published by the Free #
#  Software Foundation; either version 2 of the License or (at your option)   #
#  any later version.  See: "http://www.gruntose.com/Info/GNU/GPL.html" for a #
#  version of the License.  Please send any updates to "fred@gruntose.com".   #
###############################################################################

require "filename_helper.pl";

# call the primary subroutine to rename the files specified.

&rename_lower(@ARGV);

exit 0;

# takes a list of directories as arguments.  all of the files in each
# directory are renamed to their lower case equivalent.

sub rename_lower {
  # go through the list of files passed in.
  foreach $current (&glob_list(@_)) {
    if ($current =~ /[A-Z]/) {
#print "current is '$current'\n";
      local $old_name = $current;
#print "old name is '$old_name'\n";
      local $dir = &dirname($current);
      local $file = &basename($current);
      (local $lc_name = $file) =~ tr/[A-Z]/[a-z]/;
      local $new_name = $dir . $lc_name; 
#print "new name='$new_name'\n";
      local $intermediate_name = $dir . "RL" .  rand() . ".tmp";
#print "command A is: rename [$old_name] [$intermediate_name]\n";
#print "command B is: rename [$intermediate_name] [$new_name]\n";
      rename($old_name, $intermediate_name)
          || die "failed to do initial rename";
      rename($intermediate_name, $new_name)
          || die "failed to do secondary rename";
    }
  }
}

