#!/usr/bin/perl

##############
#  Name   : snarf_feisty_meow
#  Author : Chris Koeritz
#  Rights : Copyright (C) 1996-$now by Author
#  Purpose:
#    Gathers together the useful shell files and standard databases.
#    Backs up the full set of hoople 2.0 source code, plus some extra stuff.
##############
#  This program is free software; you can redistribute it and/or modify it    #
#  under the terms of the GNU General Public License as published by the Free #
#  Software Foundation; either version 2 of the License or (at your option)   #
#  any later version.  See: "http://www.gruntose.com/Info/GNU/GPL.html" for a #
#  version of the License.  Please send any updates to "fred@gruntose.com".   #
##############

require "shared_snarfer.pl";

use Env qw(HOME);

&initialize_snarfer;  # let the snarfer hook us in.

# get the number attachment and increment it for the next use.
local($number) = &retrieve_number("aa_backup");

# variables used throughout here.
local($snarf_file_base) = &snarf_prefix("feisty_meow");
local($snarf_file) = &snarf_name($snarf_file_base, $number);

# store the archive number in the file for retrieval on the other side.
&backup_number("aa_backup", $snarf_file_base, $number);

# the top directory where everything we're grabbing lives.
local($root) = &canonicalize("$HOME/feisty_meow");

# grab the top level stuff.
&backup_files($snarf_file_base, $number, $root, ".", ("*.txt", "make*", ".gitignore"));

# snarf up all the important directories.
# CAK: current as of 2012-05-05.
&backup_hierarchy($snarf_file_base, $number, $root, "customizing");
&backup_hierarchy($snarf_file_base, $number, $root, "database");
&backup_hierarchy($snarf_file_base, $number, $root, "doc");
&backup_hierarchy($snarf_file_base, $number, $root, "examples");
&backup_hierarchy($snarf_file_base, $number, $root, "feisty_inits");
&backup_hierarchy($snarf_file_base, $number, $root, "graphiq");
&backup_hierarchy($snarf_file_base, $number, $root, "huffware");
&backup_hierarchy($snarf_file_base, $number, $root, "kona");
&backup_hierarchy($snarf_file_base, $number, $root, "nucleus");
&backup_hierarchy($snarf_file_base, $number, $root, "octopi");
&backup_hierarchy($snarf_file_base, $number, $root, "osgi");
&backup_hierarchy($snarf_file_base, $number, $root, "scripts");
&backup_hierarchy($snarf_file_base, $number, $root, "webby");

# grab the production assets.
&backup_files($snarf_file_base, $number, $root, "production", ("*.ini", "make*"));
&backup_hierarchy($snarf_file_base, $number, "$root", "production/setup_src");

# now rename the file so only the unpacker can access it.
&rename_archive($snarf_file);

exit 0;

