
// huffware script: jaunt config funcs, by fred huffhines, released under GPL license.
//
// this is the part of the jaunt wik rez ensemble that deals with configuration.
// it reads the configuration from notecards and passes it to the main script.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//

//////////////

integer DEBUGGING = FALSE;
    // make this true for noisy runtime diagnostics.

// defines the jaunt config funcs messaging API.
//////////////
// do not redefine these constants.
integer JAUNT_CONFIGURATION_HUFFWARE_ID = 10022;
    // the unique id within the huffware system for this script's commands.
    // it's used in llMessageLinked as the num parameter.
string HUFFWARE_PARM_SEPARATOR = "{~~~}";
    // this pattern is an uncommon thing to see in text, so we use it to separate
    // our commands in link messages.
string HUFFWARE_ITEM_SEPARATOR = "{|||}";
    // used to separate lists of items from each other when stored inside a parameter.
    // this allows lists to be passed as single string parameters if needed.
integer REPLY_DISTANCE = 100008;  // offset added to service's huffware id in reply IDs.
//////////////
// commands available from the jaunt config library:
string LOAD_ALL_CONFIGURATIONS = "#rdcfg#";
    // this starts the process of loading jaunt destinations from the notecards and
    // landmarks that we find in the object.  the parms are: private channel
    // and conveyance mode.
string JAUNT_CFG_DATA_EVENT = "#yodata#";
    // this event is generated from this script back to the caller.  when we have some
    // landmark data or configuration information, it's passed back in chunks.  the
    // deluge of pieces will continue until the JAUNT_CFG_EOF event is passed.  the
    // parameters include packed destination name and vector pairs (which form 2 elements
    // in the list) and packed config variable definitions (where config items start
    // with a colon, and the definition is only one element in the list).
string JAUNT_CFG_EOF = "#done#";
    // sent after all valid configuration items that we could find were processed.  there
    // are no parameters for this command.
//
//////////////

// important constants used internally...  these should generally not be changed.

// indicates a reconnaissance command when found as a prefix on chat text
// on our official recon channel.
string RECON_TEXT = "#rcn";
string READY_TEXT = "-y";  // lets us know that a jaunter is ready to be filled.
string RETURN_WORD = "-b";  // used to signal a returning recong jaunter.

integer MAXIMUM_BLAST_SIZE = 6;  // maximum number of config items per blurt.

integer TIMEOUT_FOR_CONFIGURATION_PROCESS = 42;
    // how long the notecard and lm reading processes are allowed to last.

integer TWO_WAY_TRIP = 1;
integer AUTOREZ_JAUNTER = 2;
integer ONE_WAY_TRIP = 3;
integer RECONNAISSANCE_TRIP = 4;

string DB_SEPARATOR = "``";  // separating items in lists.

//////////////

// global variables.

integer unused_private_chat_channel;  // where sub-jaunters communicate with root.

integer conveyance_mode;  //// = TWO_WAY_TRIP;
    // default is a standard two way trip, there and back again.  note that the object
    // will fail to return if any lands in between are blocking object entry.  those
    // situations should use an auto-rez jaunter, which is specified by prefixing the
    // target vectors with AR.

////integer current_target_index;  // the location in the list that we would currently jaunt to.

list config_to_blast;  // what we will send to our client.

// notecard variables...
integer response_code;  // set to uniquely identify the notecard read in progress.
string leftover_parms;  // items leftover during config.

// lm variables...
integer current_lm;
key lm_data_req;  // this is the current landmark being served as data.

//////////////

// requires noteworthy library v5.4 or higher.
//////////////
// do not redefine these constants.
integer NOTEWORTHY_HUFFWARE_ID = 10010;
    // the unique id within the huffware system for the noteworthy script to
    // accept commands on.  this is used in llMessageLinked as the num parameter.
string BAD_NOTECARD_INDICATOR = "bad_notecard";
    // indicates that the notecard reading process has failed to find an appropriate one.
//string BUSY_READING_INDICATOR = "busy_already";
    // this return value indicates that the script is already in use by some other script.
    // the calling script should try again later.
string NOTECARD_READ_CONTINUATION = "continue!";
    // returned as first parameter if there is still more data to handle.
