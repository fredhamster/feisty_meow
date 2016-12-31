#ifndef TENTACLE_HELPER_CLASS
#define TENTACLE_HELPER_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : tentacle_helper                                                   *
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

#include "infoton.h"
#include "tentacle.h"

#include <basis/byte_array.h>
#include <basis/outcome.h>
#include <structures/string_array.h>

/*! @file tentacle_helper.h
  @brief Automates some common tasks for tentacle implementations.
  This template provides some default implementations for the methods that
  derived tentacles must implement.  This works best when the infotons being
  exchanged are derived from a common base; the base class would be used as
  the instantiation type here.  For tentacles used in network communication,
  the client side could use tentacle_helper<infoton_type> without adding any
  functionality.  The server side must override the consume() and expunge()
  methods in order to implement processing for the infotons sent by
  its clients.
*/

namespace octopi {

//!< reconstituter should work for most infotons to restore flattened infotons.
/*!< the infotons that can be used here just need valid default constructor
and unpack methods.  the "junk" parameter is needed to allow the template to
be disambiguated on some compilers--it is unused and should just be NULL_POINTER. */
template <class contents>
basis::outcome reconstituter(const structures::string_array &classifier,
    basis::byte_array &packed_form,
    infoton * &reformed, contents *formal(junk))
{
  contents *inf = new contents;
  if (!inf->unpack(packed_form)) {
    WHACK(inf);
    return tentacle::GARBAGE;
  }
  reformed = inf;
  reformed->set_classifier(classifier);
  return tentacle::OKAY;
}

//////////////

//! provides prefab implementations for parts of the tentacle object.
/*! tentacle_helper provides the base functionality of reconstitution that
should support most of the simple needs of a user of an octopus.  if the
octopus will actually implement the request processing (instead of just
unpacking returned responses), the consume method needs to be filled in
appropriately so that it implements the derived tentacle's purpose. */
template <class contents>
class tentacle_helper : public tentacle
{
public:
  tentacle_helper(const structures::string_array &classifier, bool backgrounded,
      int motivational_rate = tentacle::DEFAULT_RATE)
      : tentacle(classifier, backgrounded, motivational_rate) {}

  virtual ~tentacle_helper() {}
    //!< force a virtual destructor.

  //! this is a simple enough action that it is totally automated.
  virtual basis::outcome reconstitute(const structures::string_array &classifier,
      basis::byte_array &packed_form, infoton * &reformed) {
    return reconstituter(classifier, packed_form, reformed,
        (contents *)NULL_POINTER);
  }

  //! consume is not really provided here.  remember to implement for servers!
  /*! consume will generally need to be implemented by a "real" tentacle that
  is based on the tentacle_helper.  in the context of network communications,
  the server side will generally have a real tentacle that implements consume()
  while the client side will just have the tentacle_helper version of one,
  which does nothing. */
  virtual basis::outcome consume(infoton &formal(to_chow),
          const octopus_request_id &formal(item_id), basis::byte_array &transformed)
      { transformed.reset(); return NO_HANDLER; }

  virtual void expunge(const octopus_entity &formal(to_remove)) {}
    //!< no general actions for expunge; they are all class-specific.
};

} //namespace.

#endif

