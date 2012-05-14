
// huffware script: set comparator, by fred huffhines.
//
// provides a library of functions for managing sets.
//
// this script is licensed by the GPL v3 which is documented at: http://www.gnu.org/licenses/gpl.html
// do not use it in objects without fully realizing you are implicitly accepting that license.
//

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
    // second and further parms are a list of elements that should be in the set.  return
    // value is a boolean for success.
string REMOVE_SET_CMD = "#rm-set";
    // trashes a named set.  first parm is the name.  returns a bool for success.
string ADD_ELEMENTS_CMD = "#add-elem";
    // adds more elements to an existing set.  first parm is the name, second and more are
    // the list of new elements.  set must already exist.  returns a bool for success.
string CUT_ELEMENTS_CMD = "#cut-elem";
    // removes a set of elements from an existing set.  first parm is set name, second etc
    // is list of elements to remove.  set must already exist.  returns bool.
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

// these are the possible types of set relationships.
string POSSIBILITY_ONE_MEANING = "one meaning (don gcig)";  // same set.
string POSSIBILITY_THREE_POSSES = "proper subset (mu gsum)";  // set A contains set B or vice-versa.
string POSSIBILITY_FOUR_POSSES = "four zones (mu bzhi)";  // non-null intersection, mutual non-null differences.
string POSSIBILITY_MUTUAL_EXCLUDE = "mutually exclusive ('gal ba)";  // no members are shared between the sets.
string POSSIBILITY_ERRONEOUS = "erroneous";  // not a valid request.

// global variables...

list all_sets;  // a list of condensed lists.
list set_names;  // the names of each list in our list.

integer DEBUGGING = TRUE;  // produces noisy logging if true.

// finds index of named list.
integer find_set(string name_to_find)
{
    return find_in_list(set_names, name_to_find);
}

// ensures that the list does not contain duplicate members.
list uniquify(list to_strain) {
    integer i;
    integer j;
    for (i = 1; i < llGetListLength(to_strain); i++) {
        string curr_i = llList2String(to_strain, i);
        for (j = 0; j < i; j++) {
            if (llList2String(to_strain, j) == curr_i) {
                // this one is a duplicate!  argh, remove it at index i.
                to_strain = chop_list(to_strain, 0, i - 1)
                    + chop_list(to_strain, i + 1, llGetListLength(to_strain) - 1);
                i--;  // skip back since that element no longer exists.
                j = i*3;  // scoot j out to stop inner loop.
            }
        }
    }
    return to_strain;
}

// this version retrieves bare sets, not ones that are fluffed out from inclusions.
list simple_get_set_by_index(integer index)
{
    if ( (index >= llGetListLength(set_names)) || (index < 0) ) return [];  // out of range.
    return llParseString2List(llList2String(all_sets, index), [HUFFWARE_ITEM_SEPARATOR], []);
}


// adds or replaces the set that is called "name".
integer define_set(string name, list contents)
{
    contents = uniquify(contents);
    integer curr = find_set(name);
    if (curr >= 0) {
        // an existing entry should be updated.
        remove_set_by_index(curr);
    }
    all_sets += [ wrap_item_list(contents) ];
    set_names += [ name ];
//hmmm: may want to bomb out with false if too much space in use.
    return TRUE;
}

// turns a combination of set linkages into the full set requested.
// assumes that the index is pre-validated!
list evaluate_set_components(integer index)
{
    string wrap = llList2String(all_sets, index);
    list to_return = llParseString2List(wrap, [HUFFWARE_ITEM_SEPARATOR], []);
    // check for our "equal to set X" case.
    if (llGetListLength(to_return) == 1) {
        string memb = llList2String(to_return, 0);
        if (llGetSubString(memb, 0, 0) == "=") {
//log_it("got a set assignment to other set called: " + llGetSubString(memb, 1, -1));
            // we found a set that's equal to another.  look up its equivalent.
            index = find_set(llGetSubString(memb, 1, -1));
            if (index < 0) return [];  // unknown.
            // got another set to check.
            wrap = ""; to_return = [];
            return evaluate_set_components(index);
        }
    }
    if (llGetListLength(to_return) > 0) {
        // scan for set includers.
        integer i;
        // must iterate forwards, since included sets can also include other sets.
        for (i = 0; i < llGetListLength(to_return); i++) {
            string curr = llList2String(to_return, i);
            if (llGetSubString(curr, 0, 0) == "+") {
                // found that this set includes another one.  add in the second set,
                // but chop out the element we just looked at.
                to_return = chop_list(to_return, 0, i - 1)
                    + chop_list(to_return, i + 1, llGetListLength(to_return) - 1)
                    + simple_get_set_by_index(find_set(llGetSubString(curr, 1, -1)));
                i--;  // skip back so we don't omit the new element at i.
            }
        }
    }
    return to_return;
}

// retrieves the specified set from the list using its index.
list get_set_by_index(integer index)
{
    if ( (index >= llGetListLength(set_names)) || (index < 0) ) return [];  // out of range.
    return evaluate_set_components(index);
}

