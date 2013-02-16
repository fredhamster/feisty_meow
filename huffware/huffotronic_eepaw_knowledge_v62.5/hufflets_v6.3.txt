
// huffware script: hufflets, by fred huffhines.
//
// functions that are commonly used but really are too simple to implement via IPC.
// these just get copied and pasted into other scripts.
// *note* that means you should not drop this script into an object.  it will not
// do anything for you.  instead, copy what you need out of here.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//

integer DEBUGGING = FALSE;
    // if this is set to true, then some functions produce noisier results.

//////////////

// diagnostic hufflets...

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

//////////////

// mathematical hufflets...

// returns a floating point absolute value.
float fabs(float take_absolute_value) {
    if (take_absolute_value >= 0.0) return take_absolute_value;
    else return -1.0 * take_absolute_value;
}

//////////////

// time hufflets...

// shows a somewhat pretty printed version of the number of seconds.
string time_text(integer seconds)
{
    float s_min = 60; float s_hour = 60 * s_min; float s_day = 24 * s_hour;
    float s_year = 365.25 * s_day;
    
    if (seconds < s_min) return (string)seconds + " seconds";
    else if (seconds < s_hour) return float_chop(seconds / s_min) + " minutes";
    else if (seconds < s_day) return float_chop(seconds / s_hour) + " hours";
    else if (seconds < s_year) return float_chop(seconds / s_day) + " days";
    else return float_chop(seconds / s_year) + " years";
}

//////////////

// string processing hufflets...

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

// sim-related hufflets...

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

//////////////

// list processing hufflets...

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

// removes the entry at "index" and instead inserts the list "to_insert"
// at that position.
list replace_entry(list to_modify, integer index, list to_insert)
{
    if (llGetListLength(to_modify) == 0) return to_insert;  // special case for empty.
    return llListReplaceList(to_modify, to_insert, index, index);
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

//////////////

integer MAX_CHAT_LINE = 1008;
    // the most characters we'll try to say in one chat.

dump_list_to_log(list to_show)
{
    string text = dump_list(to_show);  // get some help from the other version.
    integer len = llStringLength(text);
    integer i;
    for (i = 0; i < len; i += MAX_CHAT_LINE) {
        integer last_bit = i + MAX_CHAT_LINE - 1;
        if (last_bit >= len) last_bit = len - 1;
        string next_line = llGetSubString(text, i, last_bit);
        log_it(next_line);
    }
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
            next_line = "'" + next_line + "'";
        }
        text = text + next_line;
        if (i < len - 1) text = text + " ";
    }
    return text;
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
            // done with quotes, so add the quoted item, even if nil.
            to_return += [ accum ];
//log_it("quote adding to set: " + accum);
            accum = "";
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

//////////////

// action queue for postponed activities.  the first field held in a list item here
// is an integer action code.  the format of the remaining parameters is up to the
// caller, and they can be used as the final parameters for when the queued action
// gets handled.
list action_queue;

// looks at the action code at the head of the queue without removing the action.
integer peek_action_code()
{
    list fields = llParseString2List(llList2String(action_queue, 0), [HUFFWARE_PARM_SEPARATOR], []);
    return extract_action_code(fields);
}

// extracts the action code from a retrieved list.
integer extract_action_code(list to_parse) { return llList2Integer(to_parse, 0); }

// removes the current head of the action queue and returns it.
list pop_action_record()
{
    if (llGetListLength(action_queue) == 0) {
//log_it("failure in action q: no entries.");
        return [];
    }
    list top_action = llParseString2List(llList2String(action_queue, 0), [HUFFWARE_PARM_SEPARATOR], []);
    action_queue = llDeleteSubList(action_queue, 0, 0);
    return top_action;
}

// adds a new action to the end of the action queue.
push_action_record(integer action, list added_parms)
{
    action_queue += [ wrap_parameters([action] + added_parms) ];
}

//////////////

// randomizing hufflets...

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

// returns a random vector where x,y,z will be between "minimums" and "maximums"
// x,y,z components.  if "allow_negative" is true, then any component will
// randomly be negative or positive.
vector random_bound_vector(vector minimums, vector maximums, integer allow_negative)
{
    return <randomize_within_range(minimums.x, maximums.x, allow_negative),
        randomize_within_range(minimums.y, maximums.y, allow_negative),
        randomize_within_range(minimums.z, maximums.z, allow_negative)>;
}

