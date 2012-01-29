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

#include "state_machine.h"

#include <basis/array.h>
#include <basis/functions.h>
#include <basis/guards.h>
#include <basis/astring.h>
#include <loggers/program_wide_logger.h>
#include <timely/time_stamp.h>

using namespace basis;
using namespace loggers;
using namespace timely;

namespace processes {

//////////////

//#define DEBUG_STATE_MACHINE
  // uncomment if you want the debugging version.

//hmmm: implement logging...
#undef LOG
#ifndef DEBUG_STATE_MACHINE
  #define LOG(to_print) CLASS_EMERGENCY_LOG(program_wide_logger::get(), to_print)
#else
  #define LOG(to_print) {}
#endif

//////////////

// checks whether the machine passed in is valid or not.
#define CHECK_VALID(m) \
  if (!m._current) return false; \
  if (!m._last) return false

// checks whether the current and next states exist or not.
#define CHECK_STATES \
  if (!current) return false; \
  if (!next) return false; \
  int indy = state_index(current); \
  if (negative(indy)) return false; \
  if (negative(state_index(next))) return false

// moves to a new state and resets the new state's timestamp.
#define MOVE_STATE(m, next, type, trigger) \
  m._last = m._current; \
  m._current = next; \
  m._type = type; \
  m._trig = trigger; \
  m._start->reset()

// locates a state or returns.
#define FIND_STATE(state) \
  int indy = state_index(state); \
  if (negative(indy)) return

//////////////

struct override { int current; int next; int duration;
  override(int _current = 0, int _next = 0, int _duration = 0)
    : current(_current), next(_next), duration(_duration) {}
};

struct transition_info {
  enum transition_type { SIMPLE, RANGE, TIMED };
  transition_type type;
  int next_state;
  int low_trigger, high_trigger;
  int time_span;

  transition_info() {}  // blank.
  transition_info(int next) : type(SIMPLE), next_state(next) {}
  transition_info(int next, int time) : type(TIMED), next_state(next),
      time_span(time) {}
  transition_info(int next, int low, int high) : type(RANGE),
      next_state(next), low_trigger(low), high_trigger(high) {}
};

struct state_info {
  int state_id;  // id for this state.
  array<transition_info> transitions;