// retrieves the specified set from the list using its name.
list get_set_by_name(string set_name) { return get_set_by_index(find_set(set_name)); }

// drops the set listed at the "index".
integer remove_set_by_index(integer index)
{
    if ( (index >= llGetListLength(set_names)) || (index < 0) ) return FALSE;  // out of range.
    set_names = llDeleteSubList(set_names, index, index);
    all_sets = llDeleteSubList(all_sets, index, index);
    return TRUE;
}

// tosses the set called "set_name".
integer remove_set_by_name(string set_name) { return remove_set_by_index(find_set(set_name)); }

integer add_items(string name, list new_items)
{
    new_items = uniquify(new_items);
    integer curr = find_set(name);
    if (curr < 0) return FALSE;  // unknown set.
    list content = get_set_by_index(curr);
    integer i;
    for (i = 0; i < llGetListLength(new_items); i++) {
        string new_item = llList2String(new_items, i);
        integer found = find_in_list(content, new_item);
        if (found < 0) {
            // was not present yet so add it.
            content += [ new_item ];
        }
    }
    define_set(name, content);
    return TRUE;
}

integer remove_items(string name, list dead_items)
{
    integer curr = find_set(name);
    if (curr < 0) return FALSE;  // unknown set.
    list content = get_set_by_index(curr);    
    integer i;
    for (i = 0; i < llGetListLength(dead_items); i++) {
        string dead_item = llList2String(dead_items, i);
        integer found = find_in_list(content, dead_item);
        if (found >= 0) {
            // was not present yet so add it.
            content = llDeleteSubList(content, found, found);
        }
    }
    
    define_set(name, content);
    return TRUE;
}

// returns the union of two named sets.
list set_union(string set_a, string set_b)
{
    integer where_a = find_set(set_a);
    if (where_a < 0) return [];  // not known.
    integer where_b = find_set(set_b);
    if (where_b < 0) return [];
    list to_return = get_set_by_index(where_a);
    list adding = get_set_by_index(where_b);
    integer i;
    for (i = 0; i < llGetListLength(adding); i++) {
        string curr = llList2String(adding, i);
        integer where = find_in_list(to_return, curr);
        if (where < 0) {
            // wasn't found in to_return, so add it.
            to_return += [ curr ];
        }
    }
    return to_return;
}

// returns the difference of two named sets.
list set_difference(string set_a, string set_b)
{
    integer where_a = find_set(set_a);
    if (where_a < 0) return [];  // not known.
    integer where_b = find_set(set_b);
    if (where_b < 0) return [];
    list to_return = get_set_by_index(where_a);
    list adding = get_set_by_index(where_b);
    integer i;
    for (i = 0; i < llGetListLength(adding); i++) {
        string curr = llList2String(adding, i);
        integer where = find_in_list(to_return, curr);
        if (where >= 0) {
            // this item does exist in both, so remove it.
            to_return = llDeleteSubList(to_return, where, where);
        }
    }
    return to_return;
}

// returns the intersection of two named sets.
list set_intersection(string set_a, string set_b)
{
    integer where_a = find_set(set_a);
    if (where_a < 0) return [];  // not known.
    integer where_b = find_set(set_b);
    if (where_b < 0) return [];
    list to_return = [];
    list a_con = get_set_by_index(where_a);
    list b_con = get_set_by_index(where_b);
    integer i;
    for (i = 0; i < llGetListLength(b_con); i++) {
        string curr = llList2String(b_con, i);
        integer where = find_in_list(a_con, curr);
        if (where >= 0) {
            // this item does exist in both, so include in the result.
            to_return += [ curr ];
        }
    }
    return to_return;
}

// returns TRUE if minor is a proper subset of major.
integer proper_subset(string minor_set, string major_set)
{
    integer where_a = find_set(minor_set);
    if (where_a < 0) return FALSE;  // not known.
    integer where_b = find_set(major_set);
    if (where_b < 0) return FALSE;
    list min_con = get_set_by_index(where_a);
    list maj_con = get_set_by_index(where_b);
    // the list must be smaller to be a proper subset.
    if (llGetListLength(min_con) >= llGetListLength(maj_con)) return FALSE;
    // now make sure anything in the min is also in maj.  if not, it's not
    // a subset at all.
    integer i;
    for (i = 0; i < llGetListLength(min_con); i++) {
        string curr = llList2String(min_con, i);
        integer where = find_in_list(maj_con, curr);
        if (where < 0) return FALSE;
    }
    // every item was present.  yippee.
    return TRUE;
}

