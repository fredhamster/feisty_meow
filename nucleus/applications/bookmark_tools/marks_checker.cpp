/*****************************************************************************\
*                                                                             *
*  Name   : marks_checker                                                     *
*  Author : Chris Koeritz                                                     *
*                                                                             *
*  Purpose:                                                                   *
*                                                                             *
*    Checks on the existence of the links listed in a HOOPLE format link      *
*  database and reports the bad ones.                                         *
*                                                                             *
*******************************************************************************
* Copyright (c) 2005-$now By Author.  This program is free software; you can  *
* redistribute it and/or modify it under the terms of the GNU General Public  *
* License as published by the Free Software Foundation; either version 2 of   *
* the License or (at your option) any later version.  This is online at:      *
*     http://www.fsf.org/copyleft/gpl.html                                    *
* Please send any updates to: fred@gruntose.com                               *
\*****************************************************************************/

#include "bookmark_tree.h"

#include <algorithms/sorts.h>
#include <application/command_line.h>
#include <application/hoople_main.h>
#include <application/windoze_helper.h>
#include <basis/astring.h>
#include <basis/functions.h>
#include <basis/guards.h>
#include <basis/mutex.h>
#include <filesystem/byte_filer.h>
#include <filesystem/filename.h>
#include <loggers/file_logger.h>
#include <mathematics/chaos.h>
#include <processes/ethread.h>
#include <processes/thread_cabinet.h>
#include <structures/static_memory_gremlin.h>
#include <structures/unique_id.h>
#include <textual/parser_bits.h>
#include <timely/time_control.h>

#include <curl/curl.h>
#include <signal.h>
#include <stdlib.h>

using namespace algorithms;
using namespace application;
using namespace basis;
using namespace filesystem;
using namespace loggers;
using namespace nodes;
using namespace mathematics;
using namespace processes;
using namespace structures;
using namespace textual;
using namespace timely;

//#define DEBUG_MARKS
  // uncomment to have more debugging noise.

#undef BASE_LOG
#define BASE_LOG(s) program_wide_logger::get().log(astring(s), ALWAYS_PRINT)
#undef LOG
#define LOG(s) CLASS_EMERGENCY_LOG(program_wide_logger::get(), \
   a_sprintf("line %d: ", _categories._line_number) + s)

const int PAUSEY_SNOOZE = 200;
  // how long we sleep if there are too many threads running already.

const int MAXIMUM_THREADS = 14;
  // we allow this many simultaneous web checks at a time.

const int MAXIMUM_READ = 1008;
  // we only download this much of the link.  this avoids huge downloads of
  // very large sites.

const int MAXIMUM_ATTEMPTS = 2;
  // we'll retry the check if we get an actual error instead of an http error
  // code.  when a name can't be found in the DNS, it sometimes comes back
  // shortly after it was checked.  if we see we can't reach the domain after
  // this many tries, then we give up on the address.

const int TIME_PER_REQUEST_IN_SEC = 60 * 6;
  // limit our requests to this long of a period.  then we will not be
  // stalled forever by uncooperative websites.

const char *FAKE_AGENT_STRING = "FredWeb/7.0 (X11; U; Linux i686; "
    "en-US; rv:1.8.19) Flecko/20081031";
  // we use this as our agent type, since some sites won't treat us fairly
  // if they think we're robots when we're checking their site health.
//still true?
  // for example (ahem!), the usa today websites.

////////////////////////////////////////////////////////////////////////////

class safe_int_array
{
public:
  safe_int_array() : _lock(), _list(0) {}

  void add(int to_add) {
///BASE_LOG(a_sprintf("adding %d to list", to_add));
    auto_synchronizer l(_lock);
    _list += to_add;
  }

  int length() {
    auto_synchronizer l(_lock);
    return _list.length();
  }

  basis::int_array make_copy() {
    auto_synchronizer l(_lock);
    return _list;
  }

private:
  basis::mutex _lock;
  basis::int_array _list;
};

////////////////////////////////////////////////////////////////////////////

