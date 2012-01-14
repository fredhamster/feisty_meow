/*****************************************************************************\
*                                                                             *
*  Name   : mailbox                                                           *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1998-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "letter.h"
#include "mailbox.h"

#include <basis/astring.h>
#include <basis/functions.h>
#include <basis/guards.h>
#include <basis/mutex.h>
#include <loggers/critical_events.h>
#include <structures/amorph.h>
#include <structures/int_hash.h>
#include <structures/unique_id.h>
#include <textual/parser_bits.h>
#include <textual/string_manipulation.h>

using namespace basis;
using namespace loggers;
using namespace structures;
using namespace textual;

namespace processes {

const int MAILBOX_BITS = 9;
  // we allow N bits in our table size, which means the table will have 2^N
  // elements.  careful with that increase...

class mail_cabinet
{
public:
  amorph<letter> _waiting;

  mail_cabinet() : _waiting(0) {}

  ~mail_cabinet() { _waiting.reset(); }

  mail_cabinet(mail_cabinet &formal(to_copy)) {
    non_continuable_error("mail_cabinet", "copy constructor", "should never be called");
  }

  mail_cabinet &operator =(mail_cabinet &formal(to_copy)) {
    non_continuable_error("mail_cabinet", "assignment operator",
        "should never be called");
    return *this;
  }
};

//////////////

class mailbox_bank : public int_hash<mail_cabinet>
{
public:
  mailbox_bank() : int_hash<mail_cabinet> (MAILBOX_BITS) {}
  ~mailbox_bank() { reset(); }

  void get_ids(int_set &to_fill);
    // returns the list of identifiers for people with mailboxes.

  void add_cabinet(const unique_int &id);
    // creates a new mail receptacle for the "id".

  bool zap_cabinet(const unique_int &id);
    // removes the cabinet for "id".

  void add_item(const unique_int &id, letter *to_add);
    // stuffs an item "to_add" in for "id".

  bool get(const unique_int &id, letter * &to_receive);
    // retrieves the next waiting package for "id" into "to_receive".

  void clean_up();
    // gets rid of any cabinets without any packages.
};

void mailbox_bank::clean_up()
{
  int_set ids;
  get_ids(ids);
  for (int i = 0; i < ids.elements(); i++) {
    mail_cabinet *entry = find(ids[i]);
    // if the cabinet has zero elements, we zap it.
    if (!entry->_waiting.elements()) zap(ids[i]);
  }
}

void mailbox_bank::get_ids(int_set &to_fill) { to_fill = ids(); }

void mailbox_bank::add_cabinet(const unique_int &id)
{
  if (find(id.raw_id())) return;  // already exists.
  mail_cabinet *to_add = new mail_cabinet;
  add(id.raw_id(), to_add);
}

bool mailbox_bank::zap_cabinet(const unique_int &id)
{
  if (!find(id.raw_id())) return false;  // doesn't exist.
  return zap(id.raw_id());
}

void mailbox_bank::add_item(const unique_int &id, letter *to_add)
{
  mail_cabinet *found = find(id.raw_id());
  if (!found) {
    add_cabinet(id);
    found = find(id.raw_id());
    // there should never be a failure case that would prevent the new cabinet
    // from being added (besides overall memory failure).
    if (!found) {
//complain
      return;
    }
  }
  found->_waiting.append(to_add);
}

bool mailbox_bank::get(const unique_int &id, letter * &to_receive)
{
  mail_cabinet *found = find(id.raw_id());
  if (!found) return false;  // no cabinet, much less any mail.

  if (!found->_waiting.elements()) return false;  // no mail waiting.
  for (int i = 0; i < found->_waiting.elements(); i++) {
    // check if its time is ripe...
    if (!found->_waiting.borrow(i)->ready_to_send()) continue;
    // get the waiting mail and remove its old slot.
    to_receive = found->_waiting.acquire(i);
    found->_waiting.zap(i, i);
    return true;
  }
  return false;
}

//////////////

mailbox::mailbox()
: _transaction_lock(new mutex),
  _packages(new mailbox_bank)
{
}

mailbox::~mailbox()
{
  WHACK(_packages);
  WHACK(_transaction_lock);
}

void mailbox::get_ids(int_set &to_fill)
{
  auto_synchronizer l(*_transaction_lock);
  _packages->get_ids(to_fill);
}

void mailbox::drop_off(const unique_int &id, letter *package)
{
  auto_synchronizer l(*_transaction_lock);
  _packages->add_item(id, package);
}

void mailbox::clean_up()
{
  auto_synchronizer l(*_transaction_lock);
  _packages->clean_up();
}

int mailbox::waiting(const unique_int &id) const
{
  auto_synchronizer l(*_transaction_lock);
  mail_cabinet *found = _packages->find(id.raw_id());
  int to_return = 0;  // if no cabinet, this is the proper count.
  // if there is a cabinet, then get the size.
  if (found)
    to_return = found->_waiting.elements();
  return to_return;
}

bool mailbox::pick_up(const unique_int &id, letter * &package)
{
  package = NIL;
  auto_synchronizer l(*_transaction_lock);
  return _packages->get(id, package);
}

bool mailbox::close_out(const unique_int &id)
{
  auto_synchronizer l(*_transaction_lock);
  bool ret = _packages->zap_cabinet(id);
  return ret;
}

void mailbox::show(astring &to_fill)
{
  auto_synchronizer l(*_transaction_lock);
  int_set ids;
  _packages->get_ids(ids);
  for (int i = 0; i < ids.elements(); i++) {
    mail_cabinet &mc = *_packages->find(ids[i]);
    to_fill += astring(astring::SPRINTF, "cabinet %d:", ids[i])
        + parser_bits::platform_eol_to_chars();
    for (int j = 0; j < mc._waiting.elements(); j++) {
      letter &l = *mc._waiting.borrow(j);
      astring text;
      l.text_form(text);
      to_fill += string_manipulation::indentation(4)
          + astring(astring::SPRINTF, "%4ld: ", j + 1)
          + text + parser_bits::platform_eol_to_chars();
    }
  }
}

void mailbox::limit_boxes(int max_letters)
{
  auto_synchronizer l(*_transaction_lock);
  int_set ids;
  _packages->get_ids(ids);
  for (int i = 0; i < ids.elements(); i++) {
    mail_cabinet &mc = *_packages->find(ids[i]);
    if (mc._waiting.elements() > max_letters) {
      // this one needs cleaning.
      mc._waiting.zap(max_letters, mc._waiting.elements() - 1);
    }
  }
}

void mailbox::apply(apply_function *to_apply, void *data_link)
{
  auto_synchronizer l(*_transaction_lock);
  int_set ids;
  _packages->get_ids(ids);
  for (int i = 0; i < ids.elements(); i++) {
    mail_cabinet &mc = *_packages->find(ids[i]);
    for (int j = 0; j < mc._waiting.elements(); j++) {
      letter &l = *mc._waiting.borrow(j);
      outcome ret = to_apply(l, ids[i], data_link);
      if ( (ret == APPLY_WHACK) || (ret == APPLY_WHACK_STOP) ) {
        // they wanted this node removed.
        mc._waiting.zap(j, j);
        j--;  // skip back before missing guy so we don't omit anyone.
        if (ret == APPLY_WHACK_STOP)
          break;  // they wanted to be done with it also.
      } else if (ret == APPLY_STOP) {
        break;  // we hit the exit condition.
      }
    }
  }
}

} //namespace.


