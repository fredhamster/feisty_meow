
// huffware script: slate reader, by fred huffhines
//
// uses the noteworthy library to read a set of notecards.  the list of notecards
// can be queried, and the current notecard's contents can be read aloud or sent
// to a link as link messages.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//

// global constants...

string SLATE_SIGNATURE = "#slate";
    // the notecard must begin with this as its first line for it to be
    // recognized as our configuration card.

// global variables...

integer response_code;  // set to uniquely identify the notecard read in progress.
integer global_link_num;  // which prim number to deliver the notecard contents to.

// implements an API for moving between the active notecards.
//////////////
// do not redefine these constants.
integer SLATE_READER_HUFFWARE_ID = 10028;
    // the unique id within the huffware system for the noteworthy script to
    // accept commands on.  this is used in llMessageLinked as the num parameter.
string HUFFWARE_PARM_SEPARATOR = "{~~~}";
    // this pattern is an uncommon thing to see in text, so we use it to separate
    // our commands in link messages.
string HUFFWARE_ITEM_SEPARATOR = "{|||}";
    // used to separate lists of items from each other when stored inside a parameter.
    // this allows lists to be passed as single string parameters if needed.
integer REPLY_DISTANCE = 100008;  // offset added to service's huffware id in reply IDs.
//////////////
string RESET_SLATE_READER_COMMAND = "#rsslt";
    // causes the notecard information to be forgotten and the script restarted.
string SR_GET_INFORMATION_COMMAND = "#infy";
    // used by clients to ask for information about the current number of notecards
    // available, and their names.  this information is sent back on the huffware ID
    // plus the reply distance.  first parm is the number, and the rest are the names.
string SR_PLAY_CARD_COMMAND = "#playvo";
    // picks a particular notecard for reading and send the notecard's contents in a
    // series of link messages, using this command and the reply distance.  there are
    // two parameters: an integer for the notecard number to read (from 0 through the
    // number of notecards - 1) and the link number to send the messages to.
//////////////

// requires noteworthy library v10.4 or better.
//////////////
// do not redefine these constants.
integer NOTEWORTHY_HUFFWARE_ID = 10010;
    // the unique id within the huffware system for the noteworthy script to
    // accept commands on.  this is used in llMessageLinked as the num parameter.
//////////////
string BAD_NOTECARD_INDICATOR = "bad_notecard";
    // indicates that the notecard reading process has failed to find an appropriate one.
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

// this function fires off a request to the noteworthy library via a link message.
// noteworthy will look for a notecard with our particular signature in it and
// if it finds one, it will read the configuration therein.  an empty string is
// returned if noteworthy couldn't find anything.
request_configuration(string note_name)
{
    // try to find a notecard with our configuration.
    response_code = -1 * (integer)randomize_within_range(23, 80000, FALSE);
    string parms_sent = wrap_parameters([SLATE_SIGNATURE, response_code, note_name]);
    llMessageLinked(LINK_THIS, NOTEWORTHY_HUFFWARE_ID, READ_SPECIFIC_NOTECARD_COMMAND,
         parms_sent);
}

// provides the functions of the slate reader API.
process_slate_requests(string msg, key id)
{
    if (msg == RESET_SLATE_READER_COMMAND) {
        llResetScript();
    } else if (msg == SR_GET_INFORMATION_COMMAND) {
        list parms = [ llGetInventoryNumber(INVENTORY_NOTECARD) ];
        integer indy;
        for (indy = 0; indy < llGetInventoryNumber(INVENTORY_NOTECARD); indy++) {
            parms += llGetInventoryName(INVENTORY_NOTECARD, indy);
        }
        llMessageLinked(LINK_THIS, SLATE_READER_HUFFWARE_ID + REPLY_DISTANCE, msg,
            wrap_parameters(parms));
    } else if (msg == SR_PLAY_CARD_COMMAND) {
        list parms = llParseString2List(id, [HUFFWARE_PARM_SEPARATOR], []);
        integer note_number = llList2Integer(parms, 0);
        integer link_number = llList2Integer(parms, 1);
        initialize_reader(note_number, link_number);
    }
}

