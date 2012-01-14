/*****************************************************************************\
*                                                                             *
*  Name   : cromp_security                                                    *
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

#include "cromp_security.h"
#include "cromp_server.h"

#include <basis/functions.h>
#include <octopus/entity_defs.h>
#include <sockets/internet_address.h>
#include <sockets/machine_uid.h>
#include <sockets/tcpip_stack.h>

using namespace basis;
using namespace octopi;
using namespace sockets;
//using namespace basis;

namespace cromp {

//#define DEBUG_CROMP_SECURITY
  // uncomment if you want the noisier version.

#undef LOG
#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)

cromp_security::cromp_security()
: _stack(new tcpip_stack)
{
}

cromp_security::~cromp_security()
{
  WHACK(_stack);
}

bool cromp_security::add_entity(const octopus_entity &client,
          const byte_array &verification)
{
#ifdef DEBUG_CROMP_SECURITY
  FUNCDEF("add_entity");
  LOG(astring("adding ") + client.mangled_form());
#endif
  return simple_entity_registry::add_entity(client, verification);
}

} //namespace.

