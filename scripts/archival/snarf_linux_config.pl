#!/usr/bin/perl

##############
#  Name   : snarf_linux_config
#  Author : Chris Koeritz
#  Rights : Copyright (C) 1996-$now by Author
#  Purpose:
#    Backs up the useful linux config files and names the file after the
#  host it's run on.
##############
#  This program is free software; you can redistribute it and/or modify it
#  under the terms of the GNU General Public License as published by the Free
#  Software Foundation; either version 2 of the License or (at your option)
#  any later version.  See: "http://www.gruntose.com/Info/GNU/GPL.html" for a
#  version of the License.  Please send any updates to "fred@gruntose.com".
##############

require "importenv.pl";
require "shared_snarfer.pl";

&initialize_snarfer;

# get the number we use and increment it for the next use.
local($number) = &retrieve_number("aa_backup");

# variables used throughout.
local($base) = snarf_prefix("linux_config");
local($snarf_file) = &snarf_name($base, $number);

# store the current archive number in the file for retrieval on the
# other side.
&backup_number("aa_backup", $base, $number);

# write a little file showing the disk status.
local($infofile) = $HOME . "/disk_info.txt";
unlink $infofile;
open(INF, ">>" . $infofile);
print INF "Current mount configuration:\n";
print INF "\n";
close(INF);
system("mount >>$infofile");
open(INF, ">>" . $infofile);
print INF "\n";
print INF "\n";
print INF "Current disk configuration:\n";
print INF "\n";
close(INF);
system("fdisk -l >>$HOME/disk_info.txt");
&backup_files($base, $number, $HOME, ".", ("disk_info.txt"));
unlink $infofile;

# backup the dpkg info.
&backup_files($base, $number, "/", "var/lib/dpkg", ("status*"));

# backup the crucial hierarchies in /var...
&backup_hierarchy($base, $number, "/", "var/named");
###not good: &backup_hierarchy($base, $number, "/", "var/lib/mysql");
###the mysql snarf is not necessarily usable, since we really should be
###backing up the databases by another means than this.
&backup_hierarchy($base, $number, "/", "var/lib/named/master");
&backup_hierarchy($base, $number, "/", "var/lib/webalizer");

# snag the grub bootloader files.
&backup_hierarchy($base, $number, "/", "boot/grub");

# now get the entire /etc hierarchy...
&backup_hierarchy($base, $number, "/", "etc");

# clean out extra files.
&remove_from_backup($base, $number, "etc/cups/ppds.dat*");
&remove_from_backup($base, $number, "etc/httpd/conf/ssl.crt/ca-bundle.crt");
&remove_from_backup($base, $number, "etc/locale/*");
&remove_from_backup($base, $number, "etc/opt/kde3/share/services/ksycoca");
&remove_from_backup($base, $number, "etc/preload.d/*");
&remove_from_backup($base, $number, "etc/rmt");
&remove_from_backup($base, $number, "etc/termcap");
&remove_from_backup($base, $number, "etc/X11/X");
&remove_from_backup($base, $number, "etc/X11/xkb/*");
&remove_from_backup($base, $number, "*.bak");
&remove_from_backup($base, $number, "*.cache");
&remove_from_backup($base, $number, "*.crt");
&remove_from_backup($base, $number, "*.old");
&remove_from_backup($base, $number, "*.schemas");
&remove_from_backup($base, $number, "*.so");
&remove_from_backup($base, $number, "*.xml");

# now rename the file so only the unpacker can access it.
&rename_archive($snarf_file);

exit 0;