class marks_checker : public application_shell
{
public:
  marks_checker()
      : application_shell(), _check_redirection(false),
        _max_threads(MAXIMUM_THREADS), _null_file(filename::null_device(), "w")
  {}

  DEFINE_CLASS_NAME("marks_checker");
  virtual int execute();
  int print_instructions(const filename &program_name);

  int test_all_links();
    // goes through the tree of links and tests them all.
  
  int check_link(const astring &url, astring &error_msg);
    // synchronously checks the "url" for health.  the return value is zero
    // on success or an HTTP error code on failure.

  void write_new_files();
    // writes out the two new files given the info accumulated so far.

private:
  bookmark_tree _categories;  // our tree of categories.
  safe_int_array _bad_lines;  // lines with bad contents.
  thread_cabinet _checkers;  // threads checking on links.
  astring _input_filename;  // we'll store our link database name here.
  astring _output_filename;  // where the list of good links is stored.
  astring _bad_link_filename;  // garbage dump of bad links.
  bool _check_redirection;  // true if redirection is disallowed.
  int _max_threads;  // the most threads we'll allow at once.
  byte_filer _null_file;  // we'll use this for trashing output data.

  static void handle_OS_signal(int sig_id);
    // handles break signals from the user.
};

////////////////////////////////////////////////////////////////////////////

class checking_thread : public ethread
{
public:
  checking_thread(const link_record &link_info, safe_int_array &bad_lines,
      marks_checker &checker)
  : ethread(), _bad_lines(bad_lines), _checker(checker), _info(link_info) {}

  void perform_activity(void *formal(ptr)) {
    astring message;
    int ret = _checker.check_link(_info._url, message);
    if (ret != 0) {
      astring complaint = a_sprintf("Bad Link at line %d:", _info._uid)
          += parser_bits::platform_eol_to_chars();
      const astring spacer(' ', 4);
      complaint += spacer + _info._url += parser_bits::platform_eol_to_chars();
      complaint += spacer + _info._description += parser_bits::platform_eol_to_chars();
      complaint += spacer + "error: " += message;
      BASE_LOG(complaint);
if ( (_info._uid> 100000) || (_info._uid < 0) ) {
BASE_LOG(a_sprintf("somehow got bogus line number!  %d", _info._uid));
return;
}
      _bad_lines.add(_info._uid);  // list ours as bad.
    }
  }

private:
  safe_int_array &_bad_lines;
  marks_checker &_checker;
  link_record _info;
};

////////////////////////////////////////////////////////////////////////////

int marks_checker::print_instructions(const filename &program_name)
{
  a_sprintf to_show("%s:\n\
This program needs three filenames as command line parameters.  The -i flag\n\
is used to specify the input filename. The -o flag specifies the file where\n\
where the good links will be written.  The -b flag specifies the file where\n\
the bad links are written.  The optional flag --no-redirs can be used to\n\
disallow web-site redirection, which will catch when the site has changed\n\
its location.  Note that redirection is not necessarily an error, but it\n\
instead may just be a link that needs its URL modified.  It is recommended\n\
that you omit this flag in early runs, in order to only locate definitely\n\
dead links.  Then later checking runs can find any sites that were redirected\n\
or being routed to a dead link page which doesn't provide an error code.\n\
The optional flag --threads with a parameter will set the maximum number of\n\
threads that will simultaneously check on links.\n\
The input file is expected to be in the HOOPLE link database format.\n\
The HOOPLE link format is documented here:\n\
    http://feistymeow.org/guides/link_database/format_manifesto.txt\n\
", program_name.basename().raw().s(), program_name.basename().raw().s());
  program_wide_logger::get().log(to_show, ALWAYS_PRINT);
  return 12;
}

// this function just eats any data it's handed.
size_t data_sink(void *formal(ptr), size_t size, size_t number, void *formal(stream))
{ return size * number; }

