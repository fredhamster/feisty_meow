#ifndef ENTITY_DEFINITIONS_GROUP
#define ENTITY_DEFINITIONS_GROUP

/*****************************************************************************\
*                                                                             *
*  Name   : various definitions for octopus                                   *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2002-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "infoton.h"

#include <basis/byte_array.h>
#include <basis/astring.h>
#include <structures/set.h>
#include <structures/amorph.h>
#include <structures/unique_id.h>
#include <timely/time_stamp.h>

namespace octopi {

//! Provides a way of identifying users of an octopus object.
/*!
  NOTE: this is a heavy-weight header intended for forward declaration.
*/

class octopus_entity : public virtual basis::packable, public virtual basis::text_formable
{
public:
  octopus_entity();  //!< blank constructor.

  octopus_entity(const basis::astring &hostname, int process_id, int sequencer,
          int add_in);
    //!< constructor taking all the available parameters for an entity.
    /*!< produces an id in the proper format given all the components.  note
    that the hostname must be some fully qualified name for the host, such
    that it is as unique as you want names within the system to be. */

  octopus_entity(const octopus_entity &to_copy);

  ~octopus_entity();

  octopus_entity &operator = (const octopus_entity &to_copy);

  DEFINE_CLASS_NAME("octopus_entity");

  bool blank() const;
    //!< true if the entity is blank, as constructed by default constructor.

  // comparison operators.
  bool operator == (const octopus_entity &that) const;

  const basis::astring &hostname() const;  //!< returns the hostname portion of the id.
  int process_id() const;   //!< returns the process number in the id.
  int sequencer() const;    //!< returns the sequencing number from the id.
  int add_in() const;       //!< returns the random add-in from the id.

  void hostname(const basis::astring &new_host);  //!< set the host.
  void process_id(int id);  //!< set the process id.
  void sequencer(int seq);  //!< set the sequencer value.
  void add_in(int add);  //!< set the add-in value.

  basis::astring mangled_form() const;
    //!< returns the combined string form of the identifier.

  basis::astring text_form() const;
    //!< returns a readable form of the identifier.

  virtual void text_form(basis::base_string &to_fill) const {
    to_fill.assign(text_form());
  }

  basis::astring to_text() const { return mangled_form(); }
    //!< conversion to text format for display.
  static octopus_entity from_text(const basis::astring &to_convert);
    //!< conversion from text format, parsing parameters out.

  static void breakout(const basis::astring &mangled_form, basis::astring &hostname,
          int &process_id, int &sequencer, int &add_in);
    //!< takes a "mangled_form" of an entity id and retrieves the components.

  int packed_size() const;
    //!< reports how large the packed entity will be.

  virtual void pack(basis::byte_array &packed_form) const;
  virtual bool unpack(basis::byte_array &packed_form);

private:
  basis::astring *_hostname;
  int _pid;
  int _sequencer;
  int _add_in;
};

//////////////

//! Identifies requests made on an octopus by users.
/*!
  Uniquely identifies a request passed to an octopus server given that
  the number of requests from one entity can be contained within the range
  of signed integers.  if the requests come fast enough for the request
  numbers to wrap around, and if the requests can remain outstanding for a
  really long time, it may be wise to create a new login.
*/

class octopus_request_id : public virtual basis::packable
{
public:
  octopus_entity _entity;  //!< the entity.
  int _request_num;  //!< the item number from the entity.

  octopus_request_id() : _entity(), _request_num(0) {}

  octopus_request_id(const octopus_entity &entity, int request_num)
      : _entity(entity), _request_num(request_num) {}

  static octopus_request_id randomized_id();
    //!< provides a pre-randomized request id.
    /*!< this should only be used for the very first request made to an
    octopus, in order to obtain a valid identity. */

  bool operator == (const octopus_request_id &that) const
          { return (_entity == that._entity)
                && (_request_num == that._request_num); }

  bool blank() const;
    //!< returns true if this is a blank id (as constructed by default ctor).

  int packed_size() const;
    //!< reports how large the packed id will be.

  basis::astring mangled_form() const;  //!< similar to entity id.
  
  basis::astring text_form() const;  //!< human readable form of the request.

  basis::astring to_text() const { return mangled_form(); }
  static octopus_request_id from_text(const basis::astring &to_convert);

  virtual void pack(basis::byte_array &packed_form) const;
  virtual bool unpack(basis::byte_array &packed_form);
};

//! a collection of unique request ids.
class octopus_request_id_set : public structures::set<octopus_request_id>
{
public:
  octopus_request_id_set() {}

  octopus_request_id_set(const structures::set<octopus_request_id> &orig)
      : structures::set<octopus_request_id>(orig) {}
};

//////////////

//! implements a list of waiting infotons.
/*! the actual infoton plus its request id are stored. */

class infoton_id_pair : public virtual basis::root_object
{
public:
  infoton *_data;
  octopus_request_id _id;

  infoton_id_pair(infoton *data, const octopus_request_id &id)
      : _data(data), _id(id) {}

  ~infoton_id_pair() {
    delete _data;
    _data = NIL;
  }
};

//! a list of pending requests and who made them.
class infoton_list : public structures::amorph<infoton_id_pair> {};

} //namespace.

#endif

