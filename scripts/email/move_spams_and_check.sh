#!/usr/bin/env bash

# retrieves the system's spam pile from sa-exim's spool folder and
# moves it to the user's home directory.  sudo access is required
# for the file moving operations.
# after the spam is all snagged, it is scanned for any untoward presence
# of non-spam folks using the user's valid email list.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

# the storage area that the spam catcher tool puts the suspected spam into.
SPAM_SPOOL="/var/spool/sa-exim"
# a temporary directory where we'll move all the spam for analysis.
SPAM_HOLD="$HOME/spamcrud"
# the white list needs to be a file of good email addresses that will
# probably never send spam.  it should be formatted one address to a line.
EMAIL_WHITE_LIST="$CLOUD_BASE/magic_cabinet/lists/email_addresses.txt"
# we'll save a report of the spam checks in the file below.
REPORT_FILE="$HOME/spam_check_report_$(date_stringer).txt"

if [ ! -d "$SPAM_HOLD" ]; then
  mkdir "$SPAM_HOLD"
fi
echo "The operation to move the spam files requires sudo privileges..."
sudo find "$SPAM_SPOOL" -type f -exec mv {} "$SPAM_HOLD" ';'
if [ $? -ne 0 ]; then
  echo "The spam moving operation failed, which probably means we failed to get sudo access"
  exit 3
fi
echo "Setting the directory back to user's ownership..."
sudo chown -R $USER "$SPAM_HOLD" 
sudo chgrp -R $USER "$SPAM_HOLD" 
echo "Checking for false-positive spams..." | tee "$REPORT_FILE"
bash "$FEISTY_MEOW_SCRIPTS/email/scan_spam.sh" "$SPAM_HOLD" "$EMAIL_WHITE_LIST" 2>&1 \
  | tee -a "$REPORT_FILE"
echo "Done checking for false-positive spams." | tee -a "$REPORT_FILE"


