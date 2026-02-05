/*****************************************************************************\
*                                                                             *
*  Name   : test_path_configuration                                           *
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

#include <basis/guards.h>
#include <basis/istring.h>
#include <loggers/console_logger.h>
#include <opsystem/path_configuration.h>
#include <data_struct/static_memory_gremlin.h>

HOOPLE_STARTUP_CODE;

int main(int argc, char *argv[])
{
  console_logger out;

  istring jammed;
  for (int i = 0; i < argc; i++)
    jammed += istring(argv[i]) + " ";
  out.log(istring("command line=") + jammed);

  istring app_dir = path_configuration::application_directory();
  out.log(istring("app dir is: ") + app_dir);

  guards::alert_message("path_configuration:: works for those functions tested.");
  return 0;
}

