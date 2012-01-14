#ifndef BUNDLE_LIST_CLASS
#define BUNDLE_LIST_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : bundle_list                                                       *
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

#include "synchronizable.h"

#include <structures/amorph.h>

namespace synchronic {

//! Provides a structure for managing a collection of synchronizables.

class bundle_list : public structures::amorph<synchronizable> {}; 

} //namespace.

#endif

