//////////////
// Name   : guards
// Author : Chris Koeritz
//////////////
// Copyright (c) 1989-$now By Author.  This program is free software; you can
// redistribute it and/or modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation:
//     http://www.gnu.org/licenses/gpl.html
// or under the terms of the GNU Library license:
//     http://www.gnu.org/licenses/lgpl.html
// at your preference.  Those licenses describe your legal rights to this
// software, and no other rights or warranties apply.
// Please send updates for this code to: fred@gruntose.com -- Thanks, fred.
//////////////

#include "astring.h"
#include "guards.h"

namespace basis {

void format_error(const base_string &class_name, const base_string &func_name,
    const base_string &error_message, base_string &to_fill)
{
  astring to_return = class_name;
  to_return += "::";
  to_return += func_name;
  to_return += ": ";
  to_return += error_message;
  to_fill = to_return;
}

void throw_error(const base_string &class_name, const base_string &func_name,
    const base_string &error_message)
{
  astring to_throw;
  format_error(class_name, func_name, error_message, to_throw);
  throw to_throw;
}

void throw_error(const astring &class_name, const astring &func_name,
    const astring &error_message)
{
  throw_error((base_string &)class_name, (base_string &)func_name, (base_string &)error_message);
}

} //namespace.

