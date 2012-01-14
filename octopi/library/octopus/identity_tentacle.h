#ifndef IDENTITY_TENTACLE_CLASS
#define IDENTITY_TENTACLE_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : identity_tentacle                                                 *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2002-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "identity_infoton.h"
#include "tentacle_helper.h"

#include <basis/byte_array.h>
#include <basis/outcome.h>
#include <structures/string_array.h>

namespace octopi {

//! Supports an early step in using octopus services: getting an identity.

class identity_tentacle
: public tentacle_helper<identity_infoton>
{
public:
  identity_tentacle(octopus &parent);
    //!< the "parent" will provide the real identity services.

  virtual ~identity_tentacle();

  DEFINE_CLASS_NAME("identity_tentacle");

  virtual basis::outcome reconstitute(const structures::string_array &classifier,
          basis::byte_array &packed_form, infoton * &reformed);
    //!< reinflates an infoton given that we know the type in "classifier".
    /*!< recreates a "reformed" infoton from the "classifier" and packed
    infoton data in "packed_form".  this will only succeed if the classifier's
    first name is understood here. */

  virtual basis::outcome consume(infoton &to_chow, const octopus_request_id &item_id,
          basis::byte_array &transformed);
    //!< chews on the "to_chow" infoton to perform the requested action.
    /*!< if it's an identity_infoton, then a new identity is provided.
    otherwise, the identity is given rudimentary checks for validity and
    the infoton is passed along. */

private:
  octopus &_parent;  //!< provides the real identification service.
};

} //namespace.

#endif

