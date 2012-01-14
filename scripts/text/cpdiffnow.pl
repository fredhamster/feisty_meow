#!/usr/bin/perl

###############################################################################
#                                                                             #
#  Name   : cpdiff                                                            #
#  Author : Chris Koeritz                                                     #
#  Rights : Copyright (C) 1996-$now by Author                                 #
#                                                                             #
#  Purpose:                                                                   #
#                                                                             #
#    Compares files in one directory to files in another directory.  Any      #
#  files with the same name whose contents differ will be copied to the       #
#  destination directory, and their date will be set to "now".                #
#                                                                             #
###############################################################################
#  This program is free software; you can redistribute it and/or modify it    #
#  under the terms of the GNU General Public License as published by the Free #
#  Software Foundation; either version 2 of the License or (at your option)   #
#  any later version.  See: "http://www.gruntose.com/Info/GNU/GPL.html" for a #
#  version of the License.  Please send any updates to "fred@gruntose.com".   #
###############################################################################

require "diff_lib.pl";

&copy_diff_dirs_using_now(@ARGV);

exit 0;  # scram.

