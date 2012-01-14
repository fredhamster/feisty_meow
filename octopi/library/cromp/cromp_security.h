#ifndef CROMP_SECURITY_CLASS
#define CROMP_SECURITY_CLASS

/***
*                                                                             *
*  Name   : cromp_security
*  Author : Chris Koeritz
* Copyright (c) 2002-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/contracts.h>
#include <tentacles/simple_entity_registry.h>
#include <sockets/tcpip_stack.h>

namespace cromp {

//! Implements the client registry in a cromp-appropriate manner.
/*!
  The identity issue request is vetted against the known connection endpoint for a client.
*/

class cromp_security : public octopi::simple_entity_registry
{
public:
  cromp_security();
  virtual ~cromp_security();

  DEFINE_CLASS_NAME("cromp_security");

  virtual bool add_entity(const octopi::octopus_entity &client,
        const basis::byte_array &verification);

  // stronger security models can be implemented by overriding add_entity().
  // this object merely verifies that we have seen the entity get issued
  // by the current server.

private:
  sockets::tcpip_stack *_stack;  // enables access to tcpip functionality.
};

} //namespace.

#endif

