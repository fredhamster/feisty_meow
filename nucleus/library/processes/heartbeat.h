#ifndef HEARTBEAT_CLASS
#define HEARTBEAT_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : heartbeat                                                         *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1996-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/astring.h>
#include <basis/contracts.h>
#include <timely/time_stamp.h>

namespace processes {

//! Monitors a periodic heartbeat to track a resource's health.
/*!
  The heartbeat is defined as a "request-and-response" based check; when the
  next periodic heartbeat is due, a request is sent out.  The heartbeat
  request is considered successfully dealt with only if a response comes back
  for the request.  If the user-defined number of requests are sent without
  a single response coming back, then the 'patient' is considered dead.
*/

class heartbeat : public virtual basis::root_object
{
public:
  heartbeat(int misses_allowed = 500, int check_interval = 10000);
    //!< creates a heartbeat monitor with the specified interval and maximum skips permitted.
    /*!< this allows the heartbeat request to be missed "misses_allowed" times.
    the heartbeats will become due every "check_interval" milliseconds.  the
    defaults are a joke; you really need to set them. */
  heartbeat(const heartbeat &to_copy);

  ~heartbeat();

  DEFINE_CLASS_NAME("heartbeat");

  heartbeat &operator =(const heartbeat &to_copy);

  basis::astring text_form(bool detailed = false) const;
    //!< returns a readable form of the heartbeat's information.

  void reset(int misses_allowed, int check_interval);
    //!< retrains the heartbeat monitor for a new configuration.

  bool due() const;
    //!< is the next heartbeat due yet?

  bool dead() const;
    //!< is this object considered dead from missing too many heartbeats?
    /*!< this is true if the heartbeat being monitored missed too many
    responses to heartbeat requests.  if the maximum allowed requests have
    been made and there was not even a single response, then the object is
    considered dead. */

  void made_request();
    //!< records that another heartbeat request was sent out.
    /*!< the time for the next heartbeat request is reset to the time between
    beats.  if there were already the maximum allowed number of missed
    responses, then the object is now dead. */
  void need_beat() { made_request(); }
    //!< a synonym for the made_request() method.

  void kabump();
    //!< registers a heartbeat response and sets the state to healthy.
    /*!< this records that a heartbeat response came back from the monitored
    object.  after this call, there are no heartbeats recorded as missed at
    all. */
  void recycle() { kabump(); }
    //!< a synonym for kabump().

  // reporting functions for internal state...

  int missed_so_far() const { return _misses; }
    //!< returns the number of heartbeat responses that are pending.
  int misses_left() const { return _misses_allowed - _misses; }
    //!< the number of misses that this object is still allowed.

  int allowed_misses() const { return _misses_allowed; }
    //!< returns the number of misses allowed overall.
  int checking_interval() const { return _check_interval; }
    //!< returns the period of the heartbeats.

  timely::time_stamp heartbeat_time() const;
    //!< returns the time when the next heartbeat will be requested.
    /*!< if no heartbeats had been missed yet, then this is the time when
    the due() method starts returning true. */

  int time_left() const;
    //!< number of milliseconds left before the next beat will be requested.
    /*!< if the number is zero or negative, then a heartbeat is due. */

private:
  timely::time_stamp *_next_heartbeat;
  int _check_interval;
  int _misses_allowed;
  int _misses;

  void reset_next_beat();  //!< resets the next_heartbeat to our interval.
};

} //namespace.

#endif

