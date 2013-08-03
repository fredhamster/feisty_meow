#!/usr/bin/perl

##############
#  Name   : change_prefix
#  Author : Chris Koeritz
#  Rights : Copyright (C) 2002-$now by Author
##############
#  Purpose:
#    Replaces all of the matching first portions of files in the current
#  directory with a different chunk.  The first parameter should be the old
#  prefix and the second parameter should be the new prefix.
##############
#  This script is free software; you can redistribute it and/or modify it
#  under the terms of the GNU General Public License as published by the Free
#  Software Foundation; either version 2 of the License or (at your option)
#  any later version.  See "http://www.fsf.org/copyleft/gpl.html" for a copy
#  of the License online.  Please send any updates to "fred@gruntose.com".
##############

require "filename_helper.pl";

local($first) = $ARGV[0];
local($second) = $ARGV[1];

#print "parms are: $first and $second \n";

if ( !length($first) || !length($second) ) {
  print "change_prefix: requires two parameters--the old prefix to look for in this\n";
  print "directory and the new prefix to replace the old suffix with.\n";
  exit 0;
}

foreach $filename (&glob_list("./$first*")) {
  local $new_name = $filename;
  $new_name =~ s/$first/$second/g;
#print "old name $filename\n";
#print "new name will be $new_name\n";
  system("mv \"$filename\" \"$new_name\"");
}


