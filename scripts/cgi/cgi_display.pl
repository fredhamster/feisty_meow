#!/usr/bin/perl

###############################################################################
#                                                                             #
#  Name   : cgi_display                                                       #
#  Author : Chris Koeritz                                                     #
#  Rights : Copyright (C) 1996-$now by Author                                 #
#                                                                             #
#  Purpose:                                                                   #
#                                                                             #
#    Spews the given file out as a CGI-interpretable text file.               #
#                                                                             #
###############################################################################
# This program is free software; you can redistribute it and/or modify it     #
# under the terms of the GNU General Public License as published by the Free  #
# Software Foundation; either version 2 of the License, or (at your option)   #
# any later version.  See: "http://www.gruntose.com/Info/GNU/GPL.html" for a  #
# version of the License.  Please send any updates to "chris@gruntose.com".   #
###############################################################################

print "Content-type: text/plain\n\n";

# isolate the file showing capability to our web sites.
local($file) = "/var/www/" . $ENV{'QUERY_STRING'};
open(TO_READ, "<" . $file) || die "Could not open $file for reading.";
foreach $line (<TO_READ>) { print $line; }

exit 1;