// processes link messages received from the noteworthy library.
handle_link_message(integer which, integer num, string msg, key id)
{
    if (num == SLATE_READER_HUFFWARE_ID) {
        process_slate_requests(msg, id);
        return;
    }
    if ( (num != NOTEWORTHY_HUFFWARE_ID + REPLY_DISTANCE)
            || (msg != READ_NOTECARD_COMMAND) )
        return;  // not for us.
    // process the result of reading the notecard.
    list parms = llParseString2List(id, [HUFFWARE_PARM_SEPARATOR], []);
    string notecard_name = llList2String(parms, 0);
    integer response_for = llList2Integer(parms, 1);
    if (response_for != response_code) return;  // oops, this isn't for us.
    if (notecard_name == BAD_NOTECARD_INDICATOR) {
        // we hated the notecards we found, or there were none.
        log_it("We apologize; there seem to be no notecards with a first line of '"
            + SLATE_SIGNATURE
            + "'.  We can't read any configuration until that situation improves.");
    } else {
        // snag all but the first two elements for our config now.
        list config_list = llList2List(parms, 2, -1);
        // a valid notecard has been found.
        integer lines_to_say = llGetListLength(config_list);
        integer indy;
        for (indy = 0; indy < lines_to_say; indy++) {
            string line = llList2String(config_list, indy);
            if (!is_prefix(line, "#")) {
                llMessageLinked(global_link_num, SLATE_READER_HUFFWARE_ID + REPLY_DISTANCE,
                    SR_PLAY_CARD_COMMAND, line);
            }
        }
    }
}

///////////////

initialize_reader(integer note_number, integer link_number)
{
    // reset our relevant variables.
    global_link_num = link_number;

    if (note_number >= llGetInventoryNumber(INVENTORY_NOTECARD)) {
        llSay(0, "Cannot initialize reader to notecard number " + (string)note_number + " because that is out of range.");
        return;
    }
    string notecard_name = llGetInventoryName(INVENTORY_NOTECARD, note_number);

    // request that the noteworthy library start looking for our notecard.
    request_configuration(notecard_name);
}

//////////////
// from hufflets...
//

integer debug_num = 0;

// a debugging output method.  can be disabled entirely in one place.
log_it(string to_say)
{
    debug_num++;
    // tell this to the owner.    
    llOwnerSay(llGetScriptName() + "[" + (string)debug_num + "] " + to_say);
    // say this on an unusual channel for chat if it's not intended for general public.
//    llSay(108, llGetScriptName() + "[" + (string)debug_num + "] " + to_say);
    // say this on open chat that anyone can hear.  we take off the bling for this one.
//    llSay(0, to_say);
}

// locates the string "text" in the list to "search_in".
integer find_in_list(list search_in, string text)
{ 
    integer len = llGetListLength(search_in);
    integer i; 
    for (i = 0; i < len; i++) { 
        if (llList2String(search_in, i) == text) 
            return i; 
    } 
    return -1;
}

// returns TRUE if the "prefix" string is the first part of "compare_with".
integer is_prefix(string compare_with, string prefix)
{ return (llSubStringIndex(compare_with, prefix) == 0); }

// returns a number at most "maximum" and at least "minimum".
// if "allow_negative" is TRUE, then the return may be positive or negative.
float randomize_within_range(float minimum, float maximum, integer allow_negative)
{
    if (minimum > maximum) {
        // flip the two if they are reversed.
        float temp = minimum; minimum = maximum; maximum = temp;
    }
    float to_return = minimum + llFrand(maximum - minimum);
    if (allow_negative) {
        if (llFrand(1.0) < 0.5) to_return *= -1.0;
    }
    return to_return;
}

// strips the spaces off of the beginning and end of a string.
string strip_spaces(string to_strip)
{
    // clean out initial spaces.
    while (llGetSubString(to_strip, 0, 0) == " ")
        to_strip = llDeleteSubString(to_strip, 0, 0);
    // clean out ending spaces.
    while (llGetSubString(to_strip, -1, -1) == " ")
        to_strip = llDeleteSubString(to_strip, -1, -1);
    return to_strip;
}

// locates the item with "name_to_find" in the inventory items with the "type".
// a value from 0 to N-1 is returned if it's found, where N is the number of
// items in the inventory.
integer find_in_inventory(string name_to_find, integer inv_type)
{
    integer num_inv = llGetInventoryNumber(inv_type);
    if (num_inv == 0) return -1;  // nothing there!
    integer inv;
    for (inv = 0; inv < num_inv; inv++) {
        if (llGetInventoryName(inv_type, inv) == name_to_find)
            return inv;
    }
    return -2;  // failed to find it.
}

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

// end hufflets
//////////////

default {
    state_entry() { if (llSubStringIndex(llGetObjectName(), "huffotronic") < 0) state real_default; }
    on_rez(integer parm) { state rerun; }
}
state rerun { state_entry() { state default; } }

state real_default
{
    state_entry()
    {
        auto_retire();
    }
    
//    on_rez(integer parm) { llResetScript(); }
    
    // process the response from the noteworthy library.
    link_message(integer which, integer num, string msg, key id)
    { handle_link_message(which, num, msg, id); }
}
