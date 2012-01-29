/*****************************************************************************\
*                                                                             *
*  Name   : short_path                                                        *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    This program converts a pathname to its 8.3 name.  Only for windows.     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2007-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <application/windoze_helper.h>
#include <basis/astring.h>
#include <structures/static_memory_gremlin.h>

#include <stdio.h>
#include <string.h>
#ifdef __WIN32__
  #include <windows.h>
#endif

using namespace basis;
using namespace structures;

///HOOPLE_STARTUP_CODE;

int main(int argc, char *argv[])
{
  astring shorty('\0', 2048);
  if (argc < 2) {
    printf("This program needs a path to convert to its short form.\n");
    return 23;
  }
#ifdef __WIN32__
  GetShortPathNameA(argv[1], shorty.s(), 2045);
#else
  strcpy(shorty.s(), argv[1]);
#endif
  shorty.replace_all('\\', '/');
  printf("%s", shorty.s());
  return 0;
}

#ifdef __BUILD_STATIC_APPLICATION__
  // static dependencies found by buildor_gen_deps.sh:
  #include <basis/astring.cpp>
  #include <basis/common_outcomes.cpp>
  #include <basis/mutex.cpp>
  #include <structures/static_memory_gremlin.cpp>
#endif // __BUILD_STATIC_APPLICATION__

