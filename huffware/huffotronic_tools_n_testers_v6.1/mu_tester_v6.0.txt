
// huffware script: mu tester, by fred huffhines.
//
// a script that puts the set comparator library through its paces
// while providing a textual method for creating and comparing sets.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//

// these are the commands a user can say:
string DEFINE_WORD = "create";
string DELETE_WORD = "delete";
string LIST_WORD = "list";
string GET_WORD = "get";
string ADD_TO_WORD = "add";
string CUT_FROM_WORD = "cut";
string INTERSECT_WORD = "intersect";
string UNION_WORD = "union";
string DIFFERENCE_WORD = "differ";
string MU_WORD = "mu";
string CLEAR_ALL_WORD = "clearall";
string RESET_WORD = "reset!";

integer NOISY_OY = FALSE;
  // produces more chatty logging when set to TRUE.  this is adjusted during
  // startup to be off.

// requires set comparator 2.8 or better.
// API for set operations.
//////////////
integer SET_COMPARATOR_HUFFWARE_ID = 10020;
    // a unique ID within the huffware system for this script.
string HUFFWARE_PARM_SEPARATOR = "{~~~}";
    // this pattern is an uncommon thing to see in text, so we use it to separate
    // our commands in link messages.
string HUFFWARE_ITEM_SEPARATOR = "{|||}";
    // used to separate lists of items from each other when stored inside a parameter.
    // this allows lists to be passed as single string parameters if needed.
integer REPLY_DISTANCE = 100008;  // offset added to service's huffware id in reply IDs.
//////////////
string DEFINE_SET_CMD = "#def-set";
    // adds a new set or replaces existing one with same name.  first parm is name of set,
    // second parm is a wrapped list of elements that should be in the set.  return value
    // is a boolean for success.
string REMOVE_SET_CMD = "#rm-set";
    // trashes a named set.  first parm is the name.  returns a bool for success.
string ADD_ELEMENTS_CMD = "#add-elem";
    // adds more elements to an existing set.  first parm is the name, second is a wrapped
    // list of new elements.  set must already exist.  returns a bool for success.
string CUT_ELEMENTS_CMD = "#cut-elem";
    // removes a set of elements from an existing set.  first parm is set name, second is
    // wrapped list of elements to remove.  set must already exist.  returns bool.
string INTERSECT_CMD = "#inter-set";
    // reports the set of elements in the intersection of two sets.  first and second parm
    // are the set names.  returns a wrapped list of elements that are common members of both sets.
string UNION_CMD = "#union-set";
    // returns the union of two named sets.  results sent similar to intersection.
string DIFFERENCE_CMD = "#diff-set";
    // returns the difference of set A (parm 1) minus set B (parm 2).  results are similar
    // to intersection.
string WHAT_MU_CMD = "#mu-set";
    // returns one of the possibility values below to describe the relationship between
    // two sets.
string GET_SET_CMD = "#get-set";
    // retrieves the contents of the set named in first parameter.
string LIST_SET_NAMES_CMD = "#whichunz";
    // retrieves the list of set names that exist.
string CLEAR_ALL_CMD = "#clearall";
    // throws out all set definitions.        
//////////////
// joins a list of parameters using the parameter sentinel for the library.
string wrap_parameters(list to_flatten)
{ return llDumpList2String(to_flatten, HUFFWARE_PARM_SEPARATOR); }
//////////////

// requires noteworthy library v8.4 or better.
//////////////
// do not redefine these constants.
integer NOTEWORTHY_HUFFWARE_ID = 10010;
    // the unique id within the huffware system for the noteworthy script to
    // accept commands on.  this is used in llMessageLinked as the num parameter.
// commands available via the noteworthy library:
string READ_NOTECARD_COMMAND = "#read_note#";
    // command used to tell the script to read notecards.  needs a signature to find
    // in the card as the first parameter, and a randomly generated response code for
    // the second parameter.  the response code is used to uniquely identify a set of
    // pending notecard readings (hopefully).  the signature can be empty or missing.
    // the results will be fired back as the string value returned, which will have
    // as first element the notecard's name (or "bad_notecard" if none was found) and
    // as subsequent elements an embedded list that was read from the notecard.  this
    // necessarily limits the size of the notecards that we can read and return.
string BAD_NOTECARD_INDICATOR = "bad_notecard";
    // indicates that the notecard reading process has failed to find an appropriate one.
string BUSY_READING_INDICATOR = "busy_already";
    // this return value indicates that the script is already in use by some other script.
    // the calling script should try again later.
string NOTECARD_READ_CONTINUATION = "continue!";
    // returned as first parameter if there is still more data to handle.
//
//////////////

