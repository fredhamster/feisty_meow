#!/usr/bin/perl

###############################################################################
#                                                                             #
#  Name   : snarf_hoople2                                                     #
#  Author : Chris Koeritz                                                     #
#  Rights : Copyright (C) 1996-$now by Author                                 #
#                                                                             #
#  Purpose:                                                                   #
#                                                                             #
#    Backs up the full set of hoople 2.0 source code, plus some extra stuff.  #
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
local($base) = &snarf_prefix("hoople2");
local($snarf_file) = &snarf_name($base, $number);

# store the archive number in the file for retrieval on the other side.
&backup_number("aa_backup", $base, $number);

# the top directory where everything we're grabbing lives.
local($root) = &canonicalize("$HOME/hoople2");

# grab the top level stuff.
&backup_files($base, $number, $root, ".", ("*.txt", "make*"));

# get the documentation directory.
&snarfer($base, $number, $root, "docs", ("-maxdepth", "1"));
# get an extra folder we like.
&backup_hierarchy($base, $number, "$root", "docs/text_examples");

# get the build scripts.
&backup_files($base, $number, $root, "build", ("*.ini"));
&backup_hierarchy($base, $number, "$root", "build/clam");
&backup_hierarchy($base, $number, "$root", "build/generator");
&backup_hierarchy($base, $number, "$root", "build/config");
&backup_hierarchy($base, $number, "$root", "build/jenkins");

# get all C++ code projects by resetting variable for main grab.
&backup_hierarchy($base, $number, "$root/source", ".");

# now rename the file so only the unpacker can access it.
&rename_archive($snarf_file);

exit 0;

