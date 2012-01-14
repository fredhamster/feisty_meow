/*****************************************************************************\
*                                                                             *
*  Name   : thread_cabinet                                                    *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*******************************************************************************
* Copyright (c) 2000-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "ethread.h"
#include "thread_cabinet.h"

#include <loggers/critical_events.h>
#include <structures/amorph.h>
#include <structures/unique_id.h>
#include <timely/time_control.h>

#undef LOCKIT
#define LOCKIT auto_synchronizer l(*_lock)
  // grabs the mutex for access to the list.

#undef LOG
#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)

//#define DEBUG_THREAD_CABINET
  // uncomment for noisier version.

using namespace basis;
using namespace loggers;
using namespace structures;
using namespace timely;

namespace processes {

class thread_record
{
public:
  ethread *_thread;
  unique_int _id;

  thread_record(const unique_int &id, ethread *t)
      : _thread(t), _id(id) {}

  ~thread_record() {
    _thread->stop();
    WHACK(_thread);
  }
};

//////////////

class thread_amorph : public amorph<thread_record>
{
public:
};

//////////////

thread_cabinet::thread_cabinet()
: _lock(new mutex),
  _threads(new thread_amorph),
  _next_id(new int_roller(1, MAXINT32 - 1)),
  _no_additions(0)
{
}

thread_cabinet::~thread_cabinet()
{
  WHACK(_threads);
  WHACK(_lock);
  WHACK(_next_id);
}

int thread_cabinet::threads() const { return _threads->elements(); }

unique_int thread_cabinet::add_thread(ethread *to_add, bool start_it,
    void *parm)
{
#ifdef DEBUG_THREAD_CABINET
  FUNCDEF("add_thread");
#endif
  LOCKIT;
  if (_no_additions) {
#ifdef DEBUG_THREAD_CABINET
    LOG("no additions flag is true; destroying the thread and failing out.");
#endif
    // can't just leave it unhooked hanging out in space...
    WHACK(to_add);
    return 0;
  }
  int use_id = _next_id->next_id();
  if (start_it) {
    to_add->start(parm);
  } else {
#ifdef DEBUG_THREAD_CABINET
    if (to_add->thread_finished())
      LOG(a_sprintf("thread %x is not going to be started and it "
          "hasn't started yet!", to_add));
#endif
  }
  _threads->append(new thread_record(use_id, to_add));
  return use_id;
}

bool thread_cabinet::any_running() const
{
  LOCKIT;
  for (int i = 0; i < _threads->elements(); i++) {
    if (_threads->borrow(i)->_thread->thread_started()) return true;
  }
  return false;
}

void thread_cabinet::start_all(void *ptr)
{
  LOCKIT;
  for (int i = 0; i < _threads->elements(); i++) {
    if (_threads->borrow(i)->_thread->thread_finished()) {
      _threads->borrow(i)->_thread->start(ptr);
    }
  }
}

void thread_cabinet::cancel_all()
{
  FUNCDEF("cancel_all");
  {
    LOCKIT;  // short lock.
    _no_additions++;  // keep people from adding new threads.
    for (int i = 0; i < _threads->elements(); i++) {
      _threads->borrow(i)->_thread->cancel();
    }
  }
  LOCKIT;
  _no_additions--;  // allow new threads again.
  if (_no_additions < 0)
    continuable_error(class_name(), func, "error in logic regarding "
        "no additions.");
}

void thread_cabinet::stop_all()
{
  FUNCDEF("stop_all");
  {
    LOCKIT;  // short lock.
    _no_additions++;  // keep people from adding new threads.
  }
  cancel_all();  // signal all threads to leave.
  // pause to give them a little while to leave.
  time_control::sleep_ms(20);
  while (true) {
    LOCKIT;  // short lock.
    if (!_threads->elements()) break;  // done; nothing left.
    clean_debris();  // remove any that did stop.
    time_control::sleep_ms(20);  // snooze for a short while.
  }
  LOCKIT;
  _no_additions--;  // allow new threads again.
  if (_no_additions < 0)
    continuable_error(class_name(), func, "error in logic regarding "
        "no additions.");
}

bool thread_cabinet::zap_thread(const unique_int &to_whack)
{
  LOCKIT;
  for (int i = 0; i < _threads->elements(); i++) {
    if (_threads->borrow(i)->_id == to_whack) {
      // this is the one they want zapped.
      _threads->zap(i, i);
      return true;
    }
  }
  return false;
}

bool thread_cabinet::cancel_thread(const unique_int &to_cancel)
{
  LOCKIT;
  for (int i = 0; i < _threads->elements(); i++) {
    if (_threads->borrow(i)->_id == to_cancel) {
      // this is the one to signal regarding its own demise.
      _threads->borrow(i)->_thread->cancel();
      return true;
    }
  }
  return false;
}

void thread_cabinet::clean_debris()
{
#ifdef DEBUG_THREAD_CABINET
  FUNCDEF("clean_debris");
#endif
  LOCKIT;
  for (int i = 0; i < _threads->elements(); i++) {
    if (_threads->borrow(i)->_thread->thread_finished()) {
      // this one's no longer doing anything.
#ifdef DEBUG_THREAD_CABINET
      LOG(a_sprintf("clearing thread %x out.", _threads->borrow(i)->_thread));
#endif
      _threads->zap(i, i);
      i--;  // skip back before the whack.
    }
  }
}

int_set thread_cabinet::thread_ids() const
{
  LOCKIT;
  int_set to_return;
  for (int i = 0; i < _threads->elements(); i++)
    to_return += _threads->borrow(i)->_id.raw_id();
  return to_return;
}

ethread *thread_cabinet::get_thread(int index)
{
  LOCKIT;
  thread_record *rec = _threads->borrow(index);
  if (rec) return rec->_thread;
  return NIL;
}

} //namespace.

