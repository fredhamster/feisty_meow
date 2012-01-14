//note: in progress.

#ifndef MENU_BASE_CLASS
#define MENU_BASE_CLASS

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



#include <basis/contracts.h>

// forward.
class menu_common_amorph;

//////////////

//! a common base class for referring to menu_items or menus polymorphically.

class menu_common_base : public virtual root_object
{
public:
  virtual ~menu_common_base();

  bool enabled() const { return _enabled; }
  void enable(bool enable = true) { _enabled = enable; }

private:
  bool _enabled;  //!< is this object enabled?
};

//////////////

//! A base class for the active items that can be stored inside a menu.

class menu_item
: public menu_common_base
{
public:
  menu_item(const string_array &triggers, const astring &text,
          const astring &description);
    //!< constructs a menu item that shows the "text" and "description".
    /*!< the "triggers" is a list of characters that are used to activate this
    menu item.  these correspond to hot keys that are often underlined. */

  menu_item(const menu_item &to_copy);

  virtual ~menu_item();

  menu_item &operator =(const menu_item &to_copy);

  DEFINE_CLASS_NAME("menu_item");

  const string_array &triggers() const;
  const astring &text() const;
  const astring &description() const;

  virtual void menu_activation(char trigger);
    //!< invoked when the user chooses the menu item in question.
    /*!< the "trigger" holds the value that they actually chose. */

private:
  string_array *_triggers;  //!< the trigger values for this menu item.
  astring *_text;  //!< the text shown for this item.
  astring *_description;  //!< the full description of the item.
};

//////////////

//! A base class for a menu-driven interface model.
/*!
  The base just provides functions for manipulating menu items and submenus.
*/

class menu_base : public menu_common_base
{
public:
  menu_base(const astring &title, const menu_item &parameters);
    //<! constructs a menu where the "title" is the name for this menu.
    /*!< the "parameters" record any activation triggers and descriptions for
    this menu; these are mainly used when this is a sub-menu. */

  virtual ~menu_base();

  DEFINE_CLASS_NAME("menu_base");

  bool validate(bool recursive = true);
    //!< checks that all of the menu_items

  astring text_form() const;
    //!< returns a string version of all the information here.
    /*!< this just prints out this menu without recursing to sub-menus. */

  astring recursive_text_form() const;
    //!< does a text_form on all menus and submenus rooted here.

  menu_common_base *evaluate_trigger(char trigger);
    //!< returns the item or menu associated with the "trigger" value.
    /*!< use of dynamic cast enables one to tell what has been returned.  NIL
    is returned if there is nothing that answers to that trigger value.
    note that this does not invoke any activation functions. */

  virtual void activate();
    //!< runs the menu structure by requesting input from the user.
    /*!< this assumes that they will type a key and hit enter afterwards.  the
    menus are displayed using the program-wide logger.  no feedback is provided
    since the menu_items will be activated automatically.  when this method
    returns, it is assumed that the user has chosen a menu item.  overriding
    this method allows a different type of menu to provide a totally different
    method for interacting with the user. */

  // note about the methods here: the menu_base takes over responsibility for
  // pointers it is handed.  do not delete the items after adding them or
  // get an item and then delete it.
  // also, the indices for menu items are separate from the indices for the
  // sub-menus.

  // menu item manipulators.  the indexes here range from 0 to items() - 1.
  int items() const;  //!< returns the number of menu items stored.
  void add_item(menu_item *to_invoke);
    //!< adds a new menu_item onto this menu.
  menu_item *get_item(int index);
    //!< gets the item at position "index".  NIL is returned if out of range.
  bool zap_item(int index);
    //!< removes the item at "index" if possible.
  bool enable_item(int index, bool enable = true);
    //!< enables or disabled the item at "index".

  // submenu manipulation support.  these range from 0 to submenus() - 1.
  int submenus() const;  //!< number of submenus total.
  void add_submenu(menu_base *sub);  //!< add a new submenu into "sub".
  menu_base *get_submenu(int index);
    //!< returns the submenu stored at "index".
  bool zap_submenu(int index);
    //!< removes the submenu at the "index".
  bool enable_submenu(int index, bool enable = true);
    //!< enables or disables the submenu at the "index".

private:
  astring *_title;  //!< the name for this menu.
  menu_item *_parameters;  //!< information regarding this menu.
  menu_common_amorph *_items;  //!< the list of items in this menu.
  menu_common_amorph *_menus;  //!< the list of sub-menus accessible from here.
};

#endif