int marks_checker::check_link(const astring &url, astring &error_msg)
{
  int to_return = -1;

  CURL *cur = curl_easy_init();

  curl_easy_setopt(cur, CURLOPT_URL, url.s());  // set the URL itself.

  curl_easy_setopt(cur, CURLOPT_SSL_VERIFYPEER, 0);
    // don't verify SSL certificates.
  curl_easy_setopt(cur, CURLOPT_MAXFILESIZE, MAXIMUM_READ);
    // limit the download size; causes size errors, which we elide to success.
  curl_easy_setopt(cur, CURLOPT_NOSIGNAL, 1);
    // don't use signals since it interferes with sleep.
  curl_easy_setopt(cur, CURLOPT_TIMEOUT, TIME_PER_REQUEST_IN_SEC);
    // limit time allowed per operation.
  curl_easy_setopt(cur, CURLOPT_AUTOREFERER, true);
    // automatically fill in the referer field when redirected.

  curl_easy_setopt(cur, CURLOPT_WRITEDATA, _null_file.file_handle());
    // set the file handle where we want our downloaded data to go.  since
    // we're just checking the links, this goes right to the trash.
  curl_easy_setopt(cur, CURLOPT_WRITEFUNCTION, data_sink);
    // set the function which will be given all the downloaded data.

  curl_easy_setopt(cur, CURLOPT_USERAGENT, FAKE_AGENT_STRING);
    // fake being a browser here since otherwise we get no respect.

  curl_easy_setopt(cur, CURLOPT_FTPLISTONLY, 1);
    // get only a simple list of files, which allows us to hit ftp sites
    // properly.  if the normal curl mode is used, we get nothing.

  if (_check_redirection) {
    // attempting to quash redirects as being valid.
    curl_easy_setopt(cur, CURLOPT_FOLLOWLOCATION, 1);  // follow redirects.
    curl_easy_setopt(cur, CURLOPT_MAXREDIRS, 0);  // allow zero redirects.
  }

  int tries = 0;
  while (tries++ < MAXIMUM_ATTEMPTS) {

    // we do the error message again every time, since it gets shrunk after
    // the web page check and is no longer available where it was in memory.
    error_msg = astring(' ', CURL_ERROR_SIZE + 5);
    curl_easy_setopt(cur, CURLOPT_ERRORBUFFER, error_msg.s());

    // set the error message buffer so we know what happened.

    // try to lookup the web page we've been given.
    to_return = curl_easy_perform(cur);

    error_msg.shrink();  // just use the message without extra spaces.

    // we turn file size errors into non-errors, since we have set a very
    // low file size in order to avoid downloading too much.  we really just
    // want to check links, not download their contents.
    if (to_return == CURLE_FILESIZE_EXCEEDED) to_return = 0;

    if (!to_return) {
      // supposedly this is a success, but let's check the result code.
      long result;
      curl_easy_getinfo(cur, CURLINFO_RESPONSE_CODE, &result);
      if (result >= 400) {
        error_msg = a_sprintf("received http failure code %d", result);
        to_return = -1;
      }
      break;  // this was a successful result, a zero outcome from perform.
    }

    time_control::sleep_ms(10 * SECOND_ms);  // give it a few more seconds...
  }

  curl_easy_cleanup(cur);

  return to_return;
}

int marks_checker::test_all_links()
{
  FUNCDEF("test_all_links");
  // traverse the tree in prefix order.
  tree::iterator itty = _categories.access_root().start(tree::prefix);
  tree *curr = NULL_POINTER;
  while ( (curr = itty.next()) ) {
    inner_mark_tree *nod = dynamic_cast<inner_mark_tree *>(curr);
    if (!nod)
      non_continuable_error(static_class_name(), func, "failed to cast a tree node");
    // iterate on all the links at this node to check them.
    for (int i = 0; i < nod->_links.elements(); i++) {
      link_record *lin = nod->_links.borrow(i);
      if (!lin->_url) continue;  // not a link.

      while (_checkers.threads() > _max_threads) {
        time_control::sleep_ms(PAUSEY_SNOOZE);
        _checkers.clean_debris();
      }
      
      checking_thread *new_thread = new checking_thread(*lin, _bad_lines,
          *this);
      unique_int id = _checkers.add_thread(new_thread, true, NULL_POINTER);
    }
  }

BASE_LOG("... finished iterating on tree.");

  // now wait until all the threads are finished.  
  while (_checkers.threads()) {
    time_control::sleep_ms(PAUSEY_SNOOZE);
    _checkers.clean_debris();
  }
  
BASE_LOG("... finished waiting for all threads.");

  return 0;
}

