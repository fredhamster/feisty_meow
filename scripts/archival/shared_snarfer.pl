#!/usr/bin/perl

###############################################################################
#                                                                             #
#  Name   : shared_snarfer                                                    #
#  Author : Chris Koeritz                                                     #
#  Rights : Copyright (C) 1996-$now by Author                                 #
#                                                                             #
#  Purpose:                                                                   #
#                                                                             #
#    A shared library collection for "snarfing up" archives.  This uses the   #
#  compressed tar format for files ending in ".snarf" to store collections    #
#  of files, folders, hierarchies and so forth.                               #
#                                                                             #
###############################################################################
#  This program is free software; you can redistribute it and/or modify it    #
#  under the terms of the GNU General Public License as published by the Free #
#  Software Foundation; either version 2 of the License or (at your option)   #
#  any later version.  See: "http://www.gruntose.com/Info/GNU/GPL.html" for a #
#  version of the License.  Please send any updates to "fred@gruntose.com".   #
###############################################################################

require "filename_helper.pl";
require "inc_num.pl";

use Cwd;
use Sys::Hostname;
use File::Which;
use Env qw(FEISTY_MEOW_SCRIPTS TMP);

$null_log = "/dev/null";

$TMP =~ s/\\/\//g;  # fix the temp variable for ms-winders.

# defines an array of problematic entries we were told to deal with.
@missing_log = ();

# these files are considered unimportant and won't be included in the archive.
@junk_file_list = ("*~", "*.$$$", "3rdparty", "*.aps", "*.bak", "binaries",
    "*.bsc", "*.cgl", "*.csm", "CVS", "Debug", "*.dll", "*.err", "*.exe",
    "generated_*", "*.git", "*.glb", "inprogress", "ipch", "*.llm",
    "*.log", "*.lnk",
    "makefile.fw*", "*.mbt", "*.mrt", "*.ncb", "*.o", "obj", "*.obj",
    "octalforty.Wizardby", "*.obr", "*.opt", "packages", 
    "*.pch", "*.pdb", "*.plg", "*.r$p", "*.rcs", "Release",
    "*.res", "*.RES", "*.rws", "*.sbr", "*.scc", "*.spx", "*.stackdump",
    "Steam",
    "*.sdf", "*.suo", ".svn", "*.sym", "*.td", "*.tds", "*.tdw", "*.tlb",
    "*.trw", "*.tmp", "*.tr", "*.user", "*_version.h", "*_version.rc",
    "*.vspscc", "waste", "zeitgeist");
#print "junk list=@junk_file_list\n";
@excludes = ();
for (local($i) = 0; $i < scalar(@junk_file_list); $i++) {
  push(@excludes, "--exclude=$junk_file_list[$i]");
}
#print "excludes list=@excludes\n";

# generic versions work on sane OSes.
$find_tool = which('find');
$tar_tool = which('tar');
#print "find tool: $find_tool\n";
#print "tar tool: $tar_tool\n";

if ( ! -f "$find_tool" || ! -f "$tar_tool" ) {
  print "Could not locate either tar or find tools for this platform.\n";
  exit 3;
}

# this is somewhat ugly, but it sets up a global variable called
# "original_path" so we remember where we started.
sub initialize_snarfer {
  $original_path = cwd();
  $original_path =~ s/\\/\//g;
}

# returns the current hostname, but without any domain included.
sub short_hostname {
  local($temphost) = hostname();
#&hostname();
  $temphost =~ s/([^.]*)\..*/\1/;
  return &lower($temphost);
}

# takes the base name and creates the full snarf prefix name, which includes
# a timestamp and hostname.
sub snarf_prefix {
  local($base) = @_;

#hmmm: extract this shared code to new function (also in safedel)
  $date_tool = "date";
  local($date_part) = `$date_tool +%Y-%m-%d-%H%M`;
  while ($date_part =~ /[\r\n]$/) { chop $date_part; }

  local($host) = &short_hostname();
  while ($host =~ /[\r\n]$/) { chop $host; }
  $base = $base . "_" . $host . "_" . $date_part;
  return $base;
}

# returns the name of the file to create based on the prefix and the
# current archive number.
sub snarf_name {
  local($prefix, $number) = @_;
  local($path) = &canonicalize($original_path);
  local($target_file) = $path . '/' . $prefix . "_" . $number . ".tar";
  return $target_file;
}

# finishes up on the archive file.
sub finalize_snarf {
  local($filename) = @_;
#print "finalizing now on filename $filename\n";
  local($outcome) = 0xff & system "gzip", $filename;
  if ($outcome) { die("failure to finalize"); }

  if (scalar(@missing_log)) {
    print "The following files or directories were missing:\n";
    print "@missing_log\n";
  }
}

