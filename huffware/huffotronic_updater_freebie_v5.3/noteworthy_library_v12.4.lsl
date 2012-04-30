
// huffware script: noteworthy library, by fred huffhines.
//
// a handy approach to reading a notecard.  this version supports requiring
// a 'signature' in the notecard's first line, so that multiple notecards can
// exist in an object without being misinterpreted.  the script is accessed via
// its link message API, so it can be used in an object without all this code
// needing to be embedded in another script.  it also supports queuing up requests
// to read notecards, so multiple scripts can use it to read their specific
// notecards without any special handling (besides waiting a bit longer).
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//

// constants that can be adapted to your needs...

integer DEBUGGING = FALSE;
    // if this is true, then a lot of extra noise is generated when notecards are read.

float TIME_OUT_ON_ONE_NOTECARD = 42.0;
    // we allow one line of a notecard this long to be read before we decide it's hosed.
    // some sims are very very slow, and a time-out of one minute has been found lacking;
    // we saw at least one case where the first notecard line to be read took longer than
    // 60 seconds.  that's why we kept cranking this time-out up...

// constants that should not be changed...

// outcomes from handling a line in a notecard.
integer STILL_READING = -8;  // the notecard seems good, but isn't finished.
integer BAD_NOTECARD = -9;  // this notecard doesn't have our signature.
integer DONE_READING = -10;  // the notecard is finished being read.

integer LIST_ELEMENT_GUESS = 200;  // guess at how many bytes average list element uses.

integer MAXIMUM_LINES_USED = 4;
    // we will read this many lines at a time from the appropriate notecard.

// this is the noteworthy library's API that is available via link messages.
//////////////
// do not redefine these constants.
integer NOTEWORTHY_HUFFWARE_ID = 10010;
    // the unique id within the huffware system for the noteworthy script to
    // accept commands on.  this is used in llMessageLinked as the num parameter.
string HUFFWARE_PARM_SEPARATOR = "{~~~}";
    // this pattern is an uncommon thing to see in text, so we use it to separate
    // our commands in link messages.
string HUFFWARE_ITEM_SEPARATOR = "{|||}";
    // used to separate lists of items from each other when stored inside a parameter.
    // this allows lists to be passed as single string parameters if needed.
integer REPLY_DISTANCE = 100008;  // offset added to service's huffware id in reply IDs.
string BAD_NOTECARD_INDICATOR = "bad_notecard";
    // indicates that the notecard reading process has failed to find an appropriate one.
    
//STRIKE THIS from everywhere.
//string BUSY_READING_INDICATOR = "busy_already";
//    // this return value indicates that the script is already in use by some other script.
//    // the calling script should try again later.

string NOTECARD_READ_CONTINUATION = "continue!";
    // returned as first parameter if there is still more data to handle.
// commands available via the noteworthy library:
string READ_NOTECARD_COMMAND = "#read_note#";
    // command used to tell the script to read notecards.  needs a signature to find
    // in the card as the first parameter, and a randomly generated response code for
    // the second parameter.  the response code is used to uniquely identify a set of
    // pending notecard readings (hopefully).  the signature can be blank.
    // the results will be fired back as the string value returned, which will have
    // as first element the notecard's name (or BAD_NOTECARD_INDICATOR if none was
    // found) and as subsequent elements an embedded list that was read from the
    // notecard.  this necessarily limits the size of the notecards that we can read
    // and return.
string READ_SPECIFIC_NOTECARD_COMMAND = "#read_thisun#";
    // like the read notecard command, but specifies the notecard name to use.  only that
    // specific notecard file will be consulted.  first and second parm are still signature
    // and response code, third parm is the notecard name.
//
//////////////
// joins a list of parameters using the parameter sentinel for the library.
string wrap_parameters(list to_flatten)
{ return llDumpList2String(to_flatten, HUFFWARE_PARM_SEPARATOR); }
//////////////

// global variables...

string requested_signature = "";
    // if this is non-empty, then it must be found in the first line of the script.

integer only_read_one_notecard = FALSE;  // true if just one specific notecard should be used.

string global_notecard_name;  // the name of the card we're reading now.
key global_query_id = NULL_KEY;  // the identifier of our data query.
integer current_response_code = 0;  // the code that our client uses for its reading.
list global_query_contents;  // the lines we have read from the notecard.

integer line_number = 0;  // which line are we at in notecard?

integer found_signature_line = FALSE;  // did we find our first line in a notecard yet?

