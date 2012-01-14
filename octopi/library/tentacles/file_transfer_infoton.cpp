/*****************************************************************************\
*                                                                             *
*  Name   : file_transfer_infoton                                             *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2005-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "file_transfer_infoton.h"

#include <basis/mutex.h>
#include <structures/string_array.h>
#include <structures/static_memory_gremlin.h>

using namespace basis;
using namespace filesystem;
using namespace structures;

namespace octopi {

file_transfer_infoton::file_transfer_infoton()
: infoton(file_transfer_classifier()),
  _success(common::OKAY),
  _request(false),
  _command(TREE_COMPARISON),
  _src_root(),
  _dest_root(),
  _packed_data()
{}

file_transfer_infoton::file_transfer_infoton(const outcome &success,
    bool request, commands command,
    const astring &source, const astring &destination,
    const byte_array &packed_data)
: infoton(file_transfer_classifier()),
  _success(success),
  _request(request),
  _command(abyte(command)),
  _src_root(source),
  _dest_root(destination),
  _packed_data(packed_data)
{}

file_transfer_infoton::~file_transfer_infoton()
{
}

void file_transfer_infoton::text_form(basis::base_string &fill) const
{
  fill.assign(astring(class_name()) + ": unimplemented text_form.");
}

const char *file_transfer_constant = "#ftran";

SAFE_STATIC_CONST(string_array,
    file_transfer_infoton::file_transfer_classifier,
    (1, &file_transfer_constant))

int file_transfer_infoton::packed_size() const
{
  return sizeof(int)
      + sizeof(abyte) * 2
      + _src_root.length() + 1
      + _dest_root.length() + 1
      + _packed_data.length() + sizeof(int);
}

void file_transfer_infoton::package_tree_info(const directory_tree &tree,
    const string_array &includes)
{
  _packed_data.reset();
  tree.pack(_packed_data);
  includes.pack(_packed_data);
}

void file_transfer_infoton::pack(byte_array &packed_form) const
{
  attach(packed_form, _success.value());
  attach(packed_form, abyte(_request));
  attach(packed_form, _command);
  _src_root.pack(packed_form);
  _dest_root.pack(packed_form);
  attach(packed_form, _packed_data);
}

bool file_transfer_infoton::unpack(byte_array &packed_form)
{
  int temp_o;
  if (!detach(packed_form, temp_o)) return false;
  _success = temp_o;
  abyte temp;
  if (!detach(packed_form, temp)) return false;
  _request = temp;
  if (!detach(packed_form, _command)) return false;
  if (!_src_root.unpack(packed_form)) return false;
  if (!_dest_root.unpack(packed_form)) return false;
  if (!detach(packed_form, _packed_data)) return false;
  return true;
}

} //namespace.

