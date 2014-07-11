/*****************************************************************************\
*                                                                             *
*  Name   : windows firewall wrapper                                          *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2009-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "windows_firewall.h"


#include <basis/functions.h>

using namespace portable;

// so far this is a good assumption about where to find netsh.
astring netsh_app() { return env_string("WINDIR") + "/System32/netsh.exe"; }

int windows_firewall::poke_firewall_hole(const astring &program_name,
    const astring &exception_name, const astring &hole_description)
{
  astring cmdline;
#ifdef __WIN32__
  known_operating_systems kind = determine_OS();
  if ( (kind == WIN_SRV2K8) || (kind == WIN_VISTA) ) {
    // newer style firewall with advfirewall.
//::MessageBox(0, "poke app srv2k8", "yodel", MB_OK);
    cmdline = a_sprintf("-c advfirewall firewall add rule name=\"%s\" dir=in "
        "action=allow program=\"%s\" enable=yes profile=any "
        "description=\"%s\"", exception_name.s(), program_name.s(),
        hole_description.s());
  } else {
    // older xp style firewall (if that).
//::MessageBox(0, "poke app xp", "yodel", MB_OK);
    cmdline = a_sprintf("-c firewall add allowedprogram program=\"%s\" "
        "name=\"%s\" mode=enable scope=all profile=all", program_name.s(),
        exception_name.s());
  }

  basis::u_int kid_id;
  basis::u_int to_return = launch_process::run(netsh_app(), cmdline,
      portable::AWAIT_APP_EXIT | portable::HIDE_APP_WINDOW
      | portable::SHELL_EXECUTE, kid_id);
  return to_return;
#else
  if (!program_name || !exception_name || !hole_description) {}  // no problem.
  return 1;  // failure on this platform.
#endif
}

int windows_firewall::remove_firewall_hole(const astring &program_name,
    const astring &exception_name)
{
#ifdef __WIN32__
  astring cmdline;

  known_operating_systems kind = determine_OS();
  if ( (kind == WIN_SRV2K8) || (kind == WIN_VISTA) ) {
//::MessageBox(0, "removing app srv2k8", "yodel", MB_OK);
    // newer style firewall with advfirewall.
    cmdline = a_sprintf("-c advfirewall firewall delete rule name=\"%s\" ",
        exception_name.s());
  } else {
//::MessageBox(0, "removing app xp", "yodel", MB_OK);
    // older xp style firewall (if that).
    cmdline = a_sprintf("-c firewall delete allowedprogram program=\"%s\" "
        "profile=all", program_name.s());
  }

  basis::u_int kid_id;
  basis::u_int to_return = launch_process::run(netsh_app(), cmdline,
      portable::AWAIT_APP_EXIT | portable::HIDE_APP_WINDOW
      | portable::SHELL_EXECUTE, kid_id);
  return to_return;
#else
  if (!program_name || !exception_name) {}  // no problem.
  return 1;  // failure on this platform.
#endif
}

int windows_firewall::poke_firewall_hole(int port_number,
    const astring &exception_name, const astring &hole_description,
    const astring &protocol)
{
#ifdef __WIN32__
  astring cmdline;

  known_operating_systems kind = determine_OS();
  if ( (kind == WIN_SRV2K8) || (kind == WIN_VISTA) ) {
    // newer style firewall with advfirewall.
//::MessageBox(0, "poke port srv2k8", "yodel", MB_OK);
    cmdline = a_sprintf("-c advfirewall firewall add rule name=\"%s\" dir=in "
        "action=allow protocol=\"%s\" enable=yes profile=any "
        "description=\"%s\" localport=%d",
        exception_name.s(), protocol.s(), hole_description.s(), port_number);
  } else {
    // older xp style firewall (if that).
//::MessageBox(0, "poke port xp", "yodel", MB_OK);
    cmdline = a_sprintf("-c firewall add portopening port=%d "
        "name=\"%s\" protocol=%s mode=enable scope=all profile=all",
        port_number, exception_name.s(), protocol.s());
  }

  basis::u_int kid_id;
  basis::u_int to_return = launch_process::run(netsh_app(), cmdline,
      portable::AWAIT_APP_EXIT | portable::HIDE_APP_WINDOW
      | portable::SHELL_EXECUTE, kid_id);
  return to_return;
#else
  if (!port_number || !exception_name || !protocol || !hole_description) {}  // no problem.
  return 1;  // failure on this platform.
#endif
}

int windows_firewall::remove_firewall_hole(int port_number,
    const astring &exception_name, const astring &protocol)
{
#ifdef __WIN32__
  astring cmdline;

  known_operating_systems kind = determine_OS();
  if ( (kind == WIN_SRV2K8) || (kind == WIN_VISTA) ) {
//::MessageBox(0, "removing port srv2k8", "yodel", MB_OK);
    // newer style firewall with advfirewall.
    cmdline = a_sprintf("-c advfirewall firewall delete rule name=\"%s\" "
        "localport=%d protocol=%s", exception_name.s(),
        port_number, protocol.s());
  } else {
//::MessageBox(0, "removing port xp", "yodel", MB_OK);
    // older xp style firewall (if that).
    cmdline = a_sprintf("-c firewall delete portopening protocol=%s "
        "port=%d profile=all", protocol.s(), port_number);
  }

  basis::u_int kid_id;
  basis::u_int to_return = launch_process::run(netsh_app(), cmdline,
      portable::AWAIT_APP_EXIT | portable::HIDE_APP_WINDOW
      | portable::SHELL_EXECUTE, kid_id);
  return to_return;
#else
  if (!port_number || !exception_name || !protocol) {}  // no problem.
  return 1;  // failure on this platform.
#endif
}

