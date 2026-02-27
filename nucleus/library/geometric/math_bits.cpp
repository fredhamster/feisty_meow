/*****************************************************************************\
*                                                                             *
*  Name   : mathematical operations group                                     *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1996-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "math_bits.h"

#include <basis/astring.h>

using namespace basis;

namespace geometric {

astring crop_numeric(const astring &input)
{
  astring to_return(input);
  for (int i = 0; i < to_return.length(); i++)
    if ( ( (to_return[i] >= '0') && (to_return[i] <= '9') )
         || (to_return[i] == '.')
         || (to_return[i] == '+') || (to_return[i] == '-')
         || (to_return[i] == 'E') || (to_return[i] == 'e') ) {
      to_return.zap(i, i);  // remove non-useful character.
      i--;  // move backwards in loop.
    } else break;
  return to_return;
}

astring crop_non_numeric(const astring &input)
{
  astring to_return(input);
  for (int i = 0; i < to_return.length(); i++)
    if ( ! ((to_return[i] >= '0') && (to_return[i] <= '9'))
         && (to_return[i] != '.')
         && (to_return[i] != '+') && (to_return[i] != '-')
         && (to_return[i] != 'E') && (to_return[i] != 'e') ) {
      to_return.zap(i, i);  // remove non-useful character.
      i--;  // move backwards in loop.
    } else break;
  return to_return;
}

}