// returns a vector whose components are between minimum and maximum.
// if allow_negative is true, then they can be either positive or negative.
vector random_vector(float minimum, float maximum, integer allow_negative)
{
    return random_bound_vector(<minimum, minimum, minimum>,
        <maximum, maximum, maximum>, allow_negative);
}

//////////////

// vector hufflets...

// returns TRUE if a is less than b in any component.
integer vector_less_than(vector a, vector b)
{ return (a.x < b.x) || (a.y < b.y) || (a.z < b.z); }

// returns TRUE if a is greater than b in any component.
integer vector_greater_than(vector a, vector b)
{ return (a.x > b.x) || (a.y > b.y) || (a.z > b.z); }

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

// prints the list of vectors with trimmed floats.
string vector_list_chop(list to_show)
{
    integer list_len = llGetListLength(to_show);
    string to_return;
    integer indy;
    for (indy = 0; indy < list_len; indy++) {
        if (indy != 0) to_return += HUFFWARE_ITEM_SEPARATOR;
        to_return += vector_chop((vector)llList2String(to_show, indy));
    }
    return to_return;
}

// returns a list with two components; a new vector and a boolean.
// the new vector starts from "starting_point".  it will have a vector
// between "minimum_addition" and "maximum_addition" added to it.
// if it is over the "minimum_allowed" or the "maximum_allowed", then
// it is reset to whichever it would have crossed over.  two booleans
// are also returned to indicate when the lower and upper limits were
// exceeded (in that order).
list limit_and_add(vector starting_point,
    vector minimum_allowed, vector maximum_allowed,
    vector minimum_addition, vector maximum_addition)
{
    integer too_low = FALSE;
    integer too_high = FALSE;
    vector new_location = starting_point;
    vector addition = random_bound_vector(minimum_addition, maximum_addition, FALSE);
//log_it("start=" + (string)starting_point + " addin=" + (string)addition);
    new_location += addition;
    if (vector_less_than(new_location, minimum_allowed)) {
        too_low = TRUE;
        new_location = minimum_allowed;
    } else if (vector_greater_than(new_location, maximum_allowed)) {
        too_high = TRUE;
        new_location = maximum_allowed;
    }
    return [ new_location, too_low, too_high ];
}

//////////////

// SL name hufflets...

// returns the position of the last space in "look_within" or a negative number.
integer find_last_space(string look_within)
{
    integer indy = llStringLength(look_within) - 1;
    while ( (indy >= 0) && (llGetSubString(look_within, indy, indy) != " ") ) indy--;
    return indy;
}

// returns the first name for an avatar with the "full_name".
string first_name(string full_name)
{
    integer finding = find_last_space(full_name);
    if (finding >= 0) return llGetSubString(full_name, 0, finding - 1);
    return full_name;  // failed to find space.
}

// returns the last name for an avatar with the "full_name".
string last_name(string full_name)
{
    integer finding = find_last_space(full_name);
    if (finding >= 0) return llGetSubString(full_name, finding + 1, -1);
    return full_name;  // failed to find space.
}

//////////////

// variable handling hufflets...

// substitutes a variable's name for its value.  note that variables are assumed to start
// with a dollar sign character, which should not be provided in the "variable_name" parameter.
string substitute_variable(string substitute_within, string variable_name, string variable_value)
{
    string to_return = substitute_within;
//log_it("before var sub: " + substitute_within);
    integer posn;
    while ( (posn = find_substring(to_return, "$" + variable_name)) >= 0) {
        // we found an occurrence of the variable.
        to_return = llDeleteSubString(to_return, posn, -1)  // keep part before.
            + variable_value  // add the value in place of the variable name.
            + llDeleteSubString(to_return, 0, posn + llStringLength(variable_name));
                // keep part after.
    }
//log_it("after var sub: " + to_return);
    return to_return;
}

