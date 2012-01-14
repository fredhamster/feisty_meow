#ifndef LOGGING_FILTERS_GROUP
#define LOGGING_FILTERS_GROUP

//////////////
// Name   : logging_filters
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

#include <basis/contracts.h>

namespace loggers {

  enum logging_filters {
    // synonyms for filters that are important enough to always show.
    FILT_FATAL = basis::ALWAYS_PRINT,
    FILT_ERROR = basis::ALWAYS_PRINT,
    // the first really selectable filters follow...  one might notice a
    // small similarity to levels available with log4j.
    DEFINE_FILTER(FILT_WARNING, 1, "Important or unusual condition occurred in the runtime."),
    DEFINE_FILTER(FILT_INFO, 2, "Information from normal runtime activities."),
    DEFINE_FILTER(FILT_DEBUG, 3, "Noisy debugging information from objects."),
    // occasionally useful filters for locally defined categories.
    DEFINE_FILTER(FILT_UNUSUAL, 4, "Unusual but not necessarily erroneous events."),
    DEFINE_FILTER(FILT_NETWORK, 5, "Network activity and events."),
    DEFINE_FILTER(FILT_DATABASE, 6, "Data storage activities or warnings.")
  };

} //namespace.

#endif