integer trying_notecard_at = -1;  // where we are currently looking for a good notecard.

list pending_signatures;  // signatures from queued requests for reading.
list pending_response_codes;  // response codes for the queued requests.
list pending_notecard_names;  // card names if it's a specific request.

//////////////

startup_initialize()
{
    pending_signatures = [];
    pending_response_codes = [];
    pending_notecard_names = [];
    current_response_code = 0;
    llSetTimerEvent(0.0);
}

reset_for_reading(string signature, integer response_code)
{
    requested_signature = signature;
    current_response_code = response_code;
    llSetTimerEvent(TIME_OUT_ON_ONE_NOTECARD);  // don't allow a read to happen forever.
    global_query_contents = [];
    global_notecard_name = "";
    line_number = 0;
    found_signature_line = FALSE;
    trying_notecard_at = -1;
    global_query_id = NULL_KEY;
}

// use the existing global notecard setting to start reading.
select_specific_notecard()
{
    global_query_id = NULL_KEY;  // reset the query id so we don't get bogus answers.
    line_number = 0;  // reset line number again.
    global_query_id = llGetNotecardLine(global_notecard_name, 0);
}

// picks the notecard at the "index" (from 0 to num_notecards - 1) and
// starts reading it.
select_notecard(integer index)
{
    global_query_id = NULL_KEY;  // reset the query id so we don't get bogus answers.
    string card_specified = llGetInventoryName(INVENTORY_NOTECARD, index);
    if (card_specified == "") return;   // not good.  bad index.
    global_notecard_name = card_specified;
    line_number = 0;  // reset line number again.
    // we have a new file name, so load up the destinations, hopefully.
    global_query_id = llGetNotecardLine(global_notecard_name, 0);
}

// increments our index in the count of notecards that the object has, and start
// reading the next notecard (at the index).
integer try_next_notecard()
{
    if (only_read_one_notecard) {
        return FALSE;  // we were only going to try one.
    }
    // reset some values that might have applied before.
    global_notecard_name = "";
    // skip to the next card.
    trying_notecard_at++;
    // make sure we're not over the count of cards.
    if (trying_notecard_at >= llGetInventoryNumber(INVENTORY_NOTECARD)) {
        // this is a problem.  we didn't find anything suitable.
        return FALSE;
    }
    // so, now we'll try the next notecard to look for our signature.
    select_notecard(trying_notecard_at);
    return TRUE;
}

// process a line of text that we received from the current notecard.
integer handle_notecard_line(key query_id, string data)
{
    // if we're not at the end of the notecard we're reading...
    if (data != EOF) {
        // there's more to read in the notecard still.
        if (data != "") {
            // make sure we even have a signature to look for.
            if (!found_signature_line && (requested_signature == "")) {
                // no signature means that we always find it.
                found_signature_line = TRUE;
            }
            // did we already get our signature?  if not, see if this is it.
            if (!found_signature_line && (data != requested_signature) ) {
                // this is a bad notecard.  skip it.
                if (!try_next_notecard()) {
                    // we have no more to work with.
                    return BAD_NOTECARD;
                }
                return STILL_READING;  // we'll keep going.
            } else if (!found_signature_line && (data == requested_signature) ) {
                // this is a good signature line, so record that and then skip it.
                found_signature_line = TRUE;
            } else {
                if (DEBUGGING
                    && ( ( (requested_signature == "") && (line_number == 0) )
                        || ( (requested_signature != "") && (line_number == 1) ) ) ) {
                    log_it("started reading " + global_notecard_name + "...");
                }
                // don't record any lines that are comments.
                if ( (llGetSubString(data, 0, 0) != "#")
                    && (llGetSubString(data, 0, 0) != ";") ) {
                    // add the non-blank line to our list.
                    global_query_contents += data;
                    // make sure we still have enough space to keep going.
                    if (llGetListLength(global_query_contents) >= MAXIMUM_LINES_USED) {
                        // ooh, make sure we pause before blowing our heap&stack.
                        send_reply(LINK_THIS, [ NOTECARD_READ_CONTINUATION,
                            current_response_code ], READ_NOTECARD_COMMAND, TRUE);                
                    }
                }
            }
        }
        line_number++;  // increase the line count.
        // reset the timer rather than timing out, if we actually got some data.
        llSetTimerEvent(TIME_OUT_ON_ONE_NOTECARD);        
        // request the next line from the notecard.
        global_query_id = llGetNotecardLine(global_notecard_name, line_number);
        if (global_query_id == NULL_KEY) {
//log_it("failed to restart notecard reading.");
            return DONE_READING;
//is that the best outcome?
        }
        return STILL_READING;
    } else {
        // that's the end of the notecard.  we need some minimal verification that it
        // wasn't full of garbage.
        if (!found_signature_line) {
            if (DEBUGGING) log_it("never found signature in " + global_notecard_name);
            if (!try_next_notecard()) {
                return BAD_NOTECARD;  // we failed to find a good line?
            } else {
                // the next notecard's coming through now.
                return STILL_READING;
            }
        } else {
//            if (DEBUGGING) log_it("found signature.");
            // saw the signature line, so this is a good one.
            return DONE_READING;
        }
    }
}

