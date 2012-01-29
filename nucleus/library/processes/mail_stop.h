#ifndef MAIL_STOP_CLASS
#define MAIL_STOP_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : mail_stop                                                         *
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

#include "safe_callback.h"

#include <structures/unique_id.h>

namespace processes {

// forward:
class letter;

//! Base class for routes on which letters are automatically delivered.
/*!
  The letters will show up for a particular unique id at this mail stop.
  They are delivered by the object serving as the post office.
*/

class mail_stop : public safe_callback
{
public:

  // it is required that the derived mail_stop invoke the end_availability()
  // method in its destructor before it destroys any other objects.

  class items_to_deliver : public callback_data_block {
  public:
    items_to_deliver(const structures::unique_int &id, letter *package)
        : _id(id), _package(package) {}
    const structures::unique_int &_id;
    letter *_package;
  };

  virtual void delivery_for_you(const structures::unique_int &id, letter *package) = 0;
    //!< the derived object must provide this function.
    /*!< prompts the recipient with the "id" to accept delivery of a "package".
    the package can be modified as desired and MUST be recycled before
    returning from this function.
    IMPORTANT NOTE: the receiver MUST be thread-safe with respect to the
    objects that it uses to handle this delivery!  this is because mail is
    delivered on a thread other than the main program thread. */

protected:
  //! invoked by the safe callback machinery.
  /*! this is implemented in this class and merely re-routes the call to the
  more specific delivery_for_you() method. */
  virtual void real_callback(callback_data_block &data) {
    items_to_deliver *bits = dynamic_cast<items_to_deliver *>(&data);
    if (!bits) return;  // bad type.
    delivery_for_you(bits->_id, bits->_package);
  }

};

} //namespace.

#endif

