
// huffware script: comfortable sitting, by fred huffhines.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//

//////////////

// global constants...

string NOTEWORTHY_SIGNATURE = "#sitting";
    // the notecard must begin with this as its first line for it to be
    // recognized as our configuration card.

//////////////

// global variables that are loaded from a notecard.

vector AVATAR_ROTATION = <0, 0, -90>;  // star chair.
//vector AVATAR_ROTATION = <0, 0, 0>;
    // the euler vector for rotation of the avatar after sitting, in degrees.
    // the rotation vector should be tailored to the object.

vector AVATAR_POSITION = <-0.1, -0.28, -0.1>;  // star chair.
//vector AVATAR_POSITION = <0.34, 0, 0>;
    // the position of the sitting offset from the object center.   also needs to be
    // tailored to the particular object.

integer GOTO_WHICH_FLOOR = 1;
    // when serving as an elevator, this is a way to cause the teleport offset
    // to go to a particular floor.  the real z position is calculated from
    // this times the floor size in meters.  note that floors are numbered
    // starting at 1 (one).
    
float FLOOR_SIZE_IN_METERS = 0.0;
    // when the script is used in an elevator, this specifies the height of the
    // floors.  our current scheme will only work if that is constant between
    // the floors.

float BASE_FLOOR_HEIGHT = 0.0;
    // the position of the first floor in meters.  this will not affect the
    // position calculations unless floor size is non-zero.

integer UNSEAT_AFTERWARDS = FALSE;
    // if this is true, then the avatar is unseated just after sitting down.
    
float PAUSE_BEFORE_EVICTION = 0.28;
    // the number of seconds that the avatars get to sit before we pop them
    // out of the chair/teleporter/whatever.

vector CAMERA_EYE_OFFSET = <3, 2, 1.5>;  // star chair.
    // the offset for the camera after the avatar sits down.
//relative to the avatar?

vector CAMERA_TARGET = <-3, 0, 1>;  // star chair.
    // the location where the camera is looking at once the avatar sits down.
//relative to the avatar?

//////////////

// global variables used in processing notecards...

integer pending_response_id;  // set to uniquely identify the notecard read in progress.
list global_config_list;  // a collection of configuration parameters from our notecard.
integer global_config_index;  // allows wrap-around feature, which we don't use here.

//////////////

// interfaces for library scripts we rely on...

// requires noteworthy library.
// in this odd case, where we are trying to shrink script count, the noteworthy library
// is embedded inside here.
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

// establishes the sitting parameters including camera offsets.
setup_seating_arrangements()
{
    llUnSit(llAvatarOnSitTarget());  // no one gets to ignore a script change.        
    vector new_rot = AVATAR_ROTATION;
    new_rot *= DEG_TO_RAD;  // convert to radians.
    rotation quat = llEuler2Rot(new_rot);  // convert to quaternion.
    // get our position set up and take into account the elevator position.
    vector position = AVATAR_POSITION;
//rename that variable to be "which floor to go to"
    if (FLOOR_SIZE_IN_METERS != 0) {
        vector temp = llGetPos();
        integer my_floor = (integer) ((temp.z - BASE_FLOOR_HEIGHT ) / FLOOR_SIZE_IN_METERS) + 1;
//log_it("my floor is " + (string)my_floor);
        float add_in = (float)(GOTO_WHICH_FLOOR - my_floor) * FLOOR_SIZE_IN_METERS;
//log_it("decided to add in z: " + (string)add_in);
        position += <0, 0, add_in>;        
    }
    // also we make this absolute by taking out the object's own rotation.
    // it's hard enough in life for z components to not mean z axis.
    position /= llGetRot();
    llSitTarget(position, quat / llGetRot());
//hmmm: do we need same rot treatment on camera things?    
//hmmm: trying it.
    // now set the camera position to avoid having random viewpoint.
    llSetCameraEyeOffset(CAMERA_EYE_OFFSET / llGetRot());
    llSetCameraAtOffset(CAMERA_TARGET / llGetRot());
}