void marks_checker::write_new_files()
{
  byte_filer input_file(_input_filename, "r");
  byte_filer output_file(_output_filename, "w");
  byte_filer badness_file(_bad_link_filename, "w");

  basis::int_array badness = _bad_lines.make_copy();
  shell_sort<int>(badness.access(), badness.length());

  BASE_LOG("bad links are on lines:");
  astring bad_list;
    for (int i = 0; i < badness.length(); i++) {
    bad_list += a_sprintf("%d, ", badness[i]);
  }
  BASE_LOG(bad_list);

  astring buffer;
  int curr_line = 0;
  while (!input_file.eof()) {
    curr_line++;
    while (badness.length() && (badness[0] < curr_line) ) {
      BASE_LOG(a_sprintf("whacking too low line number: %d", badness[0]));
      badness.zap(0, 0);
    }
    input_file.getline(buffer, 2048);
//make that a constant.
    if (badness.length() && (badness[0] == curr_line)) {
      // we seem to have found a bad line.
      badness_file.write(buffer);
      badness.zap(0, 0);  // remove the current line number.
    } else {
      // this is a healthy line.
      output_file.write(buffer);
    }
    
  }
  input_file.close();
  output_file.close();
  badness_file.close();
}

marks_checker *main_program = NULL_POINTER;

void marks_checker::handle_OS_signal(int formal(sig_id))
{
  signal(SIGINT, SIG_IGN);  // turn off that signal for now.
  BASE_LOG("caught break signal...  now writing files.");
  if (main_program) main_program->write_new_files();
  BASE_LOG("exiting after handling break.");
  main_program = NULL_POINTER;
  exit(0);
}

int marks_checker::execute()
{
  FUNCDEF("execute");
  SETUP_COMBO_LOGGER;

  main_program = this;  // used by our signal handler.

  command_line cmds(_global_argc, _global_argv);  // process the command line parameters.
  if (!cmds.get_value('i', _input_filename, false))
    return print_instructions(cmds.program_name());
  if (!cmds.get_value('o', _output_filename, false))
    return print_instructions(cmds.program_name());
  if (!cmds.get_value('b', _bad_link_filename, false))
    return print_instructions(cmds.program_name());

  astring temp;

  // optional flag for checking website redirection.
  if (cmds.get_value("no-redirs", temp, false)) {
    BASE_LOG("Enabling redirection checking: redirected web sites are reported as bad.");
    _check_redirection = true;
  }
  // optional flag for number of threads.
  astring threads;
  if (cmds.get_value("threads", threads, false)) {
    _max_threads = threads.convert(0);
    BASE_LOG(a_sprintf("Maximum threads allowed=%d", _max_threads));
  }

  BASE_LOG(astring("input file: ") + _input_filename);
  BASE_LOG(astring("output file: ") + _output_filename);
  BASE_LOG(astring("bad link file: ") + _bad_link_filename);

//hmmm: check if output file already exists.
//hmmm: check if bad file already exists.

LOG("before reading input...");

  int ret = _categories.read_csv_file(_input_filename);
  if (ret) return ret;  // failure during read means we can't do much.

LOG("after reading input...");

  signal(SIGINT, handle_OS_signal);
    // hook the break signal so we can still do part of the job if they
    // interrupt us.

  curl_global_init(CURL_GLOBAL_ALL);  // crank up the cURL engine.

  ret = test_all_links();
  
  write_new_files();
  main_program = NULL_POINTER;

  curl_global_cleanup();  // shut down cURL engine again.

  return 0;
}

////////////////////////////////////////////////////////////////////////////

HOOPLE_MAIN(marks_checker, )

