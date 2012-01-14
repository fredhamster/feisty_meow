#ifndef MAILBOX_CLASS
#define MAILBOX_CLASS

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

#include <basis/mutex.h>
#include <structures/set.h>
#include <structures/unique_id.h>

namespace processes {

class letter;
class mailbox_bank;

//! Implements a thread safe "mail" delivery system.
/*!
  Senders can drop packages off into the mailbox and the receivers can get
  those packages back out of it.  The base class for all mail items is also
  provided in this library (letter.h).  The name of this object is slightly
  misleading; this object is really more of a post office.  Each unique id
  has its own mailbox slot for receiving mail.
*/

class mailbox : public virtual basis::root_object
{
public:
  mailbox();
  virtual ~mailbox();

  void drop_off(const structures::unique_int &id, letter *package);
    //!< drops a "package" in the mailbox for "id".
    /*!< note that when you send a package to someone, you give up all
    authority and control over that package.  hopefully the end recipient
    will eventually pick it up and then delete it.  if the package is never
    received, then this object will delete it. */

  bool pick_up(const structures::unique_int &id, letter * &package);
    //!< returns true if the mailbox for "id" had a "package" to be delivered.
    /*!< don't forget to check multiple times on a true outcome, since there
    could be more than one package waiting.  false is returned when no more
    mail is waiting.  be careful; "package" could be a bomb.  dynamic casts
    seem appropriate as a method for ensuring that you get the type of
    object you expect.  note that once the invoker receives a package, it
    is their responsibility to carefully manage it and then delete the
    package after handling.  not deleting the "package" pointer is grounds
    for memory leaks. */

  int waiting(const structures::unique_int &id) const;
    //!< returns the number of items waiting for the "id" specified, if any.

  void get_ids(structures::int_set &to_fill);
    //!< stuffs the set "to_fill" with the ids of all mailboxes present.
    /*!< if you want only those mailboxes holding one or more letters, then
    call the clean_up() method prior to this method. */

  bool close_out(const structures::unique_int &id);
    //!< dumps all packages stored for the "id" and shuts down its mailbox.
    /*!< the destructors for those packages should never try to do anything
    with the mailbox system or a deadlock could result.  true is returned if
    the "id" had a registered mailbox; false just indicates there was no box
    to clear up. */

  void show(basis::astring &to_fill);
    //!< provides a picture of what's waiting in the mailbox.
    /*!< this relies on the derived letter's required text_form() function. */

  void clean_up();
    //!< removes any empty mailboxes from our list.

  void limit_boxes(int max_letters);
    //!< establishes a limit on the number of letters.
    /*!< this is a helper function for a very special mailbox; it has a
    limited maximum size and any letters above the "max_letters" count will
    be deleted.  don't use this function on any mailbox where all letters
    are important; your mailbox must have a notion of unreliability before
    this would ever be appropriate. */

  enum apply_outcomes {
    OKAY = basis::common::OKAY,   //!< continue apply process.

    DEFINE_OUTCOME(APPLY_STOP, -46, "Halt the apply process"),
    DEFINE_OUTCOME(APPLY_WHACK, -47, "Removes the current letter, but "
        "continues"),
    DEFINE_OUTCOME(APPLY_WHACK_STOP, -48, "Halts apply and trashes the "
        "current letter")
  };

  typedef basis::outcome apply_function(letter &current, int uid, void *data_link);
    //!< the "apply_function" is what a user of apply() must provide.
    /*!< the function will be called on every letter in the mailbox unless one
    of the invocations returns APPLY_STOP or APPLY_WHACK_STOP; this causes
    the apply process to stop (and zap the node for APPLY_WHACK).  the "uid"
    is the target for the "current" letter.  the "data_link" provides a way
    for the function to refer back to a parent class or data package of some
    sort.  note that all sorts of deadlocks will occur if your apply
    function tries to do anything on the mailbox, even transitively.  keep
    those functions as simple as possible. */

  void apply(apply_function *to_apply, void *data_link);
    //!< calls the "to_apply" function on possibly every letter in the mailbox.
    /*!< this iterates until the function returns a 'STOP' outcome.  the
    "data_link" pointer is passed to the apply function.  NOTE: it is NOT safe
    to rearrange or manipulate the mailbox in any way from your "to_apply"
    function; the only changes allowed are those caused by the return value
    from "to_apply". */

private:
  basis::mutex *_transaction_lock;  //!< keeps the state of the mailbox safe.
  mailbox_bank *_packages;  //!< the collection of mail that has arrived.

  // prohibited.
  mailbox(const mailbox &);
  mailbox &operator =(const mailbox &);
};

} //namespace.

#endif