// only sends reply; does not reset notecard process.
send_reply(integer destination, list parms, string command,
    integer include_query)
{
//integer items = llGetListLength(parms);
//if (include_query) items += llGetListLength(global_query_contents);
//log_it("pre-sending " + (string)items + " items, mem=" + (string)llGetFreeMemory());

   if (!include_query) {
        llMessageLinked(destination, NOTEWORTHY_HUFFWARE_ID + REPLY_DISTANCE,
            command, llDumpList2String(parms, HUFFWARE_PARM_SEPARATOR));
    } else {
        llMessageLinked(destination, NOTEWORTHY_HUFFWARE_ID + REPLY_DISTANCE,
            command,
            llDumpList2String(parms + global_query_contents, HUFFWARE_PARM_SEPARATOR));
    }
    global_query_contents = [];
//log_it("post-sending, mem=" + (string)llGetFreeMemory());
}

// a simple version of a reply for a command that has been executed.  the parameters
// might contain an outcome or result of the operation that was requested.
send_reply_and_reset(integer destination, list parms, string command,
    integer include_query)
{
    llSetTimerEvent(0.0);  // stop the timer, since we're about to reply.
    send_reply(destination, parms, command, include_query);
    current_response_code = 0;  // reset back to default so we can start another read.
    global_query_id = NULL_KEY;  // reset so we accept no more data.
}

// if there are other pending notecard reads, this goes to the next one listed.
dequeue_next_request()
{
    if (llGetListLength(pending_signatures)) {
        // get the info from the next pending item.
        string sig = llList2String(pending_signatures, 0);
        integer response_code = llList2Integer(pending_response_codes, 0);
        string notecard = llList2String(pending_notecard_names, 0);
        // whack the head of the queue since we grabbed the info.
        pending_signatures = llDeleteSubList(pending_signatures, 0, 0);
        pending_response_codes = llDeleteSubList(pending_response_codes, 0, 0);
        pending_notecard_names = llDeleteSubList(pending_notecard_names, 0, 0);
        if (llStringLength(notecard)) {
            global_notecard_name = notecard;
            select_specific_notecard();
        } else {
            reset_for_reading(sig, response_code);
        }
    }
}

// deals with one data server answer from the notecard.
process_notecard_line(key query_id, string data)
{
    // try to consume a line from the notecard.
    integer outcome = handle_notecard_line(query_id, data);
    if (outcome == DONE_READING) {
        // that was a valid notecard and we read all of it.
        if (DEBUGGING) log_it("finished reading " + global_notecard_name + ".");
        // send back the results.
        send_reply_and_reset(LINK_THIS, [ global_notecard_name, current_response_code ],
            READ_NOTECARD_COMMAND, TRUE);
    } else if (outcome == BAD_NOTECARD) {
        // bail; this problem must be addressed by other means.
        if (DEBUGGING) log_it("failed to find an appropriate notecard");
        send_reply_and_reset(LINK_THIS, [ BAD_NOTECARD_INDICATOR, current_response_code ],
            READ_NOTECARD_COMMAND, FALSE);
    } else if (outcome == STILL_READING) {
        // we have a good card and are still processing it.
        return;
    } else {
        if (DEBUGGING) log_it("unknown outcome from handle_notecard_line");
        // again, bail out.  we have no idea what happened with this.
        send_reply_and_reset(LINK_THIS, [ BAD_NOTECARD_INDICATOR, current_response_code ],
            READ_NOTECARD_COMMAND, FALSE);
    }
    // if we have reached here, we should crank up the next queued notecard reading.
    dequeue_next_request();
}

