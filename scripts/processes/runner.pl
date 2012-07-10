#!/usr/bin/perl

###############################################################################
#                                                                             #
#  Name   : runner                                                            #
#  Author : Chris Koeritz                                                     #
#  Rights : Copyright (C) 1991-$now by Author                                 #
#                                                                             #
#  Purpose:                                                                   #
#                                                                             #
#    Executes the programs in the current directory and combines their        #
#  outputs into one stream.                                                   #
#                                                                             #
###############################################################################
#  This program is free software; you can redistribute it and/or modify it    #
#  under the terms of the GNU General Public License as published by the Free #
#  Software Foundation; either version 2 of the License or (at your option)   #
#  any later version.  See: "http://www.gruntose.com/Info/GNU/GPL.html" for a #
#  version of the License.  Please send any updates to "fred@gruntose.com".   #
###############################################################################

use Env qw(OS);

$run_dir = ".";
if ($#ARGV >= 0) {
  $run_dir = $ARGV[0];
}
#print "starting in directory $run_dir\n";

# float across the executables in this directory.
if ($OS =~ /UNIX/) {
  foreach $i ( `find $run_dir -type f -perm +111` ) { &run_it($i); }
} else {
  foreach $i ( `find $run_dir -iname "*.exe"` ) { &run_it($i); }
}

# the run_it function does the real work of writing out the log and executing
# the programs of interest.
sub run_it()
{
  local($program_name) = @_;
  chop $program_name;  # take off EOL.

#printf "program name is $program_name\n";

  if ($program_name =~ /\.log$/) { return; }
  print stderr "starting $program_name.\n";
  print "\n";
  print "\n";
  print "---- starting $program_name ----\n";
  # run the program now...
  if ( ($OS eq "Windows_NT") || ($OS eq "Windows_95") ) {
    # don't use nice when it doesn't exist.
    system($program_name);
  } else {
    system("nice -4 " . $program_name);
  }
# added nice in since some tests are brutal.
  print "---- $program_name is done ----\n";
  print "\n";
  print "\n";
  print stderr "ending $program_name.\n";
}

