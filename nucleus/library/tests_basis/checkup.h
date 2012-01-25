#ifndef CHECKUP_GROUP
#define CHECKUP_GROUP

/*
*  Name   : checkup
*  Author : Chris Koeritz
*  Purpose:
*    Checks that certain critical properties are upheld by the runtime
*  environment.  This could be invoked at the beginning of a main program to
*  check these characteristics once before continuing execution.
**
* Copyright (c) 1990-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
*/

#include <basis/definitions.h>
#include <unit_test/unit_base.h>

namespace system_checkup {

  bool check_system_characteristics(unit_test::unit_base &testing);
    // used to verify that this compilation system possesses some desired
    // characteristics.  true is returned if everything checks out, and false
    // is returned if some assumption proves untrue.

}

#endif

