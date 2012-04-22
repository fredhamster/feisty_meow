#!/usr/bin/perl

###############################################################################
#                                                                             #
#  Name   : snarf_opensim                                                     #
#  Author : Chris Koeritz                                                     #
#  Rights : Copyright (C) 1996-$now by Author                                 #
#                                                                             #
#  Purpose:                                                                   #
#                                                                             #
#    Backs up the full set of opensim configuration files.                    #
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

&initialize_snarfer;  # let the snarfer hook us in.

# get the number attachment and increment it for the next use.
local($number) = &retrieve_number("aa_backup");

# variables used throughout here.
local($snarf_file_base) = &snarf_prefix("config-opensim");
local($snarf_file) = &snarf_name($snarf_file_base, $number);

# store the archive number in the file for retrieval on the other side.
&backup_number("aa_backup", $snarf_file_base, $number);

# the top directory where everything we're grabbing lives.
local($root) = &canonicalize(&glob_list("$HOME/opensim"));

# grab the top level stuff.
#&backup_files($snarf_file_base, $number, $HOME, ".",
#    ("*.sh"));
&backup_files($snarf_file_base, $number, $root, ".",
    ("*.sh"));

# snag the main config files.
&backup_files($snarf_file_base, $number, $root, "bin",
    ("OpenSim.ini", "Robust.ini", "OpenSim.exe.config", "Robust.exe.config"));

# snag the configuration include file.
&backup_files($snarf_file_base, $number, $root, "bin/config-include", ("GridCommon.ini"));

# get the region definitions.
&backup_hierarchy($snarf_file_base, $number, "$root", "bin/Regions");

# now rename the file so only the unpacker can access it.
&rename_archive($snarf_file);

exit 0;