# fixes the directory passed in, if required.  this is only needed for
# dos-like operating systems, where there are drives to worry about and where
# cygwin refuses to store the full path for an absolute pathname in the
# archive.  instead of letting it store partial paths, we change to the top
# of the drive and scoop up the files using a relative path.
sub chdir_to_top {
  local($directory) = @_;
  if ( (substr($directory, 0, 2) eq "//")
      && (substr($directory, 3, 1) eq "/") ) {
#print "into special case\n";
    # it was originally a dos path, so now we must do some directory changing
    # magic to get the paths to work right.
    local($drive) = substr($directory, 0, 4);  # get just drive letter biz.
#print "going to change to $drive\n";
    chdir($drive);
#print "cwd now=" . cwd() . "\n";
    $directory = substr($directory, 4);  # rip off absolutist path.
#print "using dir now as $directory\n";
    if (length($directory) == 0) {
#print "caught zero length dir, patching to dot now.\n";
      $directory = ".";
    }
  }
  return $directory;
}

# snarfer scoops up some files in a directory.
sub snarfer {
  local($prefix, $number, $root, $subdir, @extra_flags) = @_;
#print "prefix=$prefix, num=$number, root=$root, subdir=$subdir, extra=@extra_flags\n";

  $root = &chdir_to_top($root);

  local($target_file) = &snarf_name($prefix, $number);

  $random_num = (rand() * 1000000) % 1000000;
  $temp_file = `mktemp "$TMP/zz_snarf_tmp.XXXXXX"`;
  chop($temp_file);

  if (! -d $root . "/" . $subdir) {
    local($base) = &basename($root . "/" . $subdir);
#print "adding to missing in snarfer A: $base\n";
    push(@missing_log, $base);
    return 0;
  }
  local($currdir) = cwd();
  chdir($root);

  local($outcome) = 0;
  my @lines = qx( $find_tool "$subdir" @extra_flags "-type" "f" );
#  if ( ($! != 0) || ($? != 0) ) {
#    die("failure to find files in $subdir"); 
#  }

  open TEMPY_OUT, ">>$temp_file" or die "cannot open $temp_file";
  foreach (@lines) { print TEMPY_OUT "$_"; }
  close TEMPY_OUT;

  if (-s $temp_file == 0) {
    local($base) = &basename($root . "/" . $subdir);
#print "adding to missing in snarfer B: $base\n";
    push(@missing_log, $base);
  }

  local($outcome) = 0xff & system $tar_tool, 
#hmmm: trying to dereference symbolic links and stop missing stuff.
"-h",
      "-rf", &canonicalize($target_file), @excludes,
      "--files-from=" . &canonicalize($temp_file);
  if ($outcome) {
    unlink($temp_file);
    die("failure to archive");
  }
  # clean up temporaries.
  unlink($temp_file);
  # change back to previous directory.
  chdir($currdir);
}

# snarf_file_list is like snarfer but expects a file pattern at the end rather
# than a directory name.
sub snarf_file_list {
  local($prefix, $number, $root, $file_pattern, @extra_flags) = @_;

#print "prefix=$prefix, num=$number, root=$root, file_pattern=$file_pattern, extra=@extra_flags\n";

  $root = &chdir_to_top($root);

  local($target_file) = &snarf_name($prefix, $number);

  local($currdir) = cwd();
#print "got root as: '$root'\n";
  chdir("$root");

  local(@files) = &glob_list($file_pattern);
  if (!scalar(@files)) {
    local($base) = $root . "/" . $file_pattern;
    $base =~ s/\/\//\//g;
#print "adding to missing in snarf_file_list: $base\n";
    push(@missing_log, $base);
  }

  foreach $i (@files) {
    if ($i =~ /^\.\//) {
      $i = substr $i, 2, length($i) - 2;
    }
    local($outcome) = 0xff & system $tar_tool,
#"--directory=" . "$root",
        @extra_flags, 
#hmmm: trying to dereference symbolic links and stop missing stuff.
"-h",
"-rf", &canonicalize($target_file), @excludes, $i;
    if ($outcome) { die("failure to archive"); }
  }
  chdir("$currdir");
}

# backup some specific files.
sub backup_files {
  local($prefix, $number, $root, $subdir, @files) = @_;
#print "backup_files: ref=$prefix, num=$number, subdir=$subdir, list of files=@files\n";
  foreach $i (@files) {
    local($new_path) = $subdir . "/" . $i;
    if ($subdir eq ".") { $new_path = "$i"; }
    &snarf_file_list($prefix, $number, $root, $new_path);
  }
}

