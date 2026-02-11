#! /usr/bin/env python3

"""

Name   : generate_aliases
Author : Chris Koeritz
Rights : Copyright (C) 1996-$now by Author

Purpose:

  This script generates feisty meow script alias files.  Alias files
contain a list of definitions for command aliases that are written in the
specified shell dialect (such as bash or perl) and which are additionally
tailored for the operating system to be used.

author: chris koeritz

####
This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2 of the License or (at your option)
any later version.  See: "http://www.gruntose.com/Info/GNU/GPL.html" for a
version of the License.  Please send any updates to "fred@gruntose.com".
"""

import inspect
import os
import re
import sys

# locate where our scripts live, so we can find local library files.
THIS_SCRIPT_DIR = os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))
FILE_PROCESSING_LIBRARY = THIS_SCRIPT_DIR + '/../files/'
sys.path.append(FILE_PROCESSING_LIBRARY)

import filename_helper

# returns true if the environment variable to enable debugging noise is set.
def is_debugging():
  return len(DEBUG_FEISTY_MEOW) > 0

# given a possible aliasable filename, this will decide whether to create a perl
# or bash alias for it.  it needs the filename of the possible alias and the
# directory where that file resides.
def make_alias(file: str, dir: str) -> None:
  # we'll set the shorter alias name if we find a match for the file extension.
  aliasname = None
  # the method we'll call once we figure out what type of alias to build.
  funky = None

  alias_handling_methods = {'py':'make_python_alias', 'sh':'make_bash_alias', 'pl':'make_perl_alias'}

  for extension, method in alias_handling_methods.items():
    found = re.search('^.*\.' + extension, file, re.IGNORECASE)
    if found:
      aliasname = re.sub('^.*\.' + extension, "", file, re.IGNORECASE)
      funky = method
      break

  if aliasname is not None:
    if is_debugging():
      print("aliasname is " + aliasname + " and funky is " + funky)
    # evaluate a function call with the chosen method.
    return eval(funky+'(' + aliasname + ',' + dir + ')');
  else:
    print('could not find a matching extension for the file: ' + file)
    return None

####

# makes an alias for a bash script given the alias name.
def make_bash_alias(aliasname: str, dir: str) -> str:
  full_alias = dir + "/" + aliasname
  if is_debugging():
    print("bash full alias " + full_alias)
#huh?  aliasname = re.sub(r'^.*/([^/]*)', r'\1')
#from:  $aliasname =~ s/^.*\/([^\/]*)/\1/;
#  print "alias became: " + aliasname
  return "define_yeti_alias: " + aliasname+ '="bash "' + full_alias + '".sh"';
#  print she "define_yeti_alias $aliasname=\"bash $full_alias.sh\"";

# makes an alias for a python script given the alias name.
def make_python_alias(aliasname: str, dir: str) -> str:
  full_alias = dir + "/" + aliasname
  if is_debugging():
    print("python full alias: " + full_alias)
#hmmm: don't love that we're hardcoding python3 in here, but apparently some systems don't have a 'python' command despite having python installed.
  return "define_yeti_alias " + aliasname+ '="python3 "' + full_alias + '".py"';

# makes an alias for a perl script given the alias name.
def make_perl_alias(aliasname: str, dir: str) -> str:
  full_alias = dir + "/" + aliasname
  if is_debugging():
    print("perl full alias: " + full_alias)
  return "define_yeti_alias " + aliasname+ '="perl "' + full_alias + '".py"';

##############

# The "common.alias" file is used in the generated aliases file as a base
# set of generally useful aliases.  We also add aliases for any script files
# (perl, bash, python, etc) that we find in the feisty meow script hierarchy.
# Any *.alias files found in the $FEISTY_MEOW_LOADING_DOCK/custom folder are
# loaded also.
def rebuild_script_aliases() -> None:

  if is_debugging():
    print("rebuilding generated aliases file...")

  # create our generated shells directory if it's not already.
  if not os.path.isdir(FEISTY_MEOW_LOADING_DOCK): 
    os.mkdirs(FEISTY_MEOW_LOADING_DOCK)
    if is_debugging():
      print("made FEISTY_MEOW_LOADING_DOCK at '" + FEISTY_MEOW_LOADING_DOCK + "'")