// in "substitute_within", this finds any occurrences of items in the "variables_names"
// and replaces those with the corresponding item from "variable_values".
// note: this can be very stack intensive, so it might be better just to use multiple
// calls to the substitute_variable function.
string substitute_variable_list(string substitute_within, list variable_names, list variable_values)
{
    string to_return = substitute_within;
    integer vars = llGetListLength(variable_names);
    if (vars != llGetListLength(variable_values)) {
        log_it("error in substitute_variable_list: inequal number of variable names vs. values.");
        return to_return;
    }
    integer indy;
    for (indy = 0; indy < vars; indy++) {
        to_return = substitute_variable(to_return,
            llList2String(variable_names, indy),
            llList2String(variable_values, indy));
    }
    return to_return;
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

//////////////

// inventory hufflets...

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

// looks for an inventory item with the same prefix as the "basename_to_seek".
integer find_basename_in_inventory(string basename_to_seek, integer inv_type)
{
    integer num_inv = llGetInventoryNumber(inv_type);
    if (num_inv == 0) return -1;  // nothing there!
    integer indy;
    for (indy = 0; indy < num_inv; indy++) {
        if (is_prefix(llGetInventoryName(inv_type, indy), basename_to_seek))
            return indy;
    }
    return -2;  // failed to find it.
}

//////////////
//
// imported from auto-retire, which is the official source!...
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

// note that this new, lower memory version, depends on the inventory functions returning
// items in alphabetical order.
scrub_items_by_type(string this_guy, integer inventory_type)
{
    list removal_list;
    integer outer;
    for (outer = 0; outer < llGetInventoryNumber(inventory_type); outer++) {
        string curr = llGetInventoryName(inventory_type, outer);
        list split = compute_basename_and_version(curr);
        // make sure there was a comparable version number in this name.
        if ( (curr != this_guy) && llGetListLength(split)) {
            string curr_base = llList2String(split, 0);
            float curr_ver = (float)llList2String(split, 1);
//log_it("outer: " + curr_base + " / " + (string)curr_ver);
            integer inner;
            for (inner = outer + 1; inner < llGetInventoryNumber(inventory_type); inner++) {
                string next_guy = llGetInventoryName(inventory_type, inner);
                list comp_split = compute_basename_and_version(next_guy);
                if (llGetListLength(comp_split)) {
                    string comp_base = llList2String(comp_split, 0);
                    float comp_ver = (float)llList2String(comp_split, 1);
                    // okay, now we can actually compare.
                    if (curr_base != comp_base) {
                        // break out of inner loop.  we are past where the names can matter.
                        inner = 2 * llGetInventoryNumber(inventory_type);
                    } else {
//log_it("inner: " + comp_base + " / " + (string)comp_ver);
                        if (curr_ver <= comp_ver) {
                            // the script at inner index is comparable or better than
                            // the script at the outer index.
                            removal_list += curr;
                        } else {
                            // this inner script must be inferior to the outer one,
                            // somehow, which defies our expectation of alphabetical ordering.
                            removal_list += next_guy;
                        }
                    }
                }
            }
        }
    }

    // now actually do the deletions.
    for (outer = 0; outer < llGetListLength(removal_list); outer++) {
        string to_whack = llList2String(removal_list, outer);
        log_it("removing older asset: " + to_whack);
        llRemoveInventory(to_whack);
    }
}

// ensures that only the latest version of any script or object is kept in our inventory.
destroy_older_versions()
{
    // firstly, iterate across scripts to clean out older versions.
    scrub_items_by_type(llGetScriptName(), INVENTORY_SCRIPT);
    // secondly, try to clean out the objects.
    scrub_items_by_type(llGetScriptName(), INVENTORY_OBJECT);
}

//////////////

// interprocess communication hufflets...  i.e., IPC parts.

// a repository for commonly used inter-process communication (IPC) source
// code.  hopefully in the future this will be worthwhile coming to first,
// when a new link message API is being built.

// an example link message API...
//////////////
integer SERVICE_X_HUFFWARE_ID = -14000;
    // a unique ID within the huffware system for this script.
string HUFFWARE_PARM_SEPARATOR = "{~~~}";
    // this pattern is an uncommon thing to see in text, so we use it to separate
    // our commands in link messages.
string HUFFWARE_ITEM_SEPARATOR = "{|||}";
    // used to separate lists of items from each other when stored inside a parameter.
    // this allows lists to be passed as single string parameters if needed.
integer REPLY_DISTANCE = 100008;  // offset added to service's huffware id in reply IDs.
//////////////

// then the main body of IPC support functions.

// joins a list of parameters using the parameter sentinel for the library.
string wrap_parameters(list to_flatten)
{ return llDumpList2String(to_flatten, HUFFWARE_PARM_SEPARATOR); }

// joins a list of sub-items using the item sentinel for the library.
string wrap_item_list(list to_wrap)
{ return llDumpList2String(to_wrap, HUFFWARE_ITEM_SEPARATOR); }

// handles when blank strings need to come through the pipe.
string wrap_blank_string(string to_wrap)
{
    if (llStringLength(to_wrap)) return to_wrap;  // that one is okay.
    return "\"\"";  // return a quoted nothing as a signal for a blank.
}

// undoes a previously wrapped blank string.
string interpret_blank_string(string to_unwrap)
{
    if (to_unwrap == "\"\"") return "";  // that was an encoded blank.
    return to_unwrap;  // no encoding.
}

// a simple version of a reply for a command that has been executed.  the parameters
// might contain an outcome or result of the operation that was requested.
send_reply(integer destination, list parms, string command)
{
    llMessageLinked(destination, SERVICE_X_HUFFWARE_ID + REPLY_DISTANCE, command,
        llDumpList2String(parms, HUFFWARE_PARM_SEPARATOR));
}

// this should be invoked from the link_message event handler to process the requests
// for whatever service this library script provides.
handle_link_message(integer sender, integer huff_id, string msg, key id)
{
log_it("link msg: " + (string)sender + " " + (string)huff_id + " msg=" + msg + " id=" + (string)id);
    // this check is more for the server; the server should listen on the main huffware id.
    if (huff_id != SERVICE_X_HUFFWARE_ID) {
        // the check below would make more sense in the client; it should listen on huffware id + REPLY_DISTANCE.
        if (huff_id != SERVICE_X_HUFFWARE_ID + REPLY_DISTANCE) return;  // totally not for us.
        // this is a reply to a previous request.
        if (msg == "moobert") {
            // handle the response.
        }
        // done with reply handling.
        return;
    }
    // separate the list out
    list parms = llParseString2List(id, [HUFFWARE_PARM_SEPARATOR], []);
    
    //example usage of parsed pieces.
    key k = (key)llList2String(parms, 0);
    string s = interpret_blank_string(llList2String(parms, 1));
    integer i = (integer)llList2String(parms, 2);
    vector v = (vector)llList2String(parms, 3);
    
    // we interpret the "msg" as a command.  the "id" has extra parameters.
    if (msg == "unflux") {
        // do something
    }
}

//////////////

// graphical hufflets...

// a replacement for the deprecated llMakeExplosion function; this bangs the
// "texture" out as a particle with the "size" in meters.  the number of 
// particles in a burst is passed in "particle_count".  the function will
// generate a timer event after "timeout" seconds (pass zero for no timeout).
make_explosion(string texture, vector size, integer particle_count,
    float timeout)
{
    llParticleSystem([PSYS_PART_FLAGS, PSYS_PART_WIND_MASK | PSYS_PART_EMISSIVE_MASK,
        PSYS_SRC_PATTERN, PSYS_SRC_PATTERN_EXPLODE,
        PSYS_PART_START_SCALE, size,
        PSYS_PART_END_SCALE, size,
        PSYS_SRC_BURST_PART_COUNT, particle_count,
        PSYS_PART_MAX_AGE, 2,
        PSYS_SRC_TEXTURE, texture]);
    llSetTimerEvent(timeout);
}
// the event handler for the particle system to be shut down again.
//timer() { llParticleSystem([]); }

//////////////

// the following state mumbo jumbo is so that our scripts do not go crazy when the huffotronic
// updater is rezzed.  that device has most of our scripts in it, and cannot always crush their
// state fast enough to stop them all.
default
{
    state_entry() { if (llSubStringIndex(llGetObjectName(),  "huffotronic") < 0) state real_default; }
    on_rez(integer parm) { state rerun; }
}
state rerun { state_entry() { state default; } }

state real_default
{
    state_entry() {
        log_it("memory left " + (string)llGetFreeMemory());
    }
}