# backup some specific directories.
sub backup_directories {
  local($prefix, $number, $root, $subdir, @dirs) = @_;
#print "backup_directories: ref=$prefix, num=$number, root=$root, subdir=$subdir, list of dirs=@dirs.\n";
  foreach $i (@dirs) {
    local($path_to_use) = $subdir . "/" . $i;
    if ($i eq ".") {
      $path_to_use = $subdir;
    }
    &snarfer($prefix, $number, $root, $path_to_use, ("-maxdepth", "1"));
  }
}

# removes items from the file that match a pattern.
sub remove_from_backup {
  local($prefix, $number, $pattern) = @_;
#print "remove_from_backup: pref=$prefix, num=$number, patt=$pattern,\n";
  local($target_file) = &snarf_name($prefix, $number);

  open(TARPROC, "$tar_tool --delete -f " . &canonicalize($target_file)
      . " \"$pattern\" 2>$null_log |");
  <TARPROC>;
}

# recursively scoops up a directory hierarchy.
sub backup_hierarchy {
  local($prefix, $number, $root, $filepart) = @_;
#print "backup_hierarchy: pref=$prefix, num=$number, root=$root, filepart=$filepart\n";
  local(@locus_temp) = &glob_list($root);
  local($save_root) = $root;
  local($root) = $locus_temp[0];
  if (!length($root)) {
    local($base) = $save_root . "/" . $filepart;
#print "adding to missing in backup_hierarchy A: $base\n";
    push(@missing_log, $base);
    return;
  }
  local($new_pattern) = "$root/$filepart";
  if ($root =~ /\/$/) { $new_pattern = "$root$filepart"; }
  local(@mod_locus) = &glob_list($new_pattern);
  if (!scalar(@mod_locus)) {
    local($base) = &basename($root . "/" . $filepart);
#print "adding to missing in backup_hierarchy B: $base\n";
    push(@missing_log, $base);
  } else {
    foreach $i (@mod_locus) {
      local($new_locus) = $root;
      local $offset_len = length($root) + 1;
      local $char_len = length($i) - length($root) - 1;
      # make sure we don't double slashes up if one's already there.
      if ($root =~ /\/$/) { $offset_len--; $char_len++; }
      local($extra_portion) = substr $i, $offset_len, $char_len;
      if (!length($extra_portion)) {
        # well, in this case, there's nothing left of the extra part after
        # the root.  we'll push the last component of the root down into
        # the extra part so there's actually something to traverse.
        $new_locus = &dirname($root);
        $extra_portion = &basename($root);
      }
      &snarfer($prefix, $number, $new_locus, $extra_portion, ());
    }
  }
}

# recursively scoop up a list of directory hierarchies.
sub backup_hierarchies {
  local($prefix, $number, $root, @dirs) = @_;
#  print "backup_hierarchy: pref=$prefix, num=$number, root=$root,\n";
#  print "list of dirs=@dirs.\n";
  foreach $i (@dirs) {
    &backup_hierarchy($prefix, $number, $root, $i);
  }
}

# grab up all the files in a directory (second parm) that are named matching
# a simple text pattern (third parm).  if there is a fourth parameter, it is
# used as an extra directory component after the main directory.
sub snarf_by_pattern {
  local($prefix, $dir, $pattern, $extra_component) = @_;
  local($had_extra) = length($extra_component) != 0;
#print "snarf by pattern, dir = $dir, patt = $pattern, extra = $extra_component\n";
  if ($had_extra) {
    $dir = "$dir/$extra_component";
  }
  @dir_contents = &glob_list("$dir/*$pattern*"); 
#print "dir contents: @dir_contents\n";

  if (!scalar(@dir_contents)) {
    print "no '$pattern' directores were backed up in $dir.\n";
  }
  
  foreach $item (@dir_contents) {
    if ( ($item =~ /$pattern.*snarf/) || ($item =~ /$pattern.*tar/) ) { next; }
    if ( ! -d "$item" ) { next; }
#print "now really planning to backup hier of $item\n";
    # normal backup had no extra component.
    local $upper_dir = &dirname($item);
    local $dir_plus_base = &basename($item);
    # if we did have an extra component, we do this a bit differently.
    if ($had_extra) {
      $upper_dir = &dirname( &dirname($item) );
      $dir_plus_base = &basename( &dirname($item) ) . "/" . &basename($item);
    }
#print "using upper=$upper_dir and dir+base=$dir_plus_base\n";
    &backup_hierarchy($prefix, $number, $upper_dir, $dir_plus_base);
  }
}

