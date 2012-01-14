#!/usr/bin/perl

###############################################################################
#                                                                             #
#  Name   : change endings                                                    #
#  Author : Chris Koeritz                                                     #
#  Rights : Copyright (C) 2002-$now by Author                                 #
#                                                                             #
#  Purpose:                                                                   #
#                                                                             #
#    Replaces all of the endings on files in the current directory with       #
#  a different ending.  The first parameter should be the old ending and the  #
#  second parameter should be the new ending.                                 #
#                                                                             #
###############################################################################
#  This script is free software; you can redistribute it and/or modify it     #
#  under the terms of the GNU General Public License as published by the Free #
#  Software Foundation; either version 2 of the License or (at your option)   #
#  any later version.  See "http://www.fsf.org/copyleft/gpl.html" for a copy  #
#  of the License online.  Please send any updates to "fred@gruntose.com".    #
###############################################################################

# change_endings: takes all of the files in the current directory ending in $1
# and modifies their suffix to be $2.

require "filename_helper.pl";

#require "importenv.pl";
#require "inc_num.pl";

local($first) = $ARGV[0];
local($second) = $ARGV[1];

#print "parms are: $first and $second \n";

if ( !length($first) || !length($second) ) {
  print "change_endings: requires two parameters--the old suffix to look for in this\n";
  print "directory and the new suffix (without dots) to change the old suffix to.\n";
  exit 0;
}

foreach $filename (&glob_list("./*$first")) {
#old  print "file=$filename\n";
#old  local($ext_temp) = &extension($filename);
#old  print "ext is: $ext_temp\n";
#old  local($no_ext) = &non_extension($filename);
#old  print "cmd is: mv \"$filename\" \"$no_ext$second\"\n";
#old  system("mv \"$filename\" \"$no_ext$second\"");

  local $new_name = $filename;
  $new_name =~ s/$first/$second/g;
#print "old name $filename\n";
#print "new name would be $new_name\n";

  system("mv \"$filename\" \"$new_name\"");
}


