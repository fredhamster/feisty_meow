
// huffware script: card configurator, by fred huffhines.
//
// processes a notecard with configuration info, then sends the information as packages of
// configuration nuggest to a consuming script.  this is one level above the noteworthy script,
// which simply reads the notecard.  this script keeps the config parsing out of higher-level
// scripts.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//

// global constants...

integer MAXIMUM_ITEMS_PER_BURST = 6;
    // try this out as a good limit for the size of the sends.

// global variables...

string signature_required;
    // sent to us when the request for configuration info is made.

list good_prefixes;
    // similarly, a set of prefixes for notecard lines that the calling script
    // cares about.  if this is empty, then anything will match.

string global_notecard_name;  // name of our notecard in the object's inventory.
integer response_code;  // set to uniquely identify the notecard read in progress.
list global_config_list;  // a collection of configuration parameters from our notecard.
list global_responses;  // the items we want to send back to the requestor.

// card configurator link message API:
//////////////
// do not redefine these constants.
integer CARD_CONFIGURATOR_HUFFWARE_ID = 10042;
    // the unique id within the huffware system for the card configurator script to
    // accept commands on.  this is used in llMessageLinked as the num parameter.
string HUFFWARE_PARM_SEPARATOR = "{~~~}";
    // this pattern is an uncommon thing to see in text, so we use it to separate
    // our commands in link messages.
string HUFFWARE_ITEM_SEPARATOR = "{|||}";
    // used to separate lists of items from each other when stored inside a parameter.
    // this allows lists to be passed as single string parameters if needed.
integer REPLY_DISTANCE = 100008;  // offset added to service's huffware id in reply IDs.
//////////////
string BAD_NOTECARD_TEXT = "*badcrd*";
    // the sign that we hated the notecards we found, or there were none.
string FINISHED_READING_NOTECARDS = "**finished**";
    // the sign that we are done plowing through the card we found.
string BEGIN_READING_NOTECARD_COMMAND = "#read-cfg#";
    // requests that the configurator find a good notecard and read its contents.
    // it should send the contents via the alert below.  first parm is the signature and
    // second is the wrapped list of valid item prefixes.
string READ_PARTICULAR_NOTECARD_COMMAND = "#read-note#";
    // requests that the configurator find a good notecard and read its contents.
    // it should send the contents via the alert below.  first two parms are the same as
    // begin reading notecard, and the third parameter is the name of the specific notecard.
string CARD_CONFIG_RECEIVED_ALERT = "#cfg-event-upd#";
    // this message is sent when the configurator has found some data updates or has finished
    // reading the configuration file.
//////////////

// imported interfaces below...

// requires noteworthy library v8.5 or better.
//////////////
// do not redefine these constants.
integer NOTEWORTHY_HUFFWARE_ID = 10010;
    // the unique id within the huffware system for the noteworthy script to
    // accept commands on.  this is used in llMessageLinked as the num parameter.
// commands available via the noteworthy library:
string BUSY_READING_INDICATOR = "busy_already";
    // this return value indicates that the script is already in use by some other script.
    // the calling script should try again later.
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
request_configuration()
{
    global_notecard_name = "";  // reset any previous card.
    // try to find a notecard with our configuration.
    response_code = -1 * (integer)randomize_within_range(23, 80000, FALSE);
    string parms_sent = wrap_parameters([signature_required, response_code]);
    llMessageLinked(LINK_THIS, NOTEWORTHY_HUFFWARE_ID, READ_NOTECARD_COMMAND,
         parms_sent);
}

request_specific_configuration()
{
    // try to find a notecard with our configuration.
    response_code = -1 * (integer)randomize_within_range(23, 80000, FALSE);
    string parms_sent = wrap_parameters([signature_required, response_code, global_notecard_name]);
    llMessageLinked(LINK_THIS, NOTEWORTHY_HUFFWARE_ID, READ_SPECIFIC_NOTECARD_COMMAND,
         parms_sent);
}

