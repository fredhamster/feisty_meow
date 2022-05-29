#ifndef SYSTEM_HELPER_GROUP
#define SYSTEM_HELPER_GROUP

//////////////
// Name   : system helper header
// Author : Chris Koeritz
// Rights : Copyright (c) 2012-$now By Author
//////////////
// This file is free software; you can modify/redistribute it under the terms
// of the GNU General Public License. [ http://www.gnu.org/licenses/gpl.html ]
// Feel free to send updates to: [ fred@gruntose.com ]
//////////////

//! Isolates a few system dependencies for feisty meow runtime environment.

//////////////

/*
  default location of virtual root directory for Unix.
  the contents here are replaced at runtime on windoze if cygwin is
  available.
*/
#define FEISTY_MEOW_VIRTUAL_UNIX_ROOT "c:/cygwin"

// hmmm: support msys too at some point?  very worthy as well.

//////////////

#endif

