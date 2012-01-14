#!/usr/bin/perl

###############################################################################
#                                                                             #
#  Name   : snarf_yeti                                                        #
#  Author : Chris Koeritz                                                     #
#  Rights : Copyright (C) 1996-$now by Author                                 #
#                                                                             #
#  Purpose:                                                                   #
#                                                                             #
#    Gathers together the useful shell files and standard databases.          #
#                                                                             #
###############################################################################
#  This program is free software; you can redistribute it and/or modify it    #
#  under the terms of the GNU General Public License as published by the Free #
#  Software Foundation; either version 2 of the License or (at your option)   #
#  any later version.  See: "http://www.gruntose.com/Info/GNU/GPL.html" for a #
#  version of the License.  Please send any updates to "fred@gruntose.com".   #
###############################################################################

require "importenv.pl";
require "shared_snarfer.pl";

&initialize_snarfer;

# get the number we use and increment it for the next use.
local($number) = &retrieve_number("aa_backup");

# variables for directory location to backup and the file to dump it in.
local($root) = $YETI_DIR;
local($base) = &snarf_prefix("yeti");
local($snarf_file) = &snarf_name($base, $number);

# store the current archive number in the file for retrieval on the
# other side.
&backup_number("aa_backup", $base, $number);

# get the whole yeti hierarchy in there.
&backup_hierarchy($base, $number, $root, ".");

# now rename the file so only the unpacker can access it.
&rename_archive($snarf_file);

exit 0;

