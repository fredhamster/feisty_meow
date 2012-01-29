#ifndef OS_EVENT_CLASS
#define OS_EVENT_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : OS_event                                                          *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1995-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "letter.h"

#include <basis/astring.h>
#include <basis/contracts.h>

namespace processes {

// forward.
class post_office;

//! Models an OS-level event so we can represent activities occurring there.

class OS_event : public letter, public virtual basis::text_formable
{
public:
  basis::un_int _message;
  basis::un_int _parm1;
  basis::un_int _parm2;

  DEFINE_CLASS_NAME("OS_event");

  OS_event(int event_type, basis::un_int message, basis::un_int parm1, basis::un_int parm2)
  : letter(event_type), _message(message), _parm1(parm1), _parm2(parm2) {}

  virtual void text_form(basis::base_string &fill) const {
    fill.assign(text_form());
  }
  basis::astring text_form() const {
    return basis::a_sprintf("os_event: msg=%d parm1=%d parm2=%d", _message, _parm1, _parm2);
  }
};

} //namespace.

#endif

