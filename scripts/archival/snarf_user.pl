#!/usr/bin/perl

###############################################################################
#                                                                             #
#  Name   : snarf_user                                                        #
#  Author : Chris Koeritz                                                     #
#  Rights : Copyright (C) 2000-$now by Author                                 #
#                                                                             #
#  Purpose:                                                                   #
#                                                                             #
#    Bundles the configuration files that are commonly in a user's home dir.  #
#                                                                             #
###############################################################################
#  This program is free software; you can redistribute it and/or modify it    #
#  under the terms of the GNU General Public License as published by the Free #
#  Software Foundation; either version 2 of the License or (at your option)   #
#  any later version.  See: "http://www.gruntose.com/Info/GNU/GPL.html" for a #
#  version of the License.  Please send any updates to "fred@gruntose.com".   #
###############################################################################

require "shared_snarfer.pl";

use Env qw(HOME USER);

&initialize_snarfer;

# get the number we use and increment it for the next use.
local($number) = &retrieve_number("aa_backup");

# variables for directory location to backup and the file to dump it in.
local($root) = "$HOME";
local($snarf_file_base) = snarf_prefix("$USER");
local($snarf_file) = &snarf_name($snarf_file_base, $number);

# store the archive number in the file for retrieval on the other side.
&backup_number("aa_backup", $snarf_file_base, $number);

############################################################################

# backup all the config info for kde.
&backup_hierarchy($snarf_file_base, $number, "$root", ".kde");
# and get the config for gnome.
&backup_hierarchy($snarf_file_base, $number, "$root", ".local");

# get any dot files ending in "rc", or with "bash" or "profile" in them, or
# that start with "x".
&backup_files($snarf_file_base, $number, $root, ".",
    ("*rc", ".*bash*", ".*profile*", ".x*", ));

# get the ssh configuration files.
&backup_hierarchy($snarf_file_base, $number, "$root", ".ssh");

# now rename the file so only the unpacker can access it.
&rename_archive($snarf_file);

exit 0;