  state_info() : state_id(0) {}  // blank.
  state_info(int state_id_in) : state_id(state_id_in) {}
};

//////////////

class state_machine_override_array : public array<override> {};
class state_machine_state_array : public array<state_info> {};

//////////////

state_machine::state_machine()
: _current(0),
  _last(0),
  _trig(0),
  _type(SIMPLE),
  _start(new time_stamp()),
  _name(new astring()),
  _overrides(new state_machine_override_array)
{}

state_machine::state_machine(const state_machine &to_copy)
: root_object(),
  _current(0),
  _last(0),
  _trig(0),
  _type(SIMPLE),
  _start(new time_stamp()),
  _name(new astring()),
  _overrides(new state_machine_override_array)
{ *this = to_copy; }

state_machine::~state_machine()
{
  WHACK(_start);
  WHACK(_name);
  WHACK(_overrides);
}

int state_machine::update() { return 0; }

void state_machine::set_name(const astring &name) { *_name = name; }

astring state_machine::get_name() const { return *_name; }

state_machine &state_machine::operator = (const state_machine &to_copy)
{
  if (&to_copy == this) return *this;
  _current = to_copy._current;
  _last = to_copy._last;
  _trig = to_copy._trig;
  _type = to_copy._type;
  *_start = *to_copy._start;
  *_name = *to_copy._name;
  *_overrides = *to_copy._overrides;
//careful when overrides becomes hidden internal type.
  return *this;
}

int state_machine::duration_index(int current, int next) const
{
  for (int i = 0; i < _overrides->length(); i++)
    if ( ((*_overrides)[i].current == current)
        && ((*_overrides)[i].next == next) )
      return i;
  return common::NOT_FOUND;
}

void state_machine::set_state(int new_current, int new_last, int trigger,
    transition_types type)
{
  _current = new_current;
  _last = new_last;
  _trig = trigger;
  _type = type;
  _start->reset();
}

void state_machine::override_timing(int current, int next, int duration)
{
  int indy = duration_index(current, next);
  if (non_negative(indy)) {
    // found an existing record for this current/next pair.
    if (!duration) {
      // zero duration means removal.
      _overrides->zap(indy, indy);
      return;
    }
    // reset the duration.
    (*_overrides)[indy].duration = duration;
    return;
  }
  // no existing record, so add one.
  *_overrides += override(current, next, duration);
}

int state_machine::duration_override(int current, int next) const
{
  int indy = duration_index(current, next);
  if (negative(indy)) return 0;
  return (*_overrides)[indy].duration;
}

time_stamp state_machine::start() const { return *_start; }

//////////////

transition_map::transition_map()
: _valid(false),
  _start_state(0),
  _state_list(new state_machine_state_array)
{}

transition_map::~transition_map()
{
  _valid = false;
  WHACK(_state_list);
}

// informational functions:

int transition_map::states() const { return _state_list->length(); }

int transition_map::state_index(int state_id) const
{
  for (int i = 0; i < states(); i++)
    if ((*_state_list)[i].state_id == state_id) return i;
  return common::NOT_FOUND;
}

int transition_map::transition_index(int state_index, int next, int &start)
{
  bounds_return(state_index, 0, states() - 1, common::BAD_INPUT);
  state_info &state = (*_state_list)[state_index];
  bounds_return(start, 0, state.transitions.length() - 1, common::BAD_INPUT);
  // loop over the transitions by using our external index.
  for (start = start; start < state.transitions.length(); start++)
    if (state.transitions[start].next_state == next) {
      start++;  // position it after this index.
      return start - 1;  // return this index.
    }
  return common::NOT_FOUND;
}

// configurational functions:

void transition_map::reconfigure() { _valid = false; }

outcome transition_map::validate(int &examine)
{
  // check for a bad starting state...
  examine = _start_state;
  FIND_STATE(_start_state) BAD_START;

  if (!check_reachability(examine)) return UNREACHABLE;
    // a state is unreachable from the starting state.
  if (!check_overlapping(examine)) return OVERLAPPING_RANGES;
    // bad (overlapping) ranges were found in one state.
  _valid = true;  // set us to operational.
  return OKAY;
}

bool transition_map::add_state(int state_number)
{
  if (valid()) return false;  // this is operational; no more config!
  if (!state_number) return false;  // zero is disallowed.
  if (non_negative(state_index(state_number))) return false;  // already exists.
  *_state_list += state_info(state_number);
  return true;
}

bool transition_map::set_start(int starting_state)
{
  if (valid()) return false;  // this is operational; no more config!
  if (!starting_state) return false;

  FIND_STATE(starting_state) false;  // doesn't exist.

  _start_state = starting_state;
  return true;
}

bool transition_map::add_simple_transition(int current, int next)
{
  if (valid()) return false;  // this is operational; no more config!
  CHECK_STATES;
  (*_state_list)[indy].transitions += transition_info(next);
  return true;
}

bool transition_map::add_range_transition(int current, int next, int low, int high)
{
  if (valid()) return false;  // this is operational; no more config!
  CHECK_STATES;
  (*_state_list)[indy].transitions += transition_info(next, low, high);
  return true;
}

bool transition_map::add_timed_transition(int current, int next, int length)
{
  if (valid()) return false;  // this is operational; no more config!
  CHECK_STATES;
  state_info &found = (*_state_list)[indy];
  // locates any existing timed transition and re-uses its slot.
  for (int i = 0; i < found.transitions.length(); i++)
    if (found.transitions[i].type == transition_info::TIMED) {
      found.transitions[i] = transition_info(next, length);
      return true;
    }
  // no existing timed transition found, so add a new one.
  (*_state_list)[indy].transitions += transition_info(next, length);
  return true;
}

// operational functions:

bool transition_map::make_transition(state_machine &m, int next)
{
  if (!valid()) return false;  // this is not operational yet!
  CHECK_VALID(m);
#ifdef DEBUG_STATE_MACHINE
  if (negative(state_index(m._current)))
    LOG(astring("(%s) transition_map::make_transition: bad logic error; machine's "
        "state is missing.", m._name->s()));
  if (negative(state_index(next)))
    LOG(astring("(%s) transition_map::make_transition: next state parameter is invalid.",
        m._name->s()));
#endif
  FIND_STATE(m._current) false;  // bad next state.
  int start = 0;
  if (negative(transition_index(indy, next, start))) return false;
    // no transition exists for that next state.
  MOVE_STATE(m, next, state_machine::SIMPLE, 0);
  int trig = m.update();
  if (trig) return pulse(m, trig);
  return true;
}

bool transition_map::pulse(state_machine &m, int trigger)
{
  if (!valid()) return false;  // this is not operational yet!
  CHECK_VALID(m);
#ifdef DEBUG_STATE_MACHINE
  if (negative(state_index(m._current)))
    LOG(astring("(%s) transition_map::pulse: bad logic error; state is missing.", m._name->s()));
#endif
  FIND_STATE(m._current) false;  // logic error!
  state_info &found = (*_state_list)[indy];
  for (int i = 0; i < found.transitions.length(); i++) {
    if (found.transitions[i].type == transition_info::RANGE) {
      // found the right type of transition.
      transition_info &tran = found.transitions[i];
      if ( (tran.low_trigger <= trigger)
            && (tran.high_trigger >= trigger) ) {
        // found a transition with an acceptable range.
        MOVE_STATE(m, tran.next_state, state_machine::RANGE, trigger);
        int trig = m.update();
        if (trig) return pulse(m, trig);
        return true;
      }
    }
  }
  return false;
}

bool transition_map::time_slice(state_machine &m)
{
  if (!valid()) return false;  // this is not operational yet!
  CHECK_VALID(m);

#ifdef DEBUG_STATE_MACHINE
  if (negative(state_index(m._current)))
    LOG(astring("(%s) transition_map::time_slice: bad logic error; state "
        "is missing.", m._name->s()));
#endif
  FIND_STATE(m._current) false;  // logic error!

  state_info &found = (*_state_list)[indy];
  for (int i = 0; i < found.transitions.length(); i++) {
    if (found.transitions[i].type == transition_info::TIMED) {
      // found the right type of transition.
      transition_info &tran = found.transitions[i];
      int duration = tran.time_span;
      int override = m.duration_override(m._current, tran.next_state);
      if (override) duration = override;
      if (*m._start < time_stamp(-duration)) {
        // found a transition with an expired time.
        MOVE_STATE(m, tran.next_state, state_machine::TIMED, 0);
        int trig = m.update();
        if (trig) return pulse(m, trig);
        return true;
      }
    }
  }
  return false;
}

bool transition_map::reset(state_machine &m)
{
  if (!valid()) return false;  // this is not operational yet!
  m._current = _start_state;
  m._last = _start_state;
  m._trig = 0;
  m._type = state_machine::SIMPLE;
  m._start->reset();
  return true;
}

bool transition_map::check_overlapping(int &examine)
{
  FUNCDEF("check_overlapping");
  examine = 0;
  for (int i = 0; i < states(); i++) {
    examine = i;
    for (int j = 0; j < (*_state_list)[i].transitions.length(); j++) {
      transition_info &a = (*_state_list)[i].transitions[j];
      if (a.type != transition_info::RANGE) continue;
      for (int k = j + 1; k < (*_state_list)[i].transitions.length(); k++) {
        transition_info &b = (*_state_list)[i].transitions[k];
        if (b.type != transition_info::RANGE) continue;
        if ( (b.low_trigger <= a.low_trigger)
              && (b.high_trigger >= a.low_trigger) ) {
          LOG(astring("intersecting range on low end!"));
          return false;
        }
        if ( (b.low_trigger <= a.high_trigger)
              && (b.high_trigger >= a.high_trigger) ) {
          LOG(astring("intersecting range on high end!"));
          return false;
        }
      }
    }
  }
  return true;
}

bool transition_map::check_reachability(int &examine)
{
  FUNCDEF("check_reachability");
  examine = 0;

  // list_to_add: the list of states that are definitely reachable and that
  // need to be considered.
  int_array list_to_add;
  list_to_add += _start_state;

  // already_checked: those states that have already been completely considered
  // as to their reachability.
  array<bool> already_checked(states());
  for (int i = 0; i < already_checked.length(); i++)
    already_checked[i] = false;

  while (list_to_add.length()) {
    // the next state that IS reachable has all of the states reachable from
    // it added to the list of states that are to be checked...
    examine = list_to_add[0];
    int indy = state_index(examine);
    if (negative(indy)) {
      LOG(a_sprintf("bad state index for state %d!", examine));
      return false;
    }
#ifdef DEBUG_STATE_MACHINE
    LOG(a_sprintf("state to add is %d, list size=%d.", examine,
        list_to_add.length()));
#endif
    // delete the item that we are now checking.
    list_to_add.zap(0, 0);

    // check whether this item has already had its kids (those states reachable
    // from it) added to the list to add.  if so, skip it.
    if (already_checked[indy]) continue;

    // update the information for this state we are considering in the list of
    // already checked states.
    already_checked[indy] = true;

    state_info &found = (*_state_list)[indy];

    // all states this one can reach are added to the list to check.
    for (int i = 0; i < found.transitions.length(); i++) {
#ifdef DEBUG_STATE_MACHINE
      LOG(astring("checking transition %d: ", i));
#endif
      int now_reachable = found.transitions[i].next_state;
#ifdef DEBUG_STATE_MACHINE
      LOG(astring("now reaching %d.", now_reachable));
#endif
      if (now_reachable == examine) continue;
      else {
        int indy = state_index(now_reachable);
        if (already_checked[indy]) continue;
      }
      list_to_add += now_reachable;
    }
  }
#ifdef DEBUG_STATE_MACHINE
  LOG("done checking reachability.");
#endif
  for (int j = 0; j < already_checked.length(); j++)
    if (!already_checked.get(j)) {
      examine = (*_state_list)[j].state_id;
      LOG(a_sprintf("state %d is not reachable", examine));
      return false;
    }
  return true;  // all states are reachable.
}

} //namespace.