string MU_SIGNATURE = "#mu";
    // the notecard must begin with this as its first line for it to be
    // recognized as our configuration card.

// global variables...

string global_notecard_name;  // name of our notecard in the object's inventory.
integer response_code;  // set to uniquely identify the notecard read in progress.
list global_config_list;  // a collection of configuration parameters from our notecard.
integer global_config_index;  // allows wrap-around feature, which we don't use here.

show_instructions()
{
    string c = ", ";  // speed of light.
    string help_text = "\nThis object will operate on a collection of sets and understands the following commands:\n";
    help_text += "  " + DEFINE_WORD + c + DELETE_WORD + c + CLEAR_ALL_WORD + c + ADD_TO_WORD + c
        + CUT_FROM_WORD + c + INTERSECT_WORD + c + UNION_WORD + c + DIFFERENCE_WORD + c + GET_WORD + c
        + LIST_WORD + c + RESET_WORD + c + "and " + MU_WORD + ".\n";
    help_text += "Generally each command takes a set name or two, and sometimes a list of items.\n";
    help_text += "If an item or set name has a space in it, surround it in quote characters.\n";
    help_text += "For Example:\n";
    help_text += "  " + DEFINE_WORD + " \"jed yo\" upsa dooba gorp : create a set named \"jed yo\" with three items.\n";
    help_text += "  " + INTERSECT_WORD + " jed pfaltzgraff : returns the intersection of the two sets.\n";
    help_text += "  " + ADD_TO_WORD + " foon a \"b c\" d e : adds four elements to the set \"foon\".\n";
    help_text += "(Free Memory=" + (string)llGetFreeMemory() + ")\n";
    llSay(0, help_text);
}

// makes a request of the set comparator.
send_command(string msg, list parms)
{ llMessageLinked(LINK_THIS, SET_COMPARATOR_HUFFWARE_ID, msg, wrap_parameters(parms)); }

// processes verbal commands.
hearing_some_voices(integer chan, string name, key id, string msg_in)
{
    // clean up the string before starting.
    msg_in = llStringTrim(msg_in, STRING_TRIM);  // remove leading and trailing spaces.
    msg_in = compress_spaces(msg_in);  // turn multiple spaces into a single one.
    // separate out the separate words in what was said.
    list members = parse_quoted_strings(msg_in);
//log_it("got membs as: " + dump_list(members));
    // break out the first few to make the below cases easier.
    string first_word = llList2String(members, 0);
    string second_word = llList2String(members, 1);
    string third_word = llList2String(members, 2);

    if (first_word == DEFINE_WORD) {
        send_command(DEFINE_SET_CMD, llDeleteSubList(members, 0, 0));
    } else if (first_word == DELETE_WORD) {
        send_command(REMOVE_SET_CMD, [ second_word ]);
    } else if (first_word == GET_WORD) {
        send_command(GET_SET_CMD, [ second_word ]);
    } else if (first_word == LIST_WORD) {
        send_command(LIST_SET_NAMES_CMD, []);
    } else if (first_word == ADD_TO_WORD) {
        send_command(ADD_ELEMENTS_CMD, llDeleteSubList(members, 0, 0));
    } else if (first_word == CUT_FROM_WORD) {
        send_command(CUT_ELEMENTS_CMD, llDeleteSubList(members, 0, 0));
    } else if (first_word == INTERSECT_WORD) {
        send_command(INTERSECT_CMD, [ second_word, third_word ]);
    } else if (first_word == UNION_WORD) {
        send_command(UNION_CMD, [ second_word, third_word ]);
    } else if (first_word == DIFFERENCE_WORD) {
        send_command(DIFFERENCE_CMD, [ second_word, third_word ]);
    } else if (first_word == MU_WORD) {
        send_command(WHAT_MU_CMD, [ second_word, third_word ]);
    } else if (first_word == CLEAR_ALL_WORD) {
        send_command(CLEAR_ALL_CMD, []);
    } else if (first_word == RESET_WORD) {
        if (id != llGetOwner()) {
            llSay(0, "Sorry, only the owner is allowed to reset this object.");
        } else {
            // tell the set manager to drop any contents also.
            send_command(CLEAR_ALL_CMD, []);
            llResetScript();
        }
    }
    // we cannot have a catch-all here for if we didn't understand; that will always be
    // getting hit whenever anyone talks nearby.
}

