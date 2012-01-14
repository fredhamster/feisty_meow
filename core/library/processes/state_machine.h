#ifndef STATE_MACHINE_CLASS
#define STATE_MACHINE_CLASS

/*****************************************************************************\
*                                                                             *
*  Name   : state_machine                                                     *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 1992-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include <basis/contracts.h>
#include <timely/time_stamp.h>

namespace processes {

class state_machine_override_array;
class state_machine_state_array;

//! Monitors objects with multiple states and the transitions between states.
/*!
  A state machine is a computational abstraction for any control process
  that has discrete states and transitions between the states.
  As used here, the "state_machine" is a base class for objects that can be
  manipulated by the "transition_map" class.  Your transition map must be
  validated and you must use it to reset() your state_machine object before
  any activity can occur.  (One aspect of validation is that all states must be
  reachable from the user-specified starting state.  The other requirements
  are documented below for transition_map.) Once validation is done, the
  transition map manages the machine through the various states that are
  defined and applies trigger values to modify the current state when asked
  to do so.  It also applies any defined timed transitions automatically.
  This implementation of state machines is configured once (by defining the
  transition_map object), but then the machines can be run repeatedly (via
  the state_machine object).
*/

class state_machine : public virtual basis::root_object
{
public:
  state_machine();
    //!< sets up the machine in a blank state.

  state_machine(const state_machine &to_copy);
    //!< copies the machine provided in "to_copy".

  virtual ~state_machine();

  DEFINE_CLASS_NAME("state_machine");

  state_machine &operator =(const state_machine &to_copy);
    //!< assigns this to a copy of the machine provided in "to_copy".

  virtual int update();
    //!< this is the main implementation function provided by derived classes.
    /*!< this is implemented by derived classes that want to perform an action
    when the state machine is pulsed (see below in transition_map), which is
    why it is not pure virtual; a state machine might still be useful as a
    state tracking object rather than needing to implement extended
    functionality.  this function is invoked whenever the state changes due to
    a timed, range based or simple transition.  one must be careful when
    servicing this transition not to enmesh oneself in a snarled invocation
    situation; if make_transition or time_slice or pulse are invoked from this
    update function and conditions are right for another transition, then the
    update function will be re-entered recursively.  if the value returned
    from update is non-zero, it is used to pulse the state machine again as a
    new trigger value (this is safer than explicitly calling one of the
    transition causing functions).  note that this assumes that zero is an
    invalid trigger.  if your triggers include zero as a valid value, don't
    try returning it through update; use pulse to force the triggering to
    accept the invalid trigger instead. */

  int current() const { return _current; }
    //!< returns the current state.
    /*!< NOTE: a zero value for a state means that it is uninitialized. */

  int last() const { return _last; }
    //!< returns the last active state.

  int trigger() const { return _trig; }
    //!< returns the trigger that caused this state.
    /*!< this is only meaningful when the transition was caused by a range
    transition.  if it was, then ranged() will return true. */

  enum transition_types { SIMPLE, RANGE, TIMED };
    //!< the three types of transitions supported.

  bool simple() const { return _type == SIMPLE; }
    //!< returns true if the last transition was a simple type.
  bool ranged() const { return _type == RANGE; }
    //!< returns true if the last transition was a ranged type.
  bool timed() const { return _type == TIMED; }
    //!< returns true if the last transition was a timed type.

  void set_state(int new_current, int new_last, int trigger,
          transition_types type);
    //!< sets the current state to "new_current" and the previous state to "new_last".
    /*!< the "trigger" causing the transition, if any, will be stored also.
    the "type" of transition is stored also.  the time stamp for time spent in
    the current state is reset.  be very careful with this; if the two states
    do not conform to the legal transitions in your map, then the state
    machine will not behave properly. */

  timely::time_stamp start() const;
    //!< start time for the current state.
    /*!< this records how long we've been in this state. */

  void set_name(const basis::astring &name);
    //!< sets the name to be printed in any debugging information for "this".

  basis::astring get_name() const;
    //!< retrieves the object's current name.

