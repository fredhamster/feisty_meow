#!/usr/bin/perl

###############################################################################
#                                                                             #
#  Name   : snarf_notes                                                       #
#  Author : Chris Koeritz                                                     #
#  Rights : Copyright (C) 2000-$now by Author                                 #
#                                                                             #
#  Purpose:                                                                   #
#                                                                             #
#    Stuffs up an archive with quartz lists and current new notes.            #
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
local($root) = "$HOME";
local($base) = &snarf_prefix("notes");
local($snarf_file) = &snarf_name($base, $number);

# store the archive number in the file for retrieval on the other side.
&backup_number("aa_backup", $base, $number);

############################################################################

# get top level text files and other potentially important items...
&backup_files($base, $number, $root, ".", ("*.html", "*.txt"));
# backup all the hierarchies in our quartz directory.
&backup_hierarchy($base, $number, "$root", "quartz");

# gather any directories in our home that match these often recurring patterns.
&snarf_by_pattern("$root", "notes");
&snarf_by_pattern("$root", "project");
&snarf_by_pattern("$root", "issue");
&snarf_by_pattern("$root", "idea");
&snarf_by_pattern("$root", "crucial");
&snarf_by_pattern("$root", "list");
&snarf_by_pattern("$root", "task");

############################################################################

# now rename the file so only the unpacker can access it.
&rename_archive($snarf_file);

exit 0;

