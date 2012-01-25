#ifndef STANDARD_LOG_BASE_CLASS
#define STANDARD_LOG_BASE_CLASS

//////////////
// Name   : standard_log_base
// Author : Chris Koeritz
//////////////
// Copyright (c) 2010-$now By Author.  This program is free software; you can
// redistribute it and/or modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation:
//     http://www.gnu.org/licenses/gpl.html
// or under the terms of the GNU Library license:
//     http://www.gnu.org/licenses/lgpl.html
// at your preference.  Those licenses describe your legal rights to this
// software, and no other rights or warranties apply.
// Please send updates for this code to: fred@gruntose.com -- Thanks, fred.
//////////////

#include "eol_aware.h"
#include "filter_set.h"

#include <basis/contracts.h>

namespace loggers {

//! A base class for a very usable logger with a filter_set and eol awareness.
/*!
  We add this derived class of base_logger to incorporate some useful functionality
  for managing filters without polluting the base class.  This class allows the logging
  functionality to not deal with a lot of add-ins or chicanery.
*/

class standard_log_base
: public virtual basis::base_logger,
  public virtual basis::nameable,
  public virtual filter_set,
  public virtual eol_aware
{
};

} //namespace.

#endif

