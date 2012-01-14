/*****************************************************************************\
*                                                                             *
*  Name   : common_outcomes                                                   *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1991-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "common_outcomes.h"

namespace basis {

const char *common::outcome_name(const outcome &to_name)
{
  switch (to_name.value()) {
    case OKAY: return "OKAY";
    case NOT_IMPLEMENTED: return "NOT_IMPLEMENTED";
    case OUT_OF_RANGE: return "OUT_OF_RANGE";
    case NOT_FOUND: return "NOT_FOUND";
    case BAD_INPUT: return "BAD_INPUT";
    case BAD_TYPE: return "BAD_TYPE";
    case IS_FULL: return "IS_FULL";
    case IS_EMPTY: return "IS_EMPTY";
    case IS_NEW: return "IS_NEW";
    case EXISTING: return "EXISTING";
    case FAILURE: return "FAILURE";
    case OUT_OF_MEMORY: return "OUT_OF_MEMORY";
    case ACCESS_DENIED: return "ACCESS_DENIED";
    case IN_USE: return "IN_USE";
    case UNINITIALIZED: return "UNINITIALIZED";
    case TIMED_OUT: return "TIMED_OUT";
    case GARBAGE: return "GARBAGE";
    case NO_SPACE: return "NO_SPACE";
    case DISALLOWED: return "DISALLOWED";
    case INCOMPLETE: return "INCOMPLETE";
    case NO_HANDLER: return "NO_HANDLER";
    case NONE_READY: return "NONE_READY";
    case INVALID: return "INVALID";
    case PARTIAL: return "PARTIAL";
    case NO_LICENSE: return "NO_LICENSE";
    case UNEXPECTED: return "UNEXPECTED";
    case ENCRYPTION_MISMATCH: return "ENCRYPTION_MISMATCH";
    default: return "UNKNOWN_OUTCOME";
  }
}

} // namespace.