  void override_timing(int current, int next, int duration);
    //!< modifies the recorded timing for timed transitions.
    /*!< this overrides the transition_map's time-out value for the transition
    between the states "current" and "next".  a time-out of "duration"
    will be used instead of whatever was specified when the map was set
    up.  to erase an override, use zero as the "duration". */

  int duration_override(int current, int next) const;
    //!< reports if there's a duration override for a timed transition.
    /*!< this returns the amount of time that this particular state_machine is
    allowed to exist in the "current" state before progressing to the "next"
    state.  this has nothing to do with the transition_map; it is valid only
    for this state_machine object.  zero is returned if no override exists. */

private:
  friend class transition_map;
    //!< manager buddy object.
  int _current;  //!< the current state of the state machine.
  int _last;  //!< the previous state for the state machine.
  int _trig;  //!< the last trigger value, if _ranged is true.
  transition_types _type;  //!< what kind of transition just occurred.
  timely::time_stamp *_start;  //!< the time this state started.
  basis::astring *_name;  //!< the name this state_machine reports itself as.
  state_machine_override_array *_overrides;  //!< the timing overrides.

  int duration_index(int current, int next) const;
    //!< finds the index of a duration override in our list.
    /*!< this locates the duration override specified for the transition
    between "current" and "next".  it returns the index of that override, or
    a negative number if the override is not found. */
};

//////////////

//! The transition_map manages state machines and causes state changes to occur.
/*!
  The transition_map is a heavyweight class that is a repository for all
  information about transitions and which manages pushing the state machines
  through the proper states.

  The transition_map guarantees these characteristics:

    0) the below characteristics are checked (in validate) and no state
       machine object is allowed to operate until they are satisfied,

    1) the machine starts in the specified starting state,

    2) the current state is always one that has been added and approved,

    3) transitions are allowed between states only if the transition has
       been added and approved,

    4) the update() function is invoked every time the machine hits a
       transition between states (even if it is a transition to the same
       state),

    5) range transitions are non-intersecting within one state,
*** 5 is unimplemented ***

    6) all states are reachable from the starting state by some valid
       transition, and

    7) each state has no more than one timed transition.

  if any of these conditions are violated, then validate() will fail.
  the machine will also not operate properly (at all) until the conditions
  are satisfied by validate().  the states and transitions should
  thus be carefully examined before turning them into a state machine....
*/

class transition_map : public virtual basis::root_object
{
public:
  transition_map();
  virtual ~transition_map();

  // informational functions...

  DEFINE_CLASS_NAME("transition_map");

  bool valid() const { return _valid; }
    //!< returns true if the transition_map is valid and ready for operation.
    /*!< once the validate() call has succeeded and valid() is true, no more
    configuration functions (see below) may be called until the reconfigure()
    function is invoked. */

  int states() const;
    //!< returns the current number of states.

  // validation functions...

  enum outcomes {
    OKAY = basis::common::OKAY,
    DEFINE_OUTCOME(BAD_START, -49, "The start state has not been properly "
        "specified"),
    DEFINE_OUTCOME(OVERLAPPING_RANGES, -50, "The ranges overlap for two "
        "transitions from a state"),
    DEFINE_OUTCOME(UNREACHABLE, -51, "There is an unreachable state in the map")
  };
  basis::outcome validate(int &examine);
    //!< checks to that all required transition conditions are satisfied.
    /*!< OKAY is returned if they are and the map is then ready to operate.
    other outcomes are returned if one or more of the conditions are not met:
    BAD_START means that the starting state is badly specified.
    OVERLAPPING_RANGES means that one state has two transitions that do not
    have mutually exclusive ranges.  UNREACHABLE means that a state is not
    reachable from the starting state.  for all of these cases, the "examine"
    parameter is set to a state related to the problem. */

  void reconfigure();
    //!< puts the transition_map back into an unvalidated state.
    /*!< this allows the characteristics of the map to be changed.  validate()
    must be called again before resuming operation using this map. */

  // configuration functions...

  // NOTE: all of the functions below will fail if the transition_map has
  // already been validated previously and reconfigure() has not been called.

  // NOTE: a zero value for a state means that it is uninitialized.  thus, zero
  // is never allowed as a value for a state or a transition endpoint.  zero is
  // grudgingly allowed as a trigger value, although that interferes with the
  // state_machine object's update() method.