// processes requests from our users.
handle_link_message(integer which, integer num, string msg, key id)
{
    if (num != NOTEWORTHY_HUFFWARE_ID) return;  // not for us.

    if (msg == READ_NOTECARD_COMMAND) {
        only_read_one_notecard = FALSE;  // general inquiry for any card.
        list parms = llParseString2List(id, [HUFFWARE_PARM_SEPARATOR], []);
//log_it("read notecard--parms are: " + (string)parms);
        string signature = llList2String(parms, 0);
        integer response_code = llList2Integer(parms, 1);
//log_it("got signature " + signature + " and respcode " + (string)response_code);
//holding:        if (!current_response_code) {
            // go ahead and process this request; we aren't busy.
            reset_for_reading(signature, response_code);
            if (!try_next_notecard()) {
                if (DEBUGGING) log_it("failed to find any appropriate notecards at all.");
                send_reply_and_reset(LINK_THIS, [ BAD_NOTECARD_INDICATOR, response_code ],
                    READ_NOTECARD_COMMAND, FALSE);
                return;
            }
//holding:        } else {
//holding:            // we're already busy.
//holding://            send_reply_and_reset(LINK_THIS, [ BUSY_READING_INDICATOR, response_code ],
//holding://                READ_NOTECARD_COMMAND, FALSE);
//holding:            // stack this request; another is in progress.
//holding:            pending_signatures += signature;
//holding:            pending_response_codes += response_code;
//holding:            pending_notecard_names += "";
//holding:        }
    } else if (msg == READ_SPECIFIC_NOTECARD_COMMAND) {
        only_read_one_notecard = TRUE;  // they want one particular card.
        list parms = llParseString2List(id, [HUFFWARE_PARM_SEPARATOR], []);
//log_it("read specific--parms are: " + (string)parms);
        string signature = llList2String(parms, 0);
        integer response_code = llList2Integer(parms, 1);
        string notecard_name = llList2String(parms, 2);
//log_it("got signature " + signature + " and respcode " + (string)response_code);
//holding:        if (!current_response_code) {
            // go ahead and process this request; we aren't busy.
            reset_for_reading(signature, response_code);
            global_notecard_name = notecard_name;  // set our global.
            select_specific_notecard();
//holding:        } else {
//holding:            // we're already busy.
//holding://            send_reply_and_reset(LINK_THIS, [ BUSY_READING_INDICATOR, response_code ],
//holding://                READ_NOTECARD_COMMAND, FALSE);
//holding:            // stack this request; another is in progress.
//holding:            pending_signatures += signature;
//holding:            pending_response_codes += response_code;
//holding:            pending_notecard_names += notecard_name;
//holding:        }
    }
}


///////////////

// from hufflets...

integer debug_num = 0;

// a debugging output method.  can be disabled entirely in one place.
log_it(string to_say)
{
    debug_num++;
    // tell this to the owner.    
    llOwnerSay(llGetScriptName() + "[" + (string)debug_num + "] " + to_say);
    // say this on open chat, but use an unusual channel.
//    llSay(108, (string)debug_num + "- " + to_say);
}

//////////////