// this function fires off a request to the noteworthy library via a link message.
// noteworthy will look for a notecard with our particular signature in it and
// if it finds one, it will read the configuration therein.  an empty string is
// returned if noteworthy couldn't find anything.
request_configuration()
{
    // try to find a notecard with our configuration.
    pending_response_id = -1 * (integer)randomize_within_range(23, 80000, FALSE);
    string parms_sent = wrap_parameters([NOTEWORTHY_SIGNATURE, pending_response_id]);
//call direct into noteworthy.
noteworthy_handle_link_message(LINK_THIS, NOTEWORTHY_HUFFWARE_ID, READ_NOTECARD_COMMAND, parms_sent);
        
//    llMessageLinked(LINK_THIS, NOTEWORTHY_HUFFWARE_ID, READ_NOTECARD_COMMAND,
//         parms_sent);
}

// processes link messages received from the noteworthy library.
handle_link_message(integer which, integer num, string msg, key id)
{
    if ( (num != NOTEWORTHY_HUFFWARE_ID + REPLY_DISTANCE)
            || (msg != READ_NOTECARD_COMMAND) )
        return;  // not for us.
//log_it("handl: msg=" + msg + " id=" + id);
    // process the result of reading the notecard.
    list parms = llParseString2List(id, [HUFFWARE_PARM_SEPARATOR], []);
    string notecard_name = llList2String(parms, 0);
//log_it("raw is " + llList2String(parms, 1));
    integer response_for = llList2Integer(parms, 1);
//log_it("resp cod=" + pending_response_id + " but this for=" + response_for);
//if (response_for != pending_response_id) log_it("bad response code???");    
    if (response_for != pending_response_id) return;  // oops, this isn't for us.
    if (notecard_name == BAD_NOTECARD_INDICATOR) {
        // we hated the notecards we found, or there were none.
        log_it("We apologize; there seem to be no notecards with a first line of '"
            + NOTEWORTHY_SIGNATURE
            + "'.  We can't read any configuration until that situation improves.");
    } else {
//log_it("got to handle link");
        // snag all but the first two elements for our config now.
        global_config_list += llList2List(parms, 2, -1);
        // make sure we shouldn't keep going.
        if (notecard_name != NOTECARD_READ_CONTINUATION) {
            // a valid notecard has been found.
            global_config_index = 0;  // we are starting over in the config list.
//            log_it("read notecard \"" + notecard_name + "\":");
            // and process the file as a set of definitions.
            process_ini_config();
            // now that we have a new set of parameters, use them.
            setup_seating_arrangements();
        }
    }
}

///////////////

// consumes the notecard in a very application specific way to retrieve our configuration items.
// the example script only looks for two variables: name and description.  if those are found in
// the sample card, then they are proudly shown.
parse_variable_definition(string to_parse)
{
    string content;  // filled after finding a variable name.
    if ( (content = get_variable_value(to_parse, "avatar_rotation")) != "") {
        AVATAR_ROTATION = (vector)content;
//        log_it("** got avatar_rotation of '" + content + "'");
    } else if ( (content = get_variable_value(to_parse, "avatar_position")) != "") {
        AVATAR_POSITION = (vector)content;
//        log_it("** got avatar_position of '" + content + "'");
    } else if ( (content = get_variable_value(to_parse, "goto_which_floor")) != "") {
        GOTO_WHICH_FLOOR = (integer)content;
//        log_it("** got GOTO_WHICH_FLOOR of '" + content + "'");
    } else if ( (content = get_variable_value(to_parse, "floor_size_in_meters")) != "") {
        FLOOR_SIZE_IN_METERS = (float)content;
//        log_it("** got FLOOR_SIZE_IN_METERS of '" + content + "'");
    } else if ( (content = get_variable_value(to_parse, "base_floor_height")) != "") {
        BASE_FLOOR_HEIGHT = (float)content;
//        log_it("** got BASE_FLOOR_HEIGHT of '" + content + "'");
    } else if ( (content = get_variable_value(to_parse, "unseat_afterwards")) != "") {
        UNSEAT_AFTERWARDS = (integer)content;
//        log_it("** got unseat_afterwards of '" + content + "'");
    } else if ( (content = get_variable_value(to_parse, "pause_before_eviction")) != "") {
        PAUSE_BEFORE_EVICTION = (float)content;
//        log_it("** got pause_before_eviction of '" + content + "'");
    } else if ( (content = get_variable_value(to_parse, "camera_eye_offset")) != "") {
        CAMERA_EYE_OFFSET = (vector)content;
//        log_it("** got camera_eye_offset of '" + content + "'");
    } else if ( (content = get_variable_value(to_parse, "camera_target")) != "") {
        CAMERA_TARGET = (vector)content;
//        log_it("** got camera_target of '" + content + "'");
//    } else {
//        log_it("unknown variable seen: " + to_parse);
    }
}

