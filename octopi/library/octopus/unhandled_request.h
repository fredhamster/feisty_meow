#ifndef UNHANDLED_REQUEST_CLASS
#define UNHANDLED_REQUEST_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : unhandled_request                                                 *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2004-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "entity_defs.h"
#include "infoton.h"
#include "tentacle_helper.h"

#include <structures/string_array.h>

namespace octopi {

// forward.
class octopus_request_id;

//! Informs the caller that a request type was unknown to the server octopus.
/*!
  The accompanying octopus_request_id specifies the particular request that
  failed.  The classifier of the original request is also included for
  reference.  This allows the client to get an immediate response from the
  server when we have no idea what they are asking for, rather than the client
  needing to timeout on the failed request.  Note: this is a heavy-weight
  header that should not be included in other headers.
*/

class unhandled_request : public infoton
{
public:
  // these members are informational so they're exposed out in public.
  octopus_request_id _original_id;  //!< the failed request's identifier.
  structures::string_array _original_classifier;  //!< the original name of the request.
  basis::outcome _reason;  //!< the reason why this request was provided.

  unhandled_request(const octopus_request_id &id = octopus_request_id(),
          const structures::string_array &original_classifier = structures::string_array(),
          const basis::outcome &reason = basis::common::NO_HANDLER);

  DEFINE_CLASS_NAME("unhandled_request");

  static structures::string_array the_classifier();
    //!< the classifier for unknown infotons makes unhandled requests unique.
    /*!< __Unhandled__ is now a reserved word for classifiers.  That's the
    classifier for all unhandled_request objects as returned by
    the_classifier(). */

  virtual void text_form(basis::base_string &fill) const;
  virtual void pack(basis::byte_array &packed_form) const;
  virtual bool unpack(basis::byte_array &packed_form);
  virtual clonable *clone() const;
  virtual int packed_size() const;
};

//////////////

class unhandled_request_tentacle
: public tentacle_helper<unhandled_request>
{
public:
  unhandled_request_tentacle(bool backgrounded = false)
  : tentacle_helper<unhandled_request>(unhandled_request::the_classifier(),
        backgrounded) {}
};

} //namespace.

#endif

