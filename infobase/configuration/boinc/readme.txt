

the files in etc/sysconfig and etc/init.d should be copied into the
real /etc directory.

the file /etc/init.d/boinc starts the boinc client on system startup
and the init process will ensure that it gets restarted if it crashes.

the file /etc/sysconfig/boinc has configuration information for the
boinc process.  the user and path will need to be changed to make them
appropriate for your installation.




