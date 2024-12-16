#!/usr/bin/env bash

# updates the moodle install, assuming all paths are at the default.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

####
# some constants that we know are not really.
moodle_parent=/var/www/html
  # parent directory is one level up from where moodle lives.
moodle_dir=moodle
  # this variable is just the directory name for moodle itself, not a path.
moodle_release=moodle-3.9
  # the name of the release we're expecting to download and install
download_url="https://download.moodle.org/download.php/direct/stable39/${moodle_release}.tgz"
  # where we can get the latest version of moodle for our chosen releases.
  # this URL could change over time, but it's the best link i could find for now.
####

# everything below should be version invariant.

moodle_path="$moodle_parent/$moodle_dir"
  # composing the parent plus directory name should get us to moodle.

if [ ! -d "$moodle_path" ]; then
  echo "There was no moodle installation found at: $moodle_path"
  exit 1
fi

# where we unpack our temporary stuff.
temp_install="$(mktemp -d /tmp/update_moodle.XXXXXX)"
echo "Using temporary directory for installation: $temp_install"
if [ ! -d "$temp_install" ]; then
  echo The temporary installation directory at: $temp_install could not be created.
  exit 1
fi

# quit the running moodle instance.
echo "Stopping Apache httpd before moodle update."
systemctl stop httpd
exit_on_error stopping httpd process before moodle upgrade.

# jump into our new work area.
pushd "$temp_install"

# get the latest moodle version.
echo "Downloading latest version of '$moodle_release' now."
wget "$download_url"
exit_on_error downloading latest version of moodle.

# use the feisty meow unpack script to extract the moodle data.
echo "Installing the latest $moodle_release now."
unpack "${moodle_release}.tgz"
exit_on_error unpacking latest version of moodle.

# rename the old moodle directory to a unique name in the same area.
old_moodle_path="$moodle_parent/moodle-$(basename $temp_install)"
mv "$moodle_parent/$moodle_dir" "$old_moodle_path"
exit_on_error renaming old version of moodle.

# move the new stuff into place.
mv "arch_${moodle_release}/${moodle_dir}" "$moodle_parent"/ &>/dev/null
exit_on_error moving new version of moodle into place.

# grab our important configuration files and put them back in the new directory.
cp "$old_moodle_path/config.php" "$moodle_path"
exit_on_error copying existing moodle configuration file: config.php

# legalesey warnings about what we didn't do...
echo -e "\
====
NOTE: This script does not copy any plugins or themes.  If you are using\n\
updated or specialized additions to moodle, please copy them from here:\n\
  $old_moodle_path\n\
into the new install at:\n\
  $moodle_path\n\
====\n\
"

# restart the running moodle instance.
echo "Starting Apache httpd after moodle update."
systemctl start httpd
exit_on_error starting httpd process after moodle upgrade.

# back out of our directory and clean up the temporary junk, if any.
popd
rm -rf "$temp_install"

# sunshine and roses!  we are through the gauntlet.