#hmmm: not sure why this bit was removed from the perl code--maybe it blew up or made noise or didn't work right?
  # test if we can use color in ls...
#  $test_color=` ls --help 2>&1 | grep -i color `;

  # this is an array of files from which to draw alias definitions.
  ALIAS_DEFINITION_FILES = [ FEISTY_MEOW_SCRIPTS + "/core/common.alias" ];

  # if custom aliases files exist, add them to the list.
#hmmm: would be nice to have this name in a symbol somewhere instead of having "custom" or "customize" everywhere.
  for filename in glob_list(FEISTY_MEOW_LOADING_DOCK + "/custom/*.alias"):
    if os.path.isfile(filename): ALIAS_DEFINITION_FILES.append(filename)
  if is_debugging():
    print("using these alias files:")
    for filename in ALIAS_DEFINITION_FILES:
      base_of_dir = os.path.basename(os.path.dirname(filename))
      basename = os.path.basename(filename)
      print("  " + base_of_dir + "/" + basename)

  # write the aliases for sh and bash scripts.
  GENERATED_ALIAS_FILE = FEISTY_MEOW_LOADING_DOCK + "/fmc_core_and_custom_aliases.sh"
  if is_debugging():
    print("writing generated aliases in " + GENERATED_ALIAS_FILE + "...")

#hmmm: perhaps a good place for a function to create the header,
#      given the appropriate comment code.

  try:
    with open(GENERATED_ALIAS_FILE, "w") as GENOUT:
      GENOUT.write("##")
      GENOUT.write("## generated file: " + GENERATED_ALIAS_FILE)
      GENOUT.write("## please do not edit.")
      GENOUT.write("##")

#hmmm: old handling for the color addition.
#      starting to remember that maybe i hated where this code was being added?  and that's why it was removed?  maybe?
#  if (length($test_color)) {
#    print GENOUT "export color_add='--color=auto'";
#  } else {
#    print GENOUT "export color_add=";
#  }

    # plow in the full set of aliases into the file.
    for filename in ALIAS_DEFINITION_FILES:
      try:
        with open(filename, "r") as CURR_ALIASER:
          for line in CURR_ALIASER:
            GENOUT.write(line)
      except:
        print("cannot open current alias file: " + filename + "; skipping it.")

  except:
    print("cannot open generated aliases in " + GENERATED_ALIAS_FILE)
    exit(1)

  if is_debugging():
    print("done rebuilding generated aliases file.");

##############

#hmmm: move this to filename helpers
def add_permission(filename: str, perm_adjustment: int) -> None:
  """ Adds a permission mask into the existing permissions for the file.  Uses the stat values for file permissions. """
  # get the existing permissions.
  stats = os.stat(filename)
  # add in the requested new items.
  stats |= perm_adjustment
  # save the results back to the file.
  os.chmod(filename, stats)

##############

def not_customize(filename: str):
  """
  returns true if the filename string does not have 'customize' in it.
  this indicates that the file is not located under our customization hierarchy.
  """
  return not re.search("customize", str, re.IGNORECASE)

##############

def main() -> None:
  """ the main driver of activities for this app. """

  # load some variables from the environment, if we can.
  HOME = os.environ['HOME']
  FEISTY_MEOW_BINARIES = os.environ['FEISTY_MEOW_BINARIES']
  BUILD_TOP = os.environ['BUILD_TOP']
  FEISTY_MEOW_APEX = os.environ['FEISTY_MEOW_APEX']
  FEISTY_MEOW_LOADING_DOCK = os.environ['FEISTY_MEOW_LOADING_DOCK']
  FEISTY_MEOW_SCRIPTS = os.environ['FEISTY_MEOW_SCRIPTS']
  DEBUG_FEISTY_MEOW = os.environ['DEBUG_FEISTY_MEOW']

  if is_debugging():
    print("home is " + HOME)

  # make sure we know where to store the files we're creating.
  if len(FEISTY_MEOW_LOADING_DOCK) == 0:
    print("\
The FEISTY_MEOW_LOADING_DOCK variable is not defined.  This must point to\
the location where the generated scripts are stored.  You may still need to\
configure the feisty meow script system with something like:\
  bash /opt/feistymeow.org/feisty_meow/scripts/core/reconfigure_feisty_meow.sh\
Please see http://feistymeow.org for more details.")
    exit(1)