// returns an assessment of the two sets in terms of the
// possible relationships between them.
string consider_mu(string set_a, string set_b)
{
    // firstly, validate these set names.
    integer where_a = find_set(set_a);
    if (where_a < 0) return POSSIBILITY_ERRONEOUS;  // not known.
    integer where_b = find_set(set_b);
    if (where_b < 0) return POSSIBILITY_ERRONEOUS;
    if (set_a == set_b) return POSSIBILITY_ONE_MEANING;  // easy check.
    // check whether they have any common members at all.
    list inter = set_intersection(set_a, set_b);
    if (inter == []) return POSSIBILITY_MUTUAL_EXCLUDE;
    // see if one set is contained in the other.
    if (proper_subset(set_a, set_b)
        || proper_subset(set_b, set_a) ) {
        return POSSIBILITY_THREE_POSSES;
    }
    // resign ourselves to loading up the sets to check equality.
    integer len_a = llGetListLength(get_set_by_index(where_a));
    integer len_b = llGetListLength(get_set_by_index(where_b));
    if (len_a == len_b) {
        // this is the only way they can be equivalent.
        // simple interpretation of this currently...
        list diff = set_difference(set_a, set_b);
        if (diff == []) return POSSIBILITY_ONE_MEANING;
        // so now, although they are of equal length, we know they are not
        // equal sets.
    }
    // if our logic is correct, there is only one option left.
    return POSSIBILITY_FOUR_POSSES;
}

// this should be invoked from the link_message event handler to process the requests
// for whatever service this library script provides.
handle_link_message(integer sender, integer huff_id, string msg, key id)
{
//log_it("link msg: " + (string)sender + " " + (string)huff_id + " msg=" + msg + " id=" + (string)id);
    // separate the list out
    list parms = llParseString2List(id, [HUFFWARE_PARM_SEPARATOR], []);
    
    // we interpret the "msg" as a command.
    if (msg == DEFINE_SET_CMD) {
        string name = llList2String(parms, 0);
        parms = llDeleteSubList(parms, 0, 0);
        integer to_return = define_set(name, parms);
        send_reply(sender, [ to_return ], msg);
    } else if (msg == REMOVE_SET_CMD) {
        string name = llList2String(parms, 0);
        integer to_return = remove_set_by_name(name);
        send_reply(sender, [ to_return ], msg);
    } else if (msg == ADD_ELEMENTS_CMD) {
        string name = llList2String(parms, 0);
        parms = llDeleteSubList(parms, 0, 0);
        integer to_return = add_items(name, parms);
        send_reply(sender, [ to_return ], msg);
    } else if (msg == CUT_ELEMENTS_CMD) {
        string name = llList2String(parms, 0);
        parms = llDeleteSubList(parms, 0, 0);
        integer to_return = remove_items(name, parms);
        send_reply(sender, [ to_return ], msg);
    } else if (msg == INTERSECT_CMD) {
        string name1 = llList2String(parms, 0);
        string name2 = llList2String(parms, 1);
        list to_return = set_intersection(name1, name2);
        send_reply(sender, to_return, msg);
    } else if (msg == UNION_CMD) {
        string name1 = llList2String(parms, 0);
        string name2 = llList2String(parms, 1);
        list to_return = set_union(name1, name2);
        send_reply(sender, to_return, msg);
    } else if (msg == DIFFERENCE_CMD) {
        string name1 = llList2String(parms, 0);
        string name2 = llList2String(parms, 1);
        list to_return = set_difference(name1, name2);
        send_reply(sender, to_return, msg);
    } else if (msg == WHAT_MU_CMD) {
        string name1 = llList2String(parms, 0);
        string name2 = llList2String(parms, 1);
        string to_return = consider_mu(name1, name2);
        send_reply(sender, [ to_return ], msg);
    } else if (msg == GET_SET_CMD) {        
        string name = llList2String(parms, 0);
        send_reply(sender, get_set_by_name(name), msg);
    } else if (msg == LIST_SET_NAMES_CMD) {
        send_reply(sender, set_names, msg);
    } else if (msg == CLEAR_ALL_CMD) {
        set_names = [];
        all_sets = [];
        send_reply(sender, [ 1 ], msg);
    } else {
        log_it("unknown set command: msg=" + msg + " id=" + (string)id);
    }
}


//////////////
//
// from hufflets...

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

// string processing hufflets...

// the string processing methods are not case sensitive.
  
// returns the index of the first occurrence of "pattern" inside
// the "full_string".  if it is not found, then a negative number is returned.
integer find_substring(string full_string, string pattern)
{ return llSubStringIndex(llToLower(full_string), llToLower(pattern)); }

// returns TRUE if the "prefix" string is the first part of "compare_with".
integer is_prefix(string compare_with, string prefix)
{ return find_substring(compare_with, prefix) == 0; }

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
//log_it("reply set: " + dump_list(parms));
    llMessageLinked(destination, SET_COMPARATOR_HUFFWARE_ID + REPLY_DISTANCE, command,
        llDumpList2String(parms, HUFFWARE_PARM_SEPARATOR));
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
        text += next_line;
        if (i < len - 1) text += " ";
    }
    return text;
}

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
//        log_it("memory left " + (string)llGetFreeMemory());
//hmmm: turn this into a report function.
    }
    
    link_message(integer sender, integer num, string str, key id) {
        if (num != SET_COMPARATOR_HUFFWARE_ID) return;
        handle_link_message(sender, num, str, id);
    }
}