initialize()
{
    // reset our relevant variables.
    global_config_list = [];
    global_config_index = 0;

    // announce that we're open for business.
///    log_it("started, free mem=" + (string)llGetFreeMemory());

    // request that the noteworthy library start looking for our notecard.
    request_configuration();
    
    integer indy = 32;
    while (indy >= 0) {
        indy = find_in_inventory_partial("noteworthy", INVENTORY_SCRIPT);
        if (indy >= 0) {
            llRemoveInventory(llGetInventoryName(INVENTORY_SCRIPT, indy));
        }
    }
    
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
    return [ strip_spaces(x), strip_spaces(y) ];
}

// returns a non-empty string if "to_check" defines a value for "variable_name".
// this must be in the form "X=Y", where X is the variable_name and Y is the value.
string get_variable_value(string to_check, string variable_name)
{
    list x_y = separate_variable_definition(to_check);
    if (llGetListLength(x_y) != 2) return "";  // failure to parse a variable def at all.
    if (!is_prefix(llList2String(x_y, 0), variable_name)) return "";  // no match.
    return llList2String(x_y, 1);  // a match!
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
    string section_name;  // set later if we see one.

    // iterate across the items in our configuration to look for ones that are not done yet.            
    for (indy = global_config_index; indy < count; indy++) {
        string line = llList2String(global_config_list, indy);
        // search for a section beginning.
        if (llGetSubString(line, 0, 0) == "[") {
            // we found the start of a section name.  now read the contents.
            indy++;  // skip section line.
            section_name = llGetSubString(line, 1, -2);
            log_it("reading section: " + section_name);
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
                indy = sec_indy;  // track that we've passed this line.
            } else {
                // we're at the beginning of a new section now, so start processing its
                // configuration in the outer loop.
                indy = sec_indy - 1;  // set indy to proper beginning of section.
                global_config_index = indy;  // remember where we had read to.
                sec_indy = count + 3;  // skip remainder of inner loop.
            }
        }
    }

    global_config_index = 0;  // reset outer position if want to re-read.
}

// locates the item with "name_to_find" in the inventory items with the "type".
// a value from 0 to N-1 is returned if it's found, where N is the number of
// items in the inventory.
integer find_in_inventory_partial(string name_to_find, integer inv_type)
{
    integer num_inv = llGetInventoryNumber(inv_type);
    if (num_inv == 0) return -1;  // nothing there!
    integer inv;
    for (inv = 0; inv < num_inv; inv++) {
        if (is_prefix(llGetInventoryName(inv_type, inv), name_to_find))
            return inv;
    }
    return -2;  // failed to find it.
}

