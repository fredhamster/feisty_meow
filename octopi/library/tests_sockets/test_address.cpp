/*
*  Name   : test_address
*  Author : Chris Koeritz
*  Purpose: Tests some attributes of the network_address class for correctness.
**
* Copyright (c) 2001-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
*/

#include <application/hoople_main.h>
#include <basis/byte_array.h>
#include <basis/astring.h>
#include <loggers/program_wide_logger.h>
#include <structures/static_memory_gremlin.h>
#include <sockets/internet_address.h>
#include <unit_test/unit_base.h>

using namespace application;
using namespace basis;
//using namespace filesystem;
using namespace loggers;
using namespace mathematics;
using namespace sockets;
using namespace structures;
using namespace textual;
using namespace timely;
using namespace unit_test;

//#include <stdio.h>
//#include <string.h>

#define LOG(to_print) EMERGENCY_LOG(program_wide_logger().get(), astring(to_print))

class test_address : public virtual unit_base, virtual public application_shell
{
public:
  test_address() {}
  DEFINE_CLASS_NAME("test_address");
  virtual int execute();
};

int test_address::execute()
{
  FUNCDEF("execute");
  // testing is_valid_internet_address here...

  bool all_zero = false;
  byte_array ip_form;
  astring to_test = "12.5.55.37";
  if (!internet_address::is_valid_internet_address(to_test, ip_form, all_zero))
    deadly_error(class_name(), "1st address",
        "failed to say address was valid");
  if (all_zero)
    deadly_error(class_name(), "1st address", "said address was all zeros");
  if ( (ip_form[0] != 12) || (ip_form[1] != 5) || (ip_form[2] != 55)
      || (ip_form[3] != 37) )
    deadly_error(class_name(), "1st address", "address had incorrect contents");

  to_test = "12.5.55.372";
  if (internet_address::is_valid_internet_address(to_test, ip_form, all_zero))
    deadly_error(class_name(), "2nd address", "failed to say address was invalid");

  to_test = "12.5.55.37.3";
  if (internet_address::is_valid_internet_address(to_test, ip_form, all_zero))
    deadly_error(class_name(), "3rd address", "failed to say address was invalid");

  to_test = "12.5.55";
  if (internet_address::is_valid_internet_address(to_test, ip_form, all_zero))
    deadly_error(class_name(), "4th address", "failed to say address was invalid");

  to_test = "0.0.0.0";
  if (!internet_address::is_valid_internet_address(to_test, ip_form, all_zero))
    deadly_error(class_name(), "5th address", "failed to say address was valid");
  if (!all_zero)
    deadly_error(class_name(), "5th address", "said address was not all zeros");
  if ( (ip_form[0] != 0) || (ip_form[1] != 0) || (ip_form[2] != 0)
      || (ip_form[3] != 0) )
    deadly_error(class_name(), "5th address", "address had incorrect contents");

  to_test = "0.0.0.1";
  if (!internet_address::is_valid_internet_address(to_test, ip_form, all_zero))
    deadly_error(class_name(), "6th address", "failed to say address was valid");
  if (all_zero)
    deadly_error(class_name(), "6th address", "said address was all zeros");
  if ( (ip_form[0] != 0) || (ip_form[1] != 0) || (ip_form[2] != 0)
      || (ip_form[3] != 1) )
    deadly_error(class_name(), "6th address", "address had incorrect contents");

  to_test = "0.0.0.";
  if (internet_address::is_valid_internet_address(to_test, ip_form, all_zero))
    deadly_error(class_name(), "7th address", "failed to say address was invalid");

  to_test = "0.0.0";
  if (internet_address::is_valid_internet_address(to_test, ip_form, all_zero))
    deadly_error(class_name(), "7th address", "failed to say address was invalid");

  to_test = "this may have. an ip address in it somewhere... 92.21. 23.123. 1235.6.3  9 oops  hey where is it.  23.51.2 2.4 1.2.343 09023.2.3. marbles 23.15.123.5 manus kobble 23.1.5.2 sturp nort ation";
  astring found;
  if (!internet_address::has_ip_address(to_test, found))
    deadly_error(class_name(), "8th address", "failed to find ip address");
  ASSERT_EQUAL(found, astring("23.15.123.5"), "8th address: ip address found should be correct");

  to_test = "furples 92.23.1 9123.5.3 12398. 23 11 1 1 0202 2.4.1.5";
  if (!internet_address::has_ip_address(to_test, found))
    deadly_error(class_name(), "9th address", "failed to find ip address");
  if (found != astring("2.4.1.5"))
    deadly_error(class_name(), "9th address", astring("ip address found was wrong: ") + found + ", should have been 2.4.1.5");

  to_test = "12.5.55.2\"";
  if (!internet_address::has_ip_address(to_test, found))
    deadly_error(class_name(), "10th address", "failed to find ip address");
  if (found != astring("12.5.55.2"))
    deadly_error(class_name(), "10th address", astring("ip address found was wrong: ") + found + ", should have been 12.5.55.2");

  to_test = "12.5.55.2";
  if (!internet_address::has_ip_address(to_test, found))
    deadly_error(class_name(), "11th address", "failed to find ip address");
  if (found != astring("12.5.55.2"))
    deadly_error(class_name(), "11th address", astring("ip address found was wrong: ") + found + ", should have been 12.5.55.2");

  return final_report();
}

HOOPLE_MAIN(test_address, )

