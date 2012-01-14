/*****************************************************************************\
*                                                                             *
*  Name   : common bundler definitions                                        *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2007-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "common_bundle.h"

#include <basis/byte_array.h>
#include <basis/contracts.h>
#include <filesystem/byte_filer.h>
#include <filesystem/file_time.h>
#include <structures/object_packers.h>
#include <structures/set.h>

using namespace basis;
using namespace filesystem;
using namespace structures;

manifest_chunk::~manifest_chunk()
{}

int manifest_chunk::packed_filetime_size()
{
  static file_time hidden_comparison_object;
  return hidden_comparison_object.packed_size();
}

void manifest_chunk::pack(byte_array &target) const
{
  structures::obscure_attach(target, _size);
  _payload.pack(target);
  structures::attach(target, _flags);
  _parms.pack(target);
  _keywords.pack(target);
  target += c_filetime;
}

bool manifest_chunk::unpack(byte_array &source)
{
  if (!structures::obscure_detach(source, _size)) return false;
  if (!_payload.unpack(source)) return false;
  if (!structures::detach(source, _flags)) return false;
  if (!_parms.unpack(source)) return false;
  if (!_keywords.unpack(source)) return false;
  if (source.length() < 8) return false;
  c_filetime = source.subarray(0, 7);
  source.zap(0, 7);
  return true;
}

bool manifest_chunk::read_an_int(byte_filer &bundle, un_int &found)
{
//  FUNCDEF("read_an_int");
  byte_array temp;
  if (bundle.read(temp, sizeof(int)) != sizeof(int)) return false;
  if (!structures::detach(temp, found)) return false;
  return true;
}

bool manifest_chunk::read_an_obscured_int(byte_filer &bundle, un_int &found)
{
//  FUNCDEF("read_an_obscured_int");
  byte_array temp;
  if (bundle.read(temp, 2 * sizeof(int)) != 2 * sizeof(int)) return false;
  if (!structures::obscure_detach(temp, found)) return false;
  return true;
}

bool manifest_chunk::read_a_filetime(byte_filer &bundle, byte_array &found)
{
//  FUNCDEF("read_a_filetime");
  byte_array temp;
  // the trick below only works because we know we have a constant sized packed version
  // for the file time.
  if (bundle.read(temp, packed_filetime_size()) != packed_filetime_size()) return false;
  found = temp;
  return true;
}

astring manifest_chunk::read_a_string(byte_filer &bundle)
{
//  FUNCDEF("read_a_string");
  astring found;
  byte_array temp;
  // read in the zero-terminated character string.
  while (!bundle.eof()) {
    // read a single byte out of the file.
    if (bundle.read(temp, 1) <= 0)
      break;
    if (temp[0]) {
      // add the byte to the string we're accumulating.
      found += temp[0];
    } else {
      // this string is done now.
      break;
    }
  }
  return found;
}

bool manifest_chunk::read_manifest(byte_filer &bundle, manifest_chunk &curr)
{
  curr._size = 0;
  bool worked = read_an_obscured_int(bundle, curr._size);
  if (!worked)
    return false;
  byte_array temp;
  curr._payload = read_a_string(bundle);
  if (!curr._payload) return false;  
  worked = read_an_int(bundle, curr._flags);
  if (!worked)
    return false;
  curr._parms = read_a_string(bundle);
    // it's valid for the _parms to be empty.
//if (curr._parms.length()) { printf("parms len=%d are: \"%s\"\n", curr._parms.length(), curr._parms.s()); }
  // now get the keywords list, if it exists.
  un_int key_elems = 0;  // number of keywords.
  worked = read_an_obscured_int(bundle, key_elems);  // get number of elements.
  if (!worked)
    return false;
  curr._keywords.reset();
  for (int i = 0; i < (int)key_elems; i++) {
    astring found = read_a_string(bundle);
    if (!found) return false;  // not allowed an empty keyword.
    curr._keywords += found;
  }
  worked = read_a_filetime(bundle, curr.c_filetime);
  return worked;
}