// commands available via the noteworthy library:
string READ_NOTECARD_COMMAND = "#read_note#";
    // command used to tell the script to read notecards.  needs a signature to find
    // in the card as the only parameter.  the signature can be empty or missing.
    // the results will be fired back as the string value returned, which will have
    // an embedded list that was read from the notecard.  this necessarily limits the
    // size of the notecards that we can read and return.
//
//////////////

// begins getting configuration items from notecards and landmarks.
start_reading_configuration()
{
    llSetTimerEvent(0.0);  // cancel any existing timers.

    string JAUNT_SIGNATURE = "#jaunt";  // the expected first line of our notecards.

    // a normal startup where we read notecards and stuff...
    // see if we can load a notecard for destinations.
    response_code = random_channel();
    llMessageLinked(LINK_THIS, NOTEWORTHY_HUFFWARE_ID, READ_NOTECARD_COMMAND,
         wrap_parameters([JAUNT_SIGNATURE, response_code]));
    // we pick a private channel that fits in between our rez parm ranges.
    // during a normal trip, we will make sure the notecard reading doesn't stall.
    // the recon and one way jaunters don't need this; they're temporary.
    // we allow this number of seconds before notecard reader is awol.

    llSetTimerEvent(TIMEOUT_FOR_CONFIGURATION_PROCESS);
}

// main API implementor; answers requests to start reading config.
handle_config_request(string msg, string id)
{
    if (msg != LOAD_ALL_CONFIGURATIONS) return;  // not for us.
    list parms = llParseString2List(id, [HUFFWARE_PARM_SEPARATOR], []);
    // set our variables from the parameters we were given.
    unused_private_chat_channel = llList2Integer(parms, 0);
    conveyance_mode = llList2Integer(parms, 1);
    if ( (conveyance_mode != RECONNAISSANCE_TRIP) && (conveyance_mode != ONE_WAY_TRIP) ) {
        // setting up a root jaunter.
        start_reading_configuration();
    }
}

// sends out our current configuration perhaps.  if the check_length flag is true,
// then we'll only send if the list is already too fat.  if the flag is false, then
// we always just send our config (if it's non-empty).
send_config_blast(integer check_length)
{
    if (!check_length || (llGetListLength(config_to_blast) > MAXIMUM_BLAST_SIZE) ) {
        if (llGetListLength(config_to_blast) > 0) {
            llMessageLinked(LINK_THIS, JAUNT_CONFIGURATION_HUFFWARE_ID + REPLY_DISTANCE,
                JAUNT_CFG_DATA_EVENT, wrap_parameters(config_to_blast));
            config_to_blast = [];
        }
    }
}

// deals with responses from the notecard library.
process_notecard_lines(string msg, string id)
{
    list parms = llParseString2List(id, [HUFFWARE_PARM_SEPARATOR], []);
    if (llList2Integer(parms, 1) != response_code) return;  // this isn't for us.

    integer totally_done = TRUE;  // if left set to true, then notecard is done being consumed.
    
    string note_name = llList2String(parms, 0);
    if (DEBUGGING) log_it("note name: " + note_name);
    parms = llDeleteSubList(parms, 0, 1);
    
    // add in the leftovers.
    if (leftover_parms != "") {
        if (DEBUGGING) log_it("added leftover: " + leftover_parms);
        parms = [ leftover_parms ] + parms;
        leftover_parms = "";
    }

    if (note_name == NOTECARD_READ_CONTINUATION) {
        totally_done = FALSE;
        if (DEBUGGING) log_it("not totally done reading yet.");
    }
    if (note_name != BAD_NOTECARD_INDICATOR) {
        integer indy;
        for (indy = 0; indy < llGetListLength(parms); indy++) {
            if (DEBUGGING) log_it("index: " + (string)indy);
            string targ = llList2String(parms, indy);
            if (DEBUGGING) log_it("targ: " + targ);
            if (is_prefix(targ, ":")) {
                if (DEBUGGING) log_it("targ is special command.  just adding.");
                config_to_blast += [ targ ];
            } else {
                if (indy < llGetListLength(parms) - 1) {
                    if (DEBUGGING) log_it("decided we can steal two items.");
                    // this looks like a normal pair of destination vector and name.
                    string the_name = prune_name(llList2String(parms, indy + 1));
                    config_to_blast += [ the_name, targ ];
                    if (DEBUGGING) log_it("stole: " + the_name + " --> " + targ);
                    indy++;  // skip an extra index since we stole two.
                } else {
                    // save this tidbit for next config cycle.
                    if (DEBUGGING) log_it("saving tidbit for next round: " + targ);
                    leftover_parms = targ;
                }
            }
            send_config_blast(TRUE);
        }
    }

    if (totally_done) {
        // now that we've gotten our notecard read in, check the landmarks in
        // the inventory to see if they're useful.
        current_lm = 0;
        if (current_lm < llGetInventoryNumber(INVENTORY_LANDMARK)) {
            lm_data_req = llRequestInventoryData(
                llGetInventoryName(INVENTORY_LANDMARK, current_lm));
        } else {
            // kick out of this state and signal completion.
            llSetTimerEvent(0.001);
        }
    }
}

