#ifndef CENTRAL_MAILBOX_CLASS
#define CENTRAL_MAILBOX_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : post_office                                                       *
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

#include "mail_stop.h"

#include <processes/ethread.h>
#include <timely/time_stamp.h>

namespace processes {

class letter;
class letter_morph;
class mailbox;
class postal_cache;
class route_map;
class thread_cabinet;

//! Manages a collection of mailboxes and implements delivery routes for mail.

class post_office
{
public:
  post_office();

  virtual ~post_office();
    //!< stop_serving must be invoked prior to this destructor.

  void stop_serving();
    //!< gets the mailbox to stop delivering items prior to a shutdown.

  // informational functions...

  DEFINE_CLASS_NAME("post_office");

  void show_mail(basis::astring &to_fill);
    //!< prints a snapshot of all currently pending letters into "to_fill".

  void show_routes(basis::astring &to_fill);
    //!< writes a listing of the current routes into "to_fill".

  // general delivery services subject to the constraints of the mailbox class.

  void drop_off(const structures::unique_int &id, letter *package);
    //!< sends a "package" on its way to the "id" via the registered route.
    /*!< note that mail is not rejected if there is no known route to the
    mail_stop for the "id"; it is assumed in that case that the recipient
    will check at the post office. */

  bool pick_up(const structures::unique_int &id, letter * &package);
    //!< retrieves a "package" intended for the "id" if one exists.
    /*!< false is returned if none are available.  on success, the "package" is
    filled in with the address of the package and it is the caller's
    responsibility to destroy or recycle() it after dealing with it. */

  //////////////

  // mail delivery and routing support.

  bool register_route(const structures::unique_int &id, mail_stop &carrier_path);
    //!< registers a route "carrier_path" for mail deliveries to the "id".

  bool unregister_route(const structures::unique_int &id);
    //!< removes a route for the "id".
    /*!< this should be done before the object's destructor is invoked since
    the letter carrier could be on his way with a letter at an arbitrary time.
    also, the mail_stop should be shut down (with end_availability()) at that
    time also.  if those steps are taken, then the carrier is guaranteed not
    to bother the recipient. */

  bool route_listed(const structures::unique_int &id);
    //!< returns true if there is a route listed for the "id".
    /*!< this could change at any moment, since another place in the source
    code could remove the route just after this call.  it is information from
    the past by the time it's returned. */

  //////////////

  bool deliver_mail_on_route(const structures::unique_int &route, ethread &carrier);
    //!< for internal use only--delivers the letters to known routes.
    /*!< this function should only be used internally to prompt the delivery
    of packages that are waiting for objects we have a route to.  it returns
    true when all items that were waiting have been sent. */

private:
  basis::mutex c_mutt;  //!< protects our lists and dead letter office from corruption.
  mailbox *_post;  //!< the items awaiting handling.
  route_map *_routes;  //!< the pathways that have been defined.
  timely::time_stamp *_next_cleaning;  //!< when the next mailbox flush will occur.
  thread_cabinet *_threads;  //!< our list of threads for postal routes.

  void get_route_list(structures::int_set &route_set);
    //!< retrieves the list of routes that we have registered.

  void clean_package_list(post_office &post, letter_morph &to_clean);
    //!< recycles all of the letters held in "to_clean".
};

} //namespace.

#endif