  bool add_state(int state_number);
    //!< registers a legal state in the transition_map.
    /*!< no action will be taken for any state until it is registered.  the
    operation returns true for successful registration and false for errors
    (such as when a state is already registered or is invalid). */

  bool set_start(int starting_state);
    //!< assigns a state as the first state.
    /*!< if the "starting_state" does not already exist, it is an error and
    false is returned. */

  bool add_simple_transition(int current, int next);
    //!< adds a transition between "current" and "next".
    /*!< it is an error to use either an invalid "current" state or an invalid
    "next" state.  these errors are detected during the validate() function.
    this type of transition is unspecialized--it does not respond to triggers
    and does not occur due to timing.  to make a state_machine undergo this
    transition, make_transition() must be used. */
    
  bool add_range_transition(int current, int next, int low, int high);
    //!< adds a transition that listens to triggers in the pulse() method.
    /*!< this uses "low" and "high" as the inclusive range of triggers that
    will cause a transition to the "next" state when a state_machine is pulsed
    in the "current" state.  it is erroneous for these trigger values to
    intersect with other values in the same "current" state. */
  bool add_timed_transition(int current, int next, int duration);
    //!< adds a transition that occurs after a timeout with no other activity.
    /*!< adds a timed transition to the "next" state when the "current" state
    has lasted "duration" milliseconds.  this relies on the time_slice function
    being called periodically, with appropriate regularity for the specified
    "duration" (e.g., if one's time-out is 100 ms, then one might want to call
    time_slice() every 20 ms or so to ensure that the transition is at most 20
    ms late in firing.  NOTE: this is not an argument for calling time_slice()
    as fast as possible; it is an argument for realizing that the "duration"
    must be carefully considered to meet one's timing deadlines). */

  // transition functions...

  // NOTE: all of these actions will fail if the transition_map has not been
  // validated yet.

  bool make_transition(state_machine &m, int next);
    //!< changes the state to the "next" listed for "m" given the current state.
    /*!< it is an error to make a transition that has not been specified
    through an add_X() transition function (false is returned).  if the
    transition succeeds, then the current_state will be "next". */

  bool pulse(state_machine &m, int trigger);
    //!< applies a "trigger" to possibly cause a range transition.
    /*!< this causes the state_machine to accept the "trigger" as input and
    perform at least one traversal of the transition_map.  if the trigger
    value is found in one of the range transitions for the current state, then
    the next state specified in that transition becomes the current state and
    the update() function is invoked (and true is returned).  if the trigger
    is not found in any range transition, then false is returned. */

  bool time_slice(state_machine &m);
    // allows the transition_map to process any timed transitions that may be
    // required for the state_machine "m".  the return value is true if such
    // a transition was found.

  bool reset(state_machine &m);
    // resets the state_machine to the starting state in the transition_map.
    // the update function is NOT invoked at the time of the reset.  true is
    // returned if the reset was successful; it would fail if the transition
    // map has not been validated yet.

private:
  bool _valid;  //!< records whether we've been validated or not.
  int _start_state;  //!< remembers the default starting state.
  state_machine_state_array *_state_list;
    //!< the list of transitions between states.

  bool check_overlapping(int &examine);
    //!< a validation function that looks for erroneous range transitions.
    /*!< this returns true if there are no overlapping ranges found in the
    range transitions for each state. */

  bool check_reachability(int &examine);
    //!< returns true if all states are reachable from the starting state.

  int state_index(int state_id) const;
    //!< returns the index of "state_id" in states, if it exists.

  int transition_index(int state_index, int next, int &start);
    //!< locates a transition into "next" for a state in our list.
    /*!< returns the index of a transition between the state at "state_index"
    and the "next" state by starting the search at index "start".  if there
    is no matching transition from the "start" index on in the transition
    list, then a negative number is returned.  "start" is updated to point
    to the index just after a found transition, or to some random number if
    the index is not found. */

  bool check_states();
    //!< returns false if the current machine is hosed up.

  // disallowed functions for now:
  transition_map(const transition_map &);
  transition_map &operator =(const transition_map &);
};

} //namespace.

#endif

