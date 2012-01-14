


//note: in progress.

/*****************************************************************************\
*                                                                             *
*  Name   : menu_base                                                         *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2003-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "menu_base.h"

#include <structures/string_array.h>
#include <structures/amorph.h>

class menu_common_amorph : public amorph<menu_common_base> {};

//////////////

menu_common_base::~menu_common_base() {}

//////////////

menu_item::menu_item(const string_array &trigs,
    const astring &text, const astring &description)
: _triggers(new string_array(trigs)),
  _text(new astring(text)),
  _description(new astring(description))
{}

menu_item::menu_item(const menu_item &to_copy)
: root_object(),
  menu_common_base(),
  _triggers(new string_array),
  _text(new astring),
  _description(new astring)
{ *this = to_copy; }

menu_item::~menu_item()
{
  WHACK(_text);
  WHACK(_description);
}

menu_item &menu_item::operator =(const menu_item &to_copy)
{
  if (this == &to_copy) return *this;
  *_triggers = *to_copy._triggers;
  *_text = *to_copy._text;
  *_description = *to_copy._description;
  return *this;
}

void menu_item::menu_activation(char formal(trigger)) {}

const string_array &menu_item::triggers() const { return *_triggers; }

const astring &menu_item::text() const { return *_text; }

const astring &menu_item::description() const { return *_description; }

//////////////

//call this when menu invoked.
///  virtual bool menu_item_activity() = 0;

menu_base::menu_base(const astring &title, const menu_item &parameters)
: _title(new astring(title)),
  _parameters(new menu_item(parameters)),
  _items(new menu_common_amorph),
  _menus(new menu_common_amorph)
{
}

menu_base::~menu_base()
{
  WHACK(_title);
  WHACK(_menus);
  WHACK(_items);
}

bool menu_base::validate(bool recursive)
{
if (recursive){}
//hmmm: implement this too....
return false;
}

astring menu_base::text_form() const
{
//hmmm: implement this too....
return "";
}

astring menu_base::recursive_text_form() const
{
//hmmm: implement this too....
return "";
}

int menu_base::items() const { return _items->elements(); }

void menu_base::add_item(menu_item *to_invoke)
{
  if (!to_invoke) return;
  *_items += to_invoke;
}

menu_item *menu_base::get_item(int index)
{
  bounds_return(index, 0, _items->elements(), NIL);
  return dynamic_cast<menu_item *>(_items->borrow(index));
}

bool menu_base::zap_item(int index)
{
  bounds_return(index, 0, _items->elements(), false);
  _items->zap(index, index);
  return true;
}

bool menu_base::enable_item(int index, bool enable)
{
  bounds_return(index, 0, _items->elements(), false);
  _items->borrow(index)->enable(enable);
  return true;
}

int menu_base::submenus() const { return _menus->elements(); }

void menu_base::add_submenu(menu_base *sub)
{
  if (!sub) return;
  _menus->append(sub);
}

menu_base *menu_base::get_submenu(int index)
{
  bounds_return(index, 0, _menus->elements(), NIL);
  return dynamic_cast<menu_base *>(_menus->borrow(index));
}

bool menu_base::zap_submenu(int index)
{
  bounds_return(index, 0, _menus->elements(), false);
  _menus->zap(index, index);
  return true;
}

bool menu_base::enable_submenu(int index, bool enable)
{
  bounds_return(index, 0, _menus->elements(), false);
  _menus->borrow(index)->enable(enable);
  return true;
}

menu_common_base *menu_base::evaluate_trigger(char trigger)
{
//hmmm: implement this too....
if (!trigger){}
return NIL;
}

void menu_base::activate()
{
//hmmm: implement this too....
}




