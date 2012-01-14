/*
*  Name   : test_tokenizer
*  Author : Chris Koeritz
*  Purpose: Puts the variable_tokenizer through some paces.
**
* Copyright (c) 1998-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
*/

#include <application/hoople_main.h>
#include <basis/astring.h>
#include <basis/byte_array.h>
#include <basis/functions.h>
#include <basis/guards.h>
#include <configuration/variable_tokenizer.h>
#include <filesystem/byte_filer.h>
#include <structures/static_memory_gremlin.h>
#include <structures/string_table.h>
#include <textual/parser_bits.h>
#include <unit_test/unit_base.h>

using namespace application;
using namespace basis;
using namespace configuration;
using namespace filesystem;
using namespace loggers;
using namespace mathematics;
using namespace structures;
using namespace textual;
using namespace timely;
using namespace unit_test;

#define LOG(to_print) EMERGENCY_LOG(program_wide_logger::get(), astring(to_print))

const int MAX_LINE_SIZE = 1000;
  // the largest line we will deal with in a file.

class test_tokenizer : public virtual unit_base, public virtual application_shell
{
public:
  test_tokenizer() {}
  DEFINE_CLASS_NAME("test_tokenizer");
  virtual int execute();
};

//////////////

int test_tokenizer::execute()
{
  FUNCDEF("execute");
  {
    astring test_set_1 = "\n\
[frederick]\n\
samba=dance\n\
tantalus rex=gumby\n\
57 chevy heap=\"16 anagrams of misty immediately\"\n\
lingus distractus\n\
shouldus havus assignmentum=\n\
above better be parsed = 1\n\
;and this comment too yo\n\
ted=agent 12\n";

    astring TEST = "First Test: ";
    astring testing = test_set_1;
    LOG(astring("file before parsing:") + testing);
    variable_tokenizer jed("\n\r", "=");
    ASSERT_TRUE(jed.parse(testing), TEST + "jed should be parseable");
    astring out;
    jed.text_form(out);
    variable_tokenizer gorp("\n\r", "=");
    ASSERT_TRUE(gorp.parse(out), TEST + "gorp should be parseable");
    LOG(astring("file after parsing:") + out);
    LOG("and in tabular form:");
    LOG(jed.table().text_form());

//for (int i = 0; i < gorp.table().symbols(); i++) {
//astring name, value;
//gorp.table().retrieve(i, name, value);
//LOG(a_sprintf("item %d: name=\"%s\" value=\"%s\"", i, name.s(), value.s()));
//}

    ASSERT_TRUE(jed.exists("[frederick]"), TEST + "jed section header was omitted!");
    ASSERT_EQUAL(jed.find("[frederick]"), astring(""),
        TEST + "jed section header had unexpected contents!");
    ASSERT_EQUAL(jed.find("ted"), astring("agent 12"),
        TEST + "jed's ted is missing or invalid!");
    ASSERT_FALSE(jed.find("shouldus havus assignmentum").t(),
        TEST + "jed's shouldus had contents but shouldn't!");
    astring value = *jed.table().find("shouldus havus assignmentum");
    ASSERT_EQUAL(value, astring(" "), TEST + "jed shouldus had wrong contents, not special!");
    ASSERT_TRUE(gorp.exists("[frederick]"), TEST + "gorp section header was omitted!");
    ASSERT_EQUAL(gorp.find("[frederick]"), astring(""),
        TEST + "gorp section header had unexpected contents!");
    ASSERT_EQUAL(gorp.find("ted"), astring("agent 12"),
        TEST + "gorp's ted is missing or invalid!");
    ASSERT_FALSE(gorp.find("shouldus havus assignmentum").t(),
        TEST + "gorp's shouldus had contents but shouldn't!");
    value = *gorp.table().find("shouldus havus assignmentum");
    ASSERT_EQUAL(value, astring(" "), TEST + "gorp shouldus had wrong contents, not special!");
  }    
  {
    astring test_set_2 = "Name=SRV,   Parent=,        Persist=Y,  Entry=Y, Required=Y, Desc=Server,               Tbl=Server";

    astring TEST = "Second Test: ";
    astring testing = test_set_2;
    LOG(astring("file before parsing:") + testing);
    variable_tokenizer jed(",", "=");
    ASSERT_TRUE(jed.parse(testing), TEST + "jed should be parseable");
    astring out;
    jed.text_form(out);
    LOG(astring("file after parsing:") + out);
    LOG("and in tabular form:");
    LOG(jed.table().text_form());
    ASSERT_EQUAL(jed.find("Name"), astring("SRV"), TEST + "Name is missing or invalid!");
    ASSERT_FALSE(jed.find("Parent").t(), TEST + "Parent had contents but shouldn't!");
    astring value = *jed.table().find("Parent");
    ASSERT_EQUAL(value, astring(" "), TEST + "Parent had wrong contents, not special!");
    ASSERT_EQUAL(jed.find("Persist"), astring("Y"), TEST + "Persist is missing or invalid!");
  }    

  {
    astring test_set_3 = "\n\
[frederick]\n\
samba=dance\n\
tantalus rex=gumby \"don#t\n\n'play'\nthat\" homey '\n\ndog\n\n yo \"\ncreen\" arf'\n\
57 chevy heap=\"16 anagrams of misty immediately\"\n\
lingus distractus\n\
shouldus havus assignmentum=\n\
above better be parsed = 1\n\
;and this comment too yo\n\
ted=agent 12\n";

    astring TEST = "Third Test: ";
    astring testing = test_set_3;
    LOG(astring("file before parsing:") + testing);
    variable_tokenizer jed("\n\r", "=", "\'\"");
    ASSERT_TRUE(jed.parse(testing), TEST + "jed should be parseable");
    astring out;
    jed.text_form(out);
    variable_tokenizer gorp("\n\r", "=", "\'\"");
    ASSERT_TRUE(gorp.parse(out), TEST + "gorp should be parseable");
    LOG(astring("file after parsing:") + out);
    LOG("and in tabular form:");
    LOG(jed.table().text_form());
    ASSERT_TRUE(jed.exists("[frederick]"), TEST + "jed section header was omitted!");
    ASSERT_EQUAL(jed.find("[frederick]"), astring(""),
      TEST + "jed section header had unexpected contents!");
    ASSERT_EQUAL(jed.find("ted"), astring("agent 12"), TEST + "jed ted is missing or invalid!");
    ASSERT_FALSE(jed.find("shouldus havus assignmentum").t(),
      TEST + "jed shouldus had contents but shouldn't!");
    astring value = *jed.table().find("shouldus havus assignmentum");
    ASSERT_EQUAL(value, astring(" "), TEST + "jed shouldus had wrong contents, not special!");
    ASSERT_TRUE(gorp.exists("[frederick]"), TEST + "gorp section header was omitted!");
    ASSERT_EQUAL(gorp.find("[frederick]"), astring(""),
      TEST + "gorp section header had unexpected contents!");
    ASSERT_EQUAL(gorp.find("ted"), astring("agent 12"), TEST + "gorp second ted is missing or invalid!");
    ASSERT_FALSE(gorp.find("shouldus havus assignmentum").t(),
      TEST + "gorp shouldus had contents but shouldn't!");
    value = *gorp.table().find("shouldus havus assignmentum");
    ASSERT_EQUAL(value, astring(" "), TEST + "gorp shouldus wrong contents, was not special!");
    ASSERT_TRUE(gorp.exists("tantalus rex"), TEST + "gorp tantalus rex is missing!");
    ASSERT_EQUAL(gorp.find("tantalus rex"),
        astring("gumby \"don#t\n\n'play'\nthat\" homey '\n\ndog\n\n yo "
           "\"\ncreen\" arf'"),
      TEST + "gorp tantalus rex has incorrect contents!");
  }
  {
    astring test_set_4 = "\n\
[garfola]\n\
treadmill=\"this ain't the place\nwhere'n we been done\nseein' no quotes\"\n\
borfulate='similarly \"we\" do not like\nthe \" quote \" type thing here'\n\
";

    astring TEST = "Fourth Test: ";
    astring testing = test_set_4;
    LOG(astring("file before parsing:\n") + testing);
    variable_tokenizer jed("\n\r", "=", "\'\"", false);
    ASSERT_TRUE(jed.parse(testing), TEST + "jed should be parseable");
    astring out;
    jed.text_form(out);
    variable_tokenizer gorp("\n\r", "=", "\'\"", false);
    ASSERT_TRUE(gorp.parse(out), TEST + "gorp should be parseable");
    LOG(astring("file after parsing:\n") + out);
    LOG("and in tabular form:");
    LOG(jed.table().text_form());
    ASSERT_TRUE(gorp.exists("[garfola]"), TEST + "section header was omitted!");
    ASSERT_EQUAL(gorp.find("[garfola]"), astring(""),
        TEST + "section header had unexpected contents!");
    ASSERT_TRUE(gorp.exists("treadmill"), TEST + "treadmill is missing!");
    ASSERT_EQUAL(gorp.find("treadmill"),
        astring("\"this ain't the place\nwhere'n we been done\nseein' no quotes\""),
        TEST + "treadmill has incorrect contents!");
    ASSERT_TRUE(gorp.exists("borfulate"), TEST + "borfulate is missing!");
    ASSERT_EQUAL(gorp.find("borfulate"),
        astring("'similarly \"we\" do not like\nthe \" quote \" type thing here'"),
        TEST + "borfulate has incorrect contents!");
  }
  {
    astring test_set_5 = "\n\
 x~35; y~92   ;#comment   ; d   ~83 ;  e~   54   ; ? new comment  ;sud   ~  xj23-8 ; nigh ~2";

    astring TEST = "Fifth Test: ";
    astring testing = test_set_5;
    LOG(astring("file before parsing:\n") + testing);
    variable_tokenizer jed(";", "~");
    jed.set_comment_chars("#?");
    ASSERT_TRUE(jed.parse(testing), TEST + "jed should be parseable");
    astring out;
    jed.text_form(out);
    LOG(astring("file after parsing:\n") + out);
    LOG("and in tabular form:");
    LOG(jed.table().text_form());

    variable_tokenizer gorp(";", "~");
    gorp.set_comment_chars("#?");
    ASSERT_TRUE(gorp.parse(out), TEST + "gorp should be parseable");
    LOG("gorp in tabular form:");
    LOG(gorp.table().text_form());
//hmmm: need equalizable/orderable on table?
    ASSERT_TRUE(gorp.table() == jed.table(), TEST + "gorp text not same as jed!");

    ASSERT_EQUAL(jed.find("x"), astring("35"), TEST + "value for x missing or invalid");
    ASSERT_EQUAL(jed.find("y"), astring("92"), TEST + "value for y missing or invalid");
    ASSERT_EQUAL(jed.find("d"), astring("83"), TEST + "value for d missing or invalid");
    ASSERT_EQUAL(jed.find("e"), astring("54"), TEST + "value for e missing or invalid");
    ASSERT_EQUAL(jed.find("sud"), astring("xj23-8"), TEST + "value for sud missing or invalid");
    ASSERT_EQUAL(jed.find("nigh"), astring("2"), TEST + "value for nigh missing or invalid");
  }    
  {
    astring test_set_6 = "\r\n\r\n\r\
# this is yet another test with comments.\r\n\
; we want to be sure stuff works right.\r\n\
crumpet=tempest\r\n\
  moomar=18\r\n\
shagbot  =once upon a time there was a man  \r\n\
\t\t\tpunzola megamum  =brandle the handle  \r\n\
trapzoot=  uhhh\r\n\
mensch   = racer X\r\n\
\r\n\r\n\r\n";

    astring TEST = "Sixth Test: ";
    astring testing = test_set_6;
    LOG(astring("file before parsing:\n===========\n") + testing + "\n===========");
    variable_tokenizer jed("\n\r", "=");
    jed.set_comment_chars("#;");
    ASSERT_TRUE(jed.parse(testing), TEST + "jed should be parseable");
    astring out;
    jed.text_form(out);
    LOG(astring("file after parsing:\n===========\n") + out + "\n===========");
    LOG("and in tabular form:");
    LOG(jed.table().text_form());

    variable_tokenizer gorp("\n\r", "=");
    gorp.set_comment_chars("#;");
    ASSERT_TRUE(gorp.parse(out), TEST + "gorp should be parseable");
    LOG("gorp in tabular form:");
    LOG(gorp.table().text_form());
LOG(a_sprintf("gorp has %d fields, jed has %d fields", gorp.symbols(), jed.symbols()));
    ASSERT_TRUE(gorp.table() == jed.table(), TEST + "gorp text not same as jed!");

    ASSERT_EQUAL(jed.find("crumpet"), astring("tempest"),
        TEST + "value for crumpet missing or invalid");
    ASSERT_EQUAL(jed.find("moomar"), astring("18"),
        TEST + "value for moomar missing or invalid");
    ASSERT_EQUAL(jed.find("shagbot"), astring("once upon a time there was a man"),
        TEST + "value for shagbot missing or invalid");
    ASSERT_EQUAL(jed.find("trapzoot"), astring("uhhh"),
        TEST + "value for trapzoot missing or invalid");
    ASSERT_EQUAL(jed.find("punzola megamum"), astring("brandle the handle"),
        TEST + "value for punzola missing or invalid");
    ASSERT_EQUAL(jed.find("mensch"), astring("racer X"),
        TEST + "value for mensch missing or invalid");
  }    

  return final_report();
}

//////////////

HOOPLE_MAIN(test_tokenizer, );