// processes link messages received from the noteworthy library.
process_notecard(integer which, integer num, string msg, key id)
{
    if (msg != READ_NOTECARD_COMMAND) return;  // not for us.
    // process the result of reading the notecard.
    list parms = llParseString2List(id, [HUFFWARE_PARM_SEPARATOR], []);
    string notecard_name = llList2String(parms, 0);
    integer response_for = llList2Integer(parms, 1);
    if (response_for != response_code) return;  // oops, this isn't for us.
    if (notecard_name != BAD_NOTECARD_TEXT) {
        // a valid notecard has been found.
        global_notecard_name = notecard_name;  // record its name for later use.
        // snag all but the first two elements for our config now.
        global_config_list = llList2List(parms, 2, -1);
        parms = [];  // toss the old copy.
        // and process the file as a set of definitions.
        process_ini_config();
        send_data_burst();  // send any pending configs out.
        if (notecard_name != NOTECARD_READ_CONTINUATION) {
//log_it("sending final sentinel at end of card.");
            // blast out a fake data burst that means we're done reading.
            global_notecard_name = FINISHED_READING_NOTECARDS;
            global_responses += [ FINISHED_READING_NOTECARDS ];
            send_data_burst();
        }
//log_it("after config read, memory left=" + (string)llGetFreeMemory());
    } else {
        // we hated the notecards we found, or there were none.
        // send a failure response.
        llMessageLinked(LINK_THIS, CARD_CONFIGURATOR_HUFFWARE_ID + REPLY_DISTANCE,
            CARD_CONFIG_RECEIVED_ALERT, wrap_parameters([BAD_NOTECARD_TEXT]));
    }
}

///////////////

// sends the currently held data out to whoever requested it.
send_data_burst()
{
    if (!llGetListLength(global_responses)) return;  // nothing to send.
//log_it("sending " + (string)llGetListLength(global_responses) + " items");
    llMessageLinked(LINK_THIS, CARD_CONFIGURATOR_HUFFWARE_ID + REPLY_DISTANCE, CARD_CONFIG_RECEIVED_ALERT,
        wrap_parameters([global_notecard_name] + global_responses));
    global_responses = [];  // reset any items held.
}

// consumes the notecard in a very application specific way to retrieve our configuration items.
// the example script only looks for two variables: name and description.  if those are found in
// the sample card, then they are proudly shown.
parse_variable_definition(string to_parse)
{
    string content;  // filled after finding a variable name.
    list x_y = separate_variable_definition(to_parse);
    string x = llList2String(x_y, 0);
    if (!llGetListLength(good_prefixes)) {
        global_responses += x_y;
    } else {
        integer indy;
        for (indy = 0; indy < llGetListLength(good_prefixes); indy++) {
            // if it's one of our desired prefixes, then we add it.
            if (is_prefix(x, llList2String(good_prefixes, indy))) {
                global_responses += x_y;
                indy = llGetListLength(good_prefixes) + 5;  // skip rest of loop.
            }
        }
    }
    
    if (llGetListLength(global_responses) > MAXIMUM_ITEMS_PER_BURST) {
        send_data_burst();
    }
}

// examines all entries that we got from the notecard to see if any contain definitions.
// this is basically an INI file reader, but it uses a list instead of a file.
// ini files provide a format with multiple sections of config information, like so:
//    [section_1]
//    name1=value1
//    name2=value2 ...etc...
//    [section_2]
//    name1=value1 ...etc...
process_ini_config()
{
//    log_it("scanning notecard for variable definitions...");
    integer indy;
    integer count = llGetListLength(global_config_list);
    // iterate across the items in our configuration to look for ones that are not done yet.
    for (indy = 0; indy < count; indy++) {
        string line = llList2String(global_config_list, indy);
//log_it("ini proc: " + line);
        // search for a section beginning.
        if (llGetSubString(line, 0, 0) == "[") {
            // we found the start of a section name.  now read the contents.
//            log_it("reading section: " + llGetSubString(line, 1, -2));
//            global_responses += [ line, "e" ];
            indy++;  // skip the section line.
        }
        integer sec_indy;
        for (sec_indy = indy; sec_indy < count; sec_indy++) {
            // read the lines in the section.
            line = llList2String(global_config_list, sec_indy);
            if (llGetSubString(line, 0, 0) != "[") {
                // try to interpret this line as a variable setting.  this is just
                // one example of a way to handle the config file; one might instead
                // want to do something below once a whole section is read.
                parse_variable_definition(line);
            } else {
                // we're at the beginning of a new section now, so start processing its
                // configuration in the outer loop.
                indy = sec_indy - 1;  // set indy to proper beginning of section.
                sec_indy = count + 3;  // skip remainder of inner loop.
            }
        }
        if (sec_indy == count) indy = count + 3;  // skip out of loop now.
    }
    global_config_list = [];  // we ate it!
}