handle_link_message(integer sender, integer huff_id, string msg, key id)
{
    if (huff_id != SET_COMPARATOR_HUFFWARE_ID + REPLY_DISTANCE) {
        handle_notecard_message(sender, huff_id, msg, id);
        return;
    }
    list parms = llParseString2List(id, [HUFFWARE_PARM_SEPARATOR], []);
    string to_show;  // what to tell the user.
    if (llGetListLength(parms) == 1) {
        // mono-syllabic responses.
        string arf = llList2String(parms, 0);
        if (arf == "0") {
            to_show = "request failed";
        } else if (arf == "1") {
            to_show = "successful request";
        } else if (msg != WHAT_MU_CMD) {
            // don't know this one; assume it's a list.
            to_show = "[ " + arf + " ]";
        } else {
            // otherwise we assume it's a well-known answer we should just emit.
            to_show = arf;
        }
    } else {
        to_show = "[ " + dump_list(parms) + " ]";
    }
    if (NOISY_OY) log_it("reply says: " + to_show);
}

// this function fires off a request to the noteworthy library via a link message.
// noteworthy will look for a notecard with our particular signature in it and
// if it finds one, it will read the configuration therein.  an empty string is
// returned if noteworthy couldn't find anything.
request_configuration()
{
    log_it("reading configuration...");
    global_notecard_name = "";  // reset any previous card.
    // try to find a notecard with our configuration.
    response_code = -1 * (integer)randomize_within_range(23, 80000, FALSE);
    string parms_sent = wrap_parameters([MU_SIGNATURE, response_code]);
    llMessageLinked(LINK_THIS, NOTEWORTHY_HUFFWARE_ID, READ_NOTECARD_COMMAND,
         parms_sent);
}

// processes link messages received from the noteworthy library.
handle_notecard_message(integer which, integer num, string msg, key id)
{
    if ( (num != NOTEWORTHY_HUFFWARE_ID + REPLY_DISTANCE)
            || (msg != READ_NOTECARD_COMMAND) )
        return;  // not for us.
    // process the result of reading the notecard.
    list parms = llParseString2List(id, [HUFFWARE_PARM_SEPARATOR], []);
    string notecard_name = llList2String(parms, 0);
    integer response_for = llList2Integer(parms, 1);
    if (response_for != response_code) return;  // oops, this isn't for us.
//log_it("read more from: " + notecard_name);
    if (notecard_name == NOTECARD_READ_CONTINUATION) {
        // just save these items for now.
        global_config_list += llList2List(parms, 2, -1);
    } else if (notecard_name != BAD_NOTECARD_INDICATOR) {
        // a valid notecard has been found.
        global_notecard_name = notecard_name;  // record its name for later use.
        global_config_index = 0;  // we are starting over in the config list.
        // snag all but the first two elements for our config now.
        global_config_list += llList2List(parms, 2, -1);
        process_ini_config();
        log_it("now configured.");
        llSetTimerEvent(6);  // pause a bit before turning chat back on.
    } else {
        // we hated the notecards we found, or there were none.
        log_it("There seem to be no notecards with a first line of '"
            + MU_SIGNATURE
            + "', so no additional sets are defined.");
    }
}

///////////////

