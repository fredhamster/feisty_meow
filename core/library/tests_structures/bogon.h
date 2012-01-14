#ifndef BOGON_CLASS
#define BOGON_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : bogon                                                             *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    A simple test object for amorphs.                                        *
*                                                                             *
*******************************************************************************
* Copyright (c) 1996-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#define DEBUG_ARRAY
#define DEBUG_AMORPH

#include <basis/astring.h>
#include <basis/definitions.h>
#include <structures/amorph.h>

class bogon
{
public:
  bogon(basis::abyte *to_copy);

  bogon(const bogon &to_copy);

  bogon &operator = (const bogon &to_copy);

  ~bogon();

  basis::abyte *held() const;

  int size() const;

private:
  basis::abyte *my_held;
};

#endif

