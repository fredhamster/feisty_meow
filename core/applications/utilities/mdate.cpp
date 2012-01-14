/*****************************************************************************\
*                                                                             *
*  Name   : mdate                                                             *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Provides a more informative date command, providing milliseconds also.   *
*                                                                             *
*******************************************************************************
* Copyright (c) 2002-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/astring.h>
#include <application/hoople_main.h>
#include <loggers/console_logger.h>
#include <timely/time_stamp.h>
#include <structures/static_memory_gremlin.h>

#include <math.h>

using namespace application;
using namespace basis;
using namespace loggers;
using namespace structures;
using namespace timely;

class mdate_app : public application_shell
{
public:
  virtual int execute() {
    SETUP_CONSOLE_LOGGER;
    program_wide_logger::get().log(time_stamp::notarize(false), ALWAYS_PRINT);
    return 0;
  }
  DEFINE_CLASS_NAME("mdate_app");
};

HOOPLE_MAIN(mdate_app, )

