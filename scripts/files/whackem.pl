#!/usr/bin/perl

###############################################################################
#                                                                             #
#  Name   : whack_forever                                                     #
#  Author : Chris Koeritz                                                     #
#  Rights : Copyright (C) 1992-$now by Author                                 #
#                                                                             #
#  Purpose:                                                                   #
#                                                                             #
#    Zaps a list of files.  This file exists since the default in the YETI    #
#  shell environment is to compress files when deletion is attempted.  Only   #
#  the whack_forever command actually deletes the files for real.             #
#                                                                             #
###############################################################################
# This program is free software; you can redistribute it and/or modify it     #
# under the terms of the GNU General Public License as published by the Free  #
# Software Foundation; either version 2 of the License, or (at your option)   #
# any later version.  See: "http://www.gruntose.com/Info/GNU/GPL.html" for a  #
# version of the License.  Please send any updates to "fred@gruntose.com".    #
###############################################################################

require "filename_helper.pl";

sub interrupt_handler {
    # skip doing any deletions.
    print "\nbailing out due to interrupt.  not doing any remaining deletions.\n";
    exit(0);
}

$SIG{'INT'} = 'interrupt_handler';  # trigger our function instead of continuing.

local($flags) = "";

local(@whackers) = @ARGV;

while ($whackers[0] =~ /^-/) {
  # this one is a special flag to pass.  don't try to whack that.
  $flags = $flags . @whackers[0] . ' ';
  shift(@whackers);
}

#print "flags are $flags\n";

@whackers = &glob_list(@whackers);

if (scalar(@whackers) > 0) {
  print "ZAPPING FOREVER! @whackers ...\n";
  system("sleep 4") == 0 || &interrupt_handler;
  print "\nNow really deleting files! => @whackers\n";
}


foreach $i (@whackers) {
  if (-l $i) {
    unlink($i) || print "failed to unlink $i\n";
  } elsif (-d $i) {
    system("rm $flags \"$i\"");
  } else {
    unlink($i) || print "failed to unlink $i\n";
  }
}