//////////////
// huffware script: auto-retire, by fred huffhines, version 1.9.
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
//
// example usage of the auto-retirement script:
//
// default {
//    state_entry() {
//        auto_retire();  // make sure newest addition is only version of script.
//    }
// }
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
                if ((float)inv_version_string < (float)version_string) {
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
    if (llSubStringIndex(to_chop_up, " ") < 0) return [];  // no space found, not a valid name to work on.
        
    string basename = to_chop_up;  // script name with no version attached.
    
    integer posn;
    // minimum script name is 2 characters plus version.
    for (posn = llStringLength(to_chop_up) - 1;
        (posn >= 2) && (llGetSubString(to_chop_up, posn, posn) != " ");
        posn--) {
        // find the space.  do nothing else.
    }
    if (posn < 2) return [];  // no space found.
    string full_suffix = llGetSubString(to_chop_up, posn, -1);
    // ditch the space character for our numerical check.
    string chop_suffix = llGetSubString(full_suffix, 1, llStringLength(full_suffix) - 1);
    // strip out a 'v' if there is one.
    if (llGetSubString(chop_suffix, 0, 0) == "v")
        chop_suffix = llGetSubString(chop_suffix, 1, llStringLength(chop_suffix) - 1);
    // if valid floating point number and greater than zero, that works for our version.
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

//////////////
// from noteworthy:

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

float TIME_OUT_ON_ONE_NOTECARD = 120.0;
    // we allow one line of a notecard this long to be read before we decide it's hosed.
    // some sims are very very slow, and a time-out of one minute has been found lacking;
    // we saw at least one case where the first notecard line to be read took longer than
    // 60 seconds.  that's why we keep cranking this time-out up...

// constants that should not be changed...

// outcomes from handling a line in a notecard.
integer STILL_READING = -8;  // the notecard seems good, but isn't finished.
integer BAD_NOTECARD = -9;  // this notecard doesn't have our signature.
integer DONE_READING = -10;  // the notecard is finished being read.

integer LIST_ELEMENT_GUESS = 200;  // guess at how many bytes average list element uses.

integer MAXIMUM_LINES_USED = 4;
    // we will read this many lines at a time from the appropriate notecard.
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
    llSetTimerEvent(0.0);
    pending_signatures = [];
    pending_response_codes = [];
    pending_notecard_names = [];
    current_response_code = 0;
}