// eats a file with section names in square brackets, where every line defines a set element.
process_ini_config()
{
    integer indy;
    integer count = llGetListLength(global_config_list);
    string section_name;

    // iterate across the items in our configuration to look for ones that are not done yet.            
    for (indy = global_config_index; indy < count; indy++) {
        string line = llList2String(global_config_list, indy);
        // search for a section beginning.
        if (llGetSubString(line, 0, 0) == "[") {
            // we found the start of a section name.  now read the contents.
            indy++;  // skip section line.
            section_name = llGetSubString(line, 1, -2);
            if (NOISY_OY) log_it("set '" + section_name + "' is being defined.");
        }
        integer sec_indy;
        for (sec_indy = indy; sec_indy < count; sec_indy++) {
            // read the lines in the section.
            line = llList2String(global_config_list, sec_indy);
            if (llGetSubString(line, 0, 0) != "[") {
                if (NOISY_OY) log_it(line);
                hearing_some_voices(0, llGetScriptName(), llGetOwner(), line);
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

//////////////
// from hufflets:

integer debug_num = 0;
// a debugging output method.  can be disabled entirely in one place.
log_it(string to_say)
{
    debug_num++;
    // tell this to the owner.    
//    llOwnerSay(llGetScriptName() + "[" + (string)debug_num + "] " + to_say);
    // say this on an unusual channel for chat if it's not intended for general public.
//    llSay(108, llGetScriptName() + "[" + (string)debug_num + "] " + to_say);
    // say this on open chat that anyone can hear.  we take off the bling for this one.
    llSay(0, to_say);
}

// the string processing methods are not case sensitive.
  
// returns TRUE if the "pattern" is found in the "full_string".
integer matches_substring(string full_string, string pattern)
{ return (find_substring(full_string, pattern) >= 0); }

// returns the index of the first occurrence of "pattern" inside
// the "full_string".  if it is not found, then a negative number is returned.
integer find_substring(string full_string, string pattern)
{ return llSubStringIndex(llToLower(full_string), llToLower(pattern)); }

// returns TRUE if the "prefix" string is the first part of "compare_with".
integer is_prefix(string compare_with, string prefix)
{ return find_substring(compare_with, prefix) == 0; }

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

// returns a printable form of the list.
string dump_list(list to_show)
{
    integer len = llGetListLength(to_show);
    integer i;
    string text;
    for (i = 0; i < len; i++) {
        string next_line = llList2String(to_show, i);
        if (find_substring(next_line, " ") >= 0) {
            // this guy has a space in it, so quote it.
            next_line = "\"" + next_line + "\"";
        }
        text += next_line;
        if (i < len - 1) text += " ";
    }
    return text;
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
    return [ llStringTrim(x, STRING_TRIM), llStringTrim(y, STRING_TRIM) ];
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

// extracts space separated elements from a string, and honors quoting of either
// variety as long as the quotes come in pairs.  this enables the inclusion of
// spaces in the elements of the set.  note that this function requires a well-formed
// string where there are no multiple space characters in a row.
list parse_quoted_strings(string to_parse)
{
    list to_return;  // will pile up what we find in the string.
    integer quoting = FALSE;  // are we inside quotes?
    string curr_quote = "";  // what is current quote char, if any?
    string accum;  // accumulates parts of the current element.
    // loop over the string and apply our rules.
    integer i;
    for (i = 0; i < llStringLength(to_parse); i++) {
        string c = llGetSubString(to_parse, i, i);
        if (!quoting && (c == " ")) {
            // this space marks the end of a word.
            if (llStringLength(accum) > 0) {
//log_it("space adding to set: " + accum);
                to_return += [ accum ];
                accum = "";
            }
        } else if (quoting && (c == curr_quote)) {
            quoting = FALSE;
        } else if (!quoting && ( (c == "'") || (c == "\"") ) ) {
            // we've started into quoting mode.
            quoting = TRUE;
            curr_quote = c;
        } else {
            // if no condition applies, just add this to the accumulator.
            accum += c;
        }
    }
    // add the last thing we accumulated.
    if (llStringLength(accum) > 0) {
//log_it("last add to set: " + accum);
        to_return += [ accum ];
    }
    return to_return;
}

// returns the portion of the list between start and end, but only if they are
// valid compared with the list length.  an attempt to use negative start or
// end values also returns a blank list.
list chop_list(list to_chop, integer start, integer end)
{
    integer last_len = llGetListLength(to_chop) - 1;
    if ( (start < 0) || (end < 0) || (start > last_len) || (end > last_len) ) return [];
    return llList2List(to_chop, start, end);
}

// takes any redundant space characters out of the string.
string compress_spaces(string s)
{
    string to_return;
    integer in_space = FALSE;
    integer i;
    for (i = 0; i < llStringLength(s); i++) {
        string chunk = llGetSubString(s, i, i);
        if (chunk == " ") {
            if (in_space) {
                // we're have already seen a space.  don't keep this too.
                //continue;  no such keyword in lsl.
            } else {
                in_space = TRUE;
                to_return += chunk;
            }
        } else {
            // the current character was not a space, so just add it.
            in_space = FALSE;
            to_return += chunk;
        }
    }
    return to_return;
}

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

// end from hufflets.
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
        NOISY_OY = FALSE;
        // reset our relevant variables.
        global_notecard_name = "";
        global_config_list = [];
        global_config_index = 0;
        // request that the noteworthy library start looking for our notecard.
        request_configuration();
        // listen for commands from people.
        llListen(0, "", NULL_KEY, "");
        NOISY_OY = FALSE;
    }
    
    state_exit() { llSetTimerEvent(0.0); }

    touch_start(integer total_number) { show_instructions(); }

    listen(integer chan, string name, key id, string msg)
    { hearing_some_voices(chan, name, id, msg); }

    link_message(integer sender, integer num, string msg, key id) {
        if ( (num != SET_COMPARATOR_HUFFWARE_ID + REPLY_DISTANCE)
            && (num != NOTEWORTHY_HUFFWARE_ID + REPLY_DISTANCE) )
            return;  // not for us.
        handle_link_message(sender, num, msg, id);
    }
    
    timer() {
        NOISY_OY = TRUE;  // assume we can now chat with the user.  config is done hopefully.
        llSetTimerEvent(0.0);  // turn off timer now.
    }

    on_rez(integer parm) { show_instructions(); state rerun; }
}
