#!/bin/bash

# snags my two email accounts' mail filters into a nice storage location.

source "$FEISTY_MEOW_SCRIPTS/core/launch_feisty_meow.sh"

cp $HOME/.thunderbird/oqv30zg4.default/ImapMail/zooty.koeritz.com/msgFilterRules.dat $CLOUD_BASE/magic_cabinet/mail_filters/zooty_serene_hamstertronic_$(date_stringer).filters
test_or_fail "copying feistymeow.org filters"

cp $HOME/.thunderbird/oqv30zg4.default/ImapMail/mail.eservices.virginia.edu/msgFilterRules.dat $CLOUD_BASE/magic_cabinet/mail_filters/uva_email_$(date_stringer).filters 
test_or_fail "copying UVa filters"