initialize_specific(string specific_card)
{
    // reset our relevant variables.
    global_notecard_name = specific_card;
    global_config_list = [];
    global_responses = [];

    // request that the noteworthy library start looking for our notecard.
    request_specific_configuration();
}

initialize()
{
    // reset our relevant variables.
    global_notecard_name = "";
    global_config_list = [];
    global_responses = [];

    // request that the noteworthy library start looking for our notecard.
    request_configuration();
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
    // say this on open chat, but use an unusual channel.
//    llSay(108, llGetScriptName() + "[" + (string)debug_num + "] " + to_say);
}

integer find_substring(string full_string, string pattern)
{
    string full_lower = llToLower(full_string);
    return llSubStringIndex(full_lower, pattern);
}

//////////////

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

// parses a variable definition to find the name of the variable and its value.
// this returns two strings [X, Y], if "to_split" is in the form X=Y.
list separate_variable_definition(string to_split)
{
    integer equals_indy = llSubStringIndex(to_split, "=");
    // we don't support missing an equals sign, and we don't support it as the first character.
    if (equals_indy <= 0) return [];  // no match.
    string x = llGetSubString(to_split, 0, equals_indy - 1);
    string y = llGetSubString(to_split, equals_indy + 1, -1);
    to_split = "";  // save space.
///log_it("got x = " + x + " and y = " + y);
    return [ strip_spaces(x), strip_spaces(y) ];
}
//
// end hufflets
//////////////

//////////////
// huffware script: auto-retire, by fred huffhines, version 2.5.
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

default
{
    state_entry() { if (llSubStringIndex(llGetObjectName(),  "huffotronic") < 0) state real_default; }
    on_rez(integer parm) { state rerun; }
}
state rerun { state_entry() { state default; } }

state real_default
{
    state_entry()
    {
        auto_retire();  // make sure newest addition is only version of script.
    }

    on_rez(integer parm) { state rerun; }

    link_message(integer sender, integer num, string msg, key id) {
        if (num == NOTEWORTHY_HUFFWARE_ID + REPLY_DISTANCE) {
            if (msg == READ_NOTECARD_COMMAND) {
                process_notecard(sender, num, msg, id);
            }
            return;
        }
        
        if (num != CARD_CONFIGURATOR_HUFFWARE_ID) return;  // not even slightly for us.
        
        if (msg == BEGIN_READING_NOTECARD_COMMAND) {
            list parms = llParseString2List(id, [HUFFWARE_PARM_SEPARATOR], []);
            signature_required = llList2String(parms, 0);
            string prefixes_wrap = llList2String(parms, 1);
//log_it("pref raw is: " + prefixes_wrap);
            good_prefixes = llParseString2List(prefixes_wrap, [HUFFWARE_ITEM_SEPARATOR], []);
//log_it("signature to find: " + signature_required);
            initialize();
//log_it("prefixes are: " + (string)good_prefixes);
//log_it("began reading, memory left=" + (string)llGetFreeMemory());
            return;
        }
        if (msg == READ_PARTICULAR_NOTECARD_COMMAND) {
            list parms = llParseString2List(id, [HUFFWARE_PARM_SEPARATOR], []);
            signature_required = llList2String(parms, 0);
            string prefixes_wrap = llList2String(parms, 1);
            string specific_card = llList2String(parms, 2);
//log_it("pref raw is: " + prefixes_wrap);
            good_prefixes = llParseString2List(prefixes_wrap, [HUFFWARE_ITEM_SEPARATOR], []);
//log_it("signature to find: " + signature_required);
            initialize_specific(specific_card);
//log_it("prefixes are: " + (string)good_prefixes);
//log_it("notecard is: " + specific_card);
//log_it("began reading, memory left=" + (string)llGetFreeMemory());
            return;
        }
    }
}

