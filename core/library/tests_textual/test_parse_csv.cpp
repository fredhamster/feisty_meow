/*
*  Name   : test parsing of csv
*  Author : Chris Koeritz
*  Purpose: Checks that the CSV parsing function handles a few common scenarios.
**
* Copyright (c) 2005-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
*/

#include <application/hoople_main.h>
#include <basis/astring.h>
#include <basis/functions.h>
#include <basis/guards.h>
#include <configuration/application_configuration.h>
#include <filesystem/byte_filer.h>
#include <filesystem/filename.h>
#include <structures/string_array.h>
#include <structures/static_memory_gremlin.h>
#include <textual/list_parsing.h>
#include <timely/stopwatch.h>
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

#define LOG(s) EMERGENCY_LOG(program_wide_logger::get(), s)

// the number of times we scan our data file for performance test.
const int MAX_DATA_FILE_ITERS = 4000;

class test_parsing_csv : public virtual unit_base, public virtual application_shell
{
public:
  test_parsing_csv() {}
  DEFINE_CLASS_NAME("test_parsing_csv");
  int execute();
};

//hmmm: too congratulatory?
#define COMPLAIN_FIELD(list, index, value) \
  ASSERT_EQUAL(list[index], astring(value), \
    a_sprintf("comparison test should have field %d correct in %s", index, #list))

int test_parsing_csv::execute()
{
  FUNCDEF("execute");
  astring line1 = "\"fupe\",\"snoorp\",\"lutem\",\"fipe\"";
  string_array fields1;
  bool works1 = list_parsing::parse_csv_line(line1, fields1);  
  ASSERT_TRUE(works1, "first test should not fail to parse");
//LOG(a_sprintf("fields len now %d", fields1.length()));
  ASSERT_EQUAL(fields1.length(), 4, "first test should have right count of strings found");
  COMPLAIN_FIELD(fields1, 0, "fupe");
  COMPLAIN_FIELD(fields1, 1, "snoorp");
  COMPLAIN_FIELD(fields1, 2, "lutem");
  COMPLAIN_FIELD(fields1, 3, "fipe");

  astring line2 = "fupe,\"snoorp\",lutem,\"fipe\"";
  string_array fields2;
  bool works2 = list_parsing::parse_csv_line(line2, fields2);  
  ASSERT_TRUE(works2, "second test should not fail to parse");
  ASSERT_EQUAL(fields2.length(), 4, "second test should have right count of strings found");
  COMPLAIN_FIELD(fields2, 0, "fupe");
  COMPLAIN_FIELD(fields2, 1, "snoorp");
  COMPLAIN_FIELD(fields2, 2, "lutem");
  COMPLAIN_FIELD(fields2, 3, "fipe");

  astring line3 = "\"lowenburger\",\"wazizzle\",morphel";
  string_array fields3;
  bool works3 = list_parsing::parse_csv_line(line3, fields3);  
  ASSERT_TRUE(works3, "third test should not fail to parse");
  ASSERT_EQUAL(fields3.length(), 3, "third test should have right count of strings found");
  COMPLAIN_FIELD(fields3, 0, "lowenburger");
  COMPLAIN_FIELD(fields3, 1, "wazizzle");
  COMPLAIN_FIELD(fields3, 2, "morphel");

  astring line4 = "\"lowenburger\",\"wazizzle\",morphel,";
  string_array fields4;
  bool works4 = list_parsing::parse_csv_line(line4, fields4);  
  ASSERT_TRUE(works4, "fourth test should not fail to parse");
  ASSERT_EQUAL(fields4.length(), 4, "fourth test should not have wrong count of strings found");
  COMPLAIN_FIELD(fields4, 0, "lowenburger");
  COMPLAIN_FIELD(fields4, 1, "wazizzle");
  COMPLAIN_FIELD(fields4, 2, "morphel");
  COMPLAIN_FIELD(fields4, 3, "");

  astring line5 = "\"lowenburger\",,";
  string_array fields5;
  bool works5 = list_parsing::parse_csv_line(line5, fields5);  
  ASSERT_TRUE(works5, "fifth test should not fail to parse");
  ASSERT_EQUAL(fields5.length(), 3, "fifth test should have right count of strings found");
  COMPLAIN_FIELD(fields5, 0, "lowenburger");
  COMPLAIN_FIELD(fields5, 1, "");
  COMPLAIN_FIELD(fields5, 2, "");

  astring line6 = ",,,\"rasputy\",,\"spunk\",ralph";
  string_array fields6;
  bool works6 = list_parsing::parse_csv_line(line6, fields6);  
  ASSERT_TRUE(works6, "sixth test should not fail to parse");
  ASSERT_EQUAL(fields6.length(), 7, "sixth test should have right count of strings found");
  COMPLAIN_FIELD(fields6, 0, "");
  COMPLAIN_FIELD(fields6, 1, "");
  COMPLAIN_FIELD(fields6, 2, "");
  COMPLAIN_FIELD(fields6, 3, "rasputy");
  COMPLAIN_FIELD(fields6, 4, "");
  COMPLAIN_FIELD(fields6, 5, "spunk");
  COMPLAIN_FIELD(fields6, 6, "ralph");

  astring line7 = "\"SRV0001337CHN0000001DSP0000001SRV0001337LAY0003108,16,0,8,192\",\"\\\"row_3\\\" on 12.5.55.159\",3";
  string_array fields7;
  bool works7 = list_parsing::parse_csv_line(line7, fields7);
  ASSERT_TRUE(works7, "seventh test should not fail to parse");
  ASSERT_EQUAL(fields7.length(), 3, "seventh test should have right count of strings found");
  COMPLAIN_FIELD(fields7, 0, "SRV0001337CHN0000001DSP0000001SRV0001337LAY0003108,16,0,8,192");
  COMPLAIN_FIELD(fields7, 1, "\"row_3\" on 12.5.55.159");
  COMPLAIN_FIELD(fields7, 2, "3");

  // test 8...  use data file.
  filename df_dir = filename(application_configuration::application_name()).dirname();
  byte_filer test_data(df_dir.raw() + "/df_1.csv", "rb");
  string_array parsed;
  string_array lines;
  astring curr_line;
  int read_result;
  while ( (read_result = test_data.getline(curr_line, 1024)) > 0 )
    lines += curr_line;
  if (lines.length()) {
    // now we have the data file loaded.
    stopwatch clicker;
    clicker.start();
    for (int iterations = 0; iterations < MAX_DATA_FILE_ITERS; iterations++) {
      for (int line = 0; line < lines.length(); line++) {
        const astring &current = lines[line];
        list_parsing::parse_csv_line(current, parsed);
      }
    }
    clicker.stop();
    log(a_sprintf("%d csv lines with %d iterations took %d ms (or %d s).",
        lines.length(), MAX_DATA_FILE_ITERS, clicker.elapsed(),
        clicker.elapsed() / 1000));
  }

  // test 9: process results of create_csv_line.
  string_array fields9;
  fields9 += "ACk\"boozort";
  fields9 += "sme\"ra\"\"foop";
  fields9 += "\"gumby\"";
  astring line9 = "\"ACk\\\"boozort\",\"sme\\\"ra\\\"\\\"foop\",\"\\\"gumby\\\"\"";
  astring gen_line_9;
  list_parsing::create_csv_line(fields9, gen_line_9);
//log(astring(" got gen line: ") + gen_line_9);
//log(astring("expected line: ") + line9);
  ASSERT_EQUAL(gen_line_9, line9, "ninth test should not fail to create expected text");
  string_array fields9_b;
  bool works9 = list_parsing::parse_csv_line(gen_line_9, fields9_b);
  ASSERT_TRUE(works9, "ninth test should not fail to parse");
  ASSERT_TRUE(fields9_b == fields9, "ninth test should match original fields");

  return final_report();
}

//////////////

HOOPLE_MAIN(test_parsing_csv, )