reset_for_reading(string signature, integer response_code_in)
{
    requested_signature = signature;
    current_response_code = response_code_in;
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
//log_it("froyo: curre code=" + current_response_code);
//integer items = llGetListLength(parms);
//if (include_query) items += llGetListLength(global_query_contents);
//log_it("pre-sending " + (string)items + " items, mem=" + (string)llGetFreeMemory());

   if (!include_query) {
        handle_link_message(destination, NOTEWORTHY_HUFFWARE_ID + REPLY_DISTANCE,
            command, llDumpList2String(parms, HUFFWARE_PARM_SEPARATOR));
    } else {
        handle_link_message(destination, NOTEWORTHY_HUFFWARE_ID + REPLY_DISTANCE,
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
//log_it("about to reset response code");
    current_response_code = 0;  // reset back to default so we can start another read.
    global_query_id = NULL_KEY;  // reset so we accept no more data.
}

// if there are other pending notecard reads, this goes to the next one listed.
dequeue_next_request()
{
    if (llGetListLength(pending_signatures)) {
        // get the info from the next pending item.
        string sig = llList2String(pending_signatures, 0);
        integer response_code_temp = llList2Integer(pending_response_codes, 0);
        string notecard = llList2String(pending_notecard_names, 0);
        // whack the head of the queue since we grabbed the info.
        pending_signatures = llDeleteSubList(pending_signatures, 0, 0);
        pending_response_codes = llDeleteSubList(pending_response_codes, 0, 0);
        pending_notecard_names = llDeleteSubList(pending_notecard_names, 0, 0);
        if (llStringLength(notecard)) {
            global_notecard_name = notecard;
            select_specific_notecard();
        } else {
            reset_for_reading(sig, response_code_temp);
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
noteworthy_handle_link_message(integer which, integer num, string msg, key id)
{
    if (num != NOTEWORTHY_HUFFWARE_ID) return;  // not for us.

    if (msg == READ_NOTECARD_COMMAND) {
        only_read_one_notecard = FALSE;  // general inquiry for any card.
        list parms = llParseString2List(id, [HUFFWARE_PARM_SEPARATOR], []);
//log_it("read notecard--parms are: " + (string)parms);
        string signature = llList2String(parms, 0);
        integer response_code_temp = llList2Integer(parms, 1);
//log_it("got signature " + signature + " and respcode " + (string)response_code_temp);
//holding:        if (!current_response_code) {
            // go ahead and process this request; we aren't busy.
            reset_for_reading(signature, response_code_temp);
            if (!try_next_notecard()) {
                if (DEBUGGING) log_it("failed to find any appropriate notecards at all.");
                send_reply_and_reset(LINK_THIS, [ BAD_NOTECARD_INDICATOR, response_code_temp ],
                    READ_NOTECARD_COMMAND, FALSE);
                return;
            }
//holding:        } else {
//holding:            // we're already busy.
//holding://            send_reply_and_reset(LINK_THIS, [ BUSY_READING_INDICATOR, response_code_temp ],
//holding://                READ_NOTECARD_COMMAND, FALSE);
//holding:            // stack this request; another is in progress.
//holding:            pending_signatures += signature;
//holding:            pending_response_codes += response_code_temp;
//holding:            pending_notecard_names += "";
//holding:        }
    } else if (msg == READ_SPECIFIC_NOTECARD_COMMAND) {
        only_read_one_notecard = TRUE;  // they want one particular card.
        list parms = llParseString2List(id, [HUFFWARE_PARM_SEPARATOR], []);
//log_it("read specific--parms are: " + (string)parms);
        string signature = llList2String(parms, 0);
        integer response_code_temp = llList2Integer(parms, 1);
        string notecard_name = llList2String(parms, 2);
//log_it("got signature " + signature + " and respcode " + (string)response_code_temp);
//holding:        if (!current_response_code) {
            // go ahead and process this request; we aren't busy.
            reset_for_reading(signature, response_code_temp);
            global_notecard_name = notecard_name;  // set our global.
            select_specific_notecard();
//holding:        } else {
//holding:            // we're already busy.
//holding://            send_reply_and_reset(LINK_THIS, [ BUSY_READING_INDICATOR, response_code_temp ],
//holding://                READ_NOTECARD_COMMAND, FALSE);
//holding:            // stack this request; another is in progress.
//holding:            pending_signatures += signature;
//holding:            pending_response_codes += response_code_temp;
//holding:            pending_notecard_names += notecard_name;
//holding:        }
    }
}

    
default
{
    state_entry() { if (llSubStringIndex(llGetObjectName(),  "huffotronic") < 0) state real_default; }
    on_rez(integer parm) { state rerun; }
}
state rerun { state_entry() { state default; } }

state real_default
{
    state_entry() {
        auto_retire();  // make sure newest addition is only version of script.
/////not needed now        llSleep(1.0);  // snooze just a bit to let noteworthy start up?
        startup_initialize();
        initialize();  // start asking about the notecards.
        setup_seating_arrangements();  // use our current defaults for sitting posn.
    }

    on_rez(integer parm) { llResetScript(); }

    changed(integer change) {
        if (change & CHANGED_INVENTORY) {
//            llSleep(3.14159265358);  // delay to avoid interfering with upgrade.
            llResetScript(); 
        }
        if (!(change & CHANGED_LINK)) return;  // don't care.
        if (!UNSEAT_AFTERWARDS) return;  // nothing else below is needed.
        if (llAvatarOnSitTarget() == NULL_KEY) return;  // no one there, so ditto.
        // now give them a bit of time to rest before dumping them.
        llSetTimerEvent(PAUSE_BEFORE_EVICTION);
    }
    
    timer() {
        if (current_response_code != 0) {
            llSetTimerEvent(0.0);  // stop any timer now.
            // let the caller know this has failed out.
            if (DEBUGGING) log_it("time out processing '" + requested_signature + "'");
            send_reply_and_reset(LINK_THIS, [ BAD_NOTECARD_INDICATOR, current_response_code ],
                READ_NOTECARD_COMMAND, FALSE);
            current_response_code = 0;  // we gave up on that one.
            dequeue_next_request();  // get next reading started if we have anything to read.
        } else {
            // perform short range teleport, effectively...
            llUnSit(llAvatarOnSitTarget());  // ha, got that guy back up.
            llSetTimerEvent(0.0);  // reset timer.
        }
    }

    // process the response from the noteworthy library.
    link_message(integer which, integer num, string msg, key id)
    { handle_link_message(which, num, msg, id); }

    dataserver(key query_id, string data) {
        // make sure this data is for us.
        if (global_query_id != query_id) return;
        // yep, seems to be.
        process_notecard_line(query_id, data);
    }            
}

