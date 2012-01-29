/*****************************************************************************\
*                                                                             *
*  Name   : configuration_list                                                *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2001-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "configlet.h"
#include "configuration_list.h"

#include <basis/astring.h>
#include <structures/amorph.h>

#include <typeinfo>

using namespace basis;
using namespace structures;

namespace configuration {

class cl_figlet_list : public amorph<configlet> {};

//////////////

configuration_list::configuration_list()
: _figs(new cl_figlet_list)
{
}

configuration_list::~configuration_list()
{
  WHACK(_figs);
}

void configuration_list::reset() { _figs->reset(); }

void configuration_list::add(const configlet &new_item)
{
  zap(new_item);
  _figs->append(new_item.duplicate());
}

const configlet *configuration_list::find(const configlet &to_find) const
{
  for (int i = 0; i < _figs->elements(); i++) {
    configlet &curr = *_figs->borrow(i);
    if ( (to_find.section() == curr.section())
        && (to_find.entry() == curr.entry())
        && (typeid(curr) == typeid(to_find)) ) {
      return &curr;
    }
  }
  return NIL;
}

bool configuration_list::zap(const configlet &dead_item)
{
  for (int i = 0; i < _figs->elements(); i++) {
    configlet &curr = *_figs->borrow(i);
    if ( (dead_item.section() == curr.section())
        && (dead_item.entry() == curr.entry()) ) {
      _figs->zap(i, i);
      return true;
    }
  }
  return false;
}

bool configuration_list::load(configurator &config)
{
  bool to_return = true;
  for (int i = 0; i < _figs->elements(); i++) {
    configlet &curr = *_figs->borrow(i);
    if (!curr.load(config)) to_return = false;  // any failure is bad.
  }
  return to_return;
}

bool configuration_list::store(configurator &config) const
{
  bool to_return = true;
  for (int i = 0; i < _figs->elements(); i++) {
    configlet &curr = *_figs->borrow(i);
    if (!curr.store(config)) to_return = false;  // any failure is bad.
  }
  return to_return;
}

} //namespace.

