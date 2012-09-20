//////////////
// Name   : internet_address
// Author : Chris Koeritz
//////////////
// Copyright (c) 1995-$now By Author.  This program is free software; you can
// redistribute it and/or modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation:
//     http://www.gnu.org/licenses/gpl.html
// or under the terms of the GNU Library license:
//     http://www.gnu.org/licenses/lgpl.html
// at your preference.  Those licenses describe your legal rights to this
// software, and no other rights or warranties apply.
// Please send updates for this code to: fred@gruntose.com -- Thanks, fred.
//////////////

#include "internet_address.h"
#include "machine_uid.h"

#include <basis/byte_array.h>
#include <basis/functions.h>
#include <basis/astring.h>
#include <basis/mutex.h>
#include <configuration/configurator.h>
#include <configuration/variable_tokenizer.h>
#include <loggers/program_wide_logger.h>
#include <structures/static_memory_gremlin.h>
#include <structures/string_table.h>
#include <textual/parser_bits.h>

#include <stdio.h>
#include <string.h>

using namespace basis;
using namespace configuration;
using namespace loggers;
using namespace structures;
using namespace textual;

namespace sockets {

//#define DEBUG_ADDRESS
  // uncomment if you want a debugging version of address.

//////////////

#undef LOG
#define LOG(to_print) CLASS_EMERGENCY_LOG(program_wide_logger::get(), to_print)

//////////////

/*
//hmmm: consider moving the functions for storage out to a helper file.

internet_address internet_address::load(configurator &config,
    const astring &section, const astring &name, const internet_address &def)
{
  astring token_list;
  if (!config.get(section, name, token_list)) {
    // no entry stored means no address.  set the address to the default.
    config.put(section, name, def.tokenize());
    return def;
  }
  internet_address to_return;
  // get the rest of the work done by detokenize.
  if (to_return.detokenize(token_list)) return to_return;
  // didn't work, dang it.  Note that we don't reject it here and store the
  // default.  that's because this indicates an error in formatting of the 
  // address, and we want the user to have a chance to correct that and try
  // again.
  return internet_address();
}

bool internet_address::store(configurator &config, const astring &section,
    const astring &name, const internet_address &to_store)
{
  astring text = to_store.tokenize();
  return config.put(section, name, text);
}
*/

//////////////

// provides an easy way to cast to the proper type and provide a non-pointer
// to work with.
#define CAST_UP(type) \
  const type *temp = dynamic_cast<const type *>(&compare_in); \
  if (!temp) return false;  /* shouldn't happen but it means bad things. */ \
  const type &to_compare = *temp;

//////////////

internet_address::internet_address() { fill(byte_array(), "", 0); }

internet_address::internet_address(const byte_array &ip,
    const astring &host, int port_in)
{ fill(ip, host, port_in); }

//hmmm: for ipv6, we will need a new object, called ipv6_address perhaps.
//      it will copy everything here but will have a longer address array.
//      we will need a new object for ipv6_machine_uid also.

machine_uid internet_address::convert() const
{ return internet_machine_uid(hostname, byte_array(ADDRESS_SIZE, ip_address)); }

bool internet_address::ip_appropriate_number(const astring &to_check, int indy,
    astring &accum)
{
  FUNCDEF("ip_appropriate_number");
  accum.reset();
  for (int i = indy; (i < indy + 3) && (i < to_check.length()); i++) {
    // make sure it looks like a good number.
    if (!parser_bits::is_numeric(to_check[i]) || (to_check[i] == '-') ) {
      // this one doesn't look good right here, but if we already got a digit,
      // we're okay.
      if (i == indy) return false;  // hadn't gotten any digits at all yet.
      else break;  // got one, so let's check our progress below.
    }
    accum += to_check[i];  // snag the current digit.
  }
  if (!accum.length()) return false;  // how would that happen?
  int convert = accum.convert(-1);
  return (convert >= 0) && (convert <= 255);
}

// needs to see 1-3 numbers, period, 1-3 numbers, period etc...
// for a total of three periods and 4 number sets.
bool internet_address::has_ip_address(const astring &to_check,
    astring &ip_found)
{
  FUNCDEF("has_ip_address");
  int nums_seen = 0;
  ip_found.reset();
  for (int i = 0; i < to_check.length(); i++) {
    bool hosed = false;
    astring num_found;
//    if (!!ip_found) LOG(astring("current ip found=") + ip_found);
    if (!ip_appropriate_number(to_check, i, num_found)) {
//      LOG(a_sprintf("no ip approp %d", i));
      hosed = true;
    } else {
      // we're seeing the number we want here.
//      LOG(astring("num found = ") + num_found);
      nums_seen++;
      if (nums_seen >= 4) {
        ip_found += num_found;  // get our last part.
        return true;  // hey, that's a match.
      }
      // look for a period now.
      int period_indy = to_check.find('.', i);
      if (negative(period_indy) || (period_indy > i + 3) ) hosed = true;
      else {
        for (int x = i; x < period_indy; x++) {
          if (!parser_bits::is_numeric(to_check[x]) || (to_check[x] == '-')) {
//            LOG(a_sprintf("hosed by bad char at %d -> %c", x, to_check[x]));
            hosed = true;  // wrong character seen in between.
          }
        }
        if (!hosed) {
          ip_found += to_check.substring(i, period_indy);
          i = period_indy;  // skip to where our next number should be.
        }
      }
    }
    if (hosed) {
      nums_seen = 0;
      ip_found.reset();
    }
  }
  return false;
}

const abyte localhosts_bytes[] = { 127, 0, 0, 1 };
SAFE_STATIC_CONST(byte_array, internet_address::localhost,
    (ADDRESS_SIZE, localhosts_bytes))

bool internet_address::is_localhost() const
{
  // check whether the host is "local" or "localhost".
  astring host = hostname;
  host.to_lower();
  if ( (host.equal_to("local")) || (host.equal_to("localhost")) )
    return true;

  // check whether the address is local even if the hostname wasn't.
  for (int i = 0; i < ADDRESS_SIZE; i++) {
    if (ip_address[i] != localhost().get(i))
      return false;
  }

  return true;  // the address matched.
}

astring internet_address::normalize_host() const
{
  // try the hostname first.
  astring remote = hostname;
  if (remote.t()) return remote;
  // there was no host we can use; try the IP address instead.
  byte_array ip_form(ADDRESS_SIZE, ip_address);
  remote = ip_address_text_form(ip_form);
  return remote;  // they get whatever we had as the address.
}

int internet_address::packed_size() const
{
  return sizeof(port) +
      + sizeof(int) + ADDRESS_SIZE
      + sizeof(int) + MAXIMUM_HOSTNAME_LENGTH;
}

void internet_address::pack(byte_array &packed_form) const
{
  attach(packed_form, port);
  packed_form += byte_array(ADDRESS_SIZE, ip_address);
  packed_form += byte_array(MAXIMUM_HOSTNAME_LENGTH, (abyte *)hostname);
}

bool internet_address::unpack(byte_array &packed_form)
{
  // check for minimum expected length.
  if (packed_form.length() < int(sizeof(port)) + ADDRESS_SIZE
      + MAXIMUM_HOSTNAME_LENGTH)
    return false;
  if (!detach(packed_form, port)) return false;
  packed_form.stuff(ADDRESS_SIZE, ip_address);
  packed_form.zap(0, ADDRESS_SIZE - 1);
  packed_form.stuff(MAXIMUM_HOSTNAME_LENGTH, (abyte *)hostname);
  packed_form.zap(0, MAXIMUM_HOSTNAME_LENGTH - 1);
  return true;
}

void internet_address::fill(const byte_array &ip, const astring &host,
    int port_in)
{
  port = port_in;
  int mini = minimum(int(ADDRESS_SIZE), ip.length());
  for (int i = 0; i < mini; i++) ip_address[i] = ip[i];
  for (int j = mini; j < ADDRESS_SIZE; j++) ip_address[j] = 0;
  hostname[0] = '\0';
  host.stuff(hostname, MAXIMUM_HOSTNAME_LENGTH - 1);
}

base_address *internet_address::create_copy() const
{
  return new internet_address(byte_array(ADDRESS_SIZE, ip_address),
      hostname, port);
}

astring internet_address::text_form() const
{
  astring to_print("[");
  if (astring(hostname).t()) {
    to_print += "host=";
    to_print += hostname;
    to_print += ", ";
  }
/////  bool print_ip = false;
/////  for (int i = 0; i < ADDRESS_SIZE; i++) if (ip_address[i]) print_ip = true;
/////  if (print_ip) {
    to_print += "ip_addr=";
    for (int i = 0; i < ADDRESS_SIZE; i++) {
      to_print += a_sprintf("%d", int(ip_address[i]));
      if (i != ADDRESS_SIZE - 1) to_print += ".";
    }
    to_print += ", ";
/////  }
  to_print += a_sprintf("port=%u]", port);
  return to_print;
}

bool internet_address::is_nil_address(const address_array &ip_address)
{
  for (int i = 0; i < ADDRESS_SIZE; i++) if (ip_address[i]) return false;
  return true;
}

const abyte nil_address_bytes[] = { 0, 0, 0, 0 };
SAFE_STATIC_CONST(byte_array, internet_address::nil_address,
    (ADDRESS_SIZE, nil_address_bytes))

bool internet_address::is_nil_address() const
{ return is_nil_address(ip_address); }

bool internet_address::same_host(const base_address &compare_in) const
{
  CAST_UP(internet_address);

  // they can't be the same if one is a valid address and the other's not, but
  // they are the same if both addresses are empty.
  // even so, they're not the same if either's name was non-empty but they're
  // both nil.
  if ( (is_nil_address(ip_address) && is_nil_address(to_compare.ip_address))
      && !astring(hostname) && !astring(to_compare.hostname) )
    return true;
  if ( (is_nil_address(ip_address) && is_nil_address(to_compare.ip_address))
      && (astring(hostname).iequals(to_compare.hostname)) )
    return true;
  if (is_nil_address(ip_address)) return false;
  if (is_nil_address(to_compare.ip_address)) return false;

  // check that the bytes don't differ for the IP address.
  for (int i = 0; i < ADDRESS_SIZE; i++)
    if (ip_address[i] != to_compare.ip_address[i])
      return false;  // we have a loser.

  // they could still be different addresses if the hostnames differ...

  if (astring(hostname).t() && astring(to_compare.hostname).t()) {
    // name comparison.
    if (astring(hostname).lower() != astring(to_compare.hostname).lower())
      return false;
  }

  return true;
    // all bytes and host were identical, so it's the same address.
}

bool internet_address::same_port(const base_address &compare_in) const
{
  CAST_UP(internet_address);
  return port == to_compare.port;
}

bool internet_address::shareable(const base_address &) const
{ return true; }

bool internet_address::detokenize(const astring &info)
{
#ifdef DEBUG_ADDRESS
  FUNCDEF("detokenize");
#endif
  LOADER_ENTRY;
  // either IP address or host must be specified.

  FIND("address", addr);
#ifdef DEBUG_ADDRESS
  LOG(astring("info is ") + info + astring('.'));
  LOG(astring("addr is ") + addr);
#endif
  byte_array ip_found;
  if (addr.t()) {
    // this bit rips off successive bytes from the string until the internet
    // address is filled out.  ignores any erroneous strings.
    for (int i = 0; i < ADDRESS_SIZE; i++) {
#ifdef DEBUG_ADDRESS
      LOG(astring("ip curr: ") + addr);
#endif
      int current_byte = addr.convert(int(0));
      ip_found += abyte(current_byte);
#ifdef DEBUG_ADDRESS
      LOG(a_sprintf("%d: %02x ", i, current_byte));
#endif
      int indy = addr.find('.');
      addr.zap(0, indy);  // no error checking, but whatever.
    }
  }

  FIND("host", host);
  GRAB("port", port_t);  // this one's definitely needed.
  int port = port_t.convert(0);
  fill(ip_found, host, port);
#ifdef DEBUG_ADDRESS
  LOG(astring("tcp/ip address found::: ") + text_form());
#endif
  LOADER_EXIT;
  return true;
}

astring internet_address::tokenize() const
{
#ifdef DEBUG_ADDRESS
  FUNCDEF("tokenize");
#endif
  STORER_ENTRY;
#ifdef DEBUG_ADDRESS
  LOG(a_sprintf("host eval is %d for %s", astring(hostname).t(), hostname));
#endif
  if (astring(hostname).t()) ADD("host", hostname);
  bool print_ip = false;
  for (int i = 0; i < ADDRESS_SIZE; i++) {
    if (ip_address[i]) print_ip = true;
  }
  if (print_ip) {
    astring ip_addr;
    for (int i = 0; i < ADDRESS_SIZE; i++)
      ip_addr += a_sprintf("%d", int(ip_address[i])) + astring(".");
    ip_addr.zap(ip_addr.end(), ip_addr.end());  // remove last period. 
    ADD("address", ip_addr);
  }
  ADD("port", a_sprintf("%d", int(port)));
#ifdef DEBUG_ADDRESS
  LOG(astring("your toke's currently ") + fred.text_form());
#endif
  DUMP_EXIT;
}

bool internet_address::appropriate_for_ip(const astring &to_check)
{
  for (int i = 0; i < to_check.length(); i++) {
    char curr = to_check[i];
    if (curr == '.') continue;
    if ( (curr >= '0') && (curr <= '9') ) continue;
    // an unsavory character was seen, so fail out.
    return false;
  }
  return true;
}

// by Gary Hardley.
// fixes by CAK 6/26/2002.
// and more by CAK in november 2010.
bool internet_address::is_valid_internet_address(const astring &to_check,
    byte_array &ip_form, bool &all_zeros)
{
  astring tmpstr = to_check;  // temporary copy of the given address string.
  all_zeros = true;  // default to true until proven otherwise.
  ip_form.reset();  // empty the address.

  // if it's got anything besides dots and numbers, bail out.
  if (!appropriate_for_ip(to_check)) return false;
  // catch a case the below logic does not--where the string starts or
  // ends with a dot.
  if ( (to_check[0] == '.') || (to_check[to_check.end()] == '.') )
    return false;
  // catch another case that was missed before, where there are multiple dots
  // in a row but enough numbers to give 4 components.
  if (to_check.contains("..")) return false;

  // get the first part of the address.
  char *p = strtok(tmpstr.s(), ".");

  int index = 0;
  while (p && (index < ADDRESS_SIZE)) {
    int nTemp = astring(p).convert(-1);

    // is this part of the string a valid number between 0 & 255?
    if ( (nTemp < 0) || (nTemp > 255) ) return false;  // no.

    // yes, assign the number to the next address byte.
    ip_form += (abyte)nTemp;

    // get the next part of the string
    p = strtok(NIL, ".");
  }

  // if p is non-null, there was extra stuff at the end, so return false.
  if (p) return false;

  for (int i = 0; i < ip_form.length(); i++)
    if (ip_form[i]) { all_zeros = false; break; }

  return ip_form.length() == ADDRESS_SIZE;
}

bool internet_address::valid_address(const astring &to_check)
{
  byte_array addr;
  bool all_zeros;
  return is_valid_internet_address(to_check, addr, all_zeros);
}

astring internet_address::ip_address_text_form(const byte_array &ip_address)
{
  return a_sprintf("%d.%d.%d.%d", int(ip_address[0]),
      int(ip_address[1]), int(ip_address[2]), int(ip_address[3]));
}

} //namespace.


