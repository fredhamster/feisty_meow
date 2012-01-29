#ifndef PROCESS_ENTRY_CLASS
#define PROCESS_ENTRY_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : process_entry                                                     *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2000-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/array.h>
#include <basis/astring.h>
#include <basis/contracts.h>
#include <basis/definitions.h>

namespace processes {

//! Encapsulates information about OS processes.

class process_entry : public virtual basis::text_formable
{
public:
  basis::un_int _process_id;  //!< the OS identifier of this process.
  basis::un_int _references;  //!< the number of references to (users of) this process.
  basis::un_int _threads;  //!< the number of threads in use by this process.
  basis::un_int _parent_process_id;  //!< the process id of the owning process.
  basis::un_short _module16;  //!< non-zero if this process is a 16-bit application.

  process_entry();
  process_entry(const process_entry &to_copy);
  ~process_entry();

  DEFINE_CLASS_NAME("process_entry");

  process_entry &operator =(const process_entry &to_copy);

  const basis::astring &path() const;
  void path(const basis::astring &new_path);

  basis::astring text_form() const;
    //!< returns a descriptive string for the information here.

  void text_form(basis::base_string &fill) const;   //!< base class requirement.

private:
  basis::astring *_process_path;
};

//////////////

//! a handy class that implements an array of process entries.

class process_entry_array : public basis::array<process_entry> {};

//////////////

} //namespace.

#endif