// report our getting configured properly.
completed_configuration()
{
    send_config_blast(FALSE);  // get last of config bits out to the client.
    llMessageLinked(LINK_THIS, JAUNT_CONFIGURATION_HUFFWARE_ID + REPLY_DISTANCE,
        JAUNT_CFG_EOF, NULL_KEY);
}

// fixes a location name so we don't get crushed on memory usage.
string prune_name(string to_prune)
{
    integer MAX_DEST_NAME = 28;  // the longest name that we store.

    // see if we can strip off the locational info.  comma is a common separator for stuff that's
    // not part of the lm name.
    integer indy = find_substring(to_prune, ",");
    if (indy > 5) to_prune = llGetSubString(to_prune, 0, indy - 1);
    // parentheses are another common way of adding extra stuff we don't need for the name.
    indy = find_substring(to_prune, "(");
    if (indy > 5) to_prune = llGetSubString(to_prune, 0, indy - 1);
    // crop the name on general length issues too.
    to_prune = llGetSubString(to_prune, 0, MAX_DEST_NAME);
    return to_prune;
}

// called when dataserver brings us landmark info.
process_landmark_data(key id, string data)
{
    if (id != lm_data_req) return;  // not for us.
    lm_data_req = NULL_KEY;  // drop our prior key.
    if (data != EOF) {
        string dest = vector_chop((vector)data);
        config_to_blast += [ prune_name(llGetInventoryName(INVENTORY_LANDMARK, current_lm)), dest ];
        send_config_blast(TRUE);
        current_lm++;
        if (current_lm < llGetInventoryNumber(INVENTORY_LANDMARK)) {
            lm_data_req = llRequestInventoryData(
                llGetInventoryName(INVENTORY_LANDMARK, current_lm));
        }
    }
    // check our sentinel data key about being done.
    if (lm_data_req == NULL_KEY) {
        // looks like we got our destinations and are totally done now.
        llSetTimerEvent(0.001);  // trigger state change very shortly.
    }
}

//////////////

//////////////
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

///////////////

// returns TRUE if the value in "to_check" specifies a legal x or y value in a sim.
integer valid_sim_value(float to_check)
{
    if (to_check < 0.0) return FALSE;
    if (to_check >= 257.0) return FALSE;
    return TRUE;
}

// returns TRUE if the "to_check" vector is a location outside of the current sim.
integer outside_of_sim(vector to_check)
{
    return !valid_sim_value(to_check.x) || !valid_sim_value(to_check.y);
}

// returns text for a floating point number, but includes only
// three digits after the decimal point.
string float_chop(float to_show)
{
    integer mant = llAbs(llRound(to_show * 1000.0) / 1000);
    string neg_sign;
    if (to_show < 0.0) neg_sign = "-";
    string dec_s = (string)((llRound(to_show * 1000.0) - mant * 1000) / 1000.0);
    dec_s = llGetSubString(llGetSubString(dec_s, find_substring(dec_s, ".") + 1, -1), 0, 2);
    // strip off all trailing zeros.
    while (llGetSubString(dec_s, -1, -1) == "0")
        dec_s = llDeleteSubString(dec_s, -1, -1);
    string to_return = neg_sign + (string)mant;
    if (llStringLength(dec_s)) to_return += "." + dec_s;
    return to_return;
}

// returns a prettier form for vector text, with chopped floats.
string vector_chop(vector to_show)
{
    return "<" + float_chop(to_show.x) + ","
        + float_chop(to_show.y) + ","
        + float_chop(to_show.z) + ">";
}

