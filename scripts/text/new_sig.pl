#!/usr/bin/perl

###############################################################################
#                                                                             #
#  Name   : new_sig                                                           #
#  Author : Chris Koeritz                                                     #
#  Rights : Copyright (C) 1996-$now by Author                                 #
#                                                                             #
#  Purpose:                                                                   #
#                                                                             #
#    Creates a new signature file using Nechung.                              #
#                                                                             #
###############################################################################
#  This program is free software; you can redistribute it and/or modify it    #
#  under the terms of the GNU General Public License as published by the Free #
#  Software Foundation; either version 2 of the License or (at your option)   #
#  any later version.  See: "http://www.gruntose.com/Info/GNU/GPL.html" for a #
#  version of the License.  Please send any updates to "fred@gruntose.com".   #
###############################################################################

require "filename_helper.pl";

use Env qw(HOME TMP);

# creates a new signature file for outgoing email.
local($temp_filename) = "$TMP/zz_signate";
open(FILE, ">" . $temp_filename) || die "couldn't open $temp_filename for writing.";

print FILE "_______ chosen by the Nechung Oracle Program [ http://gruntose.com/ ] _______\n";
print FILE "\n";
close(FILE);
local($bindir) = $ENV{'FEISTY_MEOW_BINARIES'};
$bindir = &sanitize_name($bindir);
$app_path = "$bindir/nechung";
if (-e $app_path) {
  system("$app_path >>$temp_filename");
  open(FILE, ">>" . $temp_filename) || die "couldn't open $temp_filename for writing.";
} else {
  open(FILE, ">>" . $temp_filename) || die "couldn't open $temp_filename for writing.";
  print FILE "nechung oracle program (NOP) could not be found.\n";
}
print FILE "\n";
print FILE "_____________ not necessarily my opinions, not necessarily not. _____________\n";
close(FILE);

open(FILE, "<" . $temp_filename) || die "couldn't open $temp_filename for reading.";
foreach $line (<FILE>) { print $line; }
close(FILE);

