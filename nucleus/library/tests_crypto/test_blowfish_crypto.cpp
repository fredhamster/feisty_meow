/*
*  Name   : test blowfish encryption
*  Author : Chris Koeritz
*  Purpose: Exercises the BlowFish encryption methods in the crypto library.
**
* Copyright (c) 2005-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
*/

#include <application/hoople_main.h>
#include <basis/byte_array.h>
#include <basis/astring.h>
#include <crypto/blowfish_crypto.h>
#include <mathematics/chaos.h>
#include <processes/ethread.h>
#include <processes/thread_cabinet.h>
#include <structures/static_memory_gremlin.h>
#include <structures/unique_id.h>
#include <textual/byte_formatter.h>
#include <textual/string_manipulation.h>
#include <timely/time_control.h>
#include <timely/time_stamp.h>
#include <unit_test/unit_base.h>

#include <stdio.h>
#include <string.h>

using namespace application;
using namespace basis;
using namespace crypto;
using namespace filesystem;
using namespace loggers;
using namespace mathematics;
using namespace processes;
using namespace structures;
using namespace textual;
using namespace timely;
using namespace unit_test;

#define LOG(to_print) EMERGENCY_LOG(program_wide_logger::get(), to_print)

//#define DEBUG_BLOWFISH
  // uncomment for noisier run.

const int TEST_RUNS_PER_KEY = 5;  // encryption test cycles done on each key.

const int THREAD_COUNT = 10;  // number of threads testing blowfish at once.

const int ITERATIONS = 4;  // number of test runs in our testing threads.

const int MAX_STRING = 20000;  // largest chunk that we'll try to encrypt.

//////////////

class test_blowfish;  // forward.

class blowfish_thread : public ethread
{
public:
  blowfish_thread(test_blowfish &parent) : ethread(), _parent(parent) {}

  void perform_activity(void *ptr);
    // try out random blowfish keys on randomly chosen chunks of the fodder.

private:
  test_blowfish &_parent;
};

//////////////

class test_blowfish : virtual public unit_base, virtual public application_shell
{
public:
  test_blowfish()
    : _fodder(string_manipulation::make_random_name(MAX_STRING + 1, MAX_STRING + 1)) {}
  DEFINE_CLASS_NAME("test_blowfish");

  int execute();

private:
  astring _fodder;  // chunks taken from this are encrypted and decrypted.
  time_stamp _program_start;  // the time at which we started executing.
  thread_cabinet _threads;  // manages our testing threads.
  friend class blowfish_thread;  // bad practice, but saves time in test app.
};

int test_blowfish::execute()
{
  FUNCDEF("execute");
#ifdef DEBUG_BLOWFISH
  LOG(astring("starting blowfish test..."));
#endif
  int left = THREAD_COUNT;
  while (left--) {
#ifdef DEBUG_BLOWFISH
  LOG(a_sprintf("blowfish thread %d starting...", left));
#endif
    _threads.add_thread(new blowfish_thread(*this), true, NIL);
  }

#ifdef DEBUG_BLOWFISH
  LOG(astring("started all threads..."));
#endif

  while (_threads.threads()) {
#ifdef DEBUG_BLOWFISH
    LOG(astring("cleaning debris."));
#endif
    _threads.clean_debris();
    time_control::sleep_ms(1000);
  }

#ifdef DEBUG_BLOWFISH
  int duration = int(time_stamp().value() - _program_start.value());
  LOG(a_sprintf("duration for %d keys and encrypt/decrypt=%d ms,",
      ITERATIONS * TEST_RUNS_PER_KEY * THREAD_COUNT, duration));
  LOG(a_sprintf("that comes to %d ms per cycle.\n", int(double(duration
      / TEST_RUNS_PER_KEY / ITERATIONS / THREAD_COUNT))));
#endif

  return final_report();
}

//////////////

#undef UNIT_BASE_THIS_OBJECT 
#define UNIT_BASE_THIS_OBJECT (*dynamic_cast<unit_base *>(application_shell::single_instance()))

void blowfish_thread::perform_activity(void *)
{
  FUNCDEF("perform_activity");
  int left = ITERATIONS;
  while (left--) {
    time_stamp key_start;
    blowfish_crypto bc(_parent.randomizer().inclusive
        (blowfish_crypto::minimum_key_size(),
         blowfish_crypto::maximum_key_size()));
#ifdef DEBUG_BLOWFISH
    LOG(a_sprintf("%d bit key has:", bc.key_size()));
    astring dumped_key = byte_formatter::text_dump(bc.get_key());
    LOG(a_sprintf("%s", dumped_key.s()));
#endif
    int key_dur = int(time_stamp().value() - key_start.value());

#ifdef DEBUG_BLOWFISH
    LOG(a_sprintf("  key generation took %d ms", key_dur));
#endif

    for (int i = 0; i < TEST_RUNS_PER_KEY; i++) {
      byte_array key;
      byte_array iv;

      int string_start = _parent.randomizer().inclusive(0, MAX_STRING - 1);
      int string_end = _parent.randomizer().inclusive(0, MAX_STRING - 1);
      flip_increasing(string_start, string_end);
      astring ranstring = _parent._fodder.substring(string_start, string_end);
//LOG(a_sprintf("encoding %s\n", ranstring.s());
//LOG(a_sprintf("string length encoded: %d\n", ranstring.length());

      byte_array target;

      time_stamp test_start;
      bool worked = bc.encrypt(byte_array(ranstring.length() + 1,
          (abyte*)ranstring.s()), target);
      int enc_durat = int(time_stamp().value() - test_start.value());
      ASSERT_TRUE(worked, "phase 1 should not fail to encrypt the string");

      byte_array recovered;
      test_start.reset();
      worked = bc.decrypt(target, recovered);
      int dec_durat = int(time_stamp().value() - test_start.value());
      ASSERT_TRUE(worked, "phase 1 should not fail to decrypt the string");
//    LOG(a_sprintf("original has %d chars, recovered has %d chars\n",
//        ranstring.length(), recovered.length() - 1));

      astring teddro = (char *)recovered.observe();
//LOG(a_sprintf("decoded %s\n", teddro.s()));

#ifdef DEBUG_BLOWFISH
      if (teddro != ranstring) {
        LOG(a_sprintf("error!\toriginal has %d chars, recovered has %d chars\n",
            ranstring.length(), recovered.length() - 1));
        LOG(a_sprintf("\tencoded %s\n", ranstring.s()));
        LOG(a_sprintf("\tdecoded %s\n", teddro.s()));
      }
#endif
      ASSERT_EQUAL(teddro, ranstring, "should not fail to regenerate the original string");

#ifdef DEBUG_BLOWFISH
      LOG(a_sprintf("  encrypt %d ms, decrypt %d ms, data %d bytes\n",
           enc_durat, dec_durat, string_end - string_start + 1));
#endif
      time_control::sleep_ms(0);  // take a rest.
    }
    time_control::sleep_ms(0);  // take a rest.
  }
}

HOOPLE_MAIN(test_blowfish, )

