#ifndef TENTACLE_CLASS
#define TENTACLE_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : tentacle                                                          *
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

#include <basis/byte_array.h>
#include <basis/mutex.h>
#include <structures/string_array.h>

namespace octopi {

// forward.
class entity_data_bin;
class infoton;
class octopus;
class octopus_entity;
class octopus_request_id;
class pod_motivator;
class queueton;

//! Manages a service within an octopus by processing certain infotons.

class tentacle : public virtual basis::root_object
{
public:
  tentacle(const structures::string_array &group_name, bool backgrounded,
          int motivational_rate = tentacle::DEFAULT_RATE);
    //!< constructs a tentacle that handles infotons with the "group_name".
    /*!< if "backgrounded" is true, then the tentacle will periodically look
    for queued requests at the specified "motivational_rate".  if
    "backgrounded" is false, then the tentacle will not perform any background
    processing, meaning that it can only provide immediate evaluation for an
    octopus. */

  virtual ~tentacle();

  enum constants { DEFAULT_RATE = 40 };

  DEFINE_CLASS_NAME("tentacle");

  const structures::string_array &group() const;
    //!< returns the name of the group that this tentacle services.
    /*!< this can be a single string or it can be a list of names.  a tentacle
    can only service one name group currently. */

  bool backgrounding() const { return _backgrounded; }
    //!< reports on whether this tentacle supports background operation or not.

  int motivational_rate() const;
    //!< returns the background processing rate this was constructed with.

  enum outcomes {
    OKAY = basis::common::OKAY,
    NOT_FOUND = basis::common::NOT_FOUND,
    ALREADY_EXISTS = basis::common::EXISTING,
    BAD_INPUT = basis::common::BAD_INPUT,
    NO_SPACE = basis::common::NO_SPACE,
    GARBAGE = basis::common::GARBAGE,
    DISALLOWED = basis::common::DISALLOWED,

    NO_HANDLER = basis::common::NO_HANDLER,  //!< no handler for that type of infoton.
    PARTIAL = basis::common::PARTIAL,  //!< processing of request is partially done.
    ENCRYPTION_MISMATCH = basis::common::ENCRYPTION_MISMATCH
      //!< there is a disconnect regarding encryption.
  };

  static const char *outcome_name(const basis::outcome &to_name);
    //!< returns the textual form of the outcome "to_name".

  //////////////

  // functions that must be provided by derived tentacles.

  virtual basis::outcome reconstitute(const structures::string_array &classifier,
          basis::byte_array &packed_form, infoton * &reformed) = 0;
    //!< regenerates an infoton from its packed form.
    /*!< given the "classifier" under which the object is categorized, this
    reconstructs a "reformed" infoton equivalent to the flattened infoton
    in "packed_form".  the "packed_form" is destructively consumed.
    NOTE: it is crucial that the derived method calls set_classifier() on
    the "reformed" infoton from the passed "classifier". */

  virtual basis::outcome consume(infoton &to_chow, const octopus_request_id &item_id,
          basis::byte_array &transformed) = 0;
    //!< this is the main function that processes infotons for this tentacle.
    /*!< the octopus will feed this function with appropriate data "to_chow"
    for infotons that are to be processed by this tentacle's group().  the
    "item_id" provides for the requesting entity an origination marker that
    can be used in produce() below.  the outcome indicates whether the
    processing was successful.  processing could fail due to a missing
    handler for the item, due to erronous data in the infoton, because of
    resource limits placed on the tentacle, from explicit rejection by the
    tentacle, or due to other causes.  the "transformed" is a packed infoton
    that may be generated during the consumption of "to_chow" (it must
    actually be a packed classifier and then the packed infoton).  if it can
    be unpacked successfully, then it will be treated as the actual infoton
    that was to be consumed.  this is only expected from a filter.
    note: the infoton "to_chow" can be destructively manipulated by the
    tentacle, including patching the classifier for internal octopi or
    rearranging any data contained in "to_chow".  none of these changes
    will be seen by the entity that requested processing.
    regarding filters: if this tentacle is serving as a filter, then it
    may be presented with infotons that are not covered by its group.
    given such an infoton, the tentacle should perform whatever filtering
    is to be done, including modifying the infoton appropriately, and
    return PARTIAL when it liked the infoton or DISALLOWED when it
    rejects the infoton.  it is understood that the infoton will be
    passed along to the rest of the tentacles when the successful
    result of PARTIAL is returned. */

  virtual void expunge(const octopus_entity &to_remove) = 0;
    //!< called to remove traces of the entity "to_remove".
    /*!< this is an order from the octopus that all traces of the entity
    "to_remove" should now be cleaned out.  that entity has been utterly
    destroyed and any data structures held for it should be thrown out
    also.  the required actions are specific to the tentacle's design. */

  //////////////

  basis::outcome enqueue(infoton *to_chow, const octopus_request_id &item_id);
    //!< holds onto infotons coming from the octopus for backgrounding.
    /*!< this will add an infoton "to_chow" into the list of objects to be
    consumed.  at some point after a successful outcome from this, the
    tentacle will be handed the infoton for processing.  NOTE: all
    responsibility for the infoton "to_chow" is passed to this method; the
    infoton should not be touched in any way after invocation. */

  infoton *next_request(octopus_request_id &item_id);
    //!< pops out the next queued request for processing.
    /*!< this function locates the next request for the tentacle when it is
    in its consume() method.  the returned infoton was previously passed
    to the enqueue() method and needs to be processed.  if there are no
    requests ready, NULL_POINTER is returned. */

  bool store_product(infoton *product, const octopus_request_id &original_id);
    //!< used by tentacles to store the objects they produce from infotons.
    /*!< this will cache the "product" object and its "original_id" for later
    retrieval from the entity_data_bin we were given when hooked to an octopus.
    note that the "product" must be allocated dynamically and that it becomes
    owned by the response bin after this call (do not delete the original
    pointer).  if the item would not fit in the entity's bin, then false is
    returned and the "product" is deleted. */

  //////////////

  // support that is for internal use only.

  void attach_storage(entity_data_bin &storage);
    //!< used when a tentacle is being integrated with an octopus.
    /*!< not for casual external users.  note that the tentacle's background
    processing will not be started until attach is called and that it stops
    when detach is called. */
  void detach_storage();
    //!< unhooks the storage bin from this tentacle.
  entity_data_bin *get_storage();
    //!< returns the current pointer, which might be nil.

  void propel_arm();
    //!< invoked by our thread to cause requests to be processed.
 
private:
  structures::string_array *_group;  //!< the group name that this tentacle handles.
  queueton *_pending;  //!< the requests that are waiting fulfillment.
  basis::mutex *_input_guard;  //!< protects the incoming requests.
  pod_motivator *_action;  //!< the thread that keeps things moving along.
  entity_data_bin *_products;  //!< if non-nil, where we store responses.
  bool _backgrounded;  //!< records whether we're threading or not.

  // not permitted.
  tentacle(const tentacle &);
  tentacle &operator =(const tentacle &);
};

} //namespace.

#endif

