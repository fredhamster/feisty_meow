/*
*  Name   : test RSA public key encryption
*  Author : Chris Koeritz
*  Purpose:
*    Exercises the RSA encryption functions from the crypto library.
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
#include <crypto/rsa_crypto.h>
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

//#define DEBUG_RSA_CRYPTO
  // uncomment for noisy run.

#define LOG(to_print) EMERGENCY_LOG(program_wide_logger::get(), to_print)

const int KEY_SIZE = 1024;
  // the size of the RSA key that we'll create.

const int MAX_STRING = 4000;
  // the largest chunk that we'll try to encrypt.

const int THREAD_COUNT = 5;  // number of threads testing rsa at once.

const int ITERATIONS = 6;  // number of test runs in our testing threads.

//////////////

class test_rsa;  // forward.

class rsa_thread : public ethread
{
public:
  rsa_thread(test_rsa &parent) : ethread(), _parent(parent) {}

  void perform_activity(void *ptr);
    // try out random rsa keys on randomly chosen chunks of the fodder.

private:
  test_rsa &_parent;
};

//////////////

class test_rsa : public virtual unit_base, virtual public application_shell
{
public:
  test_rsa()
      : _fodder(string_manipulation::make_random_name(MAX_STRING + 1, MAX_STRING + 1)) {}
  virtual ~test_rsa() {}
  DEFINE_CLASS_NAME("test_rsa");

  const astring &fodder() const { return _fodder; }

  int execute();

private:
  astring _fodder;  // chunks taken from this are encrypted and decrypted.
  time_stamp _program_start;  // the time at which we started executing.
  thread_cabinet _threads;  // manages our testing threads.
  friend class rsa_thread;  // bad practice, but saves time in test app.
};

int test_rsa::execute()
{
  FUNCDEF("execute");
  int left = THREAD_COUNT;
  while (left--) {
    _threads.add_thread(new rsa_thread(*this), true, NIL);
  }

  while (_threads.threads()) {
#ifdef DEBUG_RSA_CRYPTO
    LOG(astring("cleaning debris."));
#endif
    _threads.clean_debris();
    time_control::sleep_ms(1000);
  }

#ifdef DEBUG_RSA_CRYPTO
  int duration = int(time_stamp().value() - _program_start.value());
  LOG(a_sprintf("duration for %d keys and encrypt/decrypt=%d ms,",
      ITERATIONS * THREAD_COUNT, duration));
  LOG(a_sprintf("that comes to %d ms per cycle.", int(double(duration
      / ITERATIONS / THREAD_COUNT))));
#endif

  return final_report();
}

//////////////

#undef UNIT_BASE_THIS_OBJECT 
#define UNIT_BASE_THIS_OBJECT (*dynamic_cast<unit_base *>(application_shell::single_instance()))

void rsa_thread::perform_activity(void *)
{
  FUNCDEF("perform_activity");
  int left = ITERATIONS;
  while (left--) {
    time_stamp start;

    rsa_crypto rc_private_here(KEY_SIZE);
    int key_durat = int(time_stamp().value() - start.value());

    byte_array public_key;
    rc_private_here.public_key(public_key);  // get our public portion.
    byte_array private_key;
    rc_private_here.private_key(private_key);  // get our private portion.

//RSA_print_fp(stdout, private_key, 0);
//RSA_print_fp(stdout, public_key, 0);

    int string_start = _parent.randomizer().inclusive(0, MAX_STRING);
    int string_end = _parent.randomizer().inclusive(0, MAX_STRING);
    flip_increasing(string_start, string_end);
    astring ranstring = _parent.fodder().substring(string_start, string_end);
    byte_array target;

    // the first phase tests the outsiders sending back data that only we,
    // with our private key, can decrypt.

    start.reset();
    rsa_crypto rc_pub(public_key);
    bool worked = rc_pub.public_encrypt(byte_array(ranstring.length() + 1,
        (abyte*)ranstring.s()), target);
    int pub_enc_durat = int(time_stamp().value() - start.value());
    ASSERT_TRUE(worked, "phase 1 shouldn't fail to encrypt the string");

    rsa_crypto rc_priv(private_key);
    byte_array recovered;
    start.reset();
    worked = rc_priv.private_decrypt(target, recovered);
    int priv_dec_durat = int(time_stamp().value() - start.value());
    ASSERT_TRUE(worked, "phase 1 should not fail to decrypt the string");

    astring teddro = (char *)recovered.observe();

    ASSERT_EQUAL(teddro, ranstring, "should not fail to get back the data");

    // the second phase tests us using our private key to encrypt data which
    // anyone with the public key can decode.

    start.reset();
    worked = rc_priv.private_encrypt(byte_array(ranstring.length() + 1,
        (abyte*)ranstring.s()), target);
    int priv_enc_durat = int(time_stamp().value() - start.value());
    ASSERT_TRUE(worked, "phase 2 should not fail to encrypt the string");

    start.reset();
    worked = rc_pub.public_decrypt(target, recovered);
    int pub_dec_durat = int(time_stamp().value() - start.value());
    ASSERT_TRUE(worked, "phase 2 should not fail to decrypt the string");

    teddro = (char *)recovered.observe();

    ASSERT_EQUAL(teddro, ranstring, "should not fail to get back the data here either");

#ifdef DEBUG_RSA_CRYPTO
    LOG(a_sprintf("key generation: %d ms, public encrypt: %d ms, private "
        "decrypt: %d ms", key_durat, pub_enc_durat, priv_dec_durat));
    LOG(a_sprintf("data size: %d bytes, private encrypt: %d ms, public "
        "decrypt: %d ms",
        string_end - string_start + 1, priv_enc_durat, pub_dec_durat));
#endif

    time_control::sleep_ms(0);  // take a rest.
  }
}

HOOPLE_MAIN(test_rsa, )

