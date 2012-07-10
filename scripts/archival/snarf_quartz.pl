#!/usr/bin/perl

###############################################################################
#                                                                             #
#  Name   : snarf_quartz                                                      #
#  Author : Chris Koeritz                                                     #
#  Rights : Copyright (C) 2000-$now by Author                                 #
#                                                                             #
#  Purpose:                                                                   #
#                                                                             #
#    Packs up an archive with the quartz repository.                          #
#                                                                             #
###############################################################################
#  This program is free software; you can redistribute it and/or modify it    #
#  under the terms of the GNU General Public License as published by the Free #
#  Software Foundation; either version 2 of the License or (at your option)   #
#  any later version.  See: "http://www.gruntose.com/Info/GNU/GPL.html" for a #
#  version of the License.  Please send any updates to "fred@gruntose.com".   #
###############################################################################

require "shared_snarfer.pl";

use Env qw(HOME);

&initialize_snarfer;

# get the number we use and increment it for the next use.
local($number) = &retrieve_number("aa_backup");

# variables for directory location to backup and the file to dump it in.
local($root) = "$HOME";
local($snarf_file_base) = &snarf_prefix("quartz");
local($snarf_file) = &snarf_name($snarf_file_base, $number);

# store the archive number in the file for retrieval on the other side.
&backup_number("aa_backup", $snarf_file_base, $number);

############################################################################

# backup all the hierarchies in our quartz directory, as well as any other well known
# repositories of goodness.
&backup_hierarchy($snarf_file_base, $number, "$root", "quartz");

############################################################################

# now rename the file so only the unpacker can access it.
&rename_archive($snarf_file);

exit 0;

