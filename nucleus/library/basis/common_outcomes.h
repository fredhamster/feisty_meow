#ifndef COMMON_OUTCOMES_CLASS
#define COMMON_OUTCOMES_CLASS

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

#include "outcome.h"

namespace basis {

//! the "common" class defines our common_outcomes.
class common
{
public:
  //! these outcomes are returned by classes that use the basic HOOPLE support.
  /*! More complicated classes will need to define their own outcome values to
  describe what occurred during the processing of requests. */
  enum outcomes {
    DEFINE_API_OUTCOME(OKAY, 0, "Everything is just fine"),
    DEFINE_API_OUTCOME(NOT_IMPLEMENTED, -1,
        "The invoked method is unimplemented"),
    DEFINE_API_OUTCOME(OUT_OF_RANGE, -2, "The value specified was out "
        "of bounds"),
    DEFINE_API_OUTCOME(NOT_FOUND, -3, "The item sought is not present"),
    DEFINE_API_OUTCOME(BAD_INPUT, -4, "Precondition failure--the parameters "
        "were inappropriate"),
    DEFINE_API_OUTCOME(BAD_TYPE, -5, "The objects are of incompatible types"),
    DEFINE_API_OUTCOME(IS_FULL, -6, "There is no room in the storage facility"),
    DEFINE_API_OUTCOME(IS_EMPTY, -7, "The container is empty currently"),
    DEFINE_API_OUTCOME(IS_NEW, -8, "The item is new"),
    DEFINE_API_OUTCOME(EXISTING, -9, "The item was already present"),
    DEFINE_API_OUTCOME(FAILURE, -10, "A failure has occurred"),
    DEFINE_API_OUTCOME(OUT_OF_MEMORY, -11, "There is not enough memory for the "
        "request according to the operating system"),
    DEFINE_API_OUTCOME(ACCESS_DENIED, -12, "The request was denied, possibly "
        "by the operating system"),
    DEFINE_API_OUTCOME(IN_USE, -13, "The object is already in exclusive use"),
    DEFINE_API_OUTCOME(UNINITIALIZED, -14, "The object has not been "
        "constructed properly"),
    DEFINE_API_OUTCOME(TIMED_OUT, -15, "The allowed time has now elapsed"),
    DEFINE_API_OUTCOME(GARBAGE, -16, "The request or response has been "
        "corrupted"),
    DEFINE_API_OUTCOME(NO_SPACE, -17, "A programmatic limit on storage space "
        "has been reached"),
    DEFINE_API_OUTCOME(DISALLOWED, -18, "The method denied the request"),
    DEFINE_API_OUTCOME(INCOMPLETE, -19, "The operation did not finish or the "
        "object is not completed"),
    DEFINE_API_OUTCOME(NO_HANDLER, -20, "The object type passed in was not "
        "understood by the invoked method"),
    DEFINE_API_OUTCOME(NONE_READY, -21, "There were no objects available"),
    DEFINE_API_OUTCOME(INVALID, -22, "That request or object was invalid"),
    DEFINE_API_OUTCOME(PARTIAL, -23, "The request was only partially finished"),
    DEFINE_API_OUTCOME(NO_LICENSE, -24, "The software license does not permit"
        "this request"),
    DEFINE_API_OUTCOME(UNEXPECTED, -25, "This item was unexpected, although "
        "not necessarily erroneous"),
    DEFINE_API_OUTCOME(ENCRYPTION_MISMATCH, -26, "The request failed due to a "
        "mismatch between encryption expected and encryption provided")
  };

  static const char *outcome_name(const outcome &to_name);
    //!< Returns a string representation of the outcome "to_name".
    /*!< If "to_name" is unknown, because it's not a member of the
    common::outcomes enum, then a string reporting that is returned. */

}; //class.

} //namespace.

#endif // outer guard.

