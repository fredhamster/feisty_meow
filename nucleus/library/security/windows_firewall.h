#ifndef WINDOWS_FIREWALL_CLASS
#define WINDOWS_FIREWALL_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : windows firewall wrapper                                          *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2009-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/



#include <basis/astring.h>

class windows_firewall
{
public:
  // adds a firewall hole for an executable called "program_name" (which should
  // be the full path) using a rule called "exception_name".  the description
  // for the firewall exception should be in "hole_description" (and it's only
  // used on vista or server 2008 or later).
  // in this and the other methods, a zero return indicates success.  any other
  // return indicates a failure.
  static int poke_firewall_hole(const astring &program_name,
      const astring &exception_name, const astring &hole_description);

  // this version will open an exception for a port rather than a program.
  static int poke_firewall_hole(int port_number,
      const astring &exception_name, const astring &hole_description,
      const astring &protocol);

  // removes a previously poked firewall hole for an application.
  static int remove_firewall_hole(const astring &program_name,
      const astring &exception_name);

  // removes a previously poked exception for a port.
  static int remove_firewall_hole(int port_number,
      const astring &exception_name, const astring &protocol);
};

#endif //outer guard