//////////////
// huffware script: auto-retire, by fred huffhines, version 2.4.
// distributed under BSD-like license.
//   partly based on the self-upgrading scripts from markov brodsky and jippen faddoul.
// the function auto_retire() should be added *inside* a version numbered script that
// you wish to give the capability of self-upgrading.
//   this script supports a notation for versions embedded in script names where a 'v'
// is followed by a number in the form "major.minor", e.g. "grunkle script by ted v8.2".
// when the containing script is dropped into an object with a different version, the
// most recent version eats any existing ones.
//   keep in mind that this code must be *copied* into your script you wish to add
// auto-retirement capability to.
// example usage of the auto-retirement script:
//     default {
//         state_entry() {
//            auto_retire();  // make sure newest addition is only version of script.
//        }
//     }
auto_retire() {
    string self = llGetScriptName();  // the name of this script.
    list split = compute_basename_and_version(self);
    if (llGetListLength(split) != 2) return;  // nothing to do for this script.
    string basename = llList2String(split, 0);  // script name with no version attached.
    string version_string = llList2String(split, 1);  // the version found.
    integer posn;
    // find any scripts that match the basename.  they are variants of this script.
    for (posn = llGetInventoryNumber(INVENTORY_SCRIPT) - 1; posn >= 0; posn--) {
//log_it("invpo=" + (string)posn);
        string curr_script = llGetInventoryName(INVENTORY_SCRIPT, posn);
        if ( (curr_script != self) && (llSubStringIndex(curr_script, basename) == 0) ) {
            // found a basic match at least.
            list inv_split = compute_basename_and_version(curr_script);
            if (llGetListLength(inv_split) == 2) {
                // see if this script is more ancient.
                string inv_version_string = llList2String(inv_split, 1);  // the version found.
                // must make sure that the retiring script is completely the identical basename;
                // just matching in the front doesn't make it a relative.
                if ( (llList2String(inv_split, 0) == basename)
                    && ((float)inv_version_string < (float)version_string) ) {
                    // remove script with same name from inventory that has inferior version.
                    llRemoveInventory(curr_script);
                }
            }
        }
    }
}
//
// separates the base script name and version number.  used by auto_retire.
list compute_basename_and_version(string to_chop_up)
{
    // minimum script name is 2 characters plus a version.
    integer space_v_posn;
    // find the last useful space and 'v' combo.
    for (space_v_posn = llStringLength(to_chop_up) - 3;
        (space_v_posn >= 2) && (llGetSubString(to_chop_up, space_v_posn, space_v_posn + 1) != " v");
        space_v_posn--) {
        // look for space and v but do nothing else.
//log_it("pos=" + (string)space_v_posn);
    }
    if (space_v_posn < 2) return [];  // no space found.
//log_it("space v@" + (string)space_v_posn);
    // now we zoom through the stuff after our beloved v character and find any evil
    // space characters, which are most likely from SL having found a duplicate item
    // name and not so helpfully renamed it for us.
    integer indy;
    for (indy = llStringLength(to_chop_up) - 1; indy > space_v_posn; indy--) {
//log_it("indy=" + (string)space_v_posn);
        if (llGetSubString(to_chop_up, indy, indy) == " ") {
            // found one; zap it.  since we're going backwards we don't need to
            // adjust the loop at all.
            to_chop_up = llDeleteSubString(to_chop_up, indy, indy);
//log_it("saw case of previously redundant item, aieee.  flattened: " + to_chop_up);
        }
    }
    string full_suffix = llGetSubString(to_chop_up, space_v_posn, -1);
    // ditch the space character for our numerical check.
    string chop_suffix = llGetSubString(full_suffix, 1, llStringLength(full_suffix) - 1);
    // strip out a 'v' if there is one.
    if (llGetSubString(chop_suffix, 0, 0) == "v")
        chop_suffix = llGetSubString(chop_suffix, 1, llStringLength(chop_suffix) - 1);
    // if valid floating point number and greater than zero, that works for our version.
    string basename = to_chop_up;  // script name with no version attached.
    if ((float)chop_suffix > 0.0) {
        // this is a big success right here.
        basename = llGetSubString(to_chop_up, 0, -llStringLength(full_suffix) - 1);
        return [ basename, chop_suffix ];
    }
    // seems like we found nothing useful.
    return [];
}
//
//////////////

// end hufflets.
//////////////

default {
    state_entry() { if (llSubStringIndex(llGetObjectName(), "huffotronic") < 0) state real_default; }
    on_rez(integer parm) { state rerun; }
}
state rerun { state_entry() { state default; } }

state real_default
{
    state_entry() {
        auto_retire();
        startup_initialize();
    }
    
    state_exit() {
        llSetTimerEvent(0);
    }
    
    // we don't do anything until we're given a command to read a notecard.
    link_message(integer which, integer num, string msg, key id) 
    {
        if (num != NOTEWORTHY_HUFFWARE_ID) return;  // not for us.
        handle_link_message(which, num, msg, id);
    }
    
    on_rez(integer parm) { state rerun; }
    
    timer() {
        llSetTimerEvent(0.0);  // stop any timer now.
        // let the caller know this has failed out.
//        if (DEBUGGING) log_it("time out processing '" + requested_signature + "'");
        send_reply_and_reset(LINK_THIS, [ BAD_NOTECARD_INDICATOR, current_response_code ],
            READ_NOTECARD_COMMAND, FALSE);
        current_response_code = 0;  // we gave up on that one.
        dequeue_next_request();  // get next reading started if we have anything to read.
    }

    dataserver(key query_id, string data) {
        // make sure this data is for us.
        if (global_query_id != query_id) return;
        // yep, seems to be.
        process_notecard_line(query_id, data);
    }
}