##############

  # replace any backslashes with forward thinking ones.
  FEISTY_MEOW_LOADING_DOCK = re.sub('\\', '/', FEISTY_MEOW_LOADING_DOCK)
  FEISTY_MEOW_SCRIPTS = re.sub('\\', '/', FEISTY_MEOW_SCRIPTS)
  FEISTY_MEOW_APEX = re.sub('\\', '/', FEISTY_MEOW_APEX)

##############

  # create our generated shells directory if it's not already there.
  if not os.path.isdir(FEISTY_MEOW_LOADING_DOCK):
    os.mkdirs(FEISTY_MEOW_LOADING_DOCK)

##############

  # set the executable bit for binaries for just this current user.
  if os.path.isdir(FEISTY_MEOW_BINARIES):
    for filename in os.listdir(FEISTY_MEOW_BINARIES):
      if is_debugging():
        print("adjusting permission on " + filename)
      # plop in executable permission for just the owner.
      add_permission(filename, stat.S_IXUSR)

##############

  # generate the first set of alias files that are defined in the core
  # and custom scripts directories.
  rebuild_script_aliases()

##############

  SCRIPT_ALIAS_FILENAME = FEISTY_MEOW_LOADING_DOCK + "/fmc_aliases_for_scripts.sh"

  # trash the old versions.
  os.unlink(SCRIPT_ALIAS_FILENAME)

  if is_debugging():
    print("writing " + SCRIPT_ALIAS_FILENAME)

##############

  # open the alias files to be created.
  try:
    with open(SCRIPT_ALIAS_FILENAME) as she:

      # find the list of files in the scripts directory.
      shell_files = [ find_files(recursive_find_directories(FEISTY_MEOW_SCRIPTS)),
        find_files(FEISTY_MEOW_LOADING_DOCK + "/custom/scripts"),
        find_files(recursive_find_directories(FEISTY_MEOW_LOADING_DOCK + "/custom/scripts")) ]

      # strip out the customization files, since they are added in only for particular users.
      print("before filtering list: " + shell_files)
      shell_files = list(filter(not_customize, shell_files))
      print("after filtering list: " + shell_files)

      print("found all these files in main script dirs:")
      print("  " + shell_files)

      # construct aliases for items in the scripts directory.
      for file in shell_files:
        # test to see what type of item we got.
        if (re.search('^\.$', file)                              # skip bare current directory.
            or re.search('^\.\.$', file)                         # skip bare parent directory.
            or re.search('^.svn$', file, re.IGNORECASE)          # skip bare svn state directory.
            or re.search('^.git$', file, re.IGNORECASE)          # skip bare git state directory.
            or re.search('/\/\.$/', file, re.IGNORECASE)         # skip relative current directory.
            or re.search('/\/\.\.$/', file, re.IGNORECASE)       # skip relative parent directory.
            or re.search('/\/\.svn$/', file, re.IGNORECASE)      # skip relative svn directory.
            or re.search('/\/\.git$/', file, re.IGNORECASE)      # skip relative git directory.
            ):
#hmmm: one could combine some of those above with a more complex regex if one wanted a project.
          # just skip this item; it's a special directory or a file we don't want to include.
          print("skipping name: " + file)
        else:
          to_add = make_alias(file, "");
          she.write(to_add)

  except:
    print("an error occurred while writing the shell aliases file " + SCRIPT_ALIAS_FILENAME)
    exit(1)

##############

  # prepare the finalizer chunk that is the last thing to load.
  SENTINEL_FILENAME = FEISTY_MEOW_LOADING_DOCK + "/fmc_ending_sentinel.sh"
  try:
    with open(SENTINEL_FILENAME) as sentinel:
      # write in our sentinel alias that says the alias loading process was handled.
      sentinel.write("define_yeti_alias CORE_ALIASES_LOADED=true")
  except:
    print("an error occurred while writing the sentinel file " + SENTINEL_FILENAME)
    exit(1)

##############

if __name__ == "__main__":
    main()