// joins a list of parameters using the parameter sentinel for the library.
string wrap_parameters(list to_flatten)
{ return llDumpList2String(to_flatten, HUFFWARE_PARM_SEPARATOR); }
//
// joins a list of sub-items using the item sentinel for the library.
string wrap_item_list(list to_wrap)
{ return llDumpList2String(to_wrap, HUFFWARE_ITEM_SEPARATOR); }

integer random_channel() { return -(integer)(llFrand(40000) + 20000); }

// returns the portion of the list between start and end, but only if they are
// valid compared with the list length.  an attempt to use negative start or
// end values also returns a blank list.
list chop_list(list to_chop, integer start, integer end)
{
    integer last_len = llGetListLength(to_chop) - 1;
    if ( (start < 0) || (end < 0) || (start > last_len) || (end > last_len) ) return [];
    return llList2List(to_chop, start, end);
}

// returns the index of the first occurrence of "pattern" inside
// the "full_string".  if it is not found, then a negative number is returned.
integer find_substring(string full_string, string pattern)
{
    string full_lower = llToLower(full_string);
    return llSubStringIndex(full_lower, pattern);
}

// returns TRUE if the "prefix" string is the first part of "compare_with".
integer is_prefix(string compare_with, string prefix)
{ return find_substring(compare_with, prefix) == 0; }

//////////////
// huffware script: auto-retire, by fred huffhines, version 2.8.
// distributed under BSD-like license.
//   !!  keep in mind that this code must be *copied* into another
//   !!  script that you wish to add auto-retirement capability to.
// when a script has auto_retire in it, it can be dropped into an
// object and the most recent version of the script will destroy
// all older versions.
//
// the version numbers are embedded into the script names themselves.
// the notation for versions uses a letter 'v', followed by two numbers
// in the form "major.minor".
// major and minor versions are implicitly considered as a floating point
// number that increases with each newer version of the script.  thus,
// "hazmap v0.1" might be the first script in the "hazmap" script continuum,
// and "hazmap v3.2" is a more recent version.
//
// example usage of the auto-retirement script:
//     default {
//         state_entry() {
//            auto_retire();  // make sure newest addition is only version of script.
//        }
//     }
// this script is partly based on the self-upgrading scripts from markov brodsky
// and jippen faddoul.
//////////////
auto_retire() {
    string self = llGetScriptName();  // the name of this script.
    list split = compute_basename_and_version(self);
    if (llGetListLength(split) != 2) return;  // nothing to do for this script.
    string basename = llList2String(split, 0);  // script name with no version attached.
    string version_string = llList2String(split, 1);  // the version found.
    integer posn;
    // find any scripts that match the basename.  they are variants of this script.
    for (posn = llGetInventoryNumber(INVENTORY_SCRIPT) - 1; posn >= 0; posn--) {
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
    }
    if (space_v_posn < 2) return [];  // no space found.
    // now we zoom through the stuff after our beloved v character and find any evil
    // space characters, which are most likely from SL having found a duplicate item
    // name and not so helpfully renamed it for us.
    integer indy;
    for (indy = llStringLength(to_chop_up) - 1; indy > space_v_posn; indy--) {
        if (llGetSubString(to_chop_up, indy, indy) == " ") {
            // found one; zap it.  since we're going backwards we don't need to
            // adjust the loop at all.
            to_chop_up = llDeleteSubString(to_chop_up, indy, indy);
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

//////////////

// default state scrounges for information in a notecard and looks for landmarks in
// inventory to add as destinations.
default
{
    state_entry() { if (llSubStringIndex(llGetObjectName(),  "huffotronic") < 0) state real_default; }
    on_rez(integer parm) { state rerun; }
}
state rerun { state_entry() { state default; } }

state real_default
{
    state_entry() { auto_retire(); }

    link_message(integer which, integer num, string msg, key id)
    {
        if (num == NOTEWORTHY_HUFFWARE_ID + REPLY_DISTANCE) {
            process_notecard_lines(msg, id);
        } else if (num == JAUNT_CONFIGURATION_HUFFWARE_ID) {
            handle_config_request(msg, id);
        }
    }
    
    dataserver(key id, string data) { process_landmark_data(id, data); }

    timer() {
        // this is the only place we publish a result.
        completed_configuration();
        llSetTimerEvent(0.0);  // stop the timer again, until next request.
    }
}

