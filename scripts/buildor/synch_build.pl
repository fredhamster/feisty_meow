#!/usr/bin/perl

###############################################################################
#                                                                             #
#  Name   : synch_build                                                       #
#  Author : Chris Koeritz                                                     #
#  Rights : Copyright (C) 2003-$now by Author                                 #
#                                                                             #
#  Purpose:                                                                   #
#                                                                             #
#    Compares the files in an installation to the files stored in the build   #
#  repository ($/dll and $/exe).  Any files differing in the installation     #
#  directory are copied from the repository to the installation.  This is a   #
#  cheaper form of an install program, without the extra rigmarole.           #
#                                                                             #
###############################################################################
#  This program is free software; you can redistribute it and/or modify it    #
#  under the terms of the GNU General Public License as published by the Free #
#  Software Foundation; either version 2 of the License or (at your option)   #
#  any later version.  See: "http://www.gruntose.com/Info/GNU/GPL.html" for a #
#  version of the License.  Please send any updates to "fred@gruntose.com".   #
###############################################################################

require "diff_lib.pl";

&synchronize_against_build(@ARGV);

exit 0;  # scram.

