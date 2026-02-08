/*****************************************************************************\
*                                                                             *
*  Name   : test_ini_configurator                                             *
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

const int test_iterations = 10;

//#define DEBUG_INI_CONFIGURATOR_TEST
  // uncomment for debugging version.

#include <application/hoople_main.h>
#include <basis/functions.h>
#include <basis/guards.h>
#include <basis/astring.h>
#include <geometric/rectangle.h>
#include <geometric/screen_rectangle.h>
#include <mathematics/double_plus.h>
#include <loggers/console_logger.h>
#include <configuration/ini_configurator.h>
#include <configuration/application_configuration.h>
#include <structures/static_memory_gremlin.h>
#include <textual/byte_formatter.h>

#ifdef DEBUG_INI_CONFIGURATOR_TEST
  #include <stdio.h>
#endif

#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), s)

#define UNWANTED_DEFAULT_VALUE 9949494.3

HOOPLE_STARTUP_CODE;

using namespace basis;
using namespace configuration;
using namespace geometric;
using namespace loggers;
using namespace mathematics;
using namespace textual;

typedef double_plus frunkle;

#define WHERE __WHERE__.s()

const char *INI_SECTION = "t_ini_configurator";

//hmmm: ugly old main() without using the hoople machinery.  ack.
astring static_class_name() { return "test_ini_configurator"; }

#define MACRO_AS_STRING(s) #s

int main(int formal(argc), char *formal(argv)[])
{
  FUNCDEF("test ini config main")

  ini_configurator ini("t_ini_configurator.ini", ini_configurator::AUTO_STORE);

LOG(astring("ini file resides in: ") + ini.name());

LOG(astring("exe directory is currently: ") + application_configuration::application_directory());

  for (int i = 0; i < test_iterations; i++) {  // outer loop bracket.
    // beginning of test sets.
    {
      // first test set.
      const char *TEST_NAME = "first test: rectangle";
      // this tests whether rectangle storage works.
      screen_rectangle default_rectangle(10, 289, 388, 191);
      ini.delete_entry(INI_SECTION, "window_size");
      screen_rectangle win_size;
      astring tmp = ini.load(INI_SECTION, "window_size",
          default_rectangle.text_form());
      win_size.from_text(tmp);
      if (win_size != default_rectangle)
        deadly_error(INI_SECTION, TEST_NAME,
              "rectangle not equal to default");
      screen_rectangle new_size(23, 49, 129, 93);
      ini.store(INI_SECTION, "window_size", new_size.text_form());
      screen_rectangle current_size;
      tmp = ini.load(INI_SECTION, "window_size",
          default_rectangle.text_form());
      current_size.from_text(tmp);
      if (current_size != new_size)
        deadly_error(INI_SECTION, TEST_NAME,
              "rectangle not equal to second size stored");
    }
    {
      // second test set.
      const char *TEST_NAME = "second test: string";
      astring junk("this is a junky string to be stored as bytes....");
      byte_array to_store(junk.length() + 1, (abyte *)junk.observe());
      astring as_bytes;
      byte_formatter::bytes_to_string(to_store, as_bytes);
      ini.store("test_of_byte_store", "test1", as_bytes);
      astring blort = "blort_fest!";
      astring rettle = ini.load("test_of_byte_store", "test1", blort);
      byte_array found_byte;
      byte_formatter::string_to_bytes(rettle, found_byte);
      astring found_junk((const char *)found_byte.observe());
      if (rettle == blort)
        deadly_error(INI_SECTION, TEST_NAME,
           "ini_configurator load failed: default was used");
      else if (found_junk != junk)
        deadly_error(INI_SECTION, TEST_NAME,
           "ini_configurator load failed: result differed from original");
    }
    {
      // third test set.
      const char *TEST_NAME = "third test: frunkle";
      frunkle def_frunkle(3.14159265358);
      astring def_text(astring::SPRINTF, "%f", def_frunkle.value());
      ini.store(INI_SECTION, TEST_NAME, def_text);
      astring found_string = ini.load(INI_SECTION, TEST_NAME, MACRO_AS_STRING(UNWANTED_DEFAULT_VALUE));
      frunkle found_frunkle = found_string.convert(0.0);
      if (found_frunkle == frunkle(UNWANTED_DEFAULT_VALUE))
        deadly_error(INI_SECTION, TEST_NAME, 
           "ini_configurator load failed: default was used");
      if (found_frunkle != def_frunkle)
        deadly_error(INI_SECTION, TEST_NAME, 
           "ini_configurator load failed: saved value differed");
    }
    {
      // fourth test set.
      const char *TEST_NAME = "fourth test: frunkle";
      frunkle def_frunkle(1487335673.1415926535834985987);
      astring def_text(astring::SPRINTF, "%f", def_frunkle.value());	  
      ini.store("test", "frunkle_test", def_text);
      astring found_string = ini.load("test", "frunkle_test", MACRO_AS_STRING(UNWANTED_DEFAULT_VALUE));
      frunkle found_frunkle = found_string.convert(0.0);
      if (found_frunkle == frunkle(0.0))
        deadly_error(INI_SECTION, TEST_NAME,
           "ini_configurator load failed: float conversion failed--got zero");
      if (found_frunkle == frunkle(UNWANTED_DEFAULT_VALUE))
        deadly_error(INI_SECTION, TEST_NAME,
           "ini_configurator load failed: wrong unwanted default was used");
      if (found_frunkle != def_frunkle)
        deadly_error(INI_SECTION, TEST_NAME, 
           "ini_configurator load failed: saved value differed");
    }
    {
      // fifth test set.
      const char *TEST_NAME = "fifth test: bytes";
      astring urp("urp");
      astring junk("this is a junky string to be stored as bytes....");
      byte_array default_bytes(urp.length() + 1, (abyte *)urp.observe());
      astring defbytes_string;
      byte_formatter::bytes_to_string(default_bytes, defbytes_string);
      byte_array found;
      astring tmp = ini.load("test_of_byte_store", "test1", defbytes_string);
      byte_formatter::string_to_bytes(tmp, found);
      astring string_found = (char *)found.observe();
      if (string_found == urp)
        deadly_error(INI_SECTION, TEST_NAME,
            "ini_configurator load_bytes failed: default was used");
      if (string_found.length() != junk.length())
        deadly_error(INI_SECTION, TEST_NAME,
            "ini_configurator load_bytes failed: array lengths differ");
      if (string_found != junk)
        deadly_error(INI_SECTION, TEST_NAME,
            "ini_configurator load_bytes failed: arrays differ");
    }
    {
      // sixth test set.
      const char *TEST_NAME = "sixth test: blank string";
      ini.delete_entry("test_of_blank_string", "test1");
      astring blank("");
      astring fund = ini.load("test_of_blank_string", "test1", blank);
      if (fund != blank)
        deadly_error(INI_SECTION, TEST_NAME, "ini_configurator load string "
               "with blank default failed: didn't return blank");
      ini.delete_entry("test_of_blank_string", "test1");
      astring non_blank("blinkblankblunk");
      fund = ini.load("test_of_blank_string", "test1", non_blank);
      if (fund != non_blank)
        deadly_error(INI_SECTION, TEST_NAME, "ini_configurator load string "
               "with non-blank default failed: didn't return default");
      fund = ini.load("test_of_blank_string", "test1", blank);
      if (fund != non_blank)
        deadly_error(INI_SECTION, TEST_NAME, "ini_configurator load string "
               "with blank default failed: returned wrong string");
    }
  }

  critical_events::alert_message(astring(static_class_name()) + ": works for those functions tested.");

  return 0;
}

