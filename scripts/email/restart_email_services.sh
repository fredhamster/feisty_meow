/etc/init.d/courier-imap stop
/etc/init.d/courier-imap-ssl stop
/etc/init.d/courier-authdaemon stop
/etc/init.d/mailman stop
/etc/init.d/exim4 stop
/etc/init.d/spamassassin stop

/etc/init.d/spamassassin start
/etc/init.d/exim4 start
/etc/init.d/mailman start
/etc/init.d/courier-authdaemon start
/etc/init.d/courier-imap start
/etc/init.d/courier-imap-ssl start