# gets the number out of the file specified by a basename.  the number file
# is assumed to be stored in the TMP directory and to have an extension of
# ".num".
sub retrieve_number {
  local($number_prefix) = @_;
  # get number from the file specified and increment it for the next use.
  local($NUMBER_FILE) = $TMP."/$number_prefix.num";
  local($number) = &get_number($NUMBER_FILE);
  &next_number($NUMBER_FILE);
  return $number;
}

# takes a name to use as the basename for a number file, and stores the
# file into the archive specified.
sub backup_number {
  local($number_prefix, $snarf_prefix, $number) = @_;
#print "backup_number: parms are: numpref=$number_prefix, archpref=$snarf_prefix, num=$number.\n";
  local($target_file) = $original_path ."/". $snarf_prefix . "_" . $number . ".tar";
  local($number_file) = $number_prefix . ".num";

  local($currdir) = cwd();
  chdir($TMP);

  local($outcome) = 0xff & system $tar_tool, "-cf",
      &canonicalize($target_file), &canonicalize($number_file);
  if ($outcome) { die("failure to archive"); }

  local($prefix_file) = "prefix.bac";
  open(NUM_PREFIX, ">" . $prefix_file);
  print NUM_PREFIX $number_prefix;
  close(NUM_PREFIX);

  $outcome = 0xff & system $tar_tool, "-rf",
      &canonicalize($target_file), &canonicalize($prefix_file);
  if ($outcome) { die("failure to archive"); }
  unlink($prefix_file);
  chdir($currdir);
}

# takes a prefix for the number file and a filename where it can be found.
# the current number in the temporary directory is compared against the file,
# and the new number's used if it's greater.
sub restore_number {
  local($number_prefix, $number_file) = @_;
#print "restore num has numpref $number_prefix and numfile $number_file\n";
  local($comparison_file) = "$TMP" . "/" . $number_prefix . ".num";
  local($number) = &get_number($number_file);
  local($old_number) = &get_number($comparison_file);
  if ($number > $old_number) {
    &store_number($number, $comparison_file);
  }
  unlink($number_file);
}

# ensures that the special restoration program is used on the archives by
# renaming their extension.
sub rename_archive {
  local($filename) = @_;
#print "rename_archive: file=$filename\n";
  &finalize_snarf($filename);
  local(@pieces) = split(/\.[^.]*$/, $filename, 3);
  local($just_dir_and_base) = $pieces[0];
  local($new_name) = $just_dir_and_base . '.snarf'; 
  rename($filename . ".gz", $new_name)
      || die("could not rename $filename to $new_name.");
}

# undoes a snarfed up archive and pulls out the number.
sub restore_archive {
  local($filename) = &canonicalize(&remove_trailing_slashes(@_));
  local(@split_name) = &split_filename($filename);
  if ($#split_name < 1) {
    print "The name \"$filename\" could not be parsed for restoration.\n";
    exit 1;
  }
  # get the basename of the file.
  local(@pieces) = split(/\.[^.]*$/, @split_name[1], 2);
  # we don't want the extension.
  local($just_dir_and_base) = $split_name[0] . $pieces[0];
  # now just get the basename without a directory.
  local(@name_components) = split(/\//, $just_dir_and_base);
  local($basename) = $name_components[$#name_components];
  local($new_dir_name) = 'snarf_' . $basename;

  local($currdir) = cwd();

  if (!chdir($new_dir_name)) {
    mkdir($new_dir_name, 0777)
        || die("could not create directory $new_dir_name.");
    if (!chdir($new_dir_name)) {
      die("could not change to directory $new_dir_name.");
    }
  }

  # patch a relative path name to reflect the fact that we're now underneath
  # the directory where we were.
  if (! ($filename =~ /^\//) 
      && ! ($filename =~ /^.:/) ) {
    $filename = "../" . $filename;
  }

  local($outcome) = 0xff & system $tar_tool, "-xzf",
      &canonicalize($filename);
  if ($outcome) { die("failure to undo archive"); }

  local($outcome) =
      0xff & system "bash", "$FEISTY_MEOW_SCRIPTS/security/normal_perm.sh", ".";
  if ($outcome) { die("failure to normalize permissions"); }

  # remove any links that might have crept in; these can cause mischief.
  local($outcome) = 0xff & system("$find_tool . -type l -exec rm {} ';'");

  # read the name of the prefix file.
  local($prefix_file) = "prefix.bac";
  open(NUM_PREFIX, "<" . $prefix_file);
  local($number_prefix) = <NUM_PREFIX>;
  close(NUM_PREFIX);

  &restore_number($number_prefix, $number_prefix . ".num");
  unlink($prefix_file);

  chdir($currdir);
}

1;

