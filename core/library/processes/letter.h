#ifndef LETTER_CLASS
#define LETTER_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : letter                                                            *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1998-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <timely/time_stamp.h>

namespace processes {

//! A virtual base class for pieces of "mail".  Used by the mailbox object.

class letter : public virtual basis::text_formable
{
public:
  letter(int type = 0, int start_after = 0);
    //!< constructs a letter with the "type" and initial pause of "start_after".
    /*!< a "type" for this letter must be specified, even if it is not intended
    to be used.  some letter managers may rely on this number identifying
    different kinds of mail.  the types should be unique within one mailbox.
    a "type" of zero indicates an invalid letter.  if the "start_after" is
    non-zero, then it indicates that this letter should not be sent until
    that many milliseconds have elapsed. */

  letter(const letter &to_copy);
    //!< copy constructor for base parts.

  virtual ~letter();
    //!< derived classes should also implement this.
    /*!< a virtual destructor should be implemented by each derived class
    to take care of class specific cleaning.  note that the destructor should
    NEVER attempt to use the mailbox system that it was stored in (or any
    other mailbox system for that matter).  this is necessary for prohibiting
    deadlock conditions, but it's not that much of a restriction usually. */

  letter &operator =(const letter &to_copy);
    //!< assignment operator for base object.

  virtual void text_form(basis::base_string &fill) const = 0;
    //!< derived letters must print a status blurb describing their contents.

  int type() const { return _type; }
    //!< returns the type of letter held here.

  bool ready_to_send();
    //!< returns true if this letter is ready to 

  void set_ready_time(int start_after);
    //!< resets the time when this letter is ready to be sent.
    /*!< the letter will now not be allowed to send until "start_after"
    milliseconds from now.  once the letter is added to a mailbox, it may
    be too late to adjust this duration. */

private:
  int _type;  //!< the kind of mail this item represents.
  timely::time_stamp *_ready_time;  //!< time when this letter will be ready to send.
};

} //namespace.

#endif

